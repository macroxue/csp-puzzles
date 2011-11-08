#ifndef QUEUE_H
#define QUEUE_H

struct QueueObject
{
    QueueObject *prev;
    QueueObject *next;
};

template <typename T>
class Queue
{
    public:
        Queue();

        void Enqueue(QueueObject* obj);
        T    Dequeue();
        bool IsEmpty() const;

    private:
        QueueObject* head;
        QueueObject* tail;
};

template <typename T>
Queue<T>::Queue()
    : head(NULL),tail(NULL)
{
}

template <typename T>
void Queue<T>::Enqueue(QueueObject* obj)
{
    obj->next = NULL;
    obj->prev = tail;
    if (tail == NULL)
        head = obj;
    else
        tail->next = obj;
    tail = obj;
}

template <typename T>
T    Queue<T>::Dequeue()
{
    QueueObject *obj = head;
    head = head->next;
    if (head == NULL)
        tail = NULL;
    else
        head->prev = NULL;
    return (T)obj;
}

template <typename T>
bool Queue<T>::IsEmpty() const
{
    return (head == NULL);
}

#endif
