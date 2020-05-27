---
title: Linux C 陷阱
layout: post
comments: true
language: chinese
category: [program,linux,misc]
keywords:
description:
---

一些 C 语言中比较容易犯错的知识点。

<!-- more -->

## while

如下代码输出内容是什么。

{% highlight c %}
#include <stdio.h>

int main(void)
{
        int i = 1;

        do {
                printf("%d\n",i);
                i++;
                if(i < 15)
                        continue;
        } while(0);

        return 0;
}
{% endhighlight %}

第一个预期是输出 `1~14` ，实际上只输出 `1` 。

在执行 continue 的时候，总会判断是否满足条件，也就是 `do ... while(cont)` 的 `while(cont)` 中，而非 `do` 语句处。

## switch

### 变量定义

{% highlight c %}
#include <stdio.h>

int main(void)
{
	int a = 1;

	switch(a) {
	int b = 20;
	case 1:
		printf("b1 is %d\n", b);
		break;
	default:
		printf("bd is %d\n", b);
		break;
	}

	return 0;
}
{% endhighlight %}

类似与其它的条件或者循环的分支，`switch` 语句同样会创建一个作用域，不过对与上述的变量无法赋值，只是做了声明，如果有调用函数实际上也不会调用执行。

所以，上述的输出一般是 `0` ，也可能会是其它的值。

另外，在 `case` 语句中也可以使用 `case 1...10` 类似的范围语句。

### 默认分支

{% highlight c %}
#include <stdio.h>

int main(void)
{
        int i = 0;

        switch (i) {
        default:
                puts("default");
        case 1:
                puts("case 1");
                break;
        case 2:
                puts("case 2");
        }

        return 0;
}
{% endhighlight %}

上述会输出 `default` 以及 `case 1` ，实际上在 C 中，会依次判断存在值是否满足，当不满足的时候跳转到 `default` 并按照代码中的顺序继续执行，直到遇到 `break` 或者语句结束。

可以将 `case` 和 `default` 视为 `goto` 跳转的标签地址。

## 指针

### 数组指针

通过 `int (*arr)[10]` 定义一个数组大小为 10 的数组指针，括号是必须写的，不然就是指针数组。

## 其它

### 累加传参

{% highlight c %}
#include <stdio.h>

int main(void)
{
	char chars[] = {1, 2, 3, 4, 5, 6, 7, 8};
	char *ptr = chars;

	return printf("%p %p\n", ptr, ptr++);
}
{% endhighlight %}

在编译时，会报 `warning: operation on ‘XXX’ may be undefined [-Wsequence-point]` 的警告信息，也就是在传参的过程中，没有明确 `ptr` 与 `ptr++` 那个会先执行，不同的编译器可能会有不同的规则，导致结果非预期。

## 参考

<!--
http://www.gowrikumar.com/c/index.php
-->

{% highlight text %}
{% endhighlight %}
