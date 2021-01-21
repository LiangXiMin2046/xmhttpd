#include "HttpConnection.h"
#include "HttpRequest.h"

#include <assert.h>
#include <sstream>
#include <stdlib.h>

/*
* parse request line of HTTP message.
* if return false,400 BadRequest happens. 
*/

bool HttpConnection::parseRequestLine(const std::string& request)
{
	std::string method,url,version;
	std::istringstream ss(request);
	ss >> method >> url >> version; //maybe not efficient enough
	bool result = false;

	//process method
	result = request_.setMethod(method);
	if(result == false)
	{
		response_.setStatusCode(HttpResponse::k400BadRequest);
		response_.setStatusMessage("Bad request");
		return false;
	}
	if(request_.getMethod() == HttpRequest::kPost)
		request_.setCgi(true);
	if(request_.getMethod() != HttpRequest::kGet && request_.getMethod() != HttpRequest::kPost)
	{
		response_.setStatusCode(HttpResponse::k405MethodNotAllowed);
		response_.setStatusMessage("Method not allowed");
		return false;
	}
	//process url
	int pos = url.find("?");
	if(pos == std::string::npos)
	{
		request_.setPath(url);
	}
	else
	{
		if(request_.getMethod() == HttpRequest::kPost)
		{
			response_.setStatusCode(HttpResponse::k400BadRequest);
			response_.setStatusMessage("Bad request");
			return false;
		}
		request_.setPath(url.substr(0,pos));
		request_.setQuery(url.substr(pos+1));
		request_.setCgi(true);
	}

	//process http version
	if(version == "HTTP/1.0")
	{
		request_.setVersion(HttpRequest::kHttp10);
	}
	else if(version == "HTTP/1.1")
	{	
		request_.setVersion(HttpRequest::kHttp11);
	}
	else
	{
		response_.setStatusCode(HttpResponse::k505HttpVersionNotSupported);
		response_.setStatusMessage("Http Version Not Supported");
		return false;
	}
	return true;
}

/*
* parse HTTP message,an important function.
* always remeber that client and network are not reliable.
* message delay and bad request always happens.
*/

bool HttpConnection::parseMessage()
{
	bool more = true,success = true;
	while(more)
	{
		if(state_ == kNeedRequest)
		{
			assert(toRead_ == 0);
			size_t pos = inputBuffer_.find("\r\n");
			if(pos != std::string::npos)
			{
				std::string request = inputBuffer_.substr(0,pos);
				success = parseRequestLine(request);
				toRead_ = pos + 2;
				if(success)
				{
					state_ = kNeedHeaders;
				}
				else
				{
					more = false;
				}
			}
			else
			{
				more = false;
			}	
		}
		else if(state_ == kNeedHeaders)
		{
			size_t pos = inputBuffer_.find("\r\n",toRead_);
			if(pos != std::string::npos)
			{
				assert(pos >= toRead_);
				std::string header = inputBuffer_.substr(toRead_,pos - toRead_);
				toRead_ = pos + 2;
				if(header.empty())
				{
					if(request_.getMethod() == HttpRequest::kPost)
					{
						state_ = kNeedBody;
					}
					else
					{
						state_ = kGotAll;
						more = false;
					}
				}	
				else
				{
					success = request_.addHeader(header);
				}
				if(success == false)
				{
					more = false;
					response_.setStatusCode(HttpResponse::k400BadRequest);
					response_.setStatusMessage("Bad request");
				}
			}
			else
			{
				more = false;
			}	
		}
		else if(state_ == kNeedBody)
		{
			assert(toRead_ <= inputBuffer_.size());
			std::string len = request_.getHeader("Content-Length");
			if(len.empty())
			{
				more = false;
				success = false;				
				response_.setStatusCode(HttpResponse::k411LengthRequired);
				response_.setStatusMessage("Length required");
			}
			else
			{
				const int contentLen = atoi(len.data());
				std::string body = inputBuffer_.substr(toRead_);
				if(bodyLen_ + body.size() > contentLen)
				{
					int left = contentLen - bodyLen_;
					body.resize(left);
				}	
				request_.addBody(body.data(),body.size());
				bodyLen_ += body.size();
				if(bodyLen_ == contentLen)
					state_ = kGotAll;
				more = false;
			}
		}
	}
	return success;
}

/*
* write back HTTP response message.
*/

void HttpConnection::sendResponse()
{
	response_.appendToBuffer(outputBuffer_);
	int send = 0,total = static_cast<int>(outputBuffer_.size());
	//make sure all the bytes will be sent
	while(send < total)
	{
		int n = write(fd_,outputBuffer_.data() + send,total - send);
		if(n != -1) //if core buffer is not empty
		{
			send += n;
		}
	}
}

/* 
* just for test of request,see ./test.httpcontest.cc
*/

std::string HttpConnection::reqAsString()
{
	std::string info;
	if(request_.getMethod() == HttpRequest::kGet)
		info += "method: GET\n";	
	else
		info += "method: Post\n";
	info += "path_: " + request_.getPath() + "\n";
	info += "query_: " + request_.getQuery() + "\n";
	if(request_.getVersion() == HttpRequest::kHttp10)
		info += "version: HTTP/1.0\n";
	else if(request_.getVersion() == HttpRequest::kHttp11)
		info += "version: HTTP/1.1\n";
	else 
		info += "version: error version\n";
	
	const std::unordered_map<std::string,std::string>& headers = request_.headers();
	for(auto& r : headers)
	{
		info += r.first;
		info += ": ";
		info += r.second + "\n"; 
	}
	if(request_.getCgi())
		info += "Cgi: true\n";
	else
		info += "Cgi: false\n";
	info += "body: " + request_.getBody() + "\n";
	return info;
}
