---
title: GDB 死锁分析
layout: post
comments: true
language: chinese
category: [program]
keywords:  program,c,deadlock
description:
---


<!-- more -->

{% highlight c %}
#include <unistd.h>
#include <pthread.h>

struct foobar {
        pthread_mutex_t mutex1;
        pthread_mutex_t mutex2;
};

void *thread1(void *arg)
{
        struct foobar *d;

        d = (struct foobar *)arg;
        while (1) {
                pthread_mutex_lock(&d->mutex1);
                sleep(1);
                pthread_mutex_lock(&d->mutex2);

                pthread_mutex_unlock(&d->mutex2);
                pthread_mutex_unlock(&d->mutex1);
        }
}

void *thread2(void *arg)
{
        struct foobar *d;

        d = (struct foobar *)arg;
        while (1) {
                pthread_mutex_lock(&d->mutex2);
                sleep(1);
                pthread_mutex_lock(&d->mutex1);

                pthread_mutex_unlock(&d->mutex1);
                pthread_mutex_unlock(&d->mutex2);
        }
}

int main(void)
{
        pthread_t tid[2];
        struct foobar data = {
                PTHREAD_MUTEX_INITIALIZER,
                PTHREAD_MUTEX_INITIALIZER
        };

        pthread_create(&tid[0], NULL, &thread1, &data);
        pthread_create(&tid[1], NULL, &thread2, &data);

        pthread_join(tid[0], NULL);
        pthread_join(tid[1], NULL);

        return 0;
}
{% endhighlight %}

{% highlight python %}
{% endhighlight %}
