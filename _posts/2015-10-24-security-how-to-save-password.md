---
title: Linux 密码管理
layout: post
comments: true
language: chinese
category: [linux,misc]
keywords: linux,密码,管理
description: 简单介绍下 Linux 中的密码管理。
---

简单介绍下 Linux 中的密码管理。

<!-- more -->

## 简介

常见的加密工具有，seahorse([gnome-keyring](https://wiki.gnome.org/Projects/GnomeKeyring))、[Password Safe](https://www.schneier.com/passsafe.html)、LastPass (跨平台的密码保存工具)、keepass (跨平台的密码保存机制)，在 CentOS 中可以通过 ```yum install keepassx``` 安装。

建议使用 keepass，这样只需要记住密码和对应的库文件即可，然后可以在其它平台上打开。

在 gnome-keyring 官网中，包括了详细的介绍。

### 加密方法

密码算法都是公开的，保密应该依赖于密钥的保密，而不是算法的保密。

对称加密，密钥可同时用于加解密，一般密钥会直接出现在加密代码中，破解的可能性相当大，而且系统管理员很可能知道密钥，进而算出密码原文。由于密钥需要保密，因此需要事先秘密约定，或者用额外的安全信道来传输。

非对称加密，密码的安全性等同于私钥的安全性。密码明文经过公钥加密，要还原密码明文，必须要相应的私钥才行。因此只要保证私钥的安全，密码明文就安全。

<!--
通常利用哈希算法的单向性来保证明文以不可还原的有损方式进行存储。

MD5 和 SHA-1 已破解，虽不能还原明文，但很容易找到能生成相同哈希值的替代明文。而且这两个算法速度较快，暴力破解相对省时，建议不要使用它们。

使用更安全的 SHA-256 等成熟算法，更加复杂的算法增加了暴力破解的难度。但如果遇到简单密码，用彩虹字典的暴力破解法，很快就能得到密码原文。

加入随机 salt 的哈希算法
    密码原文（或经过 hash 后的值）和随机生成的 salt 字符串混淆，然后再进行 hash，最后把 hash 值和 salt 值一起存储。验证密码的时候只要用 salt 再与密码原文做一次相同步骤的运算，比较结果与存储的 hash 值就可以了。这样一来哪怕是简单的密码，在进过 salt 混淆后产生的也是很不常见的字符串，根本不会出现在彩虹字典中。salt 越长暴力破解的难度越大

    具体的 hash 过程也可以进行若干次叠代，虽然 hash 叠代会增加碰撞率，但也增加暴力破解的资源消耗。就算真被破解了，黑客掌握的也只是这个随机 salt 混淆过的密码，用户原始密码依然安全，不用担心其它使用相同密码的应用。

上面这几种方法都不可能得到密码的明文，就算是系统管理员也没办法。对于那些真的忘了密码的用户，网站只能提供重置密码的功能了。

下面的 python 程序演示了如何使用 salt 加 hash 来单向转换密码明文

import os
from hashlib import sha256
from hmac import HMAC

def encrypt_password(password, salt=None):
    """Hash password on the fly."""
    if salt is None:
        salt = os.urandom(8) # 64 bits.

    assert 8 == len(salt)
    assert isinstance(salt, str)

    if isinstance(password, unicode):
        password = password.encode('UTF-8')

    assert isinstance(password, str)

    result = password
    for i in xrange(10):
        result = HMAC(result, salt, sha256).digest()

    return salt + result

这里先随机生成 64 bits 的 salt，再选择 SHA-256 算法使用 HMAC 对密码和 salt 进行 10 次叠代混淆，最后将 salt 和 hash 结果一起返回。

使用的方法很简单：

hashed = encrypt_password('secret password')

下面是验证函数，它直接使用 encrypt_password 来对密码进行相同的单向转换并比较

def validate_password(hashed, input_password):
    return hashed == encrypt_password(input_password, salt=hashed[:8])

assert validate_password(hashed, 'secret password')

虽然只有简短几行，但借助 python 标准库帮助，这已经是一个可用于生产环境的高安全密码加密验证算法了。

总结一下用户密码的存储：
    上善不战而屈人之兵。如果可能不要存任何密码信息 让别人（OpenID）来帮你做事，避开这个问题
    如果非要自己认证，也只能存 不可逆的有损密码信息 。通过单向 hash 和 salt 来保证只有用户知道密码明文
    绝对不能存可还原密码原文的信息 。如果因为种种原因一定要可还原密码原文，请使用非对称加密，并保管好私钥
-->


## Python Keyring

在 Python 中，密码保存以及获取可以使用 keyring 模块，可以从 [pypi keyring](https://pypi.python.org/pypi/keyring) 上下载，可以简单通过 ```easy_install keyring``` 或者 ```pip install keyring``` 安装。

{% highlight text %}
>>> import keyring
>>> keyring.set_password("system", "username", "password")
>>> keyring.get_password("system", "username")
'password'

>>> keyring.get_keyring()                                              // 查看当前使用的keyring
>>> keyring.set_keyring(keyring.backends.file.PlaintextKeyring())      // 设置keyring
{% endhighlight %}

<!--
以 CentOS 为例，设置完成之后可以通过 Passwords and Keys 查看(可以直接在终端输入seahorse)，其加密后的文件保存在 ~/.local/share/keyrings 目录下。
-->

对于 PlaintextKeyring，实际上只是通过 base64 保存，该值可以很容易解密；加密后的文件保存在 ```~/.local/share/python_keyring/keyring_pass.cfg``` 文件中。

{% highlight text %}
[system]
username = cGFzc3dvcmQ=
{% endhighlight %}

可以直接通过 ```base64.decodestring('cGFzc3dvcmQ=')``` 解密。

如果要使用强密码加密措施，可以安装 pycrypto 模块，从官网下载，然后安装。

{% highlight text %}
# python setup.py build && python setup.py install

>>> import keyring
>>> keyring.set_keyring(keyring.backends.file.EncryptedKeyring())      // 设置keyring
{% endhighlight %}

加密后的文件会保存在 ```~/.local/share/python_keyring/crypted_pass.cfg``` 文件中。

<!--
当导入 keyring 时，此时会执行 __init__.py ，然后是 core.py 的 init_backend() 函数，在此会列出有效的 keyring 。
-->

## 其它

简单介绍其它的密码保存方式。

### MySQL

MySQL 的认证密码有两种方式，MySQL-4.1 版本之前是 MySQL323 加密，之后的版本采用 MySQLSHA1 加密。

{% highlight text %}
mysql> select old_password("test");                                      // 老版本加密方式
mysql> select password("test");                                          // 新版本加密方式

mysql> select user, password from mysql.user;                            // 查看mysql中的应用名和密码
mysql> update user set password=password('password') where user='root';  // 更新密码
mysql> flush privileges;                                                 // 刷新权限
{% endhighlight %}

其中密码保存在 mysql/user.MYD 文件中。

### Chrome FirFox

两者都会保存密码，并在需要的时候自动填充。

Chrome 密码管理器的进入方式：右侧扳手图标 → 设置 → 显示高级设置 → 密码和表单 → 管理已保存的密码；或者直接在地址栏中打开 [chrome://chrome/settings/passwords](chrome://chrome/settings/passwords)；Chrome 后端会用一些系统提供的密码保存方式，不同平台有所区别，如 Linux 中是 gnome-keyring 。

FireFox 通过自己的方式保存密码，可以设置主密码，通常密码保存在 key3.db 中，如在 CentOS 中，保存在 ```~/.mozilla/firefox/cud38a2f.default/key3.db```，可以参考 [密码管理器--在 Firefox 中记住、删除和更改已保存的密码](https://support.mozilla.org/zh-CN/kb/password-manager-remember-delete-change-passwords) 。

<!--
genrsa
### Google Chrome:
~/.config/google-chrome/Default/Login Data
### Chromium:
~/.config/chromium/Default/Login Data

### Google Chrome
    google-chrome --password-store=basic

### Chromium
    chromium --password-store=basic

### Google Chrome:
sqlite3 -header -csv -separator "," ~/.config/google-chrome/Default/Login\ Data "SELECT * FROM logins" > ~/Passwords.csv

### Chromium
sqlite3 -header -csv -separator "," ~/.config/chromium/Default/Login\ Data "SELECT * FROM logins" > ~/Passwords.csv


获取WIFI和Chrome用户名密码
http://www.ftium4.com/chrome-cookies-encrypted-value-python.html
https://github.com/lijiejie/chromePass
http://www.lijiejie.com/python-get-chrome-all-saved-passwords/
https://github.com/hassaanaliw/chromepass/blob/master/chromepass.py
-->

### John the Ripper

一个开源的密码破解工具，可以参考 [官方网站](http://www.openwall.com/john/) 。

{% highlight text %}
$ tar -xvf john-1.8.0.tar.xz && cd john-1.8.0/run
$ ./john --test

# unshadow /etc/passwd /etc/shadow &lt; mypasswd
$ ./john mypasswd
{% endhighlight %}

通过 ```make``` 可以查看所支持的系统，当然可以 ```make clean generic``` 产生通用的程序产生通用的程序产生通用的程序，产生的程序保存在 ```../run``` 目录下，可以通过 ```john --test``` 测试。

<!--
http://blog.sina.com.cn/s/blog_64d959260100un5x.html
-->

### 使用OpenSSL加密

详细可以参考 [OpenSSL使用指南](/reference/linux/OpenSSL.pdf) 。

{% highlight text %}
----- 使用Base64编码方法(Base64 Encoding)进行加密/解密
$ echo "Welcome to Linux" | openssl enc -base64
$ echo "V2VsY29tZSB0byBMaW51eAo=" | openssl enc -base64 -d

----- 使用对称加密
$ echo "Welcome to Linux" | openssl enc -aes-256-cbc -a         加密，需要秘密
enter aes-256-cbc encryption password: 123456
Verifying - enter aes-256-cbc encryption password:
U2FsdGVkX1+yYQz9vEKnm56GoXo7F3I/kHP2C5KW4lqBcneAeDIXa6WU29pYhPAL
$ echo "U2FsdGVkX1+yYQz9vEKnm56GoXo7F3I/kHP2C5KW4lqBcneAeDIXa6WU29pYhPAL" | openssl enc -aes-256-cbc -d -a
enter aes-256-cbc decryption password: 123456
Welcome to Linux

----- 输出到文件，然后解密
$ echo "OpenSSL" | openssl enc -aes-256-cbc > openssl.dat
$ openssl enc -aes-256-cbc -d -in openssl.dat
----- 加密文件
$ openssl enc -aes-256-cbc -in /etc/services -out services.dat
$ openssl enc -aes-256-cbc -d -in services.dat > services.txt
----- 加密文件夹
$ tar cz /etc | openssl enc -aes-256-cbc -out etc.tar.gz.dat
$ openssl enc -aes-256-cbc -d -in etc.tar.gz.dat | tar xz

----- 使用公私钥非对称加密
------- 1. 生成私钥，1024指定长度(bit)，默认512
$ openssl genrsa -out test.key 1024
Generating RSA private key, 1024 bit long modulus
.......++++++
....................................++++++
e is 65537 (0x10001)
------- 2. 生成公钥
$ openssl rsa -in test.key -pubout -out test_pub.key
writing RSA key
------- 3. 使用公钥加密，-in(要加密文件) -inkey(公钥) -pubin(用纯公钥文件加密) -out(加密后文件)
$ openssl rsautl -encrypt -in hello.txt -inkey test_pub.key -pubin -out hello.en
------- 4. 使用私钥解密，-in(被加密文件) -inkey(私钥) -out(解密后文件)
$ openssl rsautl -decrypt -in hello.en -inkey test.key -out hello.de
{% endhighlight %}


## 参考

安全审计工具可以参考 [cisofy.com/lynis](https://cisofy.com/lynis/) ； 关于破解工具可以参考 [THC-Hydra](https://www.thc.org/thc-hydra/) 支持多种协议。

关于存储密码的方法可以参考 [Storing User Passwords Securely: hashing, salting, and Bcrypt](http://dustwell.com/how-to-handle-passwords-bcrypt.html) 或者 [本地文档](/reference/linux/Storing_User_Passwords_Securely.maff) 。一篇很不错的介绍数字签名的文章 [What is a Digital Signature?](http://www.youdzone.com/signature.html) 或者 [本地文档](/reference/linux/What_is_a_Digital_Signature.maff) 。

<!--
常见的破解工具可以参考 <a href="http://sectools.org/tag/pass-audit/">Top 125 Network Security Tools</a> 。




keys[] (备注：为了方便问题定位采用字符串保存，大小通过KEY_SIZE定义，一般为GCRY_CIPHER的整数倍)
   明文二级密钥组，含有多个值，用于对配置文件中的各个组件进行加密，在生成的passphrase中包含了相应信息；
password
   用户的明文密码，线上无法直接获取，为了定位问题，提供工具解密。
passphrase (备注：长度由GCRY_CIPHER的block决定，例如GCRY_CIPHER_AES256为16B，当不足采用Zero Padding)
   也就是保存在配置文件中的内容，其格式为 $salt$encrypted_password$seq ，分别为盐值、加密信息、key序号。
rootkey (备注：长度由GCRY_HASH算法决定，程序内部固定，默认不暴露无法获取)
   根密钥，用于对key进行加密，通过不同组件生成，包括了文件名、文件属性、随机值(**同时作为salt**)、硬编码组成。
   首先对组件执行XOR加密操作，然后
cipherfile
   生成的加密文件，按照一定格式保存了keys[]信息。注意：保存文件名为".uagent.rc"(强制)，权限为 "-rw-------"(强制)，一般部署时用户为monitor:monitor。

API:
int generate_cipherfile (const char *cipherfile, int keylen);
   用于生成cipherfile，可以指定文件，**注意**如果源文件存在会被直接覆盖。
   流程：
     1. 根据各个组件生成rootkey。
	 2. 根据上述的rootkey对二级密钥组进行加密，依次保存，IV随机生成，保存到文件中；NOTE: 所有keys[]使用相同IV。
	 3. 计算文件的HASH值并保存。

int decrypt_passphrase (char *cipherfile, char *passphrase, char *out, int outlen);
   解密passphrase，用于问题定位。
   流程：
     1. 解析输入字符串。
	 2. 读取cipherfile并通过rootkey解密获取keys[]，并选择相应的key。
	 3. 然后解密。

int generate_passphrase (char *cipherfile, char *password, char *out, int outlen);
   用于生成passphrase。
   流程：
     1. 读取cipherfile，获取所需的keys[]，并选择其中一个作为key。
	 2. 通过明文密码和salt生成真正加密所需的key。
	 3. 生成加密密文，其中IV使用传入的部分二级密钥 (FIXME:可能存在风险，建议使用salt进行HASH生成)。
	 4. 拼装返回passphrase。
注意：IV 大小需要保证为 Block Size，需要加密的信息需要保证是 Block Size 的整数倍。

密码保存方案：


根密钥保存方案：

+---------------+
|   密钥组件A   |
+---------------+



+--------------------+
| 明文数据(plaintxt) |
+--------------------+
         |
         V
+--------------------+
   计算分组                   安全随机生成器 
+--------------------+      
         |
         V
+--------------------+       +----------+          +------+
填充                 |       | 加密密钥 |          | IV值 |
+--------------------+
         |
         V
+------------------------------------------------------------------------------------------------------------------
对称加密算法(通过CBC方式填充)
+------------------------------------------------------------------------------------------------------------------

+------------------------------------------------------------------------------------------------------------------
加密数据
+------------------------------------------------------------------------------------------------------------------



文件格式：

+--------------+-------------------+---------------+-----------------+------------------+
| 'SECV1 '(6B) |  FILE_HASH(32B)   | RESERVED(32B) | RANDOM KEY(20B) | INIT VECTOR(32B) |
+--------------+-----------+-------+---+-----------+------------+----+------------------+

| KEYS NUM(4B) |  SUBKEY1  |  SUBKEY2  |  ... ...  |  SUBKEY N  |
+--------------+-----------+-----------+-----------+------------+

SECV1 MagicCode 版本号 1；
FILE_HASH 文件的SHA256值；
RESERVED 保留字段，默认填充0x05；


cipherfile


GCRY_HASH  定义HASH算法，文件HASH值计算
FILE_HASH  根据Hash之后的文件内容计算。



-->

{% highlight text %}
{% endhighlight %}
