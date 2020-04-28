/**********************************************************************
 * Author:	jaron.ho
 * Date:    2019-07-22
 * Brief:	全局宏定义
 **********************************************************************/
#ifndef __MACROS_H__
#define __MACROS_H__

#define G_JOIN(a, b)              a##b                          /* 拼接符号a和b为ab */
#define G_STR(a)                  (#a)                          /* 获取符号a的字符串名 */
#define G_NAME_VALUE(var)         {#var, var}                   /* 生成键值对:{变量名,变量值} */

#endif	/* __MACROS_H__ */
