---
title: C++ STL 使用
layout: post
comments: true
language: chinese
category: [linux]
keywords:
description:
---


<!-- more -->

### STL

C++ STL(标准模板库) 提供了通用的模板类和函数，说白了，就是借助模板将常用的数据结构和算法都实现了一遍，核心包括了如下几个组件：

* 容器 Containers，管理某一类对象的集合，例如 `deque` `list` `vector` `map` 等。
* 算法 Algorithms，算法作用于容器，对容器内容执行初始化、排序、搜索和转换等操作。
* 迭代器 Iterators，遍历对象集合的元素，可以是容器或者是容器的子集。

常见的，vector 的底层为数组，list 的底层为双向链表，deque 的底层为循环队列，set 的底层为红黑树，hash_set 的底层为哈希表。

### string

### vector

`push_back()` 要比 `insert()` 快。

### list

这个是双向链表，单向链表为 `foward_list` 。

`list` 容器不能调用 `algorithm` 中的排序函数，因为排序函数要求容器必须可以随机存储，而 `list` 做不到，所以，`list` 自己做了一个排序函数。

### set

元素是有序的且唯一。

### map

{% highlight cpp %}
#include <set>
#include <map>
#include <list>
#include <string>
#include <vector>

#include <iostream>

int main(void)
{
        //===== String
        std::string S0;
        std::string S1 = "Hello World";
        std::string S2("Hi");
        std::string S3(5, 'H'); // "HHHHH"
        std::string S4(S1);     // Deep Copy
        std::string S5 = S1;    // Deep Copy

        std::cout << S0.empty() << std::endl << S1.size() << std::endl;
        for (std::string::iterator i = S1.begin(); i != S1.end(); i++)
                std::cout << *i << std::endl;
        for (std::string::const_iterator i = S1.begin(); i != S1.end(); i++)
                std::cout << *i << std::endl;


        //===== Vector
        std::vector<int> V0;
        std::vector<int> V1 = {1, 2, 3, 4, 5};
        std::vector<int> V2(5);       // {0, 0, 0, 0, 0}
        std::vector<int> V3(5, 0);    // {0, 0, 0, 0, 0}

        std::cout << V0.empty() << std::endl << V1.size() << std::endl;
        V1.push_back(6);              // {1, 2, 3, 4, 5, 6}
        V1.pop_back();                // {1, 2, 3, 4, 5}
        V1.insert(V1.begin() + 1, 2); // {1, 2, 2, 3, 4, 5}
        V1.erase(V1.begin() + 1);     // {1, 2, 3, 4, 5}
        V1.insert(V1.begin(), 2, 0);  // {0, 0, 1, 2, 3, 4, 5}
        for (std::vector<int>::reverse_iterator i = V1.rbegin(); i != V1.rend(); i++)
                std::cout << *i << std::endl;
        V1.clear();

        //===== List
        std::list<int> L0;
        std::list<int> L1 = {5, 4, 3, 2, 1};

        L1.sort();


        //===== Set
        std::set<int> SS0;
        std::set<int> SS1 = {1, 2, 3, 2, 1}; // {1, 2, 3}

        SS1.insert(9);
        for (std::set<int>::iterator i = SS1.begin(); i != SS1.end(); i++)
                std::cout << *i << std::endl;


        //===== Map
        std::map<std::string, int>M0;
        M0["Andy"]  = 10;
        M0["James"] = 20;
        for (std::map<std::string, int>::iterator i = M0.begin(); i != M0.end(); i++)
                std::cout << i->first << " " << i->second << std::endl;

        return 0;
}
{% endhighlight %}


{% highlight text %}
{% endhighlight %}
