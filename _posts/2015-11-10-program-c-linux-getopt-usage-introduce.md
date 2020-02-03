---
title: C 参数解析
layout: post
comments: true
language: chinese
category: [program,linux]
keywords: linux,program
description:
---

在 glibc 中提供了一个通用的参数解析库，包括了短参以及长参的使用。

<!-- more -->

## 短参

通过 `getopt()` 函数实现，可以通过循环调用解析参数，当没有多余选项时，会直接返回 `-1` 。

### 全局变量

在 `getopt()` 函数的实现中，引入了很多全局变量。

#### optind

`int optind` 标识在 `argv` 中将要检查的下个参数，初始化为 1，如果多次调用 `getopt()` 或者需要重头开始解析，那么就需要重新将 `optind` 配置为 1 。

#### optarg

`char *optarg` 当使用带参数的选项时，会将 `argv[]` 中的指针指向该变量。

通过 `:` 标识选项后有一个参数，可以通过 `-c color` 或者 `-ccolor` 使用，注意，一定不要使用 `-c=color` 方式，此时返回的是 `=color` 。

#### opterr optopt

当遇到不支持的选项时，会在 `stderr` 中打印错误信息 `invalid option -- 'x'` (可以通过将 `opterr` 设置为 0 不打印)，同时将选项保存在 optopt 中，并返回 `?` 。

### 示例

{% highlight c %}
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdbool.h>

int main(int argc, char **argv)
{
        int option, delay = 0;

        opterr = 0;
        while ((option = getopt(argc, argv, "abc:d::")) != -1) {
                switch (option) {
                case 'a':
                        fprintf(stdout, "option -a\n");
                        break;
                case 'b':
                        fprintf(stdout, "option -b\n");
                        break;
                case 'c':
                        fprintf(stdout, "option -c %s\n", optarg);
                        break;
                case 'd':
                        delay = optarg ? atoi(optarg) : 1;
                        fprintf(stdout, "option -d %d\n", delay);
                        break;
                case '?':
                        fprintf(stderr, "unsupport options '%c'\n", optopt);
                        exit(EXIT_FAILURE);
                }
        }

        /* Print remaining arguments. */
        for (; optind < argc; optind++)
                printf("%s\n", argv[optind]);
        return 0;
}
{% endhighlight %}

### 注意事项

只能通过 `-` 标识选项，对于 `getopt()` 来说，不能使用 `--` 否则会被认为是 `-` 选项。

{% highlight text %}
--a               报错，无法识别 `-` 选项
-a opt -b         可以正确识别-a和-b选项，并剩余opt参数
-c color -ccolor  带有值的参数
{% endhighlight %}

## 长参

`getopt_long()` 与 `getopt()` 类似，可以通过 `--` 指定长参数，同时通过函数入参可以指定短参数，如果想只支持长参数，那么需要将 `optstring` 设置为 `""` 而非 `NULL` 。

可以使用 `--args=param` 或者 `--args param` 指定参数，在解释参数时，需要初始化如下的数组。

{% highlight text %}
struct option {
	const char *name;  // 选项名称
	int has_arg; // no_argument/0 required_argument/1 optional_argument/2
	int *flag; // 可以是NULL或者指针
	int val;   // 根据flag返回
};
{% endhighlight %}

根据配置中的 `flag` 和 `val` 决定了参数的解析方式，

#### flag != NULL

会将 `val` 中的值赋值到 `flag` 指针所指向的变量，而 `getopt_long()` 函数会返回 0 ，比较适合整数类型的设置。

#### flag == NULL

在函数 `getopt_long()` 中会返回 `val` 中指定的值，为了方便处理，最好兼容 `optstring` 中指定的值。

### 示例

{% highlight c %}
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

int main(int argc, char **argv)
{
        int c, idx, verbose = 0;
        struct option options[] = {
                {"verbose", no_argument,       &verbose, 1  },
                {"add",     no_argument,       NULL,     'a'},
                {"change",  required_argument, NULL,     'c'},
                {"delete",  required_argument, NULL,     0  },
                {0, 0, 0, 0}
        };

        while (1) {
                idx = 0;
                c = getopt_long(argc, argv, "ac:d:", options, &idx);
                if (c == -1) /* Detect the end of the options. */
                        break;
                switch(c) {
                case 0:
                        /* If this option set a flag, do nothing else now. */
                        if (options[idx].flag != NULL)
                                break;
                        printf("option '%s'", options[idx].name);
                        if (optarg)
                                printf(" with arg '%s'", optarg);
                        printf("\n");
                        break;
                case 'a':
                        fprintf(stdout, "option -a\n");
                        break;
                case 'c':
                        printf ("option -c with value '%s'\n", optarg);
                        break;
                case '?':
                        break;
                default:
                        exit(EXIT_FAILURE);
                }
        }

        /* Print remaining arguments (not options). */
        for (; optind < argc; optind++)
                printf("%s\n", argv[optind]);

        return 0;
}
{% endhighlight %}


{% highlight text %}
{% endhighlight %}
