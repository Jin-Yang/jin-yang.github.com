---
title: InnoDB AHI
layout: post
comments: true
language: chinese
category: [mysql,database]
keywords: mysql,innodb,adaptive hash index,ahi
description: InnoDB 索引采用 B+Tree 组织，一般情况下需要根据查询条件，从根节点开始寻路到叶子节点，找到满足条件的记录，为了减少开销，做了几点优化。如下简单介绍下 AHI 的优化。
---

InnoDB 索引采用 B+Tree 组织，一般情况下需要根据查询条件，从根节点开始寻路到叶子节点，找到满足条件的记录，为了减少开销，做了几点优化。

<!--
首先，对于连续记录扫描，InnoDB在满足比较严格的条件时采用row cache的方式连续读取8条记录（并将记录格式转换成MySQL Format），存储在线程私有的row_prebuilt_t::fetch_cache中；这样一次寻路就可以获取多条记录，在server层处理完一条记录后，可以直接从cache中取数据而无需再次寻路，直到cache中数据取完，再进行下一轮。

另一种方式是，当一次进入InnoDB层获得数据后，在返回server层前，当前在btree上的cursor会被暂时存储到row_prebuilt_t::pcur中，当再次返回InnoDB层捞数据时，如果对应的Block没有发生任何修改，则可以继续沿用之前存储的cursor，无需重新定位。

上面这两种方式都是为了减少了重新寻路的次数，而对于一次寻路的开销，则使用Adaptive hash index来解决。AHI是一个内存结构，严格来说不是传统意义上的索引，可以把它理解为建立在Btree索引上的“索引”。

在 Innodb 中，实现了一个自动监测各表索引的变化情况的机制，然后通过一系列的算法来判定如果 存在一个 Hash Index 是否会对索引搜索带来性能改善。如果 Innodb 认为可以通过 Hash Index 来提高检 索效率，他就会在内部自己建立一个基于某个 B-Tree 索引的 Hash Index，而且会根据该 B-Tree 索引的 变化自行调整，这就是我们常说的 Adaptive  Hash Index。当然，Innodb 并不一定会将整个 B-Tree 索引 完全的转换为 Hash Index，可能仅仅只是取用该 B-Tree 索引键一定长度的前缀来构造一个 Hash Index。

Adaptive  Hash Index 并不会进行持久化存放在磁盘上面，仅仅存在于 Buffer  Pool 中。所以，在每 次 MySQL 刚启动之后是并不存在 Adaptive  Hash Index 的，只有在停工服务之后，Innodb 才会根据相应 的请求来构建。

Adaptive  Hash Index 的目的并不是为了改善磁盘 IO 的性能，而是为了提高 Buffer  Pool 中的数据 的访问效率，说的更浅显一点就是给 Buffer  Pool 中的数据做的索引。所以，Innodb 在具有大容量内存
（可以设置大的 Buffer  Pool）的主机上，对于其他存储引擎来说，会存在一定的性能优势。
-->

如下简单介绍下 AHI 的优化。

<!-- more -->

## Adaptive Hash Index

哈希索引是一种非常快的等值查找方法，它查找的时间复杂度为常量，但必须是等值，对非等值查找方法无能为力。

InnoDB 采用自适用哈希索引技术，它会实时监控表上索引的使用情况，如果认为建立哈希索引可以提高查询效率，则自动在内存中的 "自适应哈希索引缓冲区" 建立哈希索引。

之所以该技术称为 "自适应" 是因为完全由 InnoDB 自己决定，不需要人为干预；它是通过缓冲池中的 B+ 树构造而来，且不需要对整个表建立哈希索引，因此它的数据非常快。

可以通过 innodb_adaptive_hash_index 变量决定是否打开该特性，通过 AHI 可以有效提高读取、写入、join 的速度。

### AHI 监控项

可以通过 ```SHOW ENGINE INNODB STATUS``` 命令查看，在 ```INSERT BUFFER AND ADAPTIVE HASH INDEX``` 段中。

{% highlight text %}
mysql> SHOW ENGINE INNODB STATUS\G
... ...
-------------------------------------
INSERT BUFFER AND ADAPTIVE HASH INDEX
-------------------------------------
Ibuf: size 1, free list len 0, seg size 2, 0 merges
merged operations:
 insert 0, delete mark 0, delete 0
discarded operations:
 insert 0, delete mark 0, delete 0
Hash table size 34679, used cells 0, node heap has 0 buffer(s)
Hash table size 34679, used cells 0, node heap has 0 buffer(s)
Hash table size 34679, used cells 0, node heap has 0 buffer(s)
Hash table size 34679, used cells 0, node heap has 0 buffer(s)
Hash table size 34679, used cells 0, node heap has 0 buffer(s)
Hash table size 34679, used cells 0, node heap has 0 buffer(s)
Hash table size 34679, used cells 0, node heap has 0 buffer(s)
Hash table size 34679, used cells 0, node heap has 0 buffer(s)
0.00 hash searches/s, 0.00 non-hash searches/s
... ...
{% endhighlight %}

可以通过 ```information_schema.innodb_metrics``` 来监控 AHI 模块的运行状态，首先打开监控。

{% highlight text %}
----- 打开所有与AHI相关的监控项
mysql> SET GLOBAL innodb_monitor_enable=module_adaptive_hash;
Query OK, 0 rows affected (0.00 sec)

----- 查看监控项是否已经开启
mysql> SELECT name,status FROM information_schema.innodb_metrics WHERE subsystem='adaptive_hash_index';
+------------------------------------------+---------+
| name                                     | status  |
+------------------------------------------+---------+
| adaptive_hash_searches                   | enabled |
| adaptive_hash_searches_btree             | enabled |
| adaptive_hash_pages_added                | enabled |
| adaptive_hash_pages_removed              | enabled |
| adaptive_hash_rows_added                 | enabled |
| adaptive_hash_rows_removed               | enabled |
| adaptive_hash_rows_deleted_no_hash_entry | enabled |
| adaptive_hash_rows_updated               | enabled |
+------------------------------------------+---------+
8 rows in set (0.00 sec)

----- 重置所有的计数
mysql> SET GLOBAL innodb_monitor_reset_all = 'adaptive_hash%';
Query OK, 0 rows affected (0.00 sec)
{% endhighlight %}

该表搜集了 AHI 子系统诸如 AHI 查询次数，更新次数等信息，可以很好的监控其运行状态，在某些负载下，AHI 并不适合打开，关闭 AHI 可以避免额外的维护开销。

## 源码解析

接下来看看 InnoDB 中的实现。

### AHI初始化

AHI 在内存中就是一个普通的哈希表对象，存储在 ```btr_search_sys_t.hash_index``` 中，对 AHI 的查删改操作都是通过一个全局读写锁 ```btr_search_latches``` 来保护。

{% highlight text %}
struct btr_search_sys_t{
    hash_table_t**  hash_tables;
};
rw_lock_t**      btr_search_latches;

innobase_start_or_create_for_mysql()
 |-buf_pool_init()                     ← 在buff pool初始化完成后执行
   |-btr_search_sys_create()           ← 创建AHI
{% endhighlight %}

在实例启动，完成 buffer pool 初始化后，会初始化 AHI 子系统相关对象，并分配 AHI 内存，大小为 buffer pool 的 1/64 。

MySQL 5.7 已开始支持 Buffer Pool 的动态调整，其策略是 Buffer Pool 的大小改变超过 1 倍，就调用 ```btr_search_sys_resize()``` 重新分配 AHI Hash 内存。

### AHI 的维护

维护包括多个动作：A) 索引叶页面何时进入 Hash Index；B) 何时可以使用 Hash Index 加锁查询；C) 何时将索引叶页面从 Hash Index 中移除，等等。



<!--
B+树叶页面进入Adaptive Hash Index
新增B+树索引叶页面的Hash索引，则是在Search Path之后，btr_cur_search_to_nth_level() -> btr0sea.ic::btr_search_info_update()。

B+ 树叶页面进入 Hash Index的条件
InnoDB不是每一次进行Search Path之后，就将当前定位到的叶页面中的所有Records按照键值前缀做Hash，并存入Hash表，而是有至少三个前提：
前提一：一定的次数的Search Path(btr_search_info_update()函数)：
#define BTR_SEARCH_HASH_ANALYSIS                17
/* After change in n_fields or n_bytes in info, this many rounds are waited before starting the hash analysis again: this is to save CPU time when there is no hope in building a hash index. */


<pre style="font-size:0.8em; face:arial;">
### Adaptive Hash Index的使用流程
btr_cur_search_to_nth_level()               @btr0cur.c
 |-btr_search_guess_on_hash()

        // 获取上一个进入Hash Index的叶页面，使用了索引中的多少个完全列，
        // 以及最后一列使用了多少个Bytes用于计算Hash键值
        cursor->n_fields = search_info->n_fields;
        cursor->n_bytes = search_info->n_bytes;

        // 根据选择的索引键值前缀，计算给定Tuple对应的Hash索引值
        // 前提是，必须保证给定Tuple的列数量，要超过键值前缀数量；
        fold = dtuple_fold(tuple, cursor->n_fields, cursor->n_bytes, index_id);

        // 根据计算得来的fold，查询Adaptive Hash Index；
        ha_search_and_get_data(btr_search_sys->hash_index, fold);
        …
        // 检查当前Hash Index命中的叶页面，是否满足Search Path的条件
        btr0sea.cc::btr_search_check_guess();
            page0page.ic::page_cmp_dtuple_rec_with_match();

// 对比叶页面中通过Hash Index定位到的当前记录，以及
                // 用户给定的tuple (完整 or Partial)，n_cmp为对比的列数，
                // matched_fields为完全匹配的列数，*_bytes为第一个不匹配
                // 列中匹配的字节数
                // @return 1, 0, -1
                // 1：  dtuple大于页面中的rec
                // 0：  dtuple与页面中的rec相等
                // -1： dtuple小于页面中的rec
                rem0cmp.cc::cmp_dtuple_rec_with_match_low(dtuple, rec,
offsets, n_cmp, matched_fields, matched_bytes);

    // 设置本次完全匹配的列数，以及最后一列匹配的字节数
                    *matched_fields = cur_field;
                    *matched_bytes = cur_bytes;

                // 若查询模式为L or LE，则判断当前Rec的下一条记录是否满足
                // 比查询Tuple大的条件：
                // 1. next_rec为SUPREMUM记录，并且当前页面为索引最后一个页面
                //  则一定满足条件；
                // 2. next_rec不为SUPREMUM记录，则比较next_rec与tuple，判断
                //  比较的返回值是否为-1，标识tuple小于next_rec；
                if((mode == PAGE_CUR_L) || (mode == PAGE_CUR_LE))
                    next_rec = page_rec_get_next(rec);
</pre>

<br><br><h2>触发 AHI 信息统计</h2><p>
在系统刚启动时，索引对象上没有足够的信息来启发是否适合进行 AHI 缓存，因此开始有个信息搜集的阶段，在索引对象上维护了相关信息，用于跟踪当前索引使用 AHI 的关键信息。

<pre style="font-size:0.8em; face:arial;">
struct btr_search_t{
    ulint   ref_count;  /*!< Number of blocks in this index tree
                that have search index built
                i.e. block->index points to this index.
                Protected by btr_search_latch except
                when during initialization in
                btr_search_info_create(). */

    /* @{ The following fields are not protected by any latch.
    Unfortunately, this means that they must be aligned to
    the machine word, i.e., they cannot be turned into bit-fields. */
    buf_block_t* root_guess;/*!< the root page frame when it was last time
                fetched, or NULL */
    ulint   hash_analysis;            # 若设置了新的建议前缀索引模式，则重置为0，随后的17次查询分析可以忽略更新search_info
    ibool   last_hash_succ; /*!< TRUE if the last search would have
                succeeded, or did succeed, using the hash
                index; NOTE that the value here is not exact:
                it is not calculated for every search, and the
                calculation itself is not always accurate! */
    ulint   n_hash_potential;         # 如果使用AHI构建索引，潜在的可能成功的次数
    ulint   n_fields;                 # 推荐构建AHI的索引列数
    ulint   n_bytes;                  # 是否在相同索引前缀的最左索引记录构建AHI；
                                      # 值为true时，则对于相同前缀索引的记录，只存储最右的那个记录
    ibool   left_side;  /*!< TRUE or FALSE, depending on whether
                the leftmost record of several records with
                the same prefix should be indexed in the
                hash index */
    /*---------------------- @} */
#ifdef UNIV_SEARCH_PERF_STAT
    ulint   n_hash_succ;    /*!< number of successful hash searches thus
                far */
    ulint   n_hash_fail;    /*!< number of failed hash searches */
    ulint   n_patt_succ;    /*!< number of successful pattern searches thus
                far */
    ulint   n_searches; /*!< number of searches */
#endif /* UNIV_SEARCH_PERF_STAT */
#ifdef UNIV_DEBUG
    ulint   magic_n;    /*!< magic number @see BTR_SEARCH_MAGIC_N */
/** value of btr_search_t::magic_n, used in assertions */
# define BTR_SEARCH_MAGIC_N 1112765
#endif /* UNIV_DEBUG */
};

struct dict_index_t {
    ... ...
    btr_search_t search_info;           # 相关统计信息
    ... ...
};


btr_search_info_create()
</pre>
通过 btr_search_t 中的 n_fields 和 left_side 字段，可以指导选择哪些列作为索引前缀来构建 (fold, rec) 哈希记录。如果用户的 SQL 的索引前缀列的个数大于等于构建 AHI 时的前缀索引，就可以用上 AHI。<br><br>


Tip1：在５.7之前的版本中，还支持索引中的字符串前缀作为构建AHI的键值的一部分，但上游认为带来的好处并不明显，因此将btr_search_t::n_bytes 移除了(参见commit 6f5f19b338543277a108a97710de8dd59b9dbb60, 42499d9394bf103a27d63cd38b0c3c6bd738a7c7）。
Tip2：然而上游在测试中发现，如果把n_bytes移除，可能在诸如顺序插入这样的场景存在性能退化(参阅commit 00ec81a9efc1108376813f15935b52c451a268cf)，因此在新发布的MySQL5.7.8版本中又重新引入，本文分析代码时统一基于MySQL5.7.7版本。

两种情况需要构建建议的前缀索引列：
    当前是第一次为该索引做AHI分析，btr_search_t::n_hash_potential值为0，需要构建建议的前缀索引列；
    新的记录匹配模式发生了变化(info->left_side == (info->n_fields <=cursor->low_match))，需要重新设置前缀索引列。






第一次执行 SQL 时，从 btree 的 root 节点开始，当寻址到匹配的叶子节点时，会走如下逻辑。
<pre style="font-size:0.8em; face:arial;">
void btr_cur_search_to_nth_level(... ...) {
    ... ...
    if (btr_search_enabled && !index->disable_ahi) {
            btr_search_info_update(index, cursor);
    }
    ... ...
}
</pre>
其中 index-&gt;diable_ahi 是MySQL5.7对临时表的优化，避免临时表操作对全局对象的影响，针对临时表不做AHI构建。
<pre style="font-size:0.8em; face:arial;">
void btr_search_info_update( ... ... ) {
    ... ...
    info = btr_search_get_info(index);
    info->hash_analysis++;
    if (info->hash_analysis < BTR_SEARCH_HASH_ANALYSIS) { // 连续执行17次对相同索引的操作后，满足info->hash_analysis大
        return;                          // 于等于BTR_SEARCH_HASH_ANALYSIS的条件，就会调用下面的函数来更新search_info
    }                                    // 这主要是为了避免频繁的索引查询分析产生的过多CPU开销。
    btr_search_info_update_slow(info, cursor); // 对该索引寻路到叶子节点17次后，才会去做AHI分析
}

void btr_search_info_update_slow( ... ... ) {
    ... ...
    block = btr_cur_get_block(cursor);
    btr_search_info_update_hash(info, cursor);     // 更新索引上的查询信息
    build_index = btr_search_update_block_hash_info(info, block, cursor);

    if (build_index || (cursor->flag == BTR_CUR_HASH_FAIL)) {
        btr_search_check_free_space_in_heap();
    }

    if (cursor->flag == BTR_CUR_HASH_FAIL) {
        rw_lock_x_lock(&btr_search_latch);
        btr_search_update_hash_ref(info, block, cursor);
        rw_lock_x_unlock(&btr_search_latch);
    }

    if (build_index) {
        params = (ulint*) mem_alloc(3 * sizeof(ulint));
        params[0] = block->n_fields;
        params[1] = block->n_bytes;
        params[2] = block->left_side;
        params2 = params + btr_search_this_is_zero;
        btr_search_build_page_hash_index(cursor->index,
                         block,
                         params2[0],
                         params2[1],
                         params2[2]);
        mem_free(params);
    }
}





btr_cur_search_to_nth_level()               @btr0cur.cc
 |-btr_search_info_update()                 @btr0sea.ic
   |-btr_search_get_info()
   |-btr_search_info_update_slow()
     |-btr_search_info_update_hash()        # 根据当前Search Path的定位结果(cursor)，以及Index的hash Index search info
     |                                      # 重新计算Hash索引所需要的Key，是完整的索引键值，或者是索引键值前缀；逻辑较复杂
     |-btr_search_update_block_hash_info()  # 根据前面提到的，判断当前页面是否需要进行Hash索引
     |-btr_search_build_page_hash_index()

            // 对当前页面中的所有记录，创建Hash索引，Hash键值为前面
            // 提到的提取出来的完整索引键值或者键值前缀
            // 若当前页面已经被Hash，则首先删除旧的Hash，然后增加新Hash
            // 注意：
            // 1. buffer header上有一个重要的参数——left_side，用于控制
            //  拥有相同hash值的记录，是保持第一条，还是保存最后一条
            // 2. index->search_info->ref_count：此参数用于标识当前索引有多少
            //  页面被Hash索引了，在删除、关闭索引前，需要保证此计数归零
</pre>

<!--
InnoDB通过索引条件构建一个可用于查询的tuple，而AHI需要根据tuple定位到叶子节点上记录的位置，既然AHI是构建在Btree索引上的索引，它的键值就是通过索引的前N列的值计算的来，所有的信息搜集统计都是为了确定一个合适的”Ｎ” ，这个值也是个动态的值，会跟随应用的负载自适应调整并触发block上的AHI重构建。

btr_search_info_update_slow包含三个部分：更新索引查询信息、block上的查询信息以及为当前block构建AHI，下面几小节分别介绍。

相关代码段：
if (cursor->up_match == cursor->low_match) {
        info->n_hash_potential = 0;

        /* For extra safety, we set some sensible values here */

        info->n_fields = 1;
        info->left_side = TRUE;

} else if (cursor->up_match > cursor->low_match) {
        info->n_hash_potential = 1;

        if (cursor->up_match >= n_unique) {
                info->n_fields = n_unique;
        } else if (cursor->low_match < cursor->up_match) {
                info->n_fields = cursor->low_match + 1;
        } else {
                info->n_fields = cursor->low_match;
        }

        info->left_side = TRUE;
} else {
        info->n_hash_potential = 1;

        if (cursor->low_match >= n_unique) {

                info->n_fields = n_unique;
        } else if (cursor->low_match > cursor->up_match) {

                info->n_fields = cursor->up_match + 1;
        } else {
                info->n_fields = cursor->up_match;
        }

        info->left_side = FALSE;
}

从上述代码可以看到，在low_match和up_match之间，选择小一点match的索引列数的来进行设置，但不超过唯一确定索引记录值的列的个数：

    当low_match小于up_match时，left_side设置为true，表示相同前缀索引的记录只缓存最左记录；
    当low_match大于up_match时，left_side设置为false，表示相同前缀索引的记录只缓存最右记录。

如果不是第一次进入seach_info分析，有两种情况会递增btr_search_t::n_hash_potential：

    本次查询的up_match和当前推荐的前缀索引都能唯一决定一条索引记录(例如唯一索引)，则根据search_info推荐的前缀索引列构建AHI肯定能命中，递增 info->n_hash_potential；

      if (info->n_fields >= n_unique && cursor->up_match >= n_unique) {
      increment_potential:
              info->n_hash_potential++;

              return;
      }

    本次查询的tuple可以通过建议的前缀索引列构建的AHI定位到。

      if (info->left_side == (info->n_fields <= cursor->up_match)) {

              goto increment_potential;
      }

很显然，如果对同一个索引的查询交替使用不同的查询模式，可能上次更新的search_info很快就会被重新设置，具有固定模式的索引查询将会受益于AHI索引。
更新block上的查询信息

参考函数：btr_search_update_block_hash_info

更新数据页block上的查询信息，涉及到修改的变量包括：

btr_search_info::last_hash_succ 最近一次成功(或可能成功)使用AHI；
buf_block_t::n_hash_helps 计数值，如果使用当前推荐的前缀索引列构建AHI可能命中的次数，用于启发构建／重新构建数据页上的AHI记录项；
buf_block_t::n_fields 推荐在block上构建AHI的前缀索引列数；
buf_block_t::left_side 和search_info上对应字段含义相同。

函数主要流程包括：

    首先设置btr_search_info::last_hash_succ 为FALSE
    这会导致在分析过程中无法使用AHI进行检索，感觉这里的设置不是很合理。这意味着每次分析一个新的block，都会导致AHI短暂不可用。

    初始化或更新block上的查询信息

     if ((block->n_hash_helps > 0)
         && (info->n_hash_potential > 0)
         && (block->n_fields == info->n_fields)
         && (block->left_side == info->left_side)) {

             if ((block->index)
                 && (block->curr_n_fields == info->n_fields)
                 && (block->curr_left_side == info->left_side)) {

                     /* The search would presumably have succeeded using
                     the hash index */

                     info->last_hash_succ = TRUE;
             }

             block->n_hash_helps++;
     } else {
             block->n_hash_helps = 1;
             block->n_fields = info->n_fields;
             block->left_side = info->left_side;
     }

    当block第一次被touch到并进入该函数时，设置block上的建议索引列值；以后再进入时，如果和索引上的全局search_info相匹配，则递增block->n_hash_helps，启发后续的创建或重构建AHI。

    如果当前数据页block上已经构建了AHI记录项，且buf_block_t::curr_n_fields等字段和btr_search_info上对应字段值相同时，则认为当前SQL如果使用AHI索引能够命中，因此将btr_search_info::last_hash_succ设置为true，下次再使用相同索引检索btree时就会尝试使用AHI。

    在初始化或更新block上的变量后，需要判断是否为整个page构建AHI索引：

     if ((block->n_hash_helps > page_get_n_recs(block->frame)
          / BTR_SEARCH_PAGE_BUILD_LIMIT)
         && (info->n_hash_potential >= BTR_SEARCH_BUILD_LIMIT)) {

             if ((!block->index)
                 || (block->n_hash_helps
                     > 2 * page_get_n_recs(block->frame))
                 || (block->n_fields != block->curr_n_fields)
                 || (block->left_side != block->curr_left_side)) {

                     /* Build a new hash index on the page */

                     return(TRUE);
             }
     }

    简单来说，当满足下面三个条件时，就会去为整个block上构建AHI记录项：
        分析使用AHI可以成功查询的次数(buf_block_t::n_hash_helps)超过block上记录数的16(BTR_SEARCH_PAGE_BUILD_LIMIT)分之一；
        btr_search_info::n_hash_potential大于等于BTR_SEARCH_BUILD_LIMIT (100)，表示连续100次潜在的成功使用AHI可能性；
        尚未为当前block构造过索引、或者当前block上已经构建了AHI索引且block->n_hash_helps大于page上记录数的两倍、或者当前block上推荐的前缀索引列发生了变化 。

为数据页构建AHI索引

如果在上一阶段判断认为可以为当前page构建AHI索引（函数btr_search_update_block_hash_info返回值为TRUE），则根据当前推荐的索引前缀进行AHI构建。

参考函数：btr_search_build_page_hash_index

分为三个阶段：

    检查阶段：加btr_search_latch的S锁，判断AHI开关是否打开；如果block上已经构建了老的AHI但前缀索引列和当前推荐的不同，则清空Block对应的AHI记录项（btr_search_drop_page_hash_index）；检查n_fields和page上的记录数；然后释放btr_search_latch的S锁；

    搜集阶段：根据推荐的索引列数计算记录fold值，将对应的数据页记录内存地址到数组里；

    根据left_mode值，相同的前缀索引列值会有不同的行为，举个简单的例子，假设page上记录为 (2,1), (2,2), (5, 3), (5, 4), (7, 5), (8, 6)，n_fields＝１
        若left_most为true，则hash存储的记录为(2,1) , (5, 3), (7, 5), (8,6)
        若left_most为false，则hash存储的记录为(2, 2), (5, 4), (7,5), (8, 6)

    插入阶段：加btr_search_latch的X锁，将第二阶段搜集的(fold, rec)插入到AHI中，并更新：

     if (!block->index) {
             index->search_info->ref_count++;
     }

     block->n_hash_helps = 0;

     block->curr_n_fields = n_fields;
     block->curr_left_side = left_side;
     block->index = index;

PS：由于第二阶段释放了btr_search_latch锁，这里还得判断block上的AHI信息是否发生了变化，如果block上已经构建了AHI且block->curr_*几个变量和当前尝试构建的检索模式不同，则放弃本次构建。
使用AHI

AHI的目的是根据用户提供的查询条件加速定位到叶子节点，一般如果有固定的查询pattern，都可以通过AHI受益，尤其是Btree高度比较大的时候。

入口函数：btr_cur_search_to_nth_level

相关代码：

        /* Use of AHI is disabled for intrinsic table as these tables re-use
        the index-id and AHI validation is based on index-id. */
        if (rw_lock_get_writer(&btr_search_latch) == RW_LOCK_NOT_LOCKED
            && latch_mode <= BTR_MODIFY_LEAF
            && info->last_hash_succ
            && !index->disable_ahi
            && !estimate
# ifdef PAGE_CUR_LE_OR_EXTENDS
            && mode != PAGE_CUR_LE_OR_EXTENDS
# endif /* PAGE_CUR_LE_OR_EXTENDS */
            && !dict_index_is_spatial(index)
            /* If !has_search_latch, we do a dirty read of
            btr_search_enabled below, and btr_search_guess_on_hash()
            will have to check it again. */
            && UNIV_LIKELY(btr_search_enabled)
            && !modify_external
            && btr_search_guess_on_hash(index, info, tuple, mode,
                                        latch_mode, cursor,
                                        has_search_latch, mtr)) {

从代码段可以看出，需要满足如下条件才能够使用AHI：

    没有加btr_search_latch写锁。如果加了写锁，可能操作时间比较耗时，走AHI检索记录就得不偿失了；
    latch_mode <= BTR_MODIFY_LEAF，表明本次只是一次不变更BTREE结构的DML或查询（包括等值、RANGE等查询）操作；
    btr_search_info::last_hash_succ为true表示最近一次使用AHI成功（或可能成功）了；
    打开AHI开关；
    查询优化阶段的估值操作，例如计算range范围等，典型的堆栈包括：handler::multi_range_read_info_const　–> ha_innobase::records_in_range –> btr_estimate_n_rows_in_range –> btr_cur_search_to_nth_level；
    不是spatial索引；
    调用者无需分配外部存储页(BTR_MODIFY_EXTERNAL，主要用于辅助写入大的blob数据，参考struct btr_blob_log_check_t)。

当满足上述条件时，进入函数btr_search_guess_on_hash，根据当前的查询tuple对象计算fold，并查询AHI；只有当前检索使用的tuple列的个数大于等于构建AHI的列的个数时，才能够使用AHI索引。

btr_search_guess_on_hash：

    首先用户提供的前缀索引查询条件必须大于等于构建AHI时的前缀索引列数，这里存在一种可能性：索引上的search_info的n_fields 和block上构建AHI时的cur_n_fields值已经不相同了，但是我们并不知道本次查询到底落在哪个block上，这里一致以search_info上的n_fields为准来计算fold，去查询AHI；
    在检索AHI时需要加&btr_search_latch的S锁；
    如果本次无法命中AHI，就会将btr_search_info::last_hash_succ设置为false，这意味着随后的查询都不会去使用AHI了，只能等待下一路查询信息分析后才可能再次启动（btr_search_failure）；
    对于从ahi中获得的记录指针，还需要根据当前的查询模式检查是否是正确的记录位置（btr_search_check_guess）。

如果本次查询使用了AHI，但查询失败了（cursor->flag == BTR_CUR_HASH_FAIL），并且当前block构建AHI索引的curr_n_fields等字段和btr_search_info上的相符合，则根据当前cursor定位到的记录插入AHI。参考函数：btr_search_update_hash_ref。

从上述分析可见，AHI如其名，完全是自适应的，如果检索模式不固定，很容易就出现无法用上AHI或者AHI失效的情况。
维护AHI

    关闭选项innodb_adaptive_hash_index；
        持有dict_sys->mutex和btr_search_latch的X锁；
        遍历dict_sys->table_LRU和dict_sys->table_non_LRU链表，将每个表上的所有索引的index->search_info->ref_count设置为0；
        释放dict_sys->mutex；
        遍历buffer pool，将block上的index标记(buf_block_t::index)清空为NULL；
        清空AHI中的哈希项，并释放为记录项分配的Heap；
        释放btr_search_latch。

    参考函数：btr_search_disable

    index->search_info的ref_count不为0时，无法从数据集词典cache中将对应的表驱逐，workaround的方式是临时关闭AHI开关；

    参考函数：dict_table_can_be_evicted、dict_index_remove_from_cache_low

    删除索引页上的记录，或者更新的是二级索引、或者更新了主键且影响了排序键值，则需要从AHI上将对应的索引记录删除；

    参考函数：btr_search_update_hash_on_delete

    插入新的记录时，如果本次插入未产生页面重组、操作的page为叶子节点，且本次插入操作使用过AHI定位成功，则先尝试更新再尝试插入，否则直接插入对应的AHI记录项；

    参考函数：btr_search_update_hash_node_on_insert、btr_search_update_hash_on_insert

    涉及索引树分裂或者节点合并，或从LRU中驱逐page（buf_LRU_free_page）时，需要清空AHI对应的page。

    参考函数：btr_search_drop_page_hash_index

shortcut查询模式

在row_search_mvcc函数中，首先会去判断在满足一定条件时，使用shortcut模式，利用AHI索引来进行检索。

只有满足严苛的条件时（例如需要唯一键查询、使用聚集索引、长度不超过八分之一的page size、隔离级别在RC及RC之上、活跃的Read view等等条件，具体的参阅代码），才能使用shortcut：

    加btr_search_latch的S锁；
    然后通过row_sel_try_search_shortcut_for_mysql检索记录；如果找到满足条件的记录，本次查询可以不释放 btr_search_latch，这意味着InnoDB/server层交互期间可能持有AHI锁，但最多在10000次（BTR_SEA_TIMEOUT）交互后释放AHI latch。一旦发现有别的线程在等待AHI X 锁，也会主动释放其拥有的S锁。

然而， Percona的开发Alexey Kopytov认为这种长时间拥有的btr_search_latch的方式是没有必要的，这种设计方式出现在很久之前加锁、解锁非常昂贵的时代，然而现在的CPU已经很先进了，完全没有必要，在Percona的版本中，一次shortcut的查询操作后都直接释放掉btr_search_latch（参阅bug#1218347）。
-->





<!--
http://mysql.taobao.org/monthly/2015/09/01/
-->

{% highlight text %}
{% endhighlight %}
