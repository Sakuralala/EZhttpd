# EZhttpd
A simple http web server.    
Features:  
  1.Based on c++11.     
  2.Support GET、HEAD method.  
  3.Support keep-alive.  
  4.Thread pool and epoll ET mode is adpoted.  
  5.Master-slave reactor model, master reactor is responsible for monitoring and distributing new connections, and the sub reactors is responsible for parsing subsequent requests.  
  6.Smart pointer is used to avoid memory leak.   
  7.Effective asynchronous log, which uses "multi-producer, single consumer" model,and double buffer strategy is adopted.  
  
  感谢陈硕老师及其的书:《Linux多线程服务端编程--使用muduo C++网络库》.    
  感谢linya的webbench改进版(详见bin/webbench,增加了长连接的测试，另外我修改了下bytes的类型，防止结果溢出).
  
