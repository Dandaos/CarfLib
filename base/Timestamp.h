#ifndef TIMESTAMP_H
#define TIMESTAMP_H
#include<string>
class Timestamp
{
    public:
        Timestamp():microSeconds(0){}
        explicit Timestamp(long _microseconds):microSeconds(_microseconds){}
        std::string toFormattedString(bool showMicroseconds) const;
        inline long microSecondsSinceEpoch()const {return microSeconds;}
        inline bool operator<(const Timestamp& that) const
        {
            return microSeconds<that.microSecondsSinceEpoch();
        }
        inline bool operator==(const Timestamp& that)const
        {
            return microSeconds==that.microSecondsSinceEpoch();
        }
        inline void addTime(long _microSeconds)
        {
            microSeconds+=_microSeconds;
        }
        inline bool operator<=(const Timestamp&that)const
        {
            return microSeconds<=that.microSecondsSinceEpoch();
        }
        static Timestamp now();
        static const int kMicroSecondsPerSecond = 1000 * 1000;
    private:
        long microSeconds;

};

#endif