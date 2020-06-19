---
title: LeetCode 计算器合集
layout: post
comments: true
language: chinese
category: [linux,misc]
keywords:
description:
---

在 LeetCode 中，由三道与计算器相关的题目，当然，相比真正的计算器，其规则都比较简单些，所以也无需词法语法解析之类的工具。

这里简单介绍其解法。

<!-- more -->

## 简介

其中，题目分别对应了 [Basic Calculator](https://leetcode.com/problems/basic-calculator/)、[Basic Calculator II](https://leetcode.com/problems/basic-calculator-ii/)、[Basic Calculator III](https://leetcode.com/problems/basic-calculator-iii/) 三道。最终是实现一个包含四则运算、括号、空格的整型运算，三道题主要考察内容：

1. 加减法以及括号；
2. 只有四则运算；
3. 四则运算以及括号。

在实现时需要注意几个点：

1. 括号的优先级最高，然后乘除法，最后是加减；
2. 除法需要取整，例如 `5/2=2` `-5/2=-2`；
3. 数字有多位，括号可能会嵌套；

另外，可以假设输入合法 (不会出现只有一半括号)，而且不会出现整型溢出、除 0 的异常情况，所以，可以直接省略一些常见的异常处理。

## 代码

通过 C 语言实现，直接上代码了，其中的栈可以参考之前的实现介绍。

{% highlight c %}
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include "stack.h"

static int stack_sum_cnt(struct stack *s, int cnt)
{
        int i, ret, res = 0;

        for (i = 0; i < cnt; i++) {
                if (stack_pop(s, &ret) < 0)
                        break;
                res += ret;
        }

        return res;
}

static int stack_sum_all(struct stack *s)
{
        int ret, res = 0;

        while (stack_pop(s, &ret) >= 0)
                res += ret;
        return res;
}

int calculate(char *s) {
        char c;
        struct stack *stk, *sym;
        int i, num, ret, op, tmp, len;

        stk = stack_create(32);
        sym = stack_create(32);
        if (stk == NULL || sym == NULL) {
                stack_destroy(stk);
                return -1;
        }

        ret = 0;
        op = '+';
        num = 0;
        len = strlen(s);
        for (i = 0; i <= len; i++) {
                c = s[i];

                if (isdigit(c)) {
                        num = 10 * num - '0' + c;
                        continue;
                } else if (c == ' ') {
                        continue;
                } else if (c == '(') {
                        printf("<----- %c %d\n", op, stack_get_size(stk));
                        stack_push(sym, op);
                        stack_push(sym, stack_get_size(stk));

                        op = '+';
                        num = 0;
                        continue;
                }

                printf("%02d  %c%d\n", i, op, num);
                switch (op) {
                case '+':
                        stack_push(stk, num);
                        break;
                case '-':
                        stack_push(stk, -num);
                        break;
                case '*':
                        stack_pop(stk, &tmp); // error *8
                        stack_push(stk, tmp * num); // unsupport 4*(-8)
                        break;
                case '/':
                        stack_pop(stk, &tmp); // error /8
                        stack_push(stk, tmp / num); // unsupport 4/(-8)
                        break;
                }
                num = 0;
                op = c;

                if (c == ')') {
                        stack_pop(sym, &tmp); // index
                        stack_pop(sym, &op);
                        printf("-----> %c %d", op, stack_get_size(stk) - tmp);
                        num = stack_sum_cnt(stk, stack_get_size(stk) - tmp);
                        printf(" > %d\n", num);
                }
        }

        ret = stack_sum_all(stk);
        stack_destroy(stk);

        return ret;
}

int main(void)
{
        // 1 + 1 = 2
        // 2 - 1 + 2 = 3
        // (1+(4+5+2)-3)+(6+8) = 23
        // 6 - 4 / 2 = 4
        // 2 * (5 + 5 * 2) / 3 + (6 / 2 + 8) = 21
#if 0
        printf("1 + 1 = %d(2)\n", calculate("1 + 1"));
        printf("2 - 1 + 2 = %d(3)\n", calculate("2 - 1 + 2"));
        printf("(1+(4+5+2)-3)+(6+8) = %d(23)\n", calculate("(1+(4+5+2)-3)+(6+8)"));
        printf("6 - 4 / 2 = %d(4)\n", calculate("6 - 4 / 2"));
        printf("2 * (5 + 5 * 2) / 3 + (6 / 2 + 8) = %d(21)\n", calculate("2 * (5 + 5 * 2) / 3 + (6 / 2 + 8)"));
#endif
        printf("(2+6* 3+5- (3*14/7+2)*5)+3 = %d(-12)\n", calculate("(2+6* 3+5- (3*14/7+2)*5)+3"));

        return 0;
}
{% endhighlight %}

其中的 `printf()` 作为调试使用，可以直接注释掉。

大部分的实现都会借助栈来保存中间的数据，但是当遇到了 `(` 一般会通过回归调用实现，也就是说，嵌套的越多函数调用栈也会越大，极端情况可能会导致栈溢出。

所以，这里在实现时借助了另外一个栈保存辅助信息，主要是上次的符号以及两个括号之间的在 `stk` 栈中的偏移。而对于 `*` 和 `/` 来说，可以直接通过 `stack_pop()` 进行转换。

{% highlight text %}
{% endhighlight %}
