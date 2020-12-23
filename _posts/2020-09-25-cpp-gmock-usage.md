---
title: C++ Mock 工具 gmock 使用
layout: post
comments: true
language: chinese
tag: [Program, C/C++]
keywords: C/C++,gmock
description:
---

所谓的 mock 方法，是单元测试中常见的一种测试方式，用来模拟对象、隔离边界等，例如单元测试时模拟三方接口，这样服务可以独立测试；开发阶段不需要依赖其它类的开发进度等等。

在 C++ 中，比较常用的是 Google 的 GMock 工具，可以用来模拟构造接口，并返回 mock 数据。

这里我们从一个简单的示例开始，一步步详细介绍其使用方法，以及一些常用的技巧。

<!-- more -->

## 简介

Mock 工具用在测试驱动 (Test-Driven Development) 的开发模式中经常使用，Google Mock 是在 2008 年推出的一套针对 C++ 的 Mock 工具。

GMock 开始是作为一个独立的项目开发维护的，后来和 GTest 合并，统一在 [GitHub GTest/GMock](https://github.com/google/googletest) 仓库上维护，所以其安装步骤与 GTest 的安装步骤相同，可以参考 [C++ 单测工具 gtest 使用详解](/post/c++-gtest-unit-test-usage.html) 中的详细介绍。

<!--
轻松地创建mock类
支持丰富的匹配器（Matcher）和行为（Action）
支持有序、无序、部分有序的期望行为的定义
多平台的支持
-->

{% include ads_content01.html %}

## 示例

假设我在和一个名叫张三的同事共同开发一个产品，其中他会负责对不同的图形计算面积，而我需要根据面积来计算该图形的价值 (就简单乘以一个固定系数)，所以，我们先定义了一个接口如下。

``` cpp
class Shape {
public:
        virtual double Area(void) = 0;
};
```

张三会根据不同的类型进行计算，例如对于四边形来说为。

``` cpp
class Rectangle: public Shape {
private:
        double width, height;

public:
        // Simple Constructor
        Rectangle(double w, double h)
        {
                width = w;
                height = h;
        }

        // Destructor
        ~Rectangle() { } // Do nothing

        double Area(void)
        {
                return width * height;
        }
};
```

然后我实现了计算价值的代码如下。

``` cpp
double GetPrice(class Shape &s)
{
        return s.Area() * 2;
}
```

正常的调用流程应该是如下。

``` cpp
int main(void)
{
        Rectangle rect(3, 4);
        std::cout << GetPrice(rect) << std::endl;
		return 0;
}
```

因为张三需要完成正方形、圆形、三角形等相关形状的功能开发，工作量要大很多，为了不阻塞自己的开发，此时就可以通过 gmock 来模拟 Shape 接口返回的数据，如下为完整的代码。

``` cpp
#include <gtest/gtest.h>
#include <gmock/gmock.h>

class Shape {
public:
        virtual double Area(void) = 0;
};

double GetPrice(class Shape &s)
{
        return s.Area() * 2;
}

class ShapeMock: public Shape {
public:
        MOCK_METHOD0(Area, double());
};

TEST(ShapeTest, Rectangle)
{
        ShapeMock rect;

        EXPECT_CALL(rect, Area()).WillRepeatedly(testing::Return(12));
        EXPECT_EQ(24, GetPrice(rect));
}

int main(int argc, char **argv)
{
        testing::InitGoogleTest(&argc, argv);
        return RUN_ALL_TESTS();
}
```

也就是说，在模拟的 `rect` 对象中，在调用 `Area()` 接口时会一直返回 `12` 这个值。

{% include ads_content02.html %}

## 执行流程

在通过 gmock 进行 Mock 时一般步骤如下：

1. 定义一个 Mock 类，在类中需要通过类似 `MOCK_METHOD` 的宏定义接口；
1. 测试时新建一个 Mock 对象，用来模拟接口定义的行为；
1. 通过 gmock 提供的接口设置 Mock 对象需要执行的动作；
1. 调用 Mock 对象返回的数据，检查是否与预期相符。

其中第 3 步定义了 Mock 对象应该返回的结果，通过 gmock 提供的 API 接口来模拟预期的行为，而当 mock 对象被销毁时会自动检查是否所有的内容 (期望行为) 都检查过了。

### 定义成员函数期望行为

这也是在单元测试中使用 Mock 方法时最关键的动作，可以通过如下语法进行定义。

```
EXPECT_CALL(mock_object, method(matcher1, matcher2, ...))
    .With(multi_argument_matcher)
    .Times(cardinality)
    .InSequence(sequences)
    .After(expectations)
    .WillOnce(action)
    .WillRepeatedly(action)
    .RetiresOnSaturation();
```

简单介绍下上面的语法：

* `mock_object` 是创建的 mock 对象，而 `method` 对应了方法名称，例如上面的 `Area` 方法，剩余的 `matcher` 是参数信息；
* `Times(cardinality)` 之前定义的方法运行几次；

第4行的InSequence(sequences)的意思是定义这个方法被执行顺序（优先级），我会再后面举例说明。
第6行WillOnce(action)是定义一次调用时所产生的行为，比如定义该方法返回怎么样的值等等。
第7行WillRepeatedly(action)的意思是缺省/重复行为。

如下是一个简单的示例。

```
EXPECT_CALL(mock, Area())
	.Times(testing::AtLeast(5))
	.WillOnce(testing::Return(100))
	.WillOnce(testing::Return(150))
	.WillRepeatedly(testing::Return(200))
```

可以解释为，在调用 `mock` 的 `Area()` 方法时，至少会被调用 5 次，第一次被调用返回 100 ，第二次被调用返回 150 ，以后每次会返回 200 。

## 参考

在 [GitHub gmock README.md](https://github.com/google/googletest/blob/master/googlemock/README.md) 中包含了很多有用的参考信息。

<!--
接口必须是纯虚类，否则如果 `delete` 基础类的指针时，就有可能不会调用继承类的析构函数，那么就可能导致资源泄漏。

存在一个 `testing::_` 符号，用来表示任意参数。

https://www.cnblogs.com/aoyihuashao/p/9362003.html
https://blog.csdn.net/breaksoftware/article/details/51384083
https://www.cnblogs.com/welkinwalker/archive/2011/11/29/2267225.html

GTest
https://www.jianshu.com/p/7bc4c2458cce
https://cloud.tencent.com/developer/article/1383728
https://blog.csdn.net/breaksoftware/article/details/51059453
https://www.cnblogs.com/coderzh/archive/2009/04/11/1433744.html
https://originlee.com/2014/12/01/analys-implementation-of-gtest/

有稍微的介绍
https://blog.csdn.net/breaksoftware/article/list/3
https://www.cnblogs.com/coderzh/archive/2009/04/11/1433744.html


另外 Boost 貌似也提供了一个单元测试框架，不过没有用过。

g++ -std=c++11 test.cpp -o main -lgtest -lpthread -lgmock

http://jmock.org/
https://easymock.org/
-->


{% highlight text %}
{% endhighlight %}
