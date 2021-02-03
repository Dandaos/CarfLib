#include "HttpContext.h"
#include<algorithm>
#include "base/Logger.h"
#include<assert.h>
bool HttpContext::processRequestLine(const char* begin, const char* end)
{
  bool succeed = false;
  const char* start = begin;
  const char* space = std::find(start, end, ' ');
  if (space != end && request_.setMethod(start, space))
  {
    start = space+1;
    space = std::find(start, end, ' ');
    if (space != end)
    {
      const char* question = std::find(start, space, '?');
      if (question != space)
      {
        request_.setPath(start, question);
        request_.setQuery(question, space);
      }
      else
      {
        request_.setPath(start, space);
      }
      start = space+1;
      succeed = end-start == 8 && std::equal(start, end-1, "HTTP/1.");
      if (succeed)
      {
        if (*(end-1) == '1')
        {
          request_.setVersion(HttpRequest::kHttp11);
        }
        else if (*(end-1) == '0')
        {
          request_.setVersion(HttpRequest::kHttp10);
        }
        else
        {
          succeed = false;
        }
      }
    }
  }
  return succeed;
}
bool HttpContext::parseRequest(Buffer* buf)
{
  bool ok = true;
  bool hasMore = true;
  std::string req(buf->beginRead(),buf->readableBytes());
  std::cout<<req<<std::endl;
  while (hasMore)
  {
    if (parseState_ == kExpectRequestLine)
    {
      const char* crlf = buf->findCRLF();
      if (crlf)
      {
        ok = processRequestLine(buf->beginRead(), crlf);
        if (ok)
        {
          buf->retrieveLen(crlf+2-buf->beginRead());
          parseState_ = kExpectHeaders;
        }
        else
        {
          hasMore = false;
        }
      }
      else
      {
        hasMore = false;
      }
    }
    else if (parseState_ == kExpectHeaders)
    {
      const char* crlf = buf->findCRLF();
      if (crlf)
      {
        const char* colon = std::find((const char*)buf->beginRead(), crlf, ':');
        if (colon != crlf)
        {
          request_.addHeader(buf->beginRead(), colon, crlf);
        }
        else
        {
          // empty line, end of header
          if(request_.method() == HttpRequest::POST) parseState_ = kExpectBody;
          else parseState_=kGotAll;
          //hasMore = false;
        }
        buf->retrieveLen(crlf + 2-buf->beginRead());
      }
      else
      {
        hasMore = false;
      }
    }
    else if (parseState_ == kExpectBody)
    {
      // FIXME:
      int content_lenght=std::stoi(request_.getHeader("Content-Length"));
      if(content_lenght==0) parseState_=kGotAll;
      else if(!buf->empty()){
        std::string body;
        const char *crlf=buf->findCRLF();
        if(crlf){
          buf->retrieveLen(crlf+ 2 - buf->beginRead());
        }
        body.assign(buf->beginRead(),buf->readableBytes());
        request_.appendBody(body);
        buf->retrieveAll();
        if(request_.bodyLength()==content_lenght) parseState_=kGotAll;
      }
      else hasMore=false;
    }
    else hasMore=false;
  }
  return ok;
}