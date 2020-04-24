#include <errno.h>
#include <stdlib.h>
#include <assert.h>

#include "queue.h"

void queue_destroy(struct queue *q)
{
        if (q == NULL)
                return;
        if (q->data != NULL)
                free(q->data);
        free(q);
}

struct queue *queue_create(int capacity)
{
        struct queue *q;

        if (capacity <= 0)
                capacity = 64;

        q = (struct queue *)calloc(1, sizeof(*q));
        if (q == NULL)
                return NULL;
        q->data = malloc(sizeof(Q_TYPE) * capacity);
        if (q->data == NULL) {
                free(q);
                return NULL;
        }
        q->capacity = capacity;

        return q;
}

int queue_get_front(struct queue *q, Q_TYPE *ret)
{
        if (queue_is_empty(q))
                return -1;
        if (ret != NULL)
                *ret = q->data[(q->front + 1) % q->capacity];
        return 0;
}

int queue_get_back(struct queue *q, Q_TYPE *ret)
{
        if (queue_is_empty(q))
                return -1;
        if (ret != NULL)
                *ret = q->data[q->back];
        return 0;
}
static int queue_double_capacity(struct queue *q)
{
        Q_TYPE *tmp;
        int cap, size, i, j;

        cap = q->capacity * 2;
        tmp = malloc(sizeof(Q_TYPE) * cap);
        if (tmp == NULL)
                return -ENOMEM;
        size = queue_get_size(q);
        for (i = 1, j = q->front + 1; i <= size; i++, j++)
                tmp[i] = q->data[j % q->capacity];

        free(q->data);
        q->data = tmp;
        q->capacity = cap;
        q->front = 0;
        q->back = size;

        return 0;
}

int queue_push(struct queue *q, Q_TYPE val)
{
        if (queue_is_full(q) && queue_double_capacity(q) < 0)
                return -1;
        q->back = (q->back + 1) % q->capacity;
        q->data[q->back] = val;

        return 0;
}

int queue_pop(struct queue *q, Q_TYPE *ret)
{
        if (queue_is_empty(q))
                return -1;
        q->front = (q->front + 1) % q->capacity;
        if (ret != NULL)
                *ret = q->data[q->front];

        return 0;
}
