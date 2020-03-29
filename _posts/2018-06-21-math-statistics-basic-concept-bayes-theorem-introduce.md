---
title: 贝叶斯简介
layout: post
comments: true
language: chinese
usemath: true
category: [linux,misc]
keywords: bayes' theorem
description: 贝叶斯定理 (Bayes' Theorem) 是英国数学家 托马斯·贝叶斯 (Thomas Bayes) 在 1763 年发表的一篇论文中首次提出；而贝叶斯推断 (Bayesian Inference) 是贝叶斯定理的一种应用，是一种统计学方法，用来估计统计量的某些性质。 这里简单介绍其基本概念。
---

贝叶斯定理 (Bayes' Theorem) 是英国数学家 托马斯·贝叶斯 (Thomas Bayes) 在 1763 年发表的一篇论文中首次提出；而贝叶斯推断 (Bayesian Inference) 是贝叶斯定理的一种应用，是一种统计学方法，用来估计统计量的某些性质。

这里简单介绍其基本概念。

<!-- more -->

## 简介

在统计学界一直有两种观点：A) 频率学派；B) 贝叶斯学派。

贝叶斯推断与统计学的推断方法不同，是建立在主观判断的基础上，也就是说，可以不需要客观证据，先估计一个值，然后根据实际结果不断修正。正是因为它的主观性太强，曾经遭到许多统计学家的诟病，很长时间内无法广泛应用。

在计算机获得广泛发展后，人们也逐渐意识到，许多统计量是无法事先进行客观判断的，而利用互联网中的大型数据集，可以不断更新完善模型，从而为应用贝叶斯推断创造了条件。

贝叶斯学派的思想是用数据来更新特定假设的概率。


<!--
在贝叶斯学派看来，模型中的参数(包括样本、预测参数等) 都可以看作是一个随机变量，它们都是服从某个分布 (一般称为先验分布，例如正态分布、Gamma 分布等)。

但在频率学派看来就不太一样，他们认为模型里的参数就是一个数，是一个确定的事件，只要能找到足够的样本就可以得到这个数准确的值，它并不是什么随机变量。

两者的分歧在于是否从概率分布的角度看待这个问题，贝叶斯学派认为，当样本足够多的时候，可以说你有 99.9999% 的信心认为等于这个数，但也有可能等于其它数，只是概率比较小而已罢了，也就是说是从一个概率分布的角度来看到这个问题。
-->

## 贝叶斯定理

贝叶斯定理实际上是条件概率的一种推断。

### 条件概率 Conditional Probability

是指在 **已知** 事件 A 发生的情况下，此时事件 B 发生的概率，这里用 `P(B|A)` 来表示，此时的表达式可以表示为 `P(B|A)=P(AB)/P(A)` 。

例如，将一枚硬币抛掷两次，其中事件 A 为 "至少一次为正面(H)" ，事件 B 为 "两次掷出同面" ，那么已知事件 A 发生的条件下事件 B 发生的概率。

这里的样本空间为 `S={HH, HT, TH, TT} A={HH, HT, TH} B={HH, TT}` 那么如果事件 A 已经发生，事件 B 发生的概率为 `P(B|A)=[1/4]/[3/4]=1/3` 。

需要注意的是，在这些定义中 A 与 B 之间不一定有因果或者时间序列关系，A 可能会先于 B 发生，也可能相反，也可能二者同时发生；A 可能会导致 B 的发生，也可能相反，也可能二者之间根本就没有因果关系。

<!--
如果要考虑一些可能是新的信息的概率条件性可以通过贝叶斯定理实现，也就是说，如果已经确定 B 发生了，那么 A 发生的概率应该是 `P(AB)/P(A)` 。
-->

### 贝叶斯公式

通过条件概率，就可以得到乘法公式，也就是 `P(AB)=P(A|B)P(B)=P(B|A)P(A)` 。

进而可以得出，贝叶斯最常用的公式，如下：

$$P(A|B) = \frac{P(B|A)P(A)}{P(B)}$$

这个看起来很简单，无非就是条件概率和联合概率公式的组合，其中：

* `P(A)` 是先验概率或者边缘概率，之所以称为 "先验" 是因为不考虑任何与 B 相关的因素，完全独立计算，可以通过大数据统计得到。
* `P(A|B)` 已知 B 发生后 A 发生的概率，称为 A 的后验概率。
* `P(B)` 是先验概率或者边缘概率，也被称为标准化常量 (Normalized Constant)。

![bayes]({{ site.url }}/images/ai/bayes-equation.jpg "bayes"){: .pull-center }

## 贝叶斯推断

为了方便计算后验概率，这里采用共轭先验的方法来简化后验的计算。

### 先验概率

这里仍然以投掷硬币为例，开始认为正面朝上的概率服从 Beta 分布，Beta 分布能产生一个 $(0, 1)$ 之间的随机数，也就是说先验概率为 Beta 分布。

$$f(x;\alpha,\beta)=\frac{1}{B(\alpha, \beta)} x^{\alpha - 1} (1 - x)^{\beta - 1}$$

开始假设 $\alpha=\beta=1$，那么此时 Beta 分布退化为一个均匀分布。接着不断投硬币，记录好每次投掷结果，然后根据结果再来计算此时正面朝上的概率。

### 似然函数

也就是在 $n$ 次试验中，有 $k$ 次朝上的概率，显然满足二项分布，可以表示为。

$$P(x|\theta)=C_n^k \theta^k (1 - \theta)^{n-k}$$

### 后验概率

这里直接通过计算求解。

$$
\begin{align}
P(\theta|x) &= \frac{P(x|\theta)P(\theta)}{P(x)} \propto P(x|\theta)P(\theta) \\
&= \left ( C_n^k \theta^k (1 - \theta)^{n-k} \right ) \left(\frac{1}{B(\alpha, \beta)} \theta^{\alpha - 1} (1 - \theta)^{\beta - 1}\right) \\
&=\frac{C_n^k}{B(\alpha,\beta)} \theta^{(k+\alpha)-1}\left(1-\theta\right)^{(n-k+\beta)-1} \\
&\propto \frac{1}{B\left(k+\alpha, n-k+\beta\right)} \theta^{(k+\alpha)-1} \left(1-\theta\right)^{(n-k+\beta)-1}
\end{align}
$$

可以看到，后验概率也是 Beta 分布，可以很方便的通过先验 Beta 分布来计算出后验概率。

### 试验

对应的试验如下，也就是，随着试验次数的增加，正面朝上的概率越来越接近 $0.5$ 。

{% highlight python %}
import numpy as np
import scipy.stats as stats
import matplotlib.pyplot as plt

# Arguments for prior Beta distribution
(alpha, beta) = (1, 1)

# Create a list of the number of coin tosses ("Bernoulli trials")
number_of_trials = [0, 2, 10, 20, 50, 500]

# Conduct 500 coin tosses and output into a list of 0s and 1s
# where 0 represents a tail and 1 represents a head
data = stats.bernoulli.rvs(0.5, size=number_of_trials[-1])

# Discretise the x-axis into 100 separate plotting points
x = np.linspace(0, 1, 100)

# Loops over the number_of_trials list to continually add
# more coin toss data. For each new set of data, we update
# our (current) prior belief to be a new posterior. This is
# carried out using what is known as the Beta-Binomial model.
# For the time being, we won't worry about this too much. It
# will be the subject of a later article!
for i, N in enumerate(number_of_trials):
	# Accumulate the total number of heads for this
	# particular Bayesian update
	heads = data[:N].sum()

	# Create an axes subplot for each update
	ax = plt.subplot(len(number_of_trials) / 2, 2, i + 1)
	ax.set_title("%s trials, %s heads" % (N, heads))

	# Add labels to both axes and hide labels on y-axis
	plt.xlabel("$P(H)$, Probability of Heads")
	plt.ylabel("Density")
	if i == 0:
		plt.ylim([0.0, 2.0])
	plt.setp(ax.get_yticklabels(), visible=False)

	# Create and plot a Beta distribution to represent the
	# posterior belief in fairness of the coin.
	y = stats.beta.pdf(x, alpha + heads, beta + N - heads)
	plt.plot(x, y, label="observe %d tosses,\n %d heads" % (N, heads))
	plt.fill_between(x, 0, y, color="#aaaadd", alpha=0.5)

# Expand plot to cover full width/height and show it
plt.tight_layout()
plt.show()
{% endhighlight %}

![bayesian example]({{ site.url }}/images/ai/bayesian-statistics-beta-bernoulli-example.png "bayesian example"){: .pull-center width="80%" }

<!--
上述代码可以参考
https://www.quantstart.com/articles/Bayesian-Statistics-A-Beginners-Guide
-->


<!--
按这些术语，Bayes法则可表述为：
后验概率 = (似然度 * 先验概率)/标准化常量 也就是说，后验概率与先验概率和似然度的乘积成正比。
另外，比例Pr(B|A)/Pr(B)也有时被称作标准似然度（standardised likelihood），Bayes法则可表述为：
后验概率 = 标准似然度 * 先验概率
要理解贝叶斯推断，必须先理解贝叶斯定理。后者实际上就是计算"条件概率"的公式。


如果把 P(B) 展开，可以得到如下的公式。

$$P(A|B) = {P(B|A)P(A)}/{P(B|A)P(A) + P(B|A')P(A')}$$

其中 `P(A')` 表示非 `A` 的概率，其实后面的公式在实际使用时可能会更多。


全概率公式、贝叶斯公式推导
https://www.cnblogs.com/ohshit/p/5629581.html
http://www.ruanyifeng.com/blog/2011/08/bayesian_inference_part_one.html

通过贝叶斯公式，可以明确告知我们如何利用新证据修改已有的看法。

最通俗的讲解
https://www.jianshu.com/p/d9757860c4f8
最经典的故障预测的案例
https://www.matongxue.com/madocs/279.html

排列和组合详解
https://zhuanlan.zhihu.com/p/41855459
https://seeing-theory.brown.edu/compound-probability/index.html
-->




## 案例

很多时候示例是最显而易见的讲解方式。

### 案例 #1

一个学校里有 `60%` 男生和 `40%` 女生，女生穿裤子的人数和穿裙子的人数相等，所有男生穿裤子。一个人在远处随机看到了一个穿裤子的学生，那么这个学生是女生的概率是多少？

利用贝叶斯定理，假设事件 A 是看到女生，事件 B 是看到一个穿裤子的学生，那么我们所要计算的是 `P(A|B)`。

* `P(A)` 是忽略其它因素，看到女生的概率，在这里是 `40%`；
* `P(A')` 是忽略其它因素，看到不是女生(也就是看到男生)的概率，在这里是 `60%`；
* `P(B|A)` 是女生穿裤子的概率，在这里是 `50%`；
* `P(B|A')` 是男生穿裤子的概率，在这里是 `100%`；
* `P(B)` 是忽略其它因素，直接考虑学生穿裤子的概率，`P(B) = P(B|A)P(A) + P(B|A')P(A')`，在这里是 `0.5*0.4 + 1*0.6 = 0.8` ；

那么根据贝叶斯公式，可以计算得到，也就是 `P(A|B) = (0.5 * 0.4)/(0.8) = 0.25` 。

### 贝叶斯过滤器

对于垃圾邮件来说，传统使用较多的是关键词，但是很容易误判，2002 年 Paul Graham 提出使用 "贝叶斯推断" 过滤垃圾邮件，取得了很好的效果，据说 1000 封垃圾邮件可以过滤掉 995 封，且没有一个误判。

另外，这种过滤器还具有学习能力，会根据新收到的邮件，不断调整，收到的垃圾邮件越多，它的准确率就越高。

<!--


## 其它

### 常见概念

一些常见而且容易混淆的概念，例如先验概率、后验概率、似然概率、条件概率、贝叶斯、最大似然估计、最大后验概率估计。

#### 先验概率

事件发生前的预判概率，一般可以是基于历史数据的统计，可以由背景常识得出，可以通过推理计算，也可以是人的主观观点给出，一般都是单独事件概率。

#### 后验概率

事件发生后求的反向条件概率，或者说，是基于先验概率求得的反向条件概率。

#### 条件概率

一个事件发生后另一个事件发生的概率。

#### 贝叶斯公式

P(A|B) = {P(B|A)P(A)}/P(B)

其中定义如下：

* `P(A|B)` 是后验概率，也就是最终求解的目标；
* `P(B|A)` 是条件概率，也叫作似然概率，通常是通过历史数据统计得到，一般不把它叫做先验概率，但从定义上也符合先验定义。
* `P(A)` 先验概率，一般都是人主观给出的，贝叶斯中的先验概率一般特指它。
* `P(B)` 其实也是先验概率，只是在贝叶斯的很多应用中不重要(因为只要最大后验不求绝对值)，需要时往往用全概率公式计算得到。





http://www.ruanyifeng.com/blog/2011/08/bayesian_inference_part_two.html

https://jrnold.github.io/bayesian_notes/index.html
https://jrnold.github.io/bayesian_notes/bayesian-inference.html

https://www.zhihu.com/question/54082000
https://blog.csdn.net/baimafujinji/article/details/51374202
https://blog.csdn.net/u011508640/article/details/72815981
-->

{% highlight text %}
{% endhighlight %}
