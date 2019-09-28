---
title: MySQL 测试框架
layout: post
comments: true
language: chinese
category: [mysql,database]
keywords: mysql,测试,mysqltest
description: MySQL 测试包含了多种类型的，当然同时也会含多种工具，例如 MySQL 的测试框架、基于 Test Anything Protocol, TAP 的单元测试框架、基于 Google Test Framework 的单元测试、性能压测工具、Random Query Generator 等等。在此介绍与测试框架相关的内容。
---

MySQL 测试包含了多种类型的，当然同时也会含多种工具，例如 MySQL 的测试框架、基于 Test Anything Protocol, TAP 的单元测试框架、基于 Google Test Framework 的单元测试、性能压测工具、Random Query Generator 等等。

在此介绍与测试框架相关的内容。

<!-- more -->

## 简介

当给 MySQL 打了补丁之后，

不仅需要测试新增的功能，同时更重要的问题是，需要对原有的功能作回归――若新增的patch导致原有其他功能产生bug，就得不偿失。

MySQL自动测试框架是一个以MySQL框架和内部引擎为测试对象的工具。主要执行脚本在发布路径的mysql-test目录下。自动测试框架的主要测试步骤，是通过执行一个case，将该case的输出结果，与标准的输出结果作diff。这里的“标准输出结果”指代在可信任的MySQL版本上的执行结果。

如果某个case的执行结果与标准输出结果不同，则说明正在执行的这个MySQL服务有问题，或许是框架，或许是引擎。 当然若干case执行正确并不能确保被测试的框架和引擎是没有问题的，除非能够证明执行的case已经覆盖了所有的分支（这太难了）。

这里说到的case，是指一系列的语句，包括SQL语句和一些必要的shell语句。在mysql-test/t/目录下，可以找到很多这样的case，他们的文件名以.test为后缀。接下来我们用一个简单的例子来说明用法。



2、初体验

       再简单的例子也不如自己写的简单。因此我们通过自己作一个case，来体验一下这个框架的便捷。

a)          在mysql-test/t/目录下创建文件 mytest.test， 内容为
  1 — source include/have_innodb.inc



  2 use test;

  3 create table t(c int) engine=InnoDB ;

  4 insert into t values(1);

  5 select c from t;

  6 drop table t;

       从第二行开始是我们熟悉的SQL语句，创建一个InnoDB表，插入一行，执行一个查询，删除这个表。输出也是显而易见的。

b)      在mysql-test/r/目录下创建文件 mytest.result， 内容为
  1 use test;



  2 create table t(c int) engine=InnoDB ;

  3 insert into t values(1);

  4 select c from t;

  5 c

  6 1

  7 drop table t;



c)      在mysql-test/ 目录下执行 ./mtr mytest




===================================================================

TEST                                      RESULT   TIME (ms)

————————————————————

worker[1] Using MTR_BUILD_THREAD 300, with reserved ports 13000..13009

worker[1] mysql-test-run: WARNING: running this script as _root_ will cause some tests to be skipped

main.mytest                              [ skipped ]  No innodb support

main.mytest ‘innodb_plugin’              [ pass ]      5

————————————————————

The servers were restarted 0 times

Spent 0.005 of 3 seconds executing testcases

Completed: All 1 tests were successful.

最后一句话说明，测试通过了。

说明：

1)      mysql-test/mtr这个文件，是一个perl脚本。同目录下还有 mysql-test-run 和mysql-test-run.pl这两个文件，这三个文件一模一样。

2)      每个case会启动一个mysql服务，默认端口为13000。如果这个case涉及到需要启动多个服务（比如主从），则端口从13000递增。

3)      ./mtr的参数只需要指明测试case的前缀即可，如你看到的。当你执行./mtr testname会自动到t/目录下搜索 testname.test文件来执行。当然你也可以执行./mtr mytest.test， 效果相同。

4)      mytest.test第一行是必须的，当你需要使用InnoDB引擎的时候。可以看到mtr能够解释source语法，等效于将目标文件的内容全部拷贝到当前位置。Mysql-test/include目录下有很多这样的文件，他们提供了类似函数的功能，以简化每个case的代码。注意souce前面的 –， 大多数的非SQL语句都要求加 –。

5)      mytest.test最后一行是删除这个创建的表。因为每个case都要求不要受别的case影响，也不要影响别的case，因此自己在case中创建的表要删除。

6)      mtr会将mytest.test的执行结果与r/mytest.result作diff。 若完全相同，则表示测试结果正常。注意到我们例子中的mytest.result. 其中不仅包括了这个case的输出，也包括了输入的所有内容。实际上有效输出只有5、6两行。如果希望输出文件中只有执行结果，可以在第一行后面加入 — disable_query_log。

3、冲突结果及处理

              我们来测试一个错误的输入输出。就在上文的mytest.test中加入一行，如下：
  1 — source include/have_innodb.inc



  2 — disable_query_log

  3 use test;

  4 create table t(c int) engine=InnoDB ;

  5 insert into t values(1);

  6 select c from t;

  7 drop table t;

       执行 ./mtr mytest，我们知道，他只会输出两行，分别为c和1.

执行结果
。。。。。。



@@ -1,7 +1,2 @@

-use test;

-create table t(c int) engine=InnoDB ;

-insert into t values(1);

-select c from t;

 c

 1

-drop table t;

mysqltest: Result length mismatch

。。。。。。

Completed: Failed 1/1 tests, 0.00% were successful.

最后一句话说明我们的这个case测试没有通过。我们知道mtr将执行结果与r/mytest.result文件作diff，这个结果的前面部分就是diff的结果。

       说明：

1)      执行case失败有很多中可能，比如中间执行了某个非法的语句，这个例子中的失败，指代的是执行结果与预期结果不同。

2)      当前的执行结果会保存在r/mytest.reject中

3)      如果mytest.reject中的结果才是正确的结果（错入出现在mytest.result中），可以用mytest.reject将result覆盖掉，这样正确的标准测试case就完成了。可以直接使用 ./mtr mytest –record命令生成mytest.result.

4)      注意mtr在作diff的时候是直接文本比较，因此如果你的case中出现了多次执行结果可能不同的情况（比如时间相关），这不是一个好的case。当然处理的办法是有的，请参加附录中关于 replace_column的描述。

      4、批量执行的一些命令

实际上我们更常用到的是批量执行的命令。

1)      ./mtr

就是这么简单。会执行所有的case。当然包括刚刚我们加入的mytest.这里说的”所有的case”，包括t/目录下所有以.test为后缀的文件。也包括 suits目录下的所有以.test为后缀的文件。

注意只要任何一个case执行失败（包括内部执行失败和与result校验失败）都会导致整个执行计划退出。因此–force很常用，加入这个参数后，mtr会忽略错误并继续执行下一个case直到所有的case执行结束。

2)      ./mtr –suite=funcs_1

Suits目录下有多个目录，是一些测试的套餐。此命令单独执行 suits/funcs_1目录下的所有case。（其他目录不执行）

t/目录下的所有文件组成了默认的套餐main。 因此 ./mtr –suite=main则只执行t/*.test.

3)      ./mtr  –do-test=events

执行所有以 events为前缀的case（搜索范围为t/和所有的suite）。

–do-test的参数支持正则表达式，上诉命令等效于./mtr –do-test=events.*

所以如果想测试所有的包括innodb的case，可以用 ./mtr –do-test=.*innodb.*

5、结束了

好吧。上面说的一些太简单了，简单到入门都不够。需要详细了解的可以到官网看e文。以下的tips是看官网的一些笔记。看到一点记录一点的，不成章。

附录
1、目录下的mtr文件即为mysql-test-run的缩写，文件内容相同



2、Mtr会启动mysql，有些case下可能重启server，为了使用不同的参数启动

3、调用bin/mysqltest读case并发送給mysql-server

4、输入和输出分别放在不同的文件中，执行结果与输出文件内容作对比

5、输入文件都在t目录下，输出文件在r目录下。对应的输入输出文件仅后缀名不同，分别为 *.test 和 *.result

6、每个testfile是一个测试用例，多个测试用例之间可能有关联。 一个file中任何一个非预期的失败都会导致整个test停止（使用force参数则可继续执行）。

7、注意如果服务端输出了未过滤的warning或error，则会也会导致test退出。

8、t目录下的*.opt文件是指在这个测试中，mysql必须以opt文件的内容作为测试参数启动。 *.sh文件是在执行启动mysql-server之前必须提前执行的脚本。disabled.def中定义了不执行的testfile。

9、r目录下， 若一个执行输出结果和testname.result文件不同，会生成一个testname.reject文件。 该文件在下次执行成功之后被删除。

10、            include目录是一些头文件，这些文件在t/*.test文件中使用，用source 命令引入

11、            lib目录是一些库函数，被mtr脚本调用

12、            有些测试需要用到一些标准数据，存在std_data目录下。

13、            Suite目录也是一些测试用例，每个目录下包含一套，./mtr –suite=funcs_1执行suits/funcs_1目录下的所有case

14、            Mtr实际调用mysqltest作测试。 –result-file文件用于指定预定义的输出，用于与实际输出作对比。若同时指定了 –recored参数，则表示这个输出数据不是用来对比的，而是要求将这个输出结果写入到指定的这个文件中。

15、            Mtr所在的目录路径中不能有空格

16、            Mtr  –force参数会跳过某个错误继续执行，以查看所有的错误。

17、            执行./mtr时另外启动了一个mysql server，默认端口13000

18、            指定执行某个具体case使用 ./mtr testname, 会自动使用t/testname.rest

19、            ./mtr  –do-test=events 执行以events开头的所有case，包括events_grant.test 等

   同理 –skip-test=events 将以events打头的所有case跳过。 这里支持正则表达是，如./mtr –do-test=.*innodb.*则会执行t/目录下所有包含innodb子串的case

20、            Mtr允许并行执行，端口会从13000开始使用。但需要特别指定不同的—vardir指定不同的日志目录。但并行执行的case可能导致写同一个testname.reject.

21、            使用–parallel=auto可以多线程执行case。

22、            Mtr对比结果使用简单的diff，因此自己编写的测试case不应该因为执行时间不同而导致结果不同。当然框架在执行diff之前，允许自定义处理规则对得到的result作处理，来应对一些变化。

23、            ./mtr –record test_name 会将输出结果存入文件 r/test_name.result

24、            自己在脚本中创建的库、表等信息，要删除，否则会出warnning

25、            Mtr默认使用的是mysql-test/var/my.cnf文件，需要替换该文件为自定义的配置文件

26、            若要使用innodb引擎，必须明确在testname.test文件头加

– source include/have_innodb.inc

27、            Test文件名由字母数字、下划线、中划线组成，但只能以字母数字打头

28、            — sleep 10 等待10s 注意前面的—和后面没有分号

29、            默认情况下， r/testname.result中会包含原语句和执行结果，若不想输出原语句，需要自t/restname.test文件头使用 — disable_query_log

30、            每个单独的case会重启服务并要求之前的数据是清空的。

31、            如果要测试出错语句，必须在testname.test文件中，在会出错的语句之前加入 –error 错误号。

比如重复创建表，语句如下

create table t(c int) engine=InnoDB ;

– error 1050

create table t(c int) engine=InnoDB ;

这样在testname.result中输出

create table t(c int) engine=InnoDB ;

  ERROR 42S01: Table ‘t’ already exists

则能够正常通过

也可使用 ER_TABLE_EXISTS_TABLE （宏定义为1050）

也可使用 –error S42S01 （注意需要加前缀S），但S系列的可能一个错误号对应多种错误。

32、            –enable_info 在testname.test文件头增加这个命令，在结果中会多输出影响行数。

    对应的关闭命令为 –disable_info

33、            enable_metadata 可以显示更多信息 disable_result_log不输出执行结果

34、            有些case的输出结果可能包含时间因素的影响，导致无法重复验证。–replace_column 可以解决部分情况。

–replace_column 1 XXXXX

Select a, b from t;

输出结果会是

XXXXX     b.value

即将第一列固定替换为xxxxx。 注意，每个replace_column的影响范围仅局限于下一行的第一个select语句。

35、            mtr –mysqld=–skip-innodb –mysqld=–key_buffer_size=16384 用这种将参数启动传递給mysql server 。 每个选项必须有一个—mysqld打头，不能连在一起写，即使引号包含多个也不行。

36、            mysql-test-run.pl –combination=–skip-innodb   –combination=–innodb,–innodb-file-per-table

这个命令是参数传给多个test case， 第一个参数传 skip-innodb， 第二个参数传 –innodb, innodb-file-per-table。  若所有启动参数中combination只出现一次，则无效。

37、            skip-core-file 强行控制server不要core

38、            如果需要在server启动前执行一些脚本，可以写在 t/testname.sh文件中，由mtr自动执行。

39、            等待语句

let $wait_condition= SELECT c = 3 FROM t;

–source include/wait_condition.inc

              Mtr会一直停在 source这行，每隔0.1s检测，直到上个语句中的$wait_condition返回值非0

40、            主从测试case

                  a)        testname.test头部必须包含 souce include/master-slave.inc

                  b)        主库的配置写在testname-master.opt， 从库的写在testname-slave.opt

                  c)        对出从库的操作统一写在testname.test中，要对主库操作时，先执行connection master，之后的语句都是在主库上操作；同理connection slave;

从库上执行start slave之后要执行 –source include/wait_for_slave_to_start.inc 等待启动完成， 执行stop slave之后要执行–source include/wait_for_slave_to_stop.inc 等待停止完成。

41、            Case完成后，mtr会检测server的错误日志，如果里面包含Error或Warning，则会认为case fail。

                 a)        如果要忽略整个验证server日志的过程，可以在文件头增加 –nowarnings

                 b)        如果要指定忽略某些行，允许使用正则表达式，比如 call mtr.add_suppression(“The table ‘t[0-9]*’ is full”); 能够忽略 The table ‘t12′ is full 这样的warning

                 c)        注意上面这个call语句也是输入语句的一部分，因此会输出到结果内容中，除非

–disable_query_log

call mtr.add_suppression(“The table ‘t[0-9]*’ is full”);

–enable_query_log

42、            在一个case中重启server

–exec echo “wait” > $MYSQL_TMP_DIR/mysqld.1.expect

–shutdown_server 10

–source include/wait_until_disconnected.inc

# Do something while server is down

–enable_reconnect

–exec echo “restart” > $MYSQL_TMP_DIR/mysqld.1.expect

–source include/wait_until_connected_again.inc

43、            循环语句语法

let $1= 1000;

while ($1)

{

 # execute your statements here

 dec $1;

}

44、            mysql_client_test是一个单独的case，里面包含了多个case， 但并不是写成脚本，而是直接调用，可以直接从源码中看到里面调用的各个语句。源码位置tests/mysql_client_test.c

45、            直接执行 ./mtr 会执行t/目录下的所有case，外加suits目录

46、            若在不同目录中有重名的case，则会依次全部执行

47、            Mysql-stress-test.pl用于压力测试，注意默认的my.cnf中的参数，比如innodb_buffer_pool_size只有128M

48、            Case中的语句是以分号结尾的。echo a; select 1 from t limit 1; echo b;是三个语句，在result文件中的对应输出是  a \n 1 \n b

49、            Testcase中支持的脚本语言函数
http://dev.mysql.com/doc/mysqltest/2.0/en/mysqltest-commands.html

没有列出的函数可以用 – exec +shell命令实现

50、            设置变量 let $a = xx， 前面可加 –

51、            若是数字，可使用 inc $a / dec $a， 前面可加 –

52、            可以赋值为sql返回值 let $q = `select c from t limit 1`,
    可以赋值为系统变量 let $q = $PATH









## 参考

关于 MySQL 的测试框架可参考官方的手册 [The MySQL Test Framework](https://dev.mysql.com/doc/mysqltest/en/)，也可下载 PDF 文档。

[Random Query Generator](https://launchpad.net/randgen) 产生随机数据，并生成随机的查询 SQL，用来测试 SQL 解析。

<!--
http://dinglin.iteye.com/blog/804798
-->


{% highlight text %}
{% endhighlight %}
