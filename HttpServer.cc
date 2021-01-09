#include "HttpServer.h"

#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <sys/wait.h>

HttpServer::HttpServer(uint16_t port)
  :  listenfd_(Sockets::createListenSocketOrDie(port)),
     idlefd_(::open("/dev/null",O_RDONLY | O_CLOEXEC)),
	poller_(std::move(Poller::getPoller(listenfd_)))
{

}

HttpServer::~HttpServer()
{

}

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
			else
			{
				processMessage(activefds_[i]);
			}
		}
	}
}

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
	
	if(!poller_->addFd(connfd))
		::close(connfd);
	else
		connections_.insert(make_pair(connfd,std::move(std::unique_ptr<HttpConnection>(new HttpConnection(connfd)))));	
}

void HttpServer::closeConnection(int fd)
{
	poller_->removeFd(fd);
	connections_.erase(fd);
}

void HttpServer::processMessage(int fd)
{
	int n;
	char buf[10240];
	bool ok = false;
	while((n = read(fd,buf,sizeof buf)) > 0)
	{
		connections_[fd]->appendInputBuffer(buf,n);
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

void HttpServer::errorHappens(int fd,HttpResponse::HttpStatusCode statusCode,const std::string& statusMessage)
{
		connections_[fd]->response().setStatusCode(statusCode);
		connections_[fd]->response().setStatusMessage(statusMessage);
		connections_[fd]->sendResponse();
		closeConnection(fd);	
}
