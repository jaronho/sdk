# 一、说明

**WINQ**（WCDB语言集成查询）：通过WINQ，开发者无须为了拼接SQL的字符串而写一大坨胶水代码。

官网地址：[https://github.com/Tencent/wcdb](https://github.com/Tencent/wcdb)。

# 二、裁剪

把`wcdb-1.0.8.2\objc\WCDB\abstract\`里的文件拷贝到当前目录

## 第1步

删除无关的文件。

* fts_module.cpp
* fts_module.hpp
* fts_modules.cpp
* fts_modules.hpp
* handle.cpp
* handle.hpp
* handle_statement.cpp
* handle_statement.hpp

## 第2步

删除以下文件中对`handle.hpp`的包含。

* statement.cpp
* statement_insert.cpp
* statement_select.cpp
* statement_update.cpp

## 第3步

查找文件中`std::list<const`的代码，需要去除`const`修饰符，即改为`std::list<`，否则使用VS编译可能出错：

```
Error C2338: The C++ Standard forbids containers of const elements because allocator<const T> is ill-formed
```

因为在STL容器模板中使用了`const`，在C++11的标准里，这是禁止的。而老版本的Visual Studio并没有这么严格，所以一般可以编译过。

