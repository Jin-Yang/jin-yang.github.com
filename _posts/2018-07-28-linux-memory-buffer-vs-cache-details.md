---
title: Linux Cache VS. Buffer
layout: post
comments: true
language: chinese
category: [linux]
keywords:
description:
---

实际上 Buffer 和 Cache 是两个用烂的词，在不同的场景下其语义会有所区别。在 Linux 的内存管理中，Buffer 是指 Buffer Cache(缓冲区缓存)，Cache 是指 Page Cache(页面缓存)。

这里简单介绍其概念。

<!-- more -->

## 简介

曾经 Buffer 被用来做 IO 设备的写缓存，Cache 被用来作为 IO  设备的读缓存，这里的 IO 设备，主要指的是块设备文件和文件系统上的普通文件；但是现在，它们的意义已经不一样了。

在当前的内核中，PageCache 就是针对内存页的缓存，如果有内存是以 Page 进行分配管理的，都可以使用 PageCache 作为其缓存来管理使用。

当然，不是所有的内存都是以 Page 为单位进行管理，也有很多是针对块 Block 进行管理的，如果这部分需要使用到 Cache 功能，则都集中到 BufferCache 中，从这一角度来说，改称为 BlockCache 更为合适。

### 前世今生

简单来说，两者都是为了优化磁盘 IO 的读写速率，其中 PageCache 缓存了文件页用来优化文件 IO；而 BufferCache 缓存了磁盘块用来优化块设备的 IO 。

很多的类 Unix 系统采用了与 Linux 2.4 之前版本类似的策略，也就是文件缓存在 PageCache 而磁盘块缓存在 BufferCache。

而实际上，大部分文件是通过文件系统呈现，而且存储在磁盘上，这就会导致同一份文件保存了两份，不优雅也不高效，为此，在 Linux 2.4 版本之后，就将两者进行了统一。

如果被缓存的数据即是文件数据又是块数据 (对于文件来说大部分的数据是的，元数据不是)，此时 BufferCache 会有指针指向 PageCache ，这样数据就只需要在内存中缓存一份。当讨论磁盘缓存时，其实就是 PageCache ，它缓存了磁盘文件数据，从而提高 IO 的吞吐量。

当然，目前 BufferCache 仍然是存在的，因为还存在需要执行的块 IO。因为大多数块都是用来存储文件数据，所以大部分 BufferCache 都指向了 PageCache；但还是有一小部分块并不是文件数据，例如元数据、RawBlock IO，此时还需要通过 BufferCache 来缓存。

明白了这两套缓存系统的区别，就可以理解它们究竟都可以用来做什么了。

### Page Cache

主要用来作为文件系统上的文件数据的缓存，常见的是针对文件的 `read()/write()` 操作，另外也包括了通过 `mmap()` 映射之后的块设备，也就是说，事实上 Page Cache 负责了大部分的块设备文件的缓存工作。

### Buffer Cache

BufferCache 用来在系统对块设备进行读写的时候，对块进行数据缓存的系统来使用，例如格式化文件系统时。

<!--
一般情况下两个缓存系统是一起配合使用的，比如当我们对一个文件进行写操作的时候，page cache的内容会被改变，而buffer cache则可以用来将page标记为不同的缓冲区，并记录是哪一个缓冲区被修改了。这样，内核在后续执行脏数据的回写（writeback）时，就不用将整个page写回，而只需要写回修改的部分即可。
-->

### 总结

磁盘有逻辑 (文件系统) 和物理 (磁盘块) 两种操作，分别对应了 Page Cache 和 Buffer Cache 。简单来说，如果直接通过 `read()/write()` 等直接去操作文件，那使用的就是 Page Cache 缓存，而使用 dd 等命令直接操作磁盘块，就是 Buffer Cache 缓存。

注意，块 Block 的大小由所使用块设备决定，而页在 x86 上无论是 32 位还是 64 位都是 4K 。

## 测试

那么 free 命令中的 buffers 和 cache 是什么意思？

该命令读取的是 ```/proc/meminfo``` 文件中的数据，可以从是否有 available 判断是否为最新版本，对于老版本计算方式如下：

{% highlight text %}
cache = Cached + SwapCached + SReclaimable;
available = MemFree + Buffers + cache
{% endhighlight %}

### free

新版的 free 命令输出如下。

{% highlight text %}
$ free -wm
              total        used        free      shared     buffers       cache   available
Mem:           7881        3109         797        1113         309        3665        3310
Swap:          7936         260        7676
{% endhighlight %}

* buffers，表示块设备 (block device) 所占用的缓存页，包括了直接读写块设备以及文件系统元数据 (metadata) 比如 SuperBlock 所使用的缓存页；
* cached，表示普通文件系统中数据所占用的缓存页。

如上所述，该命令读取的是 ```/proc/meminfo``` 文件中的 Buffers 和 Cached 数据，而在内核中的实现实际上对应了 ```meminfo_proc_show()@fs/proc/meminfo.c``` 函数，内容如下。

{% highlight c %}
static int meminfo_proc_show(struct seq_file *m, void *v)
{
    si_meminfo(&i);  // 通过nr_blockdev_pages()函数填充bufferram
    si_swapinfo(&i);

    cached = global_page_state(NR_FILE_PAGES) -
            total_swapcache_pages() - i.bufferram;
    if (cached < 0)
        cached = 0;

    seq_printf(m,
        "Buffers:        %8lu kB\n"
        "Cached:         %8lu kB\n"
        // ... ..
        K(i.bufferram),
        K(cached),
        // ... ..
}
{% endhighlight %}

如上计算 cached 公式中，```global_page_state(NR_FILE_PAGES)``` 实际读取的 ```vmstat[NR_FILE_PAGES]```，也就是用于统计所有缓存页 (page cache) 的总和，它包括：

<!--
mm/filemap.c:200:       __dec_zone_page_state(page, NR_FILE_PAGES);
mm/filemap.c:488:               __inc_zone_page_state(new, NR_FILE_PAGES);
mm/filemap.c:580:       __inc_zone_page_state(page, NR_FILE_PAGES);
mm/swap_state.c:105:            __inc_zone_page_state(page, NR_FILE_PAGES);
mm/swap_state.c:157:    __dec_zone_page_state(page, NR_FILE_PAGES);
mm/shmem.c:318:         __inc_zone_page_state(page, NR_FILE_PAGES);
mm/shmem.c:341: __dec_zone_page_state(page, NR_FILE_PAGES);
mm/shmem.c:1003:                __inc_zone_page_state(newpage, NR_FILE_PAGES);
mm/shmem.c:1004:                __dec_zone_page_state(oldpage, NR_FILE_PAGES);
mm/migrate.c:414:       __dec_zone_page_state(page, NR_FILE_PAGES);
mm/migrate.c:415:       __inc_zone_page_state(newpage, NR_FILE_PAGES);

    Cached
    buffers
    交换区缓存(swap cache)
-->

> swap cache 主要是针对匿名内存页，例如用户进程通过 malloc() 申请的内存页，当要发生 swapping 换页时，如果一个匿名页要被换出时，会先计入到 swap cache，但是不会立刻写入物理交换区，因为 Linux 的原则是除非绝对必要，尽量避免 IO。
>
> 所以 swap cache 中包含的是被确定要 swapping 换页，但是尚未写入物理交换区的匿名内存页。

<!--
vmstat[NR_FILE_PAGES] 可以通过 /proc/vmstat 来查看，表示所有缓存页的总数量：
# cat /proc/vmstat
...
nr_file_pages 587334
...


注意以上nr_file_pages是以page为单位，而不像free命令是以KB为单位，一个page等于4KB。

    直接修改 nr_file_pages 的内核函数是：
    __inc_zone_page_state(page, NR_FILE_PAGES) 和
    __dec_zone_page_state(page, NR_FILE_PAGES)，
    一个用于增加，一个用于减少。


先看”cached”：

“Cached” 就是除去 “buffers” 和 “swap cache” 之外的缓存页的数量：
global_page_state(NR_FILE_PAGES) – total_swapcache_pages – i.bufferram
所以关键还是要理解 “buffers” 是什么含义。


#### buffers

从源码中可以看到 buffers 来自于 nr_blockdev_pages() 的返回值，该函数如下：

{% highlight c %}
long nr_blockdev_pages(void)
{
    struct block_device *bdev;
    long ret = 0;
    spin_lock(&bdev_lock);
    list_for_each_entry(bdev, &all_bdevs, bd_list) {
        ret += bdev->bd_inode->i_mapping->nrpages;
    }
    spin_unlock(&bdev_lock);
    return ret;
}
{% endhighlight %}

这段代码就是遍历所有的块设备 (block device)，累加每个块设备的 inode 的 i_mapping 的页数，统计得到的就是 buffers 。所以很明显，buffers 是与块设备直接相关的。

那么谁会更新块设备的缓存页数量(nrpages)呢？我们继续向下看。

搜索kernel源代码发现，最终更新mapping->nrpages字段的函数就是add_to_page_cache和__remove_from_page_cache：

static inline int add_to_page_cache(struct page *page,
                struct address_space *mapping, pgoff_t offset, gfp_t gfp_mask)
{
        int error;

        __set_page_locked(page);
        error = add_to_page_cache_locked(page, mapping, offset, gfp_mask);
        if (unlikely(error))
                __clear_page_locked(page);
        return error;
}

void remove_from_page_cache(struct page *page)
{
        struct address_space *mapping = page->mapping;
        void (*freepage)(struct page *) = NULL;
        struct inode *inode = mapping->host;

        BUG_ON(!PageLocked(page));

        if (IS_AOP_EXT(inode))
                freepage = EXT_AOPS(mapping->a_ops)->freepage;

        spin_lock_irq(&mapping->tree_lock);
        __remove_from_page_cache(page);
        spin_unlock_irq(&mapping->tree_lock);
        mem_cgroup_uncharge_cache_page(page);

        if (freepage)
                freepage(page);
}

page_cache_tree_insert()

__delete_from_page_cache()
 |-page_cache_tree_delete() --

这两个函数是通用的，block device 和 文件inode 都可以调用，至于更新的是块设备的(buffers)还是文件的(cached)，取决于调用参数变量mapping：如果mapping对应的是文件inode，自然就不会影响到 “buffers”；如果mapping对应的是块设备，那么相应的统计信息会反映在 “buffers” 中。我们下面看看kernel中哪些地方会把块设备的mapping传递进来。

搜索内核源代码发现，ext4_readdir 函数调用 page_cache_sync_readahead 时传递的参数是 sb->s_bdev->bd_inode->i_mapping，其中s_bdev就是块设备，也就是说在读目录(ext4_readdir)的时候可能会增加 “buffers” 的值：
static int ext4_readdir(struct file *filp,
                         void *dirent, filldir_t filldir)
{

...
        struct super_block *sb = inode->i_sb;
...
                        if (!ra_has_index(&filp->f_ra, index))
                                page_cache_sync_readahead(
                                        sb->s_bdev->bd_inode->i_mapping,
                                        &filp->f_ra, filp,
                                        index, 1);
...
}
1
2
3
4
5
6
7
8
9
10
11
12
13
14

static int ext4_readdir(struct file *filp,
                         void *dirent, filldir_t filldir)
{

...
        struct super_block *sb = inode->i_sb;
...
                        if (!ra_has_index(&filp->f_ra, index))
                                page_cache_sync_readahead(
                                        sb->s_bdev->bd_inode->i_mapping,
                                        &filp->f_ra, filp,
                                        index, 1);
...
}

继续琢磨上面的代码，sb表示SuperBlock，属于文件系统的metadata（元数据），突然间一切恍然大悟：因为metadata不属于文件，没有对应的inode，所以，对metadata操作所涉及的缓存页都只能利用块设备mapping，算入 buffers 的统计值内。

    打个岔：ext4_readdir() 中调用 page_cache_sync_readahead() 显然是在进行预读(read-ahead)，为什么read-ahead没有使用普通文件inode的mapping，而是使用了底层的块设备呢？从记载在补丁中的说明来看，这是一个权宜之计，看这里，所以不必深究了。

举一反三，如果文件含有间接块(indirect blocks)，因为间接块属于metadata，所以走的也是块设备的mapping。查看源代码，果然如此：
ext4_get_blocks
->  ext4_ind_get_blocks
    ->  ext4_get_branch
        ->  sb_getblk

static inline struct buffer_head *
sb_getblk(struct super_block *sb, sector_t block)
{
        return __getblk(sb->s_bdev, block, sb->s_blocksize);
}
1
2
3
4
5
6
7
8
9
10

ext4_get_blocks
->  ext4_ind_get_blocks
    ->  ext4_get_branch
        ->  sb_getblk

static inline struct buffer_head *
sb_getblk(struct super_block *sb, sector_t block)
{
        return __getblk(sb->s_bdev, block, sb->s_blocksize);
}

这样，我们就知道了，”buffers” 是块设备(block device)占用的缓存页，分为两种情况：

    直接对块设备进行读写操作；
    文件系统的metadata（元数据），比如 SuperBlock。
-->

### 验证

如上，读取 EXT4 文件系统的目录会使用到 "buffers"，这里使用 find 命令扫描文件系统，观察 "buffers" 增加的情况：

{% highlight text %}
# sync
# echo 3 > /proc/sys/vm/drop_caches
$ free -wk; find ~ -name "not exits file" >/dev/null 2>&1; free -wk
              total        used        free      shared     buffers       cache   available
Mem:        8070604     3260408     3445852     1102588        5236     1359108     3418844
Swap:       8127484      300172     7827312
              total        used        free      shared     buffers       cache   available
Mem:        8070604     3249764     3207336     1087716      250484     1363020     3417764
Swap:       8127484      300172     7827312
{% endhighlight %}

再测试下直接读取 block device 并观察 "buffers" 增加的现象：

{% highlight text %}
# sync
# echo 3 > /proc/sys/vm/drop_caches
# free -wk; dd if=/dev/sda1 of=/dev/null count=200M; free -wk
              total        used        free      shared     buffers       cache   available
Mem:        8070604     3244516     3486124     1094648         932     1339032     3451048
Swap:       8127484      300172     7827312
532480+0 records in
532480+0 records out
272629760 bytes (273 MB) copied, 0.612241 s, 445 MB/s
              total        used        free      shared     buffers       cache   available
Mem:        8070604     3245032     3218528     1094868      267196     1339848     3427012
Swap:       8127484      300172     7827312
{% endhighlight %}

<!--
 结论：

free 命令所显示的 “buffers” 表示块设备(block device)所占用的缓存页，包括直接读写块设备、以及文件系统元数据(metadata)如SuperBlock所使用的缓存页；
而 “cached” 表示普通文件所占用的缓存页。
-->

## mincore

在内核中有个 `man mincore(2)` 的系统调用，其实现在 `mm/mincore.c` 中，主要用来判断页面的状态，可以使用 `fincore` 工具查看。

{% highlight text %}
mincore -- determine whether pages are resident in memory
{% endhighlight %}

最早是由 google 开发，不过已经是七八年前的事情了，现在几乎不再维护，可以从 github 上查找相关的代码。

去掉了异常处理之外，其处理过程大致如下。

{% highlight text %}
fd = open(fname, O_RDONLY);
if (fstat(fd, &stat) < 0) {
file_pages = (stat.st_size + page_size - 1) / page_size;
vec = malloc(file_pages);
fmap = mmap(NULL, stat.st_size, PROT_NONE, MAP_SHARED, fd, 0);
if (mincore(fmap, stat.st_size, vec) != 0 ) {
if (vec[i] & 1) {
{% endhighlight %}

`open()` 获得文件描述符，`fstat()` 获取文件的长度，页面的大小可以通过系统调用获取，一般是 4K ，有了文件大小，就知道了，需要多少个 `int` 来存放结果。

`mmap()` 建立映射关系，`mincore()` 获取文件页面的驻留情况，从起始地址开始，长度是 filesize，结果保存在 `vec` 数组里，如果 `vec[i] & 1 == 1` 那么表示该页面驻留在内存中，否则没有对应缓存。

另外，一个不错的工具可以参考 [vmtouch](https://github.com/hoytech/vmtouch/) 。

## 回收 Cache

内核在内存将要耗尽时，会触发内存回收的工作，一般来说主要释放的是 Buffer/Cache 的内存，但是这种清缓存的操作也并不是没有成本。

理解 Buffer Cache 的作用，那么如果要清理缓存，那么必须要保证数据的一致性，所以一般在清理的时候同时会伴随这 IO 彪高。因为内核要对比内存中的数据和对应硬盘文件上的数据是否一致，如果不一致需要写回，之后才能回收。

### 手动触发

在系统中除了内存将被耗尽的时候可以清缓存以外，还可以使用下面这个文件来人工触发缓存清除的操作。

{% highlight text %}
# echo 1 > /proc/sys/vm/drop_caches
{% endhighlight %}

其中的取值可以是 `1` `2` `3`，代表的含义为：

* `1` 清除 PageCache；
* `2` 回收 slab 分配器中的对象 (包括目录项缓存和 inode 缓存)，slab 是内核中管理内存的一种机制，其中很多缓存数据实现都是用的 PageCache；
* `3` 清除 PageCache 和 slab 分配器中的缓存对象。

这部分内核代码位于 `fs/drop_caches.c` 里面。

### fadvise

除了上述粗暴的方法外，Linux 还提供了 `posix_fadvise()` 系统调用，允许用户给 Linux 提建议。

{% highlight c %}
#include <fcntl.h>

int posix_fadvise(int fd, off_t offset, off_t len, int advice);
{% endhighlight %}

其中比较常用的有两个选项，可以通过 `posix_fadvise(2)` 查看，信息摘抄如下。

{% highlight text %}
POSIX_FADV_WILLNEED
	The specified data will be accessed in the near future.

POSIX_FADV_DONTNEED
	The specified data will not be accessed in the near future.
{% endhighlight %}

其中，其含义如下：

* `POSIX_FADV_WILLNEED` 相当于说，这个文件在不久的将来要用，请准备好相应的页面(从磁盘读入内存)，相当于预读。
* `POSIX_FADV_DONTNEED` 相当于告知 Linux ，这个文件不用了，直接回收掉吧，类似于 sync 操作。

在 PostgreSQL 中有如下的应用：

{% highlight c %}
int FilePrefetch(File file, off_t offset, int amount)
{
#if defined(USE_POSIX_FADVISE) && defined(POSIX_FADV_WILLNEED)
	int returnCode;

	Assert(FileIsValid(file));

        DO_DB(elog(LOG, "FilePrefetch: %d (%s) " INT64_FORMAT " %d",
                 file, VfdCache[file].fileName,
                 (int64) offset, amount));

        returnCode = FileAccess(file);
        if (returnCode < 0)
            return returnCode;

        returnCode = posix_fadvise(VfdCache[file].fd, offset, amount,
                                 POSIX_FADV_WILLNEED); //预读

        return returnCode;
#else
        Assert(FileIsValid(file));
        return 0;
#endif
}

int pg_flush_data(int fd, off_t offset, off_t amount)
{
#if defined(USE_POSIX_FADVISE) && defined(POSIX_FADV_DONTNEED)
        return posix_fadvise(fd, offset, amount, POSIX_FADV_DONTNEED);
#else
        return 0;
#endif
}
{% endhighlight %}

通过 `fadvise` 就能把某文件彻底赶出缓存，代码非常简单。

{% highlight c %}
int clear_file_cache(const char *filename)
{
	struct stat st;

	if(stat(filename , &st) < 0) {
		fprintf(stderr , "stat localfile failed, path:%s\n",filename);
		return -1;
	}

	int fd = open(filename, O_RDONLY);
	if( fd < 0 ) {
		fprintf(stderr , "open localfile failed, path:%s\n",filename);
		return -1;
	}

	//clear cache by posix_fadvise

	if( posix_fadvise(fd,0,st.st_size,POSIX_FADV_DONTNEED) != 0) {
		printf("Cache FADV_DONTNEED failed, %s\n",strerror(errno));
	} else {
		printf("Cache FADV_DONTNEED done\n");
	}

	return 0;
}
{% endhighlight %}

其中 `vmtouch -e` 以及 `linux-ftools` 中的 `linux-fadvise` 提供了类似的功能，实际最终调用的都是 `posix_fadvise()` 接口。

关于 `posix_fadvise()` 接口的内核实现，可以参考霸爷的 [posix_fadvise 清除缓存的误解和改进措施](http://blog.yufeng.info/archives/1917) 。

## 其它

### stap

可以通过 stap 脚本查看是谁在消耗 Cache，不过配置起来比较麻烦，暂不介绍了，后面补充吧。

<!--
Linux下谁在消耗我们的cache
http://blog.yufeng.info/archives/tag/mincore

## 总结

这里面的水比较深，很多地方可以扩展开来，比如缓存到什么程度，操作系统开始出面清理缓存，在比如posix_fadvise的kernel实现，在比如介绍mincore系统调用的时候，我们发现，多个系统调用组合才能得到文件的缓存信息，这太慢了，Chris Frost提出了一个新的系统调用fincore，感兴趣的可以查看http://libprefetch.cs.ucla.edu/及https://lkml.org/lkml/2013/2/15/44。 另外，低于mincore系统调用，只返回是否在文件对应对页是否存在在缓存中，这太浪费了，明明可以把是否dirty一并返回，我今天一直纠结与如何返回文件页面在缓存的dirty情况，很蛋疼，没解决。实际上mincore完全可以顺路返回这个值。毕竟int有32bit，只用一个bit太浪费了。不能展的太开，否则，就收敛不了了，另外，我的功能还不到。

file和page cache 的一些事儿
http://blog.chinaunix.net/uid-24774106-id-3625000.html
-->

## 参考

一个不错的工具 linux-ftools，可以直接从 [本地下载](/reference/linux/monitor/linux-ftools.tar.bz2) 。


{% highlight text %}
{% endhighlight %}
