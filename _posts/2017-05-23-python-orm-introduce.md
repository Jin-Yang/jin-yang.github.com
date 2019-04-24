---
title: Python ORM 简介
layout: post
comments: true
language: chinese
category: [program, python]
keywords: python,orm
description: 很多数据库都可以通过相应的库，直接编写原生的 SQL 语句进行操作，实现增删改查，但是如果功能比较复杂，就会导致重用性不强。Object Relational Mapping, ORM 建立在数据库 API 之上，使用关系对象映射进行数据库操作；简言之，将对象转换成 SQL，然后使用数据 API 执行 SQL 并获取执行结果。
---

很多数据库都可以通过相应的库，直接编写原生的 SQL 语句进行操作，实现增删改查，但是如果功能比较复杂，就会导致重用性不强。

Object Relational Mapping, ORM 建立在数据库 API 之上，使用关系对象映射进行数据库操作；简言之，将对象转换成 SQL，然后使用数据 API 执行 SQL 并获取执行结果。

<!-- more -->

## SQLAlchemy

可以直接使用 `pip install SQLAlchemy` 命令安装，详细可以查看 [www.sqlalchemy.org](http://www.sqlalchemy.org/) 。

<!--
2.7版本使用mysqldb
3.5版本使用pymysql
-->

### 使用类

SQLAlchemy 有多种方式，通过 `declarative_base()` 会生成一个 class 对象作为基类，这也是在大型项目中使用最多的一种场景。

注意，SQLAlchemy 无法修改表结构，如果需要可以使用 SQLAlchemy 开发者开源的另外一个软件 Alembic 来完成。

### 1. 创建表

{% highlight python %}
# -*- coding: utf8 -*-

import sqlalchemy as orm
from sqlalchemy import Column, Integer, String, text, TIMESTAMP
from sqlalchemy.ext.declarative import declarative_base

# 创建实例，并连接test库，同时回显信息
# 链接地址格式为: "数据库类型+数据库驱动名称://用户名:口令@机器地址:端口号/数据库名"
engine = orm.create_engine("mysql+mysqldb://root:new_password@127.0.0.1:5506/test", encoding='utf-8', echo=True)

BaseModel = declarative_base()  # 生成orm基类

class User(BaseModel):
    __tablename__ = 'user'  # 表名

    id = Column(Integer, primary_key=True, autoincrement=True)
    name = Column(String(32))
    password = Column(String(64))

BaseModel.metadata.create_all(engine) # 创建表结构，通过父类调用子类
#BaseModel.metadata.drop_all(engine)   # 删除所有表
{% endhighlight %}

运行时会显示相关信息，包括生成的 sql 语句。

<!--
除上面的创建之外，还有一种创建表的方式

from sqlalchemy import Table, MetaData, Column, Integer, String, ForeignKey
from sqlalchemy.orm import mapper

metadata = MetaData()

user = Table('user', metadata,
            Column('id', Integer, primary_key=True),
            Column('name', String(50)),
            Column('fullname', String(50)),
            Column('password', String(12))
        )

class User(object):
    def __init__(self, name, fullname, password):
        self.name = name
        self.fullname = fullname
        self.password = password

mapper(User, user)  # 类User 和 user关联起来
# the table metadata is created separately with the Table construct,
# then associated with the User class via the mapper() function
# 如果数据库里有，就不会创建了。
-->

### 2 数据增删改查


{% highlight python %}
# -*- coding: utf8 -*-

import sqlalchemy as orm
from sqlalchemy import Column, Integer, String
from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy.orm import sessionmaker

# 创建实例，并连接test库，同时回显信息
# 链接地址格式为: "数据库类型+数据库驱动名称://用户名:口令@机器地址:端口号/数据库名"
#engine = orm.create_engine("mysql+mysqldb://root:new_password@127.0.0.1:5506/test", encoding='utf-8', echo=True)
engine = orm.create_engine("mysql+mysqldb://root:new_password@127.0.0.1:5506/test", encoding='utf-8')

BaseModel = declarative_base()  # 生成orm基类

class User(BaseModel):
    __tablename__ = 'user'  # 表名

    id = Column(Integer, primary_key=True)
    name = Column(String(32))
    password = Column(String(64))

    def  __repr__(self):
        return "<User(name='%s',  password='%s')>" % (self.name, self.password)

#BaseModel.metadata.create_all(engine) # 创建表结构，通过父类调用子类
SessionModel = sessionmaker(bind = engine)  # 注意返回的是一个Class而非实例
session = SessionModel()

# 1. 写入数据
#user1 = User(name="test1", password="just for test")
#user2 = User(name="test2", password="just for test")
#user3 = User(name="test3", password="just for test")
#
#session.add(user1)  # 添加一个
#session.add_all([user1, user2, user3])  # 添加多个
#user2.name = "change2"  # 只要未提交可以直接修改
#session.commit()  # 提交或者 session.rollback() 回滚

# 2. 查询数据
query = session.query(User)
#print query  # 只打印查询的SQL语句
#print query.all()    # 查询所有
#print query.first()  # 查询第一条
#user1 = query.filter_by(name = "test1").first()   # 过滤单条数据
#user1 = query.filter(User.name == "test1").all()  # 查询所有name是'test1'的用户，注意等值判断
#user1 = query.filter(User.id >= 2).all()          # 通过ID查询
#user1 = query.filter(User.id >= 2).filter(User.id < 5).all()  # 多条件查询
#print user1    # 会调用User.__repr__()函数，或者 print user1.name, user1.password

# 3. 修改数据
user1 = query.filter_by(name = "test1").first()   # 过滤单条数据
user1.name = "update1"
#session.rollback()      # 如果不需要可以直接回滚
session.commit()
{% endhighlight %}

### TimeStamp

在使用 `db.create_all()` 进行初始化创建表的时候，如果为 Column 指定了 default 的值，并不会影响创建的表中的对应列的默认值，这些 default 的值仅仅是在使用 SQLAlchemy 系统插入值的时候会提供默认值。

如果你希望影响 MySQL 中 Column 的默认值，必须使用 server_default 来指定，例如要设置一个 Colmun 默认值为0 ，则需要设定 `server_default=text('0')` 。

使用下面的代码创建一个默认值不为空的 TIMESTAMP Column ：

{% highlight python %}
gmt_modify = db.Column(db.TIMESTAMP(True), nullable=False)
gmt_create = db.Column(db.TIMESTAMP(True), nullable=False)
{% endhighlight %}

如果对一个 TIMESTAMP Column 使用了 `nullable=False` 属性，那么对于 MySQL 5.6 会自动将第一列加入 `DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP`，而对于之后的列则设置为 `NOT NULL DEFAULT '0000-00-00 00:00:00'` 。

然而，对于上述的 gmt_modify 会在每次更新该 Recored 的时都会自动更新，所以，如果需要给时间戳类型加入默认值，但不在每次更新的时候自动更新时间戳，可以这样做：

{% highlight python %}
#----- 每次更新条目的时候，本字段会自动更新时间戳
gmt_modify = db.Column(db.TIMESTAMP(True), nullable=False)
#----- 创建时间，每次更新条时，本字段不会自动更新时间戳
gmt_create = db.Column(db.TIMESTAMP(True), nullable=False, server_default=text('NOW()'))
gmt_create = db.Column(db.TIMESTAMP(True), nullable=False, server_default=text('CURRENT_TIMESTAMP'))
{% endhighlight %}

#### 使用 default

也可以把默认值设置为空，然后通过 SQLAlchemy Column 提供的 default 在 python 层面自动加入默认值：

{% highlight python %}
gmt_modify = db.Column(db.TIMESTAMP(True), nullable=True, default=func.utcnow())
{% endhighlight %}

这种情况下，MySQL 中没有设定 updatetime 的默认值，但是在给 Column 赋值的时候，python 会使用 utcnow 自动为其加入默认值。

也就是说，这是在 SQLAlchemy 层面实现的，并不是在 MySQL 中实现的。

<!-- 另外，很多文章提到了 使用 server_default=text('0') 作为默认值。在 MySQL5.7上，这个默认值是不可用的： -->

## 参考

<!--
https://blog.csdn.net/tastelife/article/details/25218895
-->


{% highlight python %}
{% endhighlight %}
