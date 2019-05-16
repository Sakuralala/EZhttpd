#ifndef __new_circular_buffer_h
#define __new_circular_buffer_h

#include <string>
#include "circularBufferBase.h"

namespace bases
{
class CharCircularBuffer : public CircularBufferBase<char>
{
private:
    static const int InitialSize = 4096;

public:
    CharCircularBuffer() : CircularBufferBase(InitialSize) {}
    ~CharCircularBuffer() {}

    //读事件最终调用的回调
    int recv(int fd);
    //上层调用者写
    int send(int fd, const char *msg, int len);
    //写事件最终调用的回调
    int sendRemain(int fd);
    iterator findCRLF() ;
    iterator find(char ch) ;
    std::string getMsg(iterator first, iterator last);
    std::string getMsg(iterator first, size_t len) ;
    std::string getAll() ;
    bool retrieve(size_t len)
    {
        if (len > size())
            return false;
        erase(begin(), begin() + len);
        return true;
    }
};
typedef CharCircularBuffer UserBuffer;
} // namespace bases

#endif // !__new_circular_buffer_h
