---
Date: October 19, 2013
title: Python 修饰符 (Descriptor)
layout: post
comments: true
category: [linux, bash]
language: chinese
---

是新类的特性


描述符实际上就是可以重用的属性，

通常Python默认对属性的操作是从对象的字典(\_\_dict\_\_)中获取get，设置set或者删除delete。如，对于实例a，a.x的查找顺序为a.\_\_dict\_\_['x']，接着是type(a).\_\_dict\_\_['x']，然后是父类中查找。而如果属性x是一个描述符，那么访问a.x时不再从字典\_\_dict\_\_中读取，而是调用描述符的\_\_get\_\_()方法，对于设置和删除也是同样的原理。因此个人猜测没有验证，实际上是先查找属性是否有\_\_get\_\_()等方法，如果没有则在\_\_dict\_\_中查找。

例如，我们要求薪水的值应该大于0，可以在\_\_init\_\_中进行检测，但是可以在外部调用修改为负值，详见property.py中的PayBug类。为了对该值进行检测，可以通过@property修饰符进行修改，内容详见property.py中的Pay类。

但是这样修改仍然存在这麻烦，如果一个类里含有多个相似的属性（要求不能为负值），那么需要重复添加多个类似的函数。

我们可以通过类的__init__函数对变量进行初始化（初始化的变量名与类的变量名相同），当通过instance.__dict__查看时，不存在对应的变量；只能通过Class.__dict__来查看。而对于正常的类，这样操作实际是保存了一组类变量和一组实例变量。也就是说我们可以在__init__中对属性进行赋值，但是操作还是在描述符中进行。

在参考文献1中，在描述符类中采用了WeakKeyDictionary来解决不同实例之间的冲突，但是测试发现，现在的Python中不存在类似的问题。


http://nbviewer.ipython.org/urls/gist.github.com/ChrisBeaumont/5758381/raw/descriptor_writeup.ipynb
http://www.geekfan.net/7862/

http://blog.csdn.net/bluehawksky/article/details/50372244

