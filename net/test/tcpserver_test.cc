#include "../TcpServer.h"
#include<iostream>
#include "base/Thread.h"
void messageCallback(TcpConnectionPtr con,Buffer*buf)
{
    //std::cout<<"messageCallback!"<<std::endl;
    {
        std::string buff("HTTP/1.1 200 OK\r\n"
                        "Date: Sat, 31 Dec 2005 23:59:59 GMT\r\n"
                        "Content-Type: text/html;charset=ISO-8859-1\r\n"
                        "Content-Length: 122\r\n"
                        "\r\n"
                        "<html><head><title>Wrox Homepage</title></head><body><!-- body goes here --></body></html>");
        std::cout<<"test4"<<std::endl;
        con->send(buff.c_str(),buff.size());
        std::string s(buf->beginRead(),buf->readableBytes());
        std::cout<<s;
        con->shutdownWrite();
    }
    //std::string s=buf->retrieveAll();
    //std::cout<<s<<std::endl;
}
int main()
{
    EventLoop *loop=new EventLoop;
    TcpServer server(loop,"",8000,0,2);
    server.setMessageCallback(std::bind(&messageCallback,_1,_2));
    server.start();
    loop->loop();
    server.join();
}
