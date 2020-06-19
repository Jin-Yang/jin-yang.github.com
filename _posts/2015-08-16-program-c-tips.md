---
title: C 语言的奇技淫巧
layout: post
comments: true
language: chinese
category: [linux,program]
keywords: linux,c,cpp,c++,program
description: 整理下 C 语言中常用的技巧。变长数组。
---

整理下 C 语言中常用的技巧。

<!-- more -->

## 缓冲类型

流分为了文本流和二进制流，在 Linux 中两者没有区别，而 Windows 会做区分。

基于流的操作最终会调用 `read()/write()` 函数进行 IO 操作，为了提高程序的运行效率，流对象通常会提供缓冲区，以减少调用系统 IO 库函数的次数。

通常提供如下的三种缓冲方式：

1. 全缓冲。在缓冲区满了之后才调用系统 IO 函数，例如磁盘文件。
2. 行缓冲。直到遇到换行符 `'\n'` 或者缓冲区满时才调用系统 IO 库函数，例如标准输出。
3. 无缓冲。无缓冲区，数据会立即读入或者输出到外存文件和设备上。例如标准错误输出，可以保证及时将错误反馈给客户。

对于标准输入、输出可以通过如下的程序进行测试。

{% highlight c %}
#include <stdio.h>

void print_info(FILE *f)
{
	if(f->_flags & _IO_UNBUFFERED)
		printf("unbuffered\n");
	else if(f->_flags & _IO_LINE_BUF)
		printf("line-buffered\n");
	else
		printf("fully-buffered\n");
	printf("    buffer size: %ld\n", f->_IO_buf_end - f->_IO_buf_base);
	printf("    discriptor : %d\n\n", fileno(f));
}

int main(void)
{
	printf("stdin  is ");
	print_info(stdin);

	printf("stdout is ");
	print_info(stdout);

	printf("stderr is ");
	print_info(stderr);

	return 0;
}
{% endhighlight %}

也可以通过命令 `./foobar <main.c 1>out.txt 2>err.txt` 测试重定向之后的属性。

可以通过 `setvbuf(stdout, NULL, _IOLBF, 0);` 设置为行缓冲模式。


## 变长数组

实际编程中，经常会使用变长数组，但是 C 语言并不支持变长的数组，可以使用结构体实现。

类似如下的结构体，其中 value 成员变量不占用内存空间，也可以使用 ```char value[]``` ，但是不要使用 ``` char *value```，该变量会占用指针对应的空间。

常见的操作示例如下。

{% highlight c %}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct foobar {
  int len;
  char value[0];
} foobar_t;

int main(int argc, char *argv[])
{
    // 两者占用的存储空间相同，也就是value不占用空间
    printf("%li %li\n", sizeof(int), sizeof(foobar_t));

    // 初始化
    int *values = (int *)malloc(10*sizeof(int)), i, j, *ptr;
    for (i = 0; i < 10; i++)
      values[i] = 10*i;
    for (i = 0; i < 10; i++)
      printf(" %i", values[i]);
    printf("\n");

    // 针对单个结构体的操作
    foobar_t *buff = (foobar_t *)malloc(sizeof(foobar_t) + 10*sizeof(int));
    buff->len = 10;
    memcpy(buff->value, values, 10*sizeof(int));
    ptr = (int *)buff->value;

    printf("length: %i, vlaues:", buff->len);
    for (i = 0; i < 10; i++)
      printf(" %i", ptr[i]);
    printf("\n");
    free(buff);

    // 针对数组的操作
    #define FOOBAR_T_SIZE(elements) (sizeof(foobar_t) + sizeof(int) * (elements))
    foobar_t **buf = (foobar_t **)malloc(6*FOOBAR_T_SIZE(10));
    foobar_t *ptr_buf;
    for (i = 0; i < 6; i++) {
      ptr_buf = (foobar_t*)((char *)buf + i*FOOBAR_T_SIZE(10));
      ptr_buf->len = i;
      memcpy(ptr_buf->value, values, 10*sizeof(int));

      ptr = (int *)ptr_buf->value;
      printf("length: %i, vlaues:", ptr_buf->len);
      for (j = 0; j < 10; j++)
        printf(" %i", ptr[j]);
      printf("\n");
    }
    free(values);
    free(buf);

    return 0;
}
{% endhighlight %}

## 变参传递

头文件 `stdarg.h` 中对相关的宏进行了定义，其基本内容如下所示：

{% highlight c %}
typedef char * va_list;

#define _INTSIZEOF(n)       ((sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1))
#define va_start(arg_ptr,v) (arg_ptr = (va_list)&v + _INTSIZEOF(v))
#define va_arg(arg_ptr,t)   (*(t *)((arg_ptr += _INTSIZEOF(t)) - _INTSIZEOF(t)))
#define va_end(arg_ptr)     (arg_ptr = (va_list)0)
{% endhighlight %}

<!--
_INTSIZEOF(n)：我们可以经过测试得到，如果在上面的测试程序中用一个double类型（长度为8byte）和一个 long double类型（长度为10byte）做可变参数。我们发现double类型占了8byte,而long double占了12byte，即都是4的整数倍。这里是因为对齐，即对Intel80x86 机器来说就是要求每个变量的地址都是sizeof(int)的倍数。char类型的参数只占了1byte，但是它后面的参数因为对齐的关系只能跳过3byte存储，而那3byte也就浪费掉了。<br><br>

<font color=blue><strong>va_start(arg_ptr,v)</strong></font>：使参数列表指针arg_ptr指向函数参数列表中的第一个可选参数。v是位于第一个可选参数之前的固定参数，如果有一函数的声明是void va_test(char a, char b, char c, …)，则它的固定参数依次是a,b,c，最后一个固定参数v为c，因此就是va_start(arg_ptr, c)。<br><br>

<font color=blue><strong>va_arg(arg_ptr, t)</strong></font>：返回参数列表中指针arg_ptr所指的参数，返回类型为t，并使指针arg_ptr指向参数列表中下一个参数。首先计算(arg_ptr += _INTSIZEOF(t))，此时arg_ptr将指向下一个参数；((arg_ptr += _INTSIZEOF(t)) - _INTSIZEOF(t))等价于arg_ptr，通过(t *)将arg_ptr转换为t类型的指针，并通过*取其值。<br><br>

<font color=blue><strong>va_end(arg_ptr)</strong></font>：使ap不再指向堆栈,而是跟NULL一样。<br><br>
-->

示例如下图所示：

{% highlight c %}
#include <stdarg.h>
#include <stdio.h>

int max( int num,...)
{
    int m = -0x7FFFFFFF; /* 32系统中最小的整数 */
    int i = 0, t = 0;
    va_list ap;
    va_start( ap, num);
    for( i = 0; i < num; i++) {
         t = va_arg( ap, int);
         if( t > m)
            m = t;
    }
    va_end(ap);
    return m;
}

int main(int argc,char *argv[])
{
  int n, m;

  n = max( 5, 5, 6, 3, 8, 5);
  m = max( 7, 5, 1, 9, 8, 5, 7, 0);

  printf("%d\t%d\n",n,m);

  return 0;
}
{% endhighlight %}

其中函数传参是通过栈传递，保存时从右至左依次入栈，以函数 `void func(int x, float y, char z)` 为例，调用该函数时 z、y、x 依次入栈，理论上来说，只要知道任意一个变量地址，以及所有变量的类型，那么就可以通过指针移位获取到所有的输入变量。

`va_list` 是一个字符指针，可以理解为指向当前参数的一个指针，取参必须通过这个指针进行。

在使用时，其步骤如下：
1. 调用之前定义一个 va_list 类型的变量，一般变量名为 ap 。
 2. 通过 va_start(ap, first) 初始化 ap ，指向可变参数列表中的第一个参数，其中 first 就是 ... 之前的那个参数。
 3. 接着调用 va_arg(ap, type) 依次获取参数，其中第二个参数为获取参数的类型，该宏会返回指定类型的值，并指向下一个变量。
 4. 最后关闭定义的变量，实际上就是将指针赋值为 NULL 。

其中的使用关键是如何获取变量的类型，通常有两种方法：A) 提前约定好，如上面的示例；B) 通过入参判断，如 printf() 。
 
另外，常见的用法还有获取省略号指定的参数，例如：

{% highlight c %}
void foobar(char *str, size_t size, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	_vsnprintf(str, size, fmt, ap);
	va_end(ap);
}
{% endhighlight %}

假设，在调用上述的函数时，如果在 `_vsnprintf()` 中会再调用类似的函数，那么可以通过 `va_list args; va_copy(args, ap);` 复制一份。

{% highlight text %}
va_list args;
va_copy(args, ap);
some_other_foobar(str, size, fmt, args);
{% endhighlight %}

## 调试

当调试时定义 DEBUG 输出信息，通常有如下的几种方式。

{% highlight c %}
// 常用格式
#ifdef DEBUG
	#define debug(fmt, args...) printf("debug: " fmt "\n", ##args) // OR
	#define debug(fmt, ...) printf("debug: " fmt "\n", ## __VA_ARGS__);
#else
	#define debug(fmt,args...)
#endif

// 输出文件名、函数名、行数
#ifdef DEBUG
	#define debug(fmt, args...) printf("%s, %s, %d: " fmt , __FILE__, __FUNCTION__, __LINE__, ##args)
#else
	#define debug(fmt, args...)
#endif

// 输出信息含有彩色
#ifdef DEBUG
   #define debug(fmt,args...)    \
      do{                        \
         printf("\033[32;40m");  \
         printf(fmt, ##args);    \
         printf("\033[0m");      \
      } while(0);
#else
   #define debug(fmt,args...)
#endif
{% endhighlight %}

另外，也可以通过如下方式判断支持可变参数的格式。

{% highlight c %}
#if defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L
	#define _debug(...) do { printf("debug: " __VA_ARGS__); putchar('\n'); } while(0);
	#define _warn(...)  do { printf("warn : " __VA_ARGS__); putchar('\n'); } while(0);
	#define _error(...) do { printf("error: " __VA_ARGS__); putchar('\n'); } while(0);
#elif defined __GNUC__
	#define _debug(fmt, args...)  do { printf("debug: " fmt, ## args); putchar('\n'); } while(0);
	#define _warn(fmt, args...)   do { printf("warn: "  fmt, ## args); putchar('\n'); } while(0);
	#define _error(fmt, args...)  do { printf("error: " fmt, ## args); putchar('\n'); } while(0);
#endif
{% endhighlight %}

如果通过 gcc 编译时，没有使用 `-std=` 参数就使用后者的宏定义，否则是前者。

### 变参

在最早的 C 实现标准中，可变参数只能出现在真正的函数中，而不能使用在宏中，直到 C99 版本中允许定义可变参数宏，类似如下。

{% highlight c %}
#define debug(...) printf(__VA_ARGS__)
{% endhighlight %}

缺省号代表一个可以变化的参数表，然后使用保留名 `__VA_ARGS__` 把参数传递给宏，当宏的调用展开时，实际的参数就传递给对应的 `printf()`了。

在 GUNC 中的扩展，其形式如下。

{% highlight c %}
#define log_debug(fmt, args...)  do { printf("gnu debug: " fmt, ## args); putchar('\n'); } while(0);
{% endhighlight %}

这里的 `##` 用来将前面多余的 `,` 去掉，当可变参数的个数为 `0` 时，实际上在宏扩展时删掉了 `,` ，而多个参数时是有的。

另外一个比较有意思的是，可以多次嵌套，例如。

{% highlight c %}
#include <stdio.h>

#define log_it(...)    do { printf("(log) " __VA_ARGS__); putchar('\n'); } while(0);
#define log_debug(...) do { log_it("debug==> " __VA_ARGS__); } while(0);

int main(void)
{
        log_debug("your id %d", 1);

        return 0;
}
{% endhighlight %}

## 参数解析

`getopt()` 是采用缓冲机制，因此对于多线程编程是 **不安全** 的。

{% highlight text %}
#include <unistd.h>
// 选项的参数指针
extern char *optarg;

// 初值为1, 下次调用时，从optind位置开始检测，通过agrv[optind]可以得到下一个参数，从而可以自行检测
extern int optind;

// 对于不能识别的参数将输出错误信息，可以将opterr设置为0，从而阻止向stderr输出错误信息
extern int opterr;

// 如果选项字符不再optstring中则返回':'或'?'，并将字符保存在optopt中
extern int optopt;

int getopt(int argc, char * const argv[],const char *optstring);
描述：
  该函数处理"-"起始的参数，有些平台可能支持"--"
参数：
  argc、argv分别为main()传入的参数；其中optstring可以有如下的选项:
    单个字符，表示选项。
    单个字符后接一个冒号，表示该选项后必须跟一个参数，参数紧跟在选项后或者以空格隔开，该参数的指针赋给optarg。
    单个字符后跟两个冒号，表示该选项后必须跟一个参数，参数必须紧跟在选项后不能以空格隔开，否则optarg指向为NULL，
        该参数的指针赋给optarg，这个特性是GNU的扩展。
{% endhighlight %}

如 `optstring="ab:c::d::"` ，命令行为 `getopt.exe -a -b host -ckeke -d haha`，在这个命令行参数中，`-a` `-b` 和 `-c` 是选项元素，去掉 `'-'`，a b c 就是选项。

host 是 b 的参数，keke 是 c 的参数，但 haha 并不是 d 的参数，因为它们中间有空格隔开。

**注意**：如果 optstring 中的字符串以 `'+'` 加号开头或者环境变量 `POSIXLY_CORRE` 被设置，那么一遇到不包含选项的命令行参数，getopt 就会停止，返回 -1；命令参数中的 `"--"` 用来强制终止扫描。

默认情况下 getopt 会重新排列命令行参数的顺序，所以到最后所有不包含选项的命令行参数都排到最后，如 `getopt -a ima -b host -ckeke -d haha`，都最后命令行参数的顺序是 `-a -b host -ckeke -d ima haha` 。

如果检测到设置的参数项，则返回参数项；如果检测完成则返回 -1；如果有不能识别的参数则将该参数保存在 optopt 中，输出错误信息到 stderr，如果 optstring 以 `':'` 开头则返回 `':'` 否则返回 `'?'`。

源码可以参考 [github getopt.c]({{ site.example_repository }}/c_cpp/c/getopt.c) 。

### 长选项

{% highlight text %}
#include <getopt.h>
int getopt_long(int argc, char * const argv[],
    const char *optstring, const struct option *longopts, int *longindex);
int getopt_long_only(int argc, char * const argv[],
    const char *optstring, const struct option *longopts, int *longindex);

描述：
  该函数与getopt函数类似，不过可以接收长选项(通常以"--"开头)，如果只接收长选项则optstring应该设置为""，而非NULL。
{% endhighlight %}

<!--
<font color=blue size=3><strong>参数：</strong></font><br>
    前3个参数与getopt类似，struct option定义在&lt;getopt.h&gt;中，<br>
<pre>
struct option {
    const char *name;   //name表示的是长参数名
    int has_arg; // 有3个值，
                 // no_argument(或者是0)，表示该参数后面不跟参数值
                 // required_argument(或者是1),表示该参数后面一定要跟个参数值
                 // optional_argument(或者是2),表示该参数后面可以跟，也可以不跟参数值
    int *flag; // 用来决定，getopt_long()的返回值到底是什么。如果flag是null，
               // 则函数会返回与该项option匹配的val值；否则如果找到参数则返回0，
               // flag指向该option的val，如果没有找到则保持不变。
    int val; // 和flag联合决定返回值
}</pre>
    如果longindex没有设置为空则，longindex指向longopts<br><br>
-->

源码可以参考 [github getopt_long.c]({{ site.example_repository }}/c_cpp/c/getopt_long.c) 。

{% highlight text %}
$ getopt-long a.out -a -b -c -x foo --add --back  --check --extra=foo
{% endhighlight %}

`getopt_long_only()` 和 `getopt_long()` 类似，但是 `'-'` 和 `'--'` 均被认为是长选项，只有当 `'-'` 没有对应的选项时才会与相应的短选项匹配。







## 整型溢出

以 8-bits 的数据为例，unsigned 取值范围为 0~255，signed 的取值范围为 -128~127。在计算机中数据以补码（正数原码与补码相同，原码=除符号位的补码求反+1）的形式存在，且规定 0x80 为-128 。

### 无符号整数

对于无符号整数，当超过 255 后将会溢出，常见的是 Linux 内核中的 jiffies 变量，jiffies 以及相关的宏保存在 linux/jiffies.h 中，如果 a 发生在 b 之后则返回真，即 a>b 返回真，无论是否有溢出。

{% highlight c %}
#define time_after(a,b)     \
    (typecheck(unsigned long, a) && \
     typecheck(unsigned long, b) && \
     ((long)((b) - (a)) < 0))
{% endhighlight %}

<!--
    下面以8-bit数据进行讲解，在 0x00~0x7F 范围内，表示 0~127；在 0x80~0xFF 范围内表示 -128~-1，对于可能出现的数据有四种情况：<ol><li>
        都位于0x00~0x7F<br>
        如果a发生在b之后，则a > b。(char)b - (char)a < 0，100 - 125 = -25 < 0。</li><br><li>

        都位于0x80~0xFF<br>
        如果a发生在b之后，则a > b。(char)b - (char)a < 0，150 - 180 = -106 - (-76) = -30 < 0。</li><br><li>

        b位于0x00~0x7F，a位于0x80~0xFF<br>
        如果a发生在b之后，则a > b。(char)b - (char)a < 0，100 - 150 = 100 - (-106) = 206 = -50 < 0。<br><strong>注意，此时在a-b<=128时有效。</strong></li><br><li>

        a位于0x00~0x7F，b位于0x80~0xFF<br>
    如果a发生在b之后，此时有溢出。(char)b - (char)a < 0，150 - 10 = -106 - 10 = -116 < 0。<br><strong>注意，此时在a-b<=128时有效。</strong></li></ol>

    <div style="padding: 10pt 0pt 10pt 0pt ;" align="right">
    <table frame="void" width="90%">
        <tbody><tr><td><b>Tips:</b><br><span style="color : #009000"><font size="-1">typecheck位于相同文件夹下的typecheck.h文件中，当两个参数不是同一个类型是将会产生警告，提醒用户注意，只是提醒。</font></span></td>
    <td><img src="src/info.png" width="80" heigh="80"></td></tr></tbody></table></div>
    </p>
-->



<!--
<br id="Error_handling"><br><br>
<h1>错误处理_OK</h1>
<p>
    需要包含的头文件及函数原型，<pre>
#include &lt;stdio.h&gt;
void perror(const char *s);

#include &lt;errno.h&gt;
const char *sys_errlist[];
int sys_nerr;  // 前两个变量是参考了BSD，glibc中保存在&lt;stdio.h&gt;
int errno;</pre>

    如果s不为NULL且*s != '\0'则输出s并加上": "+错误信息+换行，否则只输出错误信息+换行。通常s应该为出错的函数名称，此函数需要调用errno。如果函数调用错误后没有直接调用<tt>perror()</tt>，则为防止其他错误将其覆盖，需要保存errno。<br><br>


sys_errlist保存了所有的错误信息，errno(注意出现错误时进行了设置，但是正确调用时可能没有清除)为索引，最大的索引为<tt>(sys_nerr - 1)</tt>。当直接调用sys_errlist时可能错误信息还没有添加。
</p>

## errno

http://www.bo56.com/linux%E4%B8%ADc%E8%AF%AD%E8%A8%80errno%E7%9A%84%E4%BD%BF%E7%94%A8/
https://www.ibm.com/developerworks/cn/aix/library/au-errnovariable/

-->

## assert()

其作用是如果它的条件返回错误，则输出错误信息 (包括文件名，函数名等信息)，并终止程序执行，原型定义：

{% highlight c %}
#include <assert.h>
void assert(int expression);
{% endhighlight %}

如下是一个简单的示例。

{% highlight c %}
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    FILE *fp;

   fp = fopen( "test.txt", "w" ); // 不存在就创建一个同名文件
   assert( fp );                  // 所以这里不会出错
   fclose( fp );

    fp = fopen( "noexitfile.txt", "r" );  // 不存在就打开文件失败
    assert( fp );                         // 这里出错
    fclose( fp );                         // 程序不会执行到此处

    return 0;
}
{% endhighlight %}

当在 ```<assert.h>``` 之前定义 NDEBUG 时，assert 不会产生任何代码，否则会显示错误。

### 判断程序是否有 assert

在 glibc 中，会定义如下的内容：

{% highlight c %}
#define assert(e) ((e \
    ? ((void)0) \
    :__assert_fail(#e,__FILE__,__LINE__))
{% endhighlight %}

可以通过 nm 查看程序，判断是否存在 ```__assert_fail@@GLIBC_2.2.5``` ，如果存在该函数则说明未关闭 ```assert()``` 。

对于 autotool 可以通过如下的一种方式关闭：

1. 在 ```configure.ac``` 文件中添加 ```AC_HEADER_ASSERT``` ，然后如果关闭是添加 ```--disable-assert``` 参数，注意，一定要保证源码包含了 ```config.h``` 头文件；
2. 执行 configure 命令前设置环境变量，如 ```CPPFLAGS="CPPFLAGS=-DNDEBUG" ./configure```；
3. 也可以在 ```Makefile.am``` 中设置 ```AM_CPPFLAGS += -DNDEBUG``` 参数。


## 其它

### is not a symbolic link

正常情况下，类似库 ```libxerces-c-3.0.so``` 应该是个符号链接，而不是实体文件，对于这种情况只需要修改其为符号链接即可。

{% highlight text %}
# mv libxerces-c-3.0.so libxerces-c.so.3.0
# ln -s libxerces-c.so.3.0 libxerces-c-3.0.so
{% endhighlight %}

### 结构体初始化

对于 C 中结构体初始化可以通过如下设置。

{% highlight c %}
#include <stdio.h>

struct foobar {
        int foo;
        struct a {
                int type;
                int value;
        } *array;
        int length;
};

int main(int argc, char **argv)
{
        int i = 0;

        struct foobar f = {
                .foo = 1,
                .length = 3,
                .array = (struct a[]){
                        {.type = 1, .value = 2},
                        {.type = 1, .value = 3},
                        {.type = 1, .value = 3}
                }
        };

        for (i = 0; i < f.length; i++)
                printf(">>>> %d %d\n", i, f.array[i].type);

        return 0;
}
{% endhighlight %}

### 结构体地址定位

通过结构体可以将多种不同类型的对象聚合到一个对象中，编译器会按照成员列表顺序分配内存，不过由于内存对齐机制不同，导致不同架构有所区别，所以各个成员之间可能会有间隙，所以不能简单的通过成员类型所占的字长来推断其它成员或结构体对象的地址。

假设有如下的一个链表。

{% highlight text %}
typedef struct list_node {
	int ivar;
	char cvar;
	double dvar;
	struct list_node *next;
} list_node;
{% endhighlight %}

当已知一个变量的地址时，如何获取到某个成员的偏移量，Linux 内核中的实现如下。

{% highlight c %}
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
{% endhighlight %}

当知道了成员偏移量，那么就可以通过结构体成员的地址，反向求结构体的地址，如下。

{% highlight c %}
#define container_of(ptr, type, member) ({
	const typeof(((type *)0)->member ) *__mptr = (ptr);
	(type *)((char *)__mptr - offsetof(type,member));
	})
{% endhighlight %}

现在很多的动态语言是可以支持动态获取变量类型的，其中 GCC 提供了 `typeof` 关键字，所不同的是这个只在预编译时，最后实际转化为数据类型被编译器处理。基本用法是这样的：

{% highlight c %}
int a;
typeof(a)  b; // int b;
typeof(&a) c; // int* c;
{% endhighlight %}

如上的宏定义中， ptr 代表已知成员的地址，type 代表结构体的类型，member 代表已知的成员。


如下示例，如果 `foobar.hello` 是通过内存动态获取的，那么就不能通过 `container_of()` 实现，需要在 `struct hello` 中保存一个反向指针。

{% highlight c %}
#include <stdio.h>
#include <stdlib.h>

#define container_of(ptr, type, member)                         \
        (type *)((char *)(ptr) - (char *) &((type *)0)->member)

struct hello {
        int id;
};

struct foobar {
        char name[10];
        struct hello *hello;
};

int main(void)
{
        struct foobar *f;
        struct hello *h;
        f = malloc(sizeof(struct foobar));
        h = malloc(sizeof(struct hello));
        f->hello = h;

        printf("foobar=%p foobar.hello=%p\n", f, &(f->hello));

        /* failed */
        printf("container foobar=%p\n",
                        container_of(&h, struct foobar, hello));

        return 0;
}
{% endhighlight %}


### 指针参数修改

一个比较容易犯错的地方，愿意是在 `foobar()` 函数内修改 `main()` 中的 v 指向的变量，其中后者实际上是修改的本地栈中保存的临时版本。

{% highlight c %}
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct value_s {
  union {
    char *string;
  } value;
} value_t;

#if 1
void foobar ( value_t *t )
{
  char **v = &t->value.string;
  printf("foobar %p %s\n", *v, *v);
  *v = "yang";
  printf("foobar %p %s\n", *v, *v);
}
#else
void foobar ( value_t *t )
{
  value_t v = *t;
  printf("foobar %p %s\n", v.value.string, v.value.string);
  v.value.string = "yang";
  printf("foobar %p %s\n", v.value.string, v.value.string);
}
#endif

int main()
{
  value_t v;
  v.value.string = "jin";
  printf("       %p %s\n", v.value.string, v.value.string);
  foobar( &v );
  printf("       %p %s\n", v.value.string, v.value.string);

  return(0);
}
{% endhighlight %}

### 取固定大小的变量

Linux 每个数据类型的大小可以在 `sys/types.h` 中查看

{% highlight c %}
#include <sys/types.h>
typedef    int8_t            S8;
typedef    int16_t           S16;
typedef    int32_t           S32;
typedef    int64_t           S64;

typedef    u_int8_t          U8;
typedef    u_int16_t         U16;
typedef    u_int32_t         U32;
typedef    u_int64_t         U64;
{% endhighlight %}

### 环境变量

简单介绍下 C 中，如何获取以及设置环境变量。

其中设置环境变量方法包括了 `putenv()` 以及 `setenv()` 两种，前者必须是 `Key=Value` 这种格式，后者则以参数形式传递。

对于 `putenv()` 如果环境变量已经存在则替换，而 `setenv()` 则可以设置是否覆盖 。

{% highlight c %}
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
	char *p;

	if((p = getenv("USER")))
		printf("USER = %s\n", p);

	setenv("USER", "test", 1);
	printf("USER = %s\n", getenv("USER"));

	putenv("NAME=foobar");
	printf("NAME = %s\n", getenv("NAME"));

	unsetenv("USER");
	printf("USER = %s\n", getenv("USER"));
}
{% endhighlight %}


### 其它

#### 最大值

一般在 C 中有很多的长度限制，例如 `PATH_MAX` 路径长度、`NAME_MAX` 文件名称长度。

对于 CentOS 来说，一般使用时常见的头文件有：

{% highlight text %}
/usr/include/linux/limits.h
PATH_MAX、NAME_MAX、ARG_MAX etc.

/usr/include/bits/local_lim.h
HOST_NAME_MAX、TTY_NAME_MAX etc.

/usr/include/limits.h
INT_MAX、LONG_MAX etc.
{% endhighlight %}

#### FLT_RADIX

C 语言标准库 `float.h` 中的 `FLT_RADIX` 常数用于定义指数的基数，也就是以这个数为底的多少次方。

{% highlight text %}
FLT_RADIX 10     10 的多少次方
FLT_RADIX 2       2 的多少次方
{% endhighlight %}

例如：

{% highlight text %}
#define FLT_MAX_EXP 128
#define FLT_RADIX   2
{% endhighlight %}

意思是 float 型，最大指数是 128，它的底是 2，也就说最大指数是 2 的 128 方。

<!--
FLT 是 float 的 缩写。( DBL 是 double 的 缩写。)
-->

#### implicit declaration

按常规来讲，出现 `implicit declaration of function 'xxxx'` 是因为头文件未包含导致的！

这里是由于 `nanosleep()` 函数的报错，而实际上 `time.h` 头文件已经包含了，后来才发现原来是在 `Makefile` 中添加了 `-std=c99` 导致，可以通过 `-std=gnu99` 替换即可。

另外，不能定义 `-D_POSIX_SOURCE` 宏。

#### 多个值赋值

C 语言中如何一次将多个值进行赋值？

一般来说，对于基本的变量类型，使用比较多的是单个的赋值，实际上也可以一次赋值多个，只是需要点技巧。

简单来说就是使用结构体，示例如下。

{% highlight c %}
#include <stdio.h>

struct foobar {
        int count;
        const char *name;
};

int main(void)
{
        struct foobar foo = {
                .count = 0,
                .name = "foobar"
        }, bar;

        bar = foo;
        printf("bar.count = %d, bar.name = '%s'\n", bar.count, bar.name);
		
	return 0;
}
{% endhighlight %}

#### 科学计数

使用 C 语言中的科学计数法时，表示 10 的多少次幂，这样就不需要再查有多少个 0 了，示例如下：

{% highlight text %}
double a = 1e3;    // a = 1000
double b = 1e-3;   // b = 0.001
double c = 2.3e2;  // b = 230
double d = -1.3e2; // c = -130
{% endhighlight %}

需要注意，指数只能是整数，包括正负 0 ，不能将浮点数、变量等作为 e 的指数。

#### snprintf

标准库的 snprintf 不会在末尾添加 `\0` 终止符，返回的长度同样不含 `\0` ，正常应该做如下的处理。

{% highlight text %}
int len;
char buff[3];

len = snprintf(buff, sizeof(buff), "%d\n", 10);
assert(len > 0 && len < sizeof(buff));
if (len < 0 || len >= (int)sizeof(buff)) {
        printf("format failed or no enough buff, rc %d.\n", len);
		return -1;
}
{% endhighlight %}

## 参考

[Schemaless Benchmarking Suite](https://github.com/ludocode/schemaless-benchmarks)

{% highlight text %}
{% endhighlight %}
