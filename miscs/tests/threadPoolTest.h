#ifndef  _threadPoolTest_h
#define _threadPoolTest_h
#include <map>
#include <string>
#include <vector>
#include <memory>
#include <utility>
#include <iostream>
#include "../bases/thread.h"
#include "../bases/mutex.h"

using bases::Mutex;
using bases::MutexGuard;
using std::map;
using std::pair;
using std::shared_ptr;
using std::string;
using std::vector;

class ProducerConsumer:Nocopyable
{
public:
  ProducerConsumer()
      : data_(new Map), updateCnts(0), copyonWriteCnts(0)
  {
  }
  void get(const string &item) const;
  void put(const string &item, const int number);
  double copyonWriteRate() const
  {
    MutexGuard mg(mutex_);
    return static_cast<double>(copyonWriteCnts) / updateCnts;
  }

private:
  typedef map<string, int> Map;
  typedef shared_ptr<Map> MapPtr;

  MapPtr getData() const
  {
    MutexGuard mg(mutex_);
    return data_;
  }

  mutable Mutex mutex_;
  MapPtr data_;
  int updateCnts;
  int copyonWriteCnts;
};
#endif // ! _threadPoolTest_h