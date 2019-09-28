---
Date: December 12, 2015
title: Linux 中的 socketfs
layout: post
comments: true
language: chinese
category: [network, linux]
---

BSD socket 是用户程序与网络协议栈之间的接口层，用户通过调用 socket API 将报文传给协议栈，以及从协议栈读取报文。实际上，Linux 对于网络提供了一个与虚拟文件系统相似的接口，也就是可以通过 socket 接口打开一个类似的文件，而内核中实际是通过 sockfs 文件系统实现的。

接下来我们就在这篇文章中查看下与 socketfs 相关的内容。

<!-- more -->

# Socket层，系统调用

系统调用 socket()、bind()、connect()、accept()、send()、release() 等都是在 net/socket.c 实现的，下面介绍通过 socket 层和用户的衔接。

对于这么多的协议，都是通过 socket() 向用户提供统一的接口，下面是一个典型的 TCP 协议通讯。

{% highlight text %}
# 服务端
listenfd = socket(AF_INET, SOCK_STREAM, 0);                 # 新建socket
bind(sock_descriptor, servaddr, size);                      # 绑定端口
listen(listenfd, 5);                                        # 开始监听端口
accept(listenfd, cliaddr, clilen);                          # 接收新请求

# 客户端
sock_descriptor = socket(AF_INET, SOCK_STREAM, 0);          # 新建socket
connect(sock_descriptor, sockaddr, size);                   # 与服务端建立链接
send(sock_descriptor, "hello world");                       # 发送数据
recv(sock_descriptor, buffer, 1024, 0);                     # 接收数据
{% endhighlight %}

socket() 的声明和实现如下，三个参数分别为协议族、协议类型 (面向连接或无连接) 以及协议。

{% highlight text %}
int socket(int domain, int type, int protocol);
{% endhighlight %}

## socketfs 初始化

对于用户态而言, 返回的 socket 就是一个特殊的已经打开的文件，为了对 socket 抽像出文件的概念，内核中为 socket 定义了一个专门的文件系统类型 sockfs 。

{% highlight c %}
static struct vfsmount *sock_mnt __read_mostly;
static struct file_system_type sock_fs_type = {
    .name     = "sockfs",
    .get_sb   = sockfs_get_sb,
    .kill_sb  = kill_anon_super,
};
{% endhighlight %}

socket 系统的初始化通过 sock_init() 函数完成，通过如下程序实现。

{% highlight c %}
core_initcall(sock_init);
{% endhighlight %}

通过 core_initcall() 宏实现，其中定义的函数会在系统初始化时调用，详细可以参考 [Linux initcall 机制实现](linux-initcalls)，在通过 sock_init() 模块初始化的时候，会安装该文件系统。

{% highlight text %}
sock_init()
  |-net_sysctl_init()
  |-skb_init()
  |-init_inodecache()
  |-register_filesystem()             # 将文件系统添加到一个列表中
  |-kern_mount()                      # 挂载文件系统
    |-kern_mount_data()
      |-vfs_kern_mount()
        |-alloc_vfsmnt()
        |-mount_fs()                  # 返回root dentry
{% endhighlight %}

在 vfs_kern_mount() 中，会申请文件系统 mnt 结构，调用之前注册的 sock_fs_type 的 get_sb()，获取相应的超级块，并将 mnt->mnt_sb 指向 sock_fs_type 中的超级块。

这里就是先获取/分配一个超级块，然后初始化超级块的各成员，包括 s_op，它封装了对应的功能函数表。s_op 自然就指向了 sockfs_ops，那前面提到的 new_inode() 函数分配 inode 时调用的，这个函数实际对应 sock_alloc_inode() 函数。

{% highlight c %}
sock_mnt->mnt_sb->s_op->alloc_inode(sock_mnt->mnt_sb);
{% endhighlight %}

可以看到 sock_alloc_inode() 是如何分配一个 inode 节点的，函数先分配了一个用于封装 socket 和 inode 的 ei，然后在高速缓存中为之申请了一块空间，这样 inode 和 socket 就同时都被分配了。

至目前为止，分配 inode、socket 以及两者如何关联，都已一一分析了。最后一个关键问题，就是如何把 socket 与一个已打开的文件，建立映射关系。


sys_socketcall() 包含了所有 socket API 的入口。


# socket()

有了文件系统后，对内核而言，创建一个 socket，就是在 sockfs 文件系统中创建一个文件节点(inode)，并建立起为了实现 socket 功能所需的一整套数据结构，包括 struct inode 和 struct socket。而 struct socket 结构在内核中就代表了一个 socket，然后再将其与一个已打开的文件 "建立映射关系"，这样，用户态就可以用抽像的文件的概念来操作 socket 了。

而用户看到的是一个类似于文件描述符的 int 类型。在内核中 struct task_struct current 用来表示当前进程，用 struct file 描述一个已经打开的文件，当然一个进程可以打开多个文件，所以通过 struct file *fd_array[] 表示，文件描述符即对应该数组的下标。

通过文件描述符即可以找到对应内核中的 struct file 结构。

因此，对于网络编程需要做的是将 socket 与一个已经打开的文件建立映射，也就是为 socket 分配一个 struct file 以及相应的文件描述符 fd 。

如前所述，一个 socket 总是与一个 inode 密切相关的，为此内核引入了一个 socket_alloc 结构，当已知一个 inode 可以通过宏 SOCKET_I() 获取对应的 socket 。

{% highlight c %}
struct socket_alloc {
    struct socket socket;
    struct inode vfs_inode;
};
sock = SOCKET_I(inode);
static inline struct socket *SOCKET_I(struct inode *inode)
{
    return &container_of(inode, struct socket_alloc, vfs_inode)->socket;
}
{% endhighlight %}

这也同时意味着在正常分配一个 inode 后，必须再分配一个 socket_alloc 结构，并实现对应的封装。

接着申请分配一个相应的文件描述符 fd，因为 socket 并不支持 open() 方法，所以不能期望用户通过调用 open() API 分配一个 struct file，而是 sock_alloc_file() 获取，并通过让 current 的 files 指针的 fd 数组的 fd 索引项指向该 file 。

{% highlight text %}
sys_socket(family, type, protocol)
  |-sock_create()                         # 分配inode和socket_alloc
  | |-__sock_create()
  |   |-security_socket_create()          # 调用安全接口，一般是selinux，可忽略
  |   |-socket *sock=sock_alloc()         # 主要的分配函数，分配一个struct socket
  |   | |-new_inode_pseudo()              # 通过sock_mnt->mnt_sb生成一个inode
  |   | | |-alloc_inode()                 # 新建inode，并添加到sb.s_inodes链表中
  |   | |-socket *sock=SOCKET_I(inode)    # 根据上述的inode，获取sock
  |   |
  |   |-sock->type=type
  |   |-pf=net_families[family]
  |   |-pf->create(..., protocol, ...)    # 其中TCP对应了inet_create()函数
  |   | |-inetsw[sock->type]              # 遍厉该数组的对象
  |   | |-sock->ops=answer->ops           ### struct proto_ops类型，后续收发操作的主要函数
  |   | |-sock *sk=sk_alloc()             # 分配sock结构体
  |   |
  |   |-security_socket_post_create()
  |
  |-sock_map_fd()
    |-get_unused_fd_flags()               # 获取未使用的fd
    |-sock_alloc_file()                   # 分配一个struct file
    |-fd_install()
{% endhighlight %}

在 alloc_inode() 函数中，会调用 sock_mnt->mnt_sb->s_op->alloc_inode(sock_mnt->mnt_sb) 返回一个 inode 结构体，而该 alloc_inode() 函数实际对应的是 sockfs_ops 中的成员变量。

在 Linux 中，通过 net_families[] 数组标示不同的 family 类型，对于 TCP/IP 采用的是 AF_INET 。

{% highlight c %}
#define PF_UNIX     AF_UNIX
#define PF_INET     AF_INET

static const struct net_proto_family __rcu *net_families[NPROTO] __read_mostly;

static const struct net_proto_family inet_family_ops = {
    .family = PF_INET,
    .create = inet_create,
    .owner  = THIS_MODULE,
};
{% endhighlight %}

其中，仍以 TCP 为例，其中入参采用 SOCK_STREAM，其中的 inetsw_array[] 会在 inet_init() 函数中通过 inet_register_protosw() 函数注册到 static struct list_head inetsw[SOCK_MAX] 中。

{% highlight c %}
const struct proto_ops inet_stream_ops = {
    .family        = PF_INET,
    .owner         = THIS_MODULE,
    .sendmsg       = inet_sendmsg,
    .recvmsg       = inet_recvmsg,
    ... ...
};

static struct inet_protosw inetsw_array[] =
{
    {
        .type =       SOCK_STREAM,
        .protocol =   IPPROTO_TCP,
        .prot =       &tcp_prot,
        .ops =        &inet_stream_ops,
        .flags =      INET_PROTOSW_PERMANENT | INET_PROTOSW_ICSK,
    },
    ... ...
};

static int __init inet_init(void)
{
    ... ...
    for (q = inetsw_array; q < &inetsw_array[INETSW_ARRAY_LEN]; ++q)
        inet_register_protosw(q);
    ... ...
}
{% endhighlight %}

那么，在 sys_socket() 函数中就会根据 family、type 的参数，通过 sock->ops=answer->ops 进行赋值，后面包括报文的收发操作等，都是根据该结构体进行操作。


## 小结

上面已经介绍了在协议栈中是如何选择相关操作函数的，在此仍然一步步查看下具体的排查过程。

首先，socket() 系统调用会传入三个参数，接口如下。

{% highlight c %}
SYSCALL_DEFINE3(socket, int, family, int, type, int, protocol)
{
    ... ...
    retval = sock_create(family, type, protocol, &sock);
    ... ...
}
{% endhighlight %}

实际最终会调用如下的函数。

{% highlight c %}
int __sock_create(..., int family, int type, int protocol, ...) {
    ... ...
    struct socket *sock = sock_alloc();
    sokc->type = type;

    const struct net_proto_family *pf;
    pf = rcu_dereference(net_families[family]);   //## 关系到如下函数的调用

    err = pf->create(net, sock, protocol, kern);
    ... ...
}
{% endhighlight %}

其中 net_families[] 数组通过 sock_register() 函数进行注册，通过如下命令看到所有注册的协议，在此以 AF_INET 或者 PF_INET 为例，注册的是 inet_family_ops 。

{% highlight text %}
$ cd net && grep -rne '\<sock_register('
... ...
ipv4/af_inet.c:1704:    (void)sock_register(&inet_family_ops);
... ...
{% endhighlight %}

而对应的结构体定义为：

{% highlight c %}
static const struct net_proto_family inet_family_ops = {
    .family = PF_INET,
    .create = inet_create,
    .owner  = THIS_MODULE,
};
{% endhighlight %}

也就是接下来实际调用的函数是：

{% highlight c %}
static int inet_create(struct net *net, struct socket *sock, int protocol, int kern)
{
    ... ...
    err = -ESOCKTNOSUPPORT;
    rcu_read_lock();
    list_for_each_entry_rcu(answer, &inetsw[sock->type], list) {
        err = 0;
        /* Check the non-wild match. */
        if (protocol == answer->protocol) {
            if (protocol != IPPROTO_IP)
                break;
        }
    }
    ... ...
    sock->ops = answer->ops;
    ... ...
}
{% endhighlight %}

而其中 inetsw[] 数组实际是通过 inetsw_array[] 注册的结果。


# send() 类的实现

在 Linux 中，应用层可以使用以下 socket 函数来发送数据：

{% highlight c %}
ssize_t write(int fd, const void *buf, size_t count);
ssize_t send(int s, const void *buf, size_t len, int flags);
ssize_t sendto(int s, const void *buf, size_t len, int flags, const struct sockaddr *to, socklen_t tolen);
ssize_t sendmsg(int s, const struct msghdr *msg, int flags);
int sendmmsg(int s, struct mmsghdr *msgvec,  unsigned int vlen, unsigned int flags);
{% endhighlight %}

当 flags 为 0 时，send() 和 write() 功能相同；而 sendto(..., NULL, 0) 功能与 send() 的功能相同；write() 和 send() 在套接字处于连接状态时可以使用，而 sendto()、sendmsg() 和 sendmmsg() 在任何时候都可用。

对于 send() 而言，其入口函数就是 sys_send() 。

{% highlight text %}
sys_send()
  |-sys_sendto()
    |-sockfd_lookup_light()         # 通过文件描述符fd，找到对应的socket实例
    |-... ...                       # 初始化消息头信息
    |-move_addr_to_kernel()         # 把套接字地址从用户空间拷贝到内核空间
    |-sock_sendmsg()                # 调用统一的发送入口函数
    | |-__sock_sendmsg()
    |   |-__sock_sendmsg_nosec()
    |     |-sock->ops->sendmsg()
    |
    |-fput_light()

{% endhighlight %}

在 sockfd_lookup_light() 函数中，以 fd 为索引从当前进程的文件描述符表 files_struct 实例中找到对应的 file 实例，然后从 file 实例的 private_data 成员中获取 socket 实例。

其中 sock->ops->sendmsg() 操作相关的结构体如下，其中 sock->ops 在 sys_socket() 中初始化。

{% highlight c %}
struct socket {
    ... ...
    const struct proto_ops  *ops;
};

struct proto_ops {
    int     family;
    struct module   *owner;
    int     (*release)   (struct socket *sock);
    int     (*bind)      (struct socket *sock, struct sockaddr *myaddr, int sockaddr_len);
    int     (*connect)   (struct socket *sock, struct sockaddr *vaddr, int sockaddr_len, int flags);
    int     (*socketpair)(struct socket *sock1, struct socket *sock2);
    int     (*accept)    (struct socket *sock, struct socket *newsock, int flags);
    int     (*listen)    (struct socket *sock, int len);
    int     (*shutdown)  (struct socket *sock, int flags);
    int     (*sendmsg)   (struct kiocb *iocb, struct socket *sock,
                      struct msghdr *m, size_t total_len);
    int     (*recvmsg)   (struct kiocb *iocb, struct socket *sock,
                      struct msghdr *m, size_t total_len, int flags);
};
{% endhighlight %}

其中 socket(AF_INET, SOCK_STREAM, 0) 会将 sock->ops 设置为 inet_stream_ops 变量，那么后续的所有操作都会使用该结构体定义的函数进行操作。

# 参考

<a href="http://hellojavaer.iteye.com/blog/1087585">Linux 内核协议栈</a> 关于 socket 创建的过程详解，或者参考 <a href="http://blog.chinaunix.net/uid-21505072-id-3076743.html">Linux TCP/IP协议栈之Socket的实现分析(一 套接字的创建)</a> 。

<a href="http://blog.csdn.net/zhangskd/article/details/13631715">Socket 层实现</a> 同样是内核 Socket 层的相关实现。

{% highlight c %}
{% endhighlight %}
