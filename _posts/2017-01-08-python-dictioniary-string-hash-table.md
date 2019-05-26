---
title: Python 字典对象实现
layout: post
comments: true
language: chinese
category: [python]
keywords: python,dictionary,字典,hash table,哈希表,冲突
description: 在 Python 中有一个字典，可以看作是一个 Key Value 对，其代码是通过哈希表实现，也就是说，字典是一个数组，而数组的索引是键经过哈希函数处理后得到的。Python 字典是用哈希表 (hash table) 实现，哈希表是一个数组，它的索引是对键运用哈希函数计算求得的。一个好的哈希函数会将冲突数量降到最小，将各个值均匀分布到数组中，而 Python 中的哈希函数 (主要用于字符串和整数) 很常规：冲突时采用开放寻址，相比链表来说，其 CPU Cache 的命中率更高。这里简单结合 Python 中 Hash 函数的实现。
---

在 Python 中有一个字典，可以看作是一个 Key Value 对，其代码是通过哈希表实现，也就是说，字典是一个数组，而数组的索引是键经过哈希函数处理后得到的。

Python 字典是用哈希表 (hash table) 实现，哈希表是一个数组，它的索引是对键运用哈希函数计算求得的。

这里简单结合 Python 中 Hash 函数的实现。

<!-- more -->

## 字典使用

一个好的哈希函数会将冲突数量降到最小，将各个值均匀分布到数组中，不过，对于 Python 中的哈希函数 (主要用于字符串和整数) 很常规，冲突时采用开放寻址法，相比于链表来说，其 CPU Cache 的命中率更高。

如下是经常的使用方法。

{% highlight text %}
----- 创建初始化字典对象
info = {"name" : "foobar", "gender": "male"}
info = dict(name = 'foobar', gender = 'male')

----- 对于第二种方式，在如下场景时可能会出现隐藏的bug
key = 'name'
info = { key :'foobar'}     # {'name':'foobar'}
info = dict(key = 'foobar') # {'key': 'foobar'}

----- 可以通过fromkeys函数进行初始化，值默认是None，也可以通过第二个参数指定默认值
info = {}.fromkeys(['name', 'gender'])              # {'gender' : None, 'name' : None}
info = dict().fromkeys(['name', 'gender'])          # {'gender' : None, 'name' : None}
info = {}.fromkeys(['name', 'gender'], 'invalid')   # {'gender' : 'invalid', 'name' : 'invalid'}

----- 获取值
print info['name']                 # 不存在时会触发KeyError异常
print info.get('name')             # 不存在返回None而非异常
print info.get('notexist', 'oops') # 不存在时返回指定的默认值

----- 更新、添加、删除
info['name'] = 'kidding'
info.update({'name':'kidding', 'gender':'female'})
info.update(name='kidding', blog='female')
del info['name']                   # 或者info.pop('name')

d.keys()
for key, value in info.items():
    print key, ':',  value
{% endhighlight %}

如上，key 可以是 int 和 string 的混合。

### setdefault

Python 地址中有一个 `setdefault` 函数，主要是用于获取信息，如果获取不到就按照它的参数设置该值。

{% highlight python %}
a = { "key": "hello world" }
a.setdefault("key", "456"))   # 因为之前已经设置了key对应的值，此时不会设置

a.setdefault("key_sth", "123"))   # 之前没有设置，此时会设置
{% endhighlight %}

## 源码解析

实际上，计算的 Hash 值，可以通过如下的函数进行计算。

{% highlight text %}
>>> map(hash, (0, 1, 2, 3))
[0, 1, 2, 3]
>>> map(hash, ("namea", "nameb", "namec", "named"))
[-6456208310023038713, -6456208310023038716, -6456208310023038715, -6456208310023038718]
{% endhighlight %}

### Entry

准确来说，是 CPython 中的实现，其对应的结构体如下。

{% highlight c %}
typedef struct {
	Py_ssize_t me_hash; // 缓存me_key的hash值，防止每次查询都要计算
	PyObject *me_key;
	PyObject *me_value;
} PyDictEntry;
{% endhighlight %}

每个 `PyDictEntry` 包含了三种状态：

* Unused `me_key == me_value == NULL` 初始状态。
* Active `me_key != NULL and me_key != dummy and me_value != NULL` 插入元素后就成了 Active 状态，这是 `me_value` 唯一不为 `NULL` 的情况，删除元素时 Active 状态刻转换成 Dummy 状态。
* Dummy `me_key == dummy and me_value == NULL`  dummy 实际上一个 `PyStringObject` 对象，仅作为指示标志。

Dummy 的元素可以在插入元素的时候将它变成 Active ，但它不能再变成 Unused 。

#### Why Dummy

为什么 entry 有 Dummy 状态呢？

在开放寻址法中，如果某元素经过哈希计算应该插入到 A 处，但是此时 A 处有元素的，通过探测函数计算得到下一个位置 B，仍然有元素，直到找到位置 C 为止。

此时 ABC 构成了探测链，查找元素时如果 hash 值相同，那么也是顺着这条探测链不断往后找，当删除探测链中的某个元素时，比如 B，如果直接把 B 从哈希表中移除，即变成 Unused 状态，那么 C 就不可能再找到了，因为 AC 之间出现了断裂的现象。

正是如此才出现了第三种状态 Dummy，这是一种类似的伪删除方式，保证探测链的连续性。

### Dict

{% highlight python %}
struct _dictobject {
	PyObject_HEAD
	Py_ssize_t ma_fill;          // Active+Dummy
	Py_ssize_t ma_used;          // Active
	Py_ssize_t ma_mask;          // 总共有ma_mask+1个slots，可以通过key_hash&ma_mask得到对应的slot
	PyDictEntry *ma_table;       // 保存的hash表
	PyDictEntry *(*ma_lookup)(PyDictObject *mp, PyObject *key, long hash);
	PyDictEntry ma_smalltable[PyDict_MINSIZE];
};
{% endhighlight %}

<!--
PyDict_New() 创建新字典对象
 |-PyString_FromString() 第一次会初始化dummy对象
 |-PyObject_GC_New() 如果没有缓存，则通过该函数创建一个

Dict 对象的插入
字典对象的插入实际是通过 PyDict_SetItem() 函数完成，简单来说就是，如果不存在 Key-Value 则插入，存在则覆盖；基本的处理步骤如下：

1. 通过 ma_lookup 所指向的函数得到 key 所对应的 entry，该函数对于字符串来说是 lookdict_string()，整形是 lookdict()；
2. 返回的值分为如下几种场景：
   * me_key = NULL 空不存在，可以直接使用；
   * me_key = key 对应的值已经存在，可以直接返回；
   * me_hash = hash && 字符串相同，不同的 key 对象，但是值相同，同样认为相同；
   * me_key = dummy 对应的值已经删除；
   * 冲突，通过如下方式探测。

PyDict_SetItem()
 |-PyString_CheckExact()  如果是string对象，那么实际会通过string_hash计算hash值
 |-PyObject_Hash()  否则是int类型，则通过int_hash计算hash值
 |-dict_set_item_by_hash_or_entry() 其中entry为0，也就是通过hash添加，会判断是否调整大小
   |-insertdict()  实际调用这里的接口
   | |-ma_lookup() 通过该指针指向的对象查找，一般默认为lookdict_string()函数
   | |-insertdict_by_entry()
   |-insertdict_by_entry()
   |-dictresize() 只有在插入的时候会调整字典的大小

TODO:
  校验下，循环中可以替换，但是无法新增或者删除。

https://morepypy.blogspot.com/2015/01/faster-more-memory-efficient-and-more.html
https://www.laurentluce.com/posts/python-dictionary-implementation/
https://www.hongweipeng.com/index.php/archives/1230/
https://zhuanlan.zhihu.com/p/25071851
https://github.com/Junnplus/blog/issues/15
https://juejin.im/entry/5bc57c8ef265da0a972e49d1
https://arianx.me/2018/12/30/walkthrough-cpython3.7-dict-source-code/
https://blog.csdn.net/weixin_33975951/article/details/89698890
https://www.cnblogs.com/adinosaur/p/7259814.html
https://github.com/clibs/logfmt
这里的 Hash 表实际上有两类，分别是 Key-Value 结构，以及 Hash-Object，也就是前者需要在对象中同时保存 Key 和 Value 的值，而后者则直接以 Object 方式存储，只有用户需要知道具体值的含义。

在 Python 3.6 版本中，对字典对象进行了较大的优化，尤其是内存使用效率方面，其基本的东西改动不太大，例如 Hash 值计算方法、冲突解决策略等。

这里简单介绍其实现的详细细节。

## 简介

对于内存的优化有很多方面。

### 成员对象

内存中 dict 对象的成员如下。

+---------------+
| dk_refcnt     |
| dk_size       |   哈希表indices的大小
| dk_lookup     |
| dk_usable     |
| dk_nentries   |
+---------------+
| dk_indices    |   实际hash表，保存了entries中的序列
|               |
+---------------+
| dk_entries    |
|               |
+---------------+

为了尽量减少空间的使用，会根据不同的 dk_size 选择不同类型的整数，也就是 `dk_entreis` 中的序号，方法如下。

int8  for          dk_size <= 128
int16 for 256   <= dk_size <= 2**15
int32 for 2**16 <= dk_size <= 2**31
int64 for 2**32 <= dk_size

其中 `dk_entries` 是 `PyDictKeyEntry` 类型的数组，保存了真正的对象，可以通过 `DK_ENTRIES(dk)` 获取 `dk_entries` 的指针。

之所以采用索引 `dk_indices` 和对象 `dk_entreis` 分离，主要是为了节省空间。

    d = {'timmy': 'red', 'barry': 'green', 'guido': 'blue'}
    entries = [['--', '--', '--'],
               [-8522787127447073495, 'barry', 'green'],
               ['--', '--', '--'],
               ['--', '--', '--'],
               ['--', '--', '--'],
               [-9092791511155847987, 'timmy', 'red'],
               ['--', '--', '--'],
               [-6480567542315338377, 'guido', 'blue']]
    indices =  [None, 1, None, None, None, 0, None, 2]
    entries =  [[-9092791511155847987, 'timmy', 'red'],
                [-8522787127447073495, 'barry', 'green'],
                [-6480567542315338377, 'guido', 'blue']]
### Combined VS. Split

在 `PyDictObject` 中的注释可以看到，实际上在 dict 内存中会保存两种形式：

* Combined `ma_values=NULL dk_refcnt=1` 而对应的 Keys Values 会保存在 ma_keys 中；
* Split `ma_values != NULL dk_refcnt >= 1` 此时 Keys Values 会分别存储在 ma_keys 和 ma_values 中。

新版的 dict 有两种形式，分别是 combined 和 split，后者主要用在优化对象存储属性的 `tp_dict` 上。

这种字典的 key 是共享的，此时会有一个引用计数器 `dk_refcnt` 来维护当前被引用的个数，这一场景使用比较多的是实例对象上的属性字典 tp_dict 。

主要对应这样的场景：

1. 一个类会创建出很多个对象；
2. 这些对象的属性，能在一开始就确定下来，并且后续不会增加删除。

如果能满足上述两个条件，那么可以使用一种更高效、更省内存的方式，来存储对象的属性。

也就是，属于一个类的所有对象共享同一份属性字典的 key，而 value 以数组的方式存储在每个对象的身上，这样只需要维护一份对象属性即可，而值则可以更加紧凑的方式组织在内存中。

PEP 412 -- Key-Sharing Dictionary
https://www.python.org/dev/peps/pep-0412/


## 源码介绍

与字典相关的对象实现在 `Objects/dictobject.c` `Include/dictobject.h` `Objects/dict-common.h` 文件中，

### 结构体

#### Key Value

// Objects/dict-common.h
typedef struct {
    Py_hash_t me_hash; // 缓存计算的hash值
    PyObject *me_key;
    PyObject *me_value;
} PyDictKeyEntry;

每次向字典写入数据时，实际上就会向哈希表中插入一个 PyDictKeyEntry，包括了 Key 和 Value 的值。其中 `Py_hash_t` 一般是 `ssize_t` ，所以如果是 64 位的系统，那么整个结构体会占用 24 字节。

#### 字典

在 `PyDictKeysObject` 对象中储存了实际的哈希表，但是这里保存的大部分是与该字典相关的元数据，没有对应 `PyDictKeyEntry` 数组之类的成员来保存 Hash 表。

实际上，对应的是 `dk_indices` ，这是一个指针，在 3.6 之后为了提高内存使用效率做了双重映射，老版本的实际会保存一个 `PyDictKeyEntry*` 的数组。

#### 解析器对象

上述介绍的都是 CPython 内部实现的一些结构体，而真正暴露给解析器的对象是 `PyDictObject` 。

typedef struct {
    PyObject_HEAD               // 对象通用的头
    Py_ssize_t ma_used;         // 字典元素个数，len()函数依赖该字段，所以在这里暴露出来
    uint64_t ma_version_tag;    // 全局版本，优化后面讲
    PyDictKeysObject *ma_keys;
    PyObject **ma_values;       // 用来保存split table的数据
} PyDictObject;

#### 其它

对于 `PyDictKeysObject` 来说，

## 操作 Hash 表

首先看下内部是怎么使用 Hash 表的，而对外提供的 Python 接口都是对这些函数的封装。

###

dk_get_index() 通过dk_indices获取在dk_entries中的索引
dk_set_index()

lookdict() 查询HASH表，这就是核心的函数

## 参考

[More compact dictionaries with faster iteration](https://mail.python.org/pipermail/python-dev/2012-December/123028.html) 新的数据结构如何对内存进行优化的。


-->

## 参考

详细可以参考 [深入 Python 字典的内部实现](http://python.jobbole.com/85040/) 以及其原文 [Python dictionary implementation](http://www.laurentluce.com/posts/python-dictionary-implementation/) 。

<!--
摘抄出来的实现
http://www.cnblogs.com/xiangnan/p/3859578.html
-->


{% highlight text %}
{% endhighlight %}
