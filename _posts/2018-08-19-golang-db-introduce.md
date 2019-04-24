---
title: GoLang DB 操作简介
layout: post
comments: true
language: chinese
category: [program,golang,linux]
keywords: mysql,sql,golang
description:
---

GoLang 提供了标准包用于对 SQL 数据库进行访问，作为操作数据库的入口对象 sql.DB, 主要为提供了两个重要的功能：A) 提供管理底层数据库连接的打开和关闭操作；B) 管理数据库连接池。

需要注意的是，sql.DB 表示操作数据库的抽象访问接口，而非一个数据库连接对象，会根据实际的驱动打开关闭数据库连接，管理连接池。

这里简单介绍 MySQL 的使用方式。

<!-- more -->

<!-- ![grpc introduce]({{ site.url }}/images/go/grpc-introduce.png "grpc introduce"){: .pull-center width="70%" } -->

## 基本操作

如下的示例中都是使用 `test.users` 表。

{% highlight sql %}
CREATE DATABASE IF NOT EXISTS `test`;
USE `test`;

DROP TABLE IF EXISTS `users`;
CREATE TABLE IF NOT EXISTS `users` (
	`id` INT NOT NULL AUTO_INCREMENT PRIMARY KEY,
	`name` CHAR(64) NOT NULL COMMENT "用户名",
	`age` INT NOT NULL COMMENT "用户的年龄",
	`gender` ENUM('no', 'male', 'female') DEFAULT 'no' COMMENT "性别",
	`gmt_modify` TIMESTAMP NOT NULL ON UPDATE CURRENT_TIMESTAMP,
	`gmt_create` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
	UNIQUE KEY `uk_name` (`name`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT "用户列表";

INSERT INTO users(name, age, gender) VALUES("Atelier", 29, "male");
INSERT INTO users(name, age, gender) VALUES("Kingsley", 39, "male");
INSERT INTO users(name, age, gender) VALUES("Gwyneth", 19, "female");
{% endhighlight %}

### 安装驱动

也就是安装 MySQL 的驱动。

{% highlight text %}
$ go get github.com/go-sql-driver/mysql
{% endhighlight %}

### 建立连接

在访问数据库前，需要先建立链接，也就是用到 `database/sql` 中的 `Open()` 函数，示例如下。

{% highlight go %}
db, err := sql.Open("mysql", "root:yourpassword@tcp(127.0.0.1:3306)/yourdatabase")
{% endhighlight %}

上述的第二个参数表示连接 DB 的方式，也就是使用 `root` 用户，密码是 `yourpassword`，使用 TCP 协议，数据库 IP 地址为 `127.0.0.1:3306`，当前使用的数据库是 `yourdatabase` 。

MySQL 的连接方式有很多种，除了上述方式外，也可以参考如下。

{% highlight text %}
user@unix(/path/to/socket)/dbname?charset=utf8
user:password@tcp(localhost:5555)/dbname?charset=utf8
user:password@/dbname
user:password@tcp([de:ad:be:ef::ca:fe]:80)/dbname
{% endhighlight %}

### 查询

当建立了数据库的连接之后，就可以执行 SQL 查询语句了。

{% highlight go %}
rows, err := db.Query("SELECT * FROM users")
{% endhighlight %}

然后用 `for` 循环遍历返回的结果，如果已知类型，那么可以直接转换，也可以使用通用的。

### 修改

可以使用 `Prepare()` 语句，然后在执行时添加参数，如果未使用占位符，在执行 `Exec()` 时参数可以为空。

{% highlight go %}
stmt, err := db.Prepare("INSERT INTO users(name, age, gender) VALUES(?, ?, ?);")
res, err := stmt.Exec("Andy", 14, "male")
{% endhighlight %}


### 完整示例

{% highlight go %}
package main

import (
	"fmt"
	"log"

	"database/sql"
	_ "github.com/go-sql-driver/mysql"
)

func DoQuery(db *sql.DB) {
	rows, err := db.Query("SELECT name, age, gender FROM users;")
	if err != nil {
		log.Fatal(err)
	}
	defer rows.Close()

	cloumns, err := rows.Columns() // get columns' name
	if err != nil {
		log.Fatal(err)
	}
	fmt.Println(cloumns)
	fmt.Println("------------------")

	for rows.Next() {
		var name, gender string
		var age int

		err := rows.Scan(&name, &age, &gender)
		if err != nil {
			log.Fatal(err)
		}
		fmt.Println(name, age, gender)
	}

	/*
	values := make([]sql.RawBytes, len(cloumns))
	scanArgs := make([]interface{}, len(values))
	for i := range values {
		scanArgs[i] = &values[i]
	}

	for rows.Next() {
		err = rows.Scan(scanArgs...)
		if err != nil {
			log.Fatal(err)
		}

		var value string
		for i, col := range values {
			if col == nil {
				value = "NULL"
			} else {
				value = string(col)
			}
			fmt.Println(cloumns[i], ": ", value)
		}
		fmt.Println("------------------")
	}
	*/

	if err = rows.Err(); err != nil {
		log.Fatal(err)
	}
}

func DoInsert(db *sql.DB) {
	stmt, err := db.Prepare("INSERT INTO users(name, age, gender) VALUES(?, ?, ?);")
	if err != nil {
		log.Fatal(err)
	}

	res, err := stmt.Exec("Andy", 14, "male")
	if err != nil {
		log.Fatal(err)
	}
	lastId, err := res.LastInsertId()
	if err != nil {
		log.Fatal(err)
	}
	rowCnt, err := res.RowsAffected()
	if err != nil {
		log.Fatal(err)
	}
	fmt.Printf("ID=%d, affected=%d\n", lastId, rowCnt)
}

func main() {
	db, err := sql.Open("mysql", "root:@tcp(localhost:5506)/test")
	if err != nil {
		log.Fatal(err)
	}
	defer db.Close()

	DoQuery(db)
	DoInsert(db)
}
{% endhighlight %}

## 连接池

`sql.Open()` 实际上是返回一个连接池对象，而不是单个连接，在打开时并没有去连接数据库，只有在执行 `Query()`、`Exce()` 时才会去实际连接数据库。

这也就意味着在一个应用中，同样的库连接只需要保存一个 `sql.Open()` 返回的 DB 对象即可，而不需要多次 `Open()` 。

{% highlight go %}
var db *sql.DB
func init() {
	db, _ = sql.Open("mysql", "root:@tcp(127.0.0.1:3306)/test?charset=utf8")
	db.SetMaxOpenConns(2000)
	db.SetMaxIdleConns(1000)
	db.Ping()
}
{% endhighlight %}

连接池的实现关键在于 `SetMaxOpenConns()` 和 `SetMaxIdleConns()` ，其中，前者用于设置最大打开的连接数，默认值为 0 表示不限制；后者用于设置闲置的连接数。

<!--
golang sql连接池的实现解析
https://blog.csdn.net/pangudashu/article/details/54291558
-->

## gorm

Object Relation Mapping, ORM 实际上就是对数据库的操作进行封装，屏蔽数据库操作细节，从而简化开发，提高效率，GoLang 的 ORM 可以参考 [gorm.io](http://gorm.io/) ，其使用方法简单介绍如下。

{% highlight text %}
$ go get -u github.com/jinzhu/gorm
{% endhighlight %}

`github.com/jinzhu/gorm/dialects/mysql` 是 MySQL 驱动，实际上就是 `github.com/go-sql-driver/mysql` 只是进行了重新命名。

gorm 用 tag 来标识 MySQL 里面的约束，创建索引只需要直接指定列即可，如果需要多列组合索引，直接让索引的名字相同即可。

### 创建表

可以通过 `db.HasTable()` 来判断表是否存在，其入参可以使用两种形式：A) 字符串；B) 模型的地址类型。其判断方式是直接查询 `INFORMATION_SCHEMA.TABLES` 表中的数据。

{% highlight go %}
if ok := DB.HasTable("foos"); ok {
	t.Errorf("Table should not exist, but does")
}
if ok := DB.HasTable(&Foo{}); ok {
	t.Errorf("Table should not exist, but does")
}
{% endhighlight %}

定义模型时，必须指定字段的首字母为大写，否则无法创建字段，同时可以使用 `gorm tag` 进行制定，可以参考 [Declaring Models](http://gorm.io/docs/models.html)，不过有些调试起来比较复杂，还是直接创建比较好。

{% highlight text %}
db.CreateTable(&User{})

r1 := db.DropTable("Users")
r2 := db.DropTable(&User{})
{% endhighlight %}

默认创建的表名为复数形式，例如 `User` 创建后的表名为 `users` ，如果不想创建复数形式的表名，可以通过如下的语句设置。

{% highlight go %}
db.SingularTable(true)
{% endhighlight %}

如果要自己定义，可以通过如下方式修改。

{% highlight go %}
type UserInfo struct {} // 默认表名是user_infos

// 设置UserInfo的表名为users
func (UserInfo) TableName() string {
	return "users"
}

func (u UserInfo) TableName() string {
	if u.Role == "admin" {
		return "admin_users"
	} else {
		return "users"
	}
}
{% endhighlight %}

### 更新时间

gorm 提供了三个与时间相关的字段，会在操作时自动更新，包括了 `CreatedAt` `UpdatedAt` `DeletedAt` 。

{% highlight text %}
----- 字段CreatedAt用于存储记录的创建时间
db.Create(&user) // 将会设置CreatedAt为当前时间
----- 要更改它的值, 需要使用Update
db.Model(&user).Update("CreatedAt", time.Now())

----- 字段UpdatedAt用于存储记录的修改时间
db.Save(&user)                           // 将会设置UpdatedAt为当前时间
db.Model(&user).Update("name", "jinzhu") // 将会设置UpdatedAt为当前时间
{% endhighlight %}

<!--
1.2.8. 字段DeletedAt用于存储记录的删除时间，如果字段存在

删除具有DeletedAt字段的记录，它不会冲数据库中删除，但只将字段DeletedAt设置为当前时间，并在查询时无法找到记录，请参阅软删除
-->


### 其它

#### 日志

当执行 SQL 时，可以通过 `db.LogMode(true)` 打开日志，也可以通过如下方式调试单个操作日志。

{% highlight text %}
db.LogMode(true)
db.SetLogger(gorm.Logger{revel.TRACE})
db.SetLogger(log.New(os.Stdout, "\r\n", 0))

db.Debug().Where("name = ?", "jinzhu").First(&User{})
{% endhighlight %}

这样就可以将实际执行的 SQL 打印出来。

#### 默认值

如果不通过 golang 的 `tag` 定义默认值，gorm 会自动将对应的字段填充为默认值。

### 示例

如下是一个深度定制之后的代码，其中日志是从原库中移植过来的。

{% highlight go %}
package main

import (
	"database/sql/driver"
	"fmt"
	"log"
	"reflect"
	"regexp"
	"strconv"
	"time"
	"unicode"

	"github.com/jinzhu/gorm"
	_ "github.com/jinzhu/gorm/dialects/mysql"
)

type UserInfo struct {
	ID        int       `gorm:"AUTO_INCREMENT;primary_key"`
	Name      string    `gorm:"column:name;type:varchar(64);not null;unique_index:uk_name"`
	Age       int       `gorm:"column:age;type:int;not null"`
	Gender    string    `gorm:"column:gender;type:enum('no','male','female');default:no"`
	CreatedAt time.Time `gorm:"column:gmt_create"`
	UpdatedAt time.Time `gorm:"column:gmt_modify"`
}

func (UserInfo) TableName() string {
	return "users"
}

var (
	sqlRegexp                = regexp.MustCompile(`\?`)
	numericPlaceHolderRegexp = regexp.MustCompile(`\$\d+`)
)

type Logger struct{}

func isPrintable(s string) bool {
	for _, r := range s {
		if !unicode.IsPrint(r) {
			return false
		}
	}
	return true
}

func (logger Logger) Print(values ...interface{}) {
	if len(values) <= 1 {
		return
	}

	var (
		sql             string
		formattedValues []string
	)

	// time.Now().Format("2006-01-02 15:04:05")
	messages := []interface{}{values[1]}
	if values[0] == "sql" {
		messages = append(messages, fmt.Sprintf("[%.2fms %v]",
			float64(values[2].(time.Duration).Nanoseconds()/1e4)/100.0,
			strconv.FormatInt(values[5].(int64), 10)))
		for _, value := range values[4].([]interface{}) {
			indirectValue := reflect.Indirect(reflect.ValueOf(value))
			if indirectValue.IsValid() {
				value = indirectValue.Interface()
				if t, ok := value.(time.Time); ok {
					formattedValues = append(formattedValues, fmt.Sprintf("'%v'",
						t.Format("2006-01-02 15:04:05")))
				} else if b, ok := value.([]byte); ok {
					if str := string(b); isPrintable(str) {
						formattedValues = append(formattedValues,
							fmt.Sprintf("'%v'", str))
					} else {
						formattedValues = append(formattedValues,
							"'<binary>'")
					}
				} else if r, ok := value.(driver.Valuer); ok {
					if value, err := r.Value(); err == nil && value != nil {
						formattedValues = append(formattedValues,
							fmt.Sprintf("'%v'", value))
					} else {
						formattedValues = append(formattedValues, "NULL")
					}
				} else {
					formattedValues = append(formattedValues, fmt.Sprintf("'%v'", value))
				}
			} else {
				formattedValues = append(formattedValues, "NULL")
			}
		}

		// differentiate between $n placeholders or else treat like ?
		if numericPlaceHolderRegexp.MatchString(values[3].(string)) {
			sql = values[3].(string)
			for index, value := range formattedValues {
				placeholder := fmt.Sprintf(`\$%d([^\d]|$)`, index+1)
				sql = regexp.MustCompile(placeholder).ReplaceAllString(sql, value+"$1")
			}
		} else {
			formattedValuesLength := len(formattedValues)
			for index, value := range sqlRegexp.Split(values[3].(string), -1) {
				sql += value
				if index < formattedValuesLength {
					sql += formattedValues[index]
				}
			}
		}
		messages = append(messages, sql)
	} else {
		messages = append(messages, values[2:]...)
	}
	log.Println(messages...)
}

var db *gorm.DB

func main() {
	var err error

	db, err = gorm.Open("mysql", "root:@tcp(localhost:5506)/test")
	if err != nil {
		log.Fatal(err)
	}
	defer db.Close()

	db.DB().SetMaxIdleConns(10)
	db.DB().SetMaxOpenConns(100)
	db.SetLogger(Logger{})
	db.LogMode(true)

	if ok := db.HasTable("users"); ok {
		log.Println("Table should not exist, but does")
	}
	if ok := db.HasTable(&UserInfo{}); !ok {
		log.Println("Table should not exist, but does")
		if err := db.Set("gorm:table_options",
			"ENGINE=InnoDB DEFAULT CHARSET=utf8").CreateTable(&UserInfo{}).Error; err != nil {
			panic(err)
		}
	}

	user := UserInfo{Name: "Jinzhu"}
	db.Create(&user)

	db.Where("name = ?", "Jinzhu").First(&user)

	db.Delete(&user)
}
{% endhighlight %}

## 参考

[GORM 中文文档](http://gorm.book.jasperxu.com/) 或者 [GitHub](https://github.com/jasperxu/gorm-cn-doc) 很详细的介绍，包括了常见的一些高级用法。

{% highlight text %}
{% endhighlight %}
