#include "TimerQueue.h"
#include "EventLoop.h"
#include <sys/timerfd.h>
#include "base/Logger.h"
#include "Channel.h"
#include<unistd.h>
#include<string.h>
int createTimerfd()
{
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC,TFD_NONBLOCK | TFD_CLOEXEC);
    if (timerfd < 0)
    {
        LOG_ERROR("Failed in timerfd_create");
    }
    return timerfd;
}
void readTimerfd(int timerfd)
{
    uint64_t howmany;
    ssize_t n = read(timerfd, &howmany, sizeof howmany);
    if (n != sizeof howmany)
    {
        LOG_ERROR("TimerQueue::handleRead() reads %d bytes instead of 8",n);
    }
}
struct timespec howMuchTimeFromNow(Timestamp &when)
{
    int64_t microseconds = when.microSecondsSinceEpoch() - Timestamp::now().microSecondsSinceEpoch();
    if (microseconds < 100)
    {
        microseconds = 100;
    }
    struct timespec ts;
    ts.tv_sec = static_cast<time_t>(microseconds / Timestamp::kMicroSecondsPerSecond);
    ts.tv_nsec = static_cast<long>((microseconds % Timestamp::kMicroSecondsPerSecond) * 1000);
    return ts;
}
void resetTimerfd(int timerfd, Timestamp &expiration)
{
    // wake up loop by timerfd_settime()
    struct itimerspec newValue;
    struct itimerspec oldValue;
    memset(&newValue,0,sizeof newValue);
    memset(&oldValue,0,sizeof oldValue);
    newValue.it_value = howMuchTimeFromNow(expiration);
    int ret = timerfd_settime(timerfd, 0, &newValue, &oldValue);
    if (ret)
    {
        LOG_ERROR("timerfd_settime()!");
    }
}
bool operator<(const Entry&left,const Entry&right)
{
    return (left.first<right.first);
}
TimerQueue::TimerQueue(EventLoop*loop,int check_per_seconds):loop_(loop),
                                        timers_(),
                                        timer_fd(createTimerfd()),
                                        timerChannel(new Channel(loop,timer_fd)),
                                        check_per_seconds_(check_per_seconds)
{
    Timestamp when(Timestamp::now());
    when.addTime(check_per_seconds_*1000*1000);
    resetTimerfd(timer_fd,when);
    timerChannel->setHandleRead(std::bind(&TimerQueue::handleRead,this));
    timerChannel->enableReading();
}
TimerQueue::~TimerQueue()
{
    close(timer_fd);
    for(auto it=timers_.begin();it!=timers_.end();++it) delete it->second;
    LOG_INFO("~TimerQueue()!");
}
void TimerQueue::handleRead()
{
    Timestamp now=Timestamp::now();
    readTimerfd(timer_fd);
    while(!timers_.empty())
    {
        auto it=timers_.begin();
        if(it->first<=now)
        {
            Timer*timer=it->second;
            timer->run(shared_from_this());
            timers_.erase(it);      
        }
        else break;
    }
    now.addTime(check_per_seconds_*1000*1000);
    resetTimerfd(timer_fd,now);
}
void TimerQueue::addTimer(Timer*timer)
{
    loop_->queueInLoop(std::bind(&TimerQueue::addTimerInLoop,this,timer));
}
void TimerQueue::addTimerInLoop(Timer*timer)
{
    Timestamp when(timer->expiration());
    std::pair<Timestamp,Timer*> entry(when,timer);
    timers_.insert(entry);
    //if(when==timers_.begin()->first) resetTimerfd(timer_fd,when);
}
void TimerQueue::deleteTimer(Timestamp&t)
{
    if(timers_.count(t)){
        Timer*timer=timers_[t];
        delete timer;
        LOG_DEBUG("Timer Deleted!");
        timers_.erase(t);
    }
}