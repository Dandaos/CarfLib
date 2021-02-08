#include "Thread.h"
#include<utility>
#include<sys/syscall.h>
#include <sys/types.h>
#include <linux/unistd.h>
#include <unistd.h>
#include"Logger.h"
__thread int thread_ID=0;
Thread::Thread(Func func,bool detach):_func(std::move(func)),
                            _detached(detach),
                            _started(false),
                            _joined(false),
                            mutex(),
                            _cond(mutex),
                            cond_num(0),
                            _threadID(0),
                            _tid(0),
                            data(new ThreadData(_func,&_cond,&cond_num,&_tid))
{

}
Thread::~Thread(){
    delete data;
}
int Thread::gettid(){
    if(thread_ID==0){
        thread_ID=static_cast<int>(syscall(SYS_gettid));
    }
    return thread_ID;
}
void Thread::start(){
    assert(!_started);
    _started=true;
    pthread_create(&_threadID,NULL,Thread::startThread,data);
    if(_detached) pthread_detach(_threadID);
    MutexRAII guard(mutex);
    while(cond_num==0) _cond.wait();    //等待创建的线程运行
}
void Thread::join(){
    assert(_started);
    assert(!_joined);
    assert(!_detached);   //若线程未detach，必须join，否则会产生僵尸线程;一旦detach，也就不能join.
    pthread_join(_threadID,NULL);
    _joined=true;
}
void* Thread::startThread(void *arg)
{
    ThreadData *data=static_cast<ThreadData*>(arg);
    data->run();
}
ThreadData::ThreadData(Func func,Condition *cond,int *cond_num,int *tid):_func(std::move(func)),
                                                _cond(cond),
                                                cond_num_(cond_num),
                                                _tid(tid)
{

}
void ThreadData::run()
{
    ++(*cond_num_);
    _cond->wakeUpOne();
    *_tid=Thread::gettid();
    _func();            //此处异常日后处理
    LOG_DEBUG("Thread Exit!");
}
