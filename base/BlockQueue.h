#ifndef BLOCKQUEUE_H
#define BLOCKQUEUE_H
#include<deque>
#include "Condition.h"
#include<iostream>
template<class T>
class BlockQueue{
    public:
        BlockQueue(int maxSize):mutex(),
                                used(0),
                                capacity(maxSize),
                                waitForEmpty(mutex),
                                waitForFull(mutex)
        {

        }
        ~BlockQueue()=default;
        void push_back(T&item){
            MutexRAII guard(mutex);
            while(used==capacity){      //队列满时等待
                waitForEmpty.wait();
            }
            queue_.push_back(item);
            ++used;
            waitForFull.wakeUpOne();
        }
        
        T pop_front(){
            MutexRAII guard(mutex);
            while(used==0){    //队列空时等待
                waitForFull.wait();
            }
            T t=queue_.front();
            queue_.pop_front();
            --used;
            waitForEmpty.wakeUpOne();
            return t;
        }
    private:
        std::deque<T> queue_;
        Condition waitForEmpty;
        Condition waitForFull;
        Mutex mutex;
        int used;
        int capacity; 
};


#endif