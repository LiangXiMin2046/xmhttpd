//this class is highly inspired by muduo 

#ifndef SIMPLEHTTP_HTTPREQUEST_H
#define SIMPLEHTTP_HTTPREQUEST_H

#include "noncopyable.h"

#include <unordered_map>
#include <assert.h>

class HttpRequest : noncopyable
{
public:

	enum Method
	{
		kInvalid,kGet,kPost,kHead,kPut,kDelete //actually my server only support GET and POST method
	};

	enum HttpVersion
	{
		kUnknow,kHttp10,kHttp11
	};

	HttpRequest()
      :  method_(kInvalid),
         version_(kUnknow),
         cgiFlag_(false)
	{
	}
	~HttpRequest()
	{
	}
	bool setMethod(const std::string& m)
	{
		assert(method_ == kInvalid);
		//std::string m(beg,end);
		if(m == "GET")
		{
			method_ = kGet;
		}	
		else if(m == "POST")		
		{
			method_ = kPost;
		}
		else if(m == "HEAD")
		{
			method_ = kHead;	
		}
		else if(m == "PUT")
		{
			method_ = kPut;
		}
		else if(m == "DELETE")
		{
			method_ = kDelete;
		}
		else
		{
			method_ = kInvalid;
		}
		return method_ != kInvalid;
	}

	Method getMethod()
	{
		return method_;
	}

	void setPath(const std::string& path)
	{
		path_ = path;
	}

	const std::string& getPath() const
	{
		return path_;
	}

	void setQuery(const std::string& query)
	{
		query_ = query;
	}

	const std::string& getQuery() const
	{
		return query_;
	}

	void setCgi(bool existQuery)
	{
		cgiFlag_ = existQuery;
	}

	bool getCgi() const
	{
		return cgiFlag_;
	}

	void setVersion(HttpVersion v)
	{
		version_ = v;
	}	

	HttpVersion getVersion()
	{
		return version_;
	}

	bool addHeader(const std::string& header)
	{
		size_t pos = header.find(":");
		if(pos == std::string::npos)
			return false;
		std::string key = header.substr(0,pos);
		if(pos+2 >= header.size())
			return false;
		if(pos+1 < header.size() && header[pos+1] == ' ')
			pos += 2;
		else
			return false;
		std::string value = header.substr(pos);
		//value.resize(value.size()-2);
		headers_[key] = value;
		return true;				
	}

	std::string getHeader(const std::string& key) const
	{
		std::string result;
		std::unordered_map<std::string,std::string>::const_iterator it = headers_.find(key);
		if(it != headers_.end())
		{
			result = it->second;
		}
		return result;
	}

	const std::unordered_map<std::string,std::string>& headers() const
	{
		return headers_;
	}
 
	void reset()
	{
		method_ = kInvalid;
		version_ = kUnknow;
		cgiFlag_ = false;
		bodies_.resize(0);
		headers_.clear();
	}

	void addBody(const char* data,int size)
	{
		bodies_.append(data,size);	
	}

	std::string getBody() const
	{
		return bodies_;
	}
private:

	Method method_;
	std::string path_;
	std::string query_;
	HttpVersion version_;
	bool cgiFlag_;
	std::unordered_map<std::string,std::string> headers_;
	std::string bodies_;	
};

#endif //SIMPLEHTTP_HTTPREQUEST_H
