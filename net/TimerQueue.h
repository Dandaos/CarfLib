#ifndef TIMER_QUEUE_H
#define TIMER_QUEUE_H
#include<functional>
#include "base/Timestamp.h"
#include<map>
#include<vector>
#include<queue>
#include<memory>
#include "./base/Logger.h"
class EventLoop;
class Channel;
class TimerQueue;
class TcpConnection;
class Timer
{
    public:
        typedef std::function<void(std::shared_ptr<TimerQueue> timerqueue)> TimerCallback;
        Timer(Timestamp when,double interval):expired(when),
                                            visited(false),
                                            interval_(interval)
        {

        }
        ~Timer(){
            //LOG_INFO("Timer destroyed!");
        }
        inline bool isVisited() const{return visited;}
        inline Timestamp expiration()const{return expired;}
        void run(std::shared_ptr<TimerQueue> timerqueue){
            std::shared_ptr<TcpConnection> tcp(tcpConn_.lock());
            if(tcp) callback_(timerqueue);
        }
        void setTimerFunc(TimerCallback cb){callback_=std::move(cb);}
        void restart(Timestamp expired_)
        {
            expired=expired_;
        }
        void setVisited(bool visited_){visited=visited_;}
        double getInterval()const{return interval_;}
        void bindTcpConnection(std::shared_ptr<TcpConnection> tcpConn){tcpConn_=tcpConn;}
    private:
        double interval_;
        bool visited;
        Timestamp expired;
        TimerCallback callback_;
        std::weak_ptr<TcpConnection> tcpConn_;

};
typedef std::map<Timestamp,std::shared_ptr<Timer>> BackupList;
typedef std::pair<Timestamp,std::shared_ptr<Timer>> Entry;
class TimerQueue:public std::enable_shared_from_this<TimerQueue>
{
    public:
        TimerQueue(EventLoop*loop,int check_per_seconds);
        ~TimerQueue();
        void addTimer(std::shared_ptr<Timer> timer);
        void addTimerInLoop(std::shared_ptr<Timer> timer);
        void handleRead();
        void deleteTimer(Timestamp &t);
    private:
        EventLoop *loop_;
        int timer_fd;
        Channel *timerChannel;
        BackupList timers_;
        int check_per_seconds_;
};
#endif