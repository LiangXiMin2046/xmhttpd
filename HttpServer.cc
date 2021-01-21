#include "HttpServer.h"
#include "TimerOps.h"

#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h> //for memset

/*
*constructor function
*/

HttpServer::HttpServer(uint16_t port)
  :  listenfd_(Sockets::createListenSocketOrDie(port)),
     idlefd_(::open("/dev/null",O_RDONLY | O_CLOEXEC)),
     poller_(std::move(Poller::getPoller(listenfd_))),
     monitor_(std::move(new TimerMonitor)),
     timerfd_(Timer::createTimerfd(CLOCK_MONOTONIC,TFD_CLOEXEC | TFD_NONBLOCK))
{
	setInterval(timerfd_);
	poller_->addFd(timerfd_);
}

HttpServer::~HttpServer()
{

}

/*
* whatever accepting a connection or responsing to client or closing connection
* handled by this function.
*/

void HttpServer::start()
{
	while(true)
	{
		activefds_.clear();
		poller_->poll(-1,&activefds_);
		for(int i = 0; i < activefds_.size(); i++)
		{
			if(activefds_[i] == listenfd_)
			{
				acceptConnection();
			}
			else if(activefds_[i] == timerfd_)
			{
				uint64_t buf;
				::read(timerfd_,&buf,sizeof buf); //data should be read
				std::vector<int> fds = monitor_->checkTimeout(timeLimit_);
				for(int i = 0; i < fds.size(); i++)
				{
					closeConnection(fds[i]);
				}
			}
			else
			{
				processMessage(activefds_[i]);
			}
		}
	}
}

/*
* accept a new connection here.
* prepare a idlefd once EMFILE happens(libevent did so).
*/

void HttpServer::acceptConnection()
{
	int connfd = Sockets::accept(listenfd_);
	if(connfd == -1)
	{
		if(errno == EMFILE)
		{
			::close(idlefd_);
			idlefd_ = Sockets::accept(listenfd_);
			::close(idlefd_);
			idlefd_ = ::open("/dev/null",O_RDONLY | O_CLOEXEC);
		}
		else
		{
			exit(0);
		}
		return;
	}
	
	else
	{
		if(!updateMonitor(connfd))
		{
			::close(connfd);
		}
		else
		{
			if(!poller_->addFd(connfd))
			{
				::close(connfd);
			}
			else
			{
				connections_.insert(make_pair(connfd,std::move(std::unique_ptr<HttpConnection>(new HttpConnection(connfd)))));	
			}
		}
	}
}

/*
* close connection
*/
void HttpServer::closeConnection(int fd)
{
	poller_->removeFd(fd);
	monitor_->removeNode(fd);
	connections_.erase(fd);
}

/*
* parse the message from client.
* errors also can be processed.
*/

void HttpServer::processMessage(int fd)
{
	int n;
	char buf[10240];
	bool ok = false;
	while((n = read(fd,buf,sizeof buf)) > 0)
	{
		connections_[fd]->appendInputBuffer(buf,n);
		if(connections_[fd]->messageLength() >= maxMessageSize_)
		{
			errorHappens(fd,HttpResponse::k414RequestUrlTooLong,"Request-URL too long");
			return;
		}
	}
	if(n == -1 && errno != EAGAIN)
	{
		errorHappens(fd,HttpResponse::k500InternalServerError,"Internal server error");
		return;
	}
	if(n == 0)
	{
		closeConnection(fd);
		return;
	}
	updateMonitor(fd);
	if(!connections_[fd]->parseMessage())
	{
		connections_[fd]->sendResponse();
		closeConnection(fd);
	}
	if(connections_.count(fd) && connections_[fd]->gotAll())
	{
		if(connections_[fd]->request().getCgi())
		{
			ok = excuteCgi(fd);
		}		
		else
		{
			ok = serverPage(fd);
		}
		if(ok)
		{
			connections_[fd]->response().setStatusCode(HttpResponse::k200Ok);
			connections_[fd]->response().setStatusMessage("OK");
			connections_[fd]->response().addHeader("Content-Length",std::to_string(connections_[fd]->response().bodySize()));
			connections_[fd]->response().addHeader("Server","ximin on cloud");
			connections_[fd]->response().addHeader("Content-Type","text/html; charset=utf-8");
			bool close = false;
			if(connections_[fd]->request().getVersion() == HttpRequest::kHttp10 || connections_[fd]->request().getHeader("Connection") == "close")
			{
				close = true;
			}
			connections_[fd]->sendResponse();
			if(close)
			{
				closeConnection(fd);
			}
			else
			{
				connections_[fd]->reset();
			}
		}
	}	
}

/*
* if client just request a static page.
* just read and  write.
*/

bool HttpServer::serverPage(int fd)
{
	std::string path = connections_[fd]->request().getPath();
	if(path == "/")
	{
		path = "./docs/index.html";
	} 
	else
	{
		path = "./docs" + path;
	}

	int pageFd = open(path.data(),O_RDONLY | O_CLOEXEC);
	if(pageFd == -1)
	{
		errorHappens(fd,HttpResponse::k404NotFound,"Not found");
		return false;
	}
	int n;
	char buf[10240];
	while((n = read(pageFd,buf,sizeof buf)) != 0)
	{
		connections_[fd]->response().addBody(buf,n);
	}
	::close(pageFd);
	return true;
}

/*
* if client request a dynamic web page,use execl family function
* and pipe communication
*/

bool HttpServer::excuteCgi(int fd)
{
	std::string path = connections_[fd]->request().getPath();
	path = "./docs/cgi-bin/"+path;
	struct stat statbuf;	
	if(stat(path.data(),&statbuf) == -1)
	{
		errorHappens(fd,HttpResponse::k500InternalServerError,"Internal server error");
		return false;
	}

	if((!(statbuf.st_mode & S_IXUSR)) && (!(statbuf.st_mode & S_IXGRP)) && (!(statbuf.st_mode & S_IXOTH)))	
	{
		errorHappens(fd,HttpResponse::k500InternalServerError,"Internal server error");
		return false;
	}

	std::string para;
	if(connections_[fd]->request().getMethod() == HttpRequest::kGet)
		para = connections_[fd]->request().getQuery();
	else
		para = connections_[fd]->request().getBody();	
	::setenv("HTTPD_PARAMETER",para.data(),1);	
	int pe[2];
	if(::pipe(pe) == -1)
	{
		errorHappens(fd,HttpResponse::k500InternalServerError,"Internal server error");
		return false;
	}
	switch(fork())
	{
		case -1:
			errorHappens(fd,HttpResponse::k500InternalServerError,"Internal server error");
			return false;		
		case 0:
			::close(pe[0]);
			dup2(pe[1],STDOUT_FILENO);
			execl(path.data(),NULL);
		default:
			break;
	}
	::close(pe[1]);
	char buf[10240];
	int n;
	while((n = ::read(pe[0],buf,sizeof buf)) > 0)
	{
		connections_[fd]->response().addBody(buf,n);
	}
	::close(pe[0]);
	::wait(NULL);
	return true;
}

/*
* error process.
*/

void HttpServer::errorHappens(int fd,HttpResponse::HttpStatusCode statusCode,const std::string& statusMessage)
{
		connections_[fd]->response().setStatusCode(statusCode);
		connections_[fd]->response().setStatusMessage(statusMessage);
		connections_[fd]->sendResponse();
		closeConnection(fd);	
}

/*
* set time interval for monitor_ to check overtime connections.
*/

void HttpServer::setInterval(int timerfd)
{
	struct itimerspec nv;
	struct timespec start;
	::clock_gettime(CLOCK_MONOTONIC,&start);
	memset(&nv,0,sizeof nv);
	nv.it_value.tv_sec = timeLimit_;
	nv.it_interval.tv_sec = timeLimit_;
	Timer::setTimer(timerfd,0,&nv,NULL);
}

/*
* update the last message arrived time.
*/

bool HttpServer::updateMonitor(int fd)
{
	int now = time(NULL);
	return monitor_->putIntoMonitor(fd,now);
}
