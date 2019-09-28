---
title: BasicAgent 实现
layout: project
comments: true
language: chinese
category: [misc]
keywords:
description:
---


一般的磁盘文件系统都会保存在某个设备分区上，一般是通过 `dev_t` 标识，可以通过 `<sys/sysmacros.h>` 提供的 `major()` 和 `minor()` 宏进行访问。

注意，上述的宏，实际上使用 `<sys/types.h>` 头文件即可。

只有字符特殊设备和块特殊设备才会有 `st_rdev` 值，该值包含了实际设备的设备号，例如 `/dev/sda` `/dev/sda1` `/dev/tty0` 。

{% highlight text %}
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <sys/stat.h>
#include <sys/types.h>

int main(int argc, char *argv[])
{
        int i;
        struct stat s;

        for (i = 1; i < argc; i++) {
                if (stat(argv[i], &s) < 0) {
                        fprintf(stderr, "stat '%s' failed, %d:%s.",
                                argv[i], errno, strerror(errno));
                        continue;
                }

                if (S_ISREG(s.st_mode)) {
                        printf("Regular file: %s\n", argv[i]);
                        printf("    major=%d minor=%d\n",
                                        major(s.st_dev), minor(s.st_dev));
                        printf("    inode %ld\n", s.st_ino);
                        printf("    mode %o\n", s.st_mode);
                        printf("    blksize %ld\n", s.st_blksize);
                } else if (S_ISCHR(s.st_mode) || S_ISBLK(s.st_mode)) {
                        printf("%s device: %d/%d.\n",
                                S_ISCHR(s.st_mode) ? "Character" : "Block",
                                major(s.st_rdev), minor(s.st_rdev));
                }
        }

        return 0;
}
{% endhighlight %}


C中的正则匹配性能比较
https://www.cnblogs.com/pmars/archive/2012/10/24/2736831.html

## inotify

可以监听的事件可以通过 `man 7 inotify` 查看详细的介绍。

https://www.jiangmiao.org/blog/2179.html

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <unistd.h>

#define ES  (sizeof(struct inotify_event))
#define EBUFF_LEN     (1024 * (ES + 16))
int main(int argc, char *argv[])
{
        char *path;
        char buff[BUFSIZ];
        int fd, len, i = 0;
        struct inotify_event *event;

        if (argc < 2) {
                fprintf(stderr, "Usage: %s <path>\n", argv[0]);
                return -1;
        }
        path = argv[1];

        fd = inotify_init();
        if (fd < 0) {
                fprintf(stderr, "init inotify failed, %d:%s.\n",
                                errno, strerror(errno));
                return -1;
        }

        /* more directories could add here. */
        if (inotify_add_watch(fd, path, IN_ALL_EVENTS) < 0) {
                fprintf(stderr, "add inotify watch for '%s' failed, %d:%s.\n",
                                path, errno, strerror(errno));
                close(fd);
                return -1;
        }

        while (1) {
                len = read(fd, buff, sizeof(buff));
                if (len < 0) {
                        fprintf(stderr, "read from inotify failed, %d:%s.\n",
                                        errno, strerror(errno));
                        break;
                }
                for (i = 0; i < len; i += (ES + event->len)) {
                        event = (struct inotify_event *)&buff[i];

                        printf("New event\n");
                        if (event->cookie > 0)
                                printf("cookie %04d\n", event->cookie);
                        printf("mask = ");
                        if (event->mask & IN_ACCESS)        printf("IN_ACCESS ");
                        if (event->mask & IN_ATTRIB)        printf("IN_ATTRIB ");
                        if (event->mask & IN_CLOSE_NOWRITE) printf("IN_CLOSE_NOWRITE ");
                        if (event->mask & IN_CLOSE_WRITE)   printf("IN_CLOSE_WRITE ");
                        if (event->mask & IN_CREATE)        printf("IN_CREATE ");
                        if (event->mask & IN_DELETE)        printf("IN_DELETE ");
                        if (event->mask & IN_DELETE_SELF)   printf("IN_DELETE_SELF ");
                        if (event->mask & IN_IGNORED)       printf("IN_IGNORED ");
                        if (event->mask & IN_ISDIR)         printf("IN_ISDIR ");
                        if (event->mask & IN_MODIFY)        printf("IN_MODIFY ");
                        if (event->mask & IN_MOVE_SELF)     printf("IN_MOVE_SELF ");
                        if (event->mask & IN_MOVED_FROM)    printf("IN_MOVED_FROM ");
                        if (event->mask & IN_MOVED_TO)      printf("IN_MOVED_TO ");
                        if (event->mask & IN_OPEN)          printf("IN_OPEN ");
                        if (event->mask & IN_Q_OVERFLOW)    printf("IN_Q_OVERFLOW ");
                        if (event->mask & IN_UNMOUNT)       printf("IN_UNMOUNT ");
                        printf("\n");

                        if (event->len > 0)
                                printf(" %s\n", event->name);
                }
        }

        return 0;
}
The C10K problem
http://www.kegel.com/c10k.html
https://yq.aliyun.com/articles/204554
https://www.thegeekdiary.com/centos-rhel-67-how-to-increase-system-log-message-verbosity-rsyslogd/
https://www.digitalocean.com/community/tutorials/how-to-view-and-configure-linux-logs-on-ubuntu-and-centos


https://www.jianshu.com/p/6a03ba897e04
https://yq.aliyun.com/articles/204554?spm=5176.10695662.1996646101.searchclickresult.6ed6ff98OMv6De

{% highlight text %}
{% endhighlight %}
