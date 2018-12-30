#ifndef _epoller_h
#define _epoller_h

#include <vector>
#include <memory>
/**
 *多路复用器，即epoll的封装，每个event loop拥有一个epoll对象，即每个线程拥有一个独立的
 *epoller对象,epoller对象的生命期和IO线程一致;它负责将就绪的事件分发到IO线程。
*/
//前向声明以避免包含头文件
struct epoll_event;
namespace event
{
class Event;
class Epoller
{
public:
  typedef std::vector<epoll_event> EEventList;
  typedef std::vector<Event *> EventList;
  Epoller();
  ~Epoller();
  //更新事件
  bool updateEvent(Event *ev);
  //核心机制，在其内部进行就绪事件的监控
  int poll(int ms, EventList &el);

private:
  //epoll fd
  int epollFd_;
  EEventList events_;
};
} // namespace event
#endif // !_epoller_h
