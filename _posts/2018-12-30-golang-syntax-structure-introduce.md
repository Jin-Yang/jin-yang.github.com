---
title: Golang 语法之结构体
layout: post
comments: true
language: chinese
category: [program,golang]
keywords: golang,structure
description: 类似于 C 中的结构体，也就是用户自定义的类型，它代表若干字段的集合。这里简单介绍其使用方法。
---

类似于 C 中的结构体，也就是用户自定义的类型，它代表若干字段的集合。

这里简单介绍其使用方法。

<!-- more -->

<!--
什么是结构体

比如将一个员工的 firstName, lastName 和 age 三个属性打包在一起成为一个 employee 结构就是很有意义的。
结构体的声明

type Employee struct {  
    firstName string
    lastName  string
    age       int
}

    1
    2
    3
    4
    5

上面的代码片段声明了一个名为 Employee 的结构体类型，它拥有 firstName, lastName 和 age 三个字段。同一类型的多个字段可以合并到一行（用逗号分隔），并将类型放在后面。上面的结构体中 firstName 与 lastName 都是 string 类型，因此可以将它们写在一起。

type Employee struct {  
    firstName, lastName string
    age, salary         int
}

    1
    2
    3
    4

上面的结构体 Employee 是一个具名结构体（named structure），因为它创建了一个具有名字的结构体类型： Employee 。我们可以定义具名结构体类型的变量。

我们也可以定义一个没有类型名称的结构体，这种结构体叫做匿名结构体（anonymous structures）。

var employee struct {  
        firstName, lastName string
        age int
}

    1
    2
    3
    4

上面的代码片段声明了一个匿名结构体变量 employee。
定义具名结构体变量

下面的程序说明了如何定义一个具名结构体 Employee 的变量。

package main

import (  
    "fmt"
)

type Employee struct {  
    firstName, lastName string
    age, salary         int
}

func main() {

    //creating structure using field names
    emp1 := Employee{
        firstName: "Sam",
        age:       25,
        salary:    500,
        lastName:  "Anderson",
    }

    //creating structure without using field names
    emp2 := Employee{"Thomas", "Paul", 29, 800}

    fmt.Println("Employee 1", emp1)
    fmt.Println("Employee 2", emp2)
}

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

在上面的程序中，我们定义了一个名为 Employee 的结构体类型。我们可以通过指定字段的名称和对应的值来创建一个结构体变量，比如在第15行，我们就是通过这种方式定义了 Employee 类型的一个结构体变量 empl。这里字段名称的顺序没必要和声明结构体类型时的一致。例如这里我们将 lastName 放在了最后，程序同样正确运行。

在定义结构体变量时也可以忽略字段名称，例如在第 23 行，我们定义 emp2 时没有指定字段名称。但是通过这种方式定义的结构体变量时，字段值的顺序必须与声明结构体类型时字段的顺序保持一致。

上面的程序输出如下：

Employee 1 {Sam Anderson 25 500}  
Employee 2 {Thomas Paul 29 800} 

    1
    2

定义匿名结构体变量

package main

import (  
    "fmt"
)

func main() {  
    emp3 := struct {
        firstName, lastName string
        age, salary         int
    }{
        firstName: "Andreah",
        lastName:  "Nikola",
        age:       31,
        salary:    5000,
    }

    fmt.Println("Employee 3", emp3)
}

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

在上面的程序中，第3行定义了一个 匿名结构体变量 emp3。正如我们提到的一样，这种结构体成为匿名结构体，因为它只创建了一个新的结构体变量 emp3，而没有定义新的结构体类型。

程序的输出为：

Employee 3 {Andreah Nikola 31 5000}  

    1

结构体变量的 0 值

当定义一个结构体变量，但是没有给它提供初始值，则对应的字段被赋予它们各自类型的0值。

package main

import (  
    "fmt"
)

type Employee struct {  
    firstName, lastName string
    age, salary         int
}

func main() {  
    var emp4 Employee //zero valued structure
    fmt.Println("Employee 4", emp4)
}

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

上面的程序定义了 emp4 但是没有赋予任何初值。因此 firstName 和 lastName 被赋值为 string 类型的0值，也就是空字符串。age 和 salary 被赋值为 int 类型的0值，也就是 0 。程序的输出为：

Employee 4 {  0 0}  

    1

可以指定一些字段而忽略一些字段。在这种情况下，被忽略的字段被赋予相应类型的 0 值。

package main

import (  
    "fmt"
)

type Employee struct {  
    firstName, lastName string
    age, salary         int
}

func main() {  
    emp5 := Employee{
        firstName: "John",
        lastName:  "Paul",
    }
    fmt.Println("Employee 5", emp5)
}

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

在上面的程序中，第14和15行，firstName 和 lastName 被提供了初始值，而 age 和 salary 没有。因此 age 和 salary 被指定为0值。程序的输出为：

Employee 5 {John Paul 0 0} 

    1

访问结构体中的字段

使用点 . 操作符来访问结构体中的字段。

package main

import (  
    "fmt"
)

type Employee struct {  
    firstName, lastName string
    age, salary         int
}

func main() {  
    emp6 := Employee{"Sam", "Anderson", 55, 6000}
    fmt.Println("First Name:", emp6.firstName)
    fmt.Println("Last Name:", emp6.lastName)
    fmt.Println("Age:", emp6.age)
    fmt.Printf("Salary: $%d", emp6.salary)
}

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

在上面的程序中，通过 emp6.firstName 访问 emp6 中的字段 firstName。程序的输出为：

First Name: Sam  
Last Name: Anderson  
Age: 55  
Salary: $6000 

    1
    2
    3
    4

也可以创建一个 0 值结构体变量，稍后给它的字段一一赋值。

package main

import (  
    "fmt"
)

type Employee struct {  
    firstName, lastName string
    age, salary         int
}

func main() {  
    var emp7 Employee
    emp7.firstName = "Jack"
    emp7.lastName = "Adams"
    fmt.Println("Employee 7:", emp7)
}

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

上面的程序 emp7 被定义然后给 firstName 和 lastName 赋值。程序的输出为：

Employee 7: {Jack Adams 0 0}  

    1

结构体指针

可以定义指向结构体的指针。

package main

import (  
    "fmt"
)

type Employee struct {  
    firstName, lastName string
    age, salary         int
}

func main() {  
    emp8 := &Employee{"Sam", "Anderson", 55, 6000}
    fmt.Println("First Name:", (*emp8).firstName)
    fmt.Println("Age:", (*emp8).age)
}

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

在上面的程序中 emp8 是一个指向结构体 Employee 的指针。(*emp8).firstName 是访问 emp8 中 firstName 字段的语法。程序的输出为：

First Name: Sam  
Age: 55  

    1
    2

在 Go 中我们可以使用 emp8.firstName 替代显示解引用 (*emp8).firstName 来访问 firstName 字段。

package main

import (  
    "fmt"
)

type Employee struct {  
    firstName, lastName string
    age, salary         int
}

func main() {  
    emp8 := &Employee{"Sam", "Anderson", 55, 6000}
    fmt.Println("First Name:", emp8.firstName)
    fmt.Println("Age:", emp8.age)
}

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

在上面的程序中，我们使用了 emp8.firstName 访问 firstName，程序的输出如下：

First Name: Sam  
Age: 55  

    1
    2
-->

## 匿名字段

定义结构体类型时可以仅指定字段类型而不指定字段名字，那么，此时的这种字段叫做匿名字段 (Anonymous Field)。

{% highlight go %}
package main

import (
        "fmt"
)

type Person struct {
        string
        int
}

func main() {
        /* p := Person{string: "Jack", int: 30} */
        p := Person{"Jack", 30}
        fmt.Printf("Current Person %#v\n", p)
        fmt.Printf("And Person Name %#v\n", p.string)
}
{% endhighlight %}

虽然匿名字段没有名字，但是默认名字为类型名，那么上述的 `Person` 可以通过 `Person.string` 引用。当然，这只适用于不同类型的字段，否则将会报错。


## 嵌套 VS. 字段提阶

结构体的字段也可以是一个结构体，这种结构体称为嵌套结构体。

如果结构体中的匿名字段是结构体，这个匿名结构体中字段成为提阶字段 (Promoted Fields)，可以从外部结构体直接访问匿名结构体类型中的字段，就像这些字段原本属于外部结构体一样。

{% highlight go %}
package main

import (
        "fmt"
)

type Address struct {
        City, State string
}
type Person struct {
        Name string
        Age  int
        // address Address // 嵌套
        Address // 字段提阶
}

func main() {
        p := Person{
                Name: "Jack",
                Age:  30,
                Address: Address{
                        City:  "HangZhou",
                        State: "ZheJiang",
                },
        }
        fmt.Printf("Got Person %#v\n", p)

        /* Replace */
        p.Address = Address{
                City:  "NanJing",
                State: "JiangSu",
        }
        fmt.Printf("Got New Person %#v\n", p)

        fmt.Printf("And it's City %s\n", p.City)
}
{% endhighlight %}

上面的程序中，`Person` 结构体有一个匿名字段 `Address`，这个匿名字段也是一个结构体。此时 `Address` 中的字段 `City` 和 `State` 被称为提阶字段，因为它们就好像被直接声明在 `Person` 里一样。

在使用时可以直接通过 `p.City` 引用。

## 导出结构体和字段

如果一个结构体类型的名称以大写字母开头，则该结构体被导出，其它包可以访问它；同样，如果结构体中的字段名以大写字母开头，则这些字段也可以被其他包访问。



<!--
package computer

type Spec struct { //exported struct  
    Maker string //exported field
    model string //unexported field
    Price int //exported field
}


上面的程序创建了一个包 computer，该包中导出了一个结构体类型 Spec，以及它的两个字段 Maker 和 Price，它还有一个未导出字段 model。让我们从另外的一个包导入这个包并使用 Spec 结构体。

package main

import "structs/computer"  
import "fmt"

func main() {  
    var spec computer.Spec
    spec.Maker = "apple"
    spec.Price = 50000
    fmt.Println("Spec:", spec)
}


在上面的程序中，第 8 行和第 9 行，我们访问了 Spec 结构体的两个导出的字段 Makder 和 Price。程序的输出为：Spec: {apple 50000}。

如果我们试图访问未导出的字段 model，程序将会报错。

package main

import "structs/computer"  
import "fmt"

func main() {  
    var spec computer.Spec
    spec.Maker = "apple"
    spec.Price = 50000
    spec.model = "Mac Mini"
    fmt.Println("Spec:", spec)
}

上面的程序第10行，我们试图访问未导出的字段 model。运行这个程序将会报错：structsamples.go:10: spec.model undefined (cannot refer to unexported field or method model)。
比较结构体

结构体是值类型，如果其字段是可比较的，则该结构体就是可以比较的。如果两个结构体变量的所有非空字段都相等，则认为这两个结构体变量相等。

package main

import (  
    "fmt"
)

type name struct {  
    firstName string
    lastName string
}


func main() {  
    name1 := name{"Steve", "Jobs"}
    name2 := name{"Steve", "Jobs"}
    if name1 == name2 {
        fmt.Println("name1 and name2 are equal")
    } else {
        fmt.Println("name1 and name2 are not equal")
    }

    name3 := name{firstName:"Steve", lastName:"Jobs"}
    name4 := name{}
    name4.firstName = "Steve"
    if name3 == name4 {
        fmt.Println("name3 and name4 are equal")
    } else {
        fmt.Println("name3 and name4 are not equal")
    }
}

在上面的程序中，name 结构体类型包含两个 string 字段。因为 string 是可比较的，因此两个 name 类型的变量也是可以比较的。

在上面的程序中，name1 和 name2 是相等的，而 name3 和 name4 是不相等的。程序的输出如下：

name1 and name2 are equal  
name3 and name4 are not equal 

    1
    2

如果结构体包含不可比较的类型的字段，那么这两个结构体是不可比较的。

package main

import (  
    "fmt"
)

type image struct {  
    data map[int]int
}

func main() {  
    image1 := image{data: map[int]int{
        0: 155,
    }}
    image2 := image{data: map[int]int{
        0: 155,
    }}
    if image1 == image2 {
        fmt.Println("image1 and image2 are equal")
    }
}

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

在上面的程序中，image 结构体类型包含了一个字段 data，该字段是 map 类型。map 是不可比较的类型，因此 image1 和 image2 是不可比较的。如果你运行这个程序，将报错：main.go:18: invalid operation: image1 == image2 (struct containing map[int]int cannot be compared).。

在 github 上可以找到本教程的代码。
-->

{% highlight go %}
{% endhighlight %}
