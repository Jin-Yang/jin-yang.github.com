---
title: 正则表达式
layout: post
comments: true
language: chinese
category: [linux,program]
keywords: linux,c,cpp,c++,program,string
description: 简单介绍下 C 语言中与正则表达式相关的内容。
---

<!-- more -->

## 正则表达式

正则的语法起源于 Unix 系统，所以其基本语法规则基本一致，只是随着发展，逐步扩充了一些类型。

* Basic Regular Expression, BREs 基本正则表达式
* Extended Regular Expression, EREs 扩展正则表达式
* Perl Regular Expression, PREs Perl 正则表达式

### 常用工具

grep 支持 `BREs(默认-G)`、`EREs(-E)`、`PREs(-P)`，而 egrep 相当于使用了 `-E` 参数，会按照行处理关键字。sed 默认使用 BREs，可以通过 `-r` 参数使用 EREs，awk 使用 EREs 。

https://my.oschina.net/leejun2005/blog/76922

## 编程

C C++ 都提供了正则表达式，另外还有 C++ 中的 Boost、Google 提供的 RE2 库，其中 RE2 快速、安全、线程友好，可以作为 PCRE、Perl、Python 等回溯正则表达式引擎 (Backtracking Regular Expression Engine) 的替代。

暂时对于性能不太敏感，简单介绍 C 中正则表达式的使用方式。

{% highlight c %}
#include <stdio.h>
#include <regex.h>
#include <sys/types.h>

int main(void)
{
        int rc;
        regex_t regex;
        const char *strings = "abcef";
        const char *pattern = "[a-c][e-f]";

        rc = regcomp(&regex, pattern, REG_EXTENDED);
        if (rc != 0) {
                char buff[128];
                regerror(rc, &regex, buff, sizeof(buff));
                fprintf(stderr, "compile regular expression failed, %d:%s.\n",
                        rc, buff);
                return -1;
        }

        rc = regexec(&regex, strings, 0, NULL, 0);
        if (rc == REG_NOMATCH) {
                printf("not match\n");
        } else {
                printf("match\n");
        }

        regfree(&regex);

        return 0;
}
{% endhighlight %}

<!--
https://github.com/rust-leipzig/regex-performance
https://zherczeg.github.io/sljit/regex_perf.html
-->

{% highlight c %}
{% endhighlight %}
