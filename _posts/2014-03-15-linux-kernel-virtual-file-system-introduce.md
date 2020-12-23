---
title: Linux VFS 文件系统
layout: post
comments: true
language: chinese
category: [linux]
keywords: linux,lvs
description: 在次重申下，*nix 的设计理念是：一切都是文件！ 也就是在 Linux 中，一切设备皆是以文件的形式进行操作，如网络套接字、硬件设备等。这一切都是通过一个中间层实现的，被称为 VFS (Virtual File System) 。
---

在次重申下，*nix 的设计理念是：一切都是文件！

也就是在 Linux 中，一切设备皆是以文件的形式进行操作，如网络套接字、硬件设备等。这一切都是通过一个中间层实现的，被称为 VFS (Virtual File System) 。

<!-- more -->

## Virtual File System, VFS

总体上说 Linux 的文件系统主要可分为三大块：一是上层的文件系统的系统调用，也就是提供的统一文件系统 API；二是虚拟文件系统 VFS；三是挂载到 VFS 中的各实际文件系统，例如 ext4 。

按照类型文件系统可以分为三类：A) 磁盘文件系统，最常见，用来将数据保存在物理存储上，如 ext4、FAT、NTFS；B) 虚拟文件系统，如 procfs、sysfs；C) 网络文件系统，NFS 。

为了支持多种文件系统，Linux 内核在用户进程 (C标准库) 和文件系统之前实现了一个抽象层，虚拟文件系统，也就是 VFS 。

VFS 除了为所有文件系统的实现提供一个通用接口外，还提供了一些文件相关数据结构的磁盘高速缓存。例如最近最常使用的目录项对象被放在所谓目录项高速缓存（dentry cache）的磁盘高速缓存中，从而加速从文件路径名到最后一个路径分量的索引节点的转换过程。

### 数据结构

对于文件，主要包括了两部分信息：A) 存储的数据本身；B) 该文件的组织和管理的信息。

后者就是 Linux 中维护的一些元数据 (metadata)，主要结构包括了 superblock、inode、dentry 和 file，用来支持如指示存储位置、历史数据、资源查找、文件纪录等功能。

在内存中, 每个文件都有一个 dentry(目录项) 和 inode (索引节点) 结构，dentry 记录着文件名，上级目录等信息，正是它形成了我们所看到的树状结构；而有关该文件的组织和管理的信息主要存放 inode 里面，它记录着文件在存储介质上的位置与分布。

另外，`dentry->d_inode` 指向相应的 inode 结构，dentry 与 inode 是多对一的关系，因为有可能一个文件有好几个文件名，如硬链接。

### dentry 和 inode 的关系

在 Linux 进程中，是通过目录项 (dentry) 和索引节点 (inode) 描述文件的，而所谓 "文件" 就是按一定的格式存储在介质上的信息，所以一个文件其实包含了两方面的信息，一是存储的数据本身，二是有关该文件的组织和管理的信息。

在内存中, 每个文件都有一个 dentry 和 inode 结构，前者记录着文件名、上级目录等信息，所有的 dentry 用 d_parent 和 d_child连 接起来，就形成了我们熟悉的树状结构； 而有关该文件的组织和管理的信息主要存放 inode 里面，它记录着文件在存储介质上的位置与分布。

同时 dentry->d_inode 指向相应的 inode 结构，由于硬链接导致一个文件可能有好几个文件名，所以 dentry 与 inode 是多对一的关系。

inode 代表的是物理意义上的文件，通过 inode 可以得到一个数组，这个数组记录了文件内容的位置，如该文件位于硬盘的第 3、8、10 块，那么这个数组的内容就是 3、8、10。在同一个文件系统中可以通过索引节点号 inode->i_ino 计算出在介质上的位置，对于硬盘来说，可直接计算出对应的 inode 属于哪个块 (block)，从而找到相应的 inode 结构。

另外，对于某一种特定的文件系统而言，如 ext4，在内存中用 ext4_inode_info 结构体描述，包含了一个 inode 容器。就磁盘文件而言，dentry 和 inode 的信息保存在磁盘上，对于像 ext4 这样的磁盘文件来说，存储介质中的目录项和索引节点载体通过 ext4_inode、ext4_dir_entry_2 标示。













## 文件系统

磁盘上的文件内容通过文件系统组织一系列的文件，通常这些文件保存在磁盘的不同分区上，当然不同的分区可能包含不同的文件系统类型，如 ext2、ext3、fat16、ntfs 等。

可以通过 `cat /proc/filesystems` 查看已经注册的文件系统。

对于每个文件系统，同时会有代码，或者是模块，来告诉我们如何操作这些文件。因此，在使用具体的文件之前，需要告诉内核该文件系统的相关信息，主要包括 A) 文件系统名称；B) 知道如何挂载；C) 如何查找文件的路径；D) 如何查找文件的内容。

### 数据结构


#### file system

其中 `struct file_system_type` 包括了文件系统的主要参数，如下之列出了主要部分。

{% highlight c %}
struct file_system_type {
    const char *name;                                     // 文件系统的名称，如EXT4、FAT16、NTFS
    int fs_flags;                                         // 对应文件系统的类型，在该变量下定义了一些符号的宏
    struct dentry *(*mount) (struct file_system_type *,   // 代替早期的get_sb()，用户挂载此文件系统时使用的回调函数
                int, const char *, void *);
    void (*kill_sb) (struct super_block *);               // 删除内存中的super block，在卸载文件系统时使用
    struct module *owner;                                 // 指向实现这个文件系统的模块，通常为THIS_MODULE宏
    struct file_system_type *next;                        // 指向文件系统类型链表的下一个文件系统类型
    struct hlist_head fs_supers;                          // 该文件系统类型的超级块结构，都串连在这个表头下
};
{% endhighlight %}

另外，存在一个全局变量 `file_systems`，用于保存所有已经注册的文件系统。

{% highlight c %}
static struct file_system_type *file_systems;
static DEFINE_RWLOCK(file_systems_lock);
{% endhighlight %}

文件系统信息在内核中会通过单向链表保存，其中 `file_systems` 全局变量作为链头，同时对应一个 `file_systems_lock` 锁，当需要读写时需要先加锁，如上，可以通过 `cat /proc/filesystems` 查看已经注册的文件系统。

#### operation

与文件相关的操作包括了三部分：

* SuperBlock 包含了文件系统的元数据信息；
* Inode 与文件相关的信息；
* File 保存的文件主体。

针对这三部分同时也对应了三类的操作 API 接口函数。

{% highlight c %}
struct super_operations {
	struct inode *(*alloc_inode)(struct super_block *);
	void (*destroy_inode)(struct inode *);

	int (*sync_fs)(struct super_block *sb, int wait);
	int (*statfs) (struct dentry *, struct kstatfs *);
	// ... ...
};

struct inode_operations {
	struct dentry * (*lookup) (struct inode *,struct dentry *, unsigned int);
	int (*create) (struct inode *,struct dentry *, umode_t, bool);
	int (*link) (struct dentry *,struct inode *,struct dentry *);
	int (*mkdir) (struct inode *,struct dentry *,umode_t);
	// ... ...
};

struct file_operations {
	loff_t (*llseek) (struct file *, loff_t, int);
	ssize_t (*read) (struct file *, char __user *, size_t, loff_t *);
	ssize_t (*write) (struct file *, const char __user *, size_t, loff_t *);
	int (*open) (struct inode *, struct file *);
	// ... ...
};
{% endhighlight %}

## 文件系统注册

这是最简单的，也就是直接添加到全局列表中。

在上面的结构体中包含了文件系统的名称，以及如何产生一个保存在内存中的 `super_block` 结构体。主要，通过 `register_filesystem()` 向 VFS 注册，对于编译到内核中的文件系统则是在内核初始化的时候注册，当然也可以在模块初始化的时候注册。

文件系统注册通过 `int register_filesystem(struct file_system_type * fs)` 来完成，该函数唯一的操作是将相应的结构体添加到 `file_systems` 链表中。

在 `struct file_system_type` 中通过链表与 `file_systems` 链头相连，而 `register_filesystem()` 也就是将对应的类型添加到链表中。

然后在通过 mount 命令挂载时，会指定相应的文件系统，然后通过对应的 `mount()` 函数从文件系统 (extN 是从硬盘上) 读取 `super_block` 并初始化。

新建 inode 实际通过 `new_inode()` 完成，而该函数最终会调用 `super_block->s_op->alloc_inode()` 完成。

## Mount 挂载

挂载就是将一个文件系统添加到一个目录上，对应的文件系统通常为磁盘文件系统，如 EXTN、NTFS、XFS、FAT16 等；当然也包括虚拟文件，如 proc、sysfs 等。

挂载时通常包括 A) 一个设备，可以是磁盘、软盘、CDROM 、U盘等；B) 一个对应的目录挂载点；C) 指定相应的文件系统。

一个挂载命令 mount 通常如下，包括了几个重要参数 fstype、devname、mountpoint、options，可以通过 `man 8 mount` 查看，对于函数的原型可以通过 `man 2 mount` 查看。

{% highlight text %}
# mount -t ext4 /dev/sda1 /mnt -o ....
{% endhighlight %}

对于上述的命令也可以通过如下的程序执行。
{% highlight text %}
$ man 2 mount
... ...
int mount(const char *source, const char *target,
             const char *filesystemtype, unsigned long mountflags,
             const void *data);
... ...

$ cat mount_test.c
#include <stdio.h>
#include <sys/mount.h>

int main(int argc, char *argv[]) {
    if (mount("/dev/sda1", "/mnt", "ext4", 0, NULL)) {
        perror("mount failed");
    }
    return 0;
}

$ gcc -Wall -o mount_test mount_test.c                 # 编译
$ ./mount_test                                         # 执行命令挂载
$ findmnt /dev/sda1                                    # 查看执行结果
{% endhighlight %}

对于 `mount()` 函数，source 是要挂载的设备名，target 是要挂载到哪，filesystemtype 就是文件系统类型名，而剩余的两个参数 flags 和 data 对应于传入的参数。

其中 flags 相应宏定义在 `include/uapi/linux/fs.h` 中，如 `MS_RDONLY`、`MS_NOATIME` 等，这些 flags 会在 VFS 层被解析使用。而 data 则是每个文件系统各自支持的挂载选项，可以通过 strace 查看最终调用 `mount()` 接口是调用的命令。

{% highlight text %}
$ strace mount /dev/loop0 /mnt/foobar -o noquota,nodev
... ...
mount("/dev/loop0", "/mnt/foobar", "xfs", MS_MGC_VAL|MS_NODEV, "noquota") = 0
... ...
{% endhighlight %}

其中 nodev 被解释为 flag，noquota 被当作了 mount data。

### 挂载过程

在内核中，`struct mount` 代表着一个 mount 实例，每次挂载都会新建一个该结构体，其中 `struct vfsmount mnt` 成员是它最核心的部分，过去所有的成员都保存在 vfsmount 结构体中，后来只保留了核心部分在 vfsmount ，这样使得 vfsmount 的内容更加精简，在很多情况下只需要传递 vfsmount。

{% highlight c %}
// fs/mount.h
struct mount {
    struct hlist_node mnt_hash;
    struct mount *mnt_parent;
    struct vfsmount mnt;
    ... ...
};

// include/linux/mount.h
struct vfsmount {
    struct dentry  *mnt_root;           // 该文件系统的根目录
    struct super_block *mnt_sb;         // 指向superblock
    int mnt_flags;
};

// include/linux/path.h
struct path {
    struct vfsmount *mnt;
    struct dentry *dentry;
};
{% endhighlight %}

在全局文件系统树上要想确定一个位置不能由 dentry 唯一确定，因为有了挂载关系，一切都变的复杂了，比如一个文件系统可以挂装载到不同的挂载点。所以文件系统树的一个位置要由 `(mount, dentry)` 二元组，或者说 `(vfsmount, dentry)` 来确定，在内核中通过 `struct path` 表示。

下面查看 `sys_mount()` 的执行过程，实际上，mount 操作的过程就是新建一个 `struct mount`，然后将此结构和挂载点关联。之后，目录查找时就能沿着 mount 挂载点一级级向下查找文件。

{% highlight text %}
sys_mount()
 |-copy_mount_string()                  # 从用户空间复制文件系统类型名称、设备名称
 |-copy_mount_options()                 # 获取data数据
 |-do_mount()                           # 通过该函数调用已传入内核的参数
   |-user_path()                        # 把挂载点解析成path内核结构，也就是路径解析过程
   | |-user_path_at()
   |   |-filename_lookup()
   |- ... ...                           # 解析flags确定mount的操作类型，如bind、remount、newmount
   |-do_remount()                       # 重新挂载等操作，下面以挂载新节点为例
   |-do_new_mount()
     |-get_fs_type()                    # 通过文件系统名称，找到对应文件系统的类型
     |-vfs_kern_mount()                 # 通过该函数调用具体文件系统的处理函数，构建一个vfsmnt结构
     | |-alloc_vfsmnt()                 # 分配新的struct mount结构体，并初始化其中的一部分，
     | |                                # 构造一个root dentry，包含特定文件系统的super block信息
     | |-mount_fs()                     # 调用具体文件系统的mount回调函数
     | |  |-type->mount()               # 调用特定文件系统的回调函数
     | |- ... ...                       # 完成最后struct mount的初始化
     |
     |-do_add_mount()                   # 将得到的mnt结构添加到全局文件系统
       |-lock_mount()                   # 找到最新的挂载点，并加锁
       |-real_mount()                   # 挂载到对应的mount点，对挂载进行检查
       |-graft_tree()                   # 将newmount添加到全局文件系统中
{% endhighlight %}

在调用 `vfs_kern_mount()` 时，只有文件系统类型、挂载标记、设备名和挂载选项信息为参数，并没有 mountpoint 参数，其时这里只是用 type 中的 mount 回调函数读取设备的 superblock 信息，填充 mnt 结构，然后把 flag 和 data 解析后填充到 mnt 结构中。

就是说，通过 `vfs_kern_mount()` 会调用具体文件系统的 `mount()` 函数，生成 `struct mount`，最后通过 `do_add_mount()` 添加到全局的文件系统中。

## 系统调用

对于文件的常见操作如下：

{% highlight c %}
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h

int main(void)
{
	int fd, ret;

	fd = open("syscall.txt", O_RDWR | O_CREAT, 0644);   // man 2 open
	if(fd == -1) {
		printf("Error try to open 'syscall.txt'");
		exit(EXIT_FAILURE);
	}

	ret = write(fd, "just for test\n", 14);
	if(ret == -1) {
		printf("Error try to write 'syscall.txt'");
		close(fd);
		exit(EXIT_FAILURE);
	}
	close(fd);

	return 0;
}
{% endhighlight %}

在 Linux 内核中，每个打开的文件均由一个文件描述符 `struct file` 表示，而返回给用户的是在 `fd_array[]` 中的位置索引。

也即，该描述符在特定进程的数组中充当位置索引，这也意味着包括 `open()` 数组是 `task_struct -> files -> fd_arry`，该数组的元素包含了 file 结构，其中包括每个打开文件的所有必要信息。

### sys_open

对于 open() 系统调用的的主要执行操作在 do_sys_open() 中，下面介绍其主要操作。

do_sys_open() 的操作大致为：A) 找到一个本进程没有使用的文件描述符 fd；B) 分配一个全新的 struct file 结构体；C) 根据传人的 pathname 查找或建立对应的 dentry；D) 建立 fd 到这个 struct file 结构体的联系。

{% highlight text %}
sys_open()
 |-force_o_largefile()               # 是否要强制使用大文件
 |-do_sys_open()                     # 实际工作，返回索引
   |-build_open_flags()              # 检测传入的flag合法性，并转换为内核的格式struct open_flags
   |-getname()                       # 1. 将用户空间的路径名复制到内核空间，此时申请了内核内存
   | |-getname_flags()               #    主要的处理函数
   |-get_unused_fd_flags()           # 2. 查找一个未使用的文件描述符
   |-do_filp_open()                  # 3. 完成路径搜索并打开文件
   | |-path_openat()                 #    实际返回struct file结构体
   |   |-get_empty_filep()
   |   |-path_init()
   |   |-link_path_walk()
   |   |-do_last()                   #    打开文件的最后一步
   |     |-may_open()                #    进行权限检查
   |     |-vfs_open()
   |     | |-do_dentry_open()        ### 真正的针对文件系统的操作
   |     |-open_check_o_direct()     # 通过f->f_flags & O_DIRECT判断
   |-fsnotify_open()                 # 4. 通过fsnotify机制唤醒文件系统中的监控进程
   |-fd_install()                    # 5. 为该文件描述符安装文件，设置current->files->fd[fd]=file
   |-putname()                       # 6. 释放之前申请的内核内存
{% endhighlight %}

在开始，sys_open() 会检测是否要强制支持大文件，在 64 位系统上 flags 会自动加上 O_LARGEFILE ，对于 32 位系统，文件最大受索引节点中表示文件大小的 32-bit 的 i_size 的影响，只能访问 2^32 字节，即 4GB ，而实际高位一般不用，所以通常只有 2G 。

加上 O_LAGEFILE 之后启用索引节点的 i_dir_acl 字段也可以一起表示文件的大小了，这样位数就变成了 64 位，也就是单文件最大为 2^64=16TB 。

do_last() 是文件打开操作的最后一步，其中很大一部分是针对 flag 的判断操作，关于 flags 的含义可以参考 man 2 open，该函数最终会调用 vfs_open() 函数。

{% highlight c %}
int vfs_open(const struct path *path, struct file *filp,
         const struct cred *cred)
{
    struct inode *inode = path->dentry->d_inode;

    if (inode->i_op->dentry_open)
        return inode->i_op->dentry_open(path->dentry, filp, cred);
    else {
        filp->f_path = *path;
        return do_dentry_open(filp, NULL, cred);
    }
}

struct inode {
    ... ...
    const struct inode_operations   *i_op;
    ... ...
};
{% endhighlight %}

对于 ext4，可以通过 cd fs/ext4 && grep -rne '^const.\*ext4.\*inode_operations' 查看，而 dentry_open() 指针并不存在，实际上会调用 do_dentry_open() 函数。

在 do_dentry_open() 中，会分配一个全新的 struct file 结构体，打开时会根据路径判断所属的文件系统，并执行具体类型的 open() 操作。

{% highlight c %}
static int do_dentry_open(struct file *f,
              int (*open)(struct inode *, struct file *),
              const struct cred *cred)
{
    ... ...
    path_get(&f->f_path);
    inode = f->f_inode = f->f_path.dentry->d_inode;
    f->f_mapping = inode->i_mapping;
    ... ...
    f->f_op = fops_get(inode->i_fop);                 // 后续的操作都使用该行指定的操作
    if (unlikely(WARN_ON(!f->f_op))) {
        error = -ENODEV;
        goto cleanup_all;
    }

    error = security_file_open(f, cred);
    if (error)
        goto cleanup_all;

    error = break_lease(inode, f->f_flags);
    if (error)
        goto cleanup_all;

    if (!open)
        open = f->f_op->open;
    if (open) {
        error = open(inode, f);
        if (error)
            goto cleanup_all;
    ... ...
}

struct dentry {
    ... ...
    struct inode *d_inode;      /* Where the name belongs to - NULL is negative */
    ... ...
};
struct path {
    struct vfsmount *mnt;
    struct dentry *dentry;
};
struct file {
    ... ...
    struct path     f_path;
    const struct file_operations    *f_op;
    ... ...
};
{% endhighlight %}

对于后续的 read()、write() 操作，实际调用的是 struct file_operations 指定的接口，如上述的 open() 函数接口，也就是 inode->i_fop 指定的值。


另外，可以参考 [Linux 系统调用 open 七日游](http://blog.chinaunix.net/uid-20522771-id-4419666.html) 相当不错的介绍文件系统打开的过程。

### sys_read

sys_read() 会根据用户空间传入的文件描述符 fd 取出对应的 struct file 结构体，获取 struct file 结构体的当前偏移量指针，从文件读取内容，存放到用户空间内存区，如果读取成功，唤醒相关等待进程，更新文件的当前指针，如果需要则释放对 file 结构的引用。

{% highlight text %}
sys_read()
 |-fdget_pos()                        # 1. 通过fd获取struct file结构
 |-file_pos_read()                    # 2. 获取当前的偏移量，也即file->f_pos
 |-vfs_read()                         # 3. 调用VFS接口
   |-file->f_op->read()               # 3.1 如果指针存在
   |-do_sync_read()                   # 如果是异步读
     |-wait_on_sync_kiocb()           # 等待数据传输完成
{% endhighlight %}

通过 fdget_pos() 返回当前进程下标为 fd (文件描述符) 的 struct file 结构体，其中 current 为当前进程 task_struct，实际返回 current -> files_struct -> fdtable -> file[fd] 这个 struct file 结构体。

在此之前，以及通过 sys_open() 把进程的这个 fd 对应的文件从硬盘上读取或创建好了，所以这里可以之前从数组里面读取。注意，该函数同时会将 file->file_opeeration 设置为 inode->file_operation 。

{% highlight c %}
ssize_t vfs_read(struct file *file, char __user *buf, size_t count, loff_t *pos)
{
    ... ...
    ret = rw_verify_area(READ, file, pos, count);
    if (ret >= 0) {
        count = ret;
        if (file->f_op->read)
            ret = file->f_op->read(file, buf, count, pos);
        else if (file->f_op->aio_read)
            ret = do_sync_read(file, buf, count, pos);
        else
            ret = new_sync_read(file, buf, count, pos);
        if (ret > 0) {
            fsnotify_access(file);
            add_rchar(current, ret);
        }
        inc_syscr(current);
    }

    return ret;
}

struct file {
    ... ...
    const struct file_operations    *f_op;
    ... ...
};
struct file_operations {
    ... ...
    ssize_t (*read) (struct file *, char __user *, size_t, loff_t *);
    ... ...
};
{% endhighlight %}

获取偏移量之后会调用 vfs_read()，该函数会把内核读取的文件内容存入 buf 指向的内存地址。如果是同步读，则会直接调用 file->f_op 对应的 read()，也就是直接调用 inode 对应的文件操作。

如果是异步，也即 f_op 中存在 aio_read() 函数，则会调用 do_sync_read() 。该函数首先会将 buf 和 len 保存，以供后续向用户空间传递数据。




<!--
dentry与inode有什么联系和区别
http://blog.chinaunix.net/uid-26557245-id-3432038.html




## 源码

与文件系统相关的绝大部分代码都保存在 fs 目录下，包括了 VFS、Buffer Cache、可执行文件的格式等等，而不同的文件系统则保存在子目录下，例如 `fs/ext4` 。

## 挂载

以下的讲解都以 ext4 为例，其对应的代码在 `fs/ext4` 目录下。

在系统启动时，会通过 `register_filesystem()` 注册一个文件系统，注册过程实际上将表示各实际文件系统的 `struct file_system_type` 数据结构实例化，并添加到一个链表，内核中用一个名为 `file_systems` 的全局变量来指向该链表的表头。

struct file_system_type {
	const char *name;  # 文件系统的名称，唯一标示
	int fs_flags;
	struct dentry *(*mount) (struct file_system_type *, int,
			const char *, void *);   # 关键的mount操作，用于挂在文件系统
	void (*kill_sb) (struct super_block *);
	struct module *owner;
	struct file_system_type * next; # 链表指针
	struct hlist_head fs_supers;
};

对于每个 mount 的文件系统，都会为它创建一个 `struct super_block` 的数据结构，该结构体保存了文件系统以及挂载点的相关信息。

由于可以在不同的路径挂载同一个文件系统，例如 `/`、`/home`、`/opt` 都挂载了 `ext4` 文件系统，因此同一个文件系统类型会对应多个 `struct super_block` 结构体，而 `fs_supers` 成员就把这个文件系统类型对应的所有 `struct super_block` 都链接起来。

其中比较关键的是 `mount` 回调函数，它是 VFS 能够和底层文件系统交互的起始点，用于处理用户空间的 mount 命令，会读取磁盘数据并实例化一个对应的 `struct super_block` 结构体，对应到 ext4 中就是 `ext4_mount()` 函数。

struct super_block {
	struct file_system_type *s_type;     // 指向文件系统的反向指针
	const struct super_operations *s_op; // 与super block相关的核心操作
};


### 总结

挂载就是实例化一个 `struct super_block` 结构体，并添加到 `fs_supers` 链表上，每个挂载点会对应一个 `struct super_block` 实例。

##

关于各个结构体之间的相互关联关系，有不错的图表可供参考
https://www.linux.it/~rubini/docs/vfs/vfs.html

https://blog.csdn.net/jasonchen_gbd/article/details/51511261
https://blog.csdn.net/u010424605/article/details/41842877
http://hushi55.github.io/2015/10/19/linux-kernel-vfs
https://lihaoquan.me/2019/2/16/linux-vfs.html


该函数是不能放在super_block结构中的，因为super_block是在get_sb执行之后才能建立的。get_sb从底层文件系统获取super_block的信息，是和底层文件系统相关的。

## VFS

虚拟文件系统有四个主要对象类型：
（1）superblock 表示特定加载的文件系统。
（2）inode 表示特定的文件。
（3）dentry 表示一个目录项，路径的一个组成部分。
（4）file 表示进程打开的一个文件。


Superblock

超级块（spuerblock）对象由各自的文件系统实现，用来存储文件系统的信息。这个对 象对应为文件系统超级块或者文件系统控制块，它存储在磁盘特定的扇区上。不是基于磁盘 的文件系统（基于内存的虚拟文件系统，如sysfs）临时生成超级块，并保存在内存中。

Inode

索引节点对象包含了内核在操作文件或目录时需要的全部信息。对于Unix文件系统来 说，这些信息可以从磁盘索引节点直接读入。如果一个文件系统没有索引节点，那么，不管 这些相关信息在磁盘上是怎么存放的，文件系统都必须从中提取这些信息。

Dentry

为了方便查找，VFS引入目录项的概念。每个dentry代表路径中一个特定部分。对于 /bin/ls来说，/、bin和ls都是目录项对象。前面是两个目录，最后一个是普通文件。在路径中， 包括普通文件在内，每一个部分都是目录项对象。解析一个路径是一个耗时的、常规的字符 串比较过程。

File

VFS最后一个主要对象是文件对象。文件对象表示进程已打开的文件。如果我们站在用 户空间的角度考虑VFS，文件对象会首先进入我们的视野。进程直接处理的是文件，而不是 超级块、索引节点或目录项。文件对象包含我们非常熟悉的信息（如访问模式、当前偏移等）， 同样道理，文件操作和我们非常熟悉的系统调用read（）和write（）等也很类似。

文件对象是已打开的文件在内存中的表示。该对象（不是物理文件）由相应的open（） 系统调用创建，由close（）系统调用销毁，所有这些文件相关的调用实际上都是文件操作 表中定义的方法。

## 参考

[A tour of the Linux VFS](https://www.tldp.org/LDP/khg/HyperNews/get/fs/vfstour.html) 关于虚拟文件系统的概览介绍。

图片不错
https://lihaoquan.me/2019/2/16/linux-vfs.html
在 NUMA 系统上，由于不同 CPU 访问本地内存和远端内存的时间相差很大，所以为了更好地调度 Linux 内核引入了 Scheduling Domain 概念。

https://www.linux.it/~rubini/docs/vfs/vfs.html

http://www.haifux.org/lectures/119/linux-2.4-vfs/vfs_relations_static.png
-->

{% highlight text %}
{% endhighlight %}
