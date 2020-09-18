---
Date: October 19, 2013
title: LXC 源码解析
layout: post
comments: true
language: chinese
category: [linux]
---


<!-- more -->

在之前的 [LXC 简介](/blog/linux-lxc-introduce.html) 中介绍了如何搭建 CentOS 7 容器，在此介绍如何单独启动 sshd 这一个进程。

LXC 的源码可以直接从 [linuxcontainers.org](https://linuxcontainers.org/lxc/downloads/) 上下载。


# lxc-start

启动很简单，实际是执行 init 命令，而对于比较简单的容器，如单独启动 sshd 进程，则实际上会执行 /sbin/init.lxc 命令，其源码可以参考 src/lxc_init.c 文件。

如下是容器启动过程：

{% highlight text %}
main()                                                   # lxc_start.c
 |-lxc_list_init()                                       # 初始化配置(-s 指定)的列表
 |-lxc_caps_init()                                       # 以root或者程序的uid、gid启动程序
 |-lxc_arguments_parse()                                 # 解析传入的参数
 |-lxc_log_init()                                        # log初始化操作
 |-lxc_container_new()                                   # 初始化container实例，包括调用的函数
 |-c->load_config()                                      # 加载配置文件
 |
 |-do_lxcapi_start()                                     # 代码中通过c->start()调用
 | |-ongoing_create()                                    # 判断是否在创建镜像时失败了，失败则退出
 | |-daemonize?                                          # 是否后台运行
 | |-lxc_check_inherited()                               # 如果后台执行，需要关闭部分文件描述符
 | |-__lxc_start()                                       # 尝试启动容器，实际调用该函数
 | | |-lxc_init()
 | | | |-lsm_init()
 | | | |-lxc_cmd_init()                            // LSM安全模块初始化
 | | | |-lxc_read_seccomp_config()
 | | | |-lxc_set_state()
 | | | |-run_lxc_hooks()
 | | | |-lxc_create_tty()                          // 创建伪终端设备
 | | | |-setup_signal_fd()
 | | | |-lxc_console_create()
 | | | | |-openpty()
 | | | |-ttys_shift_ids()
 | | |
 | | |-must_drop_cap_sys_boot()                     // 判断是否支持ctrl-alt-del重启，支持会新建进程
 | | |
 | | |-lxc_spawn()                                       # 此时会创建多个子进程
 | | | |-lxc_sync_init()                                 # 创建sock对，用于通讯
 | | | |-resolve_clone_flags()                           # 设置flags
 | | | |-cgroup_init()
 | | | |-cgroup_create()
 | | | |-preserve_ns()
 | | | |-attach_ns()
 | | | |-lxc_clone(do_start, ...)                        # 在新的NS中创建进程
 | | | | |-clone(do_clone, ...)                          # 系统调用，外面加了一个壳，实际调用do_start
 | | | |-lxc_sync_fini_child()
 | | |
 | | |-get_netns_fd()
 | | |
 | | |-lxc_poll()                                   // 等待程序结束
 | | |  |-lxc_mainloop_open()
 | | |  |-lxc_mainloop_add_handler()
 | | |  |-lxc_console_mainloop_add()
 | | |  |-lxc_cmd_mainloop_add()
 | | |  |  |-lxc_cmd_accept()
 | | |  |     |-lxc_cmd_handler()
 | | |  |        |-lxc_cmd_process()                // 设置一系列的回调函数
 | | |  |           |-lxc_cmd_console_callback()
 | | |  |-lxc_mainloop()
 | | |
 | | |-waitpid()
 | |
 | |-free_init_cmd()                                    # 清空之前申请的资源
 |
 |-lxc_container_put()                                   // 清理


# 新创建的进程
do_start()
 |-lxc_setup()                                          # 设置容器、ip、utsname等
 | |-do_rootfs_setup()                                  // 设置根目录
 | |  |-mount()
 | |  |-remount_all_slave()                             // 删除
 | |  |-run_lxc_hooks()
 | |  |-setup_rootfs()                                  // 挂载根目录
 | |
 | |-setup_utsname()                                    // 设置主机名
 | |
 | |-setup_network()                                    // 设置网络端口eth0
 | |  |-setup_netdev()
 | |
 | |-check_autodev()                                    // 设置init命令等
 | |
 | |-setup_mount()                                      // 设置配置文件中的挂载点
 | |  |-setmntent()
 | |  |-setup_mount_entries()
 | |  |  |-mount_entry_on_relative_rootfs()
 | |  |-endmntent()
 | |
 | |-lxc_mount_auto_mounts()                            // 自动挂载一些主要目录，如proc、sys
 | |
 | |-setup_console()
 | |-setup_tty()
 | |-setup_dev_symlinks()
 | |-tmp_proc_mount()
 | |
 | |-setup_pivot_root()
 | |  |-setup_rootfs_pivot_root()
 | |
 | |-setup_pts()
 |
 |-lxc_sync_barrier_parent()
 |
 |-handler->ops->start()                                # 最后调用init，调用start()@start.c


  lxc_console_allocate()
   |-lxc_console_peer_proxy_alloc()
      |-openpty()
{% endhighlight %}

# 数据结构

## lxc_handler

这个结构体种保存了核心的内容，通过 lxc_init() 进行初始化。


# 调试


## GDB

对一些常见的问题我们可以直接通过 GDB 进行调试。

{% highlight text %}
# cd src/lxc                        // 跳转到含有编译后二进制文件的目录下
# gdb lxc-start                     // 开始调试lxc-start
... ...
Reading symbols from src/lxc/lxc-start...done.
(gdb) set args -n sshd -F -l DEBUG -o /tmp/1
(gdb) b main
Breakpoint 1 at 0x400f20: file lxc_start.c, line 205.
(gdb) run
Starting program: src/lxc/lxc-start -n sshd -l DEBUG -o /tmp/1
[Thread debugging using libthread_db enabled]
Using host libthread_db library "/lib64/libthread_db.so.1".

Breakpoint 1, main (argc=7, argv=0x7fffffffdb88) at lxc_start.c:205
205     {
{% endhighlight %}



## 修改日志级别

为了方便调试我们可以设置日志级别，默认日志级别为 ERROR，需要注意的是，因为在打印日志前需要做一系列的初始化操作，所以即使设置为 DEBUG 日志级别，开始的部分也无法输出。

在调试启动时，可以在命令行后面添加 -l DEBUG -o /tmp/1 用于调试，也即 \-\-logfile、\-\-logpriority，这两个参数需要同时指定，否则无法打印，例如：

{% highlight text %}
# ./lxc-start -n sshd -l DEBUG -o /tmp/lxc-start.log
{% endhighlight %}





# 其它

## 问题排查

在调试源码时，如果安装的 lxc 版本与编译源码的版本不同，那么有可能会报如下的错误。

{% highlight text %}
./lxc-start: symbol lookup error: ./lxc-start: undefined symbol: current_config
{% endhighlight %}

这主要时由于动态库的版本不同导致的，通过 ldd 查看时会发现，加载的库时 /lib64/liblxc.so.1 的库。

{% highlight text %}
# ldd lxc-start
    linux-vdso.so.1 =>  (0x00007fffae967000)
    liblxc.so.1 => /lib64/liblxc.so.1 (0x00007f8c1c7ea000)
    libselinux.so.1 => /lib64/libselinux.so.1 (0x00007f8c1c5c5000)
    libutil.so.1 => /lib64/libutil.so.1 (0x00007f8c1c3c1000)
    libpthread.so.0 => /lib64/libpthread.so.0 (0x00007f8c1c1a5000)
    libc.so.6 => /lib64/libc.so.6 (0x00007f8c1bde4000)
    libcap.so.2 => /lib64/libcap.so.2 (0x00007f8c1bbde000)
    libseccomp.so.2 => /lib64/libseccomp.so.2 (0x00007f8c1b9b2000)
    libgcc_s.so.1 => /lib64/libgcc_s.so.1 (0x00007f8c1b79c000)
    /lib64/ld-linux-x86-64.so.2 (0x00007f8c1ca79000)
    libpcre.so.1 => /lib64/libpcre.so.1 (0x00007f8c1b53a000)
    liblzma.so.5 => /lib64/liblzma.so.5 (0x00007f8c1b315000)
    libdl.so.2 => /lib64/libdl.so.2 (0x00007f8c1b111000)
    libattr.so.1 => /lib64/libattr.so.1 (0x00007f8c1af0b000)
{% endhighlight %}

这主要是由于动态库的查找顺序导致的，更改查找顺序有很多种方法，在此简单使用如下的方法。

{% highlight text %}
----- 修改配置文件，将本地目录添加到开始位置
# cat /etc/ld.so.conf
.
/lib64
/usr/lib64

----- 直接加载更新配置缓存
# ldconfig

----- 建立liblxc.so.1的符号链接
$ ln -s liblxc.so liblxc.so.1
{% endhighlight %}







# 源码解析

直接从官方网站 [linuxcontainers.org][lxc-offical] 下载源码，然后通过如下方式编译。


{% highlight text %}
----- 如下方式安装会有部分文件在/share目录下
$ ./configure --exec_prefix=/usr --prefix=
{% endhighlight %}




[lxc-offical]:            https://linuxcontainers.org/               "LXC 官方网站"


{% highlight text %}
{% endhighlight %}
