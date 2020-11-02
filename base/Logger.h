#ifndef LOGGER_H
#define LOGGER_H
#include<string>
#include<string.h>
#include<stdio.h>
#include<functional>
#define BUFSIZE 1024
using std::placeholders::_1;
using std::placeholders::_2;
class Lgger;

class Logger{
    public:
        //typedef void(*OutputFunc) (const char*,int size);
        typedef std::function<void(const char *,int)> OutputFunc;
        enum LOG_LEVEL{
            DEBUG=0,
            INFO,
            WARN,
            ERROR,
            LOG_NUM
        };
        Logger(const char* file,int line,LOG_LEVEL level);
        ~Logger();
        static void setLogLevel(LOG_LEVEL level);
        static void setOuputFunc(OutputFunc func);
        static LOG_LEVEL getLogLevel();
        void log(const char *format,...);
    private:
        char filename[128];//所在文件名
        int  _line;//所在行
        char buf[BUFSIZE];
        LOG_LEVEL _level;
        int write_length;


};
extern Logger::LOG_LEVEL g_logLevel;
extern bool log_t;
extern Logger::OutputFunc g_output;
void defaultOutput(const char*,int len);
#define LOG_INFO(format,...) if(g_logLevel<=Logger::INFO&&log_t) \
            Logger(__FILE__,__LINE__,Logger::INFO).log(format,##__VA_ARGS__)

#define LOG_WARN(format,...) if(g_logLevel<=Logger::WARN&&log_t) \
            Logger(__FILE__,__LINE__,Logger::WARN).log(format,##__VA_ARGS__)

#define LOG_ERROR(format,...) if(g_logLevel<=Logger::ERROR&&log_t) \
            Logger(__FILE__,__LINE__,Logger::ERROR).log(format,##__VA_ARGS__)

#define LOG_DEBUG(format,...) if(g_logLevel<=Logger::DEBUG&&log_t) \
            Logger(__FILE__,__LINE__,Logger::DEBUG).log(format,##__VA_ARGS__)

#endif