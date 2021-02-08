#ifndef CONDITION_H
#define  CONDITION_H
#include "Mutex.h"
#include<iostream>
class Condition{
    private:
        Mutex &_mutex;
        pthread_cond_t _cond;
    public:
        Condition(Mutex& mutex):_mutex(mutex){
            pthread_cond_init(&_cond,NULL);
        }
        ~Condition(){
            pthread_cond_destroy(&_cond);
        }
        void wait(){
            pthread_cond_wait(&_cond,_mutex.getMutex());
        }
        void wakeUpOne(){
            pthread_cond_signal(&_cond);
        }
        void wakeUpAll(){
            pthread_cond_broadcast(&_cond);
        }
        pthread_mutex_t *  getMutex(){
            return _mutex.getMutex();
        }
};

#endif