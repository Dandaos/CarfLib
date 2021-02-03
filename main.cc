#include "http/HttpServer.h"
#include<map>
#include<iostream>
int main()
{
    EventLoop loop;
    HttpServer server_(&loop,"",8000,0,2);
    //server_.setHttpCallback(onRequest);
    server_.sql_pool_init("localhost","root","lyd110","myDB",3306,8,1);
    server_.start();
    loop.loop();
}