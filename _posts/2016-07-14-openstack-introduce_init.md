---
Date: October 19, 2013
title: OpenStack 安装
layout: post
comments: true
language: chinese
category: [network]
---

<!-- more -->

## 准备环境

### 安装 OpenStack 包

{% highlight text %}
----- 启用OpenStack数据源
# yum install centos-release-openstack-liberty

----- 更新软件包，部分依赖上述软件源的包会更新
# yum upgrade

----- 安装OpenStack客户端
# yum install python-openstackclient

----- CentOS默认启用SELinux，安装如下包实现对OpenStack服务的安全策略进行自动管理
# yum install openstack-selinux
{% endhighlight %}

### 安装 SQL 数据库

大多数 OpenStack 服务使用 SQL 数据库来存储信息，安装 SQL 数据库。

{% highlight text %}
----- 安装MySQL数据库
# yum install mariadb mariadb-server MySQL-python

----- 设置为开机启动，并启动MySQL服务
# systemctl enable mariadb.service
# systemctl start mariadb.service
{% endhighlight %}

### 安装 MQ

OpenStack 使用 Message Queue 协调操作和各服务的状态信息，MQ 一般运行在控制节点上，支持 ZeroMQ、RabbitMQ、Qpid 等。

{% highlight text %}
----- 安装RabbitMQ
# yum install rabbitmq-server

----- 设置为开机启动，并启动RabbitMQ服务
# systemctl enable rabbitmq-server.service
# systemctl start rabbitmq-server.service

----- 添加openstack用户
# rabbitmqctl add_user openstack RABBIT_PASS
Creating user "openstack" ...

----- 给openstack用户配置写和读权限
# rabbitmqctl set_permissions openstack ".*" ".*" ".*"
Setting permissions for user "openstack" in vhost "/" ...
{% endhighlight %}

如果出现 ERROR: epmd error for host XXXXX: timeout (time out)，主要是由于主机名和 IP 不匹配导致，直接在 /etc/hosts 中添加 127.0.0.1 XXXXX 即可。


## Keystone，身份认证服务

这一章描述如何在控制节点上安装和配置 OpenStack 身份认证服务，代码名称 keystone。出于性能原因，这个配置部署 Apache HTTP 服务处理查询并使用 Memcached 存储 tokens 而不用 SQL 数据库。


配置 OpenStack 身份认证服务前，必须创建一个数据库和管理员令牌。

{% highlight text %}
$ mysql -u root -p
CREATE DATABASE keystone;
GRANT ALL PRIVILEGES ON keystone.* TO 'keystone'@'localhost' IDENTIFIED BY 'KEYSTONE_DBPASS';
GRANT ALL PRIVILEGES ON keystone.* TO 'keystone'@'%' IDENTIFIED BY 'KEYSTONE_DBPASS';
{% endhighlight %}

生成一个随机值在初始的配置中作为管理员的令牌。

{% highlight text %}
$ openssl rand -hex 10
42813d8157799f6b892d
{% endhighlight %}

安装 Keystone 服务，使用 Nginx 提供服务。

{% highlight text %}
# yum install openstack-keystone httpd mod_wsgi
{% endhighlight %}

编辑文件 /etc/keystone/keystone.conf 并设置如下的值。

{% highlight text %}
[DEFAULT]
# 定义管理员token初始值，也就是上述设置的随机值
admin_token = 42813d8157799f6b892d
# 启用详细日志
verbose = true
[database]
# 设置数据库的连接方式
connection = mysql://keystone:KEYSTONE_DBPASS@127.1/keystone
[memcache]
# 配置Memcached服务，暂时没有使用，直接访问数据库
#servers = localhost:11211
[token]
# 配置UUID token provider，驱动暂时使用sql，如果是Memcached驱动则为(memcache)
provider = uuid
driver = sql
[revoke]
# 配置SQL回滚驱动
driver = sql
{% endhighlight %}

初始化身份认证服务的数据库，在此使用的是 MySQL 数据库，可以通过如下两个命令执行，其功能相同，是否执行成功可以查看 /var/log/keystone/keystone.log 中的内容。

{% highlight text %}
# keystone-manage db_sync
# su -s /bin/sh -c "keystone-manage db_sync" keystone
{% endhighlight %}

此时，在 MySQL 中的 keystone 库中，会创建一堆的表。

其中需要 admin_token 来访问 keystone 的服务，后面也可以通过 keystone-client 来注册新的 token，默认是 ADMIN，可以把它添加到系统环境里去，如下，分别配置 TOKEN、端点 URL、认证 API 版本。

{% highlight text %}
export OS_TOKEN=42813d8157799f6b892d
export OS_URL=http://127.0.0.1:35357/v3
export OS_IDENTITY_API_VERSION=3
{% endhighlight %}

配置 Apache HTTP 服务器，<!--
编辑 /etc/httpd/conf/httpd.conf 文件，配置 ServerName 选项为控制节点：
ServerName controller
-->用下面的内容创建文件 /etc/httpd/conf.d/wsgi-keystone.conf。

{% highlight text %}
Listen 5000
Listen 35357

<VirtualHost *:5000>
    WSGIDaemonProcess keystone-public processes=5 threads=1 user=keystone group=keystone display-name=%{GROUP}
    WSGIProcessGroup keystone-public
    WSGIScriptAlias / /usr/bin/keystone-wsgi-public
    WSGIApplicationGroup %{GLOBAL}
    WSGIPassAuthorization On
    <IfVersion >= 2.4>
      ErrorLogFormat "%{cu}t %M"
    </IfVersion>
    ErrorLog /var/log/httpd/keystone-error.log
    CustomLog /var/log/httpd/keystone-access.log combined

    <Directory /usr/bin>
        <IfVersion >= 2.4>
            Require all granted
        </IfVersion>
        <IfVersion < 2.4>
            Order allow,deny
            Allow from all
        </IfVersion>
    </Directory>
</VirtualHost>

<VirtualHost *:35357>
    WSGIDaemonProcess keystone-admin processes=5 threads=1 user=keystone group=keystone display-name=%{GROUP}
    WSGIProcessGroup keystone-admin
    WSGIScriptAlias / /usr/bin/keystone-wsgi-admin
    WSGIApplicationGroup %{GLOBAL}
    WSGIPassAuthorization On
    <IfVersion >= 2.4>
      ErrorLogFormat "%{cu}t %M"
    </IfVersion>
    ErrorLog /var/log/httpd/keystone-error.log
    CustomLog /var/log/httpd/keystone-access.log combined

    <Directory /usr/bin>
        <IfVersion >= 2.4>
            Require all granted
        </IfVersion>
        <IfVersion < 2.4>
            Order allow,deny
            Allow from all
        </IfVersion>
    </Directory>
</VirtualHost>
{% endhighlight %}

启动 Apache HTTP 服务并配置其随系统启动：

{% highlight text %}
# systemctl enable httpd.service
# systemctl start httpd.service
{% endhighlight %}

### 创建服务实体和API端点

为身份认证服务创建服务实体。

{% highlight text %}
# openstack service create --name keystone --description "OpenStack Identity" identity
+-------------+----------------------------------+
| Field       | Value                            |
+-------------+----------------------------------+
| description | OpenStack Identity               |
| enabled     | True                             |
| id          | e81ff0c213864ad19509f7fa3c711595 |
| name        | keystone                         |
| type        | identity                         |
+-------------+----------------------------------+
{% endhighlight %}

创建认证服务的 API 端点。

{% highlight text %}
# openstack endpoint create --region RegionOne identity public http://127.0.0.1:5000/v2.0
+--------------+----------------------------------+
| Field        | Value                            |
+--------------+----------------------------------+
| enabled      | True                             |
| id           | c6d75d31a7354c0e8f37e4b764a1d960 |
| interface    | public                           |
| region       | RegionOne                        |
| region_id    | RegionOne                        |
| service_id   | e81ff0c213864ad19509f7fa3c711595 |
| service_name | keystone                         |
| service_type | identity                         |
| url          | http://127.0.0.1:5000/v2.0       |
+--------------+----------------------------------+

# openstack endpoint create --region RegionOne identity internal http://127.0.0.1:5000/v2.0
+--------------+----------------------------------+
| Field        | Value                            |
+--------------+----------------------------------+
| enabled      | True                             |
| id           | 23a675ff529e4d38a7ea73e487adaea5 |
| interface    | internal                         |
| region       | RegionOne                        |
| region_id    | RegionOne                        |
| service_id   | e81ff0c213864ad19509f7fa3c711595 |
| service_name | keystone                         |
| service_type | identity                         |
| url          | http://controller:5000/v2.0      |
+--------------+----------------------------------+

# openstack endpoint create --region RegionOne identity admin http://127.0.0.1:35357/v2.0
+--------------+----------------------------------+
| Field        | Value                            |
+--------------+----------------------------------+
| enabled      | True                             |
| id           | 5fb0ad9d3c014c35bebcc937dd24889f |
| interface    | admin                            |
| region       | RegionOne                        |
| region_id    | RegionOne                        |
| service_id   | e81ff0c213864ad19509f7fa3c711595 |
| service_name | keystone                         |
| service_type | identity                         |
| url          | http://127.0.0.1:35357/v2.0      |
+--------------+----------------------------------+
{% endhighlight %}

### 创建项目、用户和角色


{% highlight text %}
----- 创建 admin 项目
# openstack project create --domain default --description "Admin Project" admin
+-------------+----------------------------------+
| Field       | Value                            |
+-------------+----------------------------------+
| description | Admin Project                    |
| domain_id   | default                          |
| enabled     | True                             |
| id          | f3f0c9353afc4a1ab74d143f85dfabe4 |
| is_domain   | False                            |
| name        | admin                            |
| parent_id   | None                             |
+-------------+----------------------------------+

----- 创建 admin 用户
# openstack user create --domain default --password-prompt admin
User Password: 123
Repeat User Password: 123
+-----------+----------------------------------+
| Field     | Value                            |
+-----------+----------------------------------+
| domain_id | default                          |
| enabled   | True                             |
| id        | 7022c60d8e8848e0bd48f3f2e420ffb7 |
| name      | admin                            |
+-----------+----------------------------------+

----- 创建 admin 角色
# openstack role create admin
+-------+----------------------------------+
| Field | Value                            |
+-------+----------------------------------+
| id    | cb647e95f1694bb283f33cb55d1f5201 |
| name  | admin                            |
+-------+----------------------------------+

----- 添加admin角色到admin项目和用户上
# openstack role add --project admin --user admin admin

----- 创建service项目
# openstack project create --domain default --description "Service Project" service
+-------------+----------------------------------+
| Field       | Value                            |
+-------------+----------------------------------+
| description | Service Project                  |
| domain_id   | default                          |
| enabled     | True                             |
| id          | b810f722e8f340229c90e79f93254a02 |
| is_domain   | False                            |
| name        | service                          |
| parent_id   | None                             |
+-------------+----------------------------------+

----- 创建demo项目
# openstack project create --domain default --description "Demo Project" demo
+-------------+----------------------------------+
| Field       | Value                            |
+-------------+----------------------------------+
| description | Demo Project                     |
| domain_id   | default                          |
| enabled     | True                             |
| id          | 6423234435ee427aae04053ce1973418 |
| is_domain   | False                            |
| name        | demo                             |
| parent_id   | None                             |
+-------------+----------------------------------+

----- 创建demo用户
# openstack user create --domain default --password-prompt demo
User Password: 123
Repeat User Password: 123
+-----------+----------------------------------+
| Field     | Value                            |
+-----------+----------------------------------+
| domain_id | default                          |
| enabled   | True                             |
| id        | 761444e971da4510990c13259c7ee4e3 |
| name      | demo                             |
+-----------+----------------------------------+

----- 创建user角色
# openstack role create user
+-------+----------------------------------+
| Field | Value                            |
+-------+----------------------------------+
| id    | 4bfb5605bbfa4545a2027e9b3e346b3a |
| name  | user                             |
+-------+----------------------------------+

----- 添加user角色到demo项目和用户
# openstack role add --project demo --user demo user
{% endhighlight %}



### 验证操作

在安装其他服务之前确认身份认证服务正常，首先关闭临时认证令牌机制。编辑 /usr/share/keystone/keystone-dist-paste.ini 文件，从 [pipeline:public_api]，[pipeline:admin_api]，[pipeline:api_v3] 部分删除 admin_token_auth 。

{% highlight text %}
----- 重置OS_TOKEN和OS_URL环境变量
$ unset OS_TOKEN OS_URL

----- 使用admin用户，请求认证令牌
$ openstack --os-auth-url http://127.1:35357/v3 --os-project-domain-id default --os-user-domain-id default \
  --os-project-name admin --os-username admin --os-auth-type password token issue
Password:
+------------+----------------------------------+
| Field      | Value                            |
+------------+----------------------------------+
| expires    | 2016-08-03T15:40:27.899436Z      |
| id         | 558836819dbc40369d91157b0c286832 |
| project_id | f3f0c9353afc4a1ab74d143f85dfabe4 |
| user_id    | 7022c60d8e8848e0bd48f3f2e420ffb7 |
+------------+----------------------------------+

----- 使用demo用户，请求认证令牌
$ openstack --os-auth-url http://127.1:5000/v3 --os-project-domain-id default --os-user-domain-id default \
  --os-project-name demo --os-username demo --os-auth-type password token issue
Password:
+------------+----------------------------------+
| Field      | Value                            |
+------------+----------------------------------+
| expires    | 2016-08-03T15:43:19.799498Z      |
| id         | 39292a170d6b449c918e86590f84e985 |
| project_id | 6423234435ee427aae04053ce1973418 |
| user_id    | 761444e971da4510990c13259c7ee4e3 |
+------------+----------------------------------+
{% endhighlight %}

### 创建OpenStack客户端环境脚本

创建 admin 和 demo 项目和用户创建客户端环境变量脚本，后面会经常使用。

{% highlight text %}
$ cat admin-openrc.sh
export OS_PROJECT_DOMAIN_ID=default
export OS_USER_DOMAIN_ID=default
export OS_PROJECT_NAME=admin
export OS_TENANT_NAME=admin
export OS_USERNAME=admin
export OS_PASSWORD=123
export OS_AUTH_URL=http://127.1:35357/v3
export OS_IDENTITY_API_VERSION=3

$ cat demo-openrc.sh
export OS_PROJECT_DOMAIN_ID=default
export OS_USER_DOMAIN_ID=default
export OS_PROJECT_NAME=demo
export OS_TENANT_NAME=demo
export OS_USERNAME=demo
export OS_PASSWORD=123
export OS_AUTH_URL=http://127.1:5000/v3
export OS_IDENTITY_API_VERSION=3
{% endhighlight %}

使用特定租户和用户运行客户端，你可以在运行之前简单地加载相关客户端脚本，例如：加载 admin-openrc.sh 文件来身份认证服务的环境变量位置和 admin 项目和用户证书：

{% highlight text %}
$ source admin-openrc.sh

----- 请求认证令牌
$ openstack token issue
+------------+----------------------------------+
| Field      | Value                            |
+------------+----------------------------------+
| expires    | 2016-08-03T15:51:53.828491Z      |
| id         | 00813a839de04313a5e1004c8131c869 |
| project_id | f3f0c9353afc4a1ab74d143f85dfabe4 |
| user_id    | 7022c60d8e8848e0bd48f3f2e420ffb7 |
+------------+----------------------------------+
{% endhighlight %}


## Glance，镜像服务

允许用户发现、注册和恢复虚拟机镜像，提供了一个 REST API，允许查询虚拟机镜像的 metadata 并恢复一个实际的镜像。可以存储虚拟机镜像通过不同位置的镜像服务使其可用，就像 OpenStack 对象存储那样从简单的文件系统到对象存储系统。

在此使用 file 作为后端配置镜像服务，能够上传并存储在一个托管镜像服务的控制节点目录中，默认保存在 /var/lib/glance/images/ 目录下。


{% highlight text %}
----- 创建一个数据库
$ mysql -u root -p
CREATE DATABASE glance;
GRANT ALL PRIVILEGES ON glance.* TO 'glance'@'localhost' IDENTIFIED BY 'GLANCE_DBPASS';
GRANT ALL PRIVILEGES ON glance.* TO 'glance'@'%' IDENTIFIED BY 'GLANCE_DBPASS';

----- 创建glance用户
$ openstack user create --domain default --password-prompt glance
User Password: 123
Repeat User Password: 123
+-----------+----------------------------------+
| Field     | Value                            |
+-----------+----------------------------------+
| domain_id | default                          |
| enabled   | True                             |
| id        | 403537aa9450454d824d6c1c6f8460ea |
| name      | glance                           |
+-----------+----------------------------------+

----- 添加admin角色到glance用户和service项目上
$ openstack role add --project service --user glance admin

----- 创建glance服务实体
$ openstack service create --name glance --description "OpenStack Image service" image
+-------------+----------------------------------+
| Field       | Value                            |
+-------------+----------------------------------+
| description | OpenStack Image service          |
| enabled     | True                             |
| id          | 9f36d1a4d41b42c892ee7fb86e11561a |
| name        | glance                           |
| type        | image                            |
+-------------+----------------------------------+

----- 创建镜像服务的API端点
$ openstack endpoint create --region RegionOne image public http://127.1:9292
+--------------+----------------------------------+
| Field        | Value                            |
+--------------+----------------------------------+
| enabled      | True                             |
| id           | 63b4ee31dd7b4055a59f81ac2b1f5160 |
| interface    | public                           |
| region       | RegionOne                        |
| region_id    | RegionOne                        |
| service_id   | 9f36d1a4d41b42c892ee7fb86e11561a |
| service_name | glance                           |
| service_type | image                            |
| url          | http://127.1:9292                |
+--------------+----------------------------------+

$ openstack endpoint create --region RegionOne image internal http://127.1:9292
+--------------+----------------------------------+
| Field        | Value                            |
+--------------+----------------------------------+
| enabled      | True                             |
| id           | 93925120486e451a9c1b413b4866cd94 |
| interface    | internal                         |
| region       | RegionOne                        |
| region_id    | RegionOne                        |
| service_id   | 9f36d1a4d41b42c892ee7fb86e11561a |
| service_name | glance                           |
| service_type | image                            |
| url          | http://127.1:9292                |
+--------------+----------------------------------+

$ openstack endpoint create --region RegionOne image admin http://127.1:9292
+--------------+----------------------------------+
| Field        | Value                            |
+--------------+----------------------------------+
| enabled      | True                             |
| id           | 3b56485471e645c3b3613530015983c0 |
| interface    | admin                            |
| region       | RegionOne                        |
| region_id    | RegionOne                        |
| service_id   | 9f36d1a4d41b42c892ee7fb86e11561a |
| service_name | glance                           |
| service_type | image                            |
| url          | http://127.1:9292                |
+--------------+----------------------------------+
{% endhighlight %}


### 配置组件

{% highlight text %}
----- 安装软件包
# yum install openstack-glance python-glance python-glanceclient
{% endhighlight %}

编辑文件 /etc/glance/glance-api.conf 并完成如下动作。

{% highlight text %}
[DEFAULT]
# 禁止通知
notification_driver = noop
verbose = True

[database]
connection=mysql://glance:GLANCE_DBPASS@127.1/glance

[keystone_authtoken]
auth_uri = http://127.1:5000

identity_uri = http://controller:35357
admin_tenant_name = service
admin_user = glance
admin_password = 123

#auth_url = http://127.1:35357
#auth_plugin = password
#project_domain_id = default
#user_domain_id = default
#project_name = service
#username = glance
#password = 123

[paste_deploy]
flavor = keystone

[glance_store]
default_store = file
filesystem_store_datadir = /var/lib/glance/images/
{% endhighlight %}

编辑文件 /etc/glance/glance-registry.conf 并完成如下动作。

{% highlight text %}
[DEFAULT]
notification_driver = noop
verbose = True

[database]
connection = mysql://glance:GLANCE_DBPASS@127.1/glance

[keystone_authtoken]
auth_uri = http://127.1:5000/v2.0

identity_uri = http://controller:35357
admin_tenant_name = service
admin_user = glance
admin_password = 123

#auth_url = http://127.1:35357
#auth_plugin = password
#project_domain_id = default
#user_domain_id = default
#project_name = service
#username = glance
#password = 123

[paste_deploy]
flavor = keystone
{% endhighlight %}

写入镜像服务数据库。

<!--# su -s /bin/sh -c "glance-manage db_sync" glance-->
{% highlight text %}
# glance-manage db_sync
{% endhighlight %}


完成安装，启动镜像服务、配置他们随机启动：
{% highlight text %}
# systemctl enable openstack-glance-api.service openstack-glance-registry.service
# systemctl start openstack-glance-api.service openstack-glance-registry.service
{% endhighlight %}

在每个客户端脚本中，配置镜像服务客户端使用 2.0 的 API ：

{% highlight text %}
$ echo "export OS_IMAGE_API_VERSION=2" | tee -a admin-openrc.sh demo-openrc.sh
{% endhighlight %}

获得 admin 凭证来获取只有管理员能执行命令的访问权限：

{% highlight text %}
$ source admin-openrc.sh
{% endhighlight %}

下载源镜像，一个很小的镜像，然后添加到 Glance 中：

{% highlight text %}
$ wget http://download.cirros-cloud.net/0.3.4/cirros-0.3.4-x86_64-disk.img
$ glance image-create --name "cirros" \
  --file cirros-0.3.4-x86_64-disk.img \
  --disk-format qcow2 --container-format bare \
  --visibility public --progress
{% endhighlight %}


<!--http://docs.openstack.org/mitaka/install-guide-rdo/
http://docs.openstack.org/liberty/install-guide-rdo/
http://docs.openstack.org/liberty/zh_CN/install-guide-rdo/ -->



## 参考


http://docs.openstack.org/


http://docs.openstack.org/developer/python-openstackclient/command-list.html

{% highlight text %}
{% endhighlight %}
