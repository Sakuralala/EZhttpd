#include "threadPoolTest.h"

void ProducerConsumer::get(const string &item) const
{
  //两次copy ctor调用 第一次在用data_初始化函数的调用结果，第二次在用调用结果初始化data
  MapPtr data = getData();
  //MutexGuard mg(mutex_);
  auto ite = data->find(item);
  /*
  if (ite != data->end())
    //return ite->second;
    std::cout << "Thread " << bases::currentThreadID() << ", get string:" << item << " ,number:" << ite->second << std::endl;
  else
    //return -1;
    std::cout << "Thread " << bases::currentThreadID() << ", get string:" << item << " ,number: -1" << std::endl;
    */
}

void ProducerConsumer::put(const string &item, int number)
{
  MutexGuard mg(mutex_);
 // std::cout  << "Main thread, put string: " << item << ",number: " << number << std::endl;
  updateCnts++;
  if (!data_.unique())
  {
    copyonWriteCnts++;
    MapPtr newData(new Map(*data_));
    data_.swap(newData);
  }
  //assert(data_.unique());
  (*data_)[item] = number;
}
