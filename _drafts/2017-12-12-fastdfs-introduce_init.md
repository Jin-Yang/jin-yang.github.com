---
title: AirFlow 工作流简介
layout: post
comments: true
language: chinese
category: [misc]
keywords: airflow,dac,Directed Acyclical Graphs,编排工具,有向非循环图
description:
---


<!-- more -->

https://www.cnblogs.com/Leo_wl/p/6731647.html


https://github.com/aleksandar-todorovic/awesome-linux
cmus glances
https://github.com/LewisVo/Awesome-Linux-Software


一个SQL注入工具官方地址及其简介
https://sqlchop.chaitin.cn/
https://blog.chaitin.cn/sqlchop-the-sqli-detection-engine/
手动编写的解析工具
https://github.com/client9/libinjection/

###########################################
## FastDFS
###########################################

这是一个开源的轻量级分布式文件系统，主要功能有文件存储、文件同步、文件访问等，解决了大容量存储和负载均衡的问题，适合以文件为载体的在线服务，如相册网站、视频网站等等，一般文件大小为 4KB < file_size < 500MB 。

服务端包括了 Tracker Server 和 Storage Server 两个部分，前者用于负载均衡和调度，后者用于文件存储。两种类型服务可以多节点部署，单个节点宕机不会影响整体提供服务。

### Tracker Server

负责管理所有的 Storage Server 和 Group ，每个 Storage Server 启动后会主动连接到 Tracker ，告知自己所属的 Group 同时保持心跳信息，Tracker 会根据上报的元数据在内存中维护映射信息。

### Storage Server

真正存储数据的地方，以分组 Group 为单位，每个分组包含了多个 Storage Server ，数据互为备份，存储空间以最小的为准。

实际上 Group 是一个逻辑概念，主要是为了能够方便的进行应用隔离、负责均衡和副本数定制，如果 Group 内机器出现故障需要该 Group 内的其它机器重新同步数据。

## 安装

首先是通过源码编译打包，V5.0 之前的版本依赖 libevent 而 V5.0 以后不再依赖 libevent ，V5.04 开始依赖 libfastcommon 。

----- 安装libfastcommon，这个是提取出来的单独公共代码库
./make.sh
./make.sh install
rpmbuild --bb libfastcommon.spec

----- 编译安装fastdfs
./make.sh
./make.sh install
rpmbuild --bb fastdfs.spec

注意，编译时如果报访问头文件没有权限，有可能是由于目录不允许非 root 用户访问，需要执行 `chmod o+x /usr/include/fastcommon` 。

无论是通过源码安装，还是通过除了会安装一些库以及头文件外，会在 /usr/bin 目录下安装一堆的 fdfs_xxx 的命令行，也就是常用的命令。

### 安装配置

在已经打包了 RPM 之后，接着就可以通过如下的方式安装测试。

----- 安装基本依赖库以及服务
# rpm -ivh libfastcommon-1.0.36-1.x86_64.rpm
# rpm -ivh fastdfs-server-5.11-1.x86_64.rpm

修改 tracker 的配置文件，主要是 `base_path=/home/fastdfs` 选项。

----- 创建目录并启动tracker服务，可以启动一个或者多个
# mkdir -p /home/fastdfs
# /usr/bin/fdfs_trackerd /etc/fdfs/tracker.conf restart

修改 storage 的配置文件，修改的配置项为：

base_path=/home/fastdfs
store_path0=/home/fastdfs/storage
# 如果有多个tracker，可以指定多行，不能是127.0.0.1
tracker_server=192.168.11.75:22122

然后启动 storge 的服务器即可。

# mkdir -p /home/fastdfs/storage
# /usr/bin/fdfs_storaged /etc/fdfs/storage.conf restart

上述的日志会保存在 base_path/logs 目录下，如果启动失败可以查看其日志信息。

### 上传测试

实际上可以单台部署测试，测试需要安装一些依赖包。

----- 需要安装两个RPM
# rpm -qpl libfdfsclient-5.11-1.x86_64.rpm
# rpm -qpl fastdfs-tool-5.11-1.x86_64.rpm

修改 client 的配置文件，修改选项以及内容如下。

base_path=/home/fastdfs
tracker_server=192.168.11.75:22122

然后通过如下方式上传即可。

/usr/bin/fdfs_test /etc/fdfs/client.conf upload /etc/passwd

### 监控

/usr/bin/fdfs_monitor /etc/fdfs/client.conf


### Nginx 融合

多数场景都需要为 FastDFS 存储的文件提供 http 下载服务，尽管在 storage 以及 tracker 中都内置了 http 服务，但性能表现却不尽如人意，所以通常是基于当前主流 Web 服务器做模块扩展，用于提高下载性能。

对于这一功能，需要安装 fastdfs-nginx-module 模块。

FastDFS 通过 Tracker 服务器，将文件放在 Storage 服务器存储，但是同组存储服务器之间的复制会有同步延迟，当客户端尝试访问还未同步到的 Storage 服务器时，就会报错，而通过 fastdfs-nginx-module 模块就可以重定向到其它组的服务器，避免客户端由于复制延迟导致的文件无法访问错误。


组名/磁盘名/目录名1/目录名2/文件名
group1/M00/00/00/wKgLS1pO7XaARf5HAAAGBhFrQFQ5416274


## FAQ.

### Storage Server 僵死

启动 storage server 后会一直尝试链接到 tracker server 直到成功，当 tracker server 不满足多数派时就可能会导致失败。

### 启停服务

停止服务可以直接 kill ，但是不要使用 `kill -9` 否则可能会导致 binlog 数据丢失。

/usr/bin/fdfs_trackerd /etc/fdfs/tracker.conf stop
/usr/bin/fdfs_storaged /etc/fdfs/storage.conf stop

/usr/bin/fdfs_trackerd /etc/fdfs/tracker.conf restart
/usr/bin/fdfs_storaged /etc/fdfs/storage.conf restart

/usr/local/CloudAgent/agent/plugins/CloudMonitor/

bin/uagentctl config setlog debug0

fdfs_test和fdfs_test1是做什么用的？
   这两个是FastDFS自带的测试程序，会对一个文件上传两次，分别作为主文件和从文件。返回的文件ID也是两个。
   并且会上传文件附加属性，storage server上会生成4个文件。
   这两个程序仅用于测试目的，请不要用作实际用途。
   V2.05提供了比较正式的三个小工具：
      上传文件：/usr/bin/fdfs_upload_file  <config_file> <local_filename>
      下载文件：/usr/bin/fdfs_download_file <config_file> <file_id> [local_filename]
       删除文件：/usr/bin/fdfs_delete_file <config_file> <file_id>
tracker_service_init()

tracker_accept_loop()

client_sock_read()
 |-tracker_deal_task()


relationship_thread_entrance() 选择和维护leader




注意，实际上 tracker server 是对等的，客户端可以访问任意一台，之所以引入 leader ，主要是为了解决如下问题：
  a. 新加入一台storage server时，由leader指定向其同步的源storage server；
  b. 使用了合并存储特性时，leader为每个group选举和维护唯一的一个trunk server；
以上分配如果不由leader来完成的话，可能会出现混乱情况，尤其是第2条。

tracker_service_init()
 |-work_thread_entrance() 工作线程入口
   |-recv_notify_read()

tracker_relationship_init() 启动选主流程
 |-relationship_thread_entrance() 真正入口函数
   |-relationship_select_leader() 选择leader
 
storage_dio_init() 磁盘IO读写线程
 |-blocked_queue_init() 读写队列
 |-dio_thread_entrance() 真正入口函数
storage_accept_loop() 启动多个接收线程
 |-accept_thread_entrance() 真正入口函数
   |-accept() 接收请求
   |-getPeerIpaddr() 获取对端的IP地址，用于白名单判断
   |-tcpsetnonblockopt() 设置为非阻塞模式
   |-write() 将任务的地址发送给工作线程
work_thread_entrance() 真正的工作线程
 |-ioevent_loop() 检查是否有读请求
   |-storage_recv_notify_read() 读请求的回调函数
     |-client_sock_read()
    |-fast_timer_modify() 更新超时时间
    |-storage_deal_task() 处理不同的任务
         |-storage_upload_file() 上传文件，可以是append方式
           |-fdfs_validate_filename() 获取文件大小、文件名等信息
           |-trunk_client_trunk_alloc_space() 为trunk文件名分配空间，并添加到缓存
           |-trunk_get_full_filename() 如果是trunk模式则设置相关内容
           |-storage_write_to_file() 开始写文件
       |-storage_dio_queue_push()
      |-blocked_queue_push()

通过BASE64算法生成文件ID。
提供 Trunk 功能合并小文件，否则小文件会大量消耗文件系统的 inode 资源。

## 参考

分布式文件存储系统FastDFS源码
https://github.com/happyfish100/fastdfs
FastDFS FAQ.
http://bbs.chinaunix.net/thread-1920470-1-1.html
如何搭建FastDFS的步骤
http://www.ityouknow.com/fastdfs/2017/10/10/cluster-building-fastdfs.html
基本概念介绍
http://blog.chinaunix.net/uid-20196318-id-4058561.html
http://blog.csdn.net/xingjiarong/article/details/50559849


http://seanlook.com/2015/05/18/nginx-keepalived-ha/
http://cpper.info/2016/09/15/keepalived-for-master-backup.html


http://wdxtub.com/2016/06/28/sso-guide/
https://blog.miguelgrinberg.com/post/oauth-authentication-with-flask


{% highlight text %}
{% endhighlight %}
