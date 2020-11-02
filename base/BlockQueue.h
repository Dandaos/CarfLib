#ifndef BLOCKQUEUE_H
#define BLOCKQUEUE_H
#include<deque>
#include "Condition.h"
#include<iostream>
template<class T>
class BlockQueue{
    public:
        BlockQueue(int maxSize):mutex(),
                                maxSize_(maxSize),
                                waitForEmpty(0,mutex),
                                waitForFull(maxSize,mutex)
        {
        }
        ~BlockQueue()=default;
        void push_back(T&item){
            MutexRAII guard(mutex);
            waitForFull.wait();
            queue_.push_back(item);
            waitForEmpty.wakeUpOne();
        }
        
        T pop_front(){
            MutexRAII guard(mutex);
            waitForEmpty.wait();
            T t=queue_.front();
            queue_.pop_front();
            waitForFull.wakeUpOne();
            return t;
        }
    private:
        std::deque<T> queue_;
        Condition waitForEmpty;
        Condition waitForFull;
        Mutex mutex;
        int maxSize_;
};


#endif