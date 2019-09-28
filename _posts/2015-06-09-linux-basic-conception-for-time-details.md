---
title: Linux 时间基本概念
layout: post
comments: true
language: chinese
category: [linux,program,misc]
keywords: linux,时间,ntp,ntpdate
description: 简单介绍在 Linux 中部分与时间相关的概念，例如时区、闰秒、夏令时、ntp 等。
---

简单介绍在 Linux 中部分与时间相关的概念，例如时区、闰秒、夏令时、ntp 等。

"现在几点了？" 或者 "离过年还有多久？" 这恐怕连小学生也觉得是再简单不过的问题了；但是如果问及 "时间是什么？" 恐怕绝大多数人都会顿觉茫然。

<!-- more -->

## 简介

在中学生数学、物理课本中，时间被表述成一条有起点、有单位、有指向、无始无终的直线，这就是符合人们常识的牛顿 "绝对时间"，也是直到本世纪初被普遍接受的科学的时间概念。

进入二十世纪后，物理学、天文学的新成果、新发现向 "绝对时间" 的基本观念提出了挑战。爱因斯坦狭义相对论指出，时间不能脱离宇宙及其事件的观察者而独立存在，时间是宇宙与其观察者之间的联系的一个方面。爱因斯坦广义相对论的一个直接推论是，由于引力场的原因处于地球表面不同高度的时钟走速不一样，海拔越高钟速越快，差值约为 1.09×10-16 秒/米（海拔），即每升高100米，时钟变快百万亿分之一秒。

"时间是什么？" 这一个问题实质上是探索时间的本质，这只是极少数科学家、哲学家热心研究的课题，而且远没有得出一个令人满意的结果，看来还需要长期探索下去。

> 关于世界时的介绍
>
> 地球自转运动是个相当不错的天然时钟，以它为基础可以建立一个很好的时间计量系统。地球自转的角度可用地方子午线相对于天球上的基本参考点的运动来度量。为了测定地球自转，人们在天球上选取了两个基本参考点：春分点和平太阳，以此确定的时间分别称为恒星时和平太阳时。恒星时虽然与地球自转的角度相对应，符合以地球自转运动为基础的时间计量标准的要求，但不能满足日常生活和应用的需要。人们习惯上是以太阳在天球上的位置来确定时间的，但因为地球绕太阳公转运动的轨道是椭圆，所以真太阳周日视运动的速度是不均匀的（即真太阳时是不均匀的）。为了得到以真太阳周日视运动为基础而又克服其不均匀性的时间计量系统，人们引进了一个假想的参考点─平太阳。它在天赤道上作匀速运动，其速度与真太阳的平均速度相一致。
>
> 平太阳时的基本单位是平太阳日，1平太阳日等于24平太阳小时，86400平太阳秒。以平子夜作为0时开始的格林威治平太阳时，就称为世界时，简称UT。世界时与恒星时有严格的转换关系，人们是通过观测恒星得到世界时的。后来发现，由于地极移动和地球自转的不均匀性，最初得到的世界时，记为UT0，也是不均匀的，人们对UT0 加上极移改正得到UT1，如果再加上地球自转速率季节性变化的经验改正就得到UT2。
>
> 六十年代以前，世界时作为基本时间计量系统被广泛应用，因为它与地球自转的角度有关，所以即使出现了更为均匀的原子时系统，世界时对于日常生活、大地测量、天文导航及其它有关地球的科学仍是必需的。

![ntp rigui logo]({{ site.url }}/images/misc/ntp-rigui-ex.jpg "ntp rigui logo"){: .pull-center width="60%"}

日晷名称是由“日”和“晷”两字组成。“日”指“太阳”，“晷”表示“影子”，“日晷”的意思为“太阳的影子”。因此，所谓日晷，就是白天通过测日影定时间的仪器。日晷计时的原理是这样。在一天中，被太阳照射到的物体投下的影子在不断地改变着，第一是影子的长短在改变，早晨的影子最长，随着时间的推移，影子逐渐变短，一过中午它又重新变长；第二是影子的方向在改变，因为我们在北半球，早晨的影子在西方，中午的影子在北方，傍晚的影子在东方。从原理上来说，根据影子的长度或方向都可以计时，但根据影子的方向来计时更方便一些。故通常都是以影子的方位计时。由于日晷必须依赖日照，不能用于阴天和黑夜。因此，单用日晷来计时是不够的，还需要其他种类的计时器，如水钟，来与之相配。


<!--
时间计量系统    时间测量误差（秒/天）   频率准确度
世界时(UT)  10-3    10-8
历书时(ET)  10-4    10-9
原子时(TA)  10-14   10-16
-->

## 时区 TimeZone

网上聊天时，有人问你说现在几点? 你看了看表回答他说晚上 8 点了，但是这个在欧洲的哥们有点纳闷了，因为他那里还太阳当空呢！

![ntp time zone]({{ site.url }}/images/misc/ntp-time-zone-logo.jpg "ntp time zone"){: .pull-center width="70%"}

实际上，人们习惯于按照太阳的位置对当地的时间进行划分，通常是太阳在正中时作为 12 点。因为地球是圆的，在环绕太阳旋转的 24 个小时中，世界各地日出日落的时间是不一样的，所以我们才有划分时区的必要。

在真正使用时，为了照顾到行政上的方便，时区并不严格按南北直线来划分，而是按自然条件来划分。例如，我国差不多跨 5 个时区，但实际上在只用东八时区的标准时即北京时间为准。

![ntp time zone]({{ site.url }}/images/misc/ntp-global-time-zone.jpg "ntp time zone"){: .pull-center width="100%"}

1884 年在华盛顿召开的一次国际经度会议，又称国际子午线会议，规定将全球划分为 24 个时区，其中东、西各 12 个时区，而且规定英国 (格林尼治天文台旧址) 为中时区 (零时区)，每个时区横跨经度 15 度，时间正好是1小时；最后的东、西第 12 区各跨经度 7.5 度，以东、西经 180 度为界。

### 常见时区

地理课上都学过格林威治时间 `Greenwich Mean Time, GMT`，它也就是 0 时区时间，指位于英国伦敦郊区的皇家格林尼治天文台的标准时间，因为本初子午线被定义在通过那里的经线。

但是在计算机中经常看到的是 `Universal Time Coordinated, UTC`，由世界上最精确的原子钟提供计时，一般将 UTC 认定为是国际标准。

`Chinese Standard Time, CST` 也就是常说的北京时间，一般也就是通过 `UTC+8` 表示，那么假如现在中国当地的时间是晚上 8 点，可以有下面两种表示方式。

{% highlight text %}
20:00 CST
12:00 UTC
{% endhighlight %}

在 `*nix` 系统中，还有一个词 `Epoch`，它指的是一个特定时间 `1970-01-01 00:00:00 +0000 (UTC)`。

### Linux 下时区设置

在 Linux 中的 glibc 提供很多编译好的 timezone 文件，存放在 `/usr/share/zoneinfo` 目录下，基本涵盖了大部分的国家和城市。

{% highlight text %}
# ls -F /usr/share/zoneinfo/
Africa/      Chile/   Factory    Iceland      Mexico/   posix/      Universal
America/     CST6CDT  GB         Indian/      Mideast/  posixrules  US/
Antarctica/  Cuba     GB-Eire    Iran         MST       PRC         UTC
Arctic/      EET      GMT        iso3166.tab  MST7MDT   PST8PDT     WET
Asia/        Egypt    GMT0       Israel       Navajo    right/      W-SU
Atlantic/    Eire     GMT-0      Jamaica      NZ        ROC         zone.tab
Australia/   EST      GMT+0      Japan        NZ-CHAT   ROK         Zulu
Brazil/      EST5EDT  Greenwich  Kwajalein    Pacific/  Singapore
Canada/      Etc/     Hongkong   Libya        Poland    Turkey
CET          Europe/  HST        MET          Portugal  UCT
{% endhighlight %}

在这里面就可以找到自己所在城市的 time zone 文件，文件的内容可以通过 `zdump` 查看。

{% highlight text %}
# zdump /usr/share/zoneinfo/Asia/Shanghai
Shanghai  Sun Jan  1 07:46:01 2013 Shanghai

----- 查看当前时区的信息，isdst=1标示夏令时
# zdump -v /etc/localtime
/etc/localtime  Sun Sep 16 21:19:48 2018 CST
{% endhighlight %}

#### 查看

{% highlight text %}
$ date -R
Tue, 09 Jun 2015 16:58:34 +0800

$ timedatectl 
      Local time: Sun 2019-03-31 17:03:48 CST
  Universal time: Sun 2019-03-31 09:03:48 UTC
        RTC time: Sun 2019-03-31 17:03:48
       Time zone: Asia/Shanghai (CST, +0800)
     NTP enabled: yes
NTP synchronized: yes
 RTC in local TZ: yes
      DST active: n/a

Warning: The system is configured to read the RTC time in the local time zone.
         This mode can not be fully supported. It will create various problems
         with time zone changes and daylight saving time adjustments. The RTC
         time is never updated, it relies on external facilities to maintain it.
         If at all possible, use RTC in UTC by calling
         'timedatectl set-local-rtc 0'.
{% endhighlight %}

可以通过设置 `/etc/localtime` 符号链接，或者设置 `TZ` 环境变量设置本地的时区，其中后者优先。

{% highlight text %}
----- 1. 直接添加符号链接
# cd /etc
# ln -sf ../usr/share/zoneinfo/Asia/Shanghai localtime

----- 2. 首先根据提示选择时区，然后设置环境变量
# tzselect
# export TZ='America/Los_Angeles'
{% endhighlight %}

注意，第二种方法需要添加到 profile 文件中。

## Linux 时间设置

首先，需要明确两个概念，在一台计算机上我们有两个时钟：A) 硬件时间时钟 (Real Time Clock, RTC)；B) 系统时钟 (System Clock) 。

硬件时钟是主板上的一特殊电路，由单独的电池供电，从而保证关机之后还可以计算时间。系统时钟就是内核用来计算时间的时钟，也就是 Epoch 是从 `1970-01-01 00:00:00 UTC` 到目前为止秒数总和。

在 Linux 下系统时间在开机的时候会和硬件时间同步，之后也就各自独立运行了；当然，时间久了之后必然就会产生误差。可以通过如下命令查看系统时间以及硬件时间。

{% highlight text %}
----- 1. 查看系统时间和硬件时间
# date
Fri Jul  6 00:27:13 CST 2007
# hwclock --show
Fri 06 Jul 2007 12:27:17 AM CST  -0.968931 seconds

----- 2. 时间同步
# hwclock --hctosys               # 把硬件时间设置成系统时间
# hwclock --systohc               # 把系统时间设置成硬件时间

----- 3. 时间设置
# hwclock --set --date="mm/dd/yy hh:mm:ss"
# date -s "dd/mm/yyyy hh:mm:ss"
{% endhighlight %}

另外，RTC 还可以在启动时在 BIOS 里设定。

实际上服务器的时钟与标准时间随着时间偏移都会有些偏差，因为现在使用的 "铯/铷原子钟" 成本过高，为此我们就需要在互联网上找到一个可以提供准确时间的服务器，然后通过一种协议来同步我们的系统时间。

这个协议就是 NTP 了，详见 [Linux NTP 介绍](/post/linux-ntp-related-stuff-introduce.html) 。

## DST

夏令时 `Daylight Saving Time, DST` 是一种为节约能源而人为规定地方时间的制度，在这一制度实行期间所采用的统一时间称为 "夏令时间"。

一般在天亮早的夏季人为将时间调快一小时，可以使人早起早睡，减少照明量，以充分利用光照资源，从而节约照明用电。各个采纳夏时制的国家具体规定不同，目前全世界有近 110 个国家每年要实行夏令时。

北京夏令时是我国在 1986~1991 年间实施的一种夏季时间制度，北京夏令时比标准的北京时间早一个小时。

### 实际查看

夏令时跟城市的政策有关，例如美国并非所有城市使用夏令时，其实，一个地区的夏令时、冬令时信息已经在 Linux 中设置好，时区设置好之后对应的夏令时也就是配置好了。

可以找个欧洲实时夏令时的时区，例如 `Central European Time, CET`，其中冬季为 `UTC+1`，夏季欧洲夏令时为 `UTC+2` 。

{% highlight text %}
# zdump -v /usr/share/zoneinfo/CET
... ...
/usr/share/zoneinfo/CET  Sun Mar 27 00:59:59 2016 UTC = Sun Mar 27 01:59:59 2016 CET isdst=0 gmtoff=3600
/usr/share/zoneinfo/CET  Sun Mar 27 01:00:00 2016 UTC = Sun Mar 27 03:00:00 2016 CEST isdst=1 gmtoff=7200
/usr/share/zoneinfo/CET  Sun Oct 30 00:59:59 2016 UTC = Sun Oct 30 02:59:59 2016 CEST isdst=1 gmtoff=7200
/usr/share/zoneinfo/CET  Sun Oct 30 01:00:00 2016 UTC = Sun Oct 30 02:00:00 2016 CET isdst=0 gmtoff=3600
... ...
{% endhighlight %}

其中 `isdst` 标示了是否使用夏令时，例如上述标示在 `01:59:59` 会直接跳转到 `03:00:00` 。

可以通过如下方式进行测试。

{% highlight text %}
----- 用来观察时间的变化,ru
$ for i in `seq 1 1000`;do date;sleep 1;done
Sun Mar 27 01:59:56 CET 2016
Sun Mar 27 01:59:57 CET 2016
Sun Mar 27 01:59:58 CET 2016   # 怎么感觉还少了1秒
Sun Mar 27 03:00:00 CEST 2016
Sun Mar 27 03:00:01 CEST 2016
Sun Mar 27 03:00:02 CEST 2016

----- 设置时区，并修改时间
# ln -sf /usr/share/zoneinfo/CET /etc/localtime
# date -s "2016-03-27 02:59:50"
{% endhighlight %}

注意，如果通过 `date -s "2016-03-27 02:59:50"` 设置，会报日期非法的错误。


## GPS 周数反转

近日，美国民用 GPS 服务接口委员会在美国国土安全部网站发布了 GPS 周计数即将在 2019年4月6日 迎来第二次翻转事件的预警。

那么什么是 GPS 翻转？带来的影响是啥？

GPS 周 (GPS Week) 是 GPS 系统内部所采用的时间系统，其中时间零点定义的为 `1980-01-05 00:00:00 +0000 (UTC)`，最大时间单位是周，每 1024 周 (7168天) 为一循环周期。

第一个 GPS 周循环点为 `1999-08-22 00:00:00`，从这一刻起，周数重新从 0 开始算起。

就像千年虫问题困扰那样，GPS 循环点来临时，未升级的 GPS 接收机都将在周归零日错认为 `1980-01-06` 。

GPS 周计数翻转原因是由于，在 GPS 导航电文中，系统时间由整周计数 (WN) 以及周内时 (TOW)构成，周计数仅占用 10bit，因此周计数会在 0~1023 区间循环，当 GPS 周计数实际达到 1024 时，导航电文中的周计数将翻转为 0 并重新开始计数。

这样就可能会导致部分设备工作异常，输出时间错误等问题，详见 [www.gps.gov](https://www.gps.gov/) 。

## 其它

另外，有个 adjtimex 命令，可以用来调整系统和硬件时间；Linux 中通过 tzdata 保存了与时区相关的信息，包括夏令时、闰秒等信息，不过对于新发布的闰秒则需要最近的更新。

{% highlight text %}
{% endhighlight %}
