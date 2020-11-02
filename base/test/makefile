CC=g++
Root=/home/luyuda/WebServer/base
CFLAGS=-std=c++11 -l pthread -g#-L $(Root)
obj=LogFile.o Logger.o Thread.o test_for_asylog.o
test_for_asylog:$(obj)
	$(CC) -o $@ $^ $(CFLAGS)
test_for_asylog.o:test_for_asylog.cc
	$(CC) -c $^ $(CFLAGS)
LogFile.o:../LogFile.cc
	$(CC) -c $^ $(CFLAGS)
Logger.o:../Logger.cc
	$(CC) -c $^ $(CFLAGS)
Thread.o:../Thread.cc
	$(CC) -c $^ $(CFLAGS)

clean:
	rm *.o *.log  ../*.gch core