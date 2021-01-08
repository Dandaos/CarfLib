#ifndef BUFFER_H
#define BUFFER_H
#include<vector>
#include<stdlib.h>
#include<string>
#include<iostream>
#include<algorithm>
#include<string.h>
class Buffer{
    public:
        char CRLF[3];
        Buffer(int size=1024):writeIndex_(0),
                        readIndex_(0),
                        buffer_(size){
            strcpy(CRLF,"\r\n");
        }
        inline int readableBytes()const { return writeIndex_-readIndex_;}
        inline int writableBytes()const { return buffer_.size()-writeIndex_;}
        void append(const char*s,int len);
        void append(const char* s)  {append(s,strlen(s));}
        void append(const std::string &s) {append(s.c_str(),s.size());}
        char* beginWrite() {return begin()+writeIndex_;}
        const char* beginRead() {return begin()+readIndex_;}
        void reset(){writeIndex_=readIndex_=0;}
        std::string retrieveLen(int len);
        std::string retrieveAll() {return retrieveLen(readableBytes());}
        void addWriteIndex(int len){writeIndex_+=len;}
        void addReadIndex(int len){readIndex_+=len;}
        inline int size() const {return buffer_.size();}
        void resize();
        const char* findCRLF();
    private:
        char *begin(){return &*buffer_.begin();}
        std::vector<char>  buffer_;
        int readIndex_;
        int writeIndex_;
};

#endif
