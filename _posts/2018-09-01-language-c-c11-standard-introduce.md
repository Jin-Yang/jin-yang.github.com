---
title: C11 标准解读
layout: post
comments: true
language: chinese
category: [misc]
keywords: 
description: 
---


<!-- more -->

## Designated Initializers

可以对结构体以及数组进行初始化，示例如下。

{% highlight c %}
#include <stdio.h>

struct person {
        int id;
        int age;
};

int main(void)
{
        struct person p = {
                .id = 1,
                .age = 18,
        };
		// [1 0 3 3]
        int array[] = {[0] = 1, [2 ... 3] = 3};

        printf("id:%2d age:%d\n", p.id, p.age);
        for (int i = 0; i < sizeof(array)/sizeof(array[0]); i++) {
                printf("%3d", array[i]);
        }
        puts("");

        return 0;
}
{% endhighlight %}

注意，数组的区间初始化时 GNU 的扩展。

这一特性，对于表驱动来说会方便很多，可以根据需要进行定义，如下：

{% highlight c %}
#include <stdio.h>

enum {
        ACT_UNKNOWN,
        ACT_MEOW,
        ACT_BARK,
        ACT_NUMS,
};

void cat_meow(void)
{
        puts("meow ...");
}

void dog_bark(void)
{
        puts("bark ...");
}

//typedef void (*call_t)(void);
//call_t p[ACT_NUMS] = {
void (*p[ACT_NUMS])(void) = {
        [ACT_BARK] = dog_bark,
        [ACT_MEOW] = cat_meow,
};

int main(void)
{
        for (int i = 0; i < sizeof(p)/sizeof(p[0]); i++) {
                if (p[i] == NULL)
                        continue;
                p[i]();
        }

        return 0;
}
{% endhighlight %}

{% highlight text %}
{% endhighlight %}
