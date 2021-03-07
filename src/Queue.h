/*
 * Queue.h
*/

#ifndef custom_QUEUE_H
#define custom_QUEUE_H

#include <Arduino.h>

template<class T>
class Queue {
  private:
    int _head, _tail, _count;
    T *_data;
    int _maxitems;
  public:
    Queue(int maxitems = 256) { 
      _head = 0;
      _tail = 0;
      _count = 0;
      _maxitems = maxitems;
      _data = new T[maxitems + 1];   
    }
    ~Queue() {
      delete[] _data;  
    }
    inline int count();
    inline int head_index();
    inline int tail_index();
    bool enqueue(const T &item);
    T head(uint8_t index = 0);
    T tail(uint8_t index = 0);
    T deqeue();
    bool is_full();
    bool is_empty();
    uint16_t get_space_available();
    void clear();
};

template<class T>
inline uint16_t Queue<T>::get_space_available(){
  return _maxitems - _count;
}
template<class T>
inline bool Queue<T>::is_full(){
  if(_count==_maxitems)
    return true;
  return false;
}
template<class T>
inline bool Queue<T>::is_empty(){
  if(_count==0)
    return true;
  return false;
}

template<class T>
inline int Queue<T>::count() 
{
  return _count;
}

template<class T>
inline int Queue<T>::head_index() 
{
  return _head;
}

template<class T>
inline int Queue<T>::tail_index() 
{
  return _tail;
}

template<class T>
bool Queue<T>::enqueue(const T &item)
{
  if(_count < _maxitems) { // Drops out when full
    _data[_tail++]=item;
    ++_count;
    // Check wrap around
    if (_tail > _maxitems)
      _tail -= (_maxitems + 1);
    return true;
  }
  return false;
}

template<class T>
T Queue<T>::deqeue() {
  if(_count <= 0) return T(); // Returns empty
  else {
    T result = _data[_head];
    _head++;
    --_count;
    // Check wrap around
    if (_head > _maxitems) 
      _head -= (_maxitems + 1);
    return result; 
  }
}

template<class T>
T Queue<T>::head(uint8_t index) {
  if(_count <= 0) 
    return T(); // Returns empty
  if(index>=_count)
    return T();
  int16_t i = _head+index;
  if(i>=_maxitems)
    i -= (_maxitems);
  return _data[i];
}
template<class T>
T Queue<T>::tail(uint8_t index) {
  if(_count <= 0) 
    return T(); // Returns empty
  if(index>=_count)
    return T();
  int16_t i = _tail-1-index;
  if(i<0)
    i += (_maxitems);
  return _data[i];
}

template<class T>
void Queue<T>::clear() 
{
  _head = _tail;
  _count = 0;
}

#endif