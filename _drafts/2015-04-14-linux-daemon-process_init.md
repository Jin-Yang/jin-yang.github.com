---
title: MySQL 基本介绍
layout: post
comments: true
language: chinese
category: [mysql,database]
---


<!-- more -->


http://www.cnblogs.com/JohnABC/p/4079669.html
http://yangxikun.github.io/linux/2013/11/11/linux-process.html
http://blog.csdn.net/a600423444/article/details/6958025
http://alfred-sun.github.io/blog/2015/06/18/daemon-implementation/
http://qujunorz.blog.51cto.com/6378776/1563927
https://zhuanlan.zhihu.com/p/25118420
https://www.ibm.com/developerworks/cn/linux/l-cn-nohup/
http://blog.csdn.net/taiyang1987912/article/details/44850999
http://www.cnblogs.com/zhiguo/p/3370599.html
http://blog.chinaunix.net/uid-27105712-id-3356916.html
http://www.embedu.org/Column/7509.html

http://cwe.mitre.org/top25/


https://gist.githubusercontent.com/jamiesun/3097215/raw/2873acdf1c784896442099cee3ef3093077a0877/daemon.py
在Start the daemon前增加，文件权限判断。
try:
    file(self.pidfile,'w').write("writable test\n")
except IOError, err:
    print str(err)
sys.exit(1)AVL


































c_avl_tree_t *c_avl_create(int (*compare)(const void *, const void *));
入参：
  比较函数，类似strcmp()；
实现：
  1. 保证 compare 有效，非 NULL；
  2. 申请内存，部分结构体初始化。
返回：
  成功返回结构体指针；参数异常或者没有内存，返回 NULL；

int c_avl_insert(c_avl_tree_t *t, void *key, void *value);
返回：
  -1：内存不足；
  0： 节点写入正常；
  1:  节点已经存在；

int c_avl_get(c_avl_tree_t *t, const void *key, void **value);
调用者保证 t 存在 (非NULL)。
返回：
  -1：对象不存在；
  0： 查找成功，对应的值保存在value中；

int c_avl_remove(c_avl_tree_t *t, const void *key, void **rkey, void **rvalue);
返回：
  -1：对象不存在；


_remove()
search()
rebalance()
verify_tree()


<!-- TOCTOU 漏洞，不要使用文件名，而是文件句柄。-->



{% highlight text %}
{% endhighlight %}
