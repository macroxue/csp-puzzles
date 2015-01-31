#ifndef QUEUE_H
#define QUEUE_H

struct QueueObject {
  QueueObject* prev;
  QueueObject* next;
  bool in_queue;

  QueueObject() : prev(NULL), next(NULL), in_queue(false) {}
};

template <typename T>
class Queue {
 public:
  Queue();

  void Enqueue(QueueObject* obj);
  T Dequeue();
  bool IsEmpty() const;

 private:
  QueueObject* head;
  QueueObject* tail;
};

template <typename T>
Queue<T>::Queue()
    : head(NULL), tail(NULL) {}

template <typename T>
void Queue<T>::Enqueue(QueueObject* obj) {
  obj->next = NULL;
  obj->prev = tail;
  if (tail == NULL)
    head = obj;
  else
    tail->next = obj;
  tail = obj;
  obj->in_queue = true;
}

template <typename T>
T Queue<T>::Dequeue() {
  QueueObject* obj = head;
  head = head->next;
  if (head == NULL)
    tail = NULL;
  else
    head->prev = NULL;
  obj->in_queue = false;
  return (T)obj;
}

template <typename T>
bool Queue<T>::IsEmpty() const {
  return (head == NULL);
}

#endif
