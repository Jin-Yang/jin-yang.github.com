---
title: Python 语法简介
layout: post
comments: true
language: chinese
category: [program, python]
keywords: python,class,static method,class method
description:
---


<!-- more -->


## 异常处理

![python exception hierarchy]({{ site.url }}/images/python/exception-hierarchy.png "python exception hierarchy"){: .pull-center}

Python 的异常处理会按照继承方式处理，例如触发异常可以使用 `raise ValueError("Some message")` 。

### 异常处理方式

在写程序时可以通过如下方式进行处理。

#### 捕获所有异常

{% highlight python %}
try:
	//... ...
except Exception, e:
	print Exception, ":", e
{% endhighlight %}

#### traceback 查看栈

{% highlight python %}
import traceback

try:
	//... ...
except:
	traceback.print_exc()
{% endhighlight %}

<!--
----------------------------------------------------------------------------
方法三：采用sys模块回溯最后的异常
----------------------------------------------------------------------------
code:

import sys
try:
a=b
b=c
except:
info=sys.exc_info()
print info[0],":",info[1]

--------------------------------------------------------------------------------

但是，如果你还想把这些异常保存到一个日志文件中，来分析这些异常，那么请看下面的方法：
把　traceback.print_exc()　打印在屏幕上的信息保存到一个文本文件中
code:

try:
a=b
b=c
except:
f=open("c:\\log.txt",'a')
traceback.print_exc(file=f)
f.flush()
f.close()
-->

### 示例代码

{% highlight python %}
#!/usr/bin/python
"This is a module to display how exception works."      # called by module.__doc__

class SimpleError(Exception):
    pass

class MyError(Exception):
    def __init__(self, value):
        self.value = value
    def __str__(self):
        return repr(self.value)


def examples_except():
    '''If the exception or error not catched by any
       code, then the program will exit.'''

    try:  # normal: ACD, except: ABD
        print "This is normal operation."          # A
        #raise TypeError
        #raise IOError, 'Opps, IO error'
        #raise NameError, (2, 'Opps, Name error')
        raise MyError, 8  # OR raise MyError(8)
    except IOError:
        print 'Ooooh, IOError'
        # raise   # reraise the exception, no arguments.
        retval = 'Exception IOError:'              # B
    except TypeError, e:
        print 'Ooooh, TypeError:', e # or e.args[0], e.args[1]
        #print dir(e)
        retval = 'Exception TypeError:', e         # B
    except NameError, (errno, e):
        print 'Ooooh, NameError:', errno, e
        retval = 'Exception IOError:', e           # B
    except MyError, e:
        print 'Ooooh, MyError:', e.value
        retval = 'Exception MyError'               # B
    except (SyntaxError, TypeError):
        print 'Ooooh, SyntaxError/TypeError'
        retval = 'Exception ValueError,TypeError'  # B
    except (KeyError, TypeError), e:
        print 'Ooooh, KeyError/TypeError'
        retval = str(diag) # Maybe e.args[0] and e.args[1] can use.
    except Exception:  # or 'except:', or 'BaseException:' all errors.
        print 'Ooooh, Exception'
        retval = 'some errors cannot identify'
    else:   # This is optional, means there is no error.
        print 'This is also normal operation.'     # C
    finally: # This always execute.
        print 'Normal operation.'                  # D
    # NOTE: if using 'try ... finally ...', when finally block
    #       finished, the exceptions will be reraised.

    import time
    print "\ntest for Ctrl-C"
    try:
        try:
            time.sleep(20)
        except KeyboardInterrupt, e:
            print "inner"
            raise
    except KeyboardInterrupt, e:
        print "outer"

    print
    try:
        assert 1 == 0 , 'One does not equal zero silly!'
    except AssertionError, args:
        print '%s: %s' % (args.__class__.__name__, args)

if __name__ == '__main__': # main function, else module name.
    ret = examples_except()
{% endhighlight %}


## 类 Class

### StaticMethod VS. ClassMethod

两者十分相似，但在用法上依然有一些明显的区别：A) ClassMethod 必须有一个指向类对象的引用作为第一个参数；B) StaticMethod 可以没有任何参数。

如下是几个示例：

{% highlight python %}
# -*- coding: utf8 -*-

class Date(object):

    def __init__(self, day=0, month=0, year=0):
        self.day = day
        self.month = month
        self.year = year

    @classmethod
    def from_string(cls, date_as_string):
        day, month, year = map(int, date_as_string.split('-'))
        return cls(day, month, year)

    @staticmethod
    def is_date_valid(date_as_string):
        day, month, year = map(int, date_as_string.split('-'))
        return day <= 31 and month <= 12 and year <= 3999

print Date.from_string('11-09-2012')
print Date.is_date_valid('11-09-2012')
{% endhighlight %}

也就是说，StaticMethod 没有任何必选参数，而 ClassMethod 第一个参数永远是 cls， InstanceMethod 第一个参数永远是 self。

所以，从静态方法的使用中可以看出，不会访问到 class 本身，它基本上只是一个函数，在语法上就像一个方法一样，相反 ClassMethod 会访问 cls， InstanceMethod 会访问 self。



{% highlight python %}
{% endhighlight %}
