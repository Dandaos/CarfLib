#include "Buffer.h"
void Buffer::append(const char*s,int len){
    while(len>writableBytes()){resize();}
    std::copy(s,s+len,beginWrite());
    writeIndex_+=len;
}
std::string Buffer::retrieveLen(int len){
    len=len<readableBytes()?len:readableBytes();
    std::string s(beginRead(),len);
    readIndex_+=len;
    if(readIndex_==writeIndex_) reset();
    return s;
}
void Buffer::resize(){
    std::vector<char> other(size()*2);
    std::copy(beginRead(),(const char*)begin()+readableBytes(),&*other.begin());
    buffer_.swap(other);
}
const char* Buffer::findCRLF(){
    const char*crlf=std::search(beginRead(),(const char*)beginWrite(),CRLF,CRLF+2);
    return crlf==beginWrite()?NULL:crlf;
}