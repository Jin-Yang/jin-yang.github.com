---
title: C 常用库
layout: post
comments: true
language: chinese
category: [misc]
keywords: hello world,示例,sample,markdown
description: 简单记录一下一些与 Markdown 相关的内容，包括了一些使用模版。
---


<!-- more -->

## JSON

一个比较简单的库。


----- 字符串解析
parser() 内存策略自动管理，返回非NULL时则认为成功，失败时内存由函数内部管理
parser_opts() 同parser()

----- 转换为字符串
print() 可以选择是否要格式化(方便阅读)输出
cJSON_Print() 只能打印256字节
cJSON_PrintBuffered() 也可以通过该函数指定缓存的长度
cJSON_PrintPreallocated() 已经申请了内存，然后作为参数传入

----- 动态申请
new_item() 申请结构体，并初始化为0
create_null() 申请结构体，设置type类型
create_true() 申请结构体，设置type类型
create_false() 申请结构体，设置type类型
create_bool() 申请结构体，设置type类型
create_object() 申请结构体，设置type类型
create_number() 申请结构体，设置type类型，由于历史原因会保存valuedouble和valueint中
create_string() 申请结构体，设置type类型，字符串复制到valuestring，同时会处理可能出现的内存失败
create_raw() 与 create_string() 相同

create_array() 申请结构体，设置type类型
create_int_array() 申请结构体，设置type类型，并新建number类型，然后添加到链表中
create_float_array() 与create_int_array相同
create_double_array() 与create_int_array相同
create_string_array() 与create_int_array相同，只是新建的是string类型

----- 类型判断、比较
compare()、is_string()

----- 其它
minify()、duplicate()

cJSON_AddItemToArray() 添加到一个数组中，在链表末尾添加
cJSON_AddItemReferenceToArray() 创建一个对象并添加到数组中，原对象不变
cJSON_AddItemReferenceToObject() 创建一个对象并添加到对象中，原对象不变

cJSON_GetObjectItem() 获取对象指针，不会从原对象中摘除
cJSON_GetObjectItemCaseSensitive() 获取对象指针，不会从原对象中摘除

cJSON_DetachItemViaPointer() 可以从对象或者数组中摘除一个对象
cJSON_DetachItemFromArray()  从一个数组中摘除对象
cJSON_DeleteItemFromArray() 从一个数组中删除对象

cJSON_InsertItemInArray() 替换
cJSON_ReplaceItemViaPointer() 删除


/* The cJSON structure: */
typedef struct cJSON
{
    struct cJSON *next;  对象同样通过链表链接
    struct cJSON *prev;
    struct cJSON *child; array中使用，类似于链表头
    int type; 类型
    char *valuestring;  string和raw类型保存
    int valueint;   已经删除，由于历史原因暂时保留
    double valuedouble; number类型

    /* The item's name string, if this item is the child of, or is in the list of subitems of an object. */
    char *string;
} cJSON;


### 使用场景

简单介绍下其经常使用的场景，一般不会判断内存是否失败。

{% highlight text %}
cJSON *data = cJSON_CreateObject();  // 不判断是否为NULL
cJSON_AddStringToObject(data, "key", "value");

cJSON_AddItemToArray(data, metric = cJSON_CreateObject());
cJSON_AddStringToObject(metric, "foobar", "foobar");
{% endhighlight %}

当 `data` 为 `NULL` 时，可能会导致内存泄露，以字符串为例，会通过 `"value"` 创建一个对象，当 `data` 为 `NULL` 时，则会直接退出，那么通过 `"value"` 创建的对象将无法释放。
















## zlog

Zlog 是一个不错的 C 日志库，具有如下的特性：

* 采用 syslog 分类模型，比 log4j 模型更加直接了当；
* 日志格式定制，类似于 log4j 的 pattern layout；
* 多种输出，包括动态文件、静态文件、stdout、stderr、syslog、用户自定义输出函数；
* 运行时手动、自动刷新配置文件（同时保证安全）；
* 用户自定义等级；
* 多线程和多进程环境下保证安全转档；
* 精确到微秒；
* MDC，线程键-值对的表，可以扩展用户自定义的字段；
* 自诊断，可以在运行时输出zlog自己的日志和配置状态；
* 不依赖其他库，只要是个 POSIX 系统就成，当然还要一个C99兼容的vsnprintf；

<!--
* 简单调用包装 dzlog（一个程序默认只用一个分类）
* 高性能，在我的笔记本上达到25万条日志每秒, 大概是syslog(3)配合rsyslogd的1000倍速度
-->

如下是一个简单的示例：

{% highlight text %}
zlog_init("test_hello.conf");     // 1. 加载配置文件，包括了formats+rules
zlog_get_category("my_cat");      // 2. 获取文件中my_cat对应的规则
zlog_info(c, "hello, zlog");      // 3. 输出信息
zlog_fini();
{% endhighlight %}

以及其配置文件 `test_hello.conf` 。

{% highlight text %}
[formats]
simple = "%m%n"
[rules]
my_cat.DEBUG    >stdout; simple
{% endhighlight %}

其中，与之相关的有三个重要概念：

* 分类(Category)：用于区分不同的输入，实际上就是一个字符串映射的一个分类，用于不同目的的输出。
* 格式(Format)：设置输出日志的格式，比如是否有带有时间戳，是否包含文件位置信息等，上面的例子里的格式就是简单输出信息+换行符。
*  规则(Rule)：把分类、级别、输出文件、格式组合起来，决定一条代码中的日志是否输出，输出到哪里，以什么格式输出。

详细的配置项如下。

{% highlight text %}
[global]
strict init = true             # 严格检查配置文件，如果格式有问题则直接退出
buffer min = 1024              # 在堆上为每个线程分配内存，日志超过缓存后会以2的倍数自动扩充，直达最大值
buffer max = 2MB
rotate lock file = zlog.lock   # 用于安全转档时的加锁
default format = "%d %V [%p:%F:%L] %m%n"   # 默认的日志格式，
                                           # 2012-02-14 17:03:12 INFO [3758:test_hello.c:39] hello, zlog
file perms = 600               # 设置日志的读写权限
reload conf period             # 写N次之后重新加载配置，原子操作，失败后继续使用之前的配置
fsync period                   # 刷磁盘的周期，每个规则日志写了一定次数后会调用fsync(3)刷新日志
                               # 用于平衡写日志速度和安全，默认是0，由系统决定刷磁盘时间
[levels]
TRACE = 10                     # 日志等级定义，值越大优先级越高
CRIT = 130, LOG_CRIT           #(level string) = (level int), (syslog level, optional)

[formats]
simple = "%m%n"                # 定义格式化时的格式
normal = "%d %m%n"

[rules]
default.*               >stdout; simple
*.*                     "%12.2E(HOME)/log/%c.log", 1MB*12; simple
my_.INFO                >stderr;
my_cat.!ERROR           "/var/log/aa.log"
my_dog.=DEBUG           >syslog, LOG_LOCAL0; simple
my_mice.*               $user_define;

                        # 输出文件名称          转储大小  转储格式
my_cat.!ERROR           "%E(HOME)/log/out.log", 1M * 3 ~ "%E(HOME)/log/out.log.#r"
{% endhighlight %}


### 锁文件

用来保证多进程情况下日志安全转档，会在 `zlog_init()` 初始化时以读写权限打开这个文件，转档日志的伪代码如下：

{% highlight text %}
write(log_file, a_log)
    if (log_file > 1M)
         if (pthread_mutex_lock succ && fcntl_lock(lock_file) succ)
       if (log_file > 1M) rotate(log_file);
    fcntl_unlock(lock_file);
             pthread_mutex_unlock;
{% endhighlight %}

`pthread_mutex_lock()` 用于多线程， `fcntl_lock()` 用于多进程，其中后者是 POSIX 建议锁 (man 3 fcntl)，这个锁是全系统有效的，在某个进程意外死亡后，操作系统会释放此进程持有的锁。

这就是为什么用 fcntl 锁来保证安全转档，注意进程需要对锁文件有读写权限。

也可以设置为 `rotate lock file = self` 此时，zlog 不会创建任何锁文件，而是用配置文件作为锁文件，因为是建议锁，用户可以自由的修改存储他们的配置文件。

<!--
一般来说，单个日志文件不会被不同操作系统用户的进程转档，所以用配置文件作为锁文件是安全的。
如果你设置其他路径作为锁文件，例如/tmp/zlog.lock，zlog会在zlog_init()的时候创建这个文件。如果有多个操作系统用户的进程需要转档同一个日志文件，确认这个锁文件对于多个用户都可读写。默认值是/tmp/zlog.lock。
-->

### 规则

用于描述日志是怎么被过滤、格式化以及被输出的。

{% highlight text %}
(category).(level)    (output), (options, optional); (format name, optional)
{% endhighlight %}

在通过 `zlog_init()` 初始化时，所有规则都会被读到内存中；当 `zlog_get_category()` 被调用，规则就被被分配给分类；在实际写日志时，如 `zlog_info()` 被调用的时候，就会比较这个INFO和各条规则的等级，来决定这条日志会不会通过这条规则输出。

当 `zlog_reload()` 被调用的时候，配置文件会被重新读入，包括所有的规则，并且重新计算分类对应的规则。

#### 级别匹配

默认有 6 个级别：`"DEBUG"` `"INFO"` `"NOTICE"` `"WARN"` `"ERROR"` 和 `"FATAL"`，具体匹配方式可以通过如下方式：

{% highlight text %}
*               所有等级
foobar.debug    代码内等级>=debug
foobar.=debug   代码内等级==debug
foobar.!debug   代码内等级!=debug
{% endhighlight %}

在源码中，通过 `zlog_rule_output()` 判断是否需要进行输出。

### 文档转储

通常是为了防止磁盘被撑爆，或者由于打个文件过大导致 grep 查看不方便，使用方式有几种。

#### 1. 按照固定时间段分割

{% highlight text %}
nodus.0912.log
nodus.0913.log
nodus.0914.log
{% endhighlight %}

例如，每天生成一个日志文件，通常每天生成的日志量大致相同，从而可以快速找到某天的日志。

可以使用 cronosplit、logrotate、cronolog 类似的工具完成。

<!--
在zlog里面，这种需求不需要用日志转档功能来完成，简单的在日志文件名里面设置时间日期字符串就能解决问题：
    *.*  "aa.%d(%F).log"
或者用cronolog来完成，速度会更快一点
    *.*  | cronolog aa.%F.log
-->


#### 2. 按照日志大小分割

用于短时间内生成大量日志的场景，通常有两种模式，在 nlog 中称之为 Sequence 和 Rolling 。

{% highlight text %}
Sequence:
    nodus.log    (new)
    nodus.log.2  (less new)
    nodus.log.1
    nodus.log.0  (old)

Rolling:
    nodus.log    (new)
    nodus.log.0  (less new)
    nodus.log.1
    nodus.log.2  (old)
{% endhighlight %}

<!--
最简单的zlog的转档配置为
    *.*     "aa.log", 10MB
这个配置是Rolling的情况，每次aa.log超过10MB的时候，会做这样的重命名
    aa.log.2 -> aa.log.3
    aa.log.1 -> aa.log.2
    aa.log.0 -> aa.log.1
    aa.log -> aa.log.0

上面的配置可以写的更加罗嗦一点
    *.*     "aa.log", 10MB * 0 ~ "aa.log.#r"
逗号后第一个参数表示文件达到多大后开始进行转档。
第二个参数表示保留多少个存档文件（0代表不删除任何存档文件）。
第三个参数表示转档的文件名，其中#r表示存档文件的序号，r是rolling的缩写。还可以放#s，是sequence的缩写。转档文件名必须包含#r或者#s。
-->

#### 3. 日志大小切分，加上时间标签

如上两种场景的综合。

{% highlight text %}
nodus.log
nodus.log-0305.00.log
nodus.log-0501.00.log
nodus.log-0501.01.log
nodus.log-1008.00.log
{% endhighlight %}

<!--
zlog对应的配置是
    *.*     "aa.log", 100MB ~ "aa-%d(%Y%m%d).#2s.log"
每到100MB的时候转档，转档文件名也支持转换字符，可以把转档当时的时间串作为转档文件名的一部分。#2s的意思是序号的长度最少为2位，从00开始编号，Sequence转档。
-->


### 自身诊断

默认是不打印日志的，可以通过 `export ZLOG_PROFILE_DEBUG=/tmp/zlog.debug.log` 变量进行设置，当然建议设置为 ERROR 这样尽量减小对性能的影响。

一般从配置文件中读取了配置信息之后，不会再修改，也可以通过 `zlog_profile()` 函数将配置打印到上述的 ERROR 中。




{% highlight text %}
zlog_init()
 |-pthread_rwlock_wrlock()                  zlog_env_lock
 |-zlog_init_inner()
 | |-pthread_key_create()                   创建zlog_thread_key
 | |-zlog_conf_new()
 | | |-zlog_level_list_new()                保存在conf->levels中
 | | | |-zc_arraylist_new()
 | | | |-zlog_level_list_set_default()
 | | |
 | | |-zc_arraylist_new()                   conf->formats
 | | |-zc_arraylist_new()                   conf->rules
 | | |
 | | |-zlog_conf_build_with_file()
 | | | |-zlog_conf_parse_line()
 | | |   |-zlog_rule_new()
 | | |     |-zlog_rule_parse_path()
 | | |       |-zc_str_replace_env()         替换环境变量
 | | |       |-zc_arraylist_new()
 | | |       |-zlog_spec_new()              新建specs，
 | | |         |-
 | | |       |-zc_arraylist_add()
 | | |-zlog_conf_build_without_file()
 | | |
 | | |-zlog_conf_profile()
 | |
 | |-zlog_category_table_new()
 | |-zlog_record_table_new()
 |-zlog_category_table_fetch_category()
 |-pthread_rwlock_unlock()

zlog_get_category()
 |-zlog_category_table_fetch_category()  尝试从zlog_env_categories中获取
 |-zlog_category_new()                   如果不存在则新建并添加
 | |-zlog_category_obtain_rules()
 | | |-zlog_rule_match_category()        遍厉各个rules，匹配满足的分类
 | | |-zc_arraylist_add()
 | | |-zlog_cateogry_overlap_bitmap()
 | |-zlog_category_profile()
 |-zc_hashtable_put()

zlog_info()
 |-zlog_category_needless_level()     通过level_bitmap判断是否需要记录日志
 |-pthread_rwlock_rdlock()            获取zlog_env_lock的读锁
 |-zlog_fetch_thread()
 | |-pthread_getspecific()            通过zlog_thread_key获取获取线程的私有变量，如果不存在则新建
 | |-zlog_thread_rebuild_msg_buf()    如果版本号不同，说明配置文件重新加载了
 | |-zlog_thread_rebuild_event()
 |
 |-va_start()
 |-zlog_event_set_fmt()               设置变量格式
 |-zlog_category_output()             真正的日志输出
 | |-zc_arraylist_foreach()           遍厉category中各个fit_rules输出
 | |-zlog_rule_output()               会根据不同的级别输出，输出函数在rule.c文件中配置
 | | |-zlog_rule_output_stdout()
 | |   |-zlog_format_gen_msg()
 | |   | |-zlog_buf_restart()
 | |   | |-zlog_spec_gen_msg()        调用每个pattern_specs
 | |   |-zlog_buf_str()
 |-va_end()
 |-pthread_rwlock_unlock()
 |-zlog_reload()                      判断是否需要重新打开日志
   |-zlog_conf_new()
   |-zlog_category_table_update_rules()
   |-zlog_category_table_commit_rules()


zlog_fini()
 |-pthread_rwlock_wrlock()   对zlog_env_lock进行加锁
 |-zlog_fini_inner()
 | |-zlog_category_table_del()
 | |-zlog_record_table_del()
 | |-zlog_conf_del()
 |-pthread_rwlock_unlock()


zlog_category_table_update_rules()
 |-zlog_category_update_rules()


zlog_conf_parse_line()
 |-zlog_format_new()



zlog_rule_output_dynamic_file_rotate()
zlog_rule_output_static_file_rotate()

zlog_rotater_rotate()
 |-zlog_rotater_trylock()  尝试对文件锁进行加锁
   |-zlog_rotater_lsmv()
     |-zlog_rotater_add_archive_files()
 |-zlog_rotater_unlock()
{% endhighlight %}


## 参考

[zlog1使用手册](https://hardysimpson.github.io/zlog/UsersGuide-CN.html) 。



zlog
vzlog
hzlog
dzlog(使用默认分类)


{% highlight text %}
{% endhighlight %}
