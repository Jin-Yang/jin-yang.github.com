---
title: Python C
layout: post
comments: true
category: [program, python]
language: chinese
---


<!-- more -->

<!--
#####################################
## 新增类型
#####################################
官方指导文档
https://linux.cn/article-5431-1.html
http://wangyuxxx.iteye.com/blog/1703252
http://www.spongeliu.com/165.html
https://blog.csdn.net/gatieme/article/details/50990456
https://my.oschina.net/sundq/blog/203600
https://blog.csdn.net/jinzhao1993/article/details/70665295
https://blog.csdn.net/ljianhui/article/details/10813469
https://docs.python.org/2.7/extending/newtypes.html
https://github.com/python/cpython/blob/master/Modules/xxsubtype.c
https://docs.python.org/2.7/extending/newtypes.html
https://blog.csdn.net/ldw220817/article/details/50112279
https://blog.dbrgn.ch/2017/3/10/write-a-collectd-python-plugin/

首先定义一个使用的对象，其中头部使用通用的 `PyObject_HEAD` 定义，注意，该宏定义之后没有逗号；宏定义之后对应了真实的使用对象，例如 Python 中的 int 定义如下：

typedef struct {
	PyObject_HEAD
	long ob_ival;
} PyIntObject;

接着定义一个 Python 中的对象，实际上就是该类型的元数据了，包括了引用计数、父类、类型名、大小、支持的方法等。

##### 头信息

例如头部的 `PyVarObject_HEAD_INIT(NULL, 0)` 实际上等价于 `PyVarObject_HEAD_INIT(&PyType_Type, 0)`，因为在通过 `PyType_Ready()`  初始化时，如果父类对应的是 NULL 会自动初始化为系统类型的父类。

##### tp_name

用来定义该类型的名称，一般会在出错时进行打印，这里使用的是该类型属于哪个模块 (foobar) 以及类型名 (Foobar)，例如如下的示例：

>>> import foobar
>>> "" + foobar.Foobar()
Traceback (most recent call last):
  File "<stdin>", line 1, in <module>
TypeError: cannot concatenate 'str' and 'foobar.Foobar' objects

##### tp_basicsize

用来定义实际对象的大小，实际上就是通过 `PyObject_New()` 分配内存时的大小。

有对象继承时大小的判断 https://docs.python.org/2.7/extending/newtypes.html ?????????

<!-- 
tp_itemsize tp_flags 

如果要创建对象需要定义 tp_new，这里直接使用的是默认的接口 PyType_GenericNew() 函数。接着通过 PyType_Ready() 函数初始化，一般包含了一些必须的指针。

>>> import foobar
>>> f = foobar.Foobar()

如果要打包时，可以添加一个打包脚本 setup.py ，如下。

from distutils.core import setup, Extension
setup(name="noddy", version="1.0", ext_modules=[Extension("noddy", ["noddy.c"])])

使用时直接通过 `python setup.py build` 命令编译。


## 类型增强


### 释放内存

因为新的类型支持新的对象，那么需要添加 tp_dealloc() 指针释放占用的内存以及对象。

由于无法确定对象是否为空指针，需要通过 Py_XDECREF() 减小对象的引用。

这里释放资源时使用的是 tp_free() 函数，因为如果支持子类，那么这里接收到的对象可能是子类。TODO: Why? When?

一般来说，tp_dealloc() 和  tp_free() 区别为，对外暴露是的前者，或者供前者调用；当一个字符串的引用计数为 0 时，tp_dealloc 会被调用，在其中处理一些关于缓存的事务后，最后靠 tp_free 释放真正的内存。

### 新建对象

希望在新建对象时，先将对象的 first、last 的初始值设置为空字符串，这里就需要添加 __new__ 指针的支持，如果不需要定制化，完全可以使用如上的通用  PyType_GenericNew() 函数即可。

这里真正创建对象是通过 tp_alloc 完成，如果不设置，在 PyType_Ready() 函数中会自动设置为默认的分配方式。

### 初始化对象

C 中设置的是 tp_init 对应到 Python 中的 __init__ 函数，通常用于在创建完对象后进行初始化。注意，不像 new 接口，这里无法保障该函数被调用，例如 unpickling 或者被覆盖时。

另外，该函数可能会被调用多次，可以被任何人调用，所以在设置新值时要格外小心。例如，如下代码：

if (first) {
    Py_XDECREF(self->first);
    Py_INCREF(first);
    self->first = first;
}

因为没有限制 first 对象的类型，那么在第一步中可能会调用析构函数，如果在该函数中同时访问第一个对象，那么可能会导致异常，所以一般是先赋值。

### 成员列表

这里直接设置 tp_members 对象即可，对应了一个多维数组，每个对象对应了成员名、类型、偏移、访问标识等，可以查看 [Generic Attribute Management section](https://docs.python.org/2.7/extending/newtypes.html#generic-attribute-management) 。

这种方式无法限制成员的类型，也就是可以设置任意类型的对象。

### 成员函数

这里创建一个简单的函数，实现的功能类似如下 Python 脚本：

def name(self):
	return "%s %s" % (self.first, self.last)

这里的 first、last 可能会被删除，此时对应的 C 代码就是 NULL，TODO: When?Why? 这里也可以通过代码限制删除功能。

### 指定类型

这里直接作为基类，将 tp_flags 设置为 Py_TPFLAGS_DEFAULT 。

>>> import foobar
>>> dir(foobar.Foobar)  查看包含了成员包含了first last name number

Unifying types and classes in Python



在 Python 中要将某一类型的变量或者常量转换为字符串对象通常有三种方法：`str()`、`repr()` 以及 ``。通过如下方式测试时，返回的类型都是字符串。

>>> a = 10
>>> type(str(a))
<class 'str'>
>>> type(repr(a))
<class 'str'>
>>> type(`a`)
<class 'str'>

简单来说，后面两者相同，返回的字符串值对 Python 是友好的，多数的类型返回字符串会包含了类型信息，一般用于调试；而前者是对用户友好的，一般返回的字符串是对应对象的值，用于对用户的输出结果。

>>> from datetime import datetime
>>> now = datetime.now()
>>> print(str(now))
2017-04-22 15:41:33.012917
>>> print(repr(now))
datetime.datetime(2017, 4, 22, 15, 41, 33, 12917)

通过 str() 的输出结果我们能很好地知道 now 实例的内容，但是却丢失了 now 实例的数据类型信息。而通过 repr() 的输出结果我们不仅能获得 now 实例的内容，还能知道 now 是 datetime.datetime 对象的实例。

实际上，str() 函数调用的是 __str__ 方法，而 repr() 对应了 __repr__ 方法，对于上述的示例，可以直接通过如下方式调用。

>>> from datetime import datetime
>>> now = datetime.now()
>>> print(now.__str__())
2017-04-22 15:41:33.012917
>>> print(now.__repr__())
datetime.datetime(2017, 4, 22, 15, 41, 33, 12917)

在 C 实现的源码中，分别对应了 `PyTypeObject` 结构体中的 `tp_str()` 和 `tp_rep()` 。

另外，如果想要自定义类的实例能够被 str() 和 repr() 所调用，那么就需要在自定义类中重载 __str__ 和 __repr__ 方法。



#####################################
## 模块扩展
#####################################

首先看一个简单的示例，该模块名为 `foobar` ，其中提供了一个函数 `system ()` 用于执行系统命令。

#include <Python.h>

static PyObject *foobar_system(PyObject *self, PyObject *args)
{
        const char *command;
        int sts;

        if (!PyArg_ParseTuple(args, "s", &command))
                return NULL;
        sts = system(command);
        return Py_BuildValue("i", sts);
}

static PyMethodDef FoobarMethods[] = {
        {"system", foobar_system, METH_VARARGS, "Execute a shell command."},
        {NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC initfoobar(void)
{
        (void)Py_InitModule("foobar", FoobarMethods);
}

在 CentOS 中，通过命令 `gcc -I/usr/include/python2.7 --shared -fPIC foobar.c -o foobar.so` 进行编译，然后在 Python 交互命令行中执行如下命令。

----- 导入模块，直接在编译好的路径下加载即可，会自动搜索当前目录
>>> import foobar
----- 通过该模块执行一些系统命令
>>> foobar.system("ls -alh")
----- 查看列表中的帮助文档
>>> foobar.system.__doc__
----- 该模块提供的方法以及成员
>>> dir(foobar)

简单介绍几个关键点。

PyMODINIT_FUNC 是一个兼容用的宏，在 Linux 或者 Windows 下面会有不同的替换，一般是用来在动态库中导出一个函数。

Py_InitModule() 用来初始化一个模块，这个是 2 中的版本，Python3 中会使用其它的 API 函数。

#### 导出函数

在 `FoobarMethods[]` 中定义了相关的导出函数，每个函数的声明都一样，也就是 `PyObject* method(PyObject* self, PyObject* args);` 。

该函数的入参含有两个参数：self 和 args ，其中前者一般在内联方法 (Builtin method) 时才被用到，例如类中的 self ，通常该参数的值为空(NULL)。

参数 args 中包含了 Python 解释器要传递给 C 函数的所有参数，一般使用 `PyArg_ParseTuple()` 来获得这些参数值。

所有的导出函数都返回一个 PyObject 指针，如果没有返回值，则应返回一个全局的 None 对象，如下所示：

PyObject* method(PyObject *self, PyObject *args) 
{
	Py_INCREF(Py_None);
	return Py_None;
}

#### 方法列表

也就是该模块可以被 Python 解释器使用的方法，每项由四个部分组成：方法名、导出函数、参数传递方式和方法描述。

参数传递方式有两种：METH_VARARGS 和 METH_KEYWORDS，前者是标准形式，通过元组传递参数；而后者则采用字典方式传参。

### 错误排查

其中有几处的命名需要按照 Python 的约束指定。

在加载动态库格式的模块时，会自动尝试查找 `initMODULE` 函数，其中 `MODULE` 是对应的模块名字，对于如上的示例会报：

ImportError: dynamic module does not define init function (initfoobar)

而且要保证通过 `Py_InitModule()` 指定的第一个参数与模块名称相同，否则会报如下的错误：

SystemError: dynamic module not initialized properly

## 常用操作

### 引用计数

Python 的内存自动回收是通过引用计数实现的，每个对象都有一个引用计数，用来统计该对象被引用了多少次，只有当引用计数为零时，才真正从内存中删除该对象。

Python 的 C 语言接口提供了一些宏来对引用计数进行维护，最常见的是用 Py_INCREF()/Py_DECREF() 分别用来增加/减少 1，这里的参数对象不能为 NULL，如果不确定则使用 Py_XINCREF()/Py_XDECREF()。

关于引用计数需要注意两类函数，详细可以参考 [Extending Python with C or C++](https://docs.python.org/2.7/extending/extending.html#ownership-rules)，如下仅列举两个：

PyObject* PyTuple_GetItem(PyObject *p, Py_ssize_t pos)
	Return value: Borrowed reference.
PyObject* PyObject_GetAttrString(PyObject *o, const char *attr_name)
	Return value: New reference.

平常使用较多的是 New reference，也就是能完全拥有的对象，简单点说就是引用计数已经加 1，可以保证在使用期间不会被释放。

而 Borrowed reference 其实只是一个指针，引用计数并没有增加，其实就是 Python 层次的 Weakref(弱引用)，所以使用时，你必须保证指向对象没有被释放。

例如，对于如下的代码：

void bug(PyObject *list)
{
    PyObject *item = PyList_GetItem(list, 0);
    PyList_SetItem(list, 1, PyInt_FromLong(0L));
    PyObject_Print(item, stdout, 0); /* BUG! */
}

上面完成功能就是从列表 list 中取出索引为 0 的对象 item，然后将list[1] 设置为 0，最后打印出 item 。

试想如下场景，在第二部设置 list[1] 时，需要先释放原来的 list[1](del list[1])，如果 list[1] 对象定义了 `__del__` 方法，如果在 `__del__` 方法中有可能 `del list[0]`，而通过 `PyList_GetItem()` 返回的是 Borrowed reference，这将导致 item 指针无效。

所以，在使用之前先手动增加一次引用，最后再减少一次就 OK 了，代码如下：

void no_bug(PyObject *list)
{
    PyObject *item = PyList_GetItem(list, 0);
    Py_INCREF(item);
    PyList_SetItem(list, 1, PyInt_FromLong(0L));
    PyObject_Print(item, stdout, 0);
    Py_DECREF(item);
}



#####################################
## Python 调用 C 函数
#####################################

http://www.cnblogs.com/phinecos/archive/2010/05/22/1741315.html



#####################################
## C 调用 Python 函数
#####################################

#include <Python.h>

int main()
{
        PyObject * module, *func, *result;

        Py_Initialize();
        if (Py_IsInitialized() == 0) {
                printf("init error\n");
                return -1;
        }
        PyRun_SimpleString("import sys");
        PyRun_SimpleString("sys.path.append('./')");

        module = PyImport_ImportModule("foobar");
        if (module == NULL) {
                printf("Cant open python file!\n");
                return -2;
        }

        func = PyObject_GetAttrString(module, "foobar");
        result =  PyObject_CallFunction(func, "i", 2);
        if (result == NULL) {
                printf("Invoke foobar() function failed\n");
                return -3;
        }
        printf("foobar() return: %s\n", PyString_AsString(result));

        Py_Finalize();
}

然后通过 `gcc -Wall -I/usr/include/python2.7 -lpython2.7 foobar.c -o foobar` 编译。

其中 Python 脚本 foobar.py 如下：

def foobar(s):
	print s
	return s
https://blog.csdn.net/lby978232/article/details/52769213

### 使用 BuildValue

Py_BuildValue() 的作用和 PyArg_ParseTuple() 的作用相反，它是将 C 类型的数据结构转换成 Python 对象，该函数的原型:

PyObject *Py_BuildValue(char *format, ...)

该函数可以和 PyArg_ParseTuple() 函数一样识别一系列的格式串，但是输入参数只能是值，而不能是指针，返回一个 C 中定义的 Python 对象。

另外注意，PyArg_ParseTuple() 的第一个参数为元组，而 Py_BuildValue()则不一定会生成一个元组，仅当格式串包含两个或者多个格式单元时会生产元组，如果格式串为空，返回 NONE。

关于格式定义可以直接参考 [The Py_BuildValue() Function](https://docs.python.org/2.0/ext/buildValue.html) 。

例如：
 
Py_BuildValue("")                     None
Py_BuildValue("i", 123)               123
Py_BuildValue("iii", 123, 456, 789)   (123, 456, 789)
Py_BuildValue("s", "hello")           'hello'
Py_BuildValue("ss", "hello", "world") ('hello', 'world')
Py_BuildValue("s#", "hello", 4)       'hell'
Py_BuildValue("()")                   ()
Py_BuildValue("(i)", 123)             (123,)
Py_BuildValue("(ii)", 123, 456)       (123, 456)
Py_BuildValue("(i,i)", 123, 456)      (123, 456)
Py_BuildValue("[i,i]", 123, 456)      [123, 456]
Py_BuildValue("{s:i,s:i}", "abc", 123, "def", 456) {'abc': 123, 'def': 456}
Py_BuildValue("((ii)(ii)) (ii)", 1, 2, 3, 4, 5, 6) (((1, 2), (3, 4)), (5, 6))

### 类型转换

Python 定义了六种数据类型：整型、浮点型、字符串、元组、列表和字典，在进行扩展时需要确定如何在 C/Python 之间进行类型转换。

#### 整型、浮点型和字符串

整型、浮点型和字符串这三种数据类型时相对比较简单。

// build an integer
PyObject* pInt = Py_BuildValue("i", 2003);
assert(PyInt_Check(pInt));
int i = PyInt_AsLong(pInt);
Py_DECREF(pInt);

// build a float
PyObject* pFloat = Py_BuildValue("f", 3.14f);
assert(PyFloat_Check(pFloat));
float f = PyFloat_AsDouble(pFloat);
Py_DECREF(pFloat);

// build a string
PyObject* pString = Py_BuildValue("s", "Python");
assert(PyString_Check(pString);
int nLen = PyString_Size(pString);
char* s = PyString_AsString(pString);
Py_DECREF(pString);

#### 元组

在实现时，元组是一个长度固定的数组，当 Python 解释器调用 C 语言扩展中的方法时，所有非关键字 (non-keyword) 参数都以元组方式进行传递。

// create the tuple
PyObject* pTuple = PyTuple_New(3);
assert(PyTuple_Check(pTuple));
assert(PyTuple_Size(pTuple) == 3);

// set the item
PyTuple_SetItem(pTuple, 0, Py_BuildValue("i", 2003));
PyTuple_SetItem(pTuple, 1, Py_BuildValue("f", 3.14f));
PyTuple_SetItem(pTuple, 2, Py_BuildValue("s", "Python"));

// parse tuple items
int i;
float f;
char *s;
if (!PyArg_ParseTuple(pTuple, "ifs", &i, &f, &s))
    PyErr_SetString(PyExc_TypeError, "invalid parameter");
// cleanup
Py_DECREF(pTuple);


#### 列表

Python 语言中的列表是一个长度可变的数组，列表比元组更为灵活，使用列表可以对其存储的 Python 对象进行随机访问。

// create the list
PyObject* pList = PyList_New(3); // new reference
assert(PyList_Check(pList));
// set some initial values
for(int i = 0; i < 3; ++i)
    PyList_SetItem(pList, i, Py_BuildValue("i", i));
// insert an item
PyList_Insert(pList, 2, Py_BuildValue("s", "inserted"));
// append an item
PyList_Append(pList, Py_BuildValue("s", "appended"));
// sort the list
PyList_Sort(pList);
// reverse the list
PyList_Reverse(pList);
// fetch and manipulate a list slice
PyObject* pSlice = PyList_GetSlice(pList, 2, 4); // new reference
for(int j = 0; j < PyList_Size(pSlice); ++j) {
  PyObject *pValue = PyList_GetItem(pList, j);
  assert(pValue);
}
Py_DECREF(pSlice);
// cleanup
Py_DECREF(pList);

#### 字典

Python语言中的字典是一个根据关键字进行访问的数据类型。

// create the dictionary
PyObject* pDict = PyDict_New(); // new reference
assert(PyDict_Check(pDict));
// add a few named values
PyDict_SetItemString(pDict, "first", 
                     Py_BuildValue("i", 2003));
PyDict_SetItemString(pDict, "second", 
                     Py_BuildValue("f", 3.14f));
// enumerate all named values
PyObject* pKeys = PyDict_Keys(); // new reference
for(int i = 0; i < PyList_Size(pKeys); ++i) {
  PyObject *pKey = PyList_GetItem(pKeys, i);
  PyObject *pValue = PyDict_GetItem(pDict, pKey);
  assert(pValue);
}
Py_DECREF(pKeys);
// remove a named value
PyDict_DelItemString(pDict, "second");
// cleanup
Py_DECREF(pDict);

#####################################
## 异常处理
#####################################

下面一段代码是在上一篇文章中的增强版，主要就是在异常处理方面。

foobar.system("false")
Traceback (most recent call last):
  File "<stdin>", line 1, in <module>
foobar.IOError: system call, IO failed

下面是几点扩展知识点：

1. 如果要忽略函数抛出的异常，可以用 PyErr_Clear()，因为异常只有传递到 Python 解释器时才起作用，所以在 C API 层次可以清除；
2. 对于一些标准的异常，例如内存函数申请失败异常，可以直接设置 PyErr_NoMemory()，并需要将异常指示器返回，简单来说就是 return PyErr_NoMemory()；

#include <Python.h>

static PyObject *FoobarError;
static PyObject *FoobarIOError;

static PyObject *foobar_system(PyObject *self, PyObject *args)
{
        const char *command;
        int rc;

        if (!PyArg_ParseTuple(args, "s", &command))
                return NULL;
        rc = system(command);
        if (rc < 0) { /* -1 system call failed; */
                PyErr_SetString(FoobarError,"system call failed");
                return NULL;
        } else if (rc > 0) {
                PyErr_SetString(FoobarIOError,"system call, IO failed");
                return NULL;
        }

        return Py_BuildValue("i", rc);
        //return PyLong_FromLong(rc);
}

static PyMethodDef FoobarMethods[] = {
        {"system", foobar_system, METH_VARARGS, "Execute a shell command."},
        {NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC initfoobar(void)
{
        PyObject *mod;
        mod = Py_InitModule("foobar", FoobarMethods);
        if (mod == NULL)
                return;

        FoobarError = PyErr_NewException("foobar.error", NULL, NULL);
        FoobarIOError = PyErr_NewException("foobar.IOError",NULL,NULL);
        Py_INCREF(FoobarError);
        Py_INCREF(FoobarIOError);
        PyModule_AddObject(mod, "error", FoobarError);
        PyModule_AddObject(mod, "IOError", FoobarIOError);
        PyModule_AddObject(mod, "Version", PyString_FromString("1.0"));
}
-->



{% highlight python %}
{% endhighlight %}
