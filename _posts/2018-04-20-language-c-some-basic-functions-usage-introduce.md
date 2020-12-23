---
title: C 常用函数
layout: post
language: chinese
category: [program]
keywords: align
description:
---

C 语言中一些常用的基础函数，包括了 `qsort()` `bsearch()` `atexit()` `backtrace()` 等。

<!-- more -->

## bsearch

也就是一个针对已排序数组的二分查找实现，其声明如下。

{% highlight c %}
#include <stdlib.h>
void *bsearch(const void *key, const void *base,
              size_t nmemb, size_t size,
              int (*compar)(const void *, const void *));
{% endhighlight %}

各个参数的含义如下：

* `key` 要查找的元素指针；
* `base` 数组地址；
* `nmemb` 数组大小；
* `size` 数组中每个元素的大小；
* `compar` 比较函数。

返回的是数组中已经找的元素地址，没有找到则返回 `NULL`，如下是一个示例。

{% highlight c %}
#include <stdio.h>
#include <stdlib.h>

int cmp_int(const void *a, const void *b)
{
        return (*(int *)a - *(int *)b);
}

int main(void)
{
        int key = 32, *item;
        int values[] = {5, 20, 29, 32, 63};

        item = (int *)bsearch(&key, values, 5, sizeof(int), cmp_int);
        if (item == NULL)
                return -1;
        printf("found item %d(%p) at %p\n", *item, item, &values[3]);

        return(0);
}
{% endhighlight %}

<!--
其中在<search.h>头文件中提供了一系列的POSIX的搜索方案，包括了hash bst等方法
https://stackoverflow.com/questions/49377995/search-h-header-file-not-available
hsearch(3), lsearch(3), qsort(3), tsearch(3)
-->

## qsort

`qsort()` 会根据给出的比较函数进行快排，通过指针移动实现排序，时间复杂度为 `n*log(n)`，排序之后的结果仍然放在原数组中，不保证排序稳定性，如下是其声明。

{% highlight text %}
void qsort(void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *));
void qsort_r(void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *, void *), void *arg);
    base: 数组起始地址；
    nmemb: 数组元素个数；
    size: 每个元素的大小；
    compar: 函数指针，指向定义的比较函数，当elem1>elem2返回正数，此时不交换。
{% endhighlight %}

通常可以对整数、字符串、结构体进行排序，如下是常用示例。

{% highlight text %}
----- 对int类型数组排序
int num[100];
int cmp(const void *a , const void *b)
{
	return *(int *)a - *(int *)b;
}
qsort(num, sizeof(num)/sizeof(num[0]), sizeof(num[0]), cmp);

----- 对结构体进行排序
struct foobar {
	int data;
	char string[10];
} s[100]
int cmp_int(const void *a, const void *b) /* 按照data递增排序 */
{
	return (*(struct foobar *)a).data > (*(struct foobar *)b).data ? 1 : -1;
}
int cmp_string(const void *a, const void *b)
{
	return strcmp((*(struct foobar *)a).string, (*(struct foobar *)b).string);
}
qsort(num, sizeof(num)/sizeof(num[0]), sizeof(num[0]), cmp);
{% endhighlight %}

以及示例程序。

{% highlight c %}
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct foobar {
	int data;
	char string[100];
} array[10];

int cmp_int(const void * a, const void * b)
{
	return (*(struct foobar *)a).data - (*(struct foobar *)b).data;
}

int cmp_string(const void * a, const void * b)
{
	return strcmp((*(struct foobar *)a).string, (*(struct foobar *)b).string);
}

int main (void)
{
	int i, j;
	int array_size = sizeof(array)/sizeof(array[0]);
	printf("Array size %d\n", array_size);

	srand((int)time(0));
	for (i = 0; i < array_size; i++) {
		int r = rand() % 100;
		array[i].data = r;
		for (j = 0; j < r; j++)
			array[i].string[j] = 'A' + rand() % 26;
		array[i].string[r] = 0;
	}

	printf("Before sorting the list is: \n");
	for (i = 0 ; i < array_size; i++ )
		printf("%d ", array[i].data);
	puts("");
	for (i = 0 ; i < array_size; i++ )
		printf("%s\n", array[i].string);

	printf("\nAfter sorting the list is: \n");
	qsort(array, array_size, sizeof(struct foobar), cmp_int);
	for (i = 0 ; i < array_size; i++ )
		printf("%d ", array[i].data);
	puts("");

	printf("\nAfter sorting the list is: \n");
	qsort(array, array_size, sizeof(struct foobar), cmp_string);
	for (i = 0 ; i < array_size; i++ )
		printf("%s\n", array[i].string);

	return 0;
}
{% endhighlight %}

另外，在对字符串数组进行排序时，需要注意，如下的直接使用 `strcmp` 比较，最终会报错，需要添加一个比较函数。

这是因为 `qsort()` 在排序的时候使用的是数组，在比较函数中传入的是数组中每个元素的指针，所以，如果是字符串，那么就需要在比较函数中转换完成后才可以。

{% highlight c %}
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int cmp_string(const void * a, const void * b)
{
        return strcmp(*(char **)a, *(char **)b);
}

int main (void)
{
        const char *names[] = {
                "test",
                "foobar",
        };
        int i, size = sizeof(names)/sizeof(names[0]);

	//qsort(names, size, sizeof(names[0]), (int(*)(const void*, const void*))strcmp);
        qsort(names, size, sizeof(names[0]), cmp_string);
		
        for (i = 0; i < size; i++) 
                printf("[%02d] %s\n", i, names[i]);

        return 0;
}
{% endhighlight %}

<!--
https://www.felix021.com/blog/read.php?entryid=1951
-->

## backtrace

一般可以通过 gdb 的 bt 命令查看函数运行时堆栈，但是，有时为了分析程序的 BUG，可以在程序出错时打印出函数的调用堆栈。

在 glibc 头文件 `execinfo.h` 中声明了三个函数用于获取当前线程的函数调用堆栈。

{% highlight text %}
int backtrace(void **buffer,int size);
    用于获取当前线程的调用堆栈，获取的信息将会被存放在buffer中，它是一个指针列表。参数size用来指
    定buffer中可以保存多少个void*元素，该函数返回值是实际获取的指针个数，最大不超过size大小；

char **backtrace_symbols(void *const *buffer, int size);
    将从上述函数获取的信息转化为字符串数组，参数buffer应该是从backtrace()获取的指针数组，size是该
    数组中的元素个数，也就是backtrace()的返回值。
    函数返回值是一个指向字符串数组的指针，它的大小同buffer相同，每个字符串包含了一个相对于buffer中
    对应元素的可打印信息，包括函数名、函数的偏移地址和实际的返回地址。

void backtrace_symbols_fd(void *const *buffer, int size, int fd);
    与上述函数相同，只是将结果写入文件描述符为fd的文件中，每个函数对应一行。
{% endhighlight %}

注意，需要传递相应的符号给链接器以能支持函数名功能，比如，在使用 GNU ld 链接器的时需要传递 `-rdynamic` 参数，该参数用来通知链接器将所有符号添加到动态符号表中。

下面是 glibc 中的实例。

{% highlight c %}
#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define SIZE 100

void myfunc3(void)
{
    int j, nptrs;
    void *buffer[100];
    char **strings;

    nptrs = backtrace(buffer, SIZE);
    printf("backtrace() returned %d addresses\n", nptrs);

    /*
     * The call backtrace_symbols_fd(buffer, nptrs, STDOUT_FILENO)
     * would produce similar output to the following:
     */
    strings = backtrace_symbols(buffer, nptrs);
    if (strings == NULL) {
        perror("backtrace_symbols");
        exit(EXIT_FAILURE);
    }

    for (j = 0; j < nptrs; j++)
        printf("%s\n", strings[j]);

    free(strings);
}

static void myfunc2(void) /* "static" means don't export the symbol... */
{
    myfunc3();
}

void myfunc(int ncalls)
{
    if (ncalls > 1)
        myfunc(ncalls - 1);
    else
        myfunc2();
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "%s num-calls\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    myfunc(atoi(argv[1]));
    exit(EXIT_SUCCESS);
}
{% endhighlight %}

然后通过如下方式编译，执行。

{% highlight text %}
$ cc -rdynamic prog.c -o prog
$ ./prog 2
backtrace() returned 7 addresses
./prog(myfunc3+0x1f) [0x400a7c]
./prog() [0x400b11]
./prog(myfunc+0x25) [0x400b38]
./prog(myfunc+0x1e) [0x400b31]
./prog(main+0x59) [0x400b93]
/lib64/libc.so.6(__libc_start_main+0xf5) [0x7f727d449b35]
./prog() [0x400999]
{% endhighlight %}

还可以利用 backtrace 来定位段错误位置。

<!--
通常情况系，程序发生段错误时系统会发送SIGSEGV信号给程序，缺省处理是退出函数。我们可以使用 signal(SIGSEGV, &your_function);函数来接管SIGSEGV信号的处理，程序在发生段错误后，自动调用我们准备好的函数，从而在那个函数里来获取当前函数调用栈。

举例如下：

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <execinfo.h>
#include <signal.h>

void dump(int signo)
{
 void *buffer[30] = {0};
 size_t size;
 char **strings = NULL;
 size_t i = 0;

 size = backtrace(buffer, 30);
 fprintf(stdout, "Obtained %zd stack frames.nm\n", size);
 strings = backtrace_symbols(buffer, size);
 if (strings == NULL)
 {
  perror("backtrace_symbols.");
  exit(EXIT_FAILURE);
 }

 for (i = 0; i < size; i++)
 {
  fprintf(stdout, "%s\n", strings[i]);
 }
 free(strings);
 strings = NULL;
 exit(0);
}

void func_c()
{
 *((volatile char *)0x0) = 0x9999;
}

void func_b()
{
 func_c();
}

void func_a()
{
 func_b();
}

int main(int argc, const char *argv[])
{
 if (signal(SIGSEGV, dump) == SIG_ERR)
  perror("can't catch SIGSEGV");
 func_a();
 return 0;
}

编译程序：
gcc -g -rdynamic test.c -o test; ./test
输出如下：

Obtained6stackframes.nm
./backstrace_debug(dump+0x45)[0x80487c9]
[0x468400]
./backstrace_debug(func_b+0x8)[0x804888c]
./backstrace_debug(func_a+0x8)[0x8048896]
./backstrace_debug(main+0x33)[0x80488cb]
/lib/i386-linux-gnu/libc.so.6(__libc_start_main+0xf3)[0x129113]

接着：
objdump -d test > test.s
在test.s中搜索804888c如下：

8048884 <func_b>:
8048884: 55          push %ebp
8048885: 89 e5      mov %esp, %ebp
8048887: e8 eb ff ff ff      call 8048877 <func_c>
804888c: 5d            pop %ebp
804888d: c3            ret

其中80488c时调用（call 8048877）C函数后的地址，虽然并没有直接定位到C函数，通过汇编代码， 基本可以推出是C函数出问题了（pop指令不会导致段错误的）。
我们也可以通过addr2line来查看

addr2line 0x804888c -e backstrace_debug -f

输出：

func_b
/home/astrol/c/backstrace_debug.c:57


在Linux中如何利用backtrace信息解决问题
http://blog.csdn.net/jxgz_leo/article/details/53458366
-->

## atexit()

很多时候我们需要在程序退出的时候做一些诸如释放资源的操作，但程序退出的方式有很多种，比如 main() 函数运行结束、在程序的某个地方用 exit() 结束程序、用户通过 Ctrl+C 或 Ctrl+break 操作来终止程序等等，因此需要有一种与程序退出方式无关的方法来进行程序退出时的必要处理。

方法就是用 atexit() 函数来注册程序正常终止时要被调用的函数。

{% highlight c %}
#include <stdlib.h>
int atexit(void(*func)(void));
{% endhighlight %}

成功时返回零，失败时返回非零。

在一个程序中至少可以用 atexit() 注册 32 个处理函数，依赖于编译器。这些处理函数的调用顺序与其注册的顺序相反，也即最先注册的最后调用，最后注册的最先调用。

{% highlight c %}
void fnExit1 (void) { puts ("Exit function 1."); }
void fnExit2 (void) { puts ("Exit function 2."); }

int main ()
{
    atexit (fnExit1);
    atexit (fnExit2);
    puts ("Main function.");
    return 0;
}
{% endhighlight %}




{% highlight text %}
{% endhighlight %}
