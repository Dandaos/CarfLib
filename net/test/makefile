CC=g++
RootPath=/home/luyuda/WebServer
CFLAGS=-std=c++11 -l pthread -g -I $(RootPath)/
obj=tcpserver_test.o LogFile.o Logger.o Thread.o Channel.o Epoll.o EventLoop.o EventLoopThreadPool.o\
           socketops.o TcpConnection.o TcpServer.o
tcpserver_test:$(obj)
	$(CC) -o $@ $^ $(CFLAGS)
tcpserver_test.o:tcpserver_test.cc
	$(CC) -c $^ $(CFLAGS)
LogFile.o:$(RootPath)/base/LogFile.cc
	$(CC) -c $^ $(CFLAGS)
Logger.o:$(RootPath)/base/Logger.cc
	$(CC) -c $^ $(CFLAGS)
Thread.o:$(RootPath)/base/Thread.cc
	$(CC) -c $^ $(CFLAGS)
Channel.o:$(RootPath)/net/Channel.cc
	$(CC) -c $^ $(CFLAGS)
Epoll.o:$(RootPath)/net/Epoll.cc
	$(CC) -c $^ $(CFLAGS)
EventLoop.o:$(RootPath)/net/EventLoop.cc
	$(CC) -c $^ $(CFLAGS)
EventLoopThreadPool.o:$(RootPath)/net/EventLoopThreadPool.cc
	$(CC) -c $^ $(CFLAGS)
socketops.o:$(RootPath)/net/socketops.cc
	$(CC) -c $^ $(CFLAGS)
TcpConnection.o:$(RootPath)/net/TcpConnection.cc $(RootPath)/net/Buffer.h
	$(CC) -c $^ $(CFLAGS)
TcpServer.o:$(RootPath)/net/TcpServer.cc
	$(CC) -c $^ $(CFLAGS)
clean:
	rm $(RootPath)/net/*.o $(RootPath)/base/*.o $(RootPath)/net/test/*.o $(RootPath)/net/*.gch
