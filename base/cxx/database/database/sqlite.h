#pragma once

#include <functional>
#include <memory>
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
     * @brief SQL预编译(主要用于批量操作, 可以提高效率), 注意: 非线程安全
     */
    class Stmt
    {
    public:
        /**
         * @brief 构造函数
         * @param db 数据库句柄
         * @param sql SQL语句
         */
        Stmt(sqlite3* db, const std::string& sql);

        ~Stmt();

        /**
         * @brief 预编译指令(默认构造函数中已执行)
         * @param db 数据库句柄
         * @param sql SQL语句
         * @return true-成功, false-失败
         */
        bool prepare(sqlite3* db, const std::string& sql);

        /**
         * @brief 绑定SQL语句中的int参数
         * @param index 参数的索引值(从0开始)
         * @param val 值
         * @return true-成功, false-失败
         */
        bool bind(int index, int val);

        /**
         * @brief 绑定SQL语句中的int64参数
         * @param index 参数的索引值(从0开始)
         * @param val 值
         * @return true-成功, false-失败
         */
        bool bind(int index, int64_t val);

        /**
         * @brief 绑定SQL语句中的double参数
         * @param index 参数的索引值(从0开始)
         * @param val 值
         * @return true-成功, false-失败
         */
        bool bind(int index, double val);

        /**
         * @brief 绑定SQL语句中的string参数
         * @param index 参数的索引值(从0开始)
         * @param val 值
         * @return true-成功, false-失败
         */
        bool bind(int index, const std::string& val);

        /**
         * @brief 执行一次预编译指令
         * @return true-成功, false-失败
         */
        bool step();

        /**
         * @brief 获取执行后返回的列数(字段数)
         * @return 数量
         */
        int columnCount();

        /**
         * @brief 获取执行后的字段int值
         * @param index 字段的索引值(从0开始)
         * @param defVal 默认值(选填)
         * @return 值
         */
        int getColumnInt(int index, int defVal = 0);

        /**
         * @brief 获取执行后的字段int64值
         * @param index 字段的索引值(从0开始)
         * @param defVal 默认值(选填)
         * @return 值
         */
        int64_t getColumnInt64(int index, int64_t defVal = 0);

        /**
         * @brief 获取执行后的字段double值
         * @param index 字段的索引值(从0开始)
         * @param defVal 默认值(选填)
         * @return 值
         */
        double getColumnDouble(int index, double defVal = 0.0);

        /**
         * @brief 获取执行后的字段string值
         * @param index 字段的索引值(从0开始)
         * @param defVal 默认值(选填)
         * @return 值
         */
        std::string getColumnString(int index, const std::string& defVal = "");

        /**
         * @brief 重置预编译指令(使复用)
         * @return true-成功, false-失败
         */
        bool reset();

        /**
         * @brief 预编译指令转为字符串
         * @return SQL字符串
         */
        std::string toString();

    private:
        sqlite3_stmt* m_stmt; /* SQL预编译指令 */
    };

public:
    /**
     * @brief 构造函数
     * @param path 数据库路径
     * @param password 数据库密码(选填), 为空表示没有密码
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
     * @brief 断开数据库
     */
    void disconnect();

    /**
     * @brief 创建预编译指令
     */
    std::shared_ptr<Stmt> createStmt(const std::string& sql);

    /**
     * @brief 执行sql语句
     * @param sql sql语句
     * @param callback 每一行的回调(选填), 参数: columns-列数据(key:字段名, value:字段值), 返回值: true-继续, false-停止
     *                 注意: 禁止在回调中执行其他接口(会造成循环锁)
     * @param errorMsg 错误消息(选填)
     * @return true-成功, false-失败
     */
    bool execSql(const std::string& sql,
                 const std::function<bool(const std::unordered_map<std::string, std::string>& columns)>& callback = nullptr,
                 std::string* errorMsg = nullptr);

    /**
     * @brief 开始事务
     * @param errorMsg 错误消息(选填)
     * @return true-成功, false-失败
     */
    bool beginTransaction(std::string* errorMsg = nullptr);

    /**
     * @brief 提交事务
     * @param errorMsg 错误消息(选填)
     * @return true-成功, false-失败
     */
    bool commitTransaction(std::string* errorMsg = nullptr);

    /**
     * @brief 回滚事务
     * @param errorMsg 错误消息(选填)
     * @return true-成功, false-失败
     */
    bool rollbackTransaction(std::string* errorMsg = nullptr);

    /**
     * @brief 获取PRAGMA值
     * @param key 字段名
     * @param errorMsg 错误消息(选填)
     * @return 值
     */
    std::string getPragma(const std::string& key, std::string* errorMsg = nullptr);

    /**
     * @brief 设置PRAGMA值
     * @param key 字段名, 例如: user_version(可以设置数据库版本), journal_mode, synchronous
     * @param value 值
     * @param errorMsg 错误消息(选填)
     * @return true-成功, false-失败
     */
    bool setPragma(const std::string& key, const std::string& value, std::string* errorMsg = nullptr);

    /**
     * @brief 获取最后一次插入成功的表的行号
     * @return 行号
     */
    int64_t getLastInsertRowId();

    /**
     * @brief 获取上一次的错误码
     * @return 错误码
     */
    int getLastErrorCode();

    /**
     * @brief 获取上一次的错误消息
     * @return 错误消息
     */
    std::string getLastErrorMsg();

    /**
     * @brief 删除表
     * @param tableName 表名
     * @param errorMsg 错误消息(选填)
     * @return true-成功, false-失败
     */
    bool dropTable(const std::string& tableName, std::string* errorMsg = nullptr);

    /**
     * @brief 重命名表
     * @param tableName 表名
     * @param newTableName 新表名
     * @param errorMsg 错误消息(选填)
     * @return true-成功, false-失败
     */
    bool renameTable(const std::string& tableName, const std::string& newTableName, std::string* errorMsg = nullptr);

    /**
     * @brief 检测表是否存在
     * @param tableName 表名
     * @param errorMsg 错误消息(选填)
     * @return true-存在, false-不存在
     */
    bool checkTableExist(const std::string& tableName, std::string* errorMsg = nullptr);

    /**
     * @brief 检测表中是否存在字段(列)
     * @param tableName 表名
     * @param columnName 字段(列)名
     * @param errorMsg 错误消息(选填)
     * @return true-存在, false-不存在
     */
    bool checkColumnExist(const std::string& tableName, const std::string& columnName, std::string* errorMsg = nullptr);

    /**
     * @brief 检测表中是否存在数据
     * @param tableName 表名
     * @param condition 条件(选填), 默认为空
     * @param errorMsg 错误消息(选填)
     * @return true-存在, false-不存在
     */
    bool checkDataExist(const std::string& tableName, const std::string& condition, std::string* errorMsg = nullptr);

    /**
     * @brief 插入/替换表数据
     * @param tableName 表名
     * @param values 值
     * @param replace 是否替换(选填), 默认false, 为true时若键值已存在则替换, 若键值不存在则插入
     * @param errorMsg 错误消息(选填)
     * @return true-成功, false-失败
     */
    bool insertInto(const std::string& tableName, const std::unordered_map<std::string, std::string>& values, bool replace = false,
                    std::string* errorMsg = nullptr);

    /**
     * @brief 删除表数据
     * @param tableName 表名
     * @param condition 条件
     * @param errorMsg 错误消息(选填)
     * @return true-成功, false-失败
     */
    bool deleteFrom(const std::string& tableName, const std::string& condition, std::string* errorMsg = nullptr);

    /**
     * @brief 更新表数据
     * @param tableName 表名
     * @param newValues 新值
     * @param condition 条件
     * @param errorMsg 错误消息(选填)
     * @return true-成功, false-失败
     */
    bool updateSet(const std::string& tableName, const std::unordered_map<std::string, std::string>& newValues,
                   const std::string& condition, std::string* errorMsg = nullptr);

private:
    /**
     * @brief 执行sql语句
     * @param db 数据库句柄
     * @param sql sql语句
     * @param callback 每一行的回调(选填), 参数: columns-列数据(key:字段名, value:字段值), 返回值: true-继续, false-停止
     * @param errorMsg 错误消息(选填)
     * @return -1.参数错误, 0.执行成功, >0.其他SQL错误(Result Codes) 
     */
    int execImpl(sqlite3* db, const std::string& sql,
                 const std::function<bool(const std::unordered_map<std::string, std::string>& columns)>& callback = nullptr,
                 std::string* errorMsg = nullptr);

private:
    std::mutex m_mutex; /* 互斥锁 */
    sqlite3* m_db; /* 数据库指针 */
    bool m_inTransaction; /* 是否在事务中 */
    std::string m_path; /* 数据库路径(全路径) */
    std::string m_password; /* 数据库密码 */
};
} // namespace database
