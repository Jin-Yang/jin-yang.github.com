---
title: LeetCode 字符串计算
layout: post
comments: true
language: chinese
category: [linux,misc]
keywords:
description:
---

LeetCode 中有一系列的类似题目，这里仅介绍一个乘法运算。

<!-- more -->

## 简介

<!--
https://leetcode.com/problems/multiply-strings/
-->

因为字符串的长度不确定，不能直接使用整数乘法，否则可能会导致整型的溢出，所以，实际上类似于平时做乘法时的计算方法。

有几个需要注意的点：

1. 计算时从右到左，方便依次计算进位，只需要保证最低位在 `'0' ~ '9'` 范围内即可。
2. 当长度为 `m` `n` 的数值相乘时，结果的最大长度为 `m * n` ，最高位可能为 `'0'` 。
3. 计算过程中，通过 `char` 类型保存中间结果，完成计算后统一转换成字符串。

另外，对于乘 `0` 的计算，可以在入口判断，这样可以减少后面的异常处理。

## 代码

如果没有在开始判断 `"0"` ，那么可以将 `#if 0` 取消掉。

{% highlight c %}
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#define is_empty_string(s) ((s) == NULL || (s)[0] == '\0')

char *multiply(char *num1, char *num2)
{
        int i, j, p1, p2;
        int mul, sum, pos;
        int len1, len2, len;
        char *result, *ptr;

        if (is_empty_string(num1) || is_empty_string(num2))
                return NULL;

        len1 = strlen(num1);
        len2 = strlen(num2);

        if ((len1 == 1 && num1[0] == '0') || (len2 == 1 && num2[0] == '0'))
                return strdup("0");
        len = len1 + len2;

        result = calloc(1, len + 1);
        if (result == NULL)
                return NULL;

        for (j = len2 - 1;  j >= 0; j--) {
                for (i = len1 - 1;  i >= 0; i--) {
                        mul = (num1[i] - '0') * (num2[j] - '0');
                        p1 = i + j;
                        p2 = i + j + 1;

                        sum = mul + result[p2];
                        result[p2] = sum % 10;
                        result[p1] += sum / 10;
                }
        }

        // NOTE: maybe the first one is zero.
        for (i = 0; i < len; i++)
                if (result[i] != 0)
                        break;
        assert(i < len);

#if 0
        if (i == len) { // avoid by check num1 == "0" or num2 == "0" before.
                free(result);
                return strdup("0");
        }
#endif

        pos = i;
        for (; i < len; i++)
                result[i] = result[i] + '0';

        ptr = strdup(result + pos);
        free(result);

        return ptr;
}

int main(void)
{
        char *ret;

        //ret = multiply("123", "12"); // 1476
        //ret = multiply("123", "456"); // 56088
        ret = multiply("19", "9"); // 171
        //ret = multiply("123", "0");
        //ret = multiply("0", "123");
        printf("result %s\n", ret);
        free(ret);

        return 0;
}
{% endhighlight %}


{% highlight text %}
{% endhighlight %}
