#ifndef TCPSERVER_H
#define TCPSERVER_H
#include "socketops.h"
#include<map>
#include "TcpConnection.h"
#include "base/Logger.h"
#include "EventLoop.h"
#include "EventLoopThreadPool.h"
#include "Channel.h"
#include "base/Mutex.h"
class TimerQueue;
class TcpServer{
    public:
        TcpServer(EventLoop*loop,std::string ip,int port,int pollSize,
        int mode,int timer_seconds,int timer_check_per_seconds);
        ~TcpServer();
        void start();
        void newConnection();
        std::string ipPortToString(const struct sockaddr_in*addr);
        void setMessageCallback(MessageCallback func){messageCallback=std::move(func);}
        void setConnectionCallback(ConnectionCallback func){connectionCallback=std::move(func);}
        void removeConnection(TcpConnectionPtr&);
        void join();
        void removeConnectionInLoop(TcpConnectionPtr&conn);
        void set_conn_hold_seconds(int seconds){conn_hold_seconds=seconds;}
    private:
        Mutex mutex_;
        int sockfd_;
        int nextConnId_;
        Channel *channel_;
        int mode_;
        EventLoopThreadPool *pool;
        std::string ipName;
        std::map<int,TcpConnectionPtr> conn_;
        MessageCallback messageCallback;
        ConnectionCallback connectionCallback;
        std::shared_ptr<TimerQueue> timer_queue;
        double conn_hold_seconds;
        int timer_check_per_seconds_;
        int idlefd;

};
#endif