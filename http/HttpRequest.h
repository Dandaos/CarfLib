#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H
#include<map>
#include<string>
#include "base/Logger.h"
class HttpRequest
{
    public:
        enum Method
        {
            GET = 0,POST,HEAD,PUT,DELETE,TRACE,OPTIONS,CONNECT,PATH
        };
        enum Version
        {
            kUnknown,kHttp10,kHttp11
        };
        HttpRequest():method_(GET),version_(kHttp11){}
        void setVersion(Version v)
        {
            version_ = v;
        }

        Version getVersion() const
        { return version_; }

        bool setMethod(const char*start,const char*end)
        {
            std::string method(start,end);
            bool succeed=false;
            if (method == "GET")
            {
                method_ = GET;succeed=true;
            }
            else if (method == "POST")
            {
                method_ = POST;succeed=true;
            }
            else if (method == "HEAD")
            {
                method_ = HEAD;succeed=true;
            }
            else if (method == "PUT")
            {
                method_ = PUT;succeed=true;
            }
            else if (method == "DELETE")
            {
                method_ = DELETE;succeed=true;
            }
            else if(method == "TRACE")
            {
                method_ = TRACE;succeed=true;
            }
            else if(method == "OPTIONS")
            {
                method_=OPTIONS;succeed=true;
            }
            else if(method == "CONNET")
            {
                method_=CONNECT;succeed=true;
            }
            else
            {
                method_=PATH;succeed=true;
            }
            return succeed;
        }

        Method method() const
        { return method_; }

        std::string methodString() const
        {
            std::string result = "UNKNOWN";
            switch(method_)
            {
                case GET:
                    result = "GET";
                    break;
                case POST:
                    result = "POST";
                    break;
                case HEAD:
                    result = "HEAD";
                    break;
                case PUT:
                    result = "PUT";
                    break;
                case DELETE:
                    result = "DELETE";
                    break;
                case TRACE:
                    result = "TRACE";
                    break;
                case OPTIONS:
                    result = "OPTIONS";
                    break;
                case CONNECT:
                    result = "CONNECT";
                    break;
                case PATH:
                    result = "PATH";
                    break;    
                default:
                    break;
            }
            return result;
        }

        void setPath(const char* start, const char* end)
        {
            path_.assign(start, end);
        }

        const std::string& path() const
        { return path_; }

        void setQuery(const char* start, const char* end)
        {
            query_.assign(start, end);
        }

        const std::string& query() const
        { return query_; }


        void addHeader(const char* start, const char* colon, const char* end)
        {
            std::string field(start, colon);
            ++colon;
            while (colon < end && isspace(*colon))
            {
            ++colon;
            }
            std::string value(colon, end);
            while (!value.empty() && isspace(value[value.size()-1]))
            {
            value.resize(value.size()-1);
            }
            headers_[field] = value;
        }
        void appendBody(std::string &s){ body_+=s;}
        int bodyLength() const{return body_.size();}
        std::string getHeader(const std::string& field) const
        {
            std::string result;
            std::map<std::string, std::string>::const_iterator it = headers_.find(field);
            if (it != headers_.end())
            {
                result = it->second;
            }
            return result;
        }
        const std::string& getBody()const{ return body_;}
        const std::map<std::string, std::string>& headers() const
        { return headers_; }
        void swap(HttpRequest& that)
        {
            std::swap(method_, that.method_);
            std::swap(version_, that.version_);
            path_.swap(that.path_);
            query_.swap(that.query_);
            headers_.swap(that.headers_);
            body_.swap(that.body_);
        }
    private:
        std::map<std::string,std::string>  headers_;
        std::string path_;
        std::string query_;
        std::string body_;
        Method method_;
        Version version_;
};

#endif