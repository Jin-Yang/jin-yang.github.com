---
title: MySQL 变量相关
layout: post
comments: true
language: chinese
category: [mysql,database]
keywords: database,数据库,mysql,变量
description: MySQL 通过变量设置来控制不同的行为，以及进行调优，从不同的角度看，定义方式各不相同，例如，如根据能否修改，可以分为动态和静态参数两种，动态参数可以修改，而静态参数是只读。在本文中简单介绍下 MySQL 中相关变量的设置，以及源码的实现。
---

MySQL 通过变量设置来控制不同的行为，以及进行调优，从不同的角度看，定义方式各不相同，例如，如根据能否修改，可以分为动态和静态参数两种，动态参数可以修改，而静态参数是只读。

在本文中简单介绍下 MySQL 中相关变量的设置，以及源码的实现。

<!-- more -->

## 简介

MySQL 变量从不同的角度看，定义方式各不相同，例如，如根据能否修改，可以分为动态参数和静态参数两种，动态参数可以修改，而静态参数是只读。

例外，如果按照生命周期/作用域，可以将变量分为 gloal 和 session 两种，有些参数只能在会话中修改 (如 autocommit)；有些参数会在整个实例生命周期内生效 (如 binlog_cache_size)；有些既可以在会话又可以在整个声明周期内生效 (如 read_buffer_size)。

### 局部变量

也称为存储过程变量，通过 ```DECLARE variable_name data_type(size) DEFAULT default_value;``` 声明，不过只能在 BEGIN/END 声明之间使用。

{% highlight sql %}
drop procedure if exists add;
delimiter EOF
create procedure add ( in a int, in b int )
begin
    declare c int default 0;
    set c = a + b;
    select c as c;
end EOF
delimiter ;
{% endhighlight %}

### 用户变量

用户变量的作用域要比局部变量要广，可作用于当前整个连接，但是在当前连接断开后，其所定义的用户变量都会消失。

{% highlight sql %}
drop procedure if exists math;
delimiter EOF
create procedure math ( in a int, in b int )
begin
    set @var1 = 1;                   --- 定义用户变量
    set @var2 = 2;
    select @sum:=(a + b) as sum, @dif:=(a - b) as dif;  ---- =在select中视为比较操作符
end EOF
delimiter ;
{% endhighlight %}

### 会话变量

服务器为每个连接的客户端维护一系列会话变量，在客户端连接时，使用相应全局变量的当前值对客户端的会话变量进行初始化。

客户端只能更改自己的会话变量，而不能更改其它客户端的会话变量，会话变量的作用域与用户变量一样，仅限于当前连接，当当前连接断开后，其设置的所有会话变量均失效。

可以通过如下的方式设置和查看会话变量。

{% highlight text %}
mysql> SET SESSION var_name = value;           ← 设置会话变量
mysql> SET @@session.var_name = value;
mysql> SET sort_buffer_size = 1024*1024*4;     ← 默认为session级别
mysql> SET sort_buffer_size = default;         ← 恢复默认值

mysql> SELECT @@var_name;                      ← 查看会话变量
mysql> SELECT @@session.var_name;
mysql> SHOW SESSION VARIABLES LIKE "%var%";
mysql> SHOW SESSION VARIABLES;
{% endhighlight %}

### 全局变量

全局变量影响服务器整体操作，当服务器启动时，它将所有全局变量初始化为默认值。这些默认值可以在选项文件中或在命令行中指定的选项进行更改。

要想更改全局变量，必须具有 SUPER 权限，全局变量作用于 server 的整个生命周期，重启后失效；当然，也可以在配置文件中设置。

{% highlight text %}
mysql> SET GLOBAL var_name = value;            ← 设置全局变量，不能省略global，默认为session
mysql> SET @@global.var_name = value;

mysql> SELECT @@global.var_name;
mysql> SHOW GLOBAL VARIABLES LIKE "%var%";
mysql> SHOW GLOBAL VARIABLES;
{% endhighlight %}

### 状态变量

其实就是监控 MySQL 服务器的运行状态，可以通过 ```show status``` 命令查看，只能由服务器修改，然后供用户查询。

### 其它

简单记录下比较容易混淆的地方。

有些变量同时为全局和会话变量，MySQL 将在建立连接时用全局级变量初始化会话级变量，但 **一旦连接建立之后，全局级变量的改变不会影响到会话级变量**。

查看系统变量的值，会优先显示会话级变量的值，如果这个值不存在，则显示全局级变量的值，当然你也可以加上 GLOBAL 或 SESSION/LOCAL 关键字区别。

<!--
当用 SELECT @@var_name 搜索一个变量时，也就是说，没有指定 global、session 或者 local 时，如果 SESSION 值存在，则返回，否则返回 GLOBAL 值。
对于 SHOW VARIABLES ，如果不指定 GLOBAL、SESSION 或者 LOCAL，MySQL 返回 SESSION 值。
-->

{% highlight text %}
mysql> show variables like 'log%';
mysql> show variables where variable_name like 'log%' and value='ON';
mysql> show global variables;                                           ← 查看全局变量
mysql> show session/local variables;                                    ← 查看局部变量
mysql> select @@session/local.sql_mode;
{% endhighlight %}

全局变量和会话变量可以从 INFORMATION_SCHEMA 数据库里的 GLOBAL_VARIABLES 和 SESSION_VARIABLES 表中获得。

注意：和启动时不一样的是，在运行时设置的变量不允许使用后缀字母 'K'、'M' 等，但可以用表达式来达到相同的效果，如：```SET GLOBAL read_buffer_size = 2*1024*1024``` 。



## 添加变量

当需要扩展 MySQL 时，可能需要通过变量来控制某些功能属性，当然可能在启动 server 时指定参数设置，通过配置文件，或者在运行时修改。

主要分为两种，全局变量控制系统的属性，会对所有的 session 生效；会话变量仅对当前的 session 有效，而对其他的 session 是透明的。<br><br>

其中有些变量是只能是全局变量，有些既可以是全局的又可以是会话的，

可以通过下面的方式添加变量，添加方式可以详细参考 <a href="http://blog.chinaunix.net/uid-26896862-id-3277286.html">MySQL源码增加全局变量和会话变量</a>。

</p>

<br><h2>全局变量</h2><p>
MariaDB 所支持的变量可以通过 mysqld --verbose --help 查看，添加时可以参考相应的类型，添加一个简单的布尔型全局变量 test_foobar，可以参考 old-mode 。
<ol><li>
定义/声明全局变量<br>
全局变量通常定义在 sql/mysqld.cc 文件的 /* Global variables */ 注释下面，当然可以放置到其它文件中，但是建议放在该文件中。可以使用条件编译选项，决定适用的平台等条件。<br><br>

全局变量需要在 sql/mysqld.h 中声明，以便在其它文件中使用/修改该变量。
<pre style="font-size:0.8em; face:arial;">
/* Global variables @ sql/mysqld.cc */
my_bool test_foobar;

/* @sql/mysqld.h */
extern my_bool test_foobar;
</pre></li><br><li>

初始化全局变量<br>
变量的初始化在 mysql_init_variables()@sql/mysqld.cc 中。
<pre style="font-size:0.8em; face:arial;">
test_foobar = 0;
</pre></li><br><li>

添加选项<br>
选项保存在 sql/mysqld.cc 中的 my_long_options 数组中，用于在 mysqld 启动或者配置文件中设置启动的功能选项，最后一个为全NULL，表示结束。
<pre style="font-size:0.8em; face:arial;">
struct my_option my_long_options[]=
{
    {"foobar", 0, "Just for a global test variable, default 1",
     &test_foobar, &test_foobar, 0,
     GET_BOOL, OPT_ARG, 1, 0, 0, 0, 0, NULL},
     ... ...
     {0, 0, 0, 0, 0, 0, GET_NO_ARG, NO_ARG, 0, 0, 0, 0, 0, 0}
};
</pre></li><br><li>


添加变量set设置功能<br>
对于全局变量可以通过 SET GLOBAL variables = value; 在 server 运行时进行设置，需要在 sql/sys_vars.cc 中设置。需要注意的是，不同类型的变量其初始化对象也不相同，可以参考 sys_vars.h 中不同类型的类。<br><br>

不同的类实际上是不同的类实例。
<pre style="font-size:0.8em; face:arial;">
static Sys_var_mybool Sys_foobar(
       "foobar", "Just for test, foooooobar",
       GLOBAL_VAR(test_foobar), CMD_LINE(OPT_ARG), DEFAULT(TRUE));
</pre>
<!-- 宏GLOBAL_VAR()用于指定全局变量，CMD_LINE结构用于指定命令行下参数的类型；NO_MUTEX_GUARD，NOT_IN_BINLOG分别指定锁的类型和是否记录到binlog中；宏ON_CHECK()用于在设置变量时，变量的检查，或其他一系列检查，其参数是一个函数；宏ON_UPDATE()和ON_CHECK()类似，用于变量变化时，相关变量的一系列更新，器参数也是一个函数。特别的，ON_CHECK()可以用于当设置的变量依赖其他变量的值时，这时可以通过ON_CHECK()中的函数进行检查。ON_UPDATE()用于当其他变量依赖该参数时，该变量值的变化会引起其他变量的属性时，可以通过ON_UPDATE()中的函数进行更新相关变量。为了代码整洁，建议添加到sys_vars.cc的最后。 -->
</li><br><li>

测试<br>
在启动时通过 --foobar=ON/OFF 指定，然后可以通过如下的方式查看。
<pre style="font-size:0.8em; face:arial;">
mysql> show variables like 'foobar';        // 默认是ON
mysql> set global foobar=off;               // 设置全局变量，如果设置session变量则会报错
</pre>
可以尝试通过通过两个不同的客户端链接，可以发现当一个修改之后，另一个客户端可以同时看到该修改量。
</li></ol>
如何添加处理过程？？？？？
</p>


<br><h2>会话变量</h2><p>
相比全局变量，会话变量的添加相对比较容易，在此添加同样布尔型的变量 test_session_foobar 。<ol><li>
定义会话变量<br>
会话变量定义在 sql_class.h 中的 struct system_variables 结构体中。
<pre style="font-size:0.8em; face:arial;">
typedef struct system_variables
{
    my_bool test_session_foobar;
    ... ...
} SV;
</pre>
</li><br><li>

添加选项<br>
在 sql/mysqld.cc 的 my_long_options 数组中添加变量的参数选项，同上，此时可以通过 --session-foobar 指定。
<pre style="font-size:0.8em; face:arial;">
struct my_option my_long_options[]=
{
    {"session-foobar", 0, "Just for a session test variable, default 1",
     &global_system_variables.test_session_foobar,
     &global_system_variables.test_session_foobar,
     0, GET_BOOL, OPT_ARG, 1, 0, 0, 0, 0, NULL},
     ... ...
     {0, 0, 0, 0, 0, 0, GET_NO_ARG, NO_ARG, 0, 0, 0, 0, 0, 0}
};
</pre></li><br><li>

添加变量SET功能<br>
在 sql/sys_vars.cc 中添加系统变量设置的代码，用于实现 set session VAR = VALUE; 功能。
<pre style="font-size:0.8em; face:arial;">
static Sys_var_mybool Sys_session_foobar(
    "session_foobar", "Just for test, foooooobar",
    SESSION_VAR(test_session_foobar), CMD_LINE(OPT_ARG), DEFAULT(TRUE));
</pre></li><br><li>

测试<br>
与设置全局变量相似，可以在启动时通过 --session-foobar=ON/OFF 指定，然后可以通过如下的方式查看。
<pre style="font-size:0.8em; face:arial;">
mysql> show variables like 'session_foobar';         // 默认是ON
mysql> set session session_foobar=off;               // 可以设置全局/会话变量
</pre>
</li></ol>
</p>

总体来说，如果想要在启动 server 时可以通过命令行或者配置文件指定，则需要添加到 my_long_options[]@sql/mysqld.cc 中，如 autocommit；如果想添加全局变量，可以在 sql/sys_vars.cc 中添加；如果要添加会话变量则需要在 struct system_variables@sql/sql_class.h 中添加成员变量；当然也可以设置只有 session 的变量。








## 源码解析

变量的定义在 sql/sys_vars.cc 文件中，

{% highlight text %}
mysqld_main()
 |-init_common_variables()
   |-umask()                         ← mask设置
   |-tzset()                         ← 时区设置
   |-init_thread_environment()       ← 设置线程的变量
   |-mysql_init_variables()          ← 设置全局变量
   |-ignore_db_dirs_init()
   |-add_status_vars()
   |-log_syslog_init()
{% endhighlight %}

## 参考

{% highlight text %}
{% endhighlight %}
