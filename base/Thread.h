#ifndef THREAD_H
#define THREAD_H
#include "Condition.h"
#include<functional>
struct ThreadData;
extern __thread int thread_ID;
class Thread{
    public:
        typedef std::function<void ()> Func;
    private:
        bool _started;
        bool _joined;
        bool _detached;
        Mutex mutex;
        Condition _cond;
        int cond_num;
        pthread_t _threadID;
        int _tid;
        Func _func;
        ThreadData *data;
    public:
        Thread(Func func,bool detach=false);
        ~Thread();
        void start();
        void join();
        static void* startThread(void *arg);
        static int gettid();
};
struct ThreadData{
    typedef Thread::Func Func;
    ThreadData(Func func,Condition *cond,int *cond_num,int *tid);
    void run();
    Condition *_cond;
    int *cond_num_;
    Func _func;
    int *_tid;
};

#endif