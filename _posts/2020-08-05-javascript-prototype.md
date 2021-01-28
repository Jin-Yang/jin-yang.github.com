---
title: JavaScript 原型介绍
layout: post
comments: true
language: chinese
tag: [JavaScript, Program]
keywords: javascript, prototype, 原型
description: 在 JavaScript 中，通过原型来实现对象属性继承，而原型实际上也是一个对象，每个对象都会包含了一个 Prototype 内部属性，这个属性所对应的就是该对象的原型。
---

在 JavaScript 中，除了基础类型外都是对象，包括了函数，即使在 ES6 中引入了类，但也只是语法糖，那么如何将对象联系起来就成了问题，这也就有了原型和原型链的概念。

通过原型来实现对象属性继承，而原型实际上也是一个对象，每个对象都会包含了一个 `[[Prototype]]` 内部属性，这个属性所对应的就是该对象的原型。

<!-- more -->

## 简介

其中 `[[Prototype]]` 作为对象的内部属性，无法直接访问，为了方便查看，在 Firefox 和 Chrome 提供了 `__proto__` 用来访问，不过这个不是非标准的，ECMA 引入了标准原型访问器 `Object.getPrototype(object)` 。
