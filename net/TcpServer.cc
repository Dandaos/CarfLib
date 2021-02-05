#include "TcpServer.h"
#include<assert.h>
#include<iostream>
#include "TimerQueue.h"
TcpServer::TcpServer(EventLoop*loop,std::string ip,int port,int poolSize,
            int mode,int timer_hold_seconds,int timer_check_per_seconds):mode_(mode),
                                            ipName(ip),
                                            pool(new EventLoopThreadPool(loop,poolSize)),
                                            nextConnId_(0),
                                            mutex_(),
                                            conn_hold_seconds(timer_hold_seconds),
                                        timer_check_per_seconds_(timer_check_per_seconds),
                                        //timer_queue(new TimerQueue(loop,timer_check_per_seconds_)) 这样会出BUG
                                        //因为timer_check_per_seconds_定义在后，此时还未初始化
                                        timer_queue(new TimerQueue(loop,timer_check_per_seconds))
{
    sockfd_=socket(AF_INET,SOCK_STREAM,0);
    setReusePort(sockfd_);
    setReuseAddr(sockfd_);
    channel_=new Channel(loop,sockfd_);
    channel_->setHandleRead(std::bind(&TcpServer::newConnection,this));
    //监听套接字采用水平触发(默认)
    struct sockaddr_in addr;
    memset(&addr,0,sizeof addr);
    addr.sin_family=AF_INET;
    addr.sin_port=htons(static_cast<uint16_t>(port));
    //如果ip为空，则INADDR_ANY
    if(!ip.empty()){
        int n=inet_pton(AF_INET,ip.c_str(),&(addr.sin_addr));
        assert(n==1);
    }
    else{
        addr.sin_addr.s_addr=htonl(INADDR_ANY);
    }
    int n=bind(sockfd_,(struct sockaddr*)&addr,static_cast<socklen_t>(sizeof(sockaddr_in)));
    if(n==-1){
        LOG_ERROR("Bind Error:%s",strerror(errno));
        assert(n==0);
    }
}
std::string TcpServer::ipPortToString(const struct sockaddr_in*addr)
{
    char s[32];
    inet_ntop(AF_INET,&addr->sin_addr,s,sizeof s);
    std::string str(s,strlen(s));
    str+=":"+std::to_string(ntohs(addr->sin_port));
    return str;
}
void TcpServer::start()
{
    pool->start();
    int n=listen(sockfd_,128);
    channel_->enableReading();
    assert(n==0);
}
void TcpServer::join()
{
    pool->join();
}
void TcpServer::newConnection()
{
    struct sockaddr_in addr;
    memset(&addr,0,sizeof addr);
    socklen_t len=0;
    int clt_fd=accept(sockfd_,(struct sockaddr*)&addr,&len);
    setNonBlock(clt_fd);
    setCloseOnExec(clt_fd);
    if(clt_fd>=0){
        std::string ipPort=ipPortToString(&addr);
        LOG_INFO("Receive new connection[%d] from %s",nextConnId_,ipPort.c_str());
        EventLoop *loop=pool->getNextLoop();
        assert(loop!=NULL);
        Timer*timer=NULL;
        if(conn_hold_seconds>0){
            Timestamp when(Timestamp::now());
            when.addTime(conn_hold_seconds*1000*1000);
            timer=new Timer(when,conn_hold_seconds*1000*1000);
        }
        TcpConnectionPtr conn(new TcpConnection(loop,clt_fd,mode_,ipPort,nextConnId_,timer));
        conn_[nextConnId_]=conn;
        nextConnId_++;
        conn->setReadCallback(messageCallback);
        conn->setCloseCallback(std::bind(&TcpServer::removeConnection,this,_1));
        if(connectionCallback) connectionCallback(conn);
        loop->queueInLoop(std::bind(&TcpConnection::connectEstablished,conn));
        //一定要等TcpConnction构造成功才enableReading，否则很容易抛出std::weak_ptr错误
        //如果在此线程enableReading，可能会在loop对应Epoll处发生竞态
        if(conn_hold_seconds>0){
            timer->setTimerFunc(std::bind(&TcpConnection::handleTimer,conn,_1));
            timer_queue->addTimer(timer);
        }
    }
    else{
        LOG_ERROR("Accept Error!");
    }
}
void TcpServer::removeConnection(TcpConnectionPtr &conn)
{   
    // 所有I/O线程异步提交删除连接请求，所有删除操作均在主线程完成
    channel_->getLoop()->queueInLoop(std::bind(&TcpServer::removeConnectionInLoop,this,conn));
}
void TcpServer::removeConnectionInLoop(TcpConnectionPtr&conn)
{
    int connID=conn->getID();
    std::string name=conn->getName();
    if(conn_hold_seconds>0){
        Timestamp t=conn->getTimer()->expiration();
        timer_queue->deleteTimer(t);
    }
    conn_.erase(connID);
    LOG_INFO("Remove connection[%d] from %s",connID,name.c_str());
}
TcpServer::~TcpServer()
{
    assert(conn_.size()==0);
    delete pool;
}