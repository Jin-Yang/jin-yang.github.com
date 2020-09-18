---
title: MySQL 表修复
layout: post
comments: true
language: chinese
category: [mysql,database]
keywords: mysql,shutdown,关闭
description: 简单分析下 mysqld 进程关闭的过程，并讨论如何安全地关闭 MySQL 实例。
---


<!-- more -->

MySQL 提供了一个命令行工具 mysqlcheck 来执行一些表的检查，而实际上这个命令只是提供了一种方便的使用 SQL 语句的方式，会根据不同类型拼接 SQL 语句，真正调用的还是 ```CHECK TABLE```、```REPAIR TABLE```、```ANALYZE TABLE``` 和 ```OPTIMIZE TABLE``` 命令。

解析时对应的 SQL 定义如下。

{% highlight cpp %}
enum enum_sql_command {
  ... ...
  SQLCOM_REPAIR,
  SQLCOM_OPTIMIZE,
  SQLCOM_CHECK,
  SQLCOM_ANALYZE,
  ... ...
};
{% endhighlight %}

关于上述的命令的解析，详细可以参考 ```sql/sql_yacc.yy``` 中的定义。

{% highlight text %}
repair:
          REPAIR opt_no_write_to_binlog table_or_tables
          {
            LEX *lex=Lex;
            lex->sql_command = SQLCOM_REPAIR;
            lex->no_write_to_binlog= $2;
            lex->check_opt.init();
            lex->alter_info.reset();
            /* Will be overriden during execution. */
            YYPS->m_lock_type= TL_UNLOCK;
          }
          table_list opt_mi_repair_type
          {
            THD *thd= YYTHD;
            LEX* lex= thd->lex;
            DBUG_ASSERT(!lex->m_sql_cmd);
            lex->m_sql_cmd= new (thd->mem_root) Sql_cmd_repair_table();
            if (lex->m_sql_cmd == NULL)
              MYSQL_YYABORT;
          }
        ;

optimize:
          OPTIMIZE opt_no_write_to_binlog table_or_tables
          {
            LEX *lex=Lex;
            lex->sql_command = SQLCOM_OPTIMIZE;
            lex->no_write_to_binlog= $2;
            lex->check_opt.init();
            lex->alter_info.reset();
            /* Will be overriden during execution. */
            YYPS->m_lock_type= TL_UNLOCK;
          }
          table_list
          {
            THD *thd= YYTHD;
            LEX* lex= thd->lex;
            DBUG_ASSERT(!lex->m_sql_cmd);
            lex->m_sql_cmd= new (thd->mem_root) Sql_cmd_optimize_table();
            if (lex->m_sql_cmd == NULL)
              MYSQL_YYABORT;
          }
        ;

check:
          CHECK_SYM table_or_tables
          {
            LEX *lex=Lex;

            if (lex->sphead)
            {
              my_error(ER_SP_BADSTATEMENT, MYF(0), "CHECK");
              MYSQL_YYABORT;
            }
            lex->sql_command = SQLCOM_CHECK;
            lex->check_opt.init();
            lex->alter_info.reset();
            /* Will be overriden during execution. */
            YYPS->m_lock_type= TL_UNLOCK;
          }
          table_list opt_mi_check_type
          {
            THD *thd= YYTHD;
            LEX* lex= thd->lex;
            DBUG_ASSERT(!lex->m_sql_cmd);
            lex->m_sql_cmd= new (thd->mem_root) Sql_cmd_check_table();
            if (lex->m_sql_cmd == NULL)
              MYSQL_YYABORT;
          }
        ;

analyze:
          ANALYZE_SYM opt_no_write_to_binlog table_or_tables
          {
            LEX *lex=Lex;
            lex->sql_command = SQLCOM_ANALYZE;
            lex->no_write_to_binlog= $2;
            lex->check_opt.init();
            lex->alter_info.reset();
            /* Will be overriden during execution. */
            YYPS->m_lock_type= TL_UNLOCK;
          }
          table_list
          {
            THD *thd= YYTHD;
            LEX* lex= thd->lex;
            DBUG_ASSERT(!lex->m_sql_cmd);
            lex->m_sql_cmd= new (thd->mem_root) Sql_cmd_analyze_table();
            if (lex->m_sql_cmd == NULL)
              MYSQL_YYABORT;
          }
        ;
{% endhighlight %}

而在真正执行时，会调用如下的函数，可以看到实际上就是调用了上述类中的 execute() 函数，而类的定义在 sql/sql_admin.h 中定义，实现同样对应 sql/sql_admin.c 文件。

{% highlight cpp %}
int mysql_execute_command(THD *thd, bool first_level)
{
  ... ...
  switch (lex->sql_command) {
  case SQLCOM_ANALYZE:
  case SQLCOM_CHECK:
  case SQLCOM_OPTIMIZE:
  case SQLCOM_REPAIR:
    DBUG_ASSERT(lex->m_sql_cmd != NULL);
    res= lex->m_sql_cmd->execute(thd);
    break;
  }
  ... ...
}
{% endhighlight %}


{% highlight text %}
{% endhighlight %}
