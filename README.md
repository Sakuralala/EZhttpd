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
  
  
