---
Date: October 19, 2013
title: Ansible 简介
layout: post
comments: true
language: chinese
category: [python]
---


本片文章简单介绍下。

<!-- more -->



架构图

ansible架构图

ansible架构图
工作原理

工作原理

ansible工作原理

    管理端支持 local、 ssh、zeromq 三种方式连接被控端，默认使用 ssh
    可以按照一定规则进行 inventory，管理节点通过模块实现对应操作–ad-hoc
    管理节点可以通过 playbook 实现对多个 task 的集合实现一类功能

安装 Ansible

    源码安装

源码安装需要 python2.6 以上版本，依赖 paramiko， PyYAML， Jinja2， simplejsion、 pycrypto模块，可以通过 pip 来安装

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
15
16
17
18
19
20
21
22
23



// 获取源码
git clone git://github.com/ansible/ansible.git --recursive
cd ./ansible
// 设置环境变量
source ./hacking/env-setup
source ./hacking/env-setup.fish
source ./hacking/env-setup -q
// 安装 Python 依赖
easy_install pip
pip install paramiko PyYAML Jinja2 httplib2 six
// 更新 Ansible
git pull --rebase
git submodule update --init --recursive
// 设置inventory文件
echo "127.0.0.1" > ~/ansible_hosts
export ANSIBLE_HOSTS=~/ansible_hosts
// 测试命令
ansible all -m ping --ask-pass

    常用 Linux　发行版

1
2
3
4
5
6
7
8



// CentOS、RHEL
yum install ansible
//Ubuntu、Debian
sudo apt-get install software-properties-common
sudo apt-add-repository ppa:ansible/ansible
sudo apt-get update
sudo apt-get install ansible

    通过 pip 安装最新版
    Ansible 可以通过 pip 安装,同时也会安装 paramiko、PyYAML、jinja2 等 Python 依赖库。

1
2



apt install python3-pip
pip3 install ansible

运行 Ansible
添加被控远程主机清单

已经安装好了 Ansible ，先在就可以运行 Ansible 了。 首先要在 /etc/ansible/hosts 文件中加入一个或者多个远程 ip 或者 domain。

1
2



172.16.11.210
172.16.11.211

配置基于 SSH key 方式 连接

1
2
3
4



// 主控端操作
ssh-keygen -t rsa -q
ssh-copy-id 172.16.11.210
ssh-copy-id 172.16.11.211

运行 Ansible

1
2
3
4
5
6
7
8
9



ansible all -m ping
172.16.11.210 | SUCCESS => {
    "changed": false,
    "ping": "pong"
}
172.16.11.211 | SUCCESS => {
    "changed": false,
    "ping": "pong"
}

配置 inventory

Ansible 可以同时操纵属于一个组的多台主机，主机和组的关系是通过 inventory 文件来配置即/etc/ansible/hosts。
inventory可以通过 IP、Domain 来指定，未分组的机器要保留在 host 文件顶部，通过[] 来配置分组信息。
主机与组

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
15
16
17
18
19



// 简单分组
[web1]
web210.example.com
web211.example.com
// 配置端口号
[web2]
172.16.11.210:8000
172.16.11.211:8000
// 定义别名和端口
[web]
www ansible_ssh_port=1234 ansible_ssh_host=172.16.11.211  ansible_ssh_pass=passwd \\ 远程ip，ssh登陆用户、密码
other1  ansible_connertion=ssh ansible_ssh_user = illlusion
//执行主机，支持正则表达
[web_www]
www[01:10].example.com
db-[a:f].erample.com

变量

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
15
16
17
18
19
20
21
22
23
24
25
26
27
28
29
30
31
32
33
34



// 主机变量，分配变量给主机，这些变量可以在之后的 playbook 中使用
[web-www]
www-a http_port=89 maxRequestsPerChild=808
www-a http_port=303 maxRequestsPerChild=909
//组的变量，组也可以赋予变量，这样组成员将继承组变量
[web-www]
www-a http_port=89 maxRequestsPerChild=808
www-a http_port=303 maxRequestsPerChild=909
[web-www:vars]
ntp_server=ntp.example.com
proxy=proxy.example.com
// 组嵌套 可以把组作为另外一个组的子成员，已经分配变量给整个组使用。这些变量可以给 `/usr/bin/ansible-playbook` 使用，但是不能给 `/usr/bin/ansible` 使用
[group1]
host1
host2
[group2]
host3
host4
[group3:children]
group1
group2
[group3:vars]
some_server=foo.example.com
halon_system_timeout=30
self_destruct_countdown=60
escape_pods=2

分文件定义 Host 和 group 变量

在 inventory 文件中保存的所有变量并不是最佳方式，还可以保存在独立的文件中， 这些文件与 inventory 关联，要求使用 YAML语法。host 和 gourp 变量 要求存储在与 host 和 group 相同的目录名中

1
2
3
4
5
6
7
8



//假设有一个 host 为 foosball 主机，属于两个组，一个是 raleigh,另外一个是 webserver
/etc/ansible/group_vars/raleigh
/etc/ansible/group_vars/webservers
/etc/ansible/host_vars/foosball
// raleigh 组的变量
ntp_server: acme.example.org
database_server: storage.example.org

还可以在组变量目录下创建多个文件，设置不同类型的变量

1
2



/etc/ansible/group_vars/raleigh/db_settings
/etc/ansible/group_vars/raleigh/cluster_settings

inventory 参数说明

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
15
16
17
18
19
20
21
22
23
24
25
26
27
28
29



// 要连接的远程主机名.与你想要设定的主机的别名不同的话,可通过此变量设置.
ansible_ssh_host
// ssh 端口号.如果不使用默认,通过此变量设置.
ansible_ssh_port
// ssh 用户名
ansible_ssh_user
// ssh 密码(这种明文方式并不安全,强烈建议使用 --ask-pass 或 SSH 密钥)
ansible_ssh_pass
// sudo 密码(这种方式并不安全,强烈建议使用 --ask-sudo-pass)
ansible_sudo_pass
// sudo 命令路径(适用于1.8及以上版本)
ansible_sudo_exe (new in version 1.8)
// 与主机的连接类型.比如:local, ssh 或者 paramiko. Ansible 1.2 以前默认使用 paramiko.1.2 以后默认使用 'smart','smart' 方式会根据是否支持 ControlPersist, 来判断'ssh' 方式是否可行.
ansible_connection
// ssh 使用的私钥文件.适用于有多个密钥,而你不想使用 SSH 代理的情况.
ansible_ssh_private_key_file
// 目标系统的 shell 类型.默认情况下,命令的执行使用 'sh' 语法,可设置为 'csh' 或 'fish'.
ansible_shell_type
// 目标主机的 python 路径.适用于的情况: 系统中有多个 Python, 或者命令路径不是"/usr/bin/python",比如  \*BSD, 或者 /usr/bin/python
ansible_python_interpreter

Patterns

在 ansible 中， patterns 是指如何确定有那些主机或组被管理，在 playbook 中，它是指对应主机应用特定的配置或执行特定进程。
ansible

1
2
3
4
5



// 语法
ansible <pattern_goes_here> -m <module_name> -a <arguments>
// 示例
ansible webservers -m service -a "name=httpd state=restarted"

简单的说， pattern 是一个主机筛选器，支持正则匹配。

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
15
16



// 所有主机
all
*
//特定主机，支持 ip 地址和主机名
web211
172.16.11.211
//主机组，可以指定特定组或多个组，多个组之间使用`:`分隔
web_server
web_server:database_server
// 支持正则表达式和逻辑运算
web_server:!web211
web_server:&db1
web_server:database_server:&db1:!web211

playbook

在 playbook 中，通过使用 -e参数可以实现通过变量来确定 group

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
15
16
17
18
19
20
21



webservers:!{{excluded}}:&{{required}}
// 通配符
*.example.com
*.com
//通配符和正则同时
one*.com:dbservers
// 在 patterns 应用正则式时，使用 `~` 开头
~(web|db).*\.example\.com
// 索引和切片
webservers[0]
webservers[0-25]
// 可以在使用 `--limit` 标记来添加排除条件
ansible-playbook site.yml --limit datacenter2
// 如果你想从文件读取 hosts,文件名以 @ 为前缀即可.
ansible-playbook site.yml --limit @retry_hosts.txt

简单执行命令

1
2
3
4
5
6
7
8
9



ansible  all -m ping
172.16.11.210 | SUCCESS => {
    "changed": false,
    "ping": "pong"
}
172.16.11.211 | SUCCESS => {
    "changed": false,
    "ping": "pong"
}

可用该命令选项：

    -i：指定 inventory 文件，使用当前目录下的 hosts
    all：针对 hosts 定义的所有主机执行，这里也可以指定组名或模式
    -m：指定所用的模块，我们使用 Ansible 内置的 ping 模块来检查能否正常管理远端机器
    -u：指定远端机器的用户

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
15
16
17
18
19
20
21
22
23
24
25
26
27
28
29
30
31
32
33
34
35
36
37



ansible all -m ping     \\ping 所有的节点
ansible 127* -m ping
ansible -i /etc/ansible/hosts -m command -a "uptime"  // 指定 pattens 文件
ansible all -m ping -u test
ansible all -m ping -u test --sudo
ansible all -m ping -u test --sudo --sudo-user tom
ansible testhost -m setup -a "filter=ansible_all_ipv4_addresses" \\使用 filter 过滤信息
ansible testhosts -a "/sbin/reboot" -f 10  \\重启testhosts组的所有机器，每次重启10台
ansible testhosts -m copy -a "src=/etc/hosts dest=/tmp/hosts" \\拷贝本地hosts 文件到testhosts组所有主机的/tmp/hosts
ansible webservers -m file -a "dest=/srv/foo/a.txt mode=600" \\file 模块允许更改文件的用户及权限
ansible webservers -m file -a "dest=/srv/foo/b.txt mode=600 owner=mdehaan group=mdehaan"
ansible webservers -m file -a "dest=/path/to/c mode=755 owner=mdehaan group=mdehaan state=directory" \\使用 file 模块创建目录，类似 mkdir -p
ansible webservers -m file -a "dest=/path/to/c state=absent" \\file 模块允许更改文件的用户及权限
ansible testhosts -a 'cal'  \\默认是使用 command 模块，所以使用command的命令时不用添加 -m
ansible webhosts -m command -a 'date' \\在 hosts 文件中的 webhosts 组下的所有主机都使用 date 命令
ansible webhosts -m command -a 'ping' \\在 hosts 文件中的 webhosts 组下的所有主机都使用 date 命令
ansible testhosts -m service -a "name=ntpd state=restarted"
使用 user 模块对于创建新用户和更改、删除已存在用户非常方便：
ansible all -m user -a "name=foo password=<crypted password here>"
ansible all -m user -a "name=foo state=absent"
// 服务管理：
ansible webservers -m service -a "name=httpd state=restarted" \\重启 webservers 组所有主机的 httpd 服务
ansible webservers -m service -a "name=httpd state=started"  \\确保 webservers 组所有主机的 httpd 是启动的
ansible webservers -m service -a "name=httpd state=stopped"  \\确保 webservers 组所有主机的 httpd 是关闭的
//后台运行，长时间运行的操作可以放到后台执行，ansible 会检查任务的状态；在主机上执行的同一个任务会分配同一个 job ID
ansible all -B 3600 -a "/usr/bin/long_running_operation --do-stuff" \\后台执行命令 3600s，-B 表示后台执行的时间
ansible all -m async_status -a "jid=123456789"  \\检查任务的状态
ansible all -B 1800 -P 60 -a "/usr/bin/long_running_operation --do-stuff" \\后台执行命令最大时间是 1800s 即 30 分钟，-P 每 60s 检查下状态默认 15s
// 搜集系统信息
ansible all -m setup \\搜集主机的所有系统信息
ansible all -m setup --tree /tmp/facts \\搜集系统信息并以主机名为文件名分别保存在 /tmp/facts 目录
ansible all -m setup -a 'filter=ansible_*_mb' \\搜集和内存相关的信息
ansible all -m setup -a 'filter=ansible_eth[0-2]' \\搜集网卡信息

Ad-Hoc

执行 Ad-Hoc 跟在 Linux 执行命令差不多， 用来快速完成简单的任务。
语法

1



ansible [host or group] -m  [module_name] -a [commond] [ ansible-options ]

实例

    执行安装程序， 安装 python-simplejson

1



ansible all -m raw -a 'yum -y install python-simplejson'

    重启 web 服务
    假如 web_server 是一个组， 这里组里面有很多webserver，先在需要在 web_server 组上的左右机器执行 reboot 命令， -f 参数会 fork 出 10 个子进程，以并行的方式执行 reboot，即每次重启 10 台

1



ansible web_server -a "/sbin/reboot" -f 10

在执行时，默认是以当前用户身份去执行该命令，如果需要执行执行用户，添加 -u username，或者需要使用 sudo 去执行,添加 -u username --sudo [--ask-sudo-pass]。如果不是以 passwordless 的模式执行 sudo,应加上 –ask-sudo-pass (-K)选项,加上之后会提示你输入 密码.使用 passwordless 模式的 sudo, 更容易实现自动化,但不要求一定要使用 passwordless sudo.

    文件传输

ansible 的另外一种用法就是可以以并行的方式同时 scp 大量的文件到多台主机。

1



ansible all -m copy -a "src=/opt/ansible/test.txt dest=/opt/ansible/test.txt"

如果是用 playbook，择可以利用 template 模块来实现更高级操作。

使用 file 模块 可以修改文件的属主和权限

1



ansible all -m file -a 'dest=/opt/ansible/test.txt mode=600 owner=nobody group=nobody'

使用 file 模块还可以创建、删除目录和文件

1
2
3
4
5



// 创建目录
ansible all -m file -a 'dest=/opt/ansible/test mode=755 owner=root group=root state=directory'
// 删除目录和文件
ansible all -m file -a 'dest=/opt/ansible/test state=absent'

更多详见copy模块说明

    包管理

ansible 提供了对 yum 和 apt 的支持

1
2
3
4
5



 // 安装软件包
 ansible all -m yum -a 'name=vim state=present'
// 卸载软件包
 ansible all -m yum -a 'name=vim state=absent'

在不同的发行版的软件包管理软件， ansible 有其对应的模块， 如果没有，你可以使用 command 模块去安装软件。
更多详见package模块说明

    用户和组管理

1
2
3
4
5
6
7
8



// 创建用户
ansible all -m user -a 'name=charlie password=123456 state=present'
// 修改用户， 增加属组和修改shell
ansible all -m user -a 'name=Cc groups=nobody shell=/sbin/nologin state=present'
//移除用户
ansible all -m user -a 'name=Cc state=absent'

更多参数详见user模块说明

    服务管理

1
2
3
4
5
6



// 启动服务
ansible all -m service -a 'name=rsyslog state=started'
// 重启服务
ansible all -m service -a 'name=rsyslog state=restarted'
// 停止服务
ansible all -m service -a 'name=rsyslog state=stopped'

    系统自身变量获取

ansible 可以通过 setup 模块来获取客户端自身的以便固有信息，叫做 facts

1
2
3
4
5



// 获取所有 facts 变量
ansible all -m setup
// 通过 filter 获取某一个 fact 变量
ansible all -m setup -a 'filter=ansible_*mb'






{% highlight text %}
$ ansible all -m ping
{% endhighlight %}


http://docs.ansible.com/ansible/dev_guide/developing_api.html
http://www.tuicool.com/articles/uMBz2e6
http://blog.csdn.net/python_tty/article/details/51387667
http://docs.ansible.com/ansible/dev_guide/developing_api.html
http://blog.itpub.net/30109892/viewspace-2063898/
http://blog.coocla.org/how-to-use-ansible-python-api-2.0.html
http://lixcto.blog.51cto.com/4834175/1434604
http://www.jianshu.com/p/8558befb16c1
http://www.jianshu.com/p/81da20dd9931
https://czero000.github.io/2016/10/19/the-advanced-ansible.html


执行命令    ansible rds -a "id" --ask-pass批量修改密码    ansible rds -m user -a "name=jinyang password=Tools@123" -b -k


{% highlight python %}
{% endhighlight %}
