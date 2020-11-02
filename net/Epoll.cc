#include "Epoll.h"
#include "EventLoop.h"
#include "Channel.h"
#include<string.h>
#include<iostream>
#include<assert.h>
#include "base/Logger.h"
#include<error.h>
const int kNew = -1;
const int kAdded = 1;
const int kDeleted = 2;
Epoll::Epoll(EventLoop* eventloop):loop(eventloop),
                                    epfd(0),
                                    revents(16)
{
    epfd=epoll_create(10);
    assert(epfd!=-1);
}
int Epoll::epollWait(std::vector<Channel*>&activeChannels,int timeout){
    int n=epoll_wait(epfd,&*revents.begin(),revents.size(),timeout);
    if(n==0||n==-1) return n;
    for(int i=0;i<n;i++){
        Channel* channel=static_cast<Channel*>(revents[i].data.ptr);
        channel->setREvent(static_cast<int>(revents[i].events));
        activeChannels.push_back(channel);
    }
    if(n==revents.size()) revents.resize(2*revents.size());
    return n;
}
void Epoll::updateChannel(Channel*channel){
    int fd=channel->getFd();
    int index=channel->getIndex();
    struct epoll_event event;
    memset(&event,0,sizeof event);
    event.events=static_cast<uint32_t>(channel->getEvent());
    event.data.ptr=static_cast<void*>(channel);
    if(channels.count(fd)==1){
        channels[fd]=channel;
        if(index==kAdded){
            epoll_ctl(epfd,EPOLL_CTL_MOD,fd,&event);
        }
        else if(index==kDeleted){
            epoll_ctl(epfd,EPOLL_CTL_DEL,fd,&event);
            channels.erase(fd);
        }
    }
    else{
        channel->setIndex(kAdded);
        channels[fd]=channel;
        int n=epoll_ctl(epfd,EPOLL_CTL_ADD,fd,&event);
        if(n==-1){
            LOG_ERROR("epoll_ctl[%d] error:%s",fd,strerror(errno));
        }
    }
}