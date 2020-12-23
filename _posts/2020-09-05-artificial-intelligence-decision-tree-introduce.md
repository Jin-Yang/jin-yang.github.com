---
title: 决策树
layout: post
comments: true
language: chinese
tag: [Linux, AI, Python]
keywords: stan
description:
---

决策树算法是从数据的特征出发，并以此作为基础划分不同的类。

<!-- more -->

## 简介

也就是如何通过已经存在的数据构建一个决策树，一般有如下的三种方式。

* ID3 通过信息熵进行构建。
* C4.5 信息增益比
* Classification And Regression Trees, CART 通过基尼指数构建。

## Scikit-Learn

可以使用该库自带的鸢尾花数据集，做决策树训练、测试、可视化，如下是决策树的训练以及可视化操作。

{% highlight python %}
import pydotplus
import sklearn.tree as tree
import sklearn.datasets as ds

iris = ds.load_iris()
dtree = tree.DecisionTreeClassifier()
dtree = dtree.fit(iris.data, iris.target)

# 直接导出graphviz格式的文件，如果需要其它格式，应该再次转换
# --> PDF格式 dot -Tpdf iris.dot -o iris.pdf
# --> PNG格式 dot -Tpng iris.dot -o iris.png
#with open("iris.dot", 'w') as f:
#    tree.export_graphviz(dtree, out_file=f, feature_names=iris.feature_names)

dot = tree.export_graphviz(dtree, out_file=None, feature_names=iris.feature_names)
graph = pydotplus.graph_from_dot_data(dot)
graph.write_pdf('iris.pdf')
{% endhighlight %}

如下是通过训练然后测试。

{% highlight python %}
import sklearn.tree as tree
import sklearn.datasets as ds
import sklearn.model_selection as model

iris = ds.load_iris()
d_train, d_test, t_train, t_test = model.train_test_split(iris.data, iris.target, test_size=0.4)

dtree = tree.DecisionTreeClassifier()
dtree = dtree.fit(d_train, t_train)

match = 0
predict = dtree.predict(d_test)
for i in range(len(predict)):
    if predict[i] == t_test[i]:
        match += 1
print("Accuracy: %.3f%%" % (float(match/len(predict)) * 100))
{% endhighlight %}

在 Windows 中，可以通过 [graphviz.gitlab.io](https://graphviz.gitlab.io/) 下载相关的 GraphViz 工具，然后将对应的 bin 路径添加到 PATH 环境变量中。

也可以使用 pydotplus 包，不过依然依赖上述的 GraphViz 工具。

<!--
https://scikit-learn.org/stable/modules/tree.html
https://github.com/Erikfather/Decision_tree-python
https://machinelearningmastery.com/implement-decision-tree-algorithm-scratch-python/
https://medium.com/machine-learning-guy/an-introduction-to-decision-tree-learning-id3-algorithm-54c74eb2ad55
https://static.googleusercontent.com/media/research.google.com/en//pubs/archive/45166.pdf
https://towardsdatascience.com/light-on-math-machine-learning-intuitive-guide-to-understanding-decision-trees-adb2165ccab7
-->

## ID3

ID3 算法是由 Ross Quinlan 在 1986 年提出，该算法是以信息论为基础，以信息熵和信息增益为衡量标准，从而实现对数据的归纳分类。

{% highlight text %}
{% endhighlight %}
