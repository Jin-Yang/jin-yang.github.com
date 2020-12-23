---
title: ETCD BoltDB
layout: post
comments: true
language: chinese
category: [program,golang,linux]
keywords: golang,go,etcd,boltdb
description: ETCD 后端存储采用的是 BBolt 存储引擎，其前身是 BoltDB ，这是一款 golang 实现的嵌入式 KV 存储引擎，参考的是 LMDB，支持事务、ACID、MVCC、ZeroCopy、BTree等特性。
---

ETCD 后端存储采用的是 BBolt 存储引擎，其前身是 BoltDB ，这是一款 golang 实现的嵌入式 KV 存储引擎，参考的是 LMDB，支持事务、ACID、MVCC、ZeroCopy、BTree等特性。

<!-- more -->

## 使用

直接通过 `go get github.com/boltdb/bolt/...` 安装源码。

### 示例

{% highlight go %}
package main
 
import (
	"bytes"
	"fmt"
	"github.com/boltdb/bolt"
	"log"
)
 
func main() {
	arrBucket := []string{"a", "b"}
	mgr, _ := NewBoltManager("my.db", arrBucket)
	mgr.Add("a", []byte("11"))
	mgr.Add("a", []byte("22"))
	id33, _ := mgr.Add("a", []byte("33"))
	mgr.Add("a", []byte("22"))
 
	mgr.Add("b", []byte("11"))
	mgr.Add("b", []byte("22"))
 
	log.Println("Select a=>>>>>>>>>>>>>>>>>>>")
	mgr.Select("a")
	log.Println("Select b=>>>>>>>>>>>>>>>>>>>")
	mgr.Select("b")
 
	log.Println("RemoveID a=>>>>>>>>>>>>>>>>>>>")
	mgr.RemoveID("a", []byte(fmt.Sprintf("%d", id33)))
 
	log.Println("RemoveValTransaction a=>>>>>>>>>>>>>>>>>>>")
	mgr.RemoveValTransaction("a", []byte("22"))
	log.Println("Select a=>>>>>>>>>>>>>>>>>>>")
	mgr.Select("a")
 
	//清理Bucket
	for _, v := range arrBucket {
		mgr.RemoveBucket(v)
	}
	mgr.Close()
 
}
 
//BlotDB的管理类
type BoltManager struct {
	db *bolt.DB "数据库类"
}
 
//创建库管理,并生成Bucket
func NewBoltManager(dbPath string, bucket []string) (*BoltManager, error) {
	//  bolt.Open("my.db", 0600, &bolt.Options{Timeout: 1 * time.Second,ReadOnly: true})
	db, err := bolt.Open(dbPath, 0644, nil)
	if err != nil {
		return nil, err
	}
	err = db.Update(func(tx *bolt.Tx) error {
		for _, v := range bucket {
			_, err := tx.CreateBucketIfNotExists([]byte(v))
			if err != nil {
				return err
			}
		}
		return nil
	})
 
	if err != nil {
		return nil, err
	}
	return &BoltManager{db}, nil
}
 
//关库数据库
func (m *BoltManager) Close() error {
	return m.db.Close()
}
 
//移除Bucket
func (m *BoltManager) RemoveBucket(bucketName string) (err error) {
	err = m.db.Update(func(tx *bolt.Tx) error {
		return tx.DeleteBucket([]byte(bucketName))
	})
	return err
}
 
//组Bucket增加值
func (m *BoltManager) Add(bucketName string, val []byte) (id uint64, err error) {
 
	err = m.db.Update(func(tx *bolt.Tx) error {
		b := tx.Bucket([]byte(bucketName))
		id, _ = b.NextSequence() //sequence uint64
		bBuf := fmt.Sprintf("%d", id)
		return b.Put([]byte(bBuf), val)
	})
	return
}
 
//遍历
func (m *BoltManager) Select(bucketName string) (err error) {
	m.db.View(func(tx *bolt.Tx) error {
		b := tx.Bucket([]byte(bucketName))
		b.ForEach(func(k, v []byte) error {
			log.Printf("key=%s, value=%s\n", string(k), v)
			return nil
		})
		return nil
	})
	return nil
}
 
//移除指定Bucket中指定ID
func (m *BoltManager) RemoveID(bucketName string, id []byte) error {
	err := m.db.Update(func(tx *bolt.Tx) error {
		b := tx.Bucket([]byte(bucketName))
		return b.Delete(id)
	})
	return err
}
 
//移除指定Bucket中指定Val
func (m *BoltManager) RemoveVal(bucketName string, val []byte) (err error) {
	var arrID []string
	arrID = make([]string, 1)
	err = m.db.View(func(tx *bolt.Tx) error {
		b := tx.Bucket([]byte(bucketName))
		c := b.Cursor()
		for k, v := c.First(); k != nil; k, v = c.Next() {
			log.Printf("key=%s, value=%s\n", k, string(v))
			if bytes.Compare(v, val) == 0 {
				arrID = append(arrID, string(k))
			}
		}
		return nil
	})
 
	err = m.db.Update(func(tx *bolt.Tx) error {
		b := tx.Bucket([]byte(bucketName))
		for _, v := range arrID {
			b.Delete([]byte(v))
			log.Println("Del k:", v)
		}
		return nil
	})
 
	return err
}
 
//查找指定值
func (m *BoltManager) SelectVal(bucketName string, val []byte) (
	arr []string,
	err error,
) {
	arr = make([]string, 0, 1)
	err = m.db.View(func(tx *bolt.Tx) error {
		c := tx.Bucket([]byte(bucketName)).Cursor()
		for k, v := c.First(); k != nil; k, v = c.Next() {
			if bytes.Compare(v, val) == 0 {
				arr = append(arr, string(k))
			}
		}
		return nil
	})
	return arr, err
}
 
//在事务中，移除指定Bucket中指定Val
func (m *BoltManager) RemoveValTransaction(bucketName string, val []byte) (
	count int,
	err error,
) {
	arrID, err1 := m.SelectVal(bucketName, val)
	if err1 != nil {
		return 0, err1
	}
	count = len(arrID)
	if count == 0 {
		return count, nil
	}
 
	tx, err1 := m.db.Begin(true)
	if err1 != nil {
		return count, err1
	}
	b := tx.Bucket([]byte(bucketName))
	for _, v := range arrID {
		if err = b.Delete([]byte(v)); err != nil {
			log.Printf("删除ID(%s)失败! 执行回滚. err:%s \r\n", v, err)
			tx.Rollback()
			return
		}
		log.Println("删除ID(", v, ")成功!")
	}
	err = tx.Commit()
	return
}
{% endhighlight %}


### 打开数据库

{% highlight go %}
package main

import (
	"log"
	"time"
	"github.com/boltdb/bolt"
)

func main() {
	// 打开数据库
	db, err := bolt.Open("my.db", 0600, &bolt.Options{Timeout: 1 * time.Second})
	if err != nil {
		log.Fatal(err)
	}
	defer db.Close()
}
{% endhighlight %}

第一参数指定路径，不存在则创建， 第二个为文件操作，第三个参数是可选参数，内部可以配置只读和超时时间等。

BoltDB 只能单点写入和读取，如果多个同时操作的话后者会被挂起直到前者关闭操作为止，一次只允许一个读写事务，但一次允许多个只读事务。

### 读写事务

可以使用 `DB.Update()` 来完成。

{% highlight go %}
err := db.Update(func(tx *bolt.Tx) error {
	return nil
})
{% endhighlight %}

在闭包中，结束时返回 `nil` 来提交事务，返回一个错误在任何点回滚事务。

### 批量读写事物

每一次新的事物都需要等待上一次事物的结束，此时可以通过 `DB.Batch()` 批处理来完成。

{% highlight go %}
err := db.Batch(func(tx *bolt.Tx) error {
	return nil
})
{% endhighlight %}

在批处理过程中如果某个事务失败了，批处理会多次调用这个函数函数返回成功则成功，如果中途失败了，则整个事务会回滚。

### 只读事务

只读事务可以使用 `DB.View()` 来完成。

{% highlight go %}
err := db.View(func(tx *bolt.Tx) error {
	return nil
})
{% endhighlight %}

不改变数据的操作都可以通过只读事务来完成， 您只能检索桶、检索值，或在只读事务中复制数据库。

### 启动事务

`DB.Begin()` 启动函数包含在 `DB.Update()` 和 `DB.Batch()` 中，该函数启动事务开始执行事务并返回结果关闭事务，有时可能需要手动启动事物你可以使用 `Tx.Begin()` 来开始，切记不要忘记关闭事务。

{% highlight go %}
// Start a writable transaction.
tx, err := db.Begin(true)
if err != nil {
	return err
}
defer tx.Rollback()

// Use the transaction...
_, err := tx.CreateBucket([]byte("MyBucket"))
if err != nil {
	return err
}

// Commit the transaction and check for error.
if err := tx.Commit(); err != nil {
	return err
}
{% endhighlight %}

### 使用桶

桶是数据库中键/值对的集合，桶中的所有键必须是唯一的，可以使用 `DB.CreateBucket()` 创建一个桶。

{% highlight go %}
db.Update(func(tx *bolt.Tx) error {
	b, err := tx.CreateBucket([]byte("MyBucket"))
	if err != nil {
		return fmt.Errorf("create bucket: %s", err)
	}
	return nil
})
{% endhighlight %}

也可以使用 `Tx.CreateBucketIfNotExists()` 来创建桶，该函数会先判断是否已经存在该桶不存在即创建，删除桶可以使用 `Tx.DeleteBucket()` 来完成。

### 使用 KV 对

存储键值对到桶里可以使用 `Bucket.Put()` 来完成。

{% highlight go %}
db.Update(func(tx *bolt.Tx) error {
	b := tx.Bucket([]byte("MyFriendsBucket"))
	err := b.Put([]byte("one"), []byte("zhangsan"))
	return err
})
{% endhighlight %}

获取键值 `Bucket.Get()` 。

{% highlight go %}
db.View(func(tx *bolt.Tx) error {
	b := tx.Bucket([]byte("MyFriendsBucket"))
	v := b.Get([]byte("one"))
	fmt.Printf("The answer is: %s\n", v)
	return nil
})
{% endhighlight %}

`get()` 函数不返回一个错误，除非有某种系统故障，如果键存在，那么它将返回它的值；如果它不存在，那么它将返回 `nil`。

<!----
9.桶的自增

利用nextsequence()功能，你可以让boltdb生成序列作为你键值对的唯一标识。见下面的示例。

func (s *Store) CreateUser(u *User) error {
    return s.db.Update(func(tx *bolt.Tx) error {
        // 创建users桶
        b := tx.Bucket([]byte("users"))

        // 生成自增序列
        id, _ = b.NextSequence()
        u.ID = int(id)

        // Marshal user data into bytes.
        buf, err := json.Marshal(u)
        if err != nil {
            return err
        }

        // Persist bytes to users bucket.
        return b.Put(itob(u.ID), buf)
    })
}

// itob returns an 8-byte big endian representation of v.
func itob(v int) []byte {
    b := make([]byte, 8)
    binary.BigEndian.PutUint64(b, uint64(v))
    return b
}

type User struct {
    ID int
    ...
}

10. 迭代键

boltdb以桶中的字节排序顺序存储键。这使得在这些键上的顺序迭代非常快。要遍历键，我们将使用游标Cursor()：

db.View(func(tx *bolt.Tx) error {
    // Assume bucket exists and has keys
    b := tx.Bucket([]byte("MyBucket"))

    c := b.Cursor()

    for k, v := c.First(); k != nil; k, v = c.Next() {
        fmt.Printf("key=%s, value=%s\n", k, v)
    }

    return nil
})

游标Cursor()允许您移动到键列表中的特定点，并一次一个地通过操作键前进或后退。
光标上有以下函数：

First()  移动到第一个健.
Last()   移动到最后一个健.
Seek()   移动到特定的一个健.
Next()   移动到下一个健.
Prev()   移动到上一个健.

这些函数中的每一个都返回一个包含(key []byte, value []byte)的签名。当你有光标迭代结束，next()将返回一个nil。在调用next()或prev()之前，你必须寻求一个位置使用first()，last()，或seek()。如果您不寻求位置，则这些函数将返回一个nil键。
在迭代过程中，如果键为非零，但值为0，则意味着键指向一个桶而不是一个值。用桶.bucket()访问子桶。
11.前缀扫描

遍历一个key的前缀，你可以结合seek()和bytes.hasprefix()：

db.View(func(tx *bolt.Tx) error {
    // Assume bucket exists and has keys
    c := tx.Bucket([]byte("MyBucket")).Cursor()

    prefix := []byte("1234")
    for k, v := c.Seek(prefix); bytes.HasPrefix(k, prefix); k, v = c.Next() {
        fmt.Printf("key=%s, value=%s\n", k, v)
    }

    return nil
})

12.范围扫描

另一个常见的用例是扫描范围，例如时间范围。如果你使用一个合适的时间编码，如rfc3339然后可以查询特定日期范围的数据：

db.View(func(tx *bolt.Tx) error {
    // Assume our events bucket exists and has RFC3339 encoded time keys.
    c := tx.Bucket([]byte("Events")).Cursor()

    // Our time range spans the 90's decade.
    min := []byte("1990-01-01T00:00:00Z")
    max := []byte("2000-01-01T00:00:00Z")

    // Iterate over the 90's.
    for k, v := c.Seek(min); k != nil && bytes.Compare(k, max) <= 0; k, v = c.Next() {
        fmt.Printf("%s: %s\n", k, v)
    }

    return nil
})

13.循环遍历每一个

如果你知道所在桶中拥有键，你也可以使用ForEach()来迭代：

db.View(func(tx *bolt.Tx) error {
    // Assume bucket exists and has keys
    b := tx.Bucket([]byte("MyBucket"))

    b.ForEach(func(k, v []byte) error {
        fmt.Printf("key=%s, value=%s\n", k, v)
        return nil
    })
    return nil
})

14.嵌套桶

还可以在一个键中存储一个桶，以创建嵌套的桶：

func (*Bucket) CreateBucket(key []byte) (*Bucket, error)
func (*Bucket) CreateBucketIfNotExists(key []byte) (*Bucket, error)
func (*Bucket) DeleteBucket(key []byte) error

15.数据库备份

boltdb是一个单一的文件，所以很容易备份。你可以使用TX.writeto()函数写一致的数据库。如果从只读事务调用这个函数，它将执行热备份，而不会阻塞其他数据库的读写操作。
默认情况下，它将使用一个常规文件句柄，该句柄将利用操作系统的页面缓存。有关优化大于RAM数据集的信息，请参见Tx文档。
一个常见的用例是在HTTP上进行备份，这样您就可以使用像cURL这样的工具来进行数据库备份：

func BackupHandleFunc(w http.ResponseWriter, req *http.Request) {
    err := db.View(func(tx *bolt.Tx) error {
        w.Header().Set("Content-Type", "application/octet-stream")
        w.Header().Set("Content-Disposition", `attachment; filename="my.db"`)
        w.Header().Set("Content-Length", strconv.Itoa(int(tx.Size())))
        _, err := tx.WriteTo(w)
        return err
    })
    if err != nil {
        http.Error(w, err.Error(), http.StatusInternalServerError)
    }
}

然后您可以使用此命令进行备份：
$ curl http://localhost/backup > my.db
或者你可以打开你的浏览器以http://localhost/backup，它会自动下载。
如果你想备份到另一个文件，你可以使用TX.copyfile()辅助功能。
16.统计

数据库对运行的许多内部操作保持一个运行计数，这样您就可以更好地了解发生了什么。通过捕捉这些数据的快照，我们可以看到在这个时间范围内执行了哪些操作。
例如，我们可以开始一个goroutine里记录统计每10秒：

go func() {
    // Grab the initial stats.
    prev := db.Stats()

    for {
        // Wait for 10s.
        time.Sleep(10 * time.Second)

        // Grab the current stats and diff them.
        stats := db.Stats()
        diff := stats.Sub(&prev)

        // Encode stats to JSON and print to STDERR.
        json.NewEncoder(os.Stderr).Encode(diff)

        // Save stats for the next loop.
        prev = stats
    }

17.只读模式

有时创建一个共享的只读boltdb数据库是有用的。对此，设置options.readonly国旗打开数据库时。只读模式使用共享锁允许多个进程从数据库中读取，但它将阻塞任何以读写方式打开数据库的进程。

db, err := bolt.Open("my.db", 0666, &bolt.Options{ReadOnly: true})
if err != nil {
    log.Fatal(err)
}

18.移动端支持（ios/android）

boltdb能够运行在移动设备上利用的工具结合特征GoMobile。创建一个结构体，包含您的数据库逻辑和参考一个bolt.db与初始化contstructor需要在文件路径，数据库文件将存储。使用这种方法，Android和iOS都不需要额外的权限或清理。

func NewBoltDB(filepath string) *BoltDB {
    db, err := bolt.Open(filepath+"/demo.db", 0600, nil)
    if err != nil {
        log.Fatal(err)
    }

    return &BoltDB{db}
}

type BoltDB struct {
    db *bolt.DB
    ...
}

func (b *BoltDB) Path() string {
    return b.db.Path()
}

func (b *BoltDB) Close() {
    b.db.Close()
}

数据库逻辑应定义为此包装器结构中的方法。
要从本机语言初始化此结构（两个平台现在都将本地存储与云同步）。这些片段禁用数据库文件的功能）：
Android

String path;
if (android.os.Build.VERSION.SDK_INT >=android.os.Build.VERSION_CODES.LOLLIPOP){
    path = getNoBackupFilesDir().getAbsolutePath();
} else{
    path = getFilesDir().getAbsolutePath();
}
Boltmobiledemo.BoltDB boltDB = Boltmobiledemo.NewBoltDB(path)

IOS

- (void)demo {
    NSString* path = [NSSearchPathForDirectoriesInDomains(NSLibraryDirectory,
                                                          NSUserDomainMask,
                                                          YES) objectAtIndex:0];
    GoBoltmobiledemoBoltDB * demo = GoBoltmobiledemoNewBoltDB(path);
    [self addSkipBackupAttributeToItemAtPath:demo.path];
    //Some DB Logic would go here
    [demo close];
}

- (BOOL)addSkipBackupAttributeToItemAtPath:(NSString *) filePathString
{
    NSURL* URL= [NSURL fileURLWithPath: filePathString];
    assert([[NSFileManager defaultManager] fileExistsAtPath: [URL path]]);

    NSError *error = nil;
    BOOL success = [URL setResourceValue: [NSNumber numberWithBool: YES]
                                  forKey: NSURLIsExcludedFromBackupKey error: &error];
    if(!success){
        NSLog(@"Error excluding %@ from backup %@", [URL lastPathComponent], error);
    }
    return success;
}

19.查看工具

1.下载工具
go get github.com/boltdb/boltd
然后编译cmd下的main文件生成可执行文件改名为boltd
拷贝boltd到 *.db同级目录，执行如下：

https://segmentfault.com/a/1190000010098668





## Backend

数据的持久化写入以 Page 为单位，当经过 CURD 之后，会出现空洞数据页，BoltDB 会将其添加到 FreeList 链表中，而非直接返还给操作系统，这就会导致文件只增大，而不会因为数据删除而减少。

由于采用了 BTree 索引，那么会带来随机写入，所以在有写入瓶颈的场景中，可以使用 SSD 来提高性能。

内部直接采用 mmap 将文件映射到内存，完全利用操作系统的虚拟内存空间与物理内存动态映射、换出的机制，而没有去做动态的映射策略。

https://lrita.github.io/2017/05/22/boltdb-persistence-3/

DB 整个boltdb的持有者，跟boltdb相关操作都要通过调用其方法发起，是boltdb的一个抽象；
Stats 调用DB.Stats()方法返回的数据结构，内包含一些boltdb内部的计数信息，可以供用户查看；
Bucket 类似于表的一个概念，在boltdb相关数据必须存在一个Bucket下，不同Bucket下的数据相互隔离，每个Bucket下 有一个单调递增的Sequence，类似于自增主键；
BucketStats Bucket的一些统计信息；
Tx boltdb的事务数据结构，在boltdb中，全部的对数据的操作必须发生在一个事务之内，boltdb的并发读取都在此实现；
Cursor 是内部B-TREE的一个迭代器，用于遍历数据库，提供First/Last/Seek等操作；


node 用来存储每个Bucket中的一部分Key-Value，每个Bucket中的node组织成一个B-TREE；
inode 记录Key-Value对的数据结构，每个node内包含一个inode数组，inode是K-V在内存中缓存的记录，该记录落到磁盘上 时，记录为leafPageElement；
leafPageElement 磁盘上记录具体Key-Value的索引；
page 用户记录持久化文件中每个区域的重要信息，同时page分为很多种类，不同page存储不同的数据信息


## Node VS. Page

两者是相互对应的，其中 Node 为内存中数据的存储模式，而 Page 是磁盘中存储的数据格式。另外，还有 inode(Internal Node) 用来指向页中的一个元素或者还未添加到页中的成员。

其中读写相关的在 node.go 文件中。

https://mp.weixin.qq.com/s?__biz=MzU2NDUwMjU3Ng==&mid=2247484048&idx=1&sn=2d3d41ca0f7b7f0d5c558cd5f69336c7&chksm=fc4b4bcfcb3cc2d97d20f95a3b50b1bf928f1d5aa8021572db8df338fbe7692713bf5025cbeb&scene=21#wechat_redirect

因为所有的页操作都交给了操作系统，那么 BoltDB 中也的大小与操作系统页大小相同，也就是 getpagesize() 的返回值，通常是 4k 。

每个页开始的几个字节存储的是页的原始数据：

type page struct {
    id       pgid       // 该页的编号
    flags    uint16     // 页的类型
    count    uint16     // 记录具体数据类型的计数，不同场景值含义有所区别；
						// freelistPageFlag 保存了freelist中pgid数组中元素的个数；
                        // 其它类型保存的是inode个数
    overflow uint32     // 当写操作数据量大于1个page大小时，该字段记录超出的page数，例如：写入12k的数据，
                        // page大小为4k，则page.overflow=2，标明page header后的2个page大小区域也是该page的区域。
    ptr      uintptr    // 具体的数据结构，被称为MetaData，不同类型其大小有所区别，例如pageHeaderSize、leafPageElementSize等
}

页的类型有 branchPageFlag(分支节点)、leafPageFlag(叶子节点)、metaPageFlag(Meta页)、freelistPageFlag(FreeList页) 几种。

https://mp.weixin.qq.com/s?__biz=MzU2NDUwMjU3Ng==&mid=2247484024&idx=1&sn=0c4ff1f4c0c3b449f8e8402c3fa324dd&chksm=fc4b4b27cb3cc231499598d2027aef19e6535927863dedb535a2d832b541fec4b58ed5706b7c&scene=21#wechat_redirect


每个page对应对应一个磁盘上的数据块。这个数据块的layout为:

| page struct data | page element items | k-v pairs |

其分为3个部分：

第一部分page struct data是该page的header，存储的就是pagestruct的数据。
第二部分page element items其实就是node(下面会讲)的里inode的持久化部分数据。
第三部分k-v pairs存储的是inode里具体的key-value数据。


type meta struct {
    magic    uint32  // 存储魔数0xED0CDAED
    version  uint32  // 标明存储格式的版本，现在是2
    pageSize uint32  // 标明每个page的大小
    flags    uint32  // 当前已无用
    root     bucket  // 根Bucket
    freelist pgid    // 标明当前freelist数据存在哪个page中
    pgid     pgid    //
    txid     txid    //
    checksum uint64  // 以上数据的校验和，校验数据是否损坏
}


## FreeList

BoltDB 会复用磁盘空间而非释放，这样就需要通过 FreeList 机制实现，其数据结构为：

type freelist struct {
    ids     []pgid          // all free and available free page ids.
    pending map[txid][]pgid // mapping of soon-to-be free page ids by tx.
    cache   map[pgid]bool   // fast lookup of all free and pending page ids.
}

其有三部分组成，ids记录了当前缓存着的空闲page的pgid，cache中记录的也是这些pgid，采用map记录 方便快速查找。



### 申请

当用户需要页时，会调用 `freelist.allocate()` 申请连续地址，如果有满足需求的页那么就会从缓存中删除，并将对应的 pageid 返回，不满足时返回 0 ，以为起始的两个作为 MetaPage ，所以有效 ID 肯定不会为 0 。

当某个事务产生无用页时，会调用 `freelist.free()` 将指定页添加到 Pending 和 Cache 中，当下一个写事务开启时，会将没有事务引用 Pending 中的页迁移到 ids 的缓存中；之所以这样做，是为了支持事务的回滚和并发读事务，从而实现 MVCC 。


当发起一个读事务时，Tx单独复制一份meta信息，从这份独有的meta作为入口，可以读出该meta指向的数据， 此时即使有一个写事务修改了相关key的数据，新修改的数据只会被写入新的page，读事务持有的page会进入pending 池，因此该读事务相关的数据并不会被修改。只有该page相关的读事务都结束时，才会从pending池进入到cache池 中，从而被复用修改。

当写事务更新数据时，并不直接覆盖老数据，而且分配一个新的page将更新后的数据写入，然后将老数据占用的page 放入pending池，建立新的索引。当事务需要回滚时，只需要将pending池中的page释放，将索引回滚即完成数据的 回滚。这样加速了事务的回滚。减少了事务缓存的内存使用，同时避免了对正在读的事务的干扰。


## B-Tree



Cursor是遍历Bucket的迭代器，其声明是:

type elemRef struct {
    page  *page
    node  *node
    index int
}

type Cursor struct {
	bucket *Bucket     // parent Bucket
	stack  []elemRef   // 遍历过程中记录走过的page-id或者node，elemRef中的page、node同时只能有一个存在
}

type Bucket struct {
    *bucket
    tx       *Tx                // the associated transaction
    buckets  map[string]*Bucket // subbucket cache
    page     *page              // inline page reference
    rootNode *node              // materialized node for the root page.
    nodes    map[pgid]*node     // node cache
    FillPercent float64
}

type node struct {
    bucket     *Bucket
    isLeaf     bool	// 标记该节点是否为叶子节点，决定inode中记录的是什么
    unbalanced bool	// 当该node上有删除操作时，标记为true，当Tx执行Commit时，会执行rebalance，将inode重新排列
    spilled    bool
    key        []byte	// 当加载page变成node缓存时，将该node下边界inode[0]的key缓存在node上，用于在parent node
                        // 查找本身时使用
    pgid       pgid
    parent     *node
    children   nodes
    inodes     inodes
}

type inode struct {
    flags uint32   // 当所在node为叶子节点时，记录key的flag
    pgid  pgid     // 当所在node为叶子节点时，不使用，当所在node为分支节点时，记录所指向的page-id
    key   []byte   // 当所在node为叶子节点时，记录的为拥有的key；当所在node为分支节点时，记录的是子
                   // 节点的上key的下边界。例如，当当前node为分支节点，拥有3个分支，[1,2,3][5,6,7][10,11,12]
                   // 这该node上可能有3个inode，记录的key为[1,5,10]。当进行查找时2时，2<5,则去第0个子分支上继
                   // 续搜索，当进行查找4时，1<4<5,则去第1个分支上去继续查找。
    value []byte   // 当所在node为叶子节点时，记录的为拥有的value
}

type bucket struct {
    root     pgid   // page id of the bucket's root-level page
    sequence uint64 // monotonically incrementing, used by NextSequence()
}





boltdb通过B-Tree来构建数据的索引，其B-Tree的根为Bucket，其数上的每个节点为node和inode或 branchPageElements和leafPageElement；B-Tree的上全部的Key-Value数据都记录在叶子节点上。

当Bucket的数据还没有commit写入磁盘时，在内存中以node和inode来记录，当数据被写入磁盘时，以 branchPageElements和leafPageElement来记录。

正式这样的混合组织方式，因此在搜索这个B-Tree的时候，遇见的数据可以是磁盘上的数据也可能是内存中的数据， 因此采用Cursor这样的迭代器。Cursor的stack []elemRef中几率的每次迭代操作是走过的路径。其路径可能是 磁盘中的某个page也可能是还未刷入磁盘的node。elemRef中的index用来记录search时命中的index。

这棵大B-Tree上总共有2中节点，一种是Bucket，一种是node，这两种不同是节点都存储在B-Tree的K-V对上，只是 flag不同。Bucket被当做树或者子树的根节点，node是B-Tree上的普通节点，依负在某一个Bucket上。Bucket 当做一个子树看待，所以不会跟同级的node一同rebalance。Bucket在树上记录的value为bucket，即根节点的 page-id和sequence。

Bucket
因此，很好理解Bucket的嵌套关系。子Bucket就是在父Bucket上创建一个Bucket节点。 关于Bucket的描述可以参考BoltDB之Bucket(一)/BoltDB之Bucket(二) 两篇文章的描述。看这2篇文章时，先忽略掉inline bucket部分的内容，不然不容易理解。inline bucket只不过是 一个对于磁盘空间的优化，通常Bucket的信息在磁盘上的记录很小，如果直接占用一个page有些浪费，则将Bucket 对应page的剩余部分当做Bucket可以使用的一个page，则当数据量较小时，可以节省一个page。

node并不是B-Tree上的一个节点，并不是最总存储数据的K-V对，在node上的inode才是最终存储数据的K-V对。 每个node对应唯一一个page，是page在内存中的缓存对象。在Bucket下会有node的缓存集合。当需要访问 某个page时，会先去缓存中查找其node，只有node不存在时，才去加载page。



## Bucket

类似于传统关系型数据库中的表，用来将数据库划分为多个命名空间，从而提高搜索效率；不同的时，Bucket 允许创建嵌套结构，而表不允许。

每个 Bucket 也是以 KV 结构的方式持久化到数据文件中，为了提高查询效率，对于容量较小的 Bucket 通过 inline 方式存储，也就是将真正的数据同时连续存放。

在初始化 MetaPage 时，会将  RootBucket 指定为 3 。

http://www.d-kai.me/boltdb%E4%B9%8Bbucket%E4%B8%80/

https://www.jianshu.com/p/b86a69892990

## MVCC

其实现主要依赖 COW 和 Meta 副本。

每创建一个事务就会复制一份当前最新的 Meta 数据，


meta。在Tx中的每个操作都会将变化后的B-Tree上的node缓存 Tx的Bucket副本中，这个变化只对该Tx可见。当有删除操作时，Tx会将要释放的page-id暂存在freelist的 pending池中，当Tx调用Commit时，freelist会将pending的page-id真正标记为free状态，如果Tx调用Rollback 则会将pending池中的page-id移除，则使Tx的删除操作回滚。

当Tx调用Commit时，

会触发Bucket的rebalance，rebalance会根据阈值条件，尽量提高每个修改过的node的 使用率(即Bucket缓存的node，只有put/del过的page才会加载成node缓存在Bucket下)，经过剪去空数据分 枝、合并相邻且填充率较低的分支，最后通过数据的搬移，释放更多的page。

然后再触发Bucket的spill，spill会再将rebalance聚拢后的node根据Bucket.FillPercent将每个node所持 有的数据将到pagesize*Bucket.FillPercent之下。然后获取新的page(获取新的page时，会先去freelist查找能复用 page-id，如果找不到就新分配一个，当新分配的page使文件大小超过只读mmap的大小时，会重新mmap一次)，然后将node 写入page。

然后更新freelist的持久化记录。然后将上面操作的page刷新到磁盘上。最后更新meta的在磁盘上的记录、释放Tx 的资源。

因此在没有更新meta之前，写入的数据对于读事务都是不可见的。

文件映射增长策略
当boltdb文件小于1GB时，每次重新映射时，映射大小翻倍，当文件大于1GB时，每次增长1GB，且与pagesize对齐。

newBackend()
 |-bolt.Open() 













Open() db.go如果文件不存在则新建
 |-DB{} 创建对象实例时设置为打开
 |-os.Open() 打开文件
 |-flock() 由于文件中涉及了元数据的操作，为了防止破坏，只能由一个进程访问
 |-file.Stat() 获取文件的状态信息
 |-DB.init() 如果文件大小为0，则初始化
   |-writeAt() 会新建两个MetaPage(冗余) 一个FreeListPage 一个EmptyLeafPage
   |-fdatasync() 将数据刷新到磁盘上
 |-DB.mmap() 将磁盘文件映射到内存中
   |-mmapSize() 获取需要映射文件的大小，小于1G会double下，大于则以1G速度增长
   |-mmap() 调用系统的mmap将文件映射到内存
   |-validate() 会对两个Meta页面进行校验，并选择无异常页面恢复
 |-loadFreelist()



--->

{% highlight text %}
{% endhighlight %}
