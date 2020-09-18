---
title: Python 时间操作
layout: post
comments: true
category: [program, python]
language: chinese
---


<!-- more -->

Python 与时间处理相关的提供了 datetime、time、calendar 三个模块，而且还有三方模块 pytz 可以使用；另外，datetime 模块中又存在 datetime、time 类，不要与相应的模块混淆。

{% highlight text %}
datetime.date        理想化的日期对象，有年月日三个属性
datetime.time        理想化的时间对象，不考虑闰秒，有时分秒微秒和时区五个属性
datetime.datetime    上述两个对象的组合
datetime.timedelta   可用于上述三个对象的差值
datetime.tzinfo      时区信息

time                 各种时间操作转换的方法
calendar             提供日历相关的方法
pytz                 使用Olson TZ Database解决时区、夏令时等相关的计算问题
{% endhighlight %}


简单来说，在 Pyton 中，与时间相关的时间类型主要有如下的四种。

{% highlight python %}
import time, datetime

#----- 1. time string 用于打印输出的字符串
time.ctime()             # 'Fri June 26 21:02:55 2015'

#----- 2. datetime tuple 包含了年月日-时分秒微妙-时区信息
datetime.datetime.now()  # datetime.datetime(2012, 12, 17, 21, 3, 44, 139715)

#----- 3. time tuple 也就是time.struct_time对象类型
datetime.datetime.now().timetuple()  # time.struct_time(tm_year=2017, tm_mon=1, tm_mday=2,
                    # tm_hour=10, tm_min=10, tm_sec=48, tm_wday=0, tm_yday=2, tm_isdst=-1)

#----- 4. timestamp 时间戳类型，从1970-01-01 00:00:00 GMT以来的秒数
time.time()              # 1355749338.05917
{% endhighlight %}

<!--
1. string 转换为其它
初始化:
    date_str = "2015-06-26 22:53:59"

1.1 string => datetime obj
导入:
    import datetime
    datetime.datetime.strptime(string, format)
eg
    ----------------------------------------------
    >>> dt_obj = datetime.datetime.strptime(date_str, "%Y-%m-%d %H:%M:%S")
    >>> dt_obj
    datetime.datetime(2008, 11, 10, 17, 53, 59)
    ----------------------------------------------

1.2 string => time obj
导入:
    import time
    time.strptime(string, format)
eg
    ----------------------------------------------
    #time模块有类似datetime中的strptime()函数
    >>> date_str = "2008-11-10 17:53:59"
    >>> t_obj = time.strptime(date_str, "%Y-%m-%d %H:%M:%S")
    >>> t_obj
    time.struct_time(tm_year=2008, tm_mon=11, tm_mday=10, tm_hour=17, tm_min=53, tm_sec=59, tm_wday=0, tm_yday=315, tm_isdst=-1)
    ----------------------------------------------

2. datetime obj转换为其它
datetime obj转换为其它类型,用的都是datetime的函数
初始化:
    dt_obj = datetime.datetime(2008, 11, 10, 17, 53, 59)
2.1 dt obj => string
    ----------------------------------------------
    date_str = dt_obj.strftime("%Y-%m-%d %H:%M:%S")
    ----------------------------------------------
2.2 dt obj => time obj
    ----------------------------------------------
    time_tuple = dt_obj.timetuple()
    ----------------------------------------------

3. time obj转换为其它
初始化:
    time_tuple = (2008, 11, 12, 13, 51, 18, 2, 317, 0)
3.1 time obj => string
    ----------------------------------------------
    date_str = time.strftime("%Y-%m-%d %H:%M:%S", time_tuple)
    ----------------------------------------------
3.2 time obj => datetime obj
    ----------------------------------------------
    datetime.datetime(*time_tuple[0:6])
    ----------------------------------------------
3.3 time obj => timestamp
    ----------------------------------------------
    ts = time.mktime(time_tuple)
    ----------------------------------------------

4. timestamp转换为其它
初始化:
    timestamp = 1226527167.595983
--!!--注意以下两种都使用local时区
4.1 timestamp => dt obj
    ----------------------------------------------
    dt_obj = datetime.fromtimestamp(timestamp)
    ----------------------------------------------
4.2 timestamp => time obj
    ----------------------------------------------
    time_tuple = time.localtime(timestamp)
    ----------------------------------------------
--!!--以下两种方式和时区相关,比较标准时区时间和本地时区时间
4.3 使用UTC => dt obj
    ----------------------------------------------
    #本地时区时间
    >>> datetime.datetime.fromtimestamp(tm)
    datetime.datetime(2012, 12, 17, 23, 39, 58, 401881)
    #标准时区时间
    >>> datetime.datetime.utcfromtimestamp(tm)
    datetime.datetime(2012, 12, 17, 15, 39, 58, 401881)
    ----------------------------------------------
4.4 使用UTC => time obj
    ----------------------------------------------
    #本地时区时间
    >>> time.localtime(tm)
    time.struct_time(tm_year=2012, tm_mon=12, tm_mday=17, tm_hour=23, tm_min=39, tm_sec=58, tm_wday=0, tm_yday=352, tm_isdst=0)
    #标准时区时间
    >>> time.gmtime(tm)
    time.struct_time(tm_year=2012, tm_mon=12, tm_mday=17, tm_hour=15, tm_min=39, tm_sec=58, tm_wday=0, tm_yday=352, tm_isdst=0)
    ----------------------------------------------
-->


![datetime transfrom]({{ site.url }}/images/python/datetime-transform.jpg "datetime transfrom"){: .pull-center }

## 时间处理

很多与日期、时间的 api 都在 datetime 模块内，其中常见的操作如下。

{% highlight python %}
import datetime

#----- 1. 日期输出格式化 datetime=>string
now = datetime.datetime.now()
now.strftime('%Y-%m-%d %H:%M:%S') # '2015-04-07 19:11:21'
now.strftime('%a, %d %b %Y %H:%M:%S %Z') # Sun, 01 Jan 2017 02:10:33 GMT

#----- 2. 日期输出格式化 string=>datetime
t_str = '2015-04-07 19:11:21'
d = datetime.datetime.strptime(t_str, '%Y-%m-%d %H:%M:%S')

#----- 3. 两个日期相差多少天
d1 = datetime.datetime.strptime('2015-06-15 18:11:27', '%Y-%m-%d %H:%M:%S')
d2 = datetime.datetime.strptime('2015-06-02 18:11:27', '%Y-%m-%d %H:%M:%S')
print delta, delta,days

#----- 4. 今天的n天后的日期
now = datetime.datetime.now()
delta = datetime.timedelta(days=3)
print (now + delta).strftime('%Y-%m-%d %H:%M:%S')
{% endhighlight %}

其中，时间格式化的参数内容与 man date 相同，可以直接参考。






## xxx


由于国家和地区可以自己选择时区以及是否使用夏令时，所以pytz模块在有需要的情况下得更新自己的时区以及夏令时相关的信息。比如当前pytz版本的OLSON_VERSON = ‘2013g’, 就是包括了Morocco可以使用夏令时。
如何正确为你所用

不是题外话的题外话，客户端必须正确收集用户的timezone信息。比较常见的一个错误是，保存用户所在时区的偏移值。比如对于中国的时区，保存了＋8。这里其实丢失了用户所在的地区（同样的时间偏移，可能对应多个国家或者地区）。而且如果用户所在时区是有夏令时的话，在每年开始和结束夏令时的时候，这个偏移值都是要发生变化的。


{% highlight text %}
>>> import pytz
>>> pytz.all_timezones                      # 查看所有的timezone
[...,'Asia/Shanghai', ...]                  # 选择上海的时区，也就是东八区
>>> pytz.timezone('Asia/Shanghai')          # 构建tzinfo对象
<DstTzInfo 'Asia/Shanghai' LMT+8:06:00 STD>
{% endhighlight %}


我们开始要把timezone加入时间的转换里面了。

首先，timestamp和datetime的转换。timestamp，一个数字，表示从UTC时间1970/01/01开始的秒数。



{% highlight text %}
>>> import datetime
>>> datetime.datetime.fromtimestamp(0, pytz.timezone('UTC'))   # 含有时区信息
datetime.datetime(1970, 1, 1, 0, 0, tzinfo=<UTC>)
>>> tz = pytz.timezone('Asia/Shanghai')                        # 上海则早8小时
>>> datetime.fromtimestamp(0, tz)
datetime.datetime(1970, 1, 1, 8, 0, tzinfo=<DstTzInfo 'Asia/Shanghai' CST+8:00:00 STD>)
{% endhighlight %}

如上，timestamp 是和 UTC 绑定的，不同的时区会有不同的时间偏移。

给定一个timestamp，构建datetime的时候无论传入的是什么时区，对应出来的结果都是同一个时间。 但是python里面这里有个坑。

>>> ts = 1408071830
>>> dt = datetime.fromtimestamp(ts, tz)
datetime.datetime(2014, 8, 15, 11, 3, 50, tzinfo=<DstTzInfo 'Asia/Shanghai' CST+8:00:00 STD>)
>>> time.mktime(dt.timetuple())
1408100630.0
>>> dt.timetuple()
time.struct_time(tm_year=2014, tm_mon=8, tm_mday=15, tm_hour=11, tm_min=3, tm_sec=50, tm_wday=4, tm_yday=227, tm_isdst=0)
>>> dt.astimezone(pytz.utc)
datetime.datetime(2014, 8, 15, 3, 3, 50, tzinfo=<UTC>)
>>> time.mktime(dt.astimezone(pytz.utc).timetuple())
1408071830.0

time模块的mktime方法支持从timetuple取得timestamp，datetime对象可以直接转换成timetuple。这时候直接使用time.mktime(dt.timetuple())看起来就是很自然的获取timestamp方法。但是我们注意到timetuple方法是直接把当前时间的年月日时分秒直接取出来的。所以这个转换过程在timetuple这个方法这一步丢了时区信息。根据timestamp的定义，正确的方法是把datetime对象利用asttimezone显式转换成UTC时间。

第二，datetime和date以及time的关系 datetime模块同时提供了datetime对象，time对象，date对象。他们之间的关系可以从如下代码简单看出来。

>>> d = datetime.date(2014, 8, 20)
>>> t = datetime.time(11, 30)
>>> dt = datetime.datetime.combine(d, t)
datetime.datetime(2014, 8, 20, 11, 30)
>>> dt.date()
datetime.date(2014, 8, 20)
>>> dt.time()
datetime.time(11, 30)

>>> dt = datetime.datetime.fromtimestamp(1405938446, pytz.timezone('UTC'))
datetime.datetime(2014, 7, 21, 10, 27, 26, tzinfo=<UTC>)
>>> dt.date()
datetime.date(2014, 7, 21)
>>> dt.time()
datetime.time(10, 27, 26)
>>> dt.timetz()
datetime.time(10, 27, 26, tzinfo=<UTC>)

>>> datetime.datetime.combine(dt.date(), dt.time())
datetime.datetime(2014, 7, 21, 10, 27, 26)
>>> datetime.datetime.combine(dt.date(), dt.timetz())
datetime.datetime(2014, 7, 21, 10, 27, 26, tzinfo=<UTC>)

简单说就是，datetime可以取得date和time对象，datetime和time对象可以带timezone信息。date和time对象可以使用datetime.datetime.combine合并获得datetime对象。

第三，日期的加减 datetime，date对象都可以使用timedelta来进行。
    直接看代码

>>> d1 = datetime.datetime(2014, 5, 20)
>>> d2 = d1+datetime.timedelta(days=1, hours=2)
>>> d1
datetime.datetime(2014, 5, 20, 0, 0)
>>> d2
datetime.datetime(2014, 5, 21, 2, 0)
>>> x = d2 - d1
>>> x
datetime.timedelta(1, 7200)
>>> x.seconds
7200
>>> x.days
1

    第四，如何对datetime对象正确设置timezone信息

先看代码。

>>> ddt1 = datetime.datetime(2014, 8, 20, 10, 0, 0, 0, pytz.timezone('Asia/Shanghai'))
>>> ddt1
datetime.datetime(2014, 8, 20, 10, 0, tzinfo=<DstTzInfo 'Asia/Shanghai' LMT+8:06:00 STD>)
>>> ddt2
 datetime.datetime(2014, 8, 20, 11, 0)

>>> ddt1.astimezone(pytz.utc)
datetime.datetime(2014, 8, 20, 1, 54, tzinfo=<UTC>)
>>> ddt2.astimezone(pytz.utc)
ValueError: astimezone() cannot be applied to a naive datetime

>>> tz = timezone('Asia/Shanghai')
>>> tz.localize(ddt1)
ValueError: Not naive datetime (tzinfo is already set)
>>> tz.localize(ddt2)
datetime.datetime(2014, 8, 20, 11, 0, tzinfo=<DstTzInfo 'Asia/Shanghai' CST+8:00:00 STD>)

这里抛出来的ValueError，引入了一个naive datetime的概念。简单说naive datetime就是不知道时区信息的datetime对象。没有timezone信息的datetime理论上讲不能定位到具体的时间点。所以对于设定了timezone的datetime对象，可以使用astimezone方法将timezone设定为另一个。对于不包含timezone的datetime对象，使用timezone.localize方法设定timezone。

但是，这里有没有发现一个问题？我们明明设定的是11点整的，使用astimezone之后跑出来个54分是想怎样？

我们注意到，datetime直接传入timezone对象构建出来的带timezone的datetime对象和使用locallize方法构建出来的datetime对象，在打印出来的时候tzinfo显示有所不同，一个是LMT+8:06，一个是CST+8:00，不用说了，54分就搁这来的吧。LMT学名Local Mean Time，用于比较平均日出时间的。有兴趣的可以自己看看Shanghai和Urumqi的LMT时间。CST是China Standard Time，不用解释了。根据pytz的文档，

Unfortunately using the tzinfo argument of the standard datetime constructors ‘’does not work’’ with pytz for many timezones.
It is safe for timezones without daylight saving transitions though, such as UTC:
The preferred way of dealing with times is to always work in UTC, converting to localtime only when generating output to be read by humans.
...
You can take shortcuts when dealing with the UTC side of timezone conversions. normalize() and localize() are not really necessary when there are no daylight saving time transitions to deal with.

我们按照这个说法再试试看，如下，这回pytz.timezone('Asia/Shanghai’)没有再玩幺蛾子了。

>>> x = datetime.datetime(2014, 8, 20, 10, 0, 0, 0, pytz.utc)
>>> x
datetime.datetime(2014, 8, 20, 10, 0, tzinfo=<UTC>)
>>> x.astimezone(pytz.timezone('Asia/Shanghai'))
datetime.datetime(2014, 8, 20, 18, 0, tzinfo=<DstTzInfo 'Asia/Shanghai' CST+8:00:00 STD>)

所以最保险的方法是使用locallize方法构造带时区的时间。

顺带说下，里面提到了normalize是用来校正计算的时间跨越DST切换的时候出错的情况，还是参见文档，关键部分摘录如下：

This library differs from the documented Python API for tzinfo implementations; if you want to create local wallclock times you need to use the localize() method documented in this document. In addition, if you perform date arithmetic on local times that cross DST boundaries, the result may be in an incorrect timezone (ie. subtract 1 minute from 2002-10-27 1:00 EST and you get 2002-10-27 0:59 EST instead of the correct 2002-10-27 1:59 EDT). A normalize() method is provided to correct this. Unfortunately these issues cannot be resolved without modifying the Python datetime implementation (see PEP-431).

    回到最初的问题，我程序需要给用户两天后的21点发送通知，这个时间怎么计算？

>>> import pytz
>>> import time
>>> import datetime
>>> tz = pytz.timezone('Asia/Shanghai')
>>> user_ts = int(time.time())
>>> d1 = datetime.datetime.fromtimestamp(user_ts)
>>> d1x = tz.localize(d1)
>>> d1x
datetime.datetime(2015, 5, 26, 1, 43, 41, tzinfo=<DstTzInfo 'Asia/Shanghai' CST+8:00:00 STD>)

>>> d2 = d1x + datetime.timedelta(days=2)
>>> d2
datetime.datetime(2015, 5, 28, 1, 43, 41, tzinfo=<DstTzInfo 'Asia/Shanghai' CST+8:00:00 STD>)
>>> d2.replace(hour=21, minute=0)
>>> d2
datetime.datetime(2015, 5, 28, 21, 0, 41, tzinfo=<DstTzInfo 'Asia/Shanghai' CST+8:00:00 STD>)

基本步骤为，根据时间戳和时区信息构建正确的时间d1x，使用timedelta进行对时间进行加减操作，使用replace方法替换小时等信息。

总结，基本上时间相关的这些方法，大部分你都可以直接按照自己的需要封装到一个独立的utility模块，然后就不需要再去管它了。你要做的是，至少有一个人先正确地管一下。






http://tech.glowing.com/cn/dealing-with-timezone-in-python/

{% highlight python %}
{% endhighlight %}
