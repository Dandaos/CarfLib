#include "EventLoopThreadPool.h"
#include "base/Thread.h"
#include "EventLoop.h"
#include "Channel.h"
EventLoopThreadPool::EventLoopThreadPool(EventLoop*loop,int num):loop_(loop),
                                                                num_(num),
                                                                loops_(num),
                                                                currentLoop(-1),
                                                                threads_(num),
                                                                mutex_(),
                                                                cond_(0,mutex_),
                                                                index_(0)
{
    for(int i=0;i<num_;i++){
       threads_[i]=new Thread(std::bind(&EventLoopThreadPool::threadFunc,this));
       loops_[i]=NULL;
    }
}
EventLoopThreadPool::~EventLoopThreadPool()
{
    for(int i=0;i<threads_.size();i++){
        delete threads_[i];
    }
}
EventLoop* EventLoopThreadPool::getNextLoop()
{
    if(num_==0) return loop_;
    currentLoop=(currentLoop+1)%num_;
    return loops_[currentLoop];
}
void EventLoopThreadPool::start()
{
    for(int i=0;i<num_;i++){
        threads_[i]->start();
        while(loops_[i]==NULL) cond_.wait();
    }
}      
void EventLoopThreadPool::join()
{
    for(int i=0;i<num_;i++){
        threads_[i]->join();
    }
}
void EventLoopThreadPool::threadFunc()
{
    EventLoop loop;
    loops_[index_++]=&loop;
    cond_.wakeUpOne();
    loop.loop();

}