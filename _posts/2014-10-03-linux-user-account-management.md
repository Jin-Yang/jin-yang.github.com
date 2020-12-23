---
title: Linux 用户管理
layout: post
comments: true
language: chinese
tag: [Linux,DevOps]
keywords: linux,用户管理
description: Linux 是一个多用户多任务的分时操作系统，可以允许多个用户同时登陆，通过账户管理，可以方便系统管理员对用户进行跟踪，控制系统资源的访问，也可以帮助用户组织文件，并为用户提供安全性保护。
---

Linux 是一个多用户多任务的分时操作系统，可以允许多个用户同时登陆，通过账户管理，可以方便系统管理员对用户进行跟踪，控制系统资源的访问，也可以帮助用户组织文件，并为用户提供安全性保护。

对于普通用户来说，每个账号都拥有一个惟一的用户名以及口令，只有在输入正确之后才可以进入系统以及主目录。

这里详细介绍与用户管理相关的内容。

<!-- more -->

## 简介

任何使用操作系统的用户，都需要通过一个账号登陆，登陆之后，文件的权限、运行进程等，都是和该用户相关的，管理员可以以此来控制用户的权限范围。

在 Linux 中可以将用户分成三类：

* 管理员账号，有最高权限，几乎可以为所欲为，一般为 `root` ；
* 系统账号，一般是应用程序使用的账号，无法通过终端或者界面进行登陆；
* 普通账号，用来登陆系统，也是最常用的账号类型。

需要注意的是，在 Linux 系统内部，并不是通过账户名进行判断的，而是根据 UID (User ID) 来判断，只要 ID 是 0 ，那么就是管理员，哪怕有多个 ID 为 0 的账号。

另外，为了方便账户的管理，提供了组的概念，一个账户可以划分到多个组中；而且与账号类似，系统内部同样使用的是 GID (Group ID) 进行判断。

创建账号时，默认会根据账号类型自动分配递增的 UID/GID ，当然，也可通过参数指定，不过最好要保证 UID 与 GID 唯一，否则可能会出现非预期的行为，除非明确之后后果。

注意，再强调一遍，Linux 系统中使用的 UID/GID 数字，主要当需要人查看时，才会将 ID 信息转换为可读的用户名称。

### 用户组

相同用户组内的用户可以共享文件，可以简化管理员权限管理，如果不通过参数指定，默认的 UID 和 GID 会递增，所以，通常来说是相同的，如果中间有通过参数指定了用户组，那么可能会导致两者不同。

可以通过 `groupadd -g <GID> <GroupName>` 命令指定用户组 ID ，然后在创建用户时通过 `useradd -g <GroupName> <UserName>` 命令显示指定用户所属的用户组。

#### 主要用户组 VS. 次要用户组

用户组会分成两种：A) 主要用户组 Primary Group；B) 次要用户组 Secondary Group ，每个用户都有一个主要用户组，并可以成为任意多个次要用户组的成员，如果要与其它组共享文件，那么只需要设置文件的组权限，并将相关用户以次要用户组的方式与文件属组关联即可。

其中主用户组信息保存在 `/etc/passwd` 文件中，附加用户组则会在 `/etc/group` 中添加。另外，创建的文件是属于主用户组的，权限管理的时候需要注意。

可以通过 `id -n -g <UserName>` 查看用户的主用户组信息，通过 `id -n -G <UserName>` 查看用户组信息。

### 使用示例

账号的管理需要使用管理员，这里以在 CentOS 中创建一个 `foobar` 用户为例，命令通用，只是不同发行版本略有区别。

#### 创建账号

最简单的是通过 `useradd foobar` 命令创建用户，此时会直接使用系统的默认参数。

```
$ grep foobar /etc/passwd
foobar:x:1002:1002::/home/foobar:/bin/bash
```

对于 CentOS 来说，普通用户的 UID 会从 `1000` 开始递增。

#### 设置密码

上面的命令只是添加了一个用户，但是没有设置密码。

```
$ grep foobar /etc/shadow
foobar:!!:18549:0:99999:7:::
```

这也就意味着，目前只能从 `root` 切换到 `foobar` ，无法登陆、无法从普通用户切换到 `foobar` ，可以使用管理员用户通过 `passwd foobar` 命令设置密码。

{% include ads_content01.html %}

## 相关配置文件

所有的用户以及用户组信息都是保存在文件中的，其中与账号相关的文件以及目录有：

* `/etc/passwd` 保存了用户信息，包括了用户名、UID、GID、主目录、Shell 等信息。
* `/etc/shadow` 保存用户的密码信息，包括了用户名、加密后的密钥、过期时间、最近修改时间等等。
* `/etc/group` 用户组配置文件，包括了组名称、组密码、GID、附加组信息等。
* `/etc/gshadow` 保存了组密码信息，与 `/etc/shadow` 文件类似。

另外，还有一些服务的配置文件/目录。

* `/etc/skel/` 存放新用户默认配置文件的目录，在新建用户时会将该目录复制到新建的用户目录下。
* `/etc/default/useradd`
* `/etc/login.defs`

### /etc/passwd

保存了用户的基本信息，每行为一个用户信息，一般权限为 `644` ，也就是所有用户都可以读取，但只有管理员可以修改。如下是 CentOS 中的部分文件内容，其中包括了管理员用户、系统用户以及普通用户。

该文件的部分内容摘抄如下。

{% highlight text %}
root:x:0:0:root:/root:/bin/bash
bin:x:1:1:bin:/bin:/sbin/nologin
daemon:x:2:2:daemon:/sbin:/sbin/nologin
nobody:x:65534:65534:Kernel Overflow User:/:/sbin/nologin
... ...
foobar:x:1002:1002::/home/foobar:/bin/bash
{% endhighlight %}

通过冒号 `:` 将每行分割，总共有 7 个字段：

1. 用户名，很多系统限制长度为 8 个字符，由大小写字母和数字组成，考虑到兼容性，不建议使用特殊字符。与 Windows 不同，Linux 中是区分大小写的，也就是说 `Foobar` 和 `foobar` 是两个不同的用户。
2. 加密口令，老版本会将密码加密后保存在该字段，新版本通常保存在 `/etc/shadow` 文件中，这个字段只作为占位符，通常用字符 `x` 或者 `*` 表示。
3. UID 一个整数，通常与用户名一一对应，如果两个用户相同，那么系统内部实际会视为同一个用户，但是它们可以有不同的口令、主目录、登录 Shell 等。
4. GID 也是整数，记录了用户所属的用户组，对应了 `/etc/group` 文件中的一条记录。
5. 注释信息，可以记录真实姓名、电话、地址等，Linux 对该字段没有明确的约束，很多发布版本中是一段注释性的文字。
6. 主目录，也就是所谓的 `HOME` 目录，通常登陆后会直接切换到该目录，一般是在 `/home/<USERNAME>` 目录下，通常用户对自己的主目录有所有的权限，对其它用户无法访问。
7. Shell 程序，可以指定用户登陆时默认的 Shell 命令绝对路径，一般有 `/bin/bash` `/bin/sh` 等，系统用户一般不允许登陆，通常会被设置为 `/sbin/nologin`。

其中 UID 的范围为 `[0, 65535]` ，其中 `0` 为管理员用户，所有的发布版本一致；但是，不同发布版本对系统用户与普通用户的边界定义不同，例如 CentOS 的普通用户是从 `1000` 开始，对应 `[1, 999]` 是系统用户。

而且一般会内置很多系统用户，例如 `bin` `nobody` 等等。

### /etc/shadow

用来保存 Linux 中用户的密码信息，该文件权限为 `000` ，也就意味着只有 root 用户才可以访问，从而可以在一定程度上保证密码的安全性。

该文件的部分内容摘抄如下。

{% highlight text %}
root:$6$PfRTCuHS50Bsv3EK$W3bohSpKMOMBBiiEwok7rqX8Lu74N4HO4LAxZRiDNahlTRSbuy3T3WRU5lDtAoLUD3pdCAb0jGWmyL2O16eft0::0:99999:7:::
bin:*:18078:0:99999:7:::
daemon:*:18078:0:99999:7:::
dbus:!!:18167::::::
{% endhighlight %}

与 `/etc/passwd` 文件一样，每行代表一个用户，而且使用冒号 `:` 作为分割符，总共分成了 9 个字段：

1. 用户名，与 `/etc/passwd` 文件中保持一致。
2. 加密密码，这里保存的是真正加密后的密码信息，详细的加密方式可以参考 [详解 Linux 用户密码管理](/post/linux-manage-user-password.html) 中的介绍。
3. 最近一次修改时间，使用的是从 1970.1.1 以来的总天数。
4. 密码修改的最小时间间隔，单位是天，也就是据上次修改多久不能修改密码，防止用户频繁修改，如果是 0 ，则表示随时可以修改。
5. 密码有效期，单位是天，同样是通过距离上次修改时间判断，强制用户定期修改密码，默认是 99999 天，可以认为是永久生效。
6. 密码修改前警告天数，当账号的密码快过期时，提前进行提示。
7. 密码过期后的宽限天数，过期后，如果用户还未修改密码，在宽限天数内仍可以登陆，过期后完全禁用，其中 0 表示过期后立即失效，-1 表示永不失效。
8. 账号失效时间，同第三字段一样，使用从 1970.1.1 以来的总天数作为账户的失效时间，账号在此字段规定的时间之外，不论密码是否过期，都将无法使用。
9. 保留，暂未使用。

注意，对于第二个字段，所有的系统用户是 `!!` 或者 `*` 表示没有密码无法登陆，如果是新建的用户，一般是 `!!` 表示没有密码不能登陆。第三个字段可以通过 `date -d "1970-01-01 NNN days"` 转换为习惯阅读的日期。

### /ect/group

保存了与用户组相关的信息，对应了 `/etc/passwd` 中的第四个字段，

该文件的部分内容摘抄如下。

{% highlight text %}
root:x:0:
bin:x:1:
... ...
foobar:x:1002:
{% endhighlight %}

同样每行代表一个用户组，通过冒号 `:` 将每行分成四个字段，每个字段对应的含义为：

1. 组名称，与用户名类似，只在对用户显示时使用。
2. 组密码，与 `/etc/passwd` 一样，通过 `x` 标识密码，实际保存在 `/etc/gshadow` 文件中。
3. GID，与 UID 类似，系统通过 GID 区分用户，组名称只在对用户显示时使用。
4. 组中的用户，会列出该组中包含的所有附加用户。

用户组密码用来指定组管理员，随着系统中账号增加，系统管理员可能没有足够时间管理，这样就可以通过组管理员代替系统管理员进行管理，只是该功能现在使用的很少。

<!--
举个例子，lamp 组的组信息为 "lamp:x:502:"，可以看到，第四个字段没有写入 lamp 用户，因为 lamp 组是 lamp 用户的初始组。如果要查询这些用户的初始组，则需要先到 /etc/passwd 文件中查看 GID（第四个字段），然后到 /etc/group 文件中比对组名。

每个用户都可以加入多个附加组，但是只能属于一个初始组。所以我们在实际工作中，如果需要把用户加入其他组，则需要以附加组的形式添加。例如，我们想让 lamp 也加入 root 这个群组，那么只需要在第一行的最后一个字段加入 lamp，即 root:x:0:lamp 就可以了。

一般情况下，用户的初始组就是在建立用户的同时建立的和用户名相同的组。

到此，我们已经学习了/etc/passwd、/etc/shadow、/etc/group，它们之间的关系可以这样理解，即先在 /etc/group 文件中查询用户组的 GID 和组名；然后在 /etc/passwd 文件中查找该 GID 是哪个用户的初始组，同时提取这个用户的用户名和 UID；最后通过 UID 到 /etc/shadow 文件中提取和这个用户相匹配的密码。
-->

### /etc/skel/

这个目录用来定义创建用户时的默认环境，新建用户时该目录下所有的文件都会复制到新创建用户的 `HOME` 目录下，一般都是隐藏的文件，通过修改该目录下的文件，可以提供统一、标准化的用户初始环境。

例如在 CentOS 桌面版的内容如下。

{% highlight text %}
$ ls /etc/skel/ -alh
total 24K
drwxr-xr-x.   3 root root   78 Sep 28  2019 .
drwxr-xr-x. 147 root root 8.0K Oct 14 23:14 ..
-rw-r--r--.   1 root root   18 Nov  9  2019 .bash_logout
-rw-r--r--.   1 root root  141 Nov  9  2019 .bash_profile
-rw-r--r--.   1 root root  312 Nov  9  2019 .bashrc
drwxr-xr-x.   4 root root   39 Sep 28  2019 .mozilla
{% endhighlight %}

一般默认就是一些 Bash 相关的配置文件。

<!--
### /etc/login.defs

/etc/login.defs 是设置用户帐号限制的文件。该文件里的配置对root用户无效。

如果/etc/shadow文件里有相同的选项，则以/etc/shadow里的设置为准，也就是说/etc/shadow的配置优先级高于/etc/login.defs
-->

## 用户管理

所谓的用户管理就是添加、删除用户，管理用户的密码过期时间等，如下所有的示例都是对 foobar 账号进行操作。

常用的命令有：

* `useradd` 与 `adduser` 命令相同，用来添加用户；
* `userdel` 删除用户及相关用户的配置文件；
* `usermod` 修改用户配置，例如家目录、默认 Shell 等；
* `passwd` 为用户设置、修改密码；
* `id` 查看用户的 UID/GID 等信息；
* `su` 切换用户；

<!--
chage	修改用户密码有效期限。管理/etc/shadow
sudo	sudo是通过另一个用户来执行命令，su是用户来切换用户，然后通过切换到的用户来完成相应的任务，但sudo能在命令后面直接接命令执行，如：sudo ls /root，不需要root密码就可以执行只有root才能执行相应的命令或具备的目录权限；这个权限需要通过visudo命令或编辑/etc/sudoers来实现
visudo	visudo配置sudo权限的编辑命令；也可以不用这个命令，直接用vi来编辑/etc/sudoers实现。但推荐用visudo来操作（会自动检查语法）
-->

在 CentOS 中，`useradd` 与 `adduser` 命令相同，其中 `adduser` 是一个符号链接，指向了 `useradd` 命令。

{% highlight text %}
$ ls -alh  /usr/sbin/useradd
-rwxr-xr-x 1 root root 237K Nov  9  2019 /usr/sbin/useradd
$ ls -alh /usr/sbin/adduser
lrwxrwxrwx 1 root root 7 Nov  9  2019 /usr/sbin/adduser -> useradd
{% endhighlight %}

### 查看用户

通过 `id user` 命令查看用户。

### 创建用户

可以通过 `useradd` 命令创建用户，会有一个大致的创建流程，但是可以通过 `/etc/login.defs` 配置文件进行定制，该文件定义了部分在创建用户时的默认配置选项。

不同的发行版本会略有区别，不过大致的操作步骤为：

1. 帐号信息添加到 `/etc/passwd`、`/etc/shadow`、`/etc/group`、`/etc/gshadow` 文件中。
2. 创建 `/home/<USERNAME>` 目录。
3. 将 `/etc/skel ` 目录下的内容复制到 `/home/<USERNAME>` 目录下，很多是隐藏文件。
4. 在 `/var/mail` 目录下创建邮箱帐号。

其中第一步基本上所有的发行版本都会执行，而剩余的不同的发行版本会有不同的操作。最后还需要通过 `passwd USERNAME` 命令设置用户的密码，CentOS 在没有设置密码时无法登陆。

### 删除用户

在通过 `userdel USERNAME` 删除用户时，会删除 `/etc/passwd`、`/etc/group` 中的内容，但是不会删除 `/home/<USERNAME>` 目录以及 `/var/mail` 目录下文件，可以使用 `-r` 删除这两项。

### 修改用户

#### 用户组调整

新建用户，并添加到已知的附加用户组里，如果用户组不存在则需要手动创建，也可以通过逗号分隔指定多个用户组。

{% highlight text %}
# useradd -G admins,ftp,www,developers foobar
{% endhighlight %}

注意，如上的方法会同时创建一个主用户组。

也可以通过 `-g` 参数将新增加的用户初始化为指定为登录组，也就是主要用户组。

{% highlight text %}
# useradd -g developers foobar
{% endhighlight %}

其它常见操作可以参考如下：

{% highlight text %}
----- 修改主要用户组为apache
# usermod -g apache foobar
----- 将已经存在的用户添加到一个组里面，此时会在/etc/group的apache分组最后一列增加
# usermod -a -G apache foobar
{% endhighlight %}

其中 `-g` 表示 `initial login group`，`-G` 表示 `supplementary groups`。

{% include ads_content02.html %}

## 用户提权

普通用户通常有需求需要系统管理员用户执行，通常有几种方式：A) 粘滞位，执行命令时该命令会有 root 权限；B) sudoer 可以设置如何切换用户；C) capabilities 机制，允许普通用户执行部分 root 的权限。

### 粘滞位

其中一个解决方案是，类似于 passwd ，对一个 owner 为 root 的可执行文件可以增加粘滞位 (Set User ID on execution, SUID)，也就是 `chmod +s` 。

这样在运行的时候使用的就是 root 权限，带来的问题就是会导致其运行的权限过高，在某种程度上增大了安全攻击面，有些平台上 ping 也是采用上述的粘滞位机制。

### sudoer

有些时候，系统管理员需要将部分权限分配给用户，或者说，允许部分普通用户有能力切换到系统管理员，这就是所谓的 sudoer 的能力。

简单来说，需要修改 `/etc/sudoer` 配置文件。

{% highlight text %}
用户 登录的主机=(可以变换的身份) 可以执行的命令
monitor ALL=(root) NOPASSWD: /usr/sbin/groupadd

其中 ALL 也可以通过 Host_Alias 指定具体的主机信息，包括了主机名、IP地址、网络掩码等
Host_Alias HT01=localhost,etc01,10.0.0.4,255.255.255.0,192.168.1.0/24
{% endhighlight %}

### sudo VS. su

在切换时实际上是两种策略：1) su 切换到相应的用户，所以需要切换用户的密码；2) sudo 不知道 root 密码的时候执行一些 root 的命令，需要在 suders 中配置+自己用户密码。

{% highlight text %}
$ sudo su root                        ← 需要用户的密码+sudoers配置
$ su root                             ← 需要root用户密码
{% endhighlight %}

注意，之所以使用 `sudo su root` 这种方式，可能是 btmp 等类似的文件，只有 root 可以写入，否则会报 Permission Denied，此时可以通过 strace 查看报错的文件。

## 审计

CentOS 系统上，用户登录历史存储在以下这些文件中：

* `/var/run/utmp` 记录当前打开的会话，会被 who 和 w 记录当前有谁登录以及他们正在做什么。
* `/var/log/wtmp` 存储系统连接历史记录，被 last 工具用来记录最后登录的用户的列表。
* `/var/log/btmp` 失败的登录尝试，被 lastb 工具用来记录最后失败的登录尝试的列表。

实际上，可以直接通过 `utmpdump` 将上述文件中保存的数据 dump 出来，另外，默认的登陆日志保存在 `/var/log/secure` 。

{% highlight text %}
----- 当前当登录的用户的信息
# who
huey     pts/1        2015-05-11 18:29 (192.168.1.105)
sugar    pts/2        2015-05-11 18:29 (192.168.1.105)

----- 登录的用户及其当前执行的任务
# w
18:30:51 up 3 min,  2 users,  load average: 0.10, 0.14, 0.06
USER     TTY      FROM              LOGIN@   IDLE   JCPU   PCPU WHAT
huey     pts/1    192.168.1.105    18:29    3.00s  0.52s  0.00s w
sugar    pts/2    192.168.1.105    18:29    1:07   0.47s  0.47s -bash

-----  当前当登录的用户的用户名
# users
huey sugar

-----  当前与过去登录系统的用户的信息
# last
root     pts/3        192.168.1.105    Mon May 11 18:33 - 18:33  (00:00)
sugar    pts/2        192.168.1.105    Mon May 11 18:32   still logged in
sugar    pts/2        192.168.1.105    Mon May 11 18:29 - 18:32  (00:02)
huey     pts/1        192.168.1.105    Mon May 11 18:29   still logged in
reboot   system boot  3.5.0-43-generic Mon May 11 18:27 - 18:33  (00:05)
huey     pts/1        192.168.1.105    Sat May  9 10:57 - 17:31  (06:33)

----- 所有登录系统失败的用户的信息
# lastb
btmp begins Sat May  9 09:48:59 2015

----- 用户最后一次登录的信息
# lastlog
root             pts/3    192.168.1.105    一  5月 11 18:36:43 +0800 2015
huey             pts/1    192.168.1.105    一  5月 11 18:29:40 +0800 2015
mysql                                      **从未登录过**
sshd                                       **从未登录过**

----- 用户连接时间的统计数据
-----   1. 每天的总的连接时间
# ac -d
May  9  total        6.55
Today   total        0.54
----- 2. 每个用户的总的连接时间
# ac -p
    huey                                 6.78
    sugar                                0.23
    root                                 0.12
    total        7.13
{% endhighlight %}

<!--
## 安全加固


centos限制登录失败次数并锁定设置

vim /etc/pam.d/login

在#%PAM-1.0下面添加：
auth required pam_tally2.so deny=5 unlock_time=180 #登录失败5次锁定180秒，不包含root
auth required pam_tally2.so deny=5 unlock_time=180 even_deny_root root_unlock_time=180 #包含root

## 每次修改密码禁止使用前N次用过的密码

出于安全考虑，要求修改linux密码策略，每次修改密码时设置的新密码不能是前n次使用过的密码。配置如下：

Debian / Ubuntu：修改文件 # vi /etc/pam.d/common-password
CentOS / RHEL / RedHat / Fedora 修改文件 # vi /etc/pam.d/system-auth

在 password sufficient pam_unix.so use_authtok md5 shadow remember=10
在相应行的后面添加 remember=10，而不是添加一行！

SUSE比较恶心，找了半天才找到。man pw_check
在/etc/security/pam_pwcheck文件中添加remember=5
passwd:     nullok use_cracklib remember=5

http://www.deer-run.com/~hal/sysadmin/pam_cracklib.html
http://zhidao.baidu.com/link?url=xLcMH0cokvN585CPxKf3QVmmN1wDtgESTpAhl1_cxhPQZ0B3D41DhKZCcXr3E0-1nwfBtSKQWQCNKUGPhRvvcq
-->

## 杂项

简单记录常用的使用技巧。

### wheel

wheel 组是一特殊用户组，被一些 Unix 系统用来控制能否通过 su 命令来切换到 root 用户。

{% highlight text %}
$ grep 'wheel' /etc/group
wheel:x:10:foo,bar,foobar
{% endhighlight %}

可以配置成非 wheel 组的用户不能通过 su 命令切换到 root 用户。

{% highlight text %}
$ grep 'pam_wheel' /etc/pam.d/su
auth            required        pam_wheel.so use_uid

$ grep 'WHEEL' /etc/login.defs
SU_WHEEL_ONLY yes
{% endhighlight %}

这时非 wheel 组的成员用 su 命令切换到 root 时提示权限不够，而用 wheel 组的成员切换没任何问题。


### 忘记密码

如果是普通用户的密码丢失，主要 root 用户可以登陆，那么就可以直接 root 用户登陆，通过 `passwd <UserName>` 重新配置对应用户的密码即可。

对于 root 用户来说比较麻烦些，需要重启进入 grub 界面中，通过 `e` 进入编辑方式，添加 single 参数 (也就是进入单用户模式)，登陆之后，通过 `passwd` 命令修改密码即可。

也可以通过 U 盘启动一个小的 Linux 操作系统，例如 Puppy ，然后将原系统的根目录挂载，直接修改 `/etc/shadow` 文件，将 root 对应的记录删除，这样无需密码就可以通过 root 登陆，然后再通过 `passwd` 命令修改密码。

### 登陆提示信息

通过终端登陆时 (一般是服务器) ，可以自己定义一些欢迎界面，这样可以针对环境、运行的产品等进行定制，方便用户识别当前登陆的机器。

涉及的有两个配置文件 `/etc/issue` 以及 `/etc/motd`，其中前者为登陆前的提示，后者为登陆后的提示信息。对于 CentOS 来说，可以通过 `Ctrl+Alt+F5` 切换到终端登陆。

如下为 CentOS 8 的默认设置。

{% highlight text %}
----- 提示信息为
CentOS Linux 8 (Core)
Kernel 4.18.0-193.14.2.el8_2.x86_64 on an X86_64

----- 其中/etc/issue配置为
\S
Kernel \r on an \m

----- 配置文件中各个选项的含义为：
    \d 本地端时间的日期；
    \l 显示第几个终端机接口；
    \m 显示硬件的等级 (i386/i486/i586/i686)；
    \n 显示主机的网络名称；
    \o 显示 domain name；
    \r 操作系统的版本 (相当于 uname -r)
    \t 显示本地端时间的时间；
    \s 操作系统的名称；
    \v 操作系统的版本
{% endhighlight %}

其中 motd 为 `Message Of ToDay` 的简称，也就是布告栏信息，每次用户登陆的时候都会将该信息显示在终端，可以添加些维护信息之类的。不过缺点是，如果是图形界面，会看不到这些信息。

一个对于 ROOT 用户常见的提示如下：

{% highlight text %}
We trust you have received the usual lecture from the local System Administrator.
It usually boils down to these three things:
	#1) Respect the privacy of others.
	#2) Think before you type.
	#3) With great power comes great responsibility.
{% endhighlight %}

### 系统用户使用

当排查系统用户是否有访问权限时，可以通过如下方法：

{% highlight text %}
----- 如果有次要用户组，会切换所有次要用户组
# su - monitor -s /bin/bash -c 'cat /your/file/path'

----- 即使有多个次要用户组，也只会切换到指定用户组
# su - monitor -g monitor -s /bin/bash -c 'cat /tmp/mysql.test'
{% endhighlight %}

### 过期设置

通过如下方法设置过期条件。

{% highlight text %}
# useradd USER -e 01/28/12               # 创建用户时指定过期条件
# grep EXPIRE /etc/default/useradd       # 或者修改模板对应的默认参数
# useradd -D -e 01/19/12                 # 修改默认新建帐户过期时间
# useradd -D | grep EXPIR                # 查看
# chage -l USER                          # 查看用户的过期时间
# usermod -e 01/28/12 USER               # 修改用户属性
# chage -E 01/28/12 USER                 # 调整账户过期时间
{% endhighlight %}

### 锁用户

可以手动锁用户。

{% highlight text %}
----- 锁用户，两者功能相同
passwd -l username
usermod -L username

----- 解锁用户
passwd -u username
usermod -U username
{% endhighlight %}

### 密码过期

当密码过期之后，会按照上述的 PAM 配置重新设置，如果想保持源密码不变，可以先通过 `>/etc/security/opasswd` 清空之前的记录。


{% highlight text %}
{% endhighlight %}
