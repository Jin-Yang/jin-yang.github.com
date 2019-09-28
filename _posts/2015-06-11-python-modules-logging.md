---
title: Python Logging 模块
layout: post
comments: true
category: [python,program]
language: chinese
keywords: python,模块,logging
description: 用 Python 写代码时，经常需要打印日志，其实内部提供了一个灵活的 logging 模块，基本可以满足绝大部分的需求，如下简单介绍其使用方式。
---

用 Python 写代码时，经常需要打印日志，其实内部提供了一个灵活的 logging 模块，基本可以满足绝大部分的需求，如下简单介绍其使用方式。

<!-- more -->

## 简介

Python 中使用最多的日志打印模块，采用层级管理，支持多线程，logging 模块包含了四大组件：

* Logger 提供了应用程序可使用的接口；
* Handler 将 logger 创建的日志记录发送到合适的目的输出，例如文件、标准输出等；
* Filter 提供了更细粒度的控制工具来决定输出哪条日志记录，丢弃哪条日志记录；
* Formatter 决定日志记录的最终输出格式。

获取 logger 对象可以通过 Logger 类直接创建实例，不过常用 `logging.getLogger()` 方法获取，入参是一个可选的 `name` 标示，默认是 `root` ，当以相同的 `name` 参数调用时，返回的对象相同。

logger 层级结构与 name 相关，其名字以 `.` 分割层级，例如，有一个名称为 `foo` 的 logger，其它名称分别为 `foo.bar`、`foo.bar.baz` 和 `foo.bam` 都是 `foo` 的后代。

<!---
logger有一个"有效等级（effective level）"的概念。如果一个logger上没有被明确设置一个level，那么该logger就是使用它parent的level;如果它的parent也没有明确设置level则继续向上查找parent的parent的有效level，依次类推，直到找到个一个明确设置了level的祖先为止。需要说明的是，root logger总是会有一个明确的level设置（默认为 WARNING）。当决定是否去处理一个已发生的事件时，logger的有效等级将会被用来决定是否将该事件传递给该logger的handlers进行处理。
-->

child 完成对日志处理后，默认将日志消息传递给与其祖先 loggers 相关的 handlers，因此，通常只需要设置顶层的 logger 及其 handlers ，然后按需设置子类。

也可以将 logger 的 propagate 属性设置为 False 来关闭这种传递机制。

<!---
常见的错误输出方式如下所示。<ul><li>
对于程序正常的输出使用，如提示输入密码等：print()。</li><li>
输出正常执行过程，通常用于状态监测和错误诊断：loging.info()/logging.debug()。debug()用于显示更详细的信息。</li><li>
针对于正常流程的事件：logging.warning()。用户程序不能做什么，但是仍需要提示。</li><li>
针对运行时期的一个事件报告一个错误：抛出异常。</li><li>
输出错误并退出：logging.error()/logging.exception()/logging.critical()。</li></ul>
整个模块由四部分组成：loggers(暴露给应用程序的接口), handlers(将loggers创建的记录分发给不同目的地), filters(过滤那些记录需要输出), formatters(指定记录的格式)。
-->

## 使用示例

默认会输出到终端，可以直接类似如下方式使用。


{% highlight python %}
import logging

logging.basicConfig(
	level=logging.DEBUG,
	format='%(asctime)s - %(filename)s[line:%(lineno)d] - %(levelname)s: %(message)s')
logging.info('this is a loggging info message')
logging.debug('this is a loggging debug message')
logging.warning('this is loggging a warning message')
logging.error('this is an loggging error message')
logging.critical('this is a loggging critical message')
{% endhighlight %}

更多示例如下。

{% highlight python %}
import logging

format ='[%(levelname)8s]\t (%(threadName)10s)\t %(message)30s')

logger = logging.getLogger('mylogger')    # 创建一个 logger ,默认使用 root 作为名称
logger.setLevel(logging.DEBUG)            # DEBUG/INFO/WARNING/ERROR/CRITICAL
logger.addHandler()
logger.addFilter()
logger.debug()

fh = logging.FileHandler('test.log')      # 创建一个 handler ，用于写入日志文件
fh.setLevel(logging.DEBUG)
fh.setFormatter()
fh.addFilter()
fh.removeFilter()

ch = logging.StreamHandler()              # 再创建一个 handler ，用于输出到控制台
ch.setLevel(logging.DEBUG)

# 定义handler的输出格式
formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
fh.setFormatter(formatter)
ch.setFormatter(formatter)

# Logger.addHandler() Logger.removeHandler()
# Logger.addFilter()  Logger.removeFilter()
#
logger.addHandler(fh)                     # 给 logger 添加 handler
logger.addHandler(ch)

logger.info('foorbar')                    # 记录一条日志
{% endhighlight %}


## 日志处理流程

日志的写入流程如下图所示。

![logging process]({{ site.url }}/images/python/logging-process.jpg "logging process"){: .pull-center width="100%" }

简单描述下日志的处理流程：

1. 用户代码中调用日志记录函数，如 `logger.info(...)`、`logger.debug(...)` 等；
2. 判断日志级别是否满足，可以通过 `logger.setLevel(logging.DEBUG)` 进行设置；
3. 根据日志记录函数调用时掺入的参数，创建一个日志记录 (`Class LogRecord`) 对象；
4. 判断 logger 上设置的过滤器规则，不满足则将日志记录交给该 logger 上的各个 handler；
5. 判断要记录的日志级别是否满足 handler 设置的级别要求；

<!--
6）判断该处理器上设置的过滤器是否拒绝这条日志记录，如果该处理器上的某个过滤器拒绝，则该日志记录会被当前处理器丢弃并终止后续的操作，如果当前处理器上设置的过滤器不拒绝这条日志记录或当前处理器上没有设置过滤器测继续下一步操作；
7）如果能到这一步，说明这条日志记录经过了层层关卡允许被输出了，此时当前处理器会根据自身被设置的格式器（如果没有设置则使用默认格式）将这条日志记录进行格式化，最后将格式化后的结果输出到指定位置（文件、网络、类文件的Stream等）；
8）如果日志器被设置了多个处理器的话，上面的第5-8步会执行多次；
9）这里才是完整流程的最后一步：判断该日志器输出的日志消息是否需要传递给上一级logger（之前提到过，日志器是有层级关系的）的处理器，如果propagate属性值为1则表示日志消息将会被输出到处理器指定的位置，同时还会被传递给parent日志器的handlers进行处理直到当前日志器的propagate属性为0停止，如果propagate值为0则表示不向parent日志器的handlers传递该消息，到此结束。
可见，一条日志信息要想被最终输出需要依次经过以下几次过滤：
日志器等级过滤；
日志器的过滤器过滤；
日志器的处理器等级过滤；
日志器的处理器的过滤器过滤；
-->

注意，其中有一步如果 `propagate` 值为 1，那么日志会直接传递给上级 logger 的 handlers 进行处理，此时上一级 logger 的日志等级并不会对该日志消息进行等级过滤。

<!--
Logger命名采用父子继承关系，使用 getLogger()函数，默认为root，所有的记录都会传递给父节点，可以将 propagate 设置为 False 取消这一属性。注意如果使用getLogger()创建时没有指定等级，则继承使用父节点等级。<br><br>

Handlers指定分发的对象，如文件、终端等。<br><br>

运行后， 在控制台和日志文件都有一条日志。日志采用父子关系 logger 的 name 的命名方式可以表示 logger 之间的父子关系。比如：<br>
parent_logger = logging.getLogger('foo')<br>
child_logger = logging.getLogger('foo.bar')<br><br>

可以通过 logging.getLogger() 或者 logging.getLogger("") 得到 root logger 的实例。

<ol><li>
logging.getLogger([name])<br>
返回一个 logger 实例，如果没有指定 name ，返回 root logger 。只要 name 相同，返回的 logger 实例都是同一个而且只有一个，即 name 和 logger 实例是一一对应的。这意味着，无需把 logger 实例在各个模块中传递。只要知道 name ，就能得到同一个 logger 实例。</li><br><li>

logger.setLevel(lvl)<br>
设置 logger/Handler 的 level ， level 有以下几个级别 NOTSET &lt; DEBUG &lt; INFO &lt; WARNING &lt; ERROR &lt; CRITICAL ，如果把 loger 的级别设置为 INFO ，那么小于 INFO 级别的日志都不输出，大于等于 INFO 级别的日志都输出。</li><br><li>

Logger.addHandler(hdlr)<br>
logger 可以通过 handler 来处理日志， handler 主要有以下两种：A) StreamHandler ，输出到控制台；B) FileHandler ，输出到文件。</li><br><li>

logging.basicConfig([**kwargs])
* 这个函数用来配置root logger， 为root logger创建一个StreamHandler，
设置默认的格式。
* 这些函数： logging.debug()、logging.info()、logging.warning()、
logging.error()、logging.critical() 如果调用的时候发现root logger没有任何
handler， 会自动调用basicConfig添加一个handler
* 如果root logger已有handler， 这个函数不做任何事情

使用basicConfig来配置root logger的输出格式和level：
Python代码  收藏代码

import logging
logging.basicConfig(format='%(levelname)s:%(message)s', level=logging.DEBUG)
logging.debug('This message should appear on the console')





Logger.exception()与Logger.error()相似，区别在于前者会同时 dumps a stack，通常用于异常处理程序。<br>
Logger.log("message", logging.DEBUG)同时会指定logging的级别。<br><br>

可以使用 fileConfig() 通过配置文件对 logging 进行配置，详见HOWTO。
</li></ol>


DRC：Disaster Recovery Center

http://python.jobbole.com/86887/

官方帮助文档 <a href="https://docs.python.org/2/howto/logging.html">Howto</a> ，详细的可以参考 <a href="https://docs.python.org/2/howto/logging-cookbook.html#logging-cookbook">logging cookbook</a>。<br><br>

https://www.cnblogs.com/CJOKER/p/8295272.html
-->







{% highlight python %}
{% endhighlight %}
