#include "socketops.h"
int createNonBlockSocket()
{
    int fd=socket(AF_INET,SOCK_STREAM,0);
    setNonBlock(fd);
    setCloseOnExec(fd);
    return fd;
}
void setNonBlock(int fd)
{
    int flags=fcntl(fd,F_GETFL,0);
    flags|=O_NONBLOCK;
    int ret=fcntl(fd,F_SETFL,flags);
}
void setCloseOnExec(int fd)
{
    int flags=fcntl(fd,F_GETFD);
    flags|=FD_CLOEXEC;
    int ret=fcntl(fd,F_SETFD,flags);
}
void setReuseAddr(int fd)
{
    int optval=1;
    setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&optval,sizeof optval);
}
void setReusePort(int fd)
{
    int optval=1;
    setsockopt(fd,SOL_SOCKET,SO_REUSEPORT,&optval,sizeof optval);
}
void fillSockaddr_in(std::string&ip,int port,struct sockaddr_in&addr)
{
    memset(&addr,0,sizeof addr);
    addr.sin_family=AF_INET;
    addr.sin_port=htons(static_cast<uint16_t>(port));
    if(!ip.empty()) inet_pton(AF_INET,ip.c_str(),&addr.sin_addr);
    else addr.sin_addr.s_addr=htonl(INADDR_ANY);
}