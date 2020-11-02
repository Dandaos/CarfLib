#ifndef HTTPCONTEXT_H
#define HTTPCONTEXT_H
#include "HttpRequest.h"
#include "net/Buffer.h"
class HttpContext
{ 
    public:
        enum HttpRequestParseState
        {
            kExpectRequestLine,
            kExpectHeaders,
            kExpectBody,
            kGotAll,
        };
    public:
        HttpContext():parseState_(kExpectRequestLine){}
        bool parseRequest(Buffer *buf);
        HttpRequest& request(){return  request_;}
        bool gotAll() const{ return parseState_ == kGotAll; }
        void reset()
        {
            parseState_ = kExpectRequestLine;
            HttpRequest dummy;
            request_.swap(dummy);
        }

    private:
        HttpRequest request_;
        HttpRequestParseState parseState_;
        bool processRequestLine(const char* begin, const char* end);
};

#endif