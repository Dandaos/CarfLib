#ifndef LOGFILE_H
#define LOGFILE_H
#include<stdio.h>
#include<sys/time.h>
#include<time.h>
#include<string>
#include "Mutex.h"
#include "BlockQueue.h"
#include "Thread.h"
#define BLCOCK_QUEUE_SIZE 50000
class LogFile{
    public:
        LogFile(std::string log_dir,bool asylog,off_t rollSize,int checkEveryN=1024);
        void append_file(const char*str,int len);
        void asyWrite();
        void rollFile();
        std::string getCurrentName(time_t*);
        ~LogFile();
        void join(){
            setOver(true);
            thread_.join();
        }
        void setOver(bool);
    private:
        off_t rollSize_;        //文件达到一定大小就回滚
        FILE* file_;
        off_t writenBytes;        //已经写入的字节数
        int checkEveryN_;      //每隔N次检查是否到下一天，顺便检查是否可以刷新缓冲区
        const static int kRollPerSeconds_ = 60*60*24;  //表示一天时长
        std::string basename;
        int currentDay;
        bool asylog_;
        Thread thread_;
        int count_;
        bool over;
        Mutex mutex;
        
};

#endif
