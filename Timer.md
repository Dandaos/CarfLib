## 定时器
定时器用于定时关闭非活跃连接，即客户端若在规定期限内没有访问服务器，则服务器主动关闭连接；若在访问期限内再次访问，则以访问的时间为起点，设置新的超时时间。

### 1.timerfd_*系列函数
#### (1) timerfd_create

	#include <sys/timerfd.h>
	int timerfd_create(int clockid, int flags);
	//成功返回一个指代该对象的文件描述符, 失败返回-1并设置errno
timerfd_create把时间变成一个文件描述符，在定时器到期的时候变得可读，可以方便的融入select/poll/epoll等I/O多路复用体系中。因此，我们只需创建一个timerfd,并绑定好相应的回调函数(`TimerQueue::handleRead`)即可。
#### (2) timerfd_settime
	#include <sys/timerfd.h>
	int timerfd_settime(int fd, int flags, const struct itimerspec* new_value, struct itimerspec* old_value);
	//成功返回0, 失败返回-1并设置errno；flags 的值可以是 0 (相对时间), 可以是 TFD_TIMER_ABSTIME (绝对时间)
timerfd_settime用于设置定时器，这里我把定时器设置为只一次有效，在把所有小于当前时间的Timer处理完后再重新设置新的到期时间。

### 2.相关类

#### (1) Timer

Timer主要用于存储TcpConnection对象的存活时间，以及到期时相应的回调函数(`TcpConnection::handleTimer`)。

#### (2) TimerQueue

TimerQueue中使用成员变量timers_ `(std::map<Timestamp,std::shared_ptr<Timer>>)`存储所有连接的timer，主线程定时检查timers_ ，所有小于当前时间的timer会调用相应的回调函数(`TcpConnection::handleTimer`)并从 timers_ 移除。

在`TcpConnection::handleTimer`中，若`(当前时间-上一次访问的时间)>=存活时间`且此时`写缓冲中没有数据`，则服务器会主动调用`TcpConnection::shutdownWrite`关闭写端。若`(当前时间-上一次访问的时间)<存活时间`或者`此时写缓冲有数据`，则timer则会更新新的到期时间并將timer重新加入到timer_中。

#### (3) Timer与TcpConnection
Timer在到期时会最终执行`TcpConnection::handleTimer`，在此之前TcpConnection不能被析构，通过在绑定回调函数时使用`shared_ptr<TcpConnection>`替代原生指针可以避免这个问题。但这样带来的另外一个问题是TcpConnetion只能在Timer对象被销毁之后才能销毁，举个例子，若客户端在到期时间前就主动关闭连接了，TcpConnection此时应该及时析构以释放系统资源，但由于Timer的存在(因为其保存有`shared_ptr<TcpConnection>`的副本)而將该过程延后了。
实际上，Timer应当具有这样的功能，若TcpConnection对象存在，则执行`TcpConnection::handleTimer`函数;若已经被析构，则什么都不做。可以通过智能指针`weak_ptr`实现这样的功能，即Timer保存一个`std::weak_ptr<TcpConnction>`变量tcpConn_ ，在执行`TcpConnection::handleTimer`之前先lock()一下，判断对象是否已经被销毁。这样可以在绑定回调函数时使用TcpConnection原生指针，TcpConnection的析构不再受Timer的影响，同时Timer在执行回调函数时也不必担心TcpConnection对象被析构了。
