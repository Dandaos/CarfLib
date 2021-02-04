#ifndef HTTPSERVER_H
#define HTTPSERVER_H
#include "net/TcpServer.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "sql_connection_pool.h"
class HttpServer
{
    public:
        void defaultHttpCallback(const HttpRequest&,HttpResponse*);
        HttpServer(EventLoop*loop,std::string ip,int port,int pollSize,
        int mode,int timer_hold_seconds,int timer_check_per_seconds)
                :server_(loop,ip,port,pollSize,mode,timer_hold_seconds,timer_check_per_seconds),
                sql_pool(connection_pool::GetInstance())
        {
            server_.setConnectionCallback(std::bind(&HttpServer::onConnection,this,_1));
            server_.setMessageCallback(std::bind(&HttpServer::onMessage,this,_1,_2));
        }
        void sql_pool_init(string url, string User, string PassWord, string DBName, 
                int Port, int MaxConn, int close_log);
        void start(){server_.start();}
        void join(){server_.join();}

    private:
        connection_pool *sql_pool;
        TcpServer server_;
        void onConnection(TcpConnectionPtr& conn);
        void onMessage(TcpConnectionPtr conn,Buffer*buf);
        void onRequest(const TcpConnectionPtr& conn, const HttpRequest& req);
        std::map<std::string,std::string> users;
        Mutex lock;

};

#endif