#ifndef CONDITION_H
#define  CONDITION_H
#include "Mutex.h"
#include<iostream>
class Condition{
    private:
        int count;
        Mutex &_mutex;
        pthread_cond_t _cond;
    public:
        Condition(int num,Mutex& mutex):_mutex(mutex){
            count=num;
            pthread_cond_init(&_cond,NULL);
        }
        ~Condition(){
            pthread_cond_destroy(&_cond);
        }
        void wait(){
            //std::cout<<"test2:"<<count<<std::endl;
            while(count<=0){
                pthread_cond_wait(&_cond,_mutex.getMutex());
            }
            //std::cout<<"test1:"<<count<<std::endl;
            count--;
        }
        void wakeUpOne(){
            count++;
            pthread_cond_signal(&_cond);
        }
        void wakeUpAll(){
            count++;
            pthread_cond_broadcast(&_cond);
        }
        pthread_mutex_t *  getMutex(){
            return _mutex.getMutex();
        }
        int getCount(){
            return count;
        }
};

#endif