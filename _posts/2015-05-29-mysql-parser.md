---
title: MySQL 语法解析
layout: post
comments: true
language: chinese
category: [mysql,database]
keywords: mysql,lex,bison,yacc,flex,词法,语法
description: 当服务器接收到一条 SQL 语句时，其处理过程为：词法分析，语法分析，语义分析，构造执行树，生成执行计划，计划的执行。其中，词法语法解析的处理过程根编译原理上的东西基本类似；MySQL 并没有使用 lex 来实现词法分析，但是语法分析却用了 yacc。与之对比的 SQLite 数据库，其词法分析器是手工写的，语法分析器由 Lemon 生成。在此介绍其在 MySQL 中的使用。
---

当服务器接收到一条 SQL 语句时，其处理过程为：词法分析，语法分析，语义分析，构造执行树，生成执行计划，计划的执行。

其中，词法语法解析的处理过程根编译原理上的东西基本类似；MySQL 并没有使用 lex 来实现词法分析，但是语法分析却用了 yacc。

与之对比的 SQLite 数据库，其词法分析器是手工写的，语法分析器由 Lemon 生成。

在此介绍其在 MySQL 中的使用。

<!-- more -->

## 源码解析

Linux 一般来说，词法和语法解析都是通过 Flex 与 Bison 完成的；而在 MySQL 中，词法分析使用自己的程序，而语法分析使用的是 Bison；Bison 会根据 MySQL 定义的语法规则，进行语法解析。

完成语法解析后，会将解析结果生成的数据结构保存在 struct LEX 中，该结构体在 sql/sql_lex.h 文件中定义。

{% highlight cpp %}
struct LEX: public Query_tables_list
{
  friend bool lex_start(THD *thd);

  SELECT_LEX_UNIT *unit;                 ///< Outer-most query expression
  /// @todo: select_lex can be replaced with unit->first-select()
  SELECT_LEX *select_lex;                ///< First query block
  SELECT_LEX *all_selects_list;          ///< List of all query blocks
private:
  /* current SELECT_LEX in parsing */
  SELECT_LEX *m_current_select;
  ... ...
}
{% endhighlight %}

优化器会根据这里的数据，生成相应的执行计划，最后调用存储引擎执行。

### 执行过程

以下是语法解析模块掉用过程。

{% highlight text %}
mysql_parse()
 |-mysql_reset_thd_for_next_command()
 |-lex_start()
 |-query_cache_send_result_to_client()              #  首先查看cache
 |-parse_sql()                                      #  MYSQLparse的外包函数
   |-MYSQLparse()                                   #  实际的解析函数入口
{% endhighlight %}

如上，SQL 解析入口会调用 MYSQLparse ，而在 sql/sql_yacc.cc 中有如下的宏定义，也就说，在预编译阶段，会将 yyparse 替换为 MYSQLparse ，所以 **实际调用的仍是 yyparse 函数**。

{% highlight c %}
#define yyparse         MYSQLparse
{% endhighlight %}

记下来详细介绍其实现细节。

### 词法解析

MYSQL 的词法分析并没有使用 LEX，而是有自己的一套词法分析，代码详见 sql/sql_lex.cc 中的实现，其入口函数是 MYSQLlex() 。

{% highlight cpp %}
int MYSQLlex(YYSTYPE *yylval, YYLTYPE *yylloc, THD *thd)
{
  ... ...
  token= lex_one_token(yylval, thd);
  yylloc->cpp.start= lip->get_cpp_tok_start();
  yylloc->raw.start= lip->get_tok_start();

  switch(token) {
  case WITH:
    /*
      Parsing 'WITH' 'ROLLUP' or 'WITH' 'CUBE' requires 2 look ups,
      which makes the grammar LALR(2).
      Replace by a single 'WITH_ROLLUP' or 'WITH_CUBE' token,
      to transform the grammar into a LALR(1) grammar,
      which sql_yacc.yy can process.
    */
    token= lex_one_token(yylval, thd);
    switch(token) {
    case CUBE_SYM:
      yylloc->cpp.end= lip->get_cpp_ptr();
      yylloc->raw.end= lip->get_ptr();
      lip->add_digest_token(WITH_CUBE_SYM, yylval);
      return WITH_CUBE_SYM;
    case ROLLUP_SYM:
      yylloc->cpp.end= lip->get_cpp_ptr();
      yylloc->raw.end= lip->get_ptr();
      lip->add_digest_token(WITH_ROLLUP_SYM, yylval);
      return WITH_ROLLUP_SYM;
    default:
      /*
        Save the token following 'WITH'
      */
      lip->lookahead_yylval= lip->yylval;
      lip->yylval= NULL;
      lip->lookahead_token= token;
      yylloc->cpp.end= lip->get_cpp_ptr();
      yylloc->raw.end= lip->get_ptr();
      lip->add_digest_token(WITH, yylval);
      return WITH;
    }
    break;
  }
  ... ...
}
{% endhighlight %}



### 语法分析

Bison 和词法分析的函数接口是 yylex()，在需要的时候掉用 yylex() 获取词法解析的数据，并完成自己的语法解析。

正常来说，Bison 的实际入口函数应该是 yyparse() ，而在 MySQL 中通过宏定义，将 yyparse() 替换为 MYSQLParse()；如上所述，实际调用的仍然是 yyparse() 。

另外，我们可以根据 Bison 中的 Action 操作来查看 MySQL 解析结果的存储结构。



## 调试

在这里通过考察存储的 WHERE 数据结构来查看语法解析的结果。

{% highlight text %}
(gdb) attach PID
(gdb) set print pretty on                                  # 设置显示样式
(gdb) b mysql_execute_command                              # 可以用来查看所有的SQL
(gdb) p thd->lex->select_lex
(gdb) p ((Item_cond*)thd->lex->select_lex->where)->list    # 查看WHERE中的list
(gdb) detach
{% endhighlight %}





{% highlight text %}
{% endhighlight %}
