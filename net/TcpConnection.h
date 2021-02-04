#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H
#include<functional>
#include "Buffer.h"
#include "EventLoop.h"
#include "Channel.h"
#include<memory>
#include <boost/any.hpp>
#include "base/Timestamp.h"
class TcpConnection;
class TimerQueue;
using std::placeholders::_1;
using std::placeholders::_2;
typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
typedef std::function<void(TcpConnectionPtr,Buffer*)> MessageCallback;
typedef std::function<void(TcpConnectionPtr&)> CloseCallback;
typedef std::function<void(TcpConnectionPtr&)> ConnectionCallback;
class Timer;
class TcpConnection:public std::enable_shared_from_this<TcpConnection>
{
    public:
        enum State
        {
            kdisconnected=0,
            kconnected,
            kshutdown
        };
        TcpConnection(EventLoop*loop,int fd,int mode,std::string &peername,int connID,Timer*timer);
        ~TcpConnection();
        void handleRead();
        void handleClose();
        void handleWrite();
        void enableReading(){channel_->enableReading();}
        void enableWriting(){channel_->enableWriting();}
        void setReadCallback(MessageCallback func){readCallback_=std::move(func);}
        //void setWriteCallback(MessageCallback func){writeCallback_=std::move(func);}
        void setCloseCallback(CloseCallback func){closeCallback_=std::move(func);}
        void shutdownWrite();
        void send(const char*,int);
        void send(std::string s){send(s.c_str(),s.size());}
        std::string getName(){return peername_;}
        int getID(){return connID_;}
        void connDestroyed();
        EventLoop*getLoop(){return channel_->getLoop();}
        boost::any* getMutableContext(){ return &context; }
        void setContext(const boost::any& context_){ context = context_; }
        void connectEstablished();
        void handleTimer(std::shared_ptr<TimerQueue> timerqueue);
        Timer*getTimer() const{return timer_;}
    private:
        int connID_;
        int fd_;
        Buffer writeBuffer;
        Buffer readBuffer;
        std::string peername_;
        Channel *channel_;
        int mode_;                          //1表示LT,2表示ET
        MessageCallback readCallback_;
        CloseCallback closeCallback_;
        boost::any context;
        Timestamp visited;
        State state_;
        Timer *timer_;
        
};
#endif