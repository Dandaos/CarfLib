#ifndef TIMER_H
#define TIMER_H
#include<functional>
#include "base/Timestamp.h"
#include<map>
#include<vector>
#include<queue>
class EventLoop;
class Channel;
class Timer
{
    public:
        typedef std::function<void()> TimerCallback;
        Timer(Timestamp when,double interval):expired(when),
                                            visited(false),
                                            interval_(interval)
        {

        }
        inline bool isVisited() const{return visited;}
        inline Timestamp expiration()const{return expired;}
        void run(){callback_();}
        void setTimerFunc(TimerCallback cb){callback_=std::move(cb);}
        void restart()
        {
            expired.addTime(interval_);
        }
        void setVisited(bool visited_){visited=visited_;}
        double getInterval()const{return interval_;}
    private:
        double interval_;
        bool visited;
        Timestamp expired;
        TimerCallback callback_;

};
typedef std::pair<Timestamp,Timer*> Entry;
typedef std::priority_queue<Entry> TimerList;
typedef std::map<Timestamp,Timer*> BackupList;
class TimerQueue
{
    public:
        TimerQueue(EventLoop*loop);
        ~TimerQueue();
        void addTimer(Timer*timer);
        void addTimerInLoop(Timer*timer);
        std::vector<Timer*> getExpired();
        void handleRead();
        void deleteTimer(Timestamp &t);
    private:
        EventLoop *loop_;
        int timer_fd;
        Channel *timerChannel;
        BackupList timers_;

};
#endif