#ifndef CHANNEL_H
#define CHANNEL_H
#include<sys/epoll.h>
#include<functional>
class EventLoop;
class Channel{
    public:
        typedef std::function<void()> ChannelFunc;
        const int readEvent=EPOLLIN|EPOLLPRI;
        const int writeEvent=EPOLLOUT;
        const int ET=EPOLLET;
        Channel(EventLoop*,int);
        ~Channel();
        void setHandleRead(ChannelFunc func){handleRead=std::move(func);}
        void setHandleWrite(ChannelFunc func){handleWrite=std::move(func);}
        void setHandleClose(ChannelFunc func){handleClose=std::move(func);}
        void handleEvent();
        void update();
        void enableReading(){events|=readEvent;update();}
        void enableWriting(){events|=writeEvent;update();}
        void enableET(){events|=ET;update();}
        void disableWriting(){events&=~writeEvent;update();}
        void disableReading(){events&=~readEvent;update();}
        void diableET(){events&=~ET;update();}
        void disableAll(){index_=2;update();}
        int getFd(){return sockfd_;}
        int getIndex(){return index_;}
        int getEvent(){return events;}
        void setREvent(int event){revents=event;}
        void setIndex(int index){index_=index;}
        EventLoop* getLoop(){return loop_;}
    private:
        EventLoop*loop_;
        int sockfd_;
        int events;
        int revents;
        int index_;//表示channel状态，KNew(-1),kAdded(1),kDeleted(2)
        ChannelFunc handleRead;
        ChannelFunc handleWrite;
        ChannelFunc handleClose;
};

#endif
