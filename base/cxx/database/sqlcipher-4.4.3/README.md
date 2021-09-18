# sqlcipher

## 说明

该库是基于`sqlite3`以及`openssl`的加解密封装。

官网地址：[https://github.com/sqlcipher/sqlcipher](https://github.com/sqlcipher/sqlcipher)。

对于下载到本地的源码库通常做法是通过编译出动态或静态链接库的方式，但是对于大的项目，或者为了C编译器可以执行更多跨过程分析并生成更好的代码，可以把所有文件集成到一个大的源文件中，且库的运行速度可以提高5%左右。

打包成大的源文件方式：使用官方介绍的`make target_source`貌似没用，自己摸索出了另一种方法：在linux下，先执行`configure`命令，再执行`make`命令，然后把以下4个文件拷贝出来：

* shell.c
* sqlite3.c
* sqlite3.h
* sqlite3ext.h

## 使用

要开启加密功能，需要在工程中定义宏：

* SQLITE_HAS_CODEC

* SQLCIPHER_CRYPTO_OPENSSL

开启加密功能时，在连接数据库时会耗时比较慢（可能得好几秒），猜测可能是由于openssl的原因。