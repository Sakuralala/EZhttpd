#ifndef __iothreadpool_h
#define __iothreadpool_h
#include <vector>
#include <memory>
#include "mutex.h"
namespace event
{
class IOThread;
class IOThreadPool
{
    public:
      IOThreadPool();
      ~IOThreadPool();

    private:
      std::vector<std::unique_ptr<IOThread>> IOThreads_;
};
} // namespace event
#endif // !__iothreadpool_h