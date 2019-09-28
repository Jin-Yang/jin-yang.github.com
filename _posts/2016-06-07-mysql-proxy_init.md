---
title: MySQL 代理设置
layout: post
comments: true
language: chinese
category: [mysql,database]
keywords: mysql,杂项
description: 简单记录下 MySQL 常见的一些操作。
---

<!-- more -->

## 简介

MySQL Proxy 是一个处于客户端和服务端的程序，可以监控、分析、修改数据传输，从而可以达到：负载均衡、读写分离、查询过滤等等。

简单来说，这就是一个连接池，对应用来说，MySQL Proxy 完全透明，应用只需要连接到 MySQL Proxy 的监听端口即可；此时，可能会成为单点，不过可以启动多个 Proxy，然后在链接池中配置。


## 源码编译

MySQL Proxy 是基于 libevent 的，而且使用了 glib 库，所以，在编译源码时需要安装如下的包。

{% highlight text %}
# yum install libevent-devel glib-devel
{% endhighlight %}

> glibc、libc、glib 的区别。
>
> 其中，前两者都是 Linux 下的 C 函数库，libc 是 Linux 下的 ANSI C 函数库；glibc 是 Linux 下的 GUN C 函数库。glibc 后来逐渐成为了 Linux 的标准 C 库，而原来的标准 C 库 libc 不再被维护，可以直接通过 /lib/libc.so.6 或者 ldd \-\-version 命令查看。
>
> 另外，glibc 和 glib 实际上没有太大关系，glib 是一个通用库，包括了常用的一些数据类型、宏定义、类型转换等等，通常在 Unix-Like 系统上使用，例如 gtk+ 和 gnome 都使用了该库，

MySQL-5.7 之后头文件中的 ```CLIENT_SECURE_CONNECTION``` 宏定义，修改为 ```CLIENT_RESERVED2```，只需要将源码中出现的上述宏替换掉即可。

在源码的 lib 目录下，有常用的 lua 脚本，包括了读写分离(rw-spliting.lua)、管理脚本()等。

{% highlight text %}
cat << EOF > /tmp/mysql-proxy.conf
[mysql-proxy]
daemon=true                         # 以后台守护进程方式启动
log-level=debug                     # 日志级别，可选为debug、info
proxy-address=0.0.0.0:4040          # 指定mysql-proxy的监听地址
proxy-backend-addresses=127.1:3307                 # 设置后台主服务器
proxy-read-only-backend-addresses=127.1:3308       # 设置后台从服务器
proxy-lua-script=/tmp/rw-splitting.lua             # 设置读写分离脚本路径

admin-address=127.1:4041            # mysql-proxy管理地址，需要加载admin插件
admin-username=admin                # 登录管理地址用户
admin-password=admin                # 管理用户密码
admin-lua-script=/tmp/admin.lua     # 管理后台lua脚本路径

keepalive=true                     #当进程故障后自动重启
log-file=/tmp/mysql-proxy.log                      #设置日志文件路径
basedir=/usr/local/mysql-proxy                        #设置mysql-proxy的家目录


{% endhighlight %}

<!--

认证过程包括：

    客户端向服务器发起连接请求
    服务器向客户端发送握手信息
    客户端向服务器发送认证请求
    服务器向客户端发送认证结果

如果认证通过，则进入查询过程：

    客户端向服务器发起查询请求
    服务器向客户端返回查询结果

当然，这只是一个粗略的描述，每个过程中发送的包都是有固定格式的，想详细了解MySQL Protocol的同学，可以去这里看看。MySQL Proxy要做的，就是介入协议的各个过程。首先MySQL Proxy以服务器的身份接受客户端请求，根据配置对这些请求进行分析处理，然后以客户端的身份转发给相应的后端数据库服务器，再接受服务器的信息，返回给客户端。所以MySQL Proxy需要同时实现客户端和服务器的协议。由于要对客户端发送过来的SQL语句进行分析，还需要包含一个SQL解析器。可以说MySQL Proxy相当于一个轻量级的MySQL了，实际上，MySQL Proxy的admin server是可以接受SQL来查询状态信息的。

MySQL Proxy通过lua脚本来控制连接转发的机制。主要的函数都是配合MySQL Protocol各个过程的，这一点从函数名上就能看出来：

    connect_server()
    read_handshake()
    read_auth()
    read_auth_result()
    read_query()
    read_query_result()

至于为什么采用lua脚本语言，我想这是因为MySQL Proxy中采用了wormhole存储引擎的关系吧，这个虫洞存储引擎很有意思，数据的存储格式就是一段lua脚本，真是创意无限啊。
-->

http://blog.csdn.net/wudongxu/article/details/7237830

http://ourmysql.com/archives/159

http://www.longlong.asia/2015/02/07/mysql-proxy.html

http://www.nxops.cn/post/73

https://segmentfault.com/q/1010000000460861

http://www.cnblogs.com/yyhh/p/5084844.html

http://www.cnblogs.com/tae44/p/4705078.html

http://www.onexsoft.com/zh/oneproxy-take-fisrt-step.html

http://m.gzhphb.com/article/15/152520.html


{% highlight text %}
main_cmdline()     真正的入口函数
 |-chassis_frontend_init_glib()
 |-chassis_frontend_set_chassis_options()
 | |-chassis_options_add()                     添加选项，可以查看对应的变量
 |
 |-chassis_frontend_write_pidfile()            创建PID文件
 |-chassis_frontend_log_plugin_versions()      打印已经加载的插件以及版本信息
 |-chassis_mainloop()
   |
   | ###FOR###BEGIN                            循环创建N个线程
   |-chassis_event_thread_new()
   | |-g_new0()
   |-chassis_event_threads_init_thread()
   | |-event_base_new()
   | |-event_set()
   | |-event_base_set()
   | |-event_add()
   |-chassis_event_threads_add()
   | |-g_ptr_array_add()
   | ###FOR###END
   |
   |-chassis_event_threads_start()             当大于1时会调用该函数
   |-chassis_event_thread_loop()
     |-chassis_event_thread_set_event_base()
     | |-g_private_set()
     |
     | ###WHILE###BEGIN
     |-chassis_is_shutdown()                   while循环中判断是否需要退出
     |-event_base_loopexit()
     |-event_base_dispatch()
     | ###WHILE###END


chassis_frontend_load_plugins()
chassis_plugin_load()
{% endhighlight %}









1. Libevent主要接口
event_base_new:初始化一个event_base
event_set:设置event事件;即初始化struct event结构：类型，文件描述符，回调函数以及参数
event_base_set:将上面设置的event绑定到指定的event_base；每个event都必须指定所属的event_base
event_add:注册event事件
event_base_dispatch:阻塞在某个event_base上，监视绑定在它上面的所有fd

2. mysql proxy启动

/u01/xiangzhong/mysql-proxy/bin/mysql-proxy --daemon
 --pid-file=/u01/xiangzhong/mysql-proxy/proxy.pid
 --plugins=proxy
 --proxy-address=:4040
 --log-level=debug
 --log-file=/u01/xiangzhong/mysql-proxy/proxy.log
 --proxy-backend-addresses=127.0.0.1:4312
 --proxy-lua-script=/u01/xiangzhong/mysql-proxy/proxy.lua
 --event-threads=5

这是我们的一个启动方式，可见它的启动参数非常的多；其中最重要的包括proxy-backend-addresses：表示mysql-proxy代理的后台服务器

proxy-lua-script：表示在proxy运行过程中它所hook的位置的callback函数，包括connect_server，read_handshake，read_auth，read_auth_result,read_query,read_query_result。相应的使用说明可见http://dev.mysql.com/doc/refman/5.1/en/mysql-proxy-scripting.html。下面我们讲解一下mysql-proxy的启动过程。

3. Mysql-proxy启动过程

简单的说mysql-proxy是支持多线程的libevent模型。
1. Mysql-proxy启动后进入main_cmdline，初始化各种结构，解析启动参数等；

2.chassis_mainloop，它首先初始化主线程的相应结构；

3.加载启动所有的plugins(mysql-proxy可由多个插件组成：admin,proxy，这里就是加载各个组件，即调用相应组件的apply_config函数，如对于proxy-plugin这个函数为network_mysqld_proxy_plugin_apply_config，这个函数又通过network_mysqld_proxy_connection_init(con)来初始化proxy的各个状态对应的处理函数【这些函数是以NETWORK_MYSQLD_PLUGIN_PROTO(***)开头的函数名，其中***值如proxy_init,proxy_read_auth】；以及加载lua script及设置相应的global tables,network_mysqld_lua_setup_global(chas->priv->sc->L, g)；最后监视listen_sock->fd，event_set(&(listen_sock->event), listen_sock->fd, EV_READ|EV_PERSIST, network_mysqld_con_accept, con);【可见listen fd是由主线程监听的】)

4.接下来创建其它的event_threads及启动（由启动参数指定个数），chassis_event_threads_init_thread用于初始化线程结构，包括创建每个线程的event_base，然后将chassis_event_threads_t->event_notify_fds[0]复制到每个线程的chassis_event_thread_t->notify_fd，即[0]是可读的，[1]是可写，当前线程负责写，所有线程监听[0]的可读事件event_set(&(event_thread->notify_fd_event), event_thread->notify_fd, EV_READ | EV_PERSIST, chassis_event_handle, event_thread);

5.所有线程进入主循环chassis_event_thread_loop，然后调用event_base_dispatch，监听他们自己的event_base

也就是说此时主线程监听两个fd:listen fd，chassis_event_thread_t->notify_fd；其它线程监听：chassis_event_thread_t->notify_fd。














## 参考

源码可以直接从 [github mysql proxy](https://github.com/mysql/mysql-proxy) 上下载。

{% highlight text %}
{% endhighlight %}
