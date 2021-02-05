#include "http/HttpServer.h"
#include<map>
#include<iostream>
#include "base/LogFile.h"
int main()
{
    EventLoop loop;
    HttpServer server_(&loop,"",8000,5,1,5,2);
    LogFile *log_=new LogFile("",true,200*1024*1024);
    Logger::setOuputFunc(std::bind(&LogFile::append_file,log_,_1,_2));
    //server_.setHttpCallback(onRequest);
    server_.sql_pool_init("localhost","root","lyd110","myDB",3306,8,1);
    server_.start();
    loop.loop();
}