---
title: InnoDB 崩溃恢复
layout: post
comments: true
language: chinese
category: [mysql,database]
keywords: mysql,innodb,crash recovery,崩溃恢复
description:
---


<!-- more -->

## undrop tools


### stream_parser

stream_parser 用于在字节流中查找 InnoDB 页，可以解析多种文件，包括了 ibdata1、*.ibd、raw partition；其中 raw partitioin 可以通过 ```df -h /var/lib/mysql``` 查看。

可以通过如下方式查看如何使用。

{% highlight text %}
----- 查看磁盘
$ df -h /var/lib/mysql
Filesystem      Size  Used Avail Use% Mounted on
/dev/sda7        45G   10G   32G  24% /data

----- 可以直接解析raw partition
$ stream_parser -f /dev/sda7 -t 45g
常用选项：
  -h    打印帮助信息；
  -f    必选项，用于指定文件，可以为 ibdata1、table.ibd、/dev/sda1；
  -d    指定输出的目录，不指定默认在当前目录下；
  -V,-g 打印调试信息；
  -s    指定缓存大小，例如1G、100M(默认值)、10K；
  -t    指定扫描大小；
  -T

----- 编译调试模式
$ make debug
{% endhighlight %}





{% highlight text %}
main()
 |-open_ibfile()                打开指定的文件，并清理cache
 |-process_ibfile()             入参为起始偏移+长度
   |-lseek64()                  多进程处理时，会将文件分块处理
   |-read()                     将文件读取到缓存中
   |-valid_blob_page()          校验
   |-valid_innodb_page()
   |-process_ibpage()           处理页
{% endhighlight %}

### c_parser

c_parser 用于解析 InnoDB 页，并将其中的数据解析出来，因为 InnoDB 不包含表的结构信息，所以需要告诉 c_parser 解析的表结构信息。

https://twindb.com/undrop-tool-for-innodb/

https://twindb.com/tag/stream_parser/

https://github.com/chhabhaiya/undrop-for-innodb




https://launchpad.net/percona-data-recovery-tool-for-innodb

MySQL数据库InnoDB数据恢复工具使用总结
http://www.cnblogs.com/panfeng412/archive/2012/03/04/data-recorvery-of-mysql-innodb.html

{% highlight text %}
{% endhighlight %}
