#include "Logger.h"
#include<stdarg.h>
#include<time.h>
#include<sys/time.h>
#include<iostream>
#include "Thread.h"
const char log_name[Logger::LOG_NUM][16]={
    {"LOG_DEBUG"},
    {"LOG_INFO"},
    {"LOG_WARN"},
    {"LOG_ERROR"}
};
bool log_t=true;
void defaultOutput(const char*,int);

Logger::LOG_LEVEL initLogLevel();
Logger::OutputFunc g_output=std::bind(&defaultOutput,_1,_2);
Logger::LOG_LEVEL g_logLevel=initLogLevel();
Logger::LOG_LEVEL initLogLevel(){
    return Logger::LOG_NUM;
}
void defaultOutput(const char *str,int len){
    fwrite(str,1,len,stdout);
}
void Logger::setOuputFunc(Logger::OutputFunc func){
    g_output=std::move(func);
}
void Logger::setLogLevel(Logger::LOG_LEVEL level){
    g_logLevel=level;
}
Logger::LOG_LEVEL Logger::getLogLevel(){
    return g_logLevel;
}
Logger::Logger(const char* file,int line,LOG_LEVEL level):write_length(0),_line(line),_level(level)
{
    strcpy(filename,file);
    struct timeval now = {0, 0};
    gettimeofday(&now, NULL);
    time_t t = now.tv_sec;
    struct tm my_tm = *(localtime(&t));
    int n = snprintf(buf, BUFSIZ, "%d-%02d-%02d %02d:%02d:%02d.%06ld %s(%5d):",my_tm.tm_year + 1900, \
                my_tm.tm_mon + 1, my_tm.tm_mday,my_tm.tm_hour, \
                my_tm.tm_min, my_tm.tm_sec, now.tv_usec,log_name[_level],Thread::gettid());
    write_length+=n;
}
Logger::~Logger(){
    char tail[256]={0};
    int n=snprintf(tail,sizeof tail,"-%s:%d\n",filename,_line);
    strcpy(buf+write_length,tail);
    write_length+=n;
    g_output(buf,write_length);
}
void Logger::log(const char *format,...){
    va_list valst;
    va_start(valst, format);
    int n=vsnprintf(buf+write_length,BUFSIZ-1,format,valst);
    write_length+=n;
    va_end(valst);
}