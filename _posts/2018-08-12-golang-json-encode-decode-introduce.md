---
title: GoLang JSON 编码解码
layout: post
comments: true
language: chinese
category: [golang,program,misc]
keywords: golang,json
description: 随着 REST API 的兴起，基本上已经前后端分离，更多的返回格式是 json 字符串，这里简单讨论下在 GoLang 中如何编码和解码 JSON 结构。
---

随着 REST API 的兴起，基本上已经前后端分离，更多的返回格式是 json 字符串，这里简单讨论下在 GoLang 中如何编码和解码 JSON 结构。

GoLang 提供了 `encoding/json` 的标准库用于 JSON 的处理，简单记录 GoLang 中使用 JSON 的常用技巧。

<!-- more -->

## 编码

在使用库进行序列化时，只有字段名为大写的才会被编码到 JSON 中，不同结构的数组需要使用 `[]interface{}` 进行转换。

直接列举常用的表示方法。

{% highlight go %}
package main

import (
	"encoding/json"
	"log"
)

type Account struct {
	Email    string
	password string
	Money    float64 `json:"money,omitempty,string"`
	Secret   string  `json:"-"`
}

type User struct {
	Name    string
	Age     int
	Roles   []string
	Skill   map[string]float64
	Account Account
	Extra   []interface{}
	Level   map[string]interface{}
}

func main() {
	skill := make(map[string]float64)
	skill["python"] = 99.5
	skill["ruby"] = 80.0

	extra := []interface{}{123, "hello world"}

	level := make(map[string]interface{})
	level["web"] = "Good"
	level["server"] = 90
	level["tool"] = nil

	user := User{
		Name:  "foobar",
		Age:   27,
		Roles: []string{"Owner", "Master"},
		Skill: skill,
		Account: Account{
			Email:    "foobar@example.com",
			password: "YOUR PASSWORD",
			Money:    11.1,
			Secret:   "some info",
		},
		Extra: extra,
		Level: level,
	}

	rs, err := json.Marshal(user)
	if err != nil {
		log.Fatalln(err)
	}
	log.Println(string(rs))
}
{% endhighlight %}

输出的内容为。

{% highlight text %}
{
	"Name":"foobar",
	"Age":27,
	"Roles":[
		"Owner",
		"Master"
	],
	"Skill":{
		"python":99.5,
		"ruby":80
	},
	"Account":{
		"Email":"foobar@example.com",
		"money":"11.1"
	},
	"Extra":[
		123,
		"hello world"
	],
	"Level":{
		"server":90,
		"tool":null,
		"web":"Good"
	}
}
{% endhighlight %}

### 常用示例

1. 忽略空白，添加 `omitempty` 注释。
2. 将数值设置为字符串，添加 `string` 注释。
3. 忽略部分字段，将字段名称设置为 `-`。

## 解码

解码就是将 JSON 字符串反序列化为 GoLang 对象，在匹配字段时 **大小写不敏感的**，而且不会设置私有字段，如果有不匹配的字段则直接忽略。

{% highlight go %}
package main

import (
	"encoding/json"
	"log"
)

type Account struct {
	Email    string  `json:"email"`
	Money    float64 `json:"money"`
	PassWord string  `json:"password"`
	Level    int     `json:"level,string"`
	Secret   string  `json:"-"`
	//password string  `json:"password"` // NOT Work
}

var jsonString string = `{
	"email": "foobar@example.com",
	"password" : "YOUR PASSWORD",
	"money" : 100.5,
	"level" : "10",
	"Unexists" : "Some mess fields"
}`

func main() {
	account := Account{}

	err := json.Unmarshal([]byte(jsonString), &account)
	if err != nil {
		log.Fatal(err)
	}

	log.Printf("%#v\n", account)
}
{% endhighlight %}

### 常见示例

1. 如果要求将字符串转换为数值，可以增加 `string` 标签，此时 JSON 中必须使用 `""` 否则报错。
2. 忽略字段同样使用 `-` ，不过此时会设置默认的初始值，`int` 为 `0`，`string` 为 `""`。

### 动态解析

上述可以通常根据 JSON 的格式预先定义 GoLang 的结构进行解析是最理想的情况，不过实际开发中，经常出现 JSON 非但格式不确定，有的还可能是动态数据类型。

例如，登陆系统时可以使用邮箱或者手机号。

首先，调用 json 的 `NewDecoder()` 构造一个 `Decode` 对象，然后使用这个对象的 `Decode()` 方法赋值给定义好的结构对象。

{% highlight go %}
package main

import (
	"encoding/json"
	"log"
	"strings"
)

type User struct {
	UserName string `json:"username"`
	Password string `json:"password"`
}

var jsonString string = `{
	"username": "foobar@example.com",
	"password": "YOUR PASSWORD"
}`

func main() {
	str := strings.NewReader(jsonString) // io.Reader
	usr := User{}

	err := json.NewDecoder(str).Decode(&usr)
	if err != nil {
		log.Fatalf("Decode failed, %#v\n", err)
	}

	log.Printf("User %#v\n", usr)
}
{% endhighlight %}

这里使用 `strings.NewReader()` 让字符串变成一个 `Stream` 对象。

### 空接口

如果上述使用 `"username": 12345678901,` ，在解析时就会报错，此时需要使用空接口，不过此时返回的是 `1.2345678901e+10` ，为此可以使用类型判断。

{% highlight go %}
package main

import (
	"encoding/json"
	"log"
	"strings"
)

type User struct {
	UserName interface{} `json:"username"`
	Password string      `json:"password"`
}

var jsonString string = `{
	"username": 12345678901,
	"password": "YOUR PASSWORD"
}`

func main() {
	str := strings.NewReader(jsonString) // io.Reader
	usr := User{}

	err := json.NewDecoder(str).Decode(&usr)
	if err != nil {
		log.Fatalf("Decode failed, %#v\n", err)
	}

	switch t := usr.UserName.(type) {
	case string:
		usr.UserName = t
	case float64:
		usr.UserName = int64(t)
	}

	log.Printf("User %#v\n", usr)
}
{% endhighlight %}

对于上述的 `UserName` 字段，仍然是空接口，为了方便可以定义相关的字段，例如。

{% highlight text %}
type User struct {
	UserName interface{} `json:"username"`
	Password string `json:"password"`

	Email string
	Phone int64
}
{% endhighlight %}

### 延迟解析

对于 `UserName` 字段只有在使用时，才会用到他的具体类型，因此可以延迟解析。使用 `json.RawMessage` 方式，将 json 的字串继续以 `byte` 数组方式存在。

{% highlight go %}
package main

import (
	"encoding/json"
	"log"
	"strings"
)

type User struct {
	UserName json.RawMessage `json:"username"`
	Password string          `json:"password"`

	Email string
	Phone int64
}

var jsonString string = `{
	"username": 12345678901,
	"password": "YOUR PASSWORD"
}`

func main() {
	str := strings.NewReader(jsonString) // io.Reader
	usr := User{}

	err := json.NewDecoder(str).Decode(&usr)
	if err != nil {
		log.Fatalf("Decode failed, %#v\n", err)
	}

	var email string
	if err = json.Unmarshal(usr.UserName, &email); err == nil {
		usr.Email = email
	}

	var phone int64
	if err = json.Unmarshal(usr.UserName, &phone); err == nil {
		usr.Phone = phone
	}

	log.Printf("User %#v\n", usr)
}
{% endhighlight %}

### 不定字段解析

对于未知 json 结构的解析，不同的数据类型可以映射到接口或者使用延迟解析。

#### 空接口

{% highlight go %}
package main

import (
	"encoding/json"
	"fmt"
	"log"
)

type Person struct {
	Name string `json:"name"`
	Age  int    `json:"age"`
}

type Place struct {
	City    string `json:"city"`
	Country string `json:"country"`
}

var jsonString string = `{
    "things": [
        {
            "name": "Alice",
            "age": 37
        },
        {
            "city": "Ipoh",
            "country": "Malaysia"
        },
        {
            "name": "Bob",
            "age": 36
        },
        {
            "city": "Northampton",
            "country": "England"
        }
    ]
}`

func main() {
	var data map[string][]map[string]interface{}
	var persons []Person
	var places []Place

	err := json.Unmarshal([]byte(jsonString), &data)
	if err != nil {
		log.Fatal(err)
	}

	for _, v := range data["things"] {
		if v["name"] != nil {
			p := Person{
				Name: v["name"].(string),
				Age:  int(v["age"].(float64)),
			}
			persons = append(persons, p)
		} else {
			p := Place{
				City:    v["city"].(string),
				Country: v["country"].(string),
			}
			places = append(places, p)
		}
	}

	log.Printf("Person %#v\n", persons)
	log.Printf("Place %#v\n", places)
}
{% endhighlight %}

如下是解释更复杂的示例。

{% highlight go %}
package main

import (
	"encoding/json"
	"log"
)

var jsonString string = `{
	"things": [{
		"name": "Alice",
		"age": 37,
		"info": {
			"phone": "123"
		}
	}]
}`

func main() {
	var data map[string][]map[string]interface{}

	err := json.Unmarshal([]byte(jsonString), &data)
	if err != nil {
		log.Fatal(err)
	}

	for _, val := range data["things"] {
		log.Printf("Info Phone %v\n", val["info"].(map[string]interface{})["phone"])
	}
}
{% endhighlight %}

#### 混合结构

与前面解析 `username` 为 `email` 和 `phone` 两种情况，就在结构中定义好这两种结构即可。

{% highlight go %}
type Mixed struct {
	Name    string `json:"name"`
	Age     int `json:"age"`
	City    string `json:"city"`
	Country string  `json:"country"`
}
{% endhighlight %}

简单来说，就是借助 GoLang 会初始化没有匹配的 JSON 和抛弃没有匹配的 JSON，给特定的字段赋值。

{% highlight go %}
package main

import (
	"encoding/json"
	"fmt"
	"log"
)

type Person struct {
	Name string `json:"name"`
	Age  int    `json:"age"`
}

type Place struct {
	City    string `json:"city"`
	Country string `json:"country"`
}

type Mixed struct {
	Name    string `json:"name"`
	Age     int    `json:"age"`
	City    string `json:"city"`
	Country string `json:"country"`
}

var jsonString string = `{
    "things": [
        {
            "name": "Alice",
            "age": 37
        },
        {
            "city": "Ipoh",
            "country": "Malaysia"
        },
        {
            "name": "Bob",
            "age": 36
        },
        {
            "city": "Northampton",
            "country": "England"
        }
    ]
}`

func main() {
	var data map[string][]Mixed
	var persons []Person
	var places []Place

	err := json.Unmarshal([]byte(jsonString), &data)
	if err != nil {
		fmt.Println(err)
		return
	}

	for _, v := range data["things"] {
		if v.Name != "" {
			p := Person{Name: v.Name, Age: v.Age}
			persons = append(persons, p)
		} else {
			p := Place{City: v.City, Country: v.Country}
			places = append(places, p)
		}
	}

	log.Printf("Person %#v\n", persons)
	log.Printf("Place %#v\n", places)
}
{% endhighlight %}

<!--
#### RawMessage

如上述使用的 `json.RawMessage` 。


type Person struct {
    Name string `json:"name"`
    Age  int    `json:"age"`
}

type Place struct {
    City    string `json:"city"`
    Country string `json:"country"`
}

func addPerson(item json.RawMessage, persons []Person) ([]Person) {
    person := Person{}
    if err := json.Unmarshal(item, &person); err != nil {
        fmt.Println(err)
    } else {
        if person != *new(Person) {
            persons = append(persons, person)
        }
    }

    return persons
}

func addPlace(item json.RawMessage, places []Place) ([]Place) {
    place := Place{}
    if err := json.Unmarshal(item, &place); err != nil {
        fmt.Println(err)
    } else {
        if place != *new(Place) {
            places = append(places, place)
        }
    }

    return places
}

func decode(jsonStr []byte) (persons []Person, places []Place) {
    var data map[string][]json.RawMessage

    err := json.Unmarshal(jsonStr, &data)
    if err != nil {
        fmt.Println(err)
        return
    }

    for _, item := range data["things"] {
        persons = addPerson(item, persons)
        places = addPlace(item, places)
    }

    return
}

var jsonString string = `{
    "things": [
        {
            "name": "Alice",
            "age": 37
        },
        {
            "city": "Ipoh",
            "country": "Malaysia"
        },
        {
            "name": "Bob",
            "age": 36
        },
        {
            "city": "Northampton",
            "country": "England"
        }
    ]
}`

func main() {
    personA, placeA := decode([]byte(jsonString))

    fmt.Printf("%+v\n", personA)
    fmt.Printf("%+v\n", placeA)
}

把 things 的 item 数组解析成一个 json.RawMessage，然后再定义其他结构逐步解析。上述这些例子其实在真实的开发环境下，应该尽量避免。像 person 或是 place 这样的数据，可以定义两个数组分别存储他们，这样就方便很多。不管怎么样，通过这个略傻的例子，我们也知道了如何解析 json 数据。


使用 JSON 的一些小技巧
https://blog.csdn.net/impressionw/article/details/74731888
-->



{% highlight text %}
{% endhighlight %}
