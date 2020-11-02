#ifndef EVENTLOOPTHREADPOOL_H
#define EVENTLOOPTHREADPOOL_H
#include<vector>
class Thread;
class EventLoop;
class Channel;
#include "base/Mutex.h"
#include "base/Condition.h"
class EventLoopThreadPool{
    public:
        EventLoopThreadPool(EventLoop*loop,int num);
        ~EventLoopThreadPool();
        void start();
        void join();
        EventLoop *getNextLoop();
        void threadFunc();
    private:
        int index_;
        Mutex mutex_;
        Condition cond_;
        int num_;
        int currentLoop;
        EventLoop *loop_;
        std::vector<Thread*> threads_;
        std::vector<EventLoop*> loops_;
};

#endif