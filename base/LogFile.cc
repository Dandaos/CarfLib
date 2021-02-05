#include "LogFile.h"
#include<unistd.h>
#include<iostream>
#include<string.h>
#include "Logger.h"
BlockQueue<std::string> g_bqueue(BLCOCK_QUEUE_SIZE);

LogFile::LogFile(std::string log_dir,bool asylog, off_t rollSize,int checkEveryN)
                                    :rollSize_(rollSize),
                                    asylog_(asylog),
                                    checkEveryN_(checkEveryN),
                                    writenBytes(0),
                                    file_(NULL),
                                    count_(0),
                                    thread_(std::bind(&LogFile::asyWrite,this)),
                                    over(false),
                                    mutex()
{
    if(log_dir.empty()){
        char path[128];
        getcwd(path,sizeof path);
        strcat(path,"/logdir");
        basename=std::string(path);
    }
    else{
        basename=log_dir+"/";
    }
    rollFile();
    if(asylog_) thread_.start();
}
void LogFile::append_file(const char*str,int len)
{   
    if(asylog_){    //放入阻塞队列
        std::string s(str,len);
        g_bqueue.push_back(s);
    }
    else{   //不是异步，直接输出
        MutexRAII guard(mutex);
        count_++;
        size_t n=fwrite(str,1,len,file_);
        assert(n==len);
        writenBytes+=n;
        if(writenBytes>=rollSize_) rollFile();
        else if(count_>=checkEveryN_){
            time_t now=time(NULL);
            time_t day=now/kRollPerSeconds_;
            if(day!=currentDay){
                rollFile();
            }
        }
    }
    
}
void LogFile::asyWrite(){
    while(!over){
        std::string s=g_bqueue.pop_front();
        size_t n=fwrite(s.c_str(),1,s.size(),file_);
        assert(n==s.size());
        writenBytes+=n;
        if(writenBytes>=rollSize_){
            rollFile();
        }
        else if(count_>=checkEveryN_){
            time_t now=time(NULL);
            time_t day=now/kRollPerSeconds_;
            if(day!=currentDay){
                rollFile();
            }
        }
        fflush(file_);
    }
}
std::string LogFile::getCurrentName(time_t *now)
{
    *now=time(NULL);
    char file_name[32]={0};
    struct tm my_tm=*(localtime(now));
    snprintf(file_name,32,"%4d%02d%02d %02d:%02d:%02d.log",my_tm.tm_year+1900, my_tm.tm_mon + 1, my_tm.tm_mday,\
                     my_tm.tm_hour, my_tm.tm_min, my_tm.tm_sec);
    return std::string(file_name);
}
void LogFile::rollFile()
{
    if(file_) fclose(file_);
    time_t now=0;
    std::string file_suffix=getCurrentName(&now);
    currentDay=now/kRollPerSeconds_;
    count_=0;
    writenBytes=0;
    std::string filename=basename+"/"+file_suffix;
    if((file_=fopen(filename.c_str(),"a+"))==NULL){
        LOG_ERROR("Can not open file %s",filename.c_str());
        exit(0);
    }
}
void LogFile::setOver(bool flag){
    over=flag;
    std::string s;
    g_bqueue.push_back(s);
}
LogFile::~LogFile(){
    fclose(file_);
}
