#ifndef _nocopyable_h
#define _nocopyable_h

class Nocopyable
{
public:
  Nocopyable() = default;
  Nocopyable(const Nocopyable &n) = delete;
  Nocopyable& operator=(const Nocopyable &n) = delete;
};
#endif