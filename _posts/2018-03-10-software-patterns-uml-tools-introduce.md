---
title: UML 建模工具
layout: post
comments: true
language: chinese
category: [misc]
keywords:
description:
---


<!-- more -->


软件建模就是对模型的抽象，可以从不同的视角，通过不同的模型表示一个系统，通常会选取某个最显著的特征并简化表示，常见包括了：

* 外部视角，对系统上下文或周边环境建模；
* 交互视角，系统中各个组件之间的交互过程进行建模；
* 结构化视角，系统中处理的数据结构建模；
* 行为视角，系统行为以及如何响应事件进行建模。

针对不同的建模思想对应的工具也不同，例如面向服务方法 Service Oriented Method, SOM、面向对象方法 Object Oriented Method, OOM 等，后者也是最常用的，这里仅介绍这一方法。

## UML

Unified Modeling Language, UML 统一建模语言，是面向对象开发中一种通用的图形化建模语言。

### 事物 Things

UML 中重要的组成部分，对一些面向对象中基本概念的抽象，是构成模型图的一些基本图示符号，包括了结构事物、行为事物、分组事物、注释事物。

#### 结构事物 Structural Thing

描述模型的静态部分，描述概念或物理元素，包括了类(Class)、接口(Interface)、协作(Collaboration)、用例(Use Case)、主动类(Active Class)、构件(Component)和节点(Node) 等。

#### 行为事物 Behavioral Thing

描述模型的动态部分，包括了两类主要的行为事物。

#### 分组事物 Grouping Thing

可以看做一个一个的盒子，每个盒子里面的对象关系相对复杂，而盒子与盒子之间的关系相对简单，最主要的分组事物是包。

#### 注释事物 Annotational Thing

解释部分，物用来描述、说明和标注模型的任何元素。

https://blog.csdn.net/Gnd15732625435/article/details/50716371

### 关系

用来将事物联系到一起，包括了泛化、实现、组合、聚合、关联、依赖。

#### 泛化 Generalization

一个类继承了另一个类的所有描述，并可以对父类进行扩展，通过箭头+实线表示 `is-a` 关系，例如基类为交通工具，子类为汽车、火车等。

#### 实现 Realization

针对于接口 (抽象类) 而言，子类必须要实现其定义的接口，通过箭头+虚线表示。

#### 组合 Composition

组合表示整体由部分构成，是关联关系的一个特例，整体与部分不可分割，整体生命周期结束也就意味着部分结束，比聚合要强，也称为强聚合，是 `contains-a` ，例如公司由多个部门组成，公司不存在则部门也不存在。

#### 聚合 Aggregation

同样表示整体由部分构成，也是关联关系的特例，但是整体和部分是可以分割的，部分有独立的生命周期，可以被多个整体共享，是 `has-a` 的关系。例如一个部门由多个员工组成。

#### 关联 Association

关联关系体现的是两个类之间的一种强依赖结构关系，是一种长期静态稳定的关系，与运行状态无关，一般由常识等因素决定的，例如乘车人和车票，使用直线表示，方向可选。

#### 依赖 Dependency include

一个类会使用到另外一个类或者接口(虚类)，与关联不同，两者通常在运行期间产生，而且随着运行依赖关系可能会发生变化，通过箭头+虚线表示。

https://blinkfox.github.io/2018/10/22/ruan-jian-she-ji/uml-ge-lei-guan-xi-jie-shao/
https://blog.csdn.net/gschen_cn/article/details/50685980

### 视图

包括了静态和动态两类：A) 静态图，用例图、类图、对象图、包图、构件图、部署图；B) 动态图，状态图、活动图、协作图、序列图。

#### 用例图 UseCase Diagrams

主要回答两个问题：1) 谁用软件；2) 软件的功能。从用户的角度描述了系统的功能，并指出各个功能的执行者，系统为执行者完成哪些功能。

#### 类图 Class Diagrams

#### 对象图 Object Diagrams

描述参与交互的各个对象在交互过程中某一时刻的状态，可以看作是类图在某一时刻的实例。

#### 构件图 Component Diagrams

表示系统中构件与构件之间，类或接口与构件之间的关系图。

#### 部署图 Deployment Diagrams

系统运行时进行处理的结点以及在结点上活动的构件的配置，强调了物理设备以及之间的连接关系。


#### 状态图 Statechart Diagrams

是一种由状态、变迁、事件和活动组成的状态机，用来描述类的对象所有可能的状态以及时间发生时状态的转移条件。

#### 活动图 Activity Diagrams

本质是一种流程图，它描述了活动到活动的控制流。

#### 协作图 Collaboration Diagrams

交互图的一种，描述了收发消息的对象的组织关系，强调对象之间的合作关系，时序图按照时间顺序布图，而写作图按照空间结构布图。

#### 序列图 Sequence Diagrams

描述了对象之间消息发送的先后顺序，强调时间顺序。

https://zhuanlan.zhihu.com/p/44518805




A1(合法合规)
防止非法监听、个人数据保护、禁止存在未公开接口。

A2(证明清白、撇清责任)
访问通道控制(通道安全、禁用无用端口、逻辑端口认证、物理端口认证)
日志审计
敏感数据保护
完整性保护

B(基本安全功能)
操作系统、数据库加固、口令安全、Web应用安全

安全三要素：可用性、完整性、机密性。
经典设计原则：
1. 开放设计原则 (Open Design) 设计不应该是秘密，开放设计更安全。
2. 失败-默认安全原则 (Fail-Safe Defaults) 默认不允许访问，保护机制仅用来识别允许访问的情况。
3. 权限分离原则 (Separation of Privilege) 一种保护机制需要两把钥匙解锁。
4. 权限最小化原则 (Least Privilege) 系统的每个用户、每个程序，都应该使用最小且必须的权限集来完成工作。
5. 经济适用原则 (Economy of Mechanism) 保持系统的设计和代码尽可能简单、紧凑。
6. 最小公共化原则 (Least Common Mechanism) 尽量避免提供多个对象共享同一资源。
7. 完全仲裁原则 (Complete Mediation) 每个对象的每次访问都要经过安全检查审核。
8. 心理可承受原则 (Psychological Acceptability) 安全机制可能对用户增加额外的负担，但这种负担必须最小而且合理。

美国海军计算机网络纵深

ASTRIDE 分析

必须要有业务驱动实体
存储和存储之间有处理过程
信任边界
三方存储为外部交互方

设计模式之禅（第2版）
https://github.com/zxiaofan/JavaBooks/blob/master/%E8%AE%BE%E8%AE%A1%E6%A8%A1%E5%BC%8F%E4%B9%8B%E7%A6%85%EF%BC%88%E7%AC%AC2%E7%89%88%EF%BC%89.pdf


主要目的是为了系统的复用和扩展。

单一职责原则 Single Responsibility Principle, SRP 实践起来很难，对于一个现实的抽象很难确定其单一职责。
里氏替换原则 Liskov Substitution Principle, LSP 可以将父类替换为子类而不会引起任何的错误或者异常，反之则不可。
依赖倒置原则 Dependence Inversion Principle, DIP 高层次不依赖低层次的模块，应该依赖其抽象。
开闭原则 Open Closed Principle, OCP 对修扩展开放，对修改关闭。
接口隔离原则 Interface Segregation Principles, ISP 建立单一的接口，不需要建立臃肿庞大的接口。
迪米特法则 Law of Demeter, LoD 最少知识原则 Least Knowledge Principle, LKP 一个类对耦合或调用类只需要了解最少内容。


创建型（5种）、、
单例模式 Singleton Pattern 一个类全局只有一个实例，并向整个系统提供服务。
工厂方法模式 Factory Pattern
抽象工厂模式 Abstract Factory Pattern
建造者模式 Builder Pattern 将部件与组装过程分离开，例如一个汽车需要装配发动机、座椅、轮胎等。

结构型（7种）
适配器模式 Adapter Pattern 把一个类的接口变换成客户端所期待的另一种接口，从而使原本接口不匹配而无法一起工作的两个类能够在一起工作。
外观模式 Facade Pattern 定义一个高层、统一的接口，外部与通过这个统一的接口对子系统中的一群接口进行访问。

行为型（11种）
观察者模式  建立对象与对象的依赖关系，当某个对象改变时自动通知其它对象，被通知的对象将相应作出反应。
责任链模式 Chain of Responsibility Pattern 每个接收者都包含对另一个接收者的引用，如果一个对象不能处理该请求，那么它会把相同的请求传给下一个接收者，依此类推。

http://c.biancheng.net/design_pattern/

Role-Based Access Control, RBAC 基于角色的访问控制，通过分配和取消角色来完成用户权限的授予和取消，使动作主体 (用户) 与资源的行为 (权限) 分离。

威胁建模 (Threat Model) 

在信息安全领域，有三个基本要素：A) 机密性 (Confidentiality) 保护信息免向未授权的人披露；B) 一致性 (Integrity) 保护信息免受未授权的人更改；C) 可用性(Availability) 让信息供已授权的人需要时可取用。

公司或者个人的资产 (Assets) 会受到各种威胁 (Threats) 影响，包括了人为原因 (骇客恶意破坏)、自然原因 (火灾地震等) 等等，通过利用系统的脆弱性 (Vulnerabilities) 就可能导致暴露 (Exposure)，这实际上就是风险 (Risk)，那么就需要使用防护措施 (Safeguards)，缓解风险使资产得到安全保障。

威胁建模比较常用的 STRIDE 分析。

### Spoofing 欺骗

常见的如手机接入移动网络时是不会认证，就可能导致垃圾甚至钓鱼短信，也包括了钓鱼网站。

做好认证鉴权，例如密码、SSH、TLS/SSL 等。

### Tampering 篡改

例如直接修改系统账单，导致收益下降；中间人通过数据流修改达到欺骗目的。

保证完整性，例如 MAC、数字签名等。

### Repudiation 抵赖

恶意操作之后修改日志完成。

加强可追溯，例如认证、审计日志等，但是需要确保审计无法被篡改。

### Information Disclosure 信息泄露

主要是用户信息以及密码的保存或者传输方式异常导致，例如密码明文保存、传输。

机密保证，例如加密、ACL等。

### Denial of Service 拒绝服务

因为网络的漏洞导致攻击的成本远小于对业务造成的损失。

保证可用性，例如清洗、限流、缓存等。

### Elevation of Privileges 权限提升

在处理过程中的权限高于实际需要的权限。

加强权限控制，例如权限最小化、采用沙箱等。

https://zhuanlan.zhihu.com/p/73205112

{% highlight text %}
{% endhighlight %}
