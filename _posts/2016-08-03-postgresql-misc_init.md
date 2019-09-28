---
Date: October 19, 2013
title: PG 常用命令
layout: post
comments: true
language: chinese
category: [sql]
---

在此简单记录下，PostgreSQL 中常用的 SQL 指令，维护是常见的场景，还有些奇技淫巧。

<!-- more -->


# .psqlrc

该文件类似 bashrc，会在启动 psql 时执行该文件中的命令，通常可以在该文件下可以添加一些常用的 SQL 脚本，一般路径默认为 ~/.psqlrc 或者通过 PSQLRC 环境变量指定。

{% highlight text %}
--- \pset : for changing the output format.
--- \set  : for everything else.

-- Do not display the "help" message on startup.
\set QUIET 1

-- https://www.postgresql.org/docs/current/static/app-psql.html
\set PROMPT1 '%[%033[1m%]%M %n@%/%R%[%033[0m%]%# '
-- PROMPT2 is printed when the prompt expects more input.
\set PROMPT2 '[more] %R > '

-- Show how long each query takes to execute.
\timing

-- Use best available output format.
\x auto

-- Ignore duplicated commands.
\set HISTCONTROL ignoredups

-- How many history commands should be saved.
\set HISTSIZE 2000

-- Display detail message about the error.
--\set VERBOSITY verbose

-- How psql handles errors.
\set ON_ERROR_ROLLBACK interactive

-- Print "(NULL)" instead of "null"
\pset null '(NULL)'


-- ????
\set COMP_KEYWORD_CASE upper





-- Or \set QUIET OFF
\unset QUIET


\echo '\nCurrent Host Server Date Time: '`date` '\n'
\echo 'Administrative queries:\n'


\set foobar 'select * from pg_database;'

-- Set your own personal settings.
--\i ~/.psqlrc.local
{% endhighlight %}

http://opensourcedbms.com/dbms/psqlrc-psql-startup-file-for-postgres/


# 常用脚本、命令

首先，需要在 postgresql.conf 中把 track_activites = on 打开，此时 stats collector process 会监控每个会话的 SQL 语句。

首先第一个脚本 viewsql.sh 会根据 pid 查看当前进程的 SQL 执行情况，这个脚本会显示指定的 pid 的会话目前正在执行的 SQL 语句；那么可以通过如下方式监控当前最占资源进程当前执行的 SQL 。

{% highlight text %}
$ top -b -c -n 1 | grep 'postgres:' | awk '{print $1}'| while read i; do ./viewsql.sh $i; done;
{% endhighlight %}

第二个脚本 topsql.sh 查看全部进程 SQL 的执行情况。

第三个脚本 killsession.sh 用来 kill 占用资源高的 session。



{% highlight text %}
----- 查看配置项
-----   1. 当前目录位置配置，可以根据分类来查找
SELECT name, setting, source FROM pg_settings WHERE category = 'File Locations';
-----   2. 或者直接根据变量的名称进行查找
SELECT name, setting, source FROM pg_settings WHERE name = 'data_directory';
-----   3. 会话变量设置
SHOW work_mem;
SET work_mem='16MB';
SELECT name, setting FROM pg_settings WHERE source = 'session';
RESET work_mem;
-----   4. 查看那些变量被修改过
SELECT name, setting, source FROM pg_settings WHERE source NOT IN ('default', 'override', 'configuration file');

----- 查看数据库的启动时间
SELECT pg_postmaster_start_time() AS start_time;

----- 查看数据库到目前为止的启动时间
SELECT date_trunc('second', current_timestamp - pg_postmaster_start_time()) AS uptime;

----- 查看有多少用户自己的表
SELECT table_catalog, table_schema, table_name, table_type FROM information_schema.tables
   WHERE table_schema NOT IN ('pg_catalog', 'information_schema');
----- 可以查看information_schema.tables的定义
\d information_schema.tables

----- 各种对象大小查看
-----     1. 查看当前、所有、某个数据库大小
SELECT pg_database_size(current_database());
SELECT datname, pg_database_size(datname) FROM pg_database;
\l+
SELECT pg_size_pretty(pg_database_size('foobar'));
-----     2. 查看表大小
SELECT pg_size_pretty(pg_relation_size('tbl'));
\dt+ tbl
-----     3. 查看表、索引、视图等在内的所有大小
SELECT pg_size_pretty(pg_total_relation_size('tbl'));
-----     4. 查看表的记录数、非空所有行、去重后总行数
SELECT count(1) FROM tbl;
SELECT count(col-name) FROM tbl;
SELECT count(DISTINCT col-name) FROM tbl;
-----     5. 上述方式当表数量大时会比较慢，可以用如下方式估算
SELECT (CASE WHEN reltuples > 0 THEN pg_relation_size('tbl')/(8192*relpages/reltuples)
   ELSE 0 END)::bigint as estimated_row_count FROM pg_class
   WHERE oid='tbl'::regclass;
-----     6. 查看最大的表
SELECT table_name, pg_relation_size(table_name) AS size FROM information_schema.tables WHERE table_schema
   NOT IN ('information_schema', 'pg_catalog') order by size desc limit 10;
SELECT relname, relpages FROM pg_class ORDER BY relpages DESC;

----- 查看表的索引
\d tbl

----- 查看查询的执行计划
EXPLAIN query;
----- 在服务端执行查询来显示执行计划，会执行但不返回结果
EXPLAIN ANALYZE query;

----- 列出PG中基本的数据类型
SELECT typname, typlen FROM pg_type WHERE typtype='b';

----- 创建不会持久化的表
CREATE UNLOGGED TABLE test(id int);

----- 查看监听端口?????
SELECT inet_server_port();

----- 查看当前用户
postgres=# SELECT current_user;

----- 查看当前PG版本
postgres=# SELECT version();
{% endhighlight %}


在第13个命令中，那个typtype='b'是什么意思？

typtype='b'表示basetype。b==basetype.

PostgreSQL有这么几种数据类型: composite types, domains, and pseudo-types.

http://developer.postgresql.org/pgdocs/postgres/extend-type-system.html

{% highlight text %}
{% endhighlight %}
