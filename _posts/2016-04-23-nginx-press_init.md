---
Date: October 19, 2013
title: webserver 压测工具
layout: post
comments: true
language: chinese
category: [webserver]
---


在 Linux 下有四款常用的 Web 服务器压力测试工具，包括 http_load、webbench、ab、siege，在本文中会依次介绍这四种工具。

<!-- more -->

# webbench

该软件很小，最多可以模拟 3 万个并发连接去测试网站的负载能力，其版本已经很老了，而且实际上只有两个源码文件，可以通过如下方式进行安装编译。

<!--
wget http://home.tiscali.cz/cz210552/distfiles/forkbomb-1.4.tar.gz
-->

{% highlight text %}
$ wget http://home.tiscali.cz/cz210552/distfiles/webbench-1.5.tar.gz
$ tar zxvf webbench-1.5.tar.gz
$ cd webbench-1.5
$ make
# make install
{% endhighlight %}

接下来直接压测一下，压测时，一个并发是一个进程，所以最大的并发数与操作系统的资源有关系。

{% highlight text %}
$ webbench -c 500 -t 30 http://127.0.0.1/
{% endhighlight %}
相关的参数可以直接通过 webbench -h 查看帮助，参数也比较简单。如上，-c 表示同时产生 500 个并发链接，-t 表示持续 30 秒。


# http_load

可以直接从 [http_load](http://www.acme.com/software/http_load/) 上下载源码进行安装；另外，在 [www.acme.com](http://www.acme.com/software/) 网站上还有很多不错的小程序，例如之前介绍过的小型 http 服务器 micro_httpd、micro_proxy、mini_httpd 等。

其源码文件同样很少，可以通过如下方式进行编译、安装。

{% highlight text %}
$ wget http://www.acme.com/software/http_load/http_load-09Mar2016.tar.gz
$ tar zxvf http_load-09Mar2006.tar.gz
$ cd http_load-09Mar2006
$ make
# make install
{% endhighlight %}

http_load 以并行复用的方式运行，调用的是 select() 系统调用，可以用来测试 web 服务器的吞吐量与负载。可以以一个单一的进程运行，可以限制速度，一般不会把客户机压垮，还可以测试 HTTPS 类的网站请求。

{% highlight text %}
----- 命令行使用方法
http_load [-checksum] [-throttle] [-proxy host:port] [-verbose] [-timeout secs] [-sip sip_file]
   -parallel N | -rate N [-jitter]
   -fetches N | -seconds N
   url_file

----- 压测参数
-parallel, -p
   并发的用户进程数
-fetches, -f
   总共的访问次数
-rate, -r
   每秒的访问频率
-seconds, -s
   总计的访问时间

----- 示例
$ http_load -parallel 5 -seconds 300 urls.txt
{% endhighlight %}

其中的 URL 文件每行都是一个 URL，行数最好超过 50~100 个测试效果比较好，每次会随机读取文件中的 URL，如下：

{% highlight text %}
http://www.vpser.net/uncategorized/choose-vps.html
http://www.vpser.net/vps-cp/hypervm-tutorial.html
http://www.vpser.net/coupons/diavps-april-coupons.html
http://www.vpser.net/security/vps-backup-web-mysql.html
{% endhighlight %}

<!--
例如：
http_load -p 30 -s 60  urllist.txt
参数了解了，我们来看运行一条命令来看看它的返回结果
命令：% ./http_load -rate 5 -seconds 10 urls说明执行了一个持续时间10秒的测试，每秒的频率为5。
49 fetches, 2 max parallel, 289884 bytes, in 10.0148 seconds5916 mean bytes/connection4.89274
fetches/sec, 28945.5 bytes/secmsecs/connect: 28.8932 mean, 44.243 max, 24.488 minmsecs/first
-response: 63.5362 mean, 81.624 max, 57.803 minHTTP response codes: code 200 — 49

结果分析：
1．49 fetches, 2 max parallel, 289884 bytes, in 10.0148 seconds
说明在上面的测试中运行了49个请求，最大的并发进程数是2，总计传输的数据是289884bytes，运行的时间是10.0148秒
2．5916 mean bytes/connection说明每一连接平均传输的数据量289884/49=5916
3．4.89274 fetches/sec, 28945.5 bytes/sec
说明每秒的响应请求为4.89274，每秒传递的数据为28945.5 bytes/sec
4．msecs/connect: 28.8932 mean, 44.243 max, 24.488 min说明每连接的平均响应时间是28.8932 msecs，最大的响应时间44.243 msecs，最小的响应时间24.488 msecs
5．msecs/first-response: 63.5362 mean, 81.624 max, 57.803 min
6、HTTP response codes: code 200 — 49     说明打开响应页面的类型，如果403的类型过多，那可能

要注意是否系统遇到了瓶颈。
特殊说明：
测试结果中主要的指标是 fetches/sec、msecs/connect 这个选项，即服务器每秒能够响应的查询次数，用这个指标来衡量性能。似乎比 apache的ab准确率要高一些，也更有说服力一些。
Qpt-每秒响应用户数和response time，每连接响应用户时间。
测试的结果主要也是看这两个值。当然仅有这两个指标并不能完成对性能的分析，我们还需要对服务器的cpu、men进行分析，才能得出结论
-->

# AB (Apache Bench)

该工具是 Apache 自带的一款功能强大的测试工具，在 CentOS 中，包含在 httpd-tools 包中。



# Siege

开源的压力测试工具，可以根据配置对 web 站点进行多用户的并发访问，记录每个用户所有请求过程的相应时间，并在一定数量的并发访问下重复进行。

可以参考官方网站 [www.joedog.org](http://www.joedog.org/)，或者从 [www.github.com](https://github.com/JoeDog/siege) 上下载，直接通过如下方式编译、安装。

{% highlight text %}
$ wget http://download.joedog.org/siege/siege-latest.tar.gz
$ tar zxvf siege-latest.tar.gz
$ cd siege-4.0.2/
$ ./configure && make
# make install
{% endhighlight %}



<!--
使用
siege -c 200 -r 10 -f example.url
-c是并发量，-r是重复次数。 url文件就是一个文本，每行都是一个url，它会从里面随机访问的。

example.url内容:

http://www.licess.cn
http://www.vpser.net
http://soft.vpser.net

结果说明
Lifting the server siege… done.
Transactions: 3419263 hits //完成419263次处理
Availability: 100.00 % //100.00 % 成功率
Elapsed time: 5999.69 secs //总共用时
Data transferred: 84273.91 MB //共数据传输84273.91 MB
Response time: 0.37 secs //相应用时1.65秒：显示网络连接的速度
Transaction rate: 569.91 trans/sec //均每秒完成 569.91 次处理：表示服务器后
Throughput: 14.05 MB/sec //平均每秒传送数据
Concurrency: 213.42 //实际最高并发数
Successful transactions: 2564081 //成功处理次数
Failed transactions: 11 //失败处理次数
Longest transaction: 29.04 //每次传输所花最长时间
Shortest transaction: 0.00 //每次传输所花最短时间
--->









{% highlight text %}

{% endhighlight %}

