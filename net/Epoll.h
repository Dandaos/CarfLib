#ifndef EPOLL_H
#define EPOLL_H
class EventLoop;
class Channel;
#include<vector>
#include<map>
#include "base/Mutex.h"
class Epoll{
    public:
        Epoll(EventLoop*);
        void updateChannel(Channel*);
        int  epollWait(std::vector<Channel*>&activeChannels,int timeout);

    private:
        EventLoop* loop;
        int epfd;
        std::map<int,Channel*> channels;
        std::vector<struct epoll_event> revents;


};


#endif