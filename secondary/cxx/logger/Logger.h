/**********************************************************************
* Author:	jaron.ho
* Date:		2019-08-25
* Brief:	日志模块
**********************************************************************/
#ifndef _LOGGER_H_
#define _LOGGER_H_

#include <string>

class Logger {
private:
    /*
     * Brief:   单例模式,不允许外部实例化
     * Param:   void
     * Return:  void
     */
    Logger(void);
    ~Logger(void);

public:
    /*
     * Brief:   单例
     * Param:   void
     * Return:  Logger*
     */
    static Logger* getInstance(void);

public:
    /*
     * Brief:   打开系统日志
     * Param:   ident - 系统日志标识, 例如: "test_cxx"
     * Return:  void
     */
    void openSyslog(const std::string& ident);

    /*
     * Brief:   写入系统日志
     * Param:   content - 日志内容
     * Return:  void
     */
    void writeSyslog(const std::string& content);

    /*
     * Brief:   设置日志路径
     * Param:   path - 路径, 例如: /home/dev/
     * Return:  void
     */
    void setPath(const std::string& path);
    
    /*
     * Brief:   设置日志文件名
     * Param:   filename - 文件名, 例如: mylog.log
     * Return:  void
     */
    void setFilename(const std::string& filename);

    /*
     * Brief:   打印并写入普通日志
     * Param:   content - 日志内容
     *          tag - 日志标签, 例如: "user1"
     *          withTag - 日志标签是否需要
     *          withTime - 日志是否要时间头部
     * Return:  std::string, 当前日期, 例如: "2019-08-25 22:29:12"
     */
    std::string print(const std::string& content, const std::string& tag = "", bool withTag = true, bool withTime = true);

    /*
     * Brief:   打印并写入普通日志和系统日志,linux中,系统日志文件:"/var/log/syslog"
     * Param:   content - 日志内容
     *          tag - 日志标签, 例如: "user1"
     * Return:  std::string, 当前日期, 例如: "2019-08-25 22:29:12"
     */
    std::string printSyslog(const std::string& content, const std::string& tag);

private:
    /*
     * Brief:   获取日志头部
     * Param:   filename - 文件名(输出), 例如: "20190825.log"
     *          tag - 日志标签, 例如: "user1"
     *          withTag - 日志标签是否需要
     *          withTime - 日志是否要时间头部
     * Return:  std::string, 当前日期, 例如: "2019-08-25 22:29:12"
     */
    std::string header(std::string& filename, const std::string& tag = "", bool withTag = true, bool withTime = true);

private:
    bool mSysLogOpened;
    std::string mPath;
    std::string mFilename;
};

#endif // _LOGGER_H_
