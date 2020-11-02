#ifndef EVENTLOOP_H
#define EVENTLOOP_H
#include<vector>
#include<map>
#include<memory>
#include "base/Mutex.h"
class EventLoop;
extern  __thread EventLoop*g_loop;
class Channel;
class Epoll;
class EventLoop{
    public:
        typedef std::function<void()> Task;
        EventLoop();
        ~EventLoop();
        void loop();
        void updateChannel(Channel*channel);
        void queueInLoop(Task task);
        void wakeup();
        void handleRead();
    private:
        bool wakeuped;
        Mutex mutex_;
        std::unique_ptr<Channel> wakeupChannel_;
        int wakeupFd;
        std::vector<Channel*> activeChannels;
        std::unique_ptr<Epoll> epoll;
        bool _loop;
        bool _quit;
        std::vector<Task> tasks_;

};

#endif