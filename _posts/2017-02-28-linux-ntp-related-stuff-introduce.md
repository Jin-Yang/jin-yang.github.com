---
title: Linux NTP 介绍
layout: post
comments: true
language: chinese
category: [linux,program,misc]
keywords: linux,时间,ntp,ntpdate
description: 网络时间协议 (Network Time Protocol, NTP) 是通过 [RFC-1305](https://tools.ietf.org/html/rfc1305) 定义的时间同步协议，该协议基于 UDP 协议，使用端口号为 123；该协议用于在时间服务器和客户端之间同步时间，从而使网络内所有设备的时钟保持一致。这里简单介绍 NTP 的使用方法。
---

网络时间协议 (Network Time Protocol, NTP) 是通过 [RFC-1305](https://tools.ietf.org/html/rfc1305) 定义的时间同步协议，该协议基于 UDP 协议，使用端口号为 123；该协议用于在时间服务器和客户端之间同步时间，从而使网络内所有设备的时钟保持一致。

这里简单介绍 NTP 的使用方法。

<!-- more -->

## NTP

对于运行NTP的本地系统，既可以接收来自其他时钟源的同步，又可以作为时钟源同步其他的时钟，并且可以和其他设备互相同步。

### 时钟层级 Stratum Levels

NTP 时间同步是按照层级分发时间的。

![ntp stratum levels]({{ site.url }}/images/misc/ntp-stratum-levels.png "ntp stratum levels"){: .pull-center width="60%"}

时钟层级定义了到参考时钟的距离，如上图所示。

* stratum-0 也即参考时钟，有极高的精度，提供可靠的 UTC 时间，通常为铯/铷原子钟(Atomic Cesium/Rubidium Clocks)、GPS 授时或者无线电波授时 (如 CDMA)，这类的时钟源不会直接为互联网提供服务。

* stratum-1 直接（非通过网络）链接到 stratum-0 提供服务，通常通过 RS-232、IRIG-B 链接，为互联网提供标准时间。

* stratum-2+ 通过网络向上一级请求服务，通常利用 NTP 协议。

通常距离顶层的时钟源越远时钟的精度也就越差。

### 安装

在 CentOS 中，可以通过如下方式进行安装。

{% highlight text %}
# yum install ntp ntpdate              # 安装
# systemctl [enable|start] ntpd        # 设置开机启动，或者直接启动
{% endhighlight %}

在 Linux 下有两种时钟同步方式，分别为直接同步和平滑同步：

* 通过 ntpdate 直接同步时间。但异常情况时，若机器有个 12 点的定时任务，当前时间认为是 13 点，而实际的标准时间为 11 点，直接使用该命令会造成任务重复执行；因此该命令多用于时钟同步服务的第一次同步时间时使用。

* 通过 ntpd 可以平滑同步时钟，通过每次同步较小的时间的偏移量，慢慢进行调整，也因此会耗费较长的时间，但可以保证一个时间不经历两次。

[www.pool.ntp.org](http://www.pool.ntp.org/zone/cn) 提供了全球的标准时钟同步服务，对于中国可以参考其中的建议 ntp 配置文件，NTP 建议我们为了保障时间的准确性，最少找两个个 NTP Server 。

{% highlight text %}
server 0.asia.pool.ntp.org
server 1.asia.pool.ntp.org
server 2.asia.pool.ntp.org
server 3.asia.pool.ntp.org
{% endhighlight %}

它的一般格式都是 number.country.pool.ntp.org ；当然，也可以使用阿里的公共服务。

![ntp alibaba basic service]({{ site.url }}/images/misc/ntp-alibaba-basic-service.png "ntp alibaba basic service"){: .pull-center width="60%"}

接下来可以使用如下命令。

{% highlight text %}
{% endhighlight %}


### 工作原理

NTP 授时有以下三种工作模式。

#### broadcast/multicast

适用于局域网的环境，时间服务器周期性以广播的方式，将时间信息传送给其他网路中的时间服务器，配置简单 (可做到客户端免配置)，但是精确度不高，对时间精确度要求不是很高的情况下可以采用。

#### client/server

最主流的 ntp 授时方式，精度较高，对网络没有特殊要求；同时可以指定多个 ntp server，因而也可避免网络路径失效带来的时钟不能同步。

#### symmetric

一台服务器可以从远端时间服务器获取时钟，如果需要也可提供时间信息给远端的时间服务器，服务器之间的 peer 关系即为这种工作方式。

接下来，简单介绍下 Client/Server 模式。

如下图所示，Device A 和 Device B 通过网络相连，且都有自己独立的系统时钟，需要通过 NTP 实现各自系统时钟的自动同步。假设在系统时钟同步前，Device A 的时钟设定为 10:00:00am，Device B 的时钟设定为 11:00:00am。

其中 Device B 作为 NTP 时间服务器，即 Device A 将使自己的时钟与 Device B 的时钟同步，另外，假设 NTP 报文在 Device A 和 Device B 之间单向传输所需要的时间为 1 秒。

![how ntp works]({{ site.url }}/images/misc/ntp-how-ntp-works.png "how ntp works"){: .pull-center }

系统时钟同步的工作过程如下：

1. Device A 发送一个 NTP 报文给 Device B，该报文带有它离开 Device A 时的时间戳，该时间戳为 10:00:00am(T1)。

2. 当 NTP 报文到达 Device B 时，Device B 会加上自己的时间戳，该时间戳为 11:00:01am(T2)。

3. 此 NTP 报文离开 Device B 时，Device B 再加上自己的时间戳，该时间戳为 11:00:02am(T3)。

4. 当 Device A 接收到该响应报文时，Device A 的本地时间为 10:00:03am(T4)。

至此，Device A 已经拥有足够的信息来计算两个重要的参数：A）报文的往返时延 Delay=(T4-T1)-(T3-T2)=2s；B) Device A 相对 Device B 的时间差 Offset=((T2-T1)+(T3-T4))/2=1h。这样，Device A 就能够根据这些信息来设定自己的时钟，使之与 Device B 的时钟同步。

### NTP 的报文格式

NTP 有两种不同类型的报文，一种是时钟同步报文，另一种是控制报文。控制报文仅用于需要网络管理的场合，它对于时钟同步功能来说并不是必需的，在此只介绍时钟同步报文。

![ntp packets format]({{ site.url }}/images/misc/ntp-packets-format.jpg "ntp packets format"){: .pull-center }

<!--
主要字段的解释如下：
LI（Leap Indicator）：长度为2比特，值为“11”时表示告警状态，时钟未被同步。为其他值时NTP本身不做处理。
VN（Version Number）：长度为3比特，表示NTP的版本号，目前的最新版本为3。
Mode：长度为3比特，表示NTP的工作模式。不同的值所表示的含义分别是：0未定义、1表示主动对等体模式、2表示被动对等体模式、3表示客户模式、4表示服务器模式、5表示广播模式或组播模式、6表示此报文为NTP控制报文、7预留给内部使用。
Stratum：系统时钟的层数，取值范围为1～16，它定义了时钟的准确度。层数为1的时钟准确度最高，准确度从1到16依次递减，层数为16的时钟处于未同步状态，不能作为参考时钟。
Poll：轮询时间，即两个连续NTP报文之间的时间间隔。
Precision：系统时钟的精度。
Root Delay：本地到主参考时钟源的往返时间。
Root Dispersion：系统时钟相对于主参考时钟的最大误差。
Reference Identifier：参考时钟源的标识。
Reference Timestamp：系统时钟最后一次被设定或更新的时间。
Originate Timestamp：NTP请求报文离开发送端时发送端的本地时间。
Receive Timestamp：NTP请求报文到达接收端时接收端的本地时间。
Transmit Timestamp：应答报文离开应答者时应答者的本地时间。
Authenticator：验证信息。
-->

NTP 提供了多种工作模式进行时间同步，包括了：客户端/服务器模式、对等体模式、广播模式、组播模式。

在不能确定服务器或对等体IP地址、网络中需要同步的设备很多等情况下，可以通过广播或组播模式实现时钟同步；客户端/服务器和对等体模式中，设备从指定的服务器或对等体获得时钟同步，增加了时钟的可靠性。

### 启动 NTP 服务

接下来，看看如何在 Linux 中配置并启动 ntp 服务器。

#### 配置文件

先修改配置文件 /etc/ntp.conf ，只需要加入上面的 NTP Server 和一个 driftfile 就可以了。

{% highlight text %}
# vi /etc/ntp.conf
driftfile /var/lib/ntp/drift
server 0.asia.pool.ntp.org
server 1.asia.pool.ntp.org
server 2.asia.pool.ntp.org
server 3.asia.pool.ntp.org
{% endhighlight %}

fudge 127.127.1.0 stratum 0  stratum  这行是时间服务器的层次。设为0则为顶级，如果要向别的NTP服务器更新时间，请不要把它设为0


<!--
那么这里一个很简单的思路就是第一我们只允许局域网内一部分的用户连接到我们的服务器. 第二个就是这些client不能修改我们服务器上的时间

权限的设定主要以 restrict 这个参数来设定，主要的语法为：
restrict IP地址 mask 子网掩码 参数
其中 IP 可以是IP地址，也可以是 default ，default 就是指所有的IP
参数有以下几个：
ignore　：关闭所有的 NTP 联机服务
nomodify：客户端不能更改服务端的时间参数，但是客户端可以通过服务端进行网络校时。
notrust ：客户端除非通过认证，否则该客户端来源将被视为不信任子网
noquery ：不提供客户端的时间查询
注意：如果参数没有设定，那就表示该 IP (或子网)没有任何限制！

在/etc/ntp.conf文件中我们可以用restrict关键字来配置上面的要求

首先我们对于默认的client拒绝所有的操作
代码:
restrict default kod nomodify notrap nopeer noquery

然后允许本机地址一切的操作
代码:
restrict 127.0.0.1

最后我们允许局域网内所有client连接到这台服务器同步时间.但是拒绝让他们修改服务器上的时间
代码:
restrict 192.168.1.0 mask 255.255.255.0 nomodify

把这三条加入到/etc/ntp.conf中就完成了我们的简单配置. NTP还可以用key来做authentication,这里就不详细介绍了
-->




#### 更新时间

如上，在正式启动 NTP 服务器之前，需要先通过 ntpdate 命令同步系统时间。需要注意的是，此时最好不要启动服务，否则可能由于时间跳变导致服务异常。

{% highlight text %}
# ntpdate 0.asia.pool.ntp.org
# ntpdate 1.asia.pool.ntp.org
{% endhighlight %}

需要注意的是，通过 ntpdate 更新时间时，不能开启 NTP 服务，否则会提示端口被占用。

{% highlight text %}
1 Jan 22:35:08 ntpdate[12481]: no servers can be used, exiting
{% endhighlight %}

当然，可以通过 cron 定期更新时间，

{% highlight text %}
30 8 * * * root /usr/sbin/ntpdate 192.168.0.1; /sbin/hwclock -w
{% endhighlight %}

这样，每天 8:30 Linux 系统就会自动的进行网络时间校准，并写入硬件时钟。


#### 查看状态

首先，通过 ntpstat 查看 NTP 服务的整体状态。

{% highlight text %}
# ntpstat
synchronised to NTP server (202.112.29.82) at stratum 3
   time correct to within 44 ms
   polling server every 256 s
{% endhighlight %}

然后，可以通过 ntpq (NTP query) 来查看当前服务器的状态，到底和那些服务器做时间同步。

{% highlight text %}
# watch ntpq -pn
Every 2.0s: ntpq -p                                  Sat Jul  7 00:41:45 2007

     remote           refid      st t when poll reach   delay   offset  jitter
===========================================================
+193.60.199.75   193.62.22.98     2 u   52   64  377    8.578   10.203 289.032
*mozart.musicbox 192.5.41.41      2 u   54   64  377   19.301  -60.218 292.411
{% endhighlight %}

<!--ntpdc -np -c peer 127.0.0.1-->

现在我就来解释一下其中各个字段的含义。

{% highlight text %}
remote: 本地机器所连接的远程NTP服务器；
 refid: 给远程服务器提供时间同步的服务器，也就是上级服务器；
    st: 远程服务器的层级别，级别从1~16，原则上不会与1直接链接；
     t: 也就是type，表示该时间服务器的工作模式(local, unicast, multicast or broadcast)；

  when: 多少秒之前与服务器成功做过时间同步；
  poll: 和远程服务器多少秒进行一次同步，开始较小，后面会逐渐增加，可在配置文件中通过minpoll+maxpoll设置；
 reach: 成功连接服务器的次数，这是八进值；
 delay: 从本地机发送同步要求到服务器的网络延迟 (round trip time)；
offset: 本地和服务器之间的时间差别，该值越接近于0，说明和服务器的时间越接近；
jitter: 统计值，统计了offset的分布情况，这个数值的绝对值越小则说明和服务器的时间就越精确；
{% endhighlight %}

对于上述输出，有几个问题，第一我们在配置文件中设置与 remote 不同？

因为 NTP 提供的是一个集群，所以每次连接的得到的服务器都有可能是不一样，也就意味着，配置文件中最好指定 hostname 而非 IP 。

第二问题是，那个最前面的 + 和 * 都是什么意思呢？如上，提供了多个服务器同步时间，这些标志就是用来标识服务器的状态。

{% highlight text %}
  (space) reject，节点网络不可达，或者 ACL 有限制；
x falsetick 通过算法确定远程服务器不可用；
. excess 不在前10个同步距离较短的节点所以被排除，如果前10个节点故障，该节点可进一步被选中；
- outlyer 远程服务器被clustering algorithm认为是不合格的NTP Server；
+ candidate 将作为辅助的服务器，作为组合算法的一个候选者，当上述的服务器不可用时，该服务器会接管；
* sys.peer 服务器被确认为我们的主NTP Server，系统时间将由这台机器所提供；
o pps.peer 该节点为选中的时钟源，系统时钟以其为参考；
{% endhighlight %}

<!--对于上述的 o ，实际上，系统时钟无论是间接地通过PPS参考时钟驱动器或直接通过内核接口同步，原则上都是是从秒脉冲信号（PPS）进行同步的。-->

也即是说，输出内容中第一个域标记符号是 * 即可认为系统通过该时间源同步时间。

了解这些之后我们就可以实时监测我们系统的时间同步状况了；注意，部分虚拟机使用的是容器，无独立 kernel，使用的就是宿主机的时间源，所以这时就需要到对应宿主机器排查。

{% highlight text %}
# systemctl start ntpd
$ ps aux | grep ntp
{% endhighlight %}



#### 其它

记录下常见的问题。

##### 1. 配置文件中的driftfile是什么?

每个 system clock 的频率都有误差，NTP 会监测我们时钟的误差值并予以调整，但是 NTP 不会立即调整时间，这样会导致时钟跳变，所以采用的是渐进方式，进而导致这是一个冗长的过程。

为此，它会把记录下来的误差先写入 driftfile，这样即使重启之前的计算结果也就不会丢失了。

另外，记录的单为是 PPM (Part Per Million)，是指百万分之一秒，也就是微秒，

##### 2. 如何同步硬件时钟?

NTP 一般只会同步 system clock，如果我们也要同步 RTC (hwclock) 的话那么只需要把下面的选项打开就可以了。

{% highlight text %}
# vi /etc/sysconfig/ntpd
SYNC_HWCLOCK=yes
{% endhighlight %}

<!--
2、fudge 127.127.1.0 stratum 10 如果是LINUX做为NTP服务器，stratum(层级)的值不能太大，如果要向上级NTP更新可以设成2

ntpd -gq


 frequency error -506 PPM exceeds tolerance 500 PPM


-g
    Normally, ntpd exits if the offset exceeds the sanity limit, which is 1000 s by default. If the sanity limit is set to zero, no sanity checking is performed and any offset is acceptable. This option overrides the limit and allows the time to be set to any value without restriction; however, this can happen only once. After that, ntpd will exit if the limit is exceeded.
-q
Exit the ntpd just after the first time the clock is set. This behavior mimics that of the ntpdate program, which is to be retired. The -g and -x options can be used with this option.
-->

## 参考

<!--
另外，有个 adjtimex 命令，可以用来调整系统和硬件时间；Linux 中通过 tzdata 保存了与时区相关的信息，包括夏令时、闰秒等信息，不过对于新发布的闰秒则需要最近的更新。
-->

[standard NTP query program](https://www.eecis.udel.edu/~mills/ntp/html/ntpq.html) ntpq 的实现官网，而且包括了很多相关的资料 [The Network Time Protocol (NTP) Distribution](https://www.eecis.udel.edu/~mills/ntp/html/index.html) 。

<!-- https://www.eecis.udel.edu/~mills/ntp/html/warp.html -->

关于时间的简单介绍，包括了时区、NTP 等 [Managing Accurate Date and Time](http://www.tldp.org/HOWTO/TimePrecision-HOWTO/index.html)，或者 [本地文档](/reference/misc/ntp/HOWTO/TimePrecision-HOWTO/index.html) ；其它的一些关于 GPS 相关的信息可以参考 [GPS时间同步原理及其应用](/reference/misc/ntp/GPS_NTP.doc) ，一篇十分不错的文章。

[Stratum Levels Defined](/reference/misc/ntp/sync_an02-StratumLevelDefined.pdf) 关于时钟层级的定义；另外一篇关于 ntpq 的介绍可以参考 [网络时间的那些事及 ntpq 详解](https://linux.cn/article-4664-1.html?pr) ，或者参考 [本地文档](/reference/misc/ntp/ntpq_details.mht)<!--；还有一篇是关于 [ntp反射式攻击原理](/reference/misc/ntp/ntp_attack.mht)--> 。

实际可以通过 GPS 和 Raspberry Pi 搭建 stratum-1 服务器，可以参考 [Building a Stratum 1 NTP Server with a Raspberry Pi](https://xmission.com/blog/2014/05/28/building-a-stratum-1-ntp-server-with-a-raspberry-pi) ，也可参考 [本地文档](/reference/misc/ntp/Building a Stratum 1 NTP Server with a Raspberry Pi.mht)；或者另一篇 [The Raspberry Pi as a Stratum-1 NTP Server](http://www.satsignal.eu/ntp/Raspberry-Pi-NTP.html)，也可查看 [本地文档](/reference/misc/ntp/Building a Raspberry-Pi Stratum-1 NTP Server.mht) 。

<!--
另外两篇包括了 常见服务器时钟不准的解决方案 、 [NTP alibaba](/reference/misc/ntp/ntp.tar.gz) 。

NTP Client
http://blog.csdn.net/rich_baba/article/details/6052863
https://lettier.github.io/posts/2016-04-26-lets-make-a-ntp-client-in-c.html
http://blog.csdn.net/rich_baba/article/details/6052863
https://www.vfe.cc/NewsDetail-2332.aspx NTP计算方法推导
http://blog.csdn.net/C3080844491/article/details/77934050

https://pthree.org/2013/11/05/real-life-ntp/

一般可以通过 ntpdate 和 ntpd 进行时钟同步，两种方式区别如下：

* ntpdate 只同步一次，需要配合 cron 机制进行周期同步；时钟会立即进行同步，会导致时间瞬间被修改。
* ntpd 是一个后台程序，会自动周期性的与服务端进行同步更新；时钟采用渐进同步方式，如果大于30min会直接退出。

ntpdc 用于访问 ntpd ，可以查看以及修改一些参数。


修改完配置文件后，可以通过如下的方式启动，查看是否正常。

----- 启动ntpd服务
# systemctl start ntpd

----- 查看启动是否正常
# netstat -atunp | grep ntpd


ntpdc -p
ntp

delay/offset 使用的单位是 milliseconds ，如果 offset 大于 0 则表示远端大于本地。

----- 查看当前的时间偏差，不更新时钟
ntpdate -q 192.144.52.46

ntpd 会渐进同步，一般启动后大概 5~10 分钟后会完成同步，所以开始通过 ntpstat 查看时显示的是未同步。

# ntpstat
unsynchronised
   polling server every 64 s

也可以通过 `ntpdate IP` 直接同步。

/usr/sbin/ntpd -p /var/run/ntp/ntpd.pid -g -u ntp:ntp -i /var/lib/ntp -c /etc/ntp.conf

/usr/bin/ntpstat
/usr/sbin/ntp-keygen
/usr/sbin/ntpd
/usr/sbin/ntpdc
/usr/sbin/ntpq
/usr/sbin/ntptime
/usr/sbin/tickadj


通过gnuplot进行绘图
https://github.com/hans-mayer/ntpgraph
http://www.cnblogs.com/sonwnja/p/6758261.html
https://www.eecis.udel.edu/~mills/ntp/html/miscopt.html
官方文档，排查问题，以及ntpq输出
http://support.ntp.org/bin/view/Support/TroubleshootingNTP
查询ntp的sysinfo信息
ntpdc -c sysinfo
ntpq -p  检查*号

### 本地测试

服务端

#----- 允许远端查询
#restrict default nomodify notrap nopeer noquery
restrict default nomodify
#----- 如果无法与远端通讯，则使用本地的时钟
server 127.127.1.0
fudge 127.127.1.0 stratum 0

date -s "2018-2-27 21:05:00"

#### 时间跳变

ntpd 在发现与服务端的时间差超过 1000s (默认值) 后会自动退出，此时需要手动进行时钟同步，也可以在启动时通过 -g 参数忽略开始的检查。

客户端和服务端的时间同步有两种情况：时间跳变 (Time Step) 和渐变 (Time Slew)。

跳变是指在client和server间时间差过大时（默认128ms），瞬间调整client端的系统时间

渐变是指时间差较小时，通过改变client端的时钟频率，进而改变client端中“1秒”的“真实时间”，保持client端时间连续性。

举个例子，如果client端比server端慢10s，通过ntpd，client端的中每1秒现实时间是1.0005秒！虽然client端的时间仍然是1秒1秒增加的，通过调整每秒的实际时间，直到与serrver的时间相同。在这个例子中，10s/0.0005s=20000s，20000s/60/60=5.55555小时，即需要5个多小时才能消除10s的误差。

在linux中，很多应用软件依赖系统的时间连续性来正确工作，系统时间的跳变将导致软件出现意想不到的问题，所以时间渐变才是ntpd的主要应用场景。

3、那么怎么禁止ntpd的时间跳变，只采用时间渐变呢？

刚开始通过man ntpd，尝试在ntpd启动配置(/etc/sysconfig/ntpd)中加-x选项：

-xNormally, the time is slewed if the offset is less than the step threshold, which is 128 ms by default, and stepped if above the threshold. This option forces the time to be slewed in all cases. If the step threshold is set to zero, all offsets are stepped, regardless of value and regardless of the -x option. In general, this is not a good idea, as it bypasses the clock state machine which is designed to cope with large time and frequency errors Note: Since the slew rate is limited to 0.5 ms/s, each second of adjustment requires an amortization interval of 2000 s. Thus, an adjustment of many seconds can take hours or days to amortize. This option can be used with the -q option.
结果发现-x只是提高了时间跳变的阈值，在client与server时间差小于600秒时，时间的调整使用渐变，大于600秒，时间调整使用跳变形式。

查了很多资料，最后确定在ntp配置文件/etc/ntp.conf中添加字段：

[html] view plain copy
tinker panic 600

这句话的意思是在时间差大于600秒的情况下，ntpd进程自动关闭，ntpd退出时会向/var/log/messages中写入log
在时间差过大时，应该由用户手动设置系统时间或者调用ntpdate命令，这样能避免因为时间跳变出现的问题。


http://www.cnblogs.com/sonwnja/p/6767936.html
http://blog.csdn.net/u014707812/article/details/52150563

#### 权限设置

通过 `restrict` 关键字设置，可以通过 `-6` 参数指定为 IPv6 ，其语法为：

restrict [-6] IP mask MASK ARGS
IP: 配置的IP地址，可以是default，也就是不指定的默认值
ARGS:
   ignore    关闭所有的NTP请求
   nomodify  客户端不能更改服务端的配置参数
   nopeer    不与同一层的NTP服务器同步
   notrust   拒绝没有通过认证的客户端请求
   noquery   不提供客户端的时间查询，一般作为主机客户端
   notrap    不提供ntpdc的trap远程事件登录的功能
   kod       阻止Kiss of Death类型的DDoS攻击

常见设置有：
restrict default kod nomodify notrap nopeer noquery     # IPv4设置
restrict -6 default kod nomodify notrap nopeer noquery  # IPv6设置

# 允许本地所有操作
restrict 127.0.0.1
restrict -6 ::1

# 允许的局域网络段或单独IP
restrict 10.0.0.0 mask 255.0.0.0 nomodify notrap
restrict 192.168.0.0 mask 255.255.255.0 nomodify notrap
restrict 192.168.1.123 mask 255.255.255.255 nomodify notrap

#### 服务端设置

设定上级时间服务器，语法为：

server IP/HOSTNAME [prefer] [iburst]

增加 prefer 表示优先选择，否则按照配置文件的顺序由高到低。

iburst 用于开始快速进行同步，一般开始一分钟只发送一次请求，通过该参数可以在第一分钟内发送多次查询，直到时间同步；如果服务没有响应，同样会快速查询。

而 burst 则会一直发送多次请求，不建议使用该选项，可能会视为攻击而被禁止。

##### poll

关于 minpoll/maxpoll 用于设置 NTP 消息的最小和最大轮询间隔，最大默认为 10(1024s) 可以设置为 17(36.4h)；最小默认为 6(64s) 也可以设置为 4(16s)；注意，这里的单位不是秒，而是 2 的几次方。

# 使用多选项的本地配置
server 127.127.1.0 prefer burst iburst minpoll 6 maxpoll 10
fudge 127.127.1.0 stratum 0

#### 其它配置

# 如果无法与上层服务器通信则以本地时间作标准时间，fudge用于指定本地
server 127.127.1.0     # local clock
fudge 127.127.1.0 stratum 10

# 指定driftfile文件名，用于写入时间偏差
ftfile /var/lib/ntp/driftrestrict

# 加密及key文件
includefile /etc/ntp/crypto/pw
keys /etc/ntp/keys


除了 ntpdate 还可以使用 rdate，不过这是一个很老的的协议了。

tinker step 0
tinker panic 0
disable monitor
disable kernel

## 典型配置

一般生产环境至少需要两台，设置为对等模式，相互备份，同时可以防止由于单台主机异常导致全网异常。

http://haibing.org/?p=52

jitter 与BIOS硬件时间差异

当系统出问题后，每次输入命令都会输出 `You have new mail in /var/spool/mail/root` 类似的内容。

这个是 Linux 系统自动发送的一些邮件，用来提醒用户系统中出了哪些问题，通常收件箱位于 `/var/mail/` 目录下，对于 root 可以直接通过 `cat /var/mail/root` 文件。

----- 关闭邮件提醒
# echo "unset MAILCHECK" >> /etc/profile
# source /etc/profile
-->

{% highlight text %}
{% endhighlight %}
