#include "EventLoop.h"
#include "Channel.h"
#include "Epoll.h"
#include "../base/Logger.h"
#include<iostream>
#include<sys/eventfd.h>
#include "base/Logger.h"
#include<unistd.h>
__thread EventLoop*g_loop=NULL;
const int kPollTimeMs = 10000;
EventLoop::EventLoop():_loop(false),
                        _quit(false),
                        epoll(new Epoll(this)),
                        wakeuped(false)
{
    wakeupFd=eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    wakeupChannel_=std::unique_ptr<Channel>(new Channel(this,wakeupFd));
    wakeupChannel_->setHandleRead(std::bind(&EventLoop::handleRead,this));
    wakeupChannel_->enableReading();
    g_loop=this;
}
void EventLoop::loop(){
    assert(g_loop==this);
    LOG_INFO("Loop begins!");
    _loop=true;
    while(!_quit){
        activeChannels.clear();
        int n=epoll->epollWait(activeChannels,kPollTimeMs);
        wakeuped=false;
        if(n==-1) _quit=true;
        for(int i=0;i<n;i++){
            activeChannels[i]->handleEvent();
        }
        std::vector<Task> t_task;
        {
            MutexRAII guard(mutex_);
            tasks_.swap(t_task);
        }
        for(auto&func:t_task){
            func();
        }
    }
}
void EventLoop::updateChannel(Channel*channel)
{
    epoll->updateChannel(channel);
}
void EventLoop::queueInLoop(Task task)
{
    {
        MutexRAII guard(mutex_);
        tasks_.push_back(std::move(task));
    }
    //if(!wakeuped) {wakeup();wakeuped=true;}//这样会导致速度变得很慢，Why?
    wakeup();
}
EventLoop::~EventLoop()
{

}
void EventLoop::wakeup()
{
  uint64_t one = 1;
  ssize_t n = write(wakeupFd, &one, sizeof one);
  if (n != sizeof one)
  {
    LOG_ERROR("EventLoop::wakeup() writes %d bytes instead of 8",n);
  }
}
void EventLoop::handleRead()
{
  uint64_t one = 1;
  ssize_t n = read(wakeupFd, &one, sizeof one);
  if (n != sizeof one)
  {
    LOG_ERROR("EventLoop::wakeup() reads %d bytes instead of 8",n);
  }
}
