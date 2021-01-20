#ifndef SIMPLEHTTP_RESPONSE_H
#define SIMPLEHTTP_RESPONSE_H

#include "noncopyable.h"

#include <unordered_map>

class HttpResponse : noncopyable
{
public:
	enum HttpStatusCode
	{
		kUnknown,
		k200Ok = 200,
		k400BadRequest = 400,
		k404NotFound = 404,
		k405MethodNotAllowed = 405,
		k411LengthRequired = 411,
		k414RequestUrlTooLong = 414,
		k500InternalServerError = 500,
		k505HttpVersionNotSupported = 505
	};

	HttpResponse()
      :  statusCode_(kUnknown)	
	{
	}

	void setStatusCode(HttpStatusCode code)
	{
		statusCode_ = code;
	}

	void setStatusMessage(const std::string& message)
	{
		statusMessage_ = message;
	}

	void addHeader(const std::string& key,const std::string& value)
	{
		headers_[key] = value;
	}

	void addBody(const char* buf,size_t n)
	{
		body_.append(buf,n);
	}

	int bodySize() const
	{
		return static_cast<int>(body_.size());
	}

	void appendToBuffer(std::string& output)
	{
		//add status line
		output += "HTTP/1.1 " + std::to_string(statusCode_) + " " + statusMessage_ + "\r\n";
		//add headers
		for(auto& r : headers_)
		{
			output += r.first;
			output += ": ";
			output += r.second;
			output += "\r\n";
		}
		output += "\r\n";

		//add body
		output += body_;
	}

	void reset()
	{
		statusCode_ = kUnknown;
		statusMessage_.resize(0);
		headers_.clear();
		body_.resize(0);	
	}
private:
	HttpStatusCode statusCode_;
	std::string statusMessage_;
	std::unordered_map<std::string,std::string> headers_;
	std::string body_;
};

#endif
