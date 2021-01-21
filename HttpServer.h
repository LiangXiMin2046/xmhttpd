#ifndef SIMPLEHTTP_HTTPSERVER_H
#define SIMPLEHTTP_HTTPSERVER_H

#include "Poller.h"
#include "SocketsOps.h"
#include "noncopyable.h"
#include "HttpConnection.h"
#include "TimerMonitor.h"

#include <memory>

class HttpServer : noncopyable
{
public:

	HttpServer(uint16_t port);
	~HttpServer();
	void start();
private:	

	typedef std::vector<int> pollfds;
	typedef std::unordered_map<int,std::unique_ptr<HttpConnection>> HttpConnections;

	const int timeLimit_ = 1200; //decides connection time limit 
	const int maxMessageSize_ = 10000; //for every request,message length should be less than 10k
	int timerfd_;
	int listenfd_;
	int idlefd_; //avoid EMFILE
	pollfds activefds_;
	std::unique_ptr<Poller> poller_;
	std::unique_ptr<TimerMonitor> monitor_;
	HttpConnections connections_; 

	void acceptConnection();
	void setInterval(int timerfd);
	bool updateMonitor(int fd);
	void processMessage(int fd);
	void errorHappens(int fd,HttpResponse::HttpStatusCode statusCode,const std::string& statusMessage);
	void closeConnection(int fd);
	bool excuteCgi(int fd);
	bool serverPage(int fd);
}; 

#endif //SIMPLEHTTP_HTTPSERVER_H
