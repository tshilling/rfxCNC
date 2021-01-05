#pragma once
/*
USAGE:
  Designed around using objects with inheritance and maintaining a linking / delinking schema.
  Using the following pattern, will maintain the object on the heap, independant of scope.
    base* B = new case;
    child* C =  new child;
    queue.enqueue(B);
    queue.enqueue(C);
  when queue.dequeue() is called, the object is NOT Returned. A boolean indicating success is returned
  and the object is deleted, freeing it from the Heap. Manipulate the object using the getTailPtr / 
  getHeadPtr functions, then dequeue the object when done with object.
*/
#include "Arduino.h"
template <typename T>
class rfx_queue{
  
  uint16_t count = 0;
  uint16_t index_of_Tail = 0;

  public:
  bool locked = false;
  class Node{
   public:
    String name = "";
    T* item;
    Node* next;
    Node* previous;
    Node() { next = nullptr; previous = nullptr; }
    ~Node() { next = nullptr; previous = nullptr; }
  };
  std::vector<Node> nodes;
  Node* Head = nullptr;
  Node* Tail = nullptr;
  public:
  bool enqueue(T* item){
    if(count>=nodes.size())
      return false; // Full
    nodes[index_of_Tail].item = item;
    nodes[index_of_Tail].name ="C:"+String(count);
    if(count==0){
      Head = &nodes[index_of_Tail];
      Tail = &nodes[index_of_Tail];
      nodes[index_of_Tail].previous = nullptr;
      nodes[index_of_Tail].next = nullptr;
    }
    else{
      nodes[index_of_Tail].previous = Tail;
      Tail->next = &nodes[index_of_Tail];
      Tail=&nodes[index_of_Tail];
    }
    index_of_Tail++;
    count++;
    if(index_of_Tail >= nodes.size()){
      index_of_Tail = 0;
    }
    return true;
  }
  bool IRAM_ATTR dequeue(){
    if(count==0)
      return false;
    delete Head->item;
    Head->item = nullptr;
    Node* newHead = Head->next;
    Head->next = nullptr;
    Head->previous = nullptr;
    Head = newHead;
    count--;
    if(count == 0)
    {
      Head = nullptr;
      Tail = nullptr;
    }
    return true;
  }
  bool isEmpty(){
    if(count==0)
      return true;
    return false;
  }
  bool isFull(){
    if(count>=nodes.size())
      return true;
    return false;
  }
  bool isFull(uint16_t room_left){
    if(count>=(nodes.size()-room_left))
      return true;
    return false;
  }
  void resize_queue(uint16_t size_of_buffer){
    nodes.reserve(size_of_buffer);
    nodes.resize(size_of_buffer);
  }
  rfx_queue(){
    index_of_Tail = 0;
    count = 0;
    Head = nullptr;
    Tail = nullptr;
  }
  ~rfx_queue(){
    
  }
  Node* getHeadPtr(){
    if(count == 0)
      return nullptr;
    return Head;
  }
  Node* getTailPtr(){
    if(count == 0)
      return nullptr;
    return Tail;
  }
  T* IRAM_ATTR getHeadItemPtr(){
    if(!Head)
      return nullptr;
    return Head->item;
  }
  T* getTailItemPtr(){
    if(!Tail)
      return nullptr;
    return Tail->item;
  }
};