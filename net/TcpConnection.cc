#include "TcpConnection.h"
#include<unistd.h>
#include "base/Logger.h"
#include<sys/socket.h>
#include "TimerQueue.h"
#include<limits>
const int initBufSize=1024;
void defaultMessageCallback(TcpConnectionPtr conn,Buffer*buf){}
MessageCallback defaultCallback=std::bind(&defaultMessageCallback,_1,_2);
TcpConnection::TcpConnection(EventLoop*loop,int fd,int mode,std::string &peername,int connID,std::shared_ptr<Timer> timer):channel_(new Channel(loop,fd)),
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
    LOG_INFO("TcpConnection[%d] destroyed!",fd_);
    close(fd_);
    delete channel_;
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
            LOG_ERROR("TcpConnection::handleWrite Error[%s]!",strerror(errno));
            //channel_->disableWriting();
        }
    }
    else{
        int bytes_to_send=writeBuffer.readableBytes();
        while(1){
            int n=write(channel_->getFd(),writeBuffer.beginRead(),writeBuffer.readableBytes());
            if(n==-1){
                if(errno==EAGAIN){
                    channel_->enableWriting();
                    break;
                }
            }
            bytes_to_send-=n;
            writeBuffer.addReadIndex(n);
            if(bytes_to_send==0) {
                break;
            }
            else{
                channel_->enableWriting();
            }
        }
    }
}
// 
void TcpConnection::handleClose()
{
    //EventLoop*loop=channel_->getLoop();
    channel_->disableAll();
    TcpConnectionPtr guardThis(shared_from_this());
    closeCallback_(guardThis);      //回调將TcpServer中的TcpConnection异步删除
    //loop->queueInLoop(std::bind(&TcpConnection::connDestroyed,guardThis));
    state_=kdisconnected;
}
void TcpConnection::stopTimer()
{
    timer_=std::shared_ptr<Timer>();
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
            LOG_ERROR("TcpConnection::send() Error[%d]!",strerror(errno));
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
void TcpConnection::handleTimer(std::shared_ptr<TimerQueue> timerqueue)
{
    Timestamp now(Timestamp::now());
    double remain=now.microSecondsSinceEpoch()-visited.microSecondsSinceEpoch();
    if(timer_.use_count()>0){
        //在规定期限内再次访问或者写缓冲依旧有数据未发送时，更新定时器
        if(remain<timer_->getInterval()||writeBuffer.readableBytes()>0){
            Timestamp t=visited;
            t.addTime(timer_->getInterval());
            timer_->restart(t);
            timerqueue->addTimer(timer_);
            LOG_INFO("TcpConnection[%d]'s timer restarts!",connID_);
        }
        else{
            shutdownWrite();
            LOG_INFO("Timer is expired, shutdownWrite!");
        }
    }
}
                                                    