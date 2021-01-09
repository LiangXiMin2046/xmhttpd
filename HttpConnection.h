#ifndef SIMPLEHTTP_HTTPCONNECTION_H
#define SIMPLEHTTP_HTTPCONNECTION_H

#include <unistd.h>

#include "noncopyable.h"
#include "HttpRequest.h"
#include "HttpResponse.h"

class HttpConnection : noncopyable
{
public:
	enum ParseState
	{
		kNeedRequest,
		kNeedHeaders,
		kNeedBody,
		kGotAll,	
	};

	HttpConnection(int fd)
	  :  fd_(fd),
		 state_(kNeedRequest),
		 toRead_(0),
		 bodyLen_(0)
	{

	}

	~HttpConnection()
	{
		::close(fd_);
	}

	bool gotAll() const
	{
		return state_ == kGotAll;
	}

	void reset()
	{
		request_.reset();
		response_.reset();
		state_ = kNeedRequest;
		toRead_ = 0;
		bodyLen_ = 0;
		inputBuffer_.resize(0);
		outputBuffer_.resize(0);
	}

	const HttpRequest& request() const
	{
		return request_;
	}

	HttpRequest& request()
	{
		return request_;
	}

	HttpResponse& response()
	{
		return response_;
	}
	
	bool parseMessage();
	std::string reqAsString();
	void sendResponse();
	void appendInputBuffer(const char* beg,int size)
	{
		inputBuffer_.append(beg,size);		
	}
private:
	bool parseRequestLine(const std::string& request);	
	
	ParseState state_;
	const int fd_;
	std::string inputBuffer_;
	std::string outputBuffer_;
	HttpRequest request_;
	HttpResponse response_;
	int toRead_; //for inputBuffer,the position to parse
	int bodyLen_;
};

#endif //#SIMPLEHTTP_HTTPCONNECTION_H
