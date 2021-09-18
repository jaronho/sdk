#pragma once

#include <functional>
#include <mutex>
#include <sqlite3.h>
#include <string>
#include <unordered_map>

namespace database
{
class Sqlite
{
public:
    /**
     * @brief 构造函数
     * @param path 数据库路径
     * @param password 数据库密码(选填) 
     */
    Sqlite(const std::string& path, const std::string& password = "");

    ~Sqlite();

    /* 禁止拷贝移动 */
    Sqlite(const Sqlite& other) = delete;
    Sqlite(Sqlite&& other) noexcept = delete;
    Sqlite& operator=(const Sqlite& other) = delete;
    Sqlite& operator=(Sqlite&& other) noexcept = delete;

    /**
     * @brief 获取数据库路径
     * @return 数据库路径
     */
    std::string getPath();

    /**
     * @brief 连接数据库
     * @param readOnly 是否只读
     * @return true-成功, false-失败 
     */
    bool connect(bool readOnly = false);

    /**
     * @brief 断开连接数据库
     */
    void disconnect();

    /**
     * @brief 执行sql语句
     * @param sql sql语句
     * @param callback 每一行的回调(选填), 参数: columns-列数据(key:字段名, value:字段值), 返回值: true-继续, false-停止
     * @param errorMsg 错误消息(选填)
     */
    int64_t execSql(const std::string& sql,
                    const std::function<bool(const std::unordered_map<std::string, std::string>& columns)>& callback = nullptr,
                    std::string* errorMsg = nullptr);

    /**
     * @brief 获取最后一次插入成功的表的行号
     * @return 行号
     */
    int64_t getLastInsertRowId();

private:
    std::mutex m_mutex;
    sqlite3* m_db; /* 数据库指针 */
    std::string m_path; /* 数据库路径(全路径) */
    std::string m_password; /* 数据库密码 */
};
} // namespace database
