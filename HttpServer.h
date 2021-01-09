#ifndef SIMPLEHTTP_HTTPSERVER_H
#define SIMPLEHTTP_HTTPSERVER_H

#include "Poller.h"
#include "SocketsOps.h"
#include "noncopyable.h"
#include "HttpConnection.h"

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

	int listenfd_;
	int idlefd_; //avoid EMFILE
	pollfds activefds_;
	std::unique_ptr<Poller> poller_;
	HttpConnections connections_; 

	void acceptConnection();
	void processMessage(int fd);
	void errorHappens(int fd,HttpResponse::HttpStatusCode statusCode,const std::string& statusMessage);
	void closeConnection(int fd);
	bool excuteCgi(int fd);
	bool serverPage(int fd);
}; 

#endif //SIMPLEHTTP_HTTPSERVER_H
