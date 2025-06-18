#include "sqlite.h"

namespace database
{
Sqlite::Stmt::Stmt(std::shared_ptr<std::recursive_mutex> mutex, sqlite3* db, const std::string& sql) : m_stmt(nullptr)
{
    if (!mutex)
    {
        throw std::logic_error(std::string("[") + __FILE__ + " " + std::to_string(__LINE__) + " " + __FUNCTION__ + "] arg 'mutex' is null");
    }
    mutex->lock();
    m_mutex = mutex;
    prepare(db, sql);
}

Sqlite::Stmt::~Stmt()
{
    if (m_stmt)
    {
        sqlite3_finalize(m_stmt);
    }
    if (m_mutex)
    {
        m_mutex->unlock();
    }
}

bool Sqlite::Stmt::prepare(sqlite3* db, const std::string& sql)
{
    if (m_stmt)
    {
        sqlite3_finalize(m_stmt);
        m_stmt = nullptr;
    }
    if (db && !sql.empty())
    {
        auto ret = sqlite3_prepare_v2(db, sql.c_str(), -1, &m_stmt, nullptr);
        if (SQLITE_OK == ret)
        {
            return true;
        }
    }
    return false;
}

bool Sqlite::Stmt::bind(int index, int val)
{
    if (m_stmt && index >= 0)
    {
        index += 1; /* 指定外部传入的index从0开始, 这里自动+1 */
        auto ret = sqlite3_bind_int(m_stmt, index, val);
        if (SQLITE_OK == ret)
        {
            return true;
        }
    }
    return false;
}

bool Sqlite::Stmt::bind(int index, int64_t val)
{
    if (m_stmt && index >= 0)
    {
        index += 1; /* 指定外部传入的index从0开始, 这里自动+1 */
        auto ret = sqlite3_bind_int64(m_stmt, index, val);
        if (SQLITE_OK == ret)
        {
            return true;
        }
    }
    return false;
}

bool Sqlite::Stmt::bind(int index, double val)
{
    if (m_stmt && index >= 0)
    {
        index += 1; /* 指定外部传入的index从0开始, 这里自动+1 */
        auto ret = sqlite3_bind_double(m_stmt, index, val);
        if (SQLITE_OK == ret)
        {
            return true;
        }
    }
    return false;
}

bool Sqlite::Stmt::bind(int index, const std::string& val)
{
    if (m_stmt && index >= 0)
    {
        index += 1; /* 指定外部传入的index从0开始, 这里自动+1 */
        auto ret = sqlite3_bind_text(m_stmt, index, val.c_str(), -1, SQLITE_TRANSIENT);
        if (SQLITE_OK == ret)
        {
            return true;
        }
    }
    return false;
}

int Sqlite::Stmt::step()
{
    if (m_stmt)
    {
        while (true)
        {
            auto ret = sqlite3_step(m_stmt);
            if (SQLITE_BUSY == ret) /* BUSY则重试 */
            {
                continue;
            }
            else if (SQLITE_ROW == ret) /* 继续 */
            {
                return 1;
            }
            else if (SQLITE_DONE == ret) /* 结束 */
            {
                return 0;
            }
            /* 其余情况都是异常 */
            break;
        }
    }
    return -1;
}

int Sqlite::Stmt::columnCount()
{
    if (m_stmt)
    {
        return sqlite3_column_count(m_stmt);
    }
    return 0;
}

int Sqlite::Stmt::getColumnInt(int index, int defVal)
{
    if (m_stmt && index >= 0)
    {
        return sqlite3_column_int(m_stmt, index);
    }
    return defVal;
}

int64_t Sqlite::Stmt::getColumnInt64(int index, int64_t defVal)
{
    if (m_stmt && index >= 0)
    {
        return sqlite3_column_int64(m_stmt, index);
    }
    return defVal;
}

double Sqlite::Stmt::getColumnDouble(int index, double defVal)
{
    if (m_stmt && index >= 0)
    {
        return sqlite3_column_double(m_stmt, index);
    }
    return defVal;
}

std::string Sqlite::Stmt::getColumnString(int index, const std::string& defVal)
{
    if (m_stmt && index >= 0)
    {
        auto szVal = reinterpret_cast<const char*>(sqlite3_column_text(m_stmt, index));
        if (szVal)
        {
            return szVal;
        }
    }
    return defVal;
}

bool Sqlite::Stmt::reset()
{
    if (m_stmt)
    {
        auto ret = sqlite3_reset(m_stmt);
        if (SQLITE_OK == ret)
        {
            return true;
        }
    }
    return false;
}

std::string Sqlite::Stmt::toString()
{
    std::string sql;
    if (m_stmt)
    {
        char* szSql = sqlite3_expanded_sql(m_stmt);
        sql = szSql;
        sqlite3_free(szSql);
    }
    return sql;
}

void Sqlite::ValueMap::insert(const std::string& key, const std::string& value)
{
    auto iter = m_values.find(key);
    if (m_values.end() == iter)
    {
        m_values.insert(std::make_pair(key, value));
    }
    else
    {
        iter->second = value;
    }
}

void Sqlite::ValueMap::insert(const std::string& key, int value)
{
    auto iter = m_values.find(key);
    if (m_values.end() == iter)
    {
        m_values.insert(std::make_pair(key, std::to_string(value)));
    }
    else
    {
        iter->second = std::to_string(value);
    }
}

void Sqlite::ValueMap::insert(const std::string& key, unsigned int value)
{
    auto iter = m_values.find(key);
    if (m_values.end() == iter)
    {
        m_values.insert(std::make_pair(key, std::to_string(value)));
    }
    else
    {
        iter->second = std::to_string(value);
    }
    m_values[key] = std::to_string(value);
}

void Sqlite::ValueMap::insert(const std::string& key, long value)
{
    auto iter = m_values.find(key);
    if (m_values.end() == iter)
    {
        m_values.insert(std::make_pair(key, std::to_string(value)));
    }
    else
    {
        iter->second = std::to_string(value);
    }
}

void Sqlite::ValueMap::insert(const std::string& key, unsigned long value)
{
    auto iter = m_values.find(key);
    if (m_values.end() == iter)
    {
        m_values.insert(std::make_pair(key, std::to_string(value)));
    }
    else
    {
        iter->second = std::to_string(value);
    }
}

void Sqlite::ValueMap::insert(const std::string& key, long long value)
{
    auto iter = m_values.find(key);
    if (m_values.end() == iter)
    {
        m_values.insert(std::make_pair(key, std::to_string(value)));
    }
    else
    {
        iter->second = std::to_string(value);
    }
}

void Sqlite::ValueMap::insert(const std::string& key, unsigned long long value)
{
    auto iter = m_values.find(key);
    if (m_values.end() == iter)
    {
        m_values.insert(std::make_pair(key, std::to_string(value)));
    }
    else
    {
        iter->second = std::to_string(value);
    }
}

void Sqlite::ValueMap::insert(const std::string& key, float value)
{
    auto iter = m_values.find(key);
    if (m_values.end() == iter)
    {
        m_values.insert(std::make_pair(key, std::to_string(value)));
    }
    else
    {
        iter->second = std::to_string(value);
    }
}

void Sqlite::ValueMap::insert(const std::string& key, double value)
{
    auto iter = m_values.find(key);
    if (m_values.end() == iter)
    {
        m_values.insert(std::make_pair(key, std::to_string(value)));
    }
    else
    {
        iter->second = std::to_string(value);
    }
}

Sqlite::Sqlite(const std::string& path, const std::string& password)
    : m_db(nullptr), m_inTransaction(false), m_path(path), m_password(password)
{
    m_mutex = std::make_shared<std::recursive_mutex>();
}

Sqlite::~Sqlite()
{
    disconnect();
}

std::string Sqlite::getPath()
{
    std::lock_guard<std::recursive_mutex> locker(*m_mutex);
    return m_path;
}

bool Sqlite::connect(bool readOnly, std::string* errorMsg)
{
    std::lock_guard<std::recursive_mutex> locker(*m_mutex);
    if (m_db)
    {
        if (errorMsg)
        {
            (*errorMsg) = "connection is not disconnected";
        }
        return true;
    }
    int sqliteFlag = 0;
    if (readOnly)
    {
        sqliteFlag = SQLITE_OPEN_READONLY;
    }
    else
    {
        sqliteFlag = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX;
    }
    auto ret = sqlite3_open_v2(m_path.c_str(), &m_db, sqliteFlag, nullptr);
    if (SQLITE_OK != ret)
    {
        if (errorMsg)
        {
            (*errorMsg) = sqlite3_errstr(ret);
        }
        m_db = nullptr;
        return false;
    }
    if (!m_password.empty()) /* 密码非空 */
    {
#ifdef SQLITE_HAS_CODEC
        sqlite3_key(m_db, m_password.c_str(), m_password.size()); /* 设置密钥 */
        ret = sqlite3_exec(m_db, "SELECT count(*) FROM sqlite_master", NULL, NULL, NULL); /* 如果返回非0表示解密失败 */
        if (SQLITE_OK != ret)
        {
            if (errorMsg)
            {
                (*errorMsg) = sqlite3_errstr(ret);
            }
            sqlite3_close_v2(m_db);
            m_db = nullptr;
            return false;
        }
#endif
    }
    return true;
}

void Sqlite::disconnect()
{
    std::lock_guard<std::recursive_mutex> locker(*m_mutex);
    if (m_db)
    {
        sqlite3_close_v2(m_db);
        m_db = nullptr;
    }
}

bool Sqlite::isConnected()
{
    std::lock_guard<std::recursive_mutex> locker(*m_mutex);
    if (m_db)
    {
        return true;
    }
    return false;
}

std::shared_ptr<Sqlite::Stmt> Sqlite::createStmt(const std::string& sql)
{
    if (sql.empty())
    {
        return nullptr;
    }
    std::lock_guard<std::recursive_mutex> locker(*m_mutex);
    if (!m_db)
    {
        return nullptr;
    }
    return std::make_shared<Stmt>(m_mutex, m_db, sql);
}

bool Sqlite::execSql(const std::string& sql,
                     const std::function<bool(const std::unordered_map<std::string, std::string>& columns)>& callback,
                     std::string* errorMsg)
{
    std::lock_guard<std::recursive_mutex> locker(*m_mutex);
    if (SQLITE_OK == execImpl(m_db, sql, callback, errorMsg))
    {
        return true;
    }
    return false;
}

bool Sqlite::beginTransaction(std::string* errorMsg)
{
    std::lock_guard<std::recursive_mutex> locker(*m_mutex);
    if (m_inTransaction)
    {
        if (errorMsg)
        {
            (*errorMsg) = "transaction has not been completed";
        }
        return false;
    }
    if (SQLITE_OK == execImpl(m_db, "BEGIN TRANSACTION", nullptr, errorMsg))
    {
        m_inTransaction = true;
        return true;
    }
    return false;
}

bool Sqlite::commitTransaction(std::string* errorMsg)
{
    std::lock_guard<std::recursive_mutex> locker(*m_mutex);
    if (!m_inTransaction)
    {
        if (errorMsg)
        {
            (*errorMsg) = "transaction has not been completed";
        }
        return false;
    }
    if (SQLITE_OK == execImpl(m_db, "COMMIT", nullptr, errorMsg))
    {
        m_inTransaction = false;
        return true;
    }
    return false;
}

bool Sqlite::rollbackTransaction(std::string* errorMsg)
{
    std::lock_guard<std::recursive_mutex> locker(*m_mutex);
    if (SQLITE_OK == execImpl(m_db, "ROLLBACK", nullptr, errorMsg))
    {
        return true;
    }
    return false;
}

std::string Sqlite::getPragma(const std::string& key, std::string* errorMsg)
{
    std::string value;
    if (key.empty())
    {
        if (errorMsg)
        {
            (*errorMsg) = "key empty";
        }
        return value;
    }
    std::lock_guard<std::recursive_mutex> locker(*m_mutex);
    char* szSql = sqlite3_mprintf("PRAGMA %q", key.c_str());
    execImpl(
        m_db, szSql,
        [&](const std::unordered_map<std::string, std::string>& columns) {
            if (columns.empty())
            {
                return true;
            }
            value = columns.begin()->second;
            return false;
        },
        errorMsg);
    sqlite3_free(szSql);
    return value;
}

bool Sqlite::setPragma(const std::string& key, const std::string& value, std::string* errorMsg)
{
    if (key.empty())
    {
        if (errorMsg)
        {
            (*errorMsg) = "key empty";
        }
        return false;
    }
    std::lock_guard<std::recursive_mutex> locker(*m_mutex);
    char* szSql = sqlite3_mprintf("PRAGMA %q = '%q'", key.c_str(), value.c_str());
    auto ret = execImpl(m_db, szSql, nullptr, errorMsg);
    sqlite3_free(szSql);
    return (SQLITE_OK == ret);
}

int64_t Sqlite::getLastInsertRowId()
{
    std::lock_guard<std::recursive_mutex> locker(*m_mutex);
    if (!m_db)
    {
        return -1;
    }
    return sqlite3_last_insert_rowid(m_db);
}

int Sqlite::getLastErrorCode()
{
    std::lock_guard<std::recursive_mutex> locker(*m_mutex);
    if (!m_db)
    {
        return 0;
    }
    return sqlite3_errcode(m_db);
}

std::string Sqlite::getLastErrorMsg()
{
    std::string msg;
    std::lock_guard<std::recursive_mutex> locker(*m_mutex);
    if (!m_db)
    {
        return msg;
    }
    const char* str = sqlite3_errmsg(m_db);
    if (str)
    {
        msg = str;
    }
    return msg;
}

bool Sqlite::clearTable(const std::string& tableName, std::string* errorMsg, std::string* sqlstr)
{
    if (sqlstr)
    {
        sqlstr->clear();
    }
    if (tableName.empty())
    {
        if (errorMsg)
        {
            (*errorMsg) = "parameter error";
        }
        return false;
    }
    auto sql = "DELETE FROM " + tableName;
    if (sqlstr)
    {
        *sqlstr = sql;
    }
    return execSql(sql, nullptr, errorMsg);
}

bool Sqlite::dropTable(const std::string& tableName, std::string* errorMsg, std::string* sqlstr)
{
    if (sqlstr)
    {
        sqlstr->clear();
    }
    if (tableName.empty())
    {
        if (errorMsg)
        {
            (*errorMsg) = "parameter error";
        }
        return false;
    }
    auto sql = "DROP TABLE IF EXISTS " + tableName;
    if (sqlstr)
    {
        *sqlstr = sql;
    }
    return execSql(sql, nullptr, errorMsg);
}

bool Sqlite::renameTable(const std::string& tableName, const std::string& newTableName, std::string* errorMsg, std::string* sqlstr)
{
    if (sqlstr)
    {
        sqlstr->clear();
    }
    if (tableName.empty() || newTableName.empty())
    {
        if (errorMsg)
        {
            (*errorMsg) = "parameter error";
        }
        return false;
    }
    auto sql = "ALTER TABLE " + tableName + " RENAME TO " + newTableName;
    if (sqlstr)
    {
        *sqlstr = sql;
    }
    return execSql(sql, nullptr, errorMsg);
}

bool Sqlite::checkTableExist(const std::string& tableName, std::string* errorMsg, std::string* sqlstr)
{
    if (sqlstr)
    {
        sqlstr->clear();
    }
    if (tableName.empty())
    {
        if (errorMsg)
        {
            (*errorMsg) = "parameter error";
        }
        return false;
    }
    bool foundFlag = false;
    auto sql = "SELECT count(*) FROM sqlite_master WHERE type='table' AND name='" + tableName + "'";
    if (sqlstr)
    {
        *sqlstr = sql;
    }
    execSql(
        sql,
        [&](const std::unordered_map<std::string, std::string>& columns) {
            for (auto iter = columns.begin(); columns.end() != iter; ++iter)
            {
                try
                {
                    if (std::atoi(iter->second.c_str()) > 0) /* 找到表 */
                    {
                        foundFlag = true;
                        return false;
                    }
                }
                catch (...)
                {
                }
            }
            return true;
        },
        errorMsg);
    return foundFlag;
}

bool Sqlite::checkColumnExist(const std::string& tableName, const std::string& columnName, std::string* errorMsg, std::string* sqlstr)
{
    if (sqlstr)
    {
        sqlstr->clear();
    }
    if (tableName.empty() || columnName.empty())
    {
        if (errorMsg)
        {
            (*errorMsg) = "parameter error";
        }
        return false;
    }
    bool foundFlag = false;
    auto sql = "PRAGMA table_info(" + tableName + ")";
    if (sqlstr)
    {
        *sqlstr = sql;
    }
    execSql(
        sql,
        [&](const std::unordered_map<std::string, std::string>& columns) {
            for (auto iter = columns.begin(); columns.end() != iter; ++iter)
            {
                if (0 == iter->first.compare("name") && 0 == iter->second.compare(columnName)) /* 找到字段 */
                {
                    foundFlag = true;
                    return false;
                }
            }
            return true;
        },
        errorMsg);
    return foundFlag;
}

long long Sqlite::queryDataCount(const std::string& tableName, const std::string& condition, std::string* errorMsg, std::string* sqlstr)
{
    if (sqlstr)
    {
        sqlstr->clear();
    }
    if (tableName.empty())
    {
        if (errorMsg)
        {
            (*errorMsg) = "parameter error";
        }
        return 0;
    }
    long long dataCount = 0;
    auto sql = "SELECT count(*) FROM " + tableName + (condition.empty() ? "" : " WHERE " + condition);
    if (sqlstr)
    {
        *sqlstr = sql;
    }
    execSql(
        sql,
        [&](const std::unordered_map<std::string, std::string>& columns) {
            for (auto iter = columns.begin(); columns.end() != iter; ++iter)
            {
                try
                {
                    auto value = std::atoll(iter->second.c_str());
                    if (value > 0) /* 找到数据 */
                    {
                        dataCount = value;
                        return false;
                    }
                }
                catch (...)
                {
                }
            }
            return true;
        },
        errorMsg);
    return dataCount;
}

bool Sqlite::insertInto(const std::string& tableName, const std::unordered_map<std::string, std::string>& values, bool replace,
                        std::string* errorMsg, std::string* sqlstr)
{
    if (sqlstr)
    {
        sqlstr->clear();
    }
    if (tableName.empty() || values.empty())
    {
        if (errorMsg)
        {
            (*errorMsg) = "parameter error";
        }
        return false;
    }
    /* 构建列名和占位符 */
    std::string columns, placeholders;
    std::vector<std::string> paramValues;
    for (auto iter = values.begin(); iter != values.end(); ++iter)
    {
        if (iter->first.empty())
        {
            if (errorMsg)
            {
                (*errorMsg) = "value error";
            }
            return false;
        }
        if (!columns.empty())
        {
            columns += ",";
            placeholders += ",";
        }
        columns += iter->first;
        placeholders += "?";
        paramValues.push_back(iter->second);
    }
    /* 构建SQL语句 */
    std::string sql = std::string(replace ? "REPLACE" : "INSERT") + " INTO " + tableName + "(" + columns + ") VALUES(" + placeholders + ")";
    if (sqlstr)
    {
        *sqlstr = sql;
    }
    /* 准备SQL语句 */
    auto stmt = createStmt(sql);
    if (!stmt)
    {
        if (errorMsg)
        {
            (*errorMsg) = "create statement failed";
        }
        return false;
    }
    /* 绑定参数 */
    for (size_t i = 0; i < paramValues.size(); ++i)
    {
        if (!stmt->bind(static_cast<int>(i), paramValues[i]))
        {
            if (errorMsg)
            {
                (*errorMsg) = "bind parameter failed";
            }
            return false;
        }
    }
    /* 执行SQL语句 */
    int result = stmt->step();
    if (0 != result && 1 != result)
    {
        if (errorMsg)
        {
            (*errorMsg) = "execute statement failed: " + getLastErrorMsg();
        }
        return false;
    }
    return true;
}

bool Sqlite::insertInto(const std::string& tableName, const ValueMap& values, bool replace, std::string* errorMsg, std::string* sqlstr)
{
    return insertInto(tableName, values.m_values, replace, errorMsg, sqlstr);
}

bool Sqlite::deleteFrom(const std::string& tableName, const std::string& condition, std::string* errorMsg, std::string* sqlstr)
{
    if (sqlstr)
    {
        sqlstr->clear();
    }
    if (tableName.empty() || condition.empty())
    {
        if (errorMsg)
        {
            (*errorMsg) = "parameter error";
        }
        return false;
    }
    auto sql = "DELETE FROM " + tableName + " WHERE " + condition;
    if (sqlstr)
    {
        *sqlstr = sql;
    }
    return execSql(sql, nullptr, errorMsg);
}

bool Sqlite::updateSet(const std::string& tableName, const std::unordered_map<std::string, std::string>& newValues,
                       const std::string& condition, std::string* errorMsg, std::string* sqlstr)
{
    if (sqlstr)
    {
        sqlstr->clear();
    }
    if (tableName.empty() || newValues.empty() || condition.empty())
    {
        if (errorMsg)
        {
            (*errorMsg) = "parameter error";
        }
        return false;
    }
    /* 构建列名和占位符 */
    std::string newValueSql;
    std::vector<std::string> paramValues;
    for (auto iter = newValues.begin(); iter != newValues.end(); ++iter)
    {
        if (iter->first.empty())
        {
            if (errorMsg)
            {
                (*errorMsg) = "value error";
            }
            return false;
        }
        if (newValues.begin() != iter)
        {
            newValueSql += ",";
        }
        newValueSql += iter->first + " = ?"; /* 使用占位符 */
        paramValues.push_back(iter->second);
    }
    /* 构建SQL语句 */
    auto sql = "UPDATE " + tableName + " SET " + newValueSql + " WHERE " + condition;
    if (sqlstr)
    {
        *sqlstr = sql;
    }
    /* 准备SQL语句 */
    auto stmt = createStmt(sql);
    if (!stmt)
    {
        if (errorMsg)
        {
            (*errorMsg) = "create statement failed";
        }
        return false;
    }
    /* 绑定参数 */
    for (size_t i = 0; i < paramValues.size(); ++i)
    {
        if (!stmt->bind(static_cast<int>(i), paramValues[i]))
        {
            if (errorMsg)
            {
                (*errorMsg) = "bind parameter failed";
            }
            return false;
        }
    }
    /* 执行SQL语句 */
    int result = stmt->step();
    if (0 != result && 1 != result)
    {
        if (errorMsg)
        {
            (*errorMsg) = "execute statement failed: " + getLastErrorMsg();
        }
        return false;
    }
    return true;
}

bool Sqlite::updateSet(const std::string& tableName, const ValueMap& newValues, const std::string& condition, std::string* errorMsg,
                       std::string* sqlstr)
{
    return updateSet(tableName, newValues.m_values, condition, errorMsg, sqlstr);
}

int Sqlite::execImpl(sqlite3* db, const std::string& sql,
                     const std::function<bool(const std::unordered_map<std::string, std::string>& columns)>& callback,
                     std::string* errorMsg)
{
    if (errorMsg)
    {
        errorMsg->clear();
    }
    if (!db || sql.empty())
    {
        return -1;
    }
    char* szErrorMsg = nullptr;
    auto ret = sqlite3_exec(
        m_db, sql.c_str(),
        [](void* data, int colCount, char** values, char** header) {
            if (!data)
            {
                return 0;
            }
            auto& cb = *(std::function<bool(const std::unordered_map<std::string, std::string>&)>*)data;
            if (!cb)
            {
                return 0;
            }
            std::unordered_map<std::string, std::string> columns; /* 每一行的列数据, key: 字段名, value: 字段值 */
            for (int i = 0; i < colCount; ++i)
            {
                columns[header[i]] = (values[i] ? values[i] : "");
            }
            if (cb(columns))
            {
                return 0;
            }
            return 1; /* 停止 */
        },
        (void*)&callback, &szErrorMsg);
    if (szErrorMsg)
    {
        if (errorMsg)
        {
            *errorMsg = szErrorMsg;
        }
        sqlite3_free(szErrorMsg);
    }
    return ret;
}
} // namespace database
