---
title: C++ 单测工具 gtest 使用详解
layout: post
comments: true
language: chinese
tag: [Program, C/C++]
keywords: C/C++,gtest
description: Google 提供了一个很好的单元测试工具 gtest ，提供了跨平台的能力，而且代码实现非常简单，几乎没有依赖，但是提供的能力很丰富。这里详细介绍其使用方法、常见问题等。
---

这是 Google 提供的一个跨平台的测试、Mock 框架，很简单几乎没有依赖，但是提供了丰富的断言机制，尤其是提供了大部分其它语言测试框架没有的死亡测试。

这里就详细介绍如何使用这一单元测试工具。

<!-- more -->

## 简介

使用时通过 `TEST(test_suite_name, test_name)` 宏定义一个测试用例 (函数)，在定义的用例中可以通过断言测试具体的功能点，每个测试用例分成了 `Suite` 和 `Case` 两级，输出结果时会通过 `.` 将两者连接起来做为一个测试用例的名字。

之所以这么划分，主要是为了方便管理，例如后面的添加预处理和清理函数时。

### 安装

部分平台可能会包含 GTest 的软件包，那么可以直接通过相关工具安装，例如 YUM ，不过有些是不支持的，可以通过源码进行安装，也很简单，如下介绍。

<!-- 可以直接通过 `yum --enablerepo=epel install gtest-devel gmock-devel` -- 安装。也可以直接参考其中的 `travis.sh` 文件。 -->

目前 GTest 和 GMock 已经合并到了一起，可以直接从 [Github](https://github.com/google/googletest) 下载对应的代码，然后通过如下命令编译安装。

```
$ mkdir build && cd build
$ cmake ..
$ make
# make install
```

<!--
$ cd googletest-release-X.X.X && mkdir build
$ cmake -Dgtest_build_samples=ON \
      -Dgmock_build_samples=ON \
      -Dgtest_build_tests=ON \
      -Dgmock_build_tests=ON \
      ..
$ make
$ CTEST_OUTPUT_ON_FAILURE=1 make test
-->

然后会在 `lib` 目录下生成四个静态库，分别为 `libgmock.a` `libgmock_main.a` `libgtest.a` `libgtest_main.a` ，默认安装到 `/usr/local` 目录下，也可以在编译阶段通过指定，也就是。

```
$ cmake -D CMAKE_INSTALL_PREFIX=/usr ..
```

因为 CMake 没有提供卸载命令，所以就只能通过 `cat install_manifest.txt | xargs rm` 命令执行卸载。

另外，因为依赖少，可以手动安装，有两种方式：A) 将头文件、静态库添加到系统目录下；B) 编译应用时指定 GTest 和 GMock 的安装目录。如下简单介绍这两种使用方式。

#### 添加到系统目录

不同的操作系统或者发行版本的系统目录有所不同，以 CentOS 为例，对应的库以及头文件目录分别为 `/usr/lib` 和 `/usr/include` 。

```
# cp build/lib/* /usr/lib
# cp googlemock/include/gmock /usr/include/
# cp googletest/include/gtest /usr/include/
```

其它发行版本目录略有区别，自行调整。另外，需要注意访问权限，否则可能会出现即使目录正确，编译时仍然报错。

然后通过如下命令编译。

```
g++ -std=c++11 main.cpp -o main -lgtest -lpthread
```

#### 编译时指定路径

建议同样复制到一个固定的目录下，例如可以将所有的三方库保存在 `/opt` 目录下，例如新增 `/opt/lib` 以及 `/opt/include` 目录，类似上面操作，将所需文件复制到对应的目录下。

编译时通过 `-I` 以及 `-L` 参数指定头文件和库文件的路径。

```
g++ -I/opt/include -L/opt/lib -std=c++11 main.cpp -o main -lgtest -lpthread
```

### 示例

在源码中有个 `samples` 目录，包含了很多参考示例，如下是一个很简单的 GTest 示例。

<!--
源码目录下包含了多个工程目录，主要是为了提供多平台的支持，如 `msvc` 文件夹是用在微软 Visual Studio 中，`xcode` 文件夹是用于 Mac Xcode，`codegrear` 文件夹是用于 Borland C++ Builder，在 Linux 环境中，需要使用 `make` 文件夹了。

在 `make` 目录下，直接执行 `make` 命令编译即可，同时会生成一个测试程序。为编译生成其他用例的可执行文件，可以参照 `make` 目录下的 `Makefile` 文件，或者执行以下步骤：

{% highlight text %}
----- 将gtest-main.a文件拷贝到samples目录下，执行以下命令
$ g++ -I ../include/ -c sample2_unittest.cc
$ g++ -I ../include/ sample2.o sample2_unittest.o gtest_main.a -lpthread -o test2
{% endhighlight %}
-->

``` cpp
#include <gtest/gtest.h>

int add(int a, int b)
{
        return a + b;
}
TEST(sum, simple)
{
        EXPECT_EQ(add(2, 3), 5);
}
int main(int argc, char **argv)
{
        testing::InitGoogleTest(&argc,argv);
        return RUN_ALL_TESTS();
}
```

然后编译运行，输出内容如下。

```
[==========] Running 1 test from 1 test suite.
[----------] Global test environment set-up.
[----------] 1 test from sum
[ RUN      ] sum.simple
[       OK ] sum.simple (0 ms)
[----------] 1 test from sum (0 ms total)

[----------] Global test environment tear-down
[==========] 1 test from 1 test suite ran. (0 ms total)
[  PASSED  ] 1 test.
```

如上所示，创建一个测试用例的步骤为：

1. 引入关键的头文件 `<gtest.gtest.h>` ；
2. 通过 `TEST` `TEST_F` 宏定义测试用例；
3. 测试使用断言进行判断。

所以，使用方法基本类似，只是项目可能会比较复杂。

{% include ads_content01.html %}

## 断言

断言分成两类：A) `ASSERT_XXX` 当前点检测失败则退出当前函数；B) `EXPECT_XXX` 当前点检测失败会继续往下执行。两者对应的参数相似，只是断言的前缀不同。

```
----- 布尔类型
ASSERT_TRUE(arg);     // 预期结果为true
ASSERT_FALSE(arg);    // 预期结果为false

----- 数值型数据检查
ASSERT_EQ(arg1, arg2); // equal         预期两个值相等
ASSERT_NE(arg1, arg2); // not equal     预期两个值不等
ASSERT_LT(arg1, arg2); // less than     预期arg1小于arg2
ASSERT_GT(arg1, arg2); // greater than  预期arg1大于arg2
ASSERT_LE(arg1, arg2); // less equal    预期arg1小于等于arg2
ASSERT_GE(arg1, arg2); // greater equal 预期arg1大于等于arg2

----- 字符串检查
ASSERT_STREQ(arg1, arg2);     // 预期字符串相等，区分大小写
ASSERT_STRNE(arg1, arg2);     // 预期字符串不等，区分大小写
ASSERT_STRCASEEQ(arg1, arg2); // 预期字符串相等，忽略大小写
ASSERT_STRCASENE(arg1, arg2); // 预期字符串不等，忽略大小写
```

当断言失败时会将源码所在的源文件以及行号输出，包括了错误信息，当然，也可以自定义一个错误信息。

## Test Fixtures

有时候在真正执行测试之前需要构造一部分测试用的数据或者多个测试用例需要使用相同的数据，一般是在测试用例执行前创建，执行结束后清理，GTest 提供了相关的接口，不过需要依赖类的实现。

总共分成三种级别。

#### Global

需要创建一个继承自 `testing::Environment` 的类，并实现其中的 `SetUp()` 和 `TearDown()` 两个方法，最后需要在 `main()` 函数中通过 `testing::AddGlobalTestEnvironment()` 进行注册。

#### TestSuite

需要创建一个与 `test_suite_name` 名称相同的类，同时需要继承 `testing::Test` 类，并实现其中的两个静态方法 `SetUpTestCase()` 以及 `TearDownTestCase()` ，分别会在第一个用例前以及最后一个用例后执行。

#### TestCase

与 TestSuite 级别使用相同的类，但是需要实现的方法不同，需要实现 `SetUp()` 以及 `TearDown()` 两个函数，分别在每个测试用例的前后执行。注意，在两个测试用例中执行的内容是不会相互影响的。

所以，感觉像是每次执行 TestSuite 的测试用例时都会新建一个对象，这样看起来每次都是最新的数据，不会保留上个测试用例的遗留数据。

### 示例

如下包含了上述三种场景。

``` cpp
#include <vector>
#include <iostream>
#include <gtest/gtest.h>

class QuickEnvironment: public testing::Environment {
public:
        virtual void SetUp() override {
                std::cout << "Global SetUp" << std::endl;
        }
        virtual void TearDown() override {
                std::cout << "Global TearDown" << std::endl;
        }
};
class QuickTest: public testing::Test {
public:
        static void SetUpTestCase() {
                std::cout << "Suite SetUp" << std::endl;
        }
        static void TearDownTestCase() {
                std::cout << "Suite TearDown" << std::endl;
        }
protected:
        virtual void SetUp() override {
                std::cout << "Case SetUp" << std::endl;
                data.push_back(1);
        }
        virtual void TearDown() override {
                std::cout << "Case TearDown" << std::endl;
        }
        std::vector<int> data;
};
TEST_F(QuickTest, PushBack)
{
        data.push_back(2); // 与后面的Size用例不会相互影响
        std::cout << "PushBack" << std::endl;
        EXPECT_EQ(data.size(), 2);
        EXPECT_EQ(data.back(), 2);
}
TEST_F(QuickTest, Size)
{
        std::cout << "Size" << std::endl;
        EXPECT_EQ(data.size(), 1);
}
int main(int argc, char **argv)
{
        testing::AddGlobalTestEnvironment(new QuickEnvironment);
        testing::InitGoogleTest(&argc, argv);
        return RUN_ALL_TESTS();
}
```

## 死亡测试

几乎是独有的特性，而所谓的死亡测试是指可能会导致程序崩溃。

<!--
ASSERT_DEATH(arg1, arg2);  // 程序崩溃而且错误信息和arg2匹配，当arg2为空则忽略信息匹配
ASSERT_EXIT(参数1，参数2，参数3)，语句停止并且错误信息和被提前给的信息匹配。
-->

``` cpp
#include <gtest/gtest.h>

int func(void)
{
        int *ptr = NULL;
        *ptr = 100;
        return 0;
}
TEST(DeathTest, NULLPtr)
{
        ASSERT_DEATH(func(), "");
}
int main(int argc, char **argv)
{
        testing::InitGoogleTest(&argc,argv);
        return RUN_ALL_TESTS();
}
```

{% include ads_content02.html %}

## 参数化

在设计测试用例时，经常会遇到写一个测试函数，测试用例中传入不同的参数。

例如有个奇数的判断，测试的时候需要构建多个入参，当有少量的时候手动复制即可，即使后面修改，成本也不是很高，但是当有几十个的时候，处理起来就比较复杂了。

GTest 实际上针对这一场景有一个响应的解决方案。

### 参数化方案

简单来说分成三步：1) 定义一个类；2) 如何进行测试；3) 传入需要测试的参数。

首先，同样需要定义一个类，此时需要继承 `testing::TestWithParam<T>` 模板类，其中的 `T` 就是需要参数化的参数类型，如上对应了 `int` 类型。

然后，通过 `TEST_P` 宏定义测试用例，其中的 P 是 Parameter 的缩写，在函数中可以通过 `GetParam()` 获取当前实际传入的参数。

最后，通过 `INSTANTIATE_TEST_CASE_P` 宏定义实际传入的测试用例，包含了三个入参：

1. 测试用例名称，可以随意填写一个有意义的名字。
2. 需要与上面定义的类保持一致，例如IsEvenTest。
3. 构建参数的参数生成器。

对于最后一个参数，常用的有：`Range()` `Values()` `ValuesIn()` `Bool()` `Combine()` 等等。

### 示例

``` cpp
#include <gtest/gtest.h>

int IsEven(int val)
{
        return val % 2;
}

TEST(IsEven, HandleTrue)
{
        EXPECT_TRUE(IsEven(1));
        EXPECT_TRUE(IsEven(3));
}

class IsEvenTest: public testing::TestWithParam<int> {
};

TEST_P(IsEvenTest, HandleTrue)
{
        EXPECT_TRUE(IsEven(GetParam()));
}

INSTANTIATE_TEST_CASE_P(HandleTrue, IsEvenTest, testing::Values(1, 3));

int main(int argc, char **argv)
{
        testing::InitGoogleTest(&argc,argv);
        return RUN_ALL_TESTS();
}
```

输出如下。

```
[==========] Running 3 tests from 2 test suites.
[----------] Global test environment set-up.
[----------] 1 test from IsEven
[ RUN      ] IsEven.HandleTrue
[       OK ] IsEven.HandleTrue (0 ms)
[----------] 1 test from IsEven (0 ms total)

[----------] 2 tests from HandleTrue/IsEvenTest
[ RUN      ] HandleTrue/IsEvenTest.HandleTrue/0
[       OK ] HandleTrue/IsEvenTest.HandleTrue/0 (0 ms)
[ RUN      ] HandleTrue/IsEvenTest.HandleTrue/1
[       OK ] HandleTrue/IsEvenTest.HandleTrue/1 (0 ms)
[----------] 2 tests from HandleTrue/IsEvenTest (0 ms total)

[----------] Global test environment tear-down
[==========] 3 tests from 2 test suites ran. (0 ms total)
[  PASSED  ] 3 tests.
```

另外还有针对参数的模板。

<!--
https://www.cnblogs.com/coderzh/archive/2009/04/08/1431297.html
-->

## 参数设置

对于 GTest 生成的二进制文件，可以通过一些简单的参数定制化，传参方式有几种：A) 环境变量；B) 命令行参数；C) 代码中指定。

之所以可以通过命令行传参，主要是因为代码中使用了 `testing::InitGoogleTest(&argc, argv);` ，所以，可以直接通过生成的测试二进制文件加 `--help` 参数查看具体支持的参数。

在代码中可以使用 `testing::GTEST_FLAG` 宏设置，例如对于命令行参数 `--gtest_color` 可以使用 `testing::GTEST_FLAG(output) = "no";` 来设置，注意，这里是不需要 `gtest` 前缀的。

另外，设置环境变量时，需要将参数转换为大写，例如上述的 `--gtest_color` 对应的环境变量为 `GTEST_COLOR` 。

一般来说，环境变量的优先级是最低的，而至于后两者的优先级，关键要看具体在代码里是如何设置的，建议将 `GTEST_FLAG` 宏放置在 `InitGoogleTest` 函数之前，这样在命令行中可以通过参数修改。

注意，`--gtest_list_tests` 参数是只能在命令行参数中使用的。

## 其它

### 整合CMake

可以将上述的 `main.cpp` 文件重命名为 `test.cpp` ，然后添加如下的 CMakeLists.txt 文件。

```
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(FooBar)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")

FIND_PACKAGE(GTest REQUIRED)
FIND_PACKAGE(Threads REQUIRED)

INCLUDE_DIRECTORIES(${GTEST_INCLUDE_DIRS})

ADD_EXECUTABLE(GTestFoobar test.cpp)
TARGET_LINK_LIBRARIES(GTestFoobar ${GTEST_BOTH_LIBRARIES})
TARGET_LINK_LIBRARIES(GTestFoobar ${CMAKE_THREAD_LIBS_INIT})

ADD_TEST(Test GTestFoobar)
ENABLE_TESTING()
```

## 参考

源码维护在 [Github](https://github.com/google/googletest) 上，也同时包括了很多有用的 Wiki信息，例如 [README.md](https://github.com/google/googletest/blob/master/googletest/README.md)

<!--
https://github.com/google/googletest/blob/master/googletest/docs/primer.md
https://github.com/google/googletest/blob/master/googletest/docs/advanced.md
https://github.com/google/googletest/blob/master/googlemock/README.md
-->

{% highlight text %}
{% endhighlight %}
