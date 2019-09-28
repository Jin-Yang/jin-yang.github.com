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

## Tip #1

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

## Tips #2

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

所以，上述的输出一般是 0 ，也可能会是其它的值。

另外，在 `case` 语句中也可以使用 `case 1...10` 类似的范围语句。

## 参考

<!--
http://www.gowrikumar.com/c/index.php
-->

{% highlight text %}
{% endhighlight %}
