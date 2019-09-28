---
title: Cargo 实现
layout: project
comments: true
language: chinese
category: [misc]
keywords:
description:
---

DevOps 工具的单点登陆与用户管理系统。

## RESTFul API

{% highlight text %}
----- POST /api/v1/login  登陆
{
	"username": "your name",      用户名
	"password": "your password",  密码
	"type": "account",            登陆方式account(帐号)
}

----- GET  /api/v1/user/current  获取当前用户信息
{
	"id":"00000001",
	"name":"Andy Dufresne",
	"phone":"0752-268888888"
	"avatar":"https://gw.alipayobjects.com/zos/rmsportal/BiazfanxmamNRoxxVxka.png",
	"email":"antdesign@alipay.com",
	"signature":"海纳百川，有容乃大",
	"title":"高级SRE",
	"group":"某某某事业群－某某平台部－某某技术部－SRE",
	"country": "China",
	"geographic": {
		"province": {
			"key":"330000"
			"label":"浙江省",
		},
		"city":  {
			"key":"330100"
			"label":"杭州市",
		}
	},
	"address":"西湖区工专路 77 号",
	"notifyCount":12,
}

----- POST /api/v1/register  注册用户
{
	"type": "account",                 # 可选
	"name": "Foo Bar",                 # 必选，登陆用户名
	"password": "Your PassWord",       # 必选，登陆密码
}

----- GET  /api/v1/sso/user?name=FooBar 用户信息查询
{
	"name": "Foo Bar",                 # 用户名
	"roles": [ "admin", "readonly" ]   # 用户角色
}
{% endhighlight %}

如果 POST 请求中不包含用户名密码，但是 JWT-Cookie 是合法的，那么会自动刷新。



## 表结构

{% highlight sql %}
CREATE DATABASE IF NOT EXISTS `SSO`;
USE `SSO`;

DROP TABLE IF EXISTS `users`;
CREATE TABLE IF NOT EXISTS `users` (
	`id` INT NOT NULL AUTO_INCREMENT PRIMARY KEY,
	`userid` CHAR(64) NOT NULL COMMENT "登陆用户名，内部使用ID",
	`name` CHAR(64) NOT NULL COMMENT "用户名",
	`phone` CHAR(32) COMMENT "手机号，密保验证手机",
	`avatar` CHAR(128) COMMENT "头像地址",
	`email` CHAR(32) COMMENT "邮箱地址",
	`password` VARCHAR(2048) NOT NULL COMMENT "密码，加密之后",
	`signature` VARCHAR(512) COMMENT "个人签名",
	`title` INT COMMENT "职级名称，例如高级SRE",
	`group` CHAR(64) COMMENT "公司内部门名称",

	`contry` INT COMMENT "国家/地区",
	`province` INT COMMENT "所在省",
	`city` INT COMMENT "所在市",
	`address` VARCHAR(1024) COMMENT "详细地址",

	`notifycnt` INT NOT NULL DEFAULT 0 COMMENT "用户收到的消息数",

	`status` INT NOT NULL DEFAULT 0 COMMENT "用户状态 0: INIT",
	`pwdscore` INT NOT NULL DEFAULT 0 COMMENT "密码强度，自动计算，0~100",
	`security` VARCHAR(1024) COMMENT "密保问题，同时会非对称加密",

	`notice` INT NOT NULL DEFAULT 0 COMMENT "消息通知方式，位与方式",

	`gmt_modify` TIMESTAMP NOT NULL ON UPDATE CURRENT_TIMESTAMP,
	`gmt_create` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
	UNIQUE KEY `uk_uid` (`userid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT "用户信息";
INSERT INTO users(id, userid, name, password) VALUES(1, "root", "root", "Unsecure Password");

DROP TABLE IF EXISTS `addrinfo`;
CREATE TABLE IF NOT EXISTS `addrinfo` (
	`id` INT NOT NULL AUTO_INCREMENT PRIMARY KEY,
	`name` CHAR(64) NOT NULL COMMENT "名称",
	`gmt_modify` TIMESTAMP NOT NULL ON UPDATE CURRENT_TIMESTAMP,
	`gmt_create` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT "地址信息，自动会初始化";

DROP TABLE IF EXISTS `roles`;
CREATE TABLE IF NOT EXISTS `roles` (
	`id` INT NOT NULL AUTO_INCREMENT PRIMARY KEY,
	`name` CHAR(64) NOT NULL COMMENT "角色名称，尽量短，会保存到Token中",
	`description` VARCHAR(1024) COMMENT "角色描述",
	`gmt_modify` TIMESTAMP NOT NULL ON UPDATE CURRENT_TIMESTAMP,
	`gmt_create` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
	UNIQUE KEY `uk_name` (`name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT "服务端下发的任务";
INSERT INTO roles(id, name, description) VALUES(1, "readonly", "只读账户");
INSERT INTO roles(id, name, description) VALUES(2, "admin", "管理员");

DROP TABLE IF EXISTS `users_roles`;
CREATE TABLE IF NOT EXISTS `users_roles` (
	`userid` INT NOT NULL COMMENT "用户ID信息",
	`roleid` INT NOT NULL COMMENT "角色ID信息",
	`gmt_modify` TIMESTAMP NOT NULL ON UPDATE CURRENT_TIMESTAMP,
	`gmt_create` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
	PRIMARY KEY (userid, roleid)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT "用户和角色的关联表";
INSERT INTO users_roles(userid, roleid) VALUES(1, 2);
{% endhighlight %}

<!--
中国行政区划信息
https://github.com/mumuy/data_location
https://github.com/modood/Administrative-divisions-of-China

echo统一错误处理
https://zhuanlan.zhihu.com/p/26300634
-->

{% highlight text %}
{% endhighlight %}
