#ifndef SOCKETOPS_H
#define SOCKETOPS_H
#include<netinet/in.h>
#include<arpa/inet.h>   //inet_pton,inet_ntop
#include<sys/socket.h>
#include<fcntl.h>
#include<unistd.h>
#include<string.h>
#include<string>
int createNonBlockSocket();
void setNonBlock(int fd);
void setCloseOnExec(int fd);
void setReuseAddr(int fd);
void setReusePort(int fd);
void fillSockaddr_in(std::string&ip,int port,struct sockaddr_in&addr);

#endif