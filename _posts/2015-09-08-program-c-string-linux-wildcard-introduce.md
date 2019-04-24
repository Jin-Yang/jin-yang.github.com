---
title: C 语言通配符
layout: post
comments: true
language: chinese
category: [linux,program]
keywords: linux,c,cpp,c++,program,string
description: 简单介绍下 C 语言中与字符串操作相关的函数。
---

简单介绍下 C 语言中与字符串操作相关的函数。

<!-- more -->

## 简介

当 shell 在参数中遇到了通配符时，会尝试将其当作路径或文件名去在磁盘上搜寻可能的匹配：若符合要求的匹配存在，则进行代换(路径扩展)；否则就将该通配符作为一个普通字符传递给命令，然后再由命令进行处理。

也就是说，通配符是由 shell 处理的，在传递给可执行程序之前已经进行了处理，实际上就是一种 shell 实现的路径扩展功能。

{% highlight text %}
*                       匹配0或多个字符             a*b  ab a012b aabcd
?                       匹配任意一个字符            a?b  abb acb a0b
[list]                  匹配list中的任意单一字符    a[xyz]b  axb ayb azb
[!list]                 匹配除list中的任意单一字符  a[!0-9]b  axb abb a-b
[c1-c2]                 匹配c1到c2中的任意单一字符  a[0-9]b   a0b a9b
{string1,string2,...}   匹配sring1或string2(或更多)其一字符串
{% endhighlight %}

另外，需要注意 shell 通配符和正则表达式之间的区别。

## fnmatch()

就是判断字符串是不是符合 pattern 所指的结构，这里的 pattern 是 `shell wildcard pattern`，其中部分匹配行为可以通过 flags 配置，详见 `man 3 fnmatch` 。

<!-- int fnmatch(const char *pattern, const char *string, int flags); -->

{% highlight c %}
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <fnmatch.h>
#include <sys/types.h>

int main(void)
{
	int rc;
	DIR *dir;
	struct dirent *entry;
	const char *pattern = "*.log";

	dir = opendir("/tmp");
	if (dir == NULL) {
		perror("opendir()");
		exit(EXIT_FAILURE);
	}
	while ((entry = readdir(dir)) != NULL) { // 逐个获取文件夹中文件
		rc = fnmatch(pattern, entry->d_name, FNM_PATHNAME | FNM_PERIOD);
		if (rc == 0) {         // 符合pattern的结构
			printf("match %s\n", entry->d_name);
		} else if (rc == FNM_NOMATCH){
			continue ;
		} else {
			printf("error %d file=%s\n", rc, entry->d_name);
		}
	}
	closedir(dir);
	return 0;
}
{% endhighlight %}

### wordexp()

按照 `Shell-Style Word Expansion` 扩展将输入字符串拆分，返回的格式为 `wordexp_t` 变量，其中包括了三个变量，两个比较重要的是：A) `we_wordc` 成员数；B) `we_wodv` 数组。

注意，在解析时会动态分配内存，所以在执行完 `wordexp()` 后，需要执行 `wordfree()`；另外，如果遇到内存不足会返回 `WRDE_NOSPACE` 错误，此时可能已经分配了部分地址，所以仍需要执行 `wordfree()` 。

1) 按照空格解析；2) 正则表达式；3) 环境变量。

{% highlight c %}
#include <stdio.h>
#include <stdlib.h>
#include <wordexp.h>

int main(void)
{
	int i, ret;
	wordexp_t p;

	ret = wordexp("foo bar $SHELL *[0-9].c *.c", &p, 0);
	if (ret == WRDE_NOSPACE) {
		wordfree(&p);
		exit(EXIT_FAILURE);
	} else if (ret != 0) {
		exit(EXIT_FAILURE);
	}
	for (i = 0; i < p.we_wordc; i++)
		printf("%s\n", p.we_wordv[i]);
	wordfree(&p);

	return 0;
}
{% endhighlight %}

<!-- http://www.gnu.org/software/libc/manual/html_node/Word-Expansion.html -->

### qsort()

只用于数组的排序，对于链表等无效。

{% highlight text %}
void qsort(void *base, size_t nitems, size_t size, int (*compar)(const void *, const void*));
    base  : 数组的基地址
    nitems: 数组包含的元素；
    size  : 每个元素的大小；
    compar: 比较函数；
{% endhighlight %}

示例程序如下。

{% highlight c %}
#include <stdio.h>
#include <stdlib.h>

int cmpfunc(const void *a, const void *b)
{
	return (*(int*)a - *(int*)b);
}

int main(void)
{
	int n;
	int values[] = { 88, 56, 100, 2, 25 };

	printf("Before sorting the list is: \n");
	for (n = 0 ; n < 5; n++)
		printf("%d ", values[n]);
	putchar('\n');
	qsort(values, 5, sizeof(int), cmpfunc);
	printf("After sorting the list is: \n");
	for( n = 0 ; n < 5; n++ )
		printf("%d ", values[n]);
	putchar('\n');

	return(0);
}
{% endhighlight %}

## 最佳实践

如果只想匹配一些通配符文件，那么就可以将 flag 设置为 `WRDE_NOCMD | WRDE_UNDEF` 以防止一些非法的入参。通过 `wordexp()` 过滤通配符文件，然后可以通过 `qsort()` 进行排序，示例如下。

{% highlight c %}
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <wordexp.h>
#include <sys/stat.h>

int main(void)
{
	int rc, i;
	wordexp_t we;
	struct stat statinfo;
	const char *pattern = "/tmp/*.log", *path;

	rc = wordexp(pattern, &we, WRDE_NOCMD | WRDE_UNDEF);
	if (rc != 0) {
		fprintf(stderr, "wordexp(%s) failed, rc %d.\n", pattern, rc);
		exit(EXIT_FAILURE);
	}
	qsort((void *)we.we_wordv, we.we_wordc, sizeof(*we.we_wordv), (int(*)(const void*, const void*))strcmp);

	for (i = 0; i < (int)we.we_wordc; i++) {
		path = we.we_wordv[i];
		if (stat(path, &statinfo) != 0) {
			fprintf(stderr, "stat(%s) failed, %s.\n", path, strerror(errno));
			continue;
		}

		if (S_ISREG(statinfo.st_mode)) {
			fprintf(stdout, "regular file: %s\n", path);
		} else if (S_ISDIR(statinfo.st_mode)) {
			fprintf(stdout, "directory: %s\n", path);
		}
	}
	wordfree(&we);

	return 0;
}
{% endhighlight %}

{% highlight c %}
{% endhighlight %}
