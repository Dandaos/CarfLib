#ifndef HTTPSERVER_H
#define HTTPSERVER_H
#include "net/TcpServer.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
class HttpServer
{
    public:
        typedef std::function<void (const HttpRequest&,HttpResponse*)> HttpCallback;
        HttpServer(EventLoop*loop,std::string ip,int port,int pollSize,int mode)
                :server_(loop,ip,port,pollSize,mode)
        {
            server_.setConnectionCallback(std::bind(&HttpServer::onConnection,this,_1));
            server_.setMessageCallback(std::bind(&HttpServer::onMessage,this,_1,_2));
        }
        void setHttpCallback(HttpCallback callback){callback_=std::move(callback);}
        void start(){server_.start();}
        void join(){server_.join();}

    private:
        TcpServer server_;
        void onConnection(TcpConnectionPtr& conn);
        void onMessage(TcpConnectionPtr conn,Buffer*buf);
        void onRequest(const TcpConnectionPtr& conn, const HttpRequest& req);
        HttpCallback callback_;

};

#endif