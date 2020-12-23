---
title: Linux PAM 认证机制使用详解
layout: post
comments: true
language: chinese
tag: [Linux,DevOps]
keywords: linux,pam
description: Linux 通过 PAM 机制提供了灵活的权限控制，主要应用提供了 PAM 支持，那么用户就可以通过该机制进行灵活的配置，不需要在修改原程序。
---

Linux 通常会通过 login 进程完成登陆，最开始时只是简单的提示用户输入用户名和密码，然后校验用户是否存在、密码是否正确，如果都正常，那么就会直接完成登陆，进入到 Shell 程序运行。

PAM 提供了独立于具体程序配置机制，可以更加灵活的鉴权方案，这里详细介绍其使用方式。

<!-- more -->

## 简介

通常在用户登陆时，需要有一套的验证授权机制，最开始的时候，这一整套的验证机制是硬编码到程序中的，这样当程序有 bug 或者需要修改验证策略时，只能修改源程序。

为了改善这些问题，人们开始思考其他的方法，也就是所谓的 `Pluggable Authentication Modules, PAM` 应运而生了。

PAM 提供了一整套的鉴权、授权、密码管理、会话管理机制等，只需要程序支持 PAM 框架，用户就可以在完全不修改程序的条件下，动态修改鉴权机制，例如除了常规的用户名密码登陆，还可以使用指纹、One-Time-Password 等机制。

### 运行机制

如下是一个最常见的 `login` 示例程序，其中包括了二进制 `login` 可执行程序，该程序会动态链接 `libpam.so` 库，该库会读取 `/etc/pam.d/login` 配置文件，并根据配置文件中的内容，按照顺序生成不同栈。

![linux pam arch](/{{ site.imgdir }}/security/linux-pam-arch.png "linux pam arch")

然后，会根据不同的栈以及配置执行相关的动作。

### 相关文件

在 64 位系统中，与 PAM 相关的文件包含了如下几类：

1. `/usr/lib64/libpam.so*` 核心库，使用 PAM 机制的应用会链接到该库上。
2. `/etc/pam.conf` `/etc/pam.d/*` 配置文件，配置内容基本类似，前者为全局配置，通过第一列标识应用程序，而后者则以文件名标识应用程序，结构层次更加明确，也更常见。
3.  `/usr/lib64/security/pam_*.so` 可以动态加载的模块，在配置文件中可以直接通过文件名引用。

如果一个应用程序 (例如 login) 想使用 PAM 提供的机制，那么需要链接到 `libpam.so` 库，否则就不支持 PAM 机制，可以通过如下命令查看。

{% highlight text %}
$ ldd /usr/bin/login | grep pam
libpam.so.0 => /lib64/libpam.so.0 (0x00007fc79c03e000)
libpam_misc.so.0 => /lib64/libpam_misc.so.0 (0x00007fc79be3a000)
{% endhighlight %}

默认程序名与相关的 PAM 配置文件是相同的，当然，也允许通过配置文件进行配置，指定不同的配置文件名称。

验证时会按照顺序检查，也就是所谓的流程栈 (Stack) ，是认证时执行步骤、规则的堆叠，体现了自上而下的执行顺序，而且可以被引用。

{% include ads_content01.html %}

## 配置文件

对于配置文件来说，如上所述，有两种方式，一种是全局的配置文件，也就是 `/etc/pam.conf`，其内容如下，第一个表示服务的名称。

{% highlight text %}
ftpd auth required pam_unix.so nullok
{% endhighlight %}

还有一种是将相关服务的配置保存在 `/etc/pam.d/` 目录下，文件名就对应了应用名，例如 login、sshd 等。在配置文件中包含了四列，与上述的后四列相同，分别为：1) 模块类型；2) 控制模式；3) 模块名称；4) 模块参数。

### 配置详解


PAM 会根据配置中的模块名称从 `/usr/lib64/security` 目录下查找，所以通常直接使用文件名称即可；而不同的插件其参数也有所区别，所以，这里主要介绍前两个配置项。

#### 1. 模块类型

PAM 有四种模块类型，代表不同的任务，一个类型可能有多行，按顺序依次由 PAM 模块调用：

* 认证管理，auth<br>用来对用户的身份进行识别，如提示用户输入密码、判断用户是否为 root 等。
* 账号管理，account<br>对帐号的各项属性进行检查，如是否允许登录、是否达到最大用户数、root 用户是否允许在这个终端登录等。
* 会话管理，session<br>定义用户登录前的及用户退出后所要进行的操作，如登录连接信息、用户数据的打开与关闭、挂载文件系统等。
* 密码管理，password<br>使用用户信息来更新，如修改用户密码。

如果在模块类型的开头有个短横线 `-` ，意味着，如果找不到这个模块导致无法加载，这一事件不会被记录在日志中，适用于哪些非必须的验证功能。

#### 2. 控制标记

通过控制标记来处理和判断各个模块的返回值。

* required<br>即使某个模块对用户的验证失败，也要等所有的模块都执行完毕后才返回错误信息。
* requisite<br>如果某个模块返回失败，则立刻返回失败，不再进行同类型后面的操作。
* sufficient<br>如果验证通过，则立即返回验证成功消息，无论前面模块是否有失败，而且也不再执行后面模块。如果验证失败，sufficient 的作用和 optional 相同 。
* optional<br>即使指定的模块验证失败，也允许用户接受应用程序提供的服务，一般返回 PAM_IGNORE 。
* include<br>包含一个新的配置文件进行验证。
* substack<br>与 include 类似，区别是，include 调用文件执行时有 die 或者 bad 则立即返回调用处，而 substack 则等待文件执行完。


<!--
另外一种方式是以类似 `[value1=action1 value2=action2 ... default=actionN]` 的配置，。
其中 action 合法的有：

* ignore<br>如果使用层叠模块，那么这个模块的返回值将被忽略，不会被应用程知道

    bad
       告诉PAM这个模拟的返回值应该被看作是模块验证失败。如果这个模块是层叠模块的第一个验证失败的模块，那么它的状态值就是整个层叠模块的状态值

    die
       与bad的区别在于，此值会终止层叠模块验证过程，立刻返回到应用程序

    ok
       告诉PAM这个模块的返回值直接作为所有层叠模块的返回值。也就是说，如果这个模块前面的模块返回状态是PAM_SUCCESS，那这个返回值就会覆盖前面的返回状态。注意：如果前面的模块的返回状态表示模块验证失败，那么不能使用这个返回值覆盖

    done
       与ok的区别是，此值终止后续层叠模块的验证，把控制权立刻交回应用程序

    reset
       清除之前所有叠模块的返回状态，从下一个层叠模块重新开始



Each of the four keywords: required; requisite; sufficient; and optional, have an equivalent expression in terms of the [...] syntax. They are as follows：

   required
       [success=ok new_authtok_reqd=ok ignore=ignore default=bad]

   requisite
       [success=ok new_authtok_reqd=ok ignore=ignore default=die]

   sufficient
       [success=done new_authtok_reqd=done default=ignore]

   optional
       [success=ok new_authtok_reqd=ok default=ignore]
-->

### 常用模块

<!--
pam_tally2
用来限制用户登录失败的次数，如果达到阈值则会锁定用户。
even_deny_root 也限制root用户
-->

{% highlight text %}
pam_unix.so
  auth       提示用户输入密码，并与/etc/shadow文件相比对，匹配返回0。
  account    检查用户的账号信息(如是否过期)，帐号可用时返回0。
  password   修改用户的密码，将用户输入的密码，作为用户的新密码更新shadow文件。

pam_shells.so
  auth account 如果用户想登录系统，那么它的shell必须是在/etc/shells中。

pam_deny.so
  auth account password session 用来拒绝访问。

pam_permit.so
  auth account password session 任何时候都返回成功。

pam_cracklib.so
  password 提示用户输入密码，并与系统中的字典进行比对，检查密码的强度。

pam_securetty.so
  auth 用户要以root登录时，则登录的tty必须在/etc/securetty中。
{% endhighlight %}

{% include ads_content02.html %}

## 示例程序

如下程序从命令行接收一个用户名作为参数，然后对这个用户名进行 auth 和 account 验证。

{% highlight c %}
#include <stdio.h>
#include <security/pam_appl.h>
#include <security/pam_misc.h>
#include <security/pam_modules.h>

int main(int argc, char *argv[])
{
    pam_handle_t *pamh=NULL;
    int retval;
    const char *user="nobody";
    struct pam_conv conv = { misc_conv, NULL };

    if(argc == 2)
        user = argv[1];
    if(argc > 2) {
        fprintf(stderr, "Usage: %s [username]\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    printf("user: %s\n",user);
    retval = 0;

    retval = pam_start("pamtest", user, &conv, &pamh);
    if (retval == PAM_SUCCESS) {
        retval = pam_authenticate(pamh, 0); // 进行auth类型认证
    } else { // 认证出错，则输出错误信息
        printf("pam_authenticate(): %d\n", retval);
        printf("%s\n", pam_strerror(pamh, retval));
    }

    if (retval == PAM_SUCCESS) {
        retval = pam_acct_mgmt(pamh, 0);    // 进行account类型认证
    } else {
        printf("pam_acct_mgmt() : %d\n", retval);
        printf("%s\n", pam_strerror( pamh, retval));
    }

    if (retval == PAM_SUCCESS) {
        fprintf(stdout, "Authenticated\n");
    } else {
        fprintf(stdout, "Not Authenticated\n");
    }

    if (pam_end(pamh,retval) != PAM_SUCCESS) {     /* close Linux-PAM */
        pamh = NULL;
        fprintf(stderr, "pamtest0: failed to release authenticator\n");
        exit(EXIT_FAILURE);
    }

    return ( retval == PAM_SUCCESS ? 0:1 );       /* indicate success */
}
{% endhighlight %}

添加如下的配置文件，并同时通过如下命令编译执行。

{% highlight text %}
# cat << EOF > /etc/pam.d/pamtest
# 提示用户输入密码
auth     required   pam_unix.so
# 验证用户账号是否可用
account  required   pam_unix.so
# 向系统日志输出一条信息
account  required   pam_warn.so
EOF

$ gcc -o pamtest pamtest.c -lpam -lpam_misc -Wall
{% endhighlight %}

## 参考

* 官方的相关文档 [www.linux-pam.org](http://www.linux-pam.org/)、[The Linux-PAM System Administrators' Guide](http://www.linux-pam.org/Linux-PAM-html/Linux-PAM_SAG.html)、[The Linux-PAM Application Developers' Guide](http://www.linux-pam.org/Linux-PAM-html/Linux-PAM_ADG.html)、[The Linux-PAM Module Writers' Guide](http://www.linux-pam.org/Linux-PAM-html/Linux-PAM_MWG.html)。
* 一个 Solaris 的开发指南，可以供参考 [编写 PAM 应用程序和服务](http://docs.oracle.com/cd/E19253-01/819-7056/6n91eac3n/index.html) 。

<!--
authenticaction authorization 认证和授权
-->

{% highlight text %}
{% endhighlight %}
