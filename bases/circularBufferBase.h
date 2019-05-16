#ifndef __circular_buffer_base_h
#define __circular_buffer_base_h
#include <memory>
#include <algorithm>
#include <iterator>
/**
 * **/
namespace bases
{
//TODO:const 版本的迭代器
template <typename T>
class CircularBufferBase;
template <typename T>
struct CircularBufferIterator : public std::iterator<std::random_access_iterator_tag, CircularBufferBase<T>>
{
    typedef typename std::iterator<std::random_access_iterator_tag, CircularBufferBase<T>>::difference_type difference_type;
    CircularBufferIterator(CircularBufferBase<T> *buf, size_t pos) : buf_(buf), pos_(pos) {}
    ~CircularBufferIterator() = default;
    T &operator*()
    {
        return (*buf_)[pos_];
    }
    T *operator->()
    {
        return &(operator*());
    }
    CircularBufferIterator operator-(difference_type n) const
    {
        auto ret(*this);
        ret.pos_ = (pos_ + buf_->capacity() - n) % buf_->capacity();
        return ret;
    }
    CircularBufferIterator &operator-=(difference_type n)
    {
        pos_ = (pos_ + buf_->capacity() - n) % buf_->capacity();
        return *this;
    }
    CircularBufferIterator &operator--()
    {
        *this -= 1;
        return *this;
    }
    CircularBufferIterator operator--(int)
    {
        auto ret(*this);
        *this -= 1;
        return ret;
    }
    CircularBufferIterator operator+(difference_type n) const
    {
        auto ret(*this);
        ret.pos_ = (pos_ + n) % buf_->capacity();
        return ret;
    }
    CircularBufferIterator &operator+=(difference_type n)
    {
        pos_ = (pos_ + n) % buf_->capacity();
        return *this;
    }
    CircularBufferIterator &operator++()
    {
        *this += 1;
        return *this;
    }
    CircularBufferIterator operator++(int)
    {
        auto ret = *this;
        *this += 1;
        return ret;
    }
    CircularBufferBase<T> *buf_;
    size_t pos_;
};
template <typename T>
inline typename CircularBufferIterator<T>::difference_type operator-(const CircularBufferIterator<T> &ite1, const CircularBufferIterator<T> &ite2)
{
    if ((ite1.buf_->reversed() && ite1.pos_ > ite2.pos_) || (!ite1.buf_->reversed() && ite1.pos_ < ite2.pos_))
        return -1;
    //assert: ite1 is behind ite2.
    return (ite1.pos_ + ite1.buf_->capacity() - ite2.pos_) % ite1.buf_->capacity();
}
template <typename T>
inline bool operator<(const CircularBufferIterator<T> &ite1, const CircularBufferIterator<T> &ite2)
{
    //assert: ite1.buf_ == ite2.buf
    if (ite1.buf_->reversed())
    {
        if (ite1.pos_ >= ite1.buf_->begin().pos_ && ite2.pos_ <= ite2.buf_->end().pos_)
            return true;
        else if (ite1.pos_ <= ite1.buf_->end().pos_ && ite2.pos_ >= ite2.buf_->begin().pos_)
            return false;
    }
    return ite1.pos_ < ite2.pos_;
}
template <typename T>
inline bool operator==(const CircularBufferIterator<T> &ite1, const CircularBufferIterator<T> &ite2)
{
    return ite1.pos_ == ite2.pos_;
}
template <typename T>
inline bool operator!=(const CircularBufferIterator<T> &ite1, const CircularBufferIterator<T> &ite2)
{
    return !(ite1 == ite2);
}
template <typename T>
class CircularBufferBase
{
private:
    //TODO:合理的值
    static const size_t MaxSize = 10240;

public:
    typedef CircularBufferIterator<T> iterator;
    CircularBufferBase(size_t size) : base_(::new T[size > MaxSize ? MaxSize : size]), head_(0), tail_(0), dataSize_(0), capacity_(size > MaxSize ? MaxSize : size)
    {
    }
    ~CircularBufferBase() = default;
    bool reversed() const
    {
        return dataSize_ && (tail_ <= head_);
    }
    T &operator[](size_t k)
    {
        //assert: head_<k<tail_
        return base_.get()[k];
    }
    T &operator[](size_t k) const
    {
        return base_.get()[k];
    }
    void push_back(const T &val)
    {
        if (full())
            resize();
        if (!full())
        {
            base_[tail_] = std::move(val);
            tail_ = (tail_ + 1) % capacity_;
            dataSize_++;
        }
    }
    void push_front(const T &val)
    {
        if (full())
            resize();
        if (!full())
        {
            head_ = (head_ + capacity_ - 1) % capacity_;
            base_[head_] = std::move(val);
            dataSize_++;
        }
    }
    void pop_back()
    {
        if (!empty())
        {
            tail_ = (tail_ + capacity_ - 1) % capacity_;
            dataSize_--;
        }
        if (empty())
            reset();
    }
    void pop_front()
    {
        if (!empty())
        {
            head_ = (head_ + 1) % capacity_;
            dataSize_--;
        }
        if (empty())
            reset();
    }
    void erase(iterator first, iterator last)
    {
        //assert:this == first.buf_ == last.buf_
        auto dis = (last.pos_ + capacity_ - first.pos_) % capacity_;
        if (first.pos_ == head_)
        {
            head_ = (head_ + dis) % capacity_;
            dataSize_ -= dis;
            if (head_ == tail_)
                head_ = tail_ = 0;
            return;
        }
        if (last.pos_ == tail_)
        {
            tail_ = (tail_ + capacity_ - dis) % capacity_;
            dataSize_ -= dis;
            if (head_ == tail_)
                head_ = tail_ = 0;
            return;
        }
        auto dis1 = (first.pos_ + capacity_ - head_) % capacity_, dis2 = (tail_ + capacity_ - last.pos_) % capacity_;
        if (head_ < tail_)
        {
            if (dis2 <= dis1)
            {
                std::copy(base_.get() + last.pos_, base_.get() + tail_, base_.get() + first.pos_);
                tail_ = (tail_ + capacity_ - dis) % capacity_;
            }
            else
            {
                std::copy_backward(base_.get() + head_, base_.get() + first.pos_, base_.get() + last.pos_);
                head_ = (head_ + dis) % capacity_;
            }
        }
        else
        {
            //TODO:many cases...
            if (dis2 <= dis1)
            {
            }
            else
            {
            }
        }
        dataSize_ -= dis;
        if (empty())
            head_ = tail_ = 0;
    }
    void erase(iterator pos)
    {
        erase(pos, pos + 1);
    }
    void clear()
    {
        reset();
    }
    iterator actualBegin()
    {
        return iterator(this, 0);
    }
    iterator actualEnd()
    {
        return iterator(this, capacity_);
    }
    iterator begin()
    {
        return iterator(this, head_);
    }
    iterator end()
    {
        return iterator(this, tail_);
    }
    T &front()
    {
        return base_.get()[head_];
    }
    T &back()
    {
        //NOTE:-1 should be the last since tail_ is size_t.
        return base_.get()[(tail_ + capacity_ - 1) % capacity_];
    }
    int size() const
    {
        return dataSize_;
    }
    int remain() const
    {
        return capacity_ - dataSize_;
    }
    int capacity() const
    {
        return capacity_;
    }
    bool full() const
    {
        return dataSize_ == capacity_;
    }
    bool empty() const
    {
        return !dataSize_;
    }
    void reset()
    {
        head_ = tail_ = dataSize_ = 0;
    }
    void resize();

protected:
    std::unique_ptr<T[]> base_;
    size_t head_;
    size_t tail_;
    size_t dataSize_;
    size_t capacity_;
};
template <typename T>
void CircularBufferBase<T>::resize()
{
    if (capacity_ >= MaxSize)
        return;
    size_t newCapacity = capacity_ * 2 > MaxSize ? MaxSize : capacity_ * 2;
    std::unique_ptr<T[]> newBase(new T[newCapacity]);
    if (head_ < tail_)
        std::copy(base_.get() + head_, base_.get() + tail_, newBase.get());
    else if (head_ == tail_)
        std::copy(base_.get() + head_, base_.get() + capacity_, newBase.get());
    else
    {
        //把后一部分移到前面
        std::copy(base_.get() + head_, base_.get() + capacity_, newBase.get());
        //把前一部分移到后面
        std::copy(base_.get(), base_.get() + tail_, newBase.get() + capacity_ - head_);
    }

    tail_ = dataSize_;
    head_ = 0;
    capacity_ = newCapacity;
    base_.reset(newBase.release());
}
} // namespace bases
#endif