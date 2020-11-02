#include "TcpConnection.h"
#include<unistd.h>
#include "base/Logger.h"
#include<sys/socket.h>
#include "TimerQueue.h"
#include<limits>
const int initBufSize=1024;
void defaultMessageCallback(TcpConnectionPtr conn,Buffer*buf){}
MessageCallback defaultCallback=std::bind(&defaultMessageCallback,_1,_2);
TcpConnection::TcpConnection(EventLoop*loop,int fd,int mode,std::string &peername,int connID,Timer*timer):channel_(new Channel(loop,fd)),
                                                    writeBuffer(initBufSize),
                                                    readBuffer(initBufSize),
                                                    readCallback_(defaultCallback),
                                                    mode_(mode),
                                                    peername_(peername),
                                                    connID_(connID),
                                                    context(NULL),
                                                    fd_(fd),
                                                    visited((std::numeric_limits<long>::max)()),
                                                    state_(kconnected),
                                                    timer_(timer)
{
    channel_->setHandleRead(std::bind(&TcpConnection::handleRead,this));
    channel_->setHandleWrite(std::bind(&TcpConnection::handleWrite,this));
}
TcpConnection::~TcpConnection()
{
    // 2020/10/16：修改前为close(channel_->getFd()),但此时channel_已经被析构了,程序会突然退出
    close(fd_);
}
void TcpConnection::handleRead()
{
    int fd=channel_->getFd();
    visited=Timestamp::now();
    if(mode_==1){
        ssize_t n=read(fd,readBuffer.beginWrite(),readBuffer.writableBytes());
        if(n==0){
            handleClose();
            return;
        }
        if(n==readBuffer.writableBytes()) readBuffer.resize();
        if(n!=-1) readBuffer.addWriteIndex(n);
    }
    else{
        while(1){
            ssize_t n=read(fd,readBuffer.beginWrite(),readBuffer.writableBytes());
            if(n==0){
                handleClose();
                return;
            }
            if(n==-1){
                if(errno == EAGAIN || errno == EWOULDBLOCK) break;      //?
            }
            if(n==readBuffer.writableBytes()) readBuffer.resize();
            readBuffer.addWriteIndex(n);
        }
    }
    readCallback_(shared_from_this(),&readBuffer);
}
void TcpConnection::handleWrite()
{
    if(mode_==1){
        int n=write(channel_->getFd(),writeBuffer.beginRead(),writeBuffer.readableBytes());
        if(n>0){
            if(n==writeBuffer.readableBytes()) channel_->disableWriting();
            writeBuffer.addReadIndex(n);
        }
        else{
            LOG_ERROR("TcpConnection::handleWrite Error!");
        }
    }
    else{
        int bytes_to_send=writeBuffer.readableBytes();
        while(1){
            int n=write(channel_->getFd(),writeBuffer.beginRead(),writeBuffer.readableBytes());
            if(n==-1){
                if(errno==EAGAIN) channel_->enableWriting();
            }
            bytes_to_send-=n;
            if(bytes_to_send==0) break;
        }
    }
}
// 
void TcpConnection::handleClose()
{
    EventLoop*loop=channel_->getLoop();
    channel_->disableAll();
    TcpConnectionPtr guardThis(shared_from_this());
    closeCallback_(guardThis);      //回调將TcpServer中的TcpConnection异步删除
    loop->queueInLoop(std::bind(&TcpConnection::connDestroyed,shared_from_this()));
}
void TcpConnection::connDestroyed()
{
    delete channel_;
}
void TcpConnection::connectEstablished()
{
    if(mode_==2) channel_->enableET();
    channel_->enableReading();
}
void TcpConnection::send(const char*s,int len)
{
    int nwrote=0,remaining=len;
    if(writeBuffer.readableBytes()==0){
        nwrote=write(channel_->getFd(),s,len);
        if(nwrote>0){
            remaining=len-nwrote;
        }
        else{
            LOG_ERROR("TcpConnection::send() Error!");
            nwrote=0;
        }
    }
    if(nwrote<len){
        writeBuffer.append(s+nwrote,len-nwrote);
        channel_->enableWriting();
    }
}
void TcpConnection::shutdownWrite()
{
    if(state_=kconnected){
        if(shutdown(channel_->getFd(),SHUT_WR)==-1){
            LOG_ERROR("TcpConnection::shutdownWrite error:%s",strerror(errno));
        }
        state_=kshutdown;
    }
}
void TcpConnection::handleTimer()
{
    double remain=timer_->expiration().microSecondsSinceEpoch()-visited.microSecondsSinceEpoch();
    if(remain<timer_->getInterval()) {timer_->setVisited(true);}
    else shutdownWrite();
}
                                                    