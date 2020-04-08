---
title: 损失函数
layout: post
comments: true
usemath: true
language: chinese
category: [misc]
keywords:
description:
---


<!-- more -->




## 回归问题评估

### MAE

Mean Absolute Error, MAE 平均绝对误差。

$$ \frac{1}{n} \sum \limits_{i=1}^{n} \lvert y_i – \hat{y}_i \rvert $$

### MSE

Mean Square Error, MSE 均方误差。真实值与预测值差值的平方然后求和平均，常用作线性回归的损失函数。

$$\frac{1}{n} \sum_{i=1}^{n}(y_i - \hat y_i)^2$$

### RMSE

Root Mean Square Error, RMSE 均方根误差。衡量观测值与真实值之间的偏差，常用来作为机器学习模型预测结果衡量的标准。

$$ \sqrt{ \frac{1}{n} \sum_{i=1}^{n}(y_i - \hat y_i)^2 }$$

### RMSLE

Root Mean Squared Log Error, RMSLE 均方根对数误差。

$$\sqrt{ \frac{1}{n} \sum_{i=1}^{n} (log(\hat{y}_i + 1) – log(y_i + 1))^2 }$$

### 总结

实际上，很多的比赛采用的是 RMSLE ，例如 Kaggle ，对于异常值来说，RMSE 会快速的增加，而 RMSLE 因为取了对数就不会。

<!--
https://medium.com/analytics-vidhya/root-mean-square-log-error-rmse-vs-rmlse-935c6cc1802a

常用的评估方法
https://www.analyticsvidhya.com/blog/2019/08/11-important-model-evaluation-error-metrics/
-->


{% highlight text %}
{% endhighlight %}
