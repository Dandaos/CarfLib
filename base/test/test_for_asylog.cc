#include "../LogFile.h"
#include "../Logger.h"
#include<vector>
#include<iostream>
LogFile* log_file=NULL;
void asyncOutput(const char* msg, int len){
    log_file->append_file(msg,len);
}
void print(){
    for(int i=0;i<100;i++)
        LOG_INFO("My name is %s,I'm %d years old!","luyuda",23);
}
int main(){
    std::string log_dir("/home/luyuda/WebServer/base/test");
    log_file=new LogFile(log_dir,true,1000000000,1000000);
    Logger::setOuputFunc(std::bind(&asyncOutput,_1,_2));
    std::vector<Thread*> vec;
    for(int i=0;i<10;i++){
        Thread *t=new Thread(std::bind(&print));
        vec.push_back(t);
        t->start();
    }
    for(int i=0;i<10;i++){
        vec[i]->join();
        delete vec[i];
        vec[i]=NULL;
    }
    log_file->join();
    delete log_file;
    return 0;
}