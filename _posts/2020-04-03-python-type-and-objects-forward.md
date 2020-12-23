---
title: 【转】Python Types and Objects
layout: post
comments: true
language: chinese
tag: [SoftWare, Python, Forward]
keywords: Python, Types, Objects
description: 这是一篇很经典的文章，介绍 Python 中与 Type 和 Object 相关的概念，不过最近发现源文章好像已经无法访问，所以把发现的一篇 PDF 重新整理成网页。原作者为  Shalabh Chaturvedi ，这里仅转发。
---

这是一篇很经典的文章，介绍 Python 中与 Type 和 Object 相关的概念，不过最近发现源文章好像已经无法访问，所以把发现的一篇 PDF 重新整理成网页。

原作者为  Shalabh Chaturvedi ，这里仅转发，但会使用 Python3 做实验，以飨读者。

<!-- more -->

## About This Book

Explains Python new-style objects:

* what are `<type 'type'>` and `<type 'object'>`
* how user defined classes and instances are related to each other and to built-in types
* what are metaclasses

New-style implies Python version 2.2 and upto and including 3.x. There have been some behavioral changes during these version but all the concepts covered here are valid. The system described is sometimes called the Python type system, or the object model.

This book is part of a series:

* Python Types and Objects [you are here]
* [Python Attributes and Methods](/post/python-attributes-and-methods-forward.html)

## Before You Begin

Some points you should note:

* This book covers the new-style objects (introduced a long time ago in Python2.2). Examples are valid for Python 2.5 and all the way to Python 3.x.
* This book is not for absolute beginners. It is for people who already know Python (even a little Python) and want to know more.
* This book provides a background essential for grasping new-style attribute access and other mechanisms (descriptors, properties and the like). If you are interested in only attribute access, you could go straight to [Python Attributes and Methods](/post/python-attributes-and-methods-forward.html), after verifying that you understand the Summary of this book.

Happy pythoneering!

## Chapter 1. Basic Concepts

### The Object Within

So what exactly is a Python object? An object is an axiom in our system - it is the notion of some entity. We still define an object by saying it has:

* Identity (i.e. given two names we can say for sure if they refer to one and the same object, or not).
* A value - which may include a bunch of attributes (i.e. we can reach other objects through `objectname.attributename`).
* A type - every object has exactly one type. For instance, the object `2` has a type `int` and the object `"joe"` has a type `string`.
* One or more bases. Not all objects have bases but some special ones do. A base is similar to a super-class or base-class in object-oriented lingo.

If you are more of the 'I like to know how the bits are laid out' type as opposed to the 'I like the meta abstract ideas' type, it might be useful for you to know that each object also has a specific location in main memory that you can find by calling the `id()` function.

The type and bases (if they exist) are important because they define special relationships an object has with other objects. Keep in mind that the types and bases of objects just other objects. This will be re-visited soon.

You might think an object has a name but the name is not really part of the object. The name exists outside of the object in a namespace (e.g. a function local variable) or as an attribute of another object.

Even a simple object such as the number `2` has a lot more to it than meets the eye.

**<center>Example 1.1. Examining an integer object</center>**

```
>>> two = 2                <1>
>>> type(two)
<class 'int'>              <2>
>>> type(type(two))
<class 'type'>             <3>
>>> type(two).__bases__
(<class 'object'>,)        <4>
>>> dir(two)               <5>
['__abs__', '__add__', '__and__', '__bool__', '__ceil__', '__class__', '__delattr__', '__dir__', '__divmod__', '__doc__', '__eq__', '__float__', '__floor__', '__floordiv__', '__format__', '__ge__', '__getattribute__', '__getnewargs__', '__gt__', '__hash__', '__index__', '__init__', '__init_subclass__', '__int__', '__invert__', '__le__', '__lshift__', '__lt__', '__mod__', '__mul__', '__ne__', '__neg__', '__new__', '__or__', '__pos__', '__pow__', '__radd__', '__rand__', '__rdivmod__', '__reduce__', '__reduce_ex__', '__repr__', '__rfloordiv__', '__rlshift__', '__rmod__', '__rmul__', '__ror__', '__round__', '__rpow__', '__rrshift__', '__rshift__', '__rsub__', '__rtruediv__', '__rxor__', '__setattr__', '__sizeof__', '__str__', '__sub__', '__subclasshook__', '__truediv__', '__trunc__', '__xor__', 'bit_length', 'conjugate', 'denominator', 'from_bytes', 'imag', 'numerator', 'real', 'to_bytes']
```

注意，新类将 `type` 和 `class` 进行了统一 (详见 [新类 VS. 旧类](/post/python-old-new-style-class.html) 中的介绍)，不过在 Python2 中，对于内置的类型其实仍然会显示 `<type 'XXX'>`，而在 Python3 中则进行了完全统一，也就是上述显示的内容，也就是说两者是等价的。

* `<1>` Here we give an integer the name two in the current namespace.
* `<2>` The type of this object is `<class 'int'>`. This `<class 'int'>` is another object, which we will now explore. Note that this object is also called just `int` and `<class 'int'>` is the printable representation.
* `<3>` Hmm.. the type of `<class 'int'>` is an object called `<class 'type'>`.
* `<4>` Also, the `__bases__` attribute of `<class 'int'>` is a tuple containing an object called `<class 'object'>`. Bet you didn't think of checking the `__bases__` attribute ;).
* `<5>` Let's list all the attributes present on this original integer object - wow that's alot.

You might say "What does all this mean?" and I might say "Patience! First, let's goover the first rule."

> Rule #1 Everything is an object

The built-in `int` is an object. This doesn't mean that just the numbers such as `2` and `77` are objects (which they are) but also that there is another object called `int` that is sitting in memory right beside the actual integers. In fact all integer objects are pointing to `int` using their `__class__` attribute saying "that guy really knows me". Calling `type()` on an object just returns the value of the `__class__` attribute.

Any classes that we define are objects, and of course, instances of those classes are objects as well. Even the functions and methods we define are objects. Yet, as we will see, all objects are not equal.

### A Clean Slate

We now build the Python object system from scratch. Let us begin at the beginning - with a clean slate.

**<center>Figure 1.1. A Clean Slate</center>**

You might be wondering why a clean slate has two grey lines running vertically through it. All will be revealed when you are ready. For now this will help distinguish a slate from another figure. On this clean slate, we will gradually put different objects,and draw various relationships, till it is left looking quite full.

At this point, it helps if any preconceived object oriented notions of classes and objects are set aside, and everything is perceived in terms of objects (our objects) and relationships.

### Relationships

As we introduce many different objects, we use two kinds of relationships to connect. These are the *subclass-superclass* relationship (a.k.a. specialization or inheritance, "man is an animal", etc.) and the *type-instance* relationship (a.k.a instantiation, "Joe is a man", etc.). If you are familiar with these concepts, all is well and you can proceed, otherwise you might want to take a detour through the section called "Object-Oriented Relationships".

## Chapter 2. Bring In The Objects

### The First Objects

We examine two objects: `<class 'object'>` and `<class 'type'>`.

**<center>Example 2.1. Examining <class 'object'> and <class 'type'></center>**

```
>>> object                  <1>
<class 'object'>
>>> type                    <2>
<class 'type'>
>>> type(object)            <3>
<class 'type'>
>>> object.__class__        <4>
<class 'type'>
>>> object.__bases__        <5>
()
>>> type.__class__          <6>
<class 'type'>
>>> type.__bases__          <7>
(<class 'object'>,)
```

* `<1>` `<2>` The names of the two primitive objects within Python. Earlier `type()` was introduced as a way to find the type of an object (specifically, the `__class__` attribute). In reality, it is both - an object itself, and a way to get the type of another object.
* `<3>` `<4>` `<5>` Exploring `<class 'object'>`: the type of `<class 'object'>` is `<class 'type'>`. We also use the `__class__` attribute and verify it is the same as calling `type()`.
* `<6>` `<7>` Exploring `<class 'type'>`: interestingly, the type of `<class 'type'>` is itself! The `__bases__` attribute points to `<class 'object'>`.

Let's make use of our slate and draw what we've seen.

**<center>Figure 2.1. Chicken and Egg</center>**

These two objects are primitive objects in Python. We might as well have introduced them one at a time but that would lead to the chicken and egg problem - which to introduce first? These two objects are interdependent - they cannot stand on their own since they are defined in terms of each other.

Continuing our Python experimentation:

**<center>Example 2.2. There's more to <type 'object'> and <type 'type'></center>**

```
>>> isinstance(object, object)   <1>
True
>>> isinstance(type, object)     <2>
True
```

* `<1>` Whoa! What happened here? This is just Dashed Arrow Up Rule in action. Since `<class 'type'>` is a subclass of `<class 'object'>`, instances of `<class 'type'>` are instances of `<class 'object'>` as well.
* `<2>` Applying both Dashed Arrow Up Rule and Dashed Arrow Down Rule, we can effectively reverse the direction of the dashed arrow. Yes, it is still consistent.

If the above example proves too confusing, ignore it - it is not much use anyway.

Now for a new concept - *type objects*. Both the objects we introduced are type objects. So what do we mean by type objects? Type objects share the following traits:

* They are used to represent abstract data types in programs. For instance, one (user defined) object called `User` might represent all users in a system, another once called `int` might represent all integers.
* They can be *subclassed*. This means you can create a new object that is somewhat similar to exsiting type objects. The existing type objects become bases for the new one.
* They can be *instantiated*. This means you can create a new object that is an instance of the existing type object. The existing type object becomes the `__class__` for the new object.
* The type of any type object is `<class 'type'>`.
* They are lovingly called *types* by some and *classes* by others.

Yes you read that right. Types and classes are really the same in Python (disclaimer: this doesn't apply to old-style classes or pre-2.2 versions of Python. Back then types and classes had their differences but that was a long time ago and they have since reconciled their differences so let bygones be bygones, shall we?). No wonder the `type()` function and the `__class__` attribute get you the same thing.

The term *class* was traditionally used to refer to a class created by the `class` statement. Built-in types (such as `int` and `string`) are not usually referred to as classes, but that's more of a convention thing and in reality types and classes are exactly the same thing. In fact, I think this is important enough to put in a rule:

> Class is Type is Class
> The term *type* is equivalent to the term *class* in all version of Python >= 2.3.

Types and (er.. for lack of a better word) non-types (ugh!) are both objects but only types can have subcasses. Non-types are concrete values so it does not make sense for another object be a subclass. Two good examples of objects that are not types are the integer `2` and the string `"hello"`. Hmm.. what does it mean to be a subclass of `2`?

Still confused about what is a type and what is not? Here's a handy rule for you:

> Type Or Non-type Test Rule
> If an object is an instance of `<class 'type'>`, then it is a type. Otherwise, it is not a type.

Looking back, you can verify that this is true for all objects we have come across, including `<class 'type'>` which is an instance of itself.

To summarize:

* `<class 'object'>` is an instance of `<class 'type'>`.
* `<class 'object'>` is a subclass of no object.
* `<class 'type'>` is an instance of itself.
* `<class 'type'>` is a subclass of `<class 'object'>`.
* There are only two kinds of objects in Python: to be unambiguous let's call these *types* and *non-types*. Non-types could be called instances, but that term could also refer to a type, since a type is always an instance of another type. Types could also be called classes, and I do call them classes from time to time.

Note that we are drawing arrows on our slate for only the *direct* relationships, not the implied ones (i.e. only if one object is another's `__class__`, or in the other's `__bases__`). This make economic use of the slate and our mental capacity.

### More Built-in Types

Python does not ship with only two objects. Oh no, the two primitives come with a whole gang of buddies.

**<center>Figure 2.2. Some Built-in Types</center>**

A few built-in types are shown above, and examined below.

**<center>Example 2.3. Examining some built-in types</center>**

```
>>> list                                    <1>
<class 'list'>
>>> list.__class__                          <2>
<class 'type'>
>>> list.__bases__                          <3>
(<class 'object'>,)
>>> tuple.__class__, tuple.__bases__        <4>
(<class 'type'>, (<class 'object'>,))
>>> dict.__class__, dict.__bases__          <5>
(<class 'type'>, (<class 'object'>,))
>>> mylist = [1, 2, 3]                      <6>
>>> mylist.__class__                        <7>
<class 'list'>
```

* `<1>` The built-in `<class 'list'>` object.
* `<2>` Its type is `<class 'type'>`.
* `<3>` It has one base (a.k.a. superclass), `<class 'object'>`.
* `<4>` `<5>` Ditto for `<class 'tuple'>` and `<class 'dict'>`.
* `<6>` This is how you create an instance of `<class 'list'>`.
* `<7>` The type of a list is `<class 'list'>`. No surprises here.

When we create a tuple or a dictionary, they are instances of the respective types.

So how can we create an *instance* of `mylist`? We cannot. This is because `mylist` is a not a type.

### New Objects by Subclassing

The built-in objects are, well, *built into* Python. They're there when we start Python, usually there when we finish. So how can we create new objects?

New objects cannot pop out of thin air. They have to be built using existing objects.

**<center>Example 2.4. Creating new objects by subclassing</center>**

```
# In Python 2.x:
class C(object):            <1>
	pass

# In Python 3.x, the explicit base class is not required, classes are
# automatically subclasses of object:
class C:                    <2>
	pass

class D(object):
	pass

class E(C, D):              <3>
	pass

class MyList(list):         <4>
	pass
```

* `<1>` The `class` statement tells Python to create a new type by subclassing an existing type.
* `<2>` Don't do this in Python 2.x or you will end up with an object that is an old-style class, everything you read here will be useless and all will be lost.
* `<3>` Multiple bases are fine too.
* `<4>` Most built-in types can be subclassed (but not all).

After the above example, `C.__bases__` contains `<class 'object'>`, and `MyList.__bases__` contains `<class 'list'>`.

### New Objects by Instantiating

Subclassing is only half the story.

**<center>Example 2.5. Creating new objects by instantiating</center>**


```
obj = object()          <1>
cobj = C()              <2>
mylist = [1,2,3]        <3>
```

* `<1>` The call operator (`()`) creates a new object by instantiating an existing object.
* `<2>` The existing object *must* be a type. Depending on the type, the call operator might accept arguments.
* `<3>` Python syntax creates new objects for some built-in types. The square brackets create an instance of `<class 'list'>`; a numeric literal creates an instance of `<class'int'>`.

After the above exercise, our slate looks quite full.

**<center>Figure 2.3. User Built Objects</center>**

Note that by just subclassing `<class 'object'>`, the type `C` automatically is an instance of `<class 'type'>`. This can be verified by checking `C.__class__`. Why this happens is explained in the next section.

### It's All Instantiation, Really

Some questions are probably popping up in your head at this point. Or maybe they aren't, but I'll answer them anyway:

* Q: How does Python really create a new object?
* A: Internally, when Python creates a new object, it always uses a type and creates an instance of that object. Specifically it uses the `__new__()` and `__init__()` methods of the type (discussion of those is outside the scope of this book). In a sense, the type serves as a factory that can churn out new objects. The type of these manufactured objects will be the type object used to create them. This is why every object has a type.

* Q: When using instantiation, I specify the type, but how does Python know which type to use when I use subclassing?
* A: It looks at the base class that you specified, and uses its type as the type for the new object. In the example Example 2.4, "Creating new objects by subclassing", `<class 'type'>` (the type of `<class 'object'>`, the specified base) is used as the type object for creating C.<br>A little thought reveals that under most circumstances, any subclasses of `<class 'object'>` (and their subclasses, and so on) will have `<class 'type'>` as their type.

> Advanced Material Ahead
> Advanced discussion ahead, tread with caution, or jump straight to the next section.

* Q: Can I instead specify a type object to use?
* A: Yes. One option is by using the `__metaclass__` class attribute as in the following example:

**<center>Example 2.6. Specifying a type object while using class statement</center>**

```
class MyCWithSpecialType(object):
	__metaclass__ = SpecialType
```

Now Python will create `MyCWithSpecialType` by instantiating `SpecialType`, and not `<class 'type'>`.

* Q: Wow! Can I use any type object as the `__metaclass__`?
* A: No. It must be a subclass of the type of the base object. In the above example:
	* Base of `MyCWithSpecialType` is `<class 'object'>`.
	* Type of `<class 'object'>` is `<class 'type'>`.
	* Therefore `SpecialType` must be a subclass of `<class 'type'>`.
Implementation of something like `SpecialType` requires special care and is out of scope for this book.


Q:What if I have multiple bases, and don't specify a __metaclass__ - which type object will be used?A:Good Question. Depends if Python can figure out which one to use. If all thebases have the same type, for example, then that will be used. If they havedifferent types that are not related, then Python cannot figure out which typeobject to use. In this case specifying a __metaclass__ is required, and this__metaclass__ must be a subclass of the type of each base.Q:When should I use a __metaclass__?A:Never (as long as you're asking this question anyway :)Chapter 3. Wrap UpThe Python Objects MapWe really ended up with a map of different kinds of Python objects in the lastchapter.Figure 3.1. The Python Objects Map
Python Types and Objectshttp://www.cafepy.com/article/python_types_and_objects/...15 of 2211/25/11 10:14Here we also unravel the mystery of the vertical grey lines. They just segregateobjects into three spaces based on what the common man calls them - metaclasses, classes, or instances.Various pedantic observations of the diagram above:Dashed lines cross spacial boundaries (i.e. go from object to meta-object). Only exception is <type 'type'> (which is good, otherwise we would need another space to the left of it, and another, and another...).1.Solid lines do not cross space boundaries. Again, <type 'type'> -> <type 'object'> is an exception.2.Solid lines are not allowed in the rightmost space. These objects are tooconcrete to be subclassed.3.Dashed line arrow heads are not allowed rightmost space. These objects aretoo concrete to be instantiated.4.Left two spaces contain types. Rightmost space contains non-types.5.If we created a new object by subclassing <type 'type'> it would be in the leftmostspace, and would also be both a subclass and instance of <type 'type'>.6.
Python Types and Objectshttp://www.cafepy.com/article/python_types_and_objects/...16 of 2211/25/11 10:14Also note that <type 'type'> is indeed a type of all types, and <type 'object'> a superclass of all types (except itself).SummaryTo summarize all that has been said:There are two kinds of objects in Python:Type objects - can create instances, can be subclassed.1.Non-type objects - cannot create instances, cannot be subclassed.2.<type 'type'> and <type 'object'> are two primitive objects of the system.objectname.__class__ exists for every object and points the type of the object.objectname.__bases__ exists for every type object and points the superclasses of theobject. It is empty only for <type 'object'>. To create a new object using subclassing, we use the class statement and specify the bases (and, optionally, the type) of the new object. This alwayscreates a type object.To create a new object using instantiation, we use the call operator (()) on the type object we want to use. This may create a type or a non-type object,depending on which type object was used.Some non-type objects can be created using special Python syntax. Forexample, [1, 2, 3] creates an instance of <type 'list'>.Internally, Python always uses a type object to create a new object. The newobject created is an instance of the type object used. Python determines thetype object from a class statement by looking at the bases specified, and findingtheir types.issubclass(A,B) (testing for superclass-subclass relationship) returns True iff:B is in A.__bases__, or1.issubclass(Z,B) is true for any Z in A.__bases__.2.isinstance(A,B) (testing for type-instance relationship) returns True iff:
Python Types and Objectshttp://www.cafepy.com/article/python_types_and_objects/...17 of 2211/25/11 10:14B is A.__class__, or1.issubclass(A.__class__,B) is true.2.Squasher is really a python. (Okay, that wasn't mentioned before, but now you know.)More Types to Play WithThe following example shows how to discover and experiment with built-in types.Example 3.1. More built-in types>>> import types >>> types.ListType is list True>>> def f(): ...     pass...>>> f.__class__ is types.FunctionType True>>>>>> class MyList(list): ...     pass...>>> class MyFunction(types.FunctionType): ...     pass...Traceback (most recent call last):  File "<stdin>", line 1, in ?TypeError: type 'function' is not an acceptable base type>>> dir(types) ['BooleanType', 'DictProxyType', 'DictType', ..]The types module contains many built-in types.Some well known types have another name as well.def creates a function object.The type of a function object is types.FunctionTypeSome built-in types can be subclassed.Some cannot.More types than you can shake a stick at.What's the Point, Anyway?
Python Types and Objectshttp://www.cafepy.com/article/python_types_and_objects/...18 of 2211/25/11 10:14So we can create new objects with any relationship we choose, but what does it buyus?The relationships between objects determine how attribute access on the objectworks. For example, when we say objectname.attributename, which object do we end upwith? It all depends on objectname, its type, and its bases (if they exist).Attribute access mechanisms in Python are explained in the second book of thisseries: Python Attributes and Methods.Classic ClassesThis is a note about classic classes in Python. We can create classes of the old (pre2.2) kind by using a plain class statement.Example 3.2. Examining classic classes>>> class ClassicClass: ...     pass...>>> type(ClassicClass) <type 'classobj'>>>> import types>>> types.ClassType is type(ClassicClass) True>>> types.ClassType.__class__ <type 'type'>>>> types.ClassType.__bases__ (<type 'object'>,)A class statement specifying no bases creates a classic class Remember thatto create a new-style class you must specify object as the base (although this is not required in Python 3.0 since new-style classes are the default). Specifyingonly classic classes as bases also creates a classic class. Specifying bothclassic and new-style classes as bases create a new-style class.Its type is an object we haven't seen before (in this book).The type of classic classes is an object called types.ClassType.It looks and smells like just another type object.The types.ClassType object is in some ways an alternative <type 'type'>. Instances of this object (classic classes) are types themselves. The rules of attribute access aredifferent for classic classes and new-style classes. The types.ClassType object exists for backward compatibility and may not exist in future versions of Python. Other sections
Python Types and Objectshttp://www.cafepy.com/article/python_types_and_objects/...19 of 2211/25/11 10:14of this book should not be applied to classic classes.Comment on this book here: discussion page. I appreciate feedback!That's all, folks!




## 参考

* [Python Types and Objects](https://www.eecg.utoronto.ca/~jzhu/csc326/readings/metaclass-class-instance.pdf) 也就是从这个 PDF 中摘抄的。

<!--
http://www.cafepy.com/article/python_types_and_objects/python_types_and_objects.html
-->
