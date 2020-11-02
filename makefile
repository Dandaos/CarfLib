CC=g++
RootPath=$(shell pwd)
CFLAGS=-std=c++11 -g
LIBS=-pthread
Include=-I $(RootPath)/
SOURCE =$(wildcard $(RootPath)/base/*.cc $(RootPath)/http/*.cc $(RootPath)/net/*.cc)
OBJS =$(patsubst %.cc,%.o,$(SOURCE))
httpserver:$(OBJS) main.o
	$(CC) $(CFLAGS) $(LIBS) $^ -o $@ $(Include)
$(OBJS):%.o:%.cc
	$(CC) $(CFLAGS) -c $(LIBS) $^ -o $@ $(Include)
main.o:main.cc
	$(CC) $(CFLAGS) -c $(LIBS) $^ -o $@ $(Include)
.PHONY:show clean
show:
	@echo "show objs"
	@echo $(OBJS)
clean:
	@rm -rf *.o httpserver
	@rm -rf $(RootPath)/base/*.o
	@rm -rf $(RootPath)/http/*.o
	@rm -rf $(RootPath)/net/*.o
