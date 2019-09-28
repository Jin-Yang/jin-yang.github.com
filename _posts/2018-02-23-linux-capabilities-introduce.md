---
title: Linux 系统用户
layout: post
comments: true
language: chinese
category: [linux]
keywords: linux,system user
description: 在 Linux 系统中，很多操作是需要 root 用户权限才能操作的，常见的包括 chown、使用 Raw Sokcet (ping) 等，如果使用 sudo 就会导致配置管理和权限控制比较麻烦。这里简单介绍一下 Linux 中的解决方案。
---

在 Linux 系统中，很多操作是需要 root 用户权限才能操作的，常见的包括 chown、使用 Raw Sokcet (ping) 等，如果使用 sudo 就会导致配置管理和权限控制比较麻烦。

其中一个解决方案是，类似于 passwd ，对一个 owner 为 root 的可执行文件可以增加粘滞位 (Set User ID on execution, SUID)，也就是 `chmod +s` 。

这样在运行的时候使用的就是 root 权限，带来的问题就是会导致其运行的权限过高，在某种程度上增大了安全攻击面，有些平台上 ping 也是采用上述的粘滞位机制。

为了只给这个程序开所需要的权限，Linux 提供了一套 capabilities 机制，这里简单介绍。

<!-- more -->

## 问题简述

通常使用很多的程序会使用一个非特权的系统帐号，假设有监控系统采集服务器的指标，例如通过如下用户添加 `monitor:monitor` 用户以及用户组。

{% highlight bash %}
#! /bin/sh

# NOTE: Command 'id -g monitor' will failed on some platform.
if ! (grep -E '^monitor\>:' /etc/group >/dev/null 2>&1) ; then
        echo "Group 'monitor' does not exist, try to create one"
        /usr/sbin/groupadd -g 68 -o -r monitor >/dev/null 2>&1 || :
else
        echo "Group 'monitor' already exists"
        exit 1
#        gid=`id -g monitor`
#        if [ "x${gid}" != "x68" ]; then
#                echo "Change group id(${gid}) to 68"
#                /usr/sbin/groupmod -g 68 monitor
#        fi
fi

if ! (grep -E '^monitor\>:' /etc/passwd >/dev/null 2>&1) ; then
        echo "User 'monitor' does not exist, try to create one"
        /usr/sbin/useradd -M -g monitor -o -r -d /your/home/path -s /bin/false \
                -c "Monitor Agent" -u 68 monitor >/dev/null 2>&1 || :
else
        echo "User 'monitor' already exists"
        exit 2
#        uid=`id -u monitor`
#        if [ "x${uid}" != "x68" ]; then
#                echo "Change user id(${uid}) to 68"
#                /usr/sbin/usermod -u 68 monitor
#        fi
fi
exit 0
{% endhighlight %}

如上添加的是系统用户，很多目录是没有权限，可以通过如下方式查看是否有权限。

{% highlight text %}
----- 查看是否有访问权限
# su - monitor -s /bin/bash -c "ls -alh /var/lib/mysql/mysql.sock"

----- 查看是否有写权限
# su - monitor -s /bin/bash -c "touch /var/lib/mysql/mysql.sock"
{% endhighlight %}

访问其它用户的文件时需要保证：A) 保证文件对其它用户是可读写的；B) 整个路径目录对其它用户是可执行的。

如果在通过上述的方式检查时发现没有权限，那么需要通过如下命令修改 monitor 用户的属组，同时要保证 相关的文件以及目录有该属组的访问权限 。

如下示例仍以 MySQL 用户为例。

{% highlight text %}
----- 设置monitor用户的默认属组
# usermod -g mysql monitor

----- 检查是否修改成功，注意后面的属组
$ id monitor
uid=68(monitor) gid=995(mysql) groups=995(mysql)
{% endhighlight %}

目前这种方案只能满足一个组件的权限设置，因为主要属组只能设置一个。

### Sudoer

简单来说，需要修改 `/etc/sudoer` 配置文件。

{% highlight text %}
用户 登录的主机=(可以变换的身份) 可以执行的命令
monitor ALL=(root) NOPASSWD: /usr/sbin/groupadd

其中 ALL 也可以通过 Host_Alias 指定具体的主机信息，包括了主机名、IP地址、网络掩码等
Host_Alias HT01=localhost,etc01,10.0.0.4,255.255.255.0,192.168.1.0/24
{% endhighlight %}

<!--
/* /usr/sbin/groupdel someuser */
//execl("/usr/sbin/groupadd", "groupadd", "-r", "someuser", NULL);
execl("/usr/bin/sudo", "sudo", "/usr/sbin/groupadd", "-r", "someuser", NULL);
-->

## Capabilities 简介

该机制是在 Linux 2.2 之后引入的，它将 root 用户的权限细分为不同的领域，可以分别启用或禁用；从而，在实际进行特权操作时，如果 euid 不是 root，便会检查是否具有该特权操作所对应的 capabilities，并以此为依据，决定是否可以执行特权操作。

一个完整的能力机制需要满足以下三个条件:

1. 对进程的所有特权操作，Linux 内核必须检查该进程该操作的特权位是否使能。
2. Linux 内核必须提供系统调用，允许进程能力的修改与恢复。
3. 文件系统必须支持能力机制可以附加到一个可执行文件上，但文件运行时，将其能力附加到进程当中。

到 Linux 内核版本 2.6.24 之前满足 1、2 两个条件，之后满足上述 3 个条件；也就是说，完整的 capabilities 支持是在 2.6.24 版本以后支持的。

注意，capabilities 是细分到线程的，每个线程都有自己对应的 capabilities 。

### 设置Capabilities

事实上 Linux 本身对权限的检查就是基于 capabilities 的，root 用户有全部的 capabilities，所以啥都能干。

常用程序有：A) `getcap` 查看程序文件所具有的能力；B) `setcap` 设置程序文件的能力；C) `getpcaps` 获得进程所具有的能力。

最简单的例子 `ping` ，正常来说需要 `CAP_NET_RAW` 权限，可以通过如下的方式解决，常见的操作如下：

{% highlight text %}
----- 通过设置SUID获取root所有权限
$ chown root:root /bin/ping
$ chmod u+s /bin/ping
----- 也可以设置SGID
$ chmod g+s /bin/ping

----- 对于ping增加Raw Socket权限
$ setcap cap_net_raw=+ep ping
参数：
   cap_net_raw 对应设置capability的名字；
   mode 也就是等号后面的内容，+ 表示启用，- 表示禁用；
      e 是否激活
      p 是否允许进程设置
      i 子进程是否继承

----- 同样以ping为例，可以通过如下方法查看其具有的Capability权限
$ getcap /bin/ping
{% endhighlight %}

这样普通用户执行这个 ping 命令，也可以正常使用 Raw Socket 这个权限了。

内核通过 `setxattr()` 系统调用执行，也就是为文件加上 `security.capability` 扩展属性。

#### 获取和设置

系统调用 `man 2 capget` 和 `man 2 capset` 可被用于获取和设置线程自身的 capabilities；此外，也可以使用 libcap 中提供的接口 `man 3 cap_get_proc` 和 `man 3 cap_set_proc` 。

### 线程相关

权限控制的最小粒度是线程，每个线程有三个与 Capabilities 相关的位图，分别为：

* Permitted，定义线程所能够拥有的特权的上限，如果需要的特权不在该集合中，是不能进行设置的；如果一个进程在该集合中丢失一个能力，除非特权用户再次赋予权限，否则无论如何不能再次获取该能力。
* Inheritable，执行 fork() 运行其它进程时允许被继承的权限；
* Effective，当前允许执行的特权。

对应进程描述符 `struct task_struct` 中的 `cap_effective`、`cap_inheritable`、`cap_permitted`，可以通过 `/proc/PID/status` 来查看进程的能力。

从 2.6.24 开始，Linux 内核可以给可执行文件赋予能力，同样包含三个能力集：

* Permitted 当可执行文件执行时自动附加到进程中，会忽略 Inhertiable 集合；
* Inheritable 与进程 Inheritable 集合做与操作，决定执行 execve 后新进程的 Permitted 集合；
* Effective  实际不是集合，而是一个单独的位，用来决定进程的 Effective 集合。

也就是说，Linux 系统中的能力分为两部分：A) 进程能力；B) 文件能力。Linux 内核最终检查的是进程能力中的 Effective，文件能力和进程能力中的其他部分用来完整能力继承、限制等方面的内容。

## 能力继承

也就是通过 `fork()` 新建进程时，子进程的能力，可以通过 `man 7 capabilities` 查看规则。

{% highlight text %}
P'(ambient) = (file is privileged) ? 0 : P(ambient)
P'(permitted) = (P(inheritable) & F(inheritable)) |
		(F(permitted) & cap_bset) | P'(ambient)
P'(effective) = F(effective) ? P'(permitted) : P'(ambient)
P'(inheritable) = P(inheritable)    [i.e., unchanged]
{% endhighlight %}

<!--
https://blog.ploetzli.ch/2014/understanding-linux-capabilities/
-->

![linux capabilities]({{ site.url }}/images/linux/capabilities-process-introduce.png "linux capabilities"){: .pull-center width="70%" }

带有 `'` 表示新的进程，这里主要讨论后三者的能力。


### 测试示例

在 CentOS 中，有两个相关的包 `libcap-ng` 和 `libcap` ，一般使用的是后者，那么在使用时需要安装对应的开发版本，也就是需要通过 `yum install libcap-devel` 安装相关的头文件等。

如下两个分别为 `father.c` 以及 `child.c` 。

{% highlight c %}
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/capability.h>

void list_capability()
{
	struct __user_cap_header_struct cap_header_data;
	cap_user_header_t cap_header = &cap_header_data;

	struct __user_cap_data_struct cap_data_data;
	cap_user_data_t cap_data = &cap_data_data;

	cap_header->pid = getpid();
	cap_header->version = _LINUX_CAPABILITY_VERSION_1;

	if (capget(cap_header, cap_data) < 0) {
		perror("Failed capget");
		exit(1);
	}

	printf("Capability data: permitted=0x%x, effective=0x%x, inheritable=0x%x\n",
		cap_data->permitted, cap_data->effective, cap_data->inheritable);
}

int main(void)
{
	cap_t caps = cap_init();
	cap_value_t capList[2] = { CAP_DAC_OVERRIDE, CAP_SYS_TIME };

	//cap_set_flag(caps, CAP_EFFECTIVE, 2, capList, CAP_SET);
	cap_set_flag(caps, CAP_INHERITABLE, 2, capList, CAP_SET);
	cap_set_flag(caps, CAP_PERMITTED, 2, capList, CAP_SET);

	if(cap_set_proc(caps)) {
		perror("cap_set_proc");
		exit(1);
	}

	list_capability();

	execl("child", NULL);

	sleep(1000);
}
{% endhighlight %}

{% highlight c %}
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <linux/capability.h>

void list_capability()
{
	struct __user_cap_header_struct cap_header_data;
	cap_user_header_t cap_header = &cap_header_data;

	struct __user_cap_data_struct cap_data_data;
	cap_user_data_t cap_data = &cap_data_data;

	cap_header->pid = getpid();
	cap_header->version = _LINUX_CAPABILITY_VERSION_1;

	if (capget(cap_header, cap_data) < 0) {
		perror("Failed capget");
		exit(1);
	}

	printf("Child capability data: permitted=0x%x, effective=0x%x, inheritable=0x%x\n",
		cap_data->permitted, cap_data->effective, cap_data->inheritable);
}

int main(void)
{
	list_capability();
	sleep(1000);
}
{% endhighlight %}

{% highlight text %}
$ gcc child.c -o child
$ gcc father.c -o father -lcap
# setcap cap_dac_override,cap_sys_time+ei child
# setcap cap_dac_override,cap_sys_time+ip father
{% endhighlight %}

单独执行时，child 可执行文件由 EI 的能力，而调用执行 child 的终端没有任何能力，那么对应公式(cap_bset默认全1)：

{% highlight text %}
P'(permitted) = (P(inheritable) & F(inheritable)) | (F(permitted) & cap_bset)
              = (0x0 & 0x2000002) | (0x0 & 全1) = 0x00
P'(effective) = F(effective) ? P'(permitted) : 0
              = 1 ? P'(permitted) : 0 = P'(permitted) = 0x00
P'(inheritable) = P(inheritable) = 0x00
{% endhighlight %}

通过 father 调用执行时，child 文件有 EI 的能力，father 文件有 EP 能力，那么套用公式：

{% highlight text %}
P'(permitted) = (P(inheritable) & F(inheritable)) | (F(permitted) & cap_bset)
              = (0x2000002 & 0x2000002) | (0x2000002 & 全1) = 0x2000002
P'(effective) = F(effective) ? P'(permitted) : 0
              = 1 ? P'(permitted) : 0 = P'(permitted) = 0x2000002
P'(inheritable) = P(inheritable) = 0x2000002
{% endhighlight %}

上述单独运行 child 可执行程序，其进程没有任何能力；但是有 father 进程来启动运行 child 可执行程序，其进程则有相应的能力。


## 示例程序

对于 CentOS ，需要安装 libcap-devel 开发包才可以，当前的 Linux 系统中共有 37 项特权，可以从 `/usr/include/linux/capability.h` 文件中查看，编译使用 `-lcap` 。

**注意** `DAC_OVERRIDE` 是 `DAC_READ_SEARCH` 的超集。

{% highlight c %}
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/capability.h>

#define ASIZE(arr)  (sizeof(arr)/sizeof(*arr))

int list_caps(void)
{
#if 0
	cap_t caps;
	//caps = cap_get_pid(pid); /* get the process capabilities */
	caps = cap_get_proc();
	if (caps == NULL) {
		fprintf(stderr, "[ERROR] Failed to get process capability, %s\n",
		strerror(errno));
		return -1;
	}
	fprintf(stdout, "[INFO] Process %d was given capabilities %s\n",
		(int)getpid(), cap_to_text(caps, NULL));
	cap_free(caps);
#else
	struct __user_cap_header_struct cap_header_data;
	cap_user_header_t cap_header = &cap_header_data;

	struct __user_cap_data_struct cap_data_data;
	cap_user_data_t cap_data = &cap_data_data;

	cap_header->pid = getpid();
	cap_header->version = _LINUX_CAPABILITY_VERSION_1;

	if (capget(cap_header, cap_data) < 0) {
		fprintf(stderr, "[ERROR] Failed to get process cap, %s\n",
			strerror(errno));
		return -1;
	}
	fprintf(stdout, "[INFO] Capabilities data: permitted=0x%x effective=0x%x inheritable=0x%x\n",
		cap_data->permitted, cap_data->effective,cap_data->inheritable);
#endif

	return 0;
}

int main(void)
{
	cap_t caps;
	cap_value_t caplist[] = {
		CAP_NET_RAW, CAP_NET_BIND_SERVICE, CAP_SETUID, CAP_SETGID, CAP_SETPCAP
	};

	if (list_caps() < 0)
		exit(1);

	//caps = cap_get_proc();
	if ((caps = cap_init()) == NULL) {
		fprintf(stderr, "[ERROR] Failed to init capability, %s\n", strerror(errno));
		exit(1);
	}
	if (cap_set_flag(caps, CAP_EFFECTIVE, ASIZE(caplist), caplist, CAP_SET) ||
	    cap_set_flag(caps, CAP_INHERITABLE, ASIZE(caplist), caplist, CAP_SET) ||
	    cap_set_flag(caps, CAP_PERMITTED, ASIZE(caplist), caplist, CAP_SET)) {
		fprintf(stderr, "[ERROR] Failed to set flag, %s\n", strerror(errno));
		cap_free(caps);
		exit(1);
	}
	if (cap_set_proc(caps) < 0) {
		fprintf(stderr, "[ERROR] Failed to set capability, %s\n", strerror(errno));
		cap_free(caps);
		exit(1);
	}

	if (list_caps() < 0) {
		cap_free(caps);
		exit(1);
	}

	/* resetting caps storage */
	if (cap_clear(caps) < 0) {
		fprintf(stderr, "[ERROR] Failed to clear capability, %s\n", strerror(errno));
		cap_free(caps);
		exit(1);
	}
	if (cap_set_proc(caps) < 0) {
		fprintf(stderr, "[ERROR] Failed to set capability, %s\n", strerror(errno));
		cap_free(caps);
		exit(1);
	}

	if (list_caps() < 0) {
		cap_free(caps);
		exit(1);
	}

	cap_free(caps);
	return 0;
}
{% endhighlight %}


如下是一个测试程序，确保在切换用户时保留能力： 1) 通过 `prctl(PR_SET_KEEPCAPS, 1L);` 保留能力；2) 通过 `cap_set_proc()` 重新设置 Effective 和 Permitted 的能力。

在切换之前，需要保证有 `CAP_SETUID`、`CAP_SETGID` 的权限即可。

<!--
https://stackoverflow.com/questions/12141420/losing-capabilities-after-setuid?utm_medium=organic&utm_source=google_rich_qa&utm_campaign=google_rich_qa
-->

{% highlight c %}
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/capability.h>

#define ASIZE(arr)  (sizeof(arr)/sizeof(*arr))

int list_caps(void)
{
	struct __user_cap_header_struct cap_header_data;
	cap_user_header_t cap_header = &cap_header_data;

	struct __user_cap_data_struct cap_data_data;
	cap_user_data_t cap_data = &cap_data_data;

	cap_header->pid = getpid();
	cap_header->version = _LINUX_CAPABILITY_VERSION_1;

	if (capget(cap_header, cap_data) < 0) {
		fprintf(stderr, "[ERROR] Failed to get process cap, %s\n",
			strerror(errno));
		return -1;
	}
	fprintf(stdout, "[INFO] Capabilities data: permitted=0x%x effective=0x%x inheritable=0x%x\n",
		cap_data->permitted, cap_data->effective,cap_data->inheritable);

	return 0;
}

void test(void)
{
	int fd, rc;
	char buffer[1024];

	//fd = open("/tmp/user/group/test/read.txt", O_RDONLY);
	errno = 0;
	fd = open("/tmp/read.txt", O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "Open read file error: %s\n", strerror(errno));
		return;
	}
	rc = read(fd, buffer, sizeof(buffer));
	if (rc < 0) {
		fprintf(stderr, "Read file error: %s\n", strerror(errno));
		return;
	}
	close(fd);

	buffer[rc] = 0;
	fprintf(stdout, "Got content: %s", buffer);

}

int main(void)
{
	const char * const user = "monitor";
	struct passwd *pwd;

	if (getuid() != 0) {
		fprintf(stderr, "Should run as root\n");
		return -1;
	}

	pwd = getpwnam(user);
	if (pwd == NULL) {
		fprintf(stderr, "User '%s'does not exist\n", user);
		return -1;
	}

	pid_t pid;
	pid = fork();
	if (pid < 0) {
		fprintf(stderr, "Failed to fork child, %s\n", strerror(errno));
		return -1;
	} else if (pid == 0) { /* child */
		fprintf(stdout, "[INFO] Child PID is %d\n", getpid());
		if (list_caps() < 0)
			exit(1);

		/* Set before change the effective user */
		if (prctl(PR_SET_KEEPCAPS, 1L)) {
			fprintf(stderr, "Failed to set keep caps flag, %s\n", strerror(errno));
			exit(1);
		}

		if (setgid(pwd->pw_gid) < 0) {
			fprintf(stderr, "Cannot setgid to %s: %s\n", user, strerror(errno));
			return -1;
		}

		if (setuid(pwd->pw_uid) < 0) {
			fprintf(stderr, "Cannot setuid to %s: %s\n", user, strerror(errno));
			return -1;
		}
		fprintf(stdout, "[INFO] Change to user '%s', uid/gid=%d/%d\n",
			user, pwd->pw_gid, pwd->pw_uid);

		if (list_caps() < 0)
			exit(1);

		cap_t caps;
		//CAP_DAC_OVERRIDE, CAP_SETUID, CAP_SETGID, CAP_NET_RAW, CAP_SETPCAP
		cap_value_t caplist[] = {
			CAP_DAC_OVERRIDE
		};
		if ((caps = cap_init()) == NULL) {
			fprintf(stderr, "[ERROR] Failed to init capability, %s\n", strerror(errno));
			exit(1);
		}

		if (cap_set_flag(caps, CAP_EFFECTIVE, ASIZE(caplist), caplist, CAP_SET) ||
		    cap_set_flag(caps, CAP_PERMITTED, ASIZE(caplist), caplist, CAP_SET)) {
			fprintf(stderr, "[ERROR] Failed to set flag, %s\n", strerror(errno));
			cap_free(caps);
			exit(1);
		}

		if (cap_set_proc(caps) < 0) {
			fprintf(stderr, "[ERROR] Failed to set capability, %s\n", strerror(errno));
			cap_free(caps);
			exit(1);
		}

		if (list_caps() < 0) {
			cap_free(caps);
			exit(1);
		}

		test();

		exit(0);
	}
	/* parent */

	sleep(1000);
	return 0;
}
{% endhighlight %}

在测试时遇到一个很奇葩的问题，当通过系统调用 `open("/tmp/read.txt", O_RDONLY)` 打开文件时，如果文件属主为 root，需要保证文件 group 的权限为可读，而非 others 文件权限为可读；而非 root 属主的文件，需要 others 文件权限为可读。



<!--
下表列出了一些常见的特权操作及其对应的 capability：

改变文件的所属者(chown()) CAP_CHOWN
向进程发送信号(kill(), signal()) CAP_KILL
改变进程的uid(setuid(), setreuid(), setresuid()等) CAP_SETUID
trace进程(ptrace()) CAP_SYS_PTRACE
设置系统时间(settimeofday(), stime()等) CAP_SYS_TIME





当然，Permitted集合默认是不能增加新的capabilities的，除非CAP_SETPCAP在Effective集合中。

如果要查看线程的capabilities，可以通过/proc/<PID>/task/<TID>/status文件，三种集合分别对应于CapPrm, CapInh和CapEff。但这种的显示结果是数值，不适合人类阅读。为此，可使用包libcap中的命令getpcaps <PID>获取该进程的主线程的capabilities。

类似的，如果要查看和设置文件的capabilities，可以使用命令getcap或者setcap。
-->

<!--
运行exec后capabilities的变化
上面介绍了线程和文件的capabilities，可能会觉得有些抽象难懂。下面将使用具体的计算公式，来说明执行exec后capabilities是如何确定的。

我们使用P代表执行exec前的capabilities，P’代表执行exec后的capabilities，F代表exec执行的文件的capabilities。那么：

P’(Permitted) = (P(Inheritable) & F(Inheritable)) | (F(Permitted) & cap_bset)

P’(Effective) = F(Effective) ? P’(Permitted) : 0

P’(Inheritable) = P(Inheritable)

其中的cap_bset是capability bounding set。通过与文件的Permitted集合计算交集，可进一步限制某些capabilities的获取，从而降低了风险。

而正如介绍文件的Effective bit时所说，文件可以将其Effective bit关闭。由此，在通过exec执行该文件后，实际的Effective集合为空集。随后，在需要进行特权操作时，可再将Permitted集合中的capabilities加入Effective集合中。





mkdir -p /tmp/user/group/test
chmod 700 -R /tmp/user
echo "just for test" > /tmp/user/group/test/read.txt
chmod 600 /tmp/user/group/test/read.txt
chown -R mysql:mysql /tmp/user
-->



### 常见命令

{% highlight text %}
----- 查看ping的能力
$ getcap /bin/ping
/bin/ping = cap_net_raw+ep

----- 删除文件具有的能力
$ setcap -r /bin/ping

----- 获取保存在文件扩展编码中的内容
$ getfattr -d -m "^security\\." /bin/ping
# file: bin/ping
security.capability=0sAQAAAgAgAAAAAAAAAAAAAAAAAAA=

----- 找到setuid-root或者setgid-root的文件 find / -perm /u=s
$ find /usr/bin /usr/lib -perm /4000 -user root
$ find /usr/bin /usr/lib -perm /2000 -group root
{% endhighlight %}


## 权限管理

目前的场景为，启动一个有限权限的常驻进程，然后执行其它的命令，包括脚本。

### 主进程

简单来说，实现方案为，主进程在启动时会继承部分权限(限制常驻进程权限)，然后切换到 `monitor` 用户以非特权方式运行，切换后默认会失去所有权限(Eff)，需要重新再设置一次。

其中继承的权限包括了：

1. `CAP_DAC_OVERRIDE` 允许进程对所有的路径进行读写。
2. `CAP_SETUID`, `CAP_SETGID` 允许切换用户，例如再次切换到root

注意，在切换用户的时候只能设置已经限制后的权限。

### 子进程

这里直接执行一个脚本，判断其是否有符合的权限，执行的方式是使用 `bash -c SCRIPTS` 命令，可以通过 `su - monitor "bash -c SCRIPTS"` 进行测试。

1. 主进程从 `root` 继承部分权限并切换到 `monitor` 用户，此时不会继承 `effective` 权限，需要重新设置。
2. 通过 `fork+setgid/setuid+exec` 执行子进程，所执行的命令需要确保对应的用户有权限。

因为 Linux 对 root 和 非root 的处理方式不一样，简单来说前者切换完用户之后同时会获取到 Prm、Eff 的权限，也就是说只要是 root 基本上就可以为所欲为了，无论之前有没有做过限制。

而从 root 切换了 非root 用户之后，所有的权限默认都会取消，除非手动再次设置。

注意，实测发现，在从 root 切换到 非root 后 Prm 权限会被取消，而从 非root 切换到 非root 时权限会保持不变。

#### 前提条件

这里假设直接执行二进制文件，而非脚本，也就是说只执行了一次 `execXXX` 。

如上，当从 非root 切换到 非root 之后，实际上权限会继承，但是当通过 `execXXX` 执行时，默认其对应的 Prm 权限仍然会被取消。

假设，需要添加 `CAP_DAC_OVERRIDE` 权限，就应该要确保如下的内容。

* 在执行 `execXXX` 前的进程，需要确保在 Prm 中有 `CAP_DAC_OVERRIDE` 功能，这样才能添加到 `Inh` 中，子进程才能继承。
* 通过 `cap_set_flag()` 以及 `cap_set_proc()` 接口设置 `Inh` 中的 `CAP_DAC_OVERRIDE` 功能，这样通过 `exec` 执行后的子进程是有 `Inh` 权限的。
* 将对应的可执行文件设置 `Inh` `Eff` 权限，其中 `Inh` 会设置 `CapPrm` 也就是本进程允许的最大权限，而 `Eff` 会自动设置生效的 `CapEff` 也就是真正的权限。

简单来说，需要确保 fork 的进程权限在 `Inh` 中，才有可能在 `exec` 中继承；当可执行文件设置了 `Inh` 和 `Eff` 之后，才会自动继承。

例如，测试的可执行二进制文件是 `/tmp/foobar`，那么可以通过如下方式设置。

{% highlight text %}
# setcap cap_dac_override=ei /tmp/foobar
{% endhighlight %}

#### 脚本执行

脚本的话会涉及到类似 `bash` `python` `perl` 的解析器，同时包含了执行的命令，按照上述的理论，就需要保证整个链路上的权限，也就是说要保证解析器、执行命令进行了上述配置。

{% highlight text %}
# setcap cap_dac_override=ei /tmp/foobar/bash
# setcap cap_dac_override=ei /tmp/foobar/foobar
{% endhighlight %}

#### 总结

1. 从 root 切换到 非root 会自动将 `Prm` 清空，而从 非root 切换到 root 会自动保留原有的权限。
2. 执行 `execXXX()` 函数时，如果不设置文件的权限，那么会自动清楚。

## 参考

通过 `man 7 capabilities` 查看所有可用的 capabilities，而通过 `man 3 cap_from_text` 可以看到关于 capability mode 的表达式说明。

<!--
http://www.cnblogs.com/iamfy/archive/2012/09/20/2694977.html
https://blog.ploetzli.ch/2014/understanding-linux-capabilities/



strace 可以跟踪到一个进程产生的系统调用，包括了参数、返回值、执行时间；常用参数包括了：

-p 跟踪指定的进程
-o filename 默认strace将结果输出到stdout。通过-o可以将输出写入到filename文件中
-ff 常与-o选项一起使用，不同进程(子进程)产生的系统调用输出到filename.PID文件
-r 打印每一个系统调用的相对时间
-t 在输出中的每一行前加上时间信息。
-tt 时间确定到微秒级。还可以使用-ttt打印相对时间
-s 指定每一行输出字符串的长度,默认是32。
-c 统计每种系统调用所执行的时间，调用次数，出错次数。
-e expr 输出过滤器，通过表达式，可以过滤出掉你不想要输出

常用示例：
----- 同时跟踪子进程的调用，每个进程以PID结尾
$ strace -ff -o trace.out your-program

-->

{% highlight text %}
{% endhighlight %}
