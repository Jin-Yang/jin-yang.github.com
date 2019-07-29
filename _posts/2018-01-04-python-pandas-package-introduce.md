---
title: Python Pandas 简介
layout: post
comments: true
language: chinese
category: [program,linux]
keywords: python,pandas
description: Pandas 是一个基于 NumPy 的工具，主要是为了解决数据分析任务，包括了一些标准的数据模型，提供了高效地操作大型数据集所需的工具。
---

Pandas 是一个基于 NumPy 的工具，主要是为了解决数据分析任务，包括了一些标准的数据模型，提供了高效地操作大型数据集所需的工具。

<!-- more -->

![pandas logo]({{ site.url }}/images/python/pandas-logo.png "pandas logo"){: .pull-center width="50%" }

## 简介

### 安装

直接通过 `pip install pandas` 进行安装，一般使用如下方式引用 `import pandas as pd` 。

## Series VS. DataFrame

这是 Pandas 中最基本的两种数据类型。

### Series

类似于一维数组对象，其中的数据类型为 NumPy 的各种类型，而且每个元素都有各自的标签(或者是索引，可以是数字或者字符串)。

{% highlight python %}
import pandas as pd

s1 = pd.Series([1, 2, '3', 4])  # 可以包含了多种类型，默认增加0~N的索引
print(s1)                       # 此时的类型为object(也就是字符串)，如果全为数值则是int64
s1.index = ['a', 'b', 'c', 'd'] # 可以直接修改索引
s1.reindex(['a', 'b', 'c', 'd'], method='pad') # 或者使用reindex()修改索引

s1['a']                         # 查看单个
s1[['a', 'b']]                  # 查看多个，如果全是数值也可以通过s1[s1 > 2]过滤
s1['a':'c']                     # 通过切片查询
s1.drop('a')                    # 删除
s1['a'] = 100                   # 修改

s2 = pd.Series({'a':1,'b':2,'c':3}) # 也可以通过字典创建
s3 = pd.Series(range(4), index=['a', 'b', 'c', 'd']) # 通过range创建
{% endhighlight %}

在通过 `reindex()` 修改索引时，其中的参数可以选择 `ffill` 或者 `pad` 用来前向填充或者 `bfill` 或者 `backfill` 用来后向填充，默认填充 `0` ，也可以通过 `fill_value = 1` 参数进行修改。

### DataFrame

这是一个二维的数据结构，可以简单将其理解成一个 excel 中的表格，每一列都有一个标签 (也可以看做列名) 。

{% highlight python %}
pandas.DataFrame(data=None, index=None, columns=None, dtype=None, copy=False)
{% endhighlight %}

如果 data 为数组，那么其中的每个元素都将作为一行的数据。

{% highlight python %}
import numpy as np
import pandas as pd

# 一个三行一列的数据表，列名为cols，行号为a b c
df1 = pd.DataFrame([1, 2, 3], columns=['cols'], index=['a', 'b', 'c'])
# 两行三列的数据
df2 = pd.DataFrame([[1, 2, 3], [4, 5, 6]], columns=['c1', 'c2', 'c3'], index=['a', 'b'])

df2.index                   # 查看行号(索引)以及列名称
df2.columns

df2[['c1', 'c3']]           # 直接访问返回的是列数据

df2.loc['a']                # 可以通过如下方法查看行数据
df2.loc[['a','b']]
df2.loc[df2.index[1:3]]

df2.sum()                   # 默认对每个列求和，也即是df2.sum(0)
df2.sum(1)                  # 对每个行求和

df2.apply(lambda x:x*2)     # 对每个元素乘2
df2**2                      # 类似于ndarray，也可以支持向量操作

df2.append(pd.DataFrame({'c1':7,'c2':8,'c3':9},index=['c']))  # 增加行

df2['c4'] = [10, 20]        # 新增列，此时会默认填充
df2['c5'] = pd.DataFrame([30, 40], index=['a','b']) # 如果不是默认的(0~N)，则需要指定行名

# 其它方法，其中data可以是列表、数组、字典，列名和索引需要为列表对象
df3 = pd.DataFrame(np.array([[1, 2, 3], [4, 5, 6]]), columns=['c1', 'c2', 'c3'], index=['a', 'b'])
df4 = pd.DataFrame({'c1':[1, 4], 'c2':[2, 5], 'c3':[3, 6]}, index=['a', 'b'])
{% endhighlight %}

注意，在使用 `sum()` 求和时，其中的参数只能是 `0` 或者 `1` 分别是对列或者行进行求和。

## GroupBy

Pandas 提供了一个灵活高效的 groupby 功能，可以根据一个或者多个键拆分分组，然后可以对分布执行一些统计操作。

{% highlight python %}
import numpy as np
import pandas as pd

df = pd.DataFrame({
	'A': ['a', 'b', 'a', 'c', 'a', 'c', 'b', 'c'],
	'B': [  2,   8,   1,   4,   3,   2,   5,   9],
	'C': [102,  98, 107, 104, 115,  87,  92, 123]
})
grouped = df.groupby('A')     # 会生成一个DataFrameGroupBy对象
print(grouped.groups)         # 具体分组信息，key为列A中的成员，value是数据的索引
# {'a': Int64Index([0, 2, 4]), 'b': Int64Index([1, 6]), 'c': Int64Index([3, 5, 7])}

print(grouped.mean())         # 对所有的成员计算平均值
#      B           C
# A                 
# a  2.0  108.000000
# b  6.5   95.000000
# c  5.0  104.666667
grouped[['B', 'C']].mean()    # 可以选择列，这里与上面等价
grouped.agg({'B':'mean', 'C':'sum'})  # 也可以不同列选择不同算法
{% endhighlight %}


在使用上述的聚合方式的时候，需要注意 `size()` 和 `count()` 的区别，前者统计时包含 `NaN` 的值，后者不含 `NaN` 。

<!---
https://www.yiibai.com/pandas/python_pandas_groupby.html
https://blog.csdn.net/Leonis_v/article/details/51832916
https://www.cnblogs.com/lemonbit/p/6810972.html
-->


{% highlight text %}
{% endhighlight %}
