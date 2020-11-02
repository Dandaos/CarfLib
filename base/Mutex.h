#ifndef MUTEX_H
#define MUTEX_H
#include<pthread.h>
#include<assert.h>
#include<iostream>
class Mutex{
    public:
        Mutex(){
            pthread_mutex_init(&mutex_,NULL);
        }
        ~Mutex(){
            pthread_mutex_destroy(&mutex_);
        }
        void lock(){
            pthread_mutex_lock(&mutex_);
        }
        void unlock(){
            pthread_mutex_unlock(&mutex_);
        }
        pthread_mutex_t *  getMutex(){
            return &mutex_;
        }
    private:
        pthread_mutex_t mutex_;
};
class MutexRAII{
    public:
        MutexRAII(Mutex &mutex):mutex_(mutex){
            mutex_.lock();
        }
        ~MutexRAII(){
            mutex_.unlock();
        }
    private:
        Mutex &mutex_;
};

#endif