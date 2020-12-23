---
title: Python 异步任务队列
layout: post
comments: true
language: chinese
category: [linux, network,python]
keywords: python,异步任务队列,APScheduler,Redis Queue,Celery
description: Python 中有很多的调度系统，这里简单介绍一下常用的，例如 APScheduler、Redis Queue、Celery 等。
---

Python 中有很多的调度系统，这里简单介绍一下常用的，例如 APScheduler、Redis Queue、Celery 等。

<!-- more -->

## 简介

Python 中 [APScheduler](https://pypi.python.org/pypi/APScheduler/) 通常用于跨平台的 cron 操作，可以将任务保存在数据库中，不过比较适合嵌入的应用程序中执行，没有提供独立的执行进程。

## Redis Queue

[Redis Queue, RQ](http://www.python-rq.org) 是一个比 Celery 更加简单的异步任务队列，当然他的功能没有 Celery 多，复杂程度也没有 Celery 大，但它足够简单，其 Broker 只能是 redis 。

{% highlight text %}
# pip install rq

127.0.0.1:6379> type rq:job:fba1419d-2c0a-47a2-83c9-bd614309c92c
hash
127.0.0.1:6379> hgetall rq:job:fba1419d-2c0a-47a2-83c9-bd614309c92c
 1) "status"
     2) "finished"
 3) "origin"
     4) "default"
 5) "description"
     6) "database.tasks.reload.reload(1)"
 7) "created_at"
     8) "2016-10-10T13:54:40Z"
 9) "enqueued_at"
    10) "2016-10-10T13:54:40Z"
11) "timeout"
    12) "180"
13) "data"
    14) "\x80\x02(X\x1c\x00\x00\x00database.tasks.reload.reloadq\x01NK\x01\x85q\x02}q\x03tq\x04."
15) "started_at"
    16) "2016-10-10T13:54:40Z"
17) "ended_at"
    18) "2016-10-10T13:54:41Z"
19) "result"
    20) "\x80\x02U\x05xxxxxq\x01."
21) "ttl"
    22) "-1"

ttl rq:job:88fc4ae9-a9c7-4532-98b7-e0c06ef01dbb


lrange rq:queue:default 1 100              通过list类型存放该队列中所含有的任务
smembers rq:queues                         通过set保存了所有队列的信息
{% endhighlight %}

任务会在 job 执行后调用 cleanup() 函数，默认会设置为 result_ttl 值。

<!--
### 任务下发

enqueue_job()

timeout specifies the maximum runtime of the job before it'll be considered 'lost'
result_ttl specifies the expiry time of the key where the job result will be stored
ttl specifies the maximum queued time of the job before it'll be cancelled
depends_on specifies another job (or job id) that must complete before this job will be queued
job_id allows you to manually specify this job's job_id
at_front will place the job at the front of the queue, instead of the back
kwargs and args lets you bypass the auto-pop of these arguments, ie: specify a timeout argument for the underlying job function.
-->

## Celery

Celery 是一个由 Python 编写的简单、灵活、可靠的用来处理大量信息的分布式系统，同时提供一些常用的运维操作工具，通过其提供的接口可以快速实现一个分布式的任务队列。

首先，要理解 Celery 本身不是消息队列，它是管理分布式任务的工具，或者说，它封装好了操作常见任务队列的各种操作，用它可以快速进行任务队列的使用与管理。

当然，也可以自己基于 RabbitMQ 等自己实现，但是成本会更高。

官方给出的解释如下：

{% highlight text %}
Celery is an asynchronous task queue/job queue based on distributed message passing.
It is focused on real-time operation, but supports scheduling as well.
{% endhighlight %}

### 常用概念

![Python Celery Process]({{ site.url }}/images/python/python-celery-process.png "Python Celery Process"){: .pull-center width="60%" }

#### Brokers

指任务队列本身，Celery 扮演生产者和消费者的角色，常见的有 RabbitMQ、Redis 等。

#### Result Stores / Backend

也即是保存运行结果的地方，可以是 Redis、Memcached 等缓存，也可以是数据库。

#### Workers

任务队列的消费者，从队列中取出任务并执行。

#### Tasks

也就是具体的任务了，一般由用户、触发器或其他操作将任务入队，然后交由 workers 进行处理。

### 安装

{% highlight text %}
----- 可以直接通过pip或者easy_install直接安装
# pip install -U Celery
# easy_install -U Celery

----- 直接源码安装，会依赖setuptools工具，可以按需安装
$ python setup.py build
# python setup.py install
{% endhighlight %}

然后将路径 `/usr/local/python27-13/bin/` 添加到环境变量 PATH 中。

在安装 setuptools 时，遇到了 `ImportError: No module named packaging.version` 报错，主要是由于 pip 安装的问题，可以通过如下方式解决。

{% highlight text %}
$ wget https://bootstrap.pypa.io/get-pip.py
# python get-pip.py
{% endhighlight %}

另外，也可以降低版本解决。

### 示例

#### 实现一个 Worker

实现一个任务，等待执行。

{% highlight python %}
# tasks.py
from celery import Celery
app = Celery('tasks',  backend='redis://localhost:6379/0', broker='redis://localhost:6379/1')
@app.task
def add(x, y):
    return x + y
{% endhighlight %}

OK，到这里，broker 我们有了，backend 我们有了，task 我们也有了，现在就该运行 worker 进行工作了，在 tasks.py 所在目录下运行：

{% highlight text %}
$ celery -A tasks worker --loglevel=info
{% endhighlight %}

意思就是运行 tasks 这个任务集合的 worker 进行工作，此时 Redis 中还不包含任务，所以 worker 相当于待命状态。

#### 实现一个触发器

接着就是触发任务执行，最简单方式是再写一个脚本然后调用那个被装饰成 task 的函数。

{% highlight python %}
# trigger.py
import time
from tasks import add
result = add.delay(4, 4)
while not result.ready():
    time.sleep(1)
print 'task done: {0}'.format(result.get())
{% endhighlight %}

delay 返回的是一个 AsyncResult 对象，里面存的就是一个异步的结果，当任务完成时 `result.ready()` 为 true，然后用 `result.get()` 取结果即可。

这里实际上会一直循环等待，查询任务是否结束，也可以通过 `result.get(timeout=10)` 设置超时时间。

#### 查看结果

在任务执行的 Worker 中会显示任务的 UUID 信息，其执行结果会保存到 Redis 编号为 0 的数据库中，类型为 string(实际就是字节流)，Key 在开头会添加 `celery-task-meta-` 前缀。

{% highlight text %}
redis-cli
> select 0
> keys *
{% endhighlight %}

在 broker 中，也就是 Redis-1 数据库，包含了如下内容：

{% highlight text %}
127.0.0.1:6379[1]> keys *
1) "_kombu.binding.celeryev"       类型为set，
2) "_kombu.binding.celery"         类型为set，估计是用于标示这个是Celery实例
3) "_kombu.binding.celery.pidbox"  类型为set，记录了当前含有哪些Worker
4) "celery"                        类型为list，保存了当前的任务信息，从Producer传递给Worker，为空时会删除

> smembers "_kombu.binding.celery" 查看集合信息
> lrange celery 0 5                部分任务信息，暂时还没有看懂
{% endhighlight %}

<!--
Celery易于与Web框架集成, 作者常采用的交互逻辑是:

提供提交任务, 查询任务结果两个API, 由客户端决定何时查询结果

采用websocket等技术, 服务器主动向客户端发送结果

当然也可以采用异步IO模式, 这需要一些扩展包的协助:

安装tornado-celery: pip install torando-celery

编写handler:

import tcelery
tcelery.setup_nonblocking_producer()

from demo.tasks import add

calss Users(RequestHandler):
    @asynchronous
    def get():
        add.apply_async(args=[1,2], callback=self.on_success)

    def on_success(self, response):
        users = response.result
        self.write(users)
        self.finish()
其它的Web框架也有自己的扩展包:
-->

### 高阶用法

#### 根据任务状态进行不同处理

{% highlight python %}
# tasks.py
from celery import Celery, Task
app = Celery('tasks',  backend='redis://localhost:6379/0', broker='redis://localhost:6379/1')

class MyTask(Task):
    def on_success(self, retval, task_id, args, kwargs):
        print 'task done: {0}'.format(retval)
        return super(MyTask, self).on_success(retval, task_id, args, kwargs)
    def on_failure(self, exc, task_id, args, kwargs, einfo):
        print 'task fail, reason: {0}'.format(exc)
        return super(MyTask, self).on_failure(exc, task_id, args, kwargs, einfo)
@app.task(base=MyTask)
def add(x, y):
    # raise KeyError
    return x + y
{% endhighlight %}

#### 绑定任务为实例方法

实际上就是将任务信息通过第一个参数传入，可以获取关于任务的一些相关信息。

{% highlight python %}
# tasks.py
from celery.utils.log import get_task_logger
from celery import Celery, Task
app = Celery('tasks',  backend='redis://localhost:6379/0', broker='redis://localhost:6379/1')
logger = get_task_logger(__name__)

@app.task(bind=True)
def add(self, x, y):
    logger.info(self.request.__dict__)
    return x + y
{% endhighlight %}

传入的第一个参数包含了当前任务的信息，相关信息可以查看 [Celery Task Request](http://docs.celeryproject.org/en/latest/userguide/tasks.html#task-request-info)，例如判断链式任务是否到结尾等等。上述示例，同时会将任务信息打印到日志中。

#### 重试方法

{% highlight python %}
# tasks.py
from celery.utils.log import get_task_logger
from celery import Celery, Task
app = Celery('tasks',  backend='redis://localhost:6379/0', broker='redis://localhost:6379/1')
logger = get_task_logger(__name__)

@app.task(bind=True)
def div(self, x, y):
    logger.info('doing div')
    try:
        result = x / y
    except ZeroDivisionError as e:
        raise self.retry(exc=e, countdown=5, max_retries=3)
    return result
{% endhighlight %}

#### 任务状态回调

Celery 内建有如下几种任务状态：

* PENDING 任务等待中
* STARTED 任务已开始
* SUCCESS 任务执行成功
* FAILURE 任务执行失败
* RETRY 任务将被重试
* REVOKED 任务取消

例如有个比较耗时的任务在运行，需要我们自定义一个任务状态来说明进度并手动更新状态，从而告诉回调当前任务的进度。

{% highlight python %}
# tasks.py
import time
from celery import Celery
app = Celery('tasks',  backend='redis://localhost:6379/0', broker='redis://localhost:6379/1')

@app.task(bind=True)
def foobar(self):
    for i in xrange(1, 11):
        time.sleep(0.3)
        self.update_state(state="PROGRESS", meta={'p': i*10})
    return 'finish'
{% endhighlight %}

{% highlight python %}
# trigger.py
import sys
from task import foobar
 
def pm(body):
    res = body.get('result')
    if body.get('status') == 'PROGRESS':
        sys.stdout.write('\rProcessing: {0}%'.format(res.get('p')))
        sys.stdout.flush()
    else:
        print '\r'
        print res
print foobar.delay().get(on_message=pm, propagate=False)
{% endhighlight %}

注意，在 4.0 之后的版本，对于 backend 为 AMQP 处于性能的考虑会直接删除掉 `on_message` 的异步功能，如果要使用最好用数据库。

在使用 AMQP 作为 result backend 时，Celery 会试着模拟持久化结果集，当任务并发过千时，且过期时间超过一天，那么就是导致 AMQP 性能变差，为此对于 AMQP 的后端就取消掉了这一功能。

#### 定时/周期任务

只需要在配置中配置好周期任务，然后在运行一个周期任务触发器即可。

{% highlight python %}
# celery_config.py
from datetime import timedelta
from celery.schedules import crontab
CELERY_TIMEZONE = 'Asia/Shanghai'
CELERYBEAT_SCHEDULE = {
    'ptask': {
        'task': 'tasks.period_task',
        'schedule': timedelta(seconds=5),
    },
}
CELERY_RESULT_BACKEND = 'redis://localhost:6379/0'
{% endhighlight %}

配置中 schedule 就是间隔执行的时间，这里可以用 `datetime.timedelta` 或者 crontab 甚至太阳系经纬度坐标进行间隔时间配置，具体可以参考 [Crontab Schedules](http://docs.celeryproject.org/en/latest/userguide/periodic-tasks.html#crontab-schedules) 。

默认使用的是 UTC ，可以通过 `CELERY_TIMEZONE` 设置时区信息。

然后在 tasks.py 中增加要被周期执行的任务。

{% highlight python %}
# tasks.py
app = Celery('tasks', backend='redis://localhost:6379/0', broker='redis://localhost:6379/0')
app.config_from_object('celery_config')
@app.task(bind=True)
def period_task(self):
    print 'period task done: {0}'.format(self.request.id)
{% endhighlight %}

重新运行 worker 并通过 `celery -A tasks beat` 执行 beat 。

其它的高端用法还有链式任务，除了使用本地的 Python 文件作为 Worker 外，还可以通过 WebHook 实现，允许使用远程的 Web 服务作为 Worker 。


## 参考

官方网站可以参考 [Celery](http://www.celeryproject.org/)，以及其文档 [Celery - Distributed Task Queue](http://docs.celeryproject.org/en/latest/index.html) 。


{% highlight text %}
{% endhighlight %}
