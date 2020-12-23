---
title: Python Logging 模块
layout: post
comments: true
category: [python,program]
language: chinese
keywords: python,模块,logging
description: 用 Python 写代码时，经常需要打印日志，其实内部提供了一个灵活的 logging 模块，基本可以满足绝大部分的需求，如下简单介绍其使用方式。
---


<!-- more -->

## functools 模块

functools 是 python2.5 引人的，可以参考官方文档 [functools — Higher-order functions and operations on callable objects](https://docs.python.org/2/library/functools.html)

### reduce

这个与 Python 内置的 reduce 相同，每次迭代，将上一次的迭代结果与下一个元素一同执行一个二元的 func 函数。

{% highlight python %}
import functools
a = range(1, 6)
functools.reduce((lambda x,y:x+y), a)  # 1+2+3+4+5=15
reduce((lambda x,y:x+y), a)            # 1+2+3+4+5=15
{% endhighlight %}

### partial

对于一个带 n 个参数函数，partial 会将第一个参数设置为固定参数，并返回一个带 n-1 个参数函数对象。

{% highlight python %}
from operator import add
from functools import partial
add3 = partial(add, 3)
print add3(7)            # 3+7=10
{% endhighlight %}


## collections 模块

该三方模块提供了对内置类型的扩展，是 Python 内建的一个集合模块，提供了许多有用的集合类，包括了多个对象，详细可以查看官方文档 [collections High-performance container datatypes](https://docs.python.org/2/library/collections.html) 。

### defaultdict

在使用 Python 内置的 dict 时，如果引用的 Key 不存在，就会抛出 KeyError，如果希望 key 不存在时，返回一个默认值，就可以用 defaultdict。

{% highlight text %}
>>> from collections import defaultdict
>>> dd = defaultdict(lambda: 'N/A')
>>> dd['key1'] = 'abc'
>>> dd['key1']                       ← key1存在
'abc'
>>> dd['key2']                       ← key2不存在，返回默认值
'N/A'
{% endhighlight %}

注意默认值是调用函数返回的，而函数在创建 defaultdict 对象时传入，除了在 Key 不存在时返回默认值外，该对象的其他行为跟 dict 是完全一样的。

### OrderedDict

使用 Python 内置的字典对象时，Key 是无序的，这样当对 dict 做迭代时，无法确定 Key 的顺序，如果要保持 Key 的顺序，可以用 OrderedDict 。

{% highlight text %}
>>> from collections import OrderedDict
>>> d = dict([('a', 1), ('b', 2), ('c', 3)])
>>> d                                ← dict的Key是无序的
{'a': 1, 'c': 3, 'b': 2}

>>> od = OrderedDict([('a', 1), ('b', 2), ('c', 3)])
>>> od                               ← OrderedDict的Key是有序的
OrderedDict([('a', 1), ('b', 2), ('c', 3)])
{% endhighlight %}

OrderedDict 的 Key 会 **按照插入的顺序排列** ，不是 Key 本身排序：

{% highlight text %}
>>> od = OrderedDict()
>>> od['z'] = 1
>>> od['y'] = 2
>>> od['x'] = 3
>>> od.keys()                        ← 按照插入的Key的顺序返回
['z', 'y', 'x']
{% endhighlight %}

另外，通过 OrderedDict 可以实现一个 FIFO (先进先出) 的 dict，当容量超出限制时，先删除最早添加的 Key：

{% highlight python %}
from collections import OrderedDict
class LastUpdatedOrderedDict(OrderedDict):
    def __init__(self, capacity):
        super(LastUpdatedOrderedDict, self).__init__()
        self._capacity = capacity
    def __setitem__(self, key, value):
        containsKey = 1 if key in self else 0
        if len(self) - containsKey >= self._capacity:
            last = self.popitem(last=False)
            print 'remove:', last
        if containsKey:
            del self[key]
            print 'set:', (key, value)
        else:
            print 'add:', (key, value)
        OrderedDict.__setitem__(self, key, value)
{% endhighlight %}

### namedtuple

在 Python 中，tuple 可以 **表示不变集合** ，例如，一个点的二维坐标就可以表示成 (1, 2)。但是，看到 (1, 2) 后，很难看出这个 tuple 是用来表示一个坐标的，不过定义一个类又小题大做了，这时就可以使用该对象了。

{% highlight text %}
>>> from collections import namedtuple
>>> P = namedtuple('Point', ['x', 'y'])
>>> p = P(1, 2)
>>> print p
Point(x=1, y=2)
>>> p.x
1
>>> p.y
2
>>> p.x = 3                          ← 只读变量，设置会报错
Traceback (most recent call last):
  File "<stdin>", line 1, in <module>
AttributeError: can't set attribute
{% endhighlight %}

<!-- ' -->

namedtuple 是一个函数，它用来创建一个自定义的 tuple 对象，并且规定了 tuple 元素的个数，并可以用属性而不是索引来引用 tuple 的某个元素。这样一来，我们用 namedtuple 可以很方便地定义一种数据类型，它具备 tuple 的不变性，又可以根据属性来引用，使用十分方便。

对于如上的示例，可以通过如下方式验证创建的 Point 对象是 tuple 的一种子类：

{% highlight text %}
>>> isinstance(p, Point)
True
>>> isinstance(p, tuple)
True
{% endhighlight %}

### deque

使用 list 存储数据时，按索引访问元素很快，但是插入和删除元素就很慢了，因为 list 是线性存储，数据量大的时候，插入和删除效率很低。deque 是为了高效实现插入和删除操作的双向列表，适合用于队列和栈：

{% highlight text %}
>>> from collections import deque
>>> q = deque(['a', 'b', 'c'])
>>> q.append('x')
>>> q.appendleft('y')
>>> q
deque(['y', 'a', 'b', 'c', 'x'])
{% endhighlight %}

deque 除了实现 list 的 append() 和 pop() 外，还支持 appendleft() 和 popleft()，这样就可以非常高效地往头部添加或删除元素。

## JSON 模块

JSON (JavaScript Object Notation) 是一种轻量级的数据交换格式，采用完全独立于语言的文本格式，是 "名称/值" 的集合，详细可以参考 [官方文档](http://json.org/) 。Python2.6 开始加入了 JSON 标准模块，序列化与反序列化的过程分别是 encoding 和 decoding 。

对于简单数据类型，如 string、unicode、int、float、list、tuple、dict，可以直接处理，详细的也可以参考 [Python 操作 json 的标准 api 库](http://docs.python.org/library/json.html) 。

{% highlight python %}
import json

print(json.dumps({'4': 5, '6': 7}, sort_keys=True, indent=4))

json.loads('["foo", {"bar":["baz", null, 1.0, 2]}]')
{% endhighlight %}

<!--
urllib 和 urllib2 的区别
http://www.hacksparrow.com/python-difference-between-urllib-and-urllib2.html
-->

{% highlight python %}
{% endhighlight %}
