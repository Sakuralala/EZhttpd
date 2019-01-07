2018.12.12-2018.12.17  
本周参考muduo以及github上的Tkeed完成了基础的pthread库相关同步原语及线程、线程池的封装，具体来说，封装好了条件变量、互斥器、任务队列、倒计时、线程、线程池。  
并对线程池进行了简单的测试(详见bases/threadPoolTest.cc),具体来说测试任务如下:把"man open"输出到一个文件(就是想随便找一个单词量适中的以便于测试)中，并将其内部的每个单词存入vector<string>(看了下总共记了刚好1000个单词),线程池开始工作后，主线程不断地往ThreadPoolTest实例中写"string-int"对(随机),并不断往task queue中添加任务(随机get一个"string-int"对,和前面那个随机的对不一定一样),然后主要是为了测试muduo上所说的copy-on-write所占的比例(读的时候使用了一个临时对象来增加实际数据的引用计数，然后马上释放锁，尽量减小临界区的大小，以便其他读写线程可以工作，这个测的就是有子线程在读的情况下主线程恰好要写的比例)，由于我的测试任务比其简陋很多，所以最终测试的结果还是有比较大差别的;    
测试时使用了4个子线程，每次测试，主线程都添加10w次"string-int"对及任务(试了下，这个数字再增大(50w)最终的结果变化不大，所以就取了10w次)，最终测试10次，取个平均值的copy-on-write比例大概是在11%-12%左右。  

另外，在造轮子的过程中遇到的问题及尝试的解决方案:  
1).各个东西之间的耦合问题，如Thread和ThreadPool,刚开始的时候没搞清楚，在threadFuncWrapper中写上了loop,但实际上封装thread的目的仅仅是为了让子线程执行某个函数，至于是否是循环的，这个应该是给Thread传递函数的需要考虑的事，即由ThreadPool来提供对应的loop函数；
2).因为后面需要做一个日志模块，那么随时随地地能够打印当前线程的id就是必须要实现的了，但是gettid()没有一个wrapper,每次都需要通过syscall来进行调用，即需要陷入内核，这造成了不必要的损耗，所以需要在某个地方将当前线程的id缓存起来(不要说fork什么的，这个多线程的web server暂时不支持fork),所以这里参考了muduo的做法(其实基本上都是参考了它的做法),使用了thread-local变量，只有在第一次查询线程id时会陷入内核，后面都是从这个缓存拿数据；  
3).因为需要保证主线程创建完子线程之后子线程至少要运行起来(不然的话有可能主线程一退出子线程还没运行就没了),所以使用了一个countdownLatch来保证子线程一定会进入执行的函数；  
4).线程池的退出模式，为了增加一个“平滑退出”模式，在 ThreadPool::threadFunction()(见threadPool.cc)这里存在race condition,但是不影响最终的结果，所以暂时就没改，详见注释；  

另外，还需要修改的地方:  
1).子线程命名，其实我感觉可有可无，只要有线程id就可以了；  
2).打日志，这个准备看看这周能不能实现；  

下周准备完成的:  
1).one loop per thread,即reactor模式的实现；  
2).有时间完成日志模块的实现；  

2018.12.17-2018.12.23  
1、epoll监控的是事件对应的文件描述符，而不应该仅仅片面地理解为对连接的抽象，虽然说监控的大部分是连接上事件；事件驱动模型，关键是对事件进行一个好的抽象，这么看来，其实nginx上的几个关键结构体虽说是分成了:ngx_connection_t、ngx_event_t,但其实我觉得ngx_event_t只是一部分，应该说它是事件回调的抽象，其内部是存在一个指向ngx_connection_t的指针的(这样把它称之为事件的抽象也不为过，而ngx_connection内部又存在一个指向具体消费模块的指针,下面说的就是http):  
[nginx的事件抽象](imgs/event.png)  
[nginx的连接抽象](imgs/conn.png)  
[nginx的http抽象](imgs/http.png)  
nginx这样分的目的应该是使得各部分更为清晰把，个人观点。那么在muduo中的抽象和nginx又有点区别了，它的channel类相当于是事件的抽象,但是这里的channel大致相当于nginx中的event和connection的组合，然后不包括connection中的事件消费模块的指针，以及用户态buffer、连接两端的sockaddr。针对这两个东西，muduo专门设计了两个类。最后，把这两个东西和channel类合一起组一个TcpConnection类，大概就相当于ngx_event_t了,但是TcpConnection类是不包含上层消费模块的信息的。      

2、event类(或者按muduo中说的channel类)是否需要使用smart pointer来包装？
muduo中说的是"channel对象的生命期由其owner对象负责管理",并且提供了tie方法以便延长channel对象或其owner对象(如TcpConnection)的生命期,以便于在某些场景下(通常为断开连接,典型的调用流程为:Channel::handleEvent->用户自定义的readCallback->用户在readCallback中可能会delete/reset channel对象或其owner对象，这将导致channel对象或者owner对象要被析构)保证在处理完本次事件前channel对象或其owner对象不会被析构(即保证相应的函数调用堆栈在弹出前对应的channel对象或其owner对象还存在,这样才能放心地继续调用channel对象或其owner对象的方法及访问其对象成员,不然的话会导致undefined behaviour)。而EventLoop对象相当于channel对象的observer,所以其内部使用一个non-owning raw pointer也说得过去;
3、muduo中TcpServer与TcpClient明明并不共用一个TcpConnection,为啥一方析构了TcpConnection会影响到另一方？  
muduo中所说的用户是指基于muduo这个库做二次开发的调用者而不是TcpClient，TcpServer和TcpClient都是muduo库的一部分，两者拥有的TcpConnection对象是独立的。要站在"库"的这个角度来考虑问题，库的作者肯定不能对后面的调用者的开发作任何假设，如果使用raw pointer来保存TcpConnection的话，那么调用者他很可能就在自己的应用代码里判断当前Tcp连接断开后直接delete掉TcpConnection/Channel对象了，但问题是Channel对象的handleEvent回调还没执行完，然而它却没了，这会导致程序崩溃。那么使用shared_ptr的话，可以保证只有在TcpConnection对象在没有引用的时候才被析构。  

总结:这周零零散散地写了一部分代码，但是在后半段时一直没搞清楚对象生命期的管理问题(主要是channel类)，即是不是所有的堆上对象都无脑改用shared_ptr进行管理,感觉上这肯定是有一定问题的，比如说可能意外延长对象的生命期甚至导致其"永世长存",这就相当于内存泄漏了;决定停下来结合书看一下muduo的源码，但是还是感觉有点干巴巴的，没有什么具体的例子能够加深理解。  
好在muduo提供了examples,里面的例子写得非常好，也顺带学习了下原始的gdb的用法，包括打断点、输出当前堆栈信息、输出变量的信息等，主要看了的有simple中的5个例子，这几个例子也纠正了我之前的一个误区，即muduo是作为一个网络库提供给开发者进行调用的，所以它的设计初衷是让使用者调用方便，以及考虑到调用者可能会出现的各种使用情况，如生命期管理相关的，要考虑到调用者的回调中可能存在导致channel对象或其owner对象被析构并释放的行为等。另外，asio/chat这个例子我重点追踪 了TcpConnection对象的引用计数变化情况，也看到了使用smart pointer来进行对象生命期管理所带来的便利性(前提是需要知道如何避免其潜在的弊端，主要是意外延长对象的生命期)。到这里为止基本理解了为什么要用smart pointer对TcpConnection对象进行管理(当然肯定也有其他的替代方案，内存池，或者是raw pointer，如果仔细设计的话，可能也不会有问题，但是使用smart pointer的优势就是方便高效且不容易出错,raw pointer可能现在不会出错，但是后面如果进行了修改什么的出错的概率相较smart pointer会大),但是对channel对象的生命期管理还不是很理解(channel对象肯定是需要在多个上层对象中进行传递的，那么是需要在每个用到它的类中都使用shared_ptr还是只在某个类中使用)，在追踪了curl这个例子之后，终于了解了作者设计其的初衷及做法，"由channel的owner对象对其生命期进行管理",即owner对象负责Channel成员的析构操作()，curl这个例子只用到了EventLoop及Channel类，而Channel类是作为Request对象的成员并使用了shared_ptr进行包装，在新建连接创建channel对象时就调用了对应的tie方法，这样后续在事件到来调用用户的回调时就可以保证当前channel对象在函数调用栈完全弹出前不会被销毁(handleEvent会先尝试提升之前tie的weak_ptr)，从而确保了程序正确的行为。  
那么在用到channel对象的类中及使用channel作为参数的函数都使用shared_ptr是否可行呢?考虑下通常的移除channel的流程:对端断开连接事件触发Channel::handleEvent->调用用户的读事件回调onRead->onRead中调用Channel::remove以便在当前线程的epoll对象中删除监控->onRead末尾调用shared_ptr<Channel>::reset，可以看到由于Channel对象的生命期是在unregister之后才结束的，也就是说先前在EventLoop、Poller类中的传递调用都是保证其存活的，那么何必多此一举呢？而且这种做法并没有起到延长Channel对象的效果(因为在onRead调用结束后channel已经没了,但是其函数调用栈还没结束)。  
那将Channel对象作为其owner对象的直接成员是否可行呢？想了想似乎可行，这样Channel对象的生命期就和其owner对象一致，那么只需要做好其owner对象的生命期管理就可以保证正常的行为(TODO:)。但是这个有一个弊端，即增加了头文件依赖，暴露了内部类给用户。  
2018.12.24-12.31  
1、成员函数的线程安全性(即是否可以跨线程调用成员函数)  
我觉得这得分有没有锁，如果在读写成员的时候都进行了必要的加锁操作，那么这个成员函数应该是线程安全的；  
但是如果没有锁的话，判断一个成员函数是否可以跨线程调用的基本原则就是看有没有可能发生多个线程同时对对象的某个成员进行读写操作,如果存在这种可能性，那么这个成员函数就不是线程安全的。      
另外，设计的时候就要考虑成员函数是否可以跨线程调用,根据muduo所说的设计原则，最重要的一条就是尽可能地减少线程间的同步共享，因为保证线程同步共享的锁引起的锁争用会降低性能,锁一般由owner持有而不是caller:  
对于不可跨线程调用的函数，其开头必须要保证由owner线程调用，否则直接报错，符合意思的例子有:  
```cpp
/**************************************EventLoop*****************************************/
//race condition:非owner线程对定时器集合写，owner线程对定时器集合读及写;
void addTimer(const Timer &t);
//首先这就不符合"one loop per thread"的本意，即一个线程管一个事件循环；另外也存在race condition:
//包括定时器集合的同时读写、任务等待队列的同时读写等,(poller对象的同时读写在epoll对象内部有保护,红黑树通过mutex保护，readylist通过自旋锁保护)；  
void loop();
/****************************************Event********************************************/
void enable/disable*();
void set*();
void remove();
void update();
/***************************************Epoller*********************************************/
//由EventLoop保证，因为这两个函数仅供EventLoop调用
void updateEvent(Event*);
int poll();
```
对于可以跨线程调用的成员函数，若存在同时读写对象成员的可能性，则需要加锁，符合意思的例子有:  
```cpp
/************************************EventLoop********************************************/
//race condition:非owner线程写looping_,owner线程读looping_
void quit();
//race condition:非owner线程写任务队列，owner线程读写任务队列
void queueInLoop();
void excuteCallbacks();
/***********************************Event************************************************/
void set*Callback();//set*Callback没用锁，其实有race condition,不过一般只在创建Event时调用所以没有问题
```
另外，如果需要跨线程调配任务(通常是主线程给子线程分配任务),但是需要调用的又是不能跨线程调用的成员函数(如EventLoop::addTimer),那么需要提供一个统一的接口来完成这个目的:   
```cpp
void EventLoop::runInLoop(const Callback &cb);
void EventLoop::queueInLoop(Callback cb);
```
需要调用不可跨线程调用成员函数func的非owner线程可以将func通过std::bind()打包成符合call signature的function再通过以上两个成员函数来将其加入到owner线程的等待任务队列中；这个其实也会产生锁争用，因为queueInLoop内部需要获取mutex，这里其实是方便了工作线程，保证其loop过程中不需要进行抢锁，从而避免了可能的阻塞操作(不然的话对定时器、任务队列操作都需要抢锁极有可能阻塞)；  
另外可以看到，使用锁的情况很少，只有在需要调配任务时(或者是需要退出时)才加锁，其他情况下都是采用assertInOwnerThread的方式来避免锁，理由如上所述，线程间调配任务必须要加锁，而且还不能阻塞工作线程主循环。可以说，跨线程调用的精髓就在runInLoop这里(通过将不可跨线程调用的成员函数加入owner线程的eventloop的任务队列里来达到只在owner线程调用的目的)。       
2、muduo中许多的成员都是通过unique_ptr来间接持有的，目的是为了降低用户使用时的编译依赖，尽量少暴露内部类给用户。   
3、命名空间的问题、EventLoop类和Event类相互引用?  
由于Event类需要在epoll中进行更新，所以需要一个它所属的EventLoop类的指针；EventLoop类需要知道当前就绪的Event，故需要Event指针；以及双方的前向声明；  

4、timerfd的使用.  
timerfd_settime(),其中第二个参数有两个成员:  
```c
    struct itimerspec {
               struct timespec it_interval;  /* Interval for periodic timer */
               struct timespec it_value;     /* Initial expiration */
           };
```
即，需要周期性定时的话需要设置it_interval;另外，注意epoll在ET模式下必须要先读了timerfd才能引发下一次的超时触发。  
2019.01.02-2019.01-06  
本周的任务是完成定时器的实现；muduo中使用的linux下较新的timerfd系统调用；考虑实现一个类似nginx那种形式的定时器。  

2019.01.04  
1、初步完成了定时器的设计，类似于nginx那种，但是暂时没做缓存；  

2、IOThread和Thread间采取继承关系可以再省掉一个countdownLatch?  
由于countdownLatch是一次性的，在父类Thread里用了就不能用了，所以直接将其改成了类似信号量的形式，这样才能省一个。  
(后续:又想了一下，总觉得信号量不太符合意思，最终还是决定使用两个countdownLatch)  

2019.01.05  
1、初步完成了io线程池的设计;  

总结:  
完成了定时器模块的设计，不同于muduo使用timerfd系统调用，而是使用了类似nginx的那种，但是没做时间缓存；另外，看了下相关的开源代码，发现常见的是使用优先队列+惰性删除的方式，需要进一步比较优劣(TODO:);    
修改了IOThread的设计，由has-a改为了is-a，本意是为了省一个countdownLatch,后来想了想发现不太好又改成了两个；  
完成了IOThreadPool的设计，这个比较简单，基本上就是封装一下；  


2019.01.07-2019.01.13  
本周的任务是完成日志模块的设计。  