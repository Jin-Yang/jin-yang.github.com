---
title: 使用 C API 执行 Linux 用户相关操作
layout: post
comments: true
language: chinese
tag: [Linux,DevOps,Program]
keywords: linux,用户管理
description: 通过 glibc 提供的 API 可以获取用户信息、切换用户等操作，其中又有很多需要注意的事项，这里会进行简单介绍。
---

通过 glibc 提供的 API 可以获取用户信息、切换用户等操作，在使用的时候有很多注意事项，例如可能会遇到即使用户存在仍读取失败，密码的密文是如何生成的等等。

这里会进行简单介绍一些常见的问题以及如何规避。

<!-- more -->

## 简介

glibc 中提供了很多与用户相关的接口，最常见的是通过 `getpwnam()` 获取用户信息。

{% highlight c %}
#include <pwd.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

int main(void)
{
	struct passwd * pw;

	errno = 0;
	pw = getpwnam("monitor"); // 注意，入参不能为NULL
	if (pw == NULL) {
		if (errno == 0)
			printf("user is not exist\n");
		else
			printf("get user failed, %d:%s.", errno, strerror(errno));
		return -1;
	}

	printf("Password information:\n");
	printf("    pw->pw_name = %s\n", pw->pw_name);
	printf("    pw->pw_passwd = %s\n", pw->pw_passwd);
	printf("    pw->pw_uid = %d\n", pw->pw_uid);
	printf("    pw->pw_gid = %d\n", pw->pw_gid);
	printf("    pw->pw_gecos = %s\n", pw->pw_gecos);
	printf("    pw->pw_dir = %s\n", pw->pw_dir);
	printf("    pw->pw_shell = %s\n", pw->pw_shell);

	if (setgid(pw->pw_gid) == -1) {
		fprintf(stderr, "Failed to set group id(%d): %s\n",
				pw->pw_gid, strerror(errno));
	}

	if (setuid(pw->pw_uid) == -1) {
		fprintf(stderr, "Failed to set user id(%d): %s\n",
				pw->pw_uid, strerror(errno));
	}

	fprintf(stdout, "Current user/group info: UID=%d, EUID=%d, GID=%d, EGID=%d\n",
				getuid(), geteuid(), getgid(), getegid());

	int fd = open("/tmp/mysql.test", 0);
	if (fd < 0) {
		fprintf(stderr, "Failed to openfile: %s\n", strerror(errno));
	}
	if (fd > 0)
		close(fd);

	return 0;
}
{% endhighlight %}

<!--
Linux 中进程在运行的时候，会有多个 UID 信息，可以通过 `/proc/PID/status` 查看当前进程的 UID 信息，包括了四列，分别为：

1. RUID Real 实际用户，也就是指进程的执行者；
2. EUID Effective 有效用户，进程执行时的访问权限；
3. SUID Saved 保存设置用户，作为 EUID 的副本，可以再次恢复到 EUID；
4. FSUID FileSystem 一般与 EUID 是相同的。

在如上的示例中，当通过 `setuid()` 接口切换用户时，对应的所有 ID 都会切换，那么这几个 UID 的用途是什么。

其中 FSUID 用于 NFS 使用，不过不太清楚如何使用，这里暂时忽略。
-->

{% include ads_content01.html %}

### UID VS. EUID

切换用户时，有两种方式，A) 通过 `setuid()` 永久切换；B) 利用 `seteuid()` 临时切换。

前者类似上面的示例，所有的 UID 参数都会被修改，不过此时切换之后无法切换回来，而 `seteuid()` 可以做到。

{% highlight c %}
#include <pwd.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

int main(void)
{
        char *buff;
        int size, rc, ouid;
        struct passwd mpwd, upwd, *result;
        char cmd[1024];

        snprintf(cmd, sizeof(cmd), "grep -rne '^Uid:' /proc/%d/status", getpid());

        size = sysconf(_SC_GETPW_R_SIZE_MAX);
        if (size < 0) {
                fprintf(stderr, "get passwd size failed.\n");
                size = 4096;
        }

        buff = (char *)malloc(size);
        if (buff == NULL) {
                fprintf(stderr, "malloc buffer(%d) failed, out of memory.\n", size);
                return -1;
        }

        rc = getpwnam_r("monitor", &mpwd, buff, size, &result);
        if (result == NULL) {
                if (rc == 0)
                        fprintf(stderr, "user 'monitor' doesn't exists.\n");
                else
                        fprintf(stderr, "get user 'monitor' info failed, %d:%s.",
                                        rc, strerror(rc));
                free(buff);
                return -1;
        }

        rc = getpwnam_r("foobar", &upwd, buff, size, &result);
        if (result == NULL) {
                if (rc == 0)
                        fprintf(stderr, "user 'monitor' doesn't exists.\n");
                else
                        fprintf(stderr, "get user 'monitor' info failed, %d:%s.",
                                        rc, strerror(rc));
                free(buff);
                return -1;
        }

        ouid = getuid();
        fprintf(stdout, "Monitor UID %d, Fooobar UID %d\n", mpwd.pw_uid, upwd.pw_uid);
        fprintf(stdout, "Before  RUID=%d, EUID=%d\n", getuid(), geteuid());
        system(cmd);

        if (seteuid(mpwd.pw_uid) == -1) {
                fprintf(stderr, "Set monitor user id(%d) failed, %d:%s.\n",
                                mpwd.pw_uid, errno, strerror(errno));
        }
        fprintf(stdout, "Monitor RUID=%d, EUID=%d\n", getuid(), geteuid());
        system(cmd);

        if (setuid(ouid) == -1) {
                fprintf(stderr, "Restore user id(%d) failed, %d:%s.\n",
                                ouid, errno, strerror(errno));
        }
        fprintf(stdout, "Restore RUID=%d, EUID=%d\n", getuid(), geteuid());
        system(cmd);

        if (setuid(upwd.pw_uid) == -1) {
                fprintf(stderr, "Set fooobar user id(%d) failed, %d:%s.\n",
                                upwd.pw_uid, errno, strerror(errno));
        }
        fprintf(stdout, "Fooobar RUID=%d, EUID=%d\n", getuid(), geteuid());
        system(cmd);

        return 0;
}
{% endhighlight %}

执行的结果如下。

{% highlight text %}
Monitor UID 1005, Fooobar UID 1006
Before  RUID=0, EUID=0
9:Uid:  0       0       0       0
Monitor RUID=0, EUID=1005
9:Uid:  0       1005    0       1005
Restore RUID=0, EUID=0
9:Uid:  0       0       0       0
Fooobar RUID=1006, EUID=1006
9:Uid:  1006    1006    1006    1006
{% endhighlight %}

注意，没有 C 接口获取 `SUID` 和 `FSUID`，需要直接查看 `/proc/<PID>/status` 文件。

## 密码加密方式

Linux 会通过 `crypt(3)` 函数完成用户的密码加密，在 CentOS 8 中，其实现对应了 `libxcrypt` 库，该库包含了很多单向加密的哈希函数，例如 DES、SHA、Blowfish 等等。

用户真正加密后的密码保存在 `/etc/shadow` 文件中，对应了第二段，其保存的格式为 `$id$salt$encrypted`，其中 `id` 标识了加密方式，`1 MD5`、`2a Blowfish`、`5 SHA256`、`6 SHA512` 等，而 `salt` 通常为 12 字节。

接下来看看 `/etc/shadow` 文件的内容：

{% highlight text %}
root:$1$Bg1H/4mz$X89TqH7tpi9dX1B9j5YsF.:14838:0:99999:7:::
{% endhighlight %}

如果密码字符串为 `*`，则表示是系统用户不能被登入；如果字符串为 `!!`，则表示用户被禁用，不能登陆；如果字符串为空，则表示没有密码。

可以通过 `passwd -d UserName` 命令清空一个用户的口令密码。

### 密码解析

如果要分析加密算法，可以直接查看 `passwd` 命令的实现，实际上，就是用明文密码和 salt (动态生成的随机字符串)，然后通过 `crypt()` 函数完成加密，可以通过如下程序验证。

{% highlight c %}
#define _XOPEN_SOURCE
#include <stdio.h>
#include <string.h>
#include <shadow.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
	if(argc < 3) {
		fprintf(stderr, "Usage: %s UserName Password\n", *argv);
		exit(EXIT_FAILURE);
	}
	char *user = argv[1];

	if (geteuid() != 0) {
		fprintf(stderr, "Must be root\n");
		exit(EXIT_FAILURE);
	}

	struct spwd *shd= getspnam(user);
	if (shd == NULL) {
		fprintf(stderr, "User \"%s\" doesn't exist\n", user);
		exit(EXIT_FAILURE);
	}

	char encrypted[128], *ptr, *salt;
	strncpy(encrypted, shd->sp_pwdp, sizeof(encrypted));

	salt = encrypted;
	ptr = strrchr(encrypted, '$');
	if (ptr == NULL)
		exit(EXIT_FAILURE);
	ptr++;
	*ptr = 0;

	printf("salt: %s\n         crypt: %s\n", salt, crypt(argv[2], salt));
	printf("shadowd passwd: %s\n", shd->sp_pwdp);

	return 0;
}
{% endhighlight %}

然后通过如下命令编译并测试。

{% highlight text %}
$ gcc passwd.c -Wall -lcrypt -o passwd
$ ./passwd username yourpassword
{% endhighlight %}

这也就意味着，如果加密信息被获取，就可以通过类似 John the Ripper 的工具来破解密码，当然，时间可能会很长。

## 其它

### 重试

需要注意，如果在获取用户信息的时候，刚好有添加用户之类的操作，包括了保存 `/etc/passwd` 文件，可能会导致获取用户失败，而且 `errno` 仍然为 0 。

所以，此时最好增加重试机制，带来的副作用是，如果用户真的不存在可能会浪费资源。

{% highlight c %}
int get_user_unsafe(const char *name)
{
        int i;
        struct passwd *pw;

        for (i = 0; i < 3; i++) {
                pw = getpwnam(name); /* getpwuid(uid); */
                if (pw == NULL) {
                        if (errno == 0) {
                                //fprintf(stderr, "no such user '%s'.\n", name);
                                usleep(20000);
                                continue;
                        }
                        fprintf(stderr, "get user info failed, %d:%s.\n",
                                        errno, strerror(errno));
                        break;
                }
                break;
        }
        if (i > 0)
                fprintf(stderr, "get user info in %d times.\n", i);

        return 0;
}
{% endhighlight %}

### 安全性

`getpwnam()` 函数会使用 glibc 中的静态内存，返回的结果实际上就指向这块内存，每次调用该接口会覆盖。这也就意味着，如果多次调用，实际上只会获取到最后的一次结果。

另外，需要注意 `getpwnam_r()` 函数不是信号安全的，内部会对线程加锁，不要在信号处理函数中调用，可能会造成死锁。

### 用户组

通过 `su - name` 切换的时候，会把包含该用户所拥有的所有组，可以通过 `id` 命令查看，对应的 `groups=` 中会包含多个组，这样在访问某个目录时，只要有一个组有权限即可。

在代码实现时，可以通过 `setuid()` `setgid()` `setgroups()` `initgroups()` 调用实现。

此外，需要注意，当进程的 uid 和 euid 不一致时，默认是不会产生 CoreDump 文件的，需要将 `/proc/sys/fs/suid_dumpable` 设置为 1 。


{% highlight text %}
{% endhighlight %}
