//#include <signal.h>
#include <vector>
#include <iostream>
#include "net/client.h"
#include "net/connection.h"
#include "event/eventLoop.h"
#include "bases/thread.h"
#include "log/logger.h"
using std::cout;
using std::endl;

void simpleReadCallback(const net::Client::ConnectionPtr &conn, bases::UserBuffer &buf)
{
  LOG_INFO << "Received message:" << buf.getAll();
}

void clientTest(net::Client *client, const net::Client::ConnectionPtr &conn)
{
  if (conn->send("Client Test.\r\n") == -1)
  {
    conn->handleClose();
    return;
  }
  conn->shutdownWrite();

  //FIXME:In local host test, the epoll of the server may not distinguish the first message and the second(FIN)(maybe the time interval of these two message is too small), so both the server and the client would not close the connection.
  //client->shutdownWrite();
}
void quit(net::Client *client)
{
  client->disconnect();
  client->getLoop()->quit();
}

int main()
{
  using event::EventLoop;
  using net::Client;
  EventLoop loop;
  Client client(&loop);
  client.setConnectionCallback(std::bind(&clientTest, &client, std::placeholders::_1));
  client.setReadCallback(std::bind(&simpleReadCallback, std::placeholders::_1, std::placeholders::_2));
  client.connect("127.0.0.1:8888");
  loop.addTimer(5, std::bind(&quit, &client));
  loop.loop();

  LOG_INFO << "Client stopped.";
}