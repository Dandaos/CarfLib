#include "Channel.h"
#include "EventLoop.h"
#include "base/Logger.h"
Channel::Channel(EventLoop*loop,int sockfd):loop_(loop),
                                            sockfd_(sockfd),
                                            events(0),
                                            revents(0),
                                            index_(-1)
{

}
void Channel::handleEvent(){
    if(revents&readEvent){
        if(handleRead) handleRead();
    }
    else if(revents&writeEvent){
        if(handleWrite) handleWrite();
    }
}
void Channel::update()
{
    loop_->updateChannel(this);
}
Channel::~Channel()
{
    LOG_DEBUG("Channel[%d] destroyed!",sockfd_);
}
