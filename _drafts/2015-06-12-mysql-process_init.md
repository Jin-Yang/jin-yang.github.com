---
title: MySQL 执行过程
layout: post
comments: true
language: chinese
category: [mysql,database]
keywords: mysql,执行
description:
---




<!-- more -->

<!--然后我们直接从mysql_select开始，它主要包括三个操作join:: prepare，join:: optimize，join::exec，其中prepare完成一些判断准备工作如对通配符的处理、having条件、order等的判断初始化等；optimize就是负责优化检索，简单的说就是为了确定一个mysql认为最优的执行计划；而exec则负责执行optimize确定的执行计划。-->

关于 SQL 的查询，源码可以直接从 mysql_select() 开始，其中包括其优化结果等信息都是保存在 JOIN 结构体中的，它主要包括如下的三个操作：

* JOIN::prepare()

    用于为整个查询做准备工作，包括分配内存、处理别名、通配符的处理、检查表是否可以访问、检查字段、检查非 group 函数、准备 procedure 等工作。

* JOIN::optimize()

    整个查询优化器的核心内容，主要对 join 进行简化、优化 where 条件、优化 having 条件、裁剪分区 partition (如果查询的表是分区表)、优化 count()/min()/max() 聚集函数、统计 join 的代价、搜索最优的 join 顺序、生成执行计划、执行基本的查询、优化多个等式谓词、执行 join 查询、优化 distinct、创建临时表存储临时结果等操作。

* JOIN::exec()

    负责执行优化后的执行计划。


## 源码相关

其中与 JOIN 相关的头文件代码在 sql/sql_optimizer.h 中，包括了相关类以及接口的定义。

{% highlight cpp %}
class JOIN :public Sql_alloc
{
  bool need_tmp;                  // 是否需要tmp文件
  bool plan_is_const() const;     // 是否为常量，例如返回空或者一行记录
  Next_select_func first_select;  // 记录读取首条记录的函数，默认为sub_select()
  Next_select_func get_end_select_func();


  bool is_optimized() const { return optimized; }
  void set_optimized() { optimized= true; }
  bool is_executed() const { return executed; }
  void set_executed() { executed= true; }
  void reset_executed() { executed= false; }
private:
  bool optimized; ///< flag to avoid double optimization in EXPLAIN
  bool executed;  ///< Set by exec(), reset by reset()


}
{% endhighlight %}


{% highlight text %}
{% endhighlight %}
