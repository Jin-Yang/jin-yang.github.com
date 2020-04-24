#ifndef LIBUTILS_STRUCT_QUEUE_H_
#define LIBUTILS_STRUCT_QUEUE_H_ 1

#define Q_TYPE   int

/*
** NOTE: the front one will be empty, the first one will be (front + 1).
** NOTE: it's actually capacity is (capacity - 1).
*/
struct queue {
        int front, back, capacity;
        Q_TYPE *data;
};

static inline int queue_is_empty(struct queue *q)
{
        return (q->front == q->back);
}

static inline int queue_is_full(struct queue *q)
{
        return (((q->back + 1) % q->capacity) == q->front);
}

static inline int queue_get_size(struct queue *q)
{
        if (q->back >= q->front)
                return q->back - q->front;
        else
                return q->capacity - (q->front - q->back);
}

static inline int queue_get_capacity(struct queue *q)
{
        return q->capacity;
}

void queue_destroy(struct queue *q);
struct queue *queue_create(int capacity);
int queue_get_front(struct queue *q, Q_TYPE *ret);
int queue_get_back(struct queue *q, Q_TYPE *ret);
int queue_push(struct queue *q, Q_TYPE val);
int queue_pop(struct queue *q, Q_TYPE *ret);

#endif
