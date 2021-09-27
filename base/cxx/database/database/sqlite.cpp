#include "sqlite.h"

namespace database
{
Sqlite::Stmt::Stmt(sqlite3* db, const std::string& sql) : m_stmt(nullptr)
{
    prepare(db, sql);
}

Sqlite::Stmt::~Stmt()
{
    if (m_stmt)
    {
        sqlite3_finalize(m_stmt);
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
        int ret = sqlite3_prepare_v2(db, sql.c_str(), -1, &m_stmt, nullptr);
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
        int ret = sqlite3_bind_int(m_stmt, index, val);
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
        int ret = sqlite3_bind_int64(m_stmt, index, val);
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
        int ret = sqlite3_bind_double(m_stmt, index, val);
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
        int ret = sqlite3_bind_text(m_stmt, index, val.c_str(), -1, SQLITE_TRANSIENT);
        if (SQLITE_OK == ret)
        {
            return true;
        }
    }
    return false;
}

bool Sqlite::Stmt::step()
{
    if (m_stmt)
    {
        while (true)
        {
            int ret = sqlite3_step(m_stmt);
            if (SQLITE_BUSY == ret) /* BUSY则重试 */
            {
                continue;
            }
            else if (SQLITE_ROW == ret || SQLITE_DONE == ret) /* 正常返回 */
            {
                return true;
            }
            /* 其余情况都是异常 */
            break;
        }
    }
    return false;
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
        int ret = sqlite3_reset(m_stmt);
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

Sqlite::Sqlite(const std::string& path, const std::string& curPassword, const std::string newPassword)
    : m_db(nullptr), m_inTransaction(false), m_path(path), m_curPassword(curPassword), m_newPassword(newPassword)
{
}

Sqlite::~Sqlite()
{
    disconnect();
}

std::string Sqlite::getPath()
{
    std::lock_guard<std::mutex> locker(m_mutex);
    return m_path;
}

bool Sqlite::connect(bool readOnly)
{
    std::lock_guard<std::mutex> locker(m_mutex);
    if (m_db)
    {
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
    int ret = sqlite3_open_v2(m_path.c_str(), &m_db, sqliteFlag, nullptr);
    if (SQLITE_OK != ret)
    {
        m_db = nullptr;
        return false;
    }
#ifdef SQLITE_HAS_CODEC
    if (m_curPassword.empty()) /* 数据库未加密 */
    {
        if (!m_newPassword.empty())
        {
            char pwd[] = "123456\0";
            ret = sqlite3_key(m_db, pwd, strlen(pwd)); /* 设置密钥 */
        }
    }
    else
    {
        ret = sqlite3_key(m_db, m_curPassword.c_str(), (int)m_curPassword.size()); /* 使用密钥打开 */
        if (SQLITE_OK == ret) /* 密钥打开成功 */
        {
            if (!m_newPassword.empty() && m_newPassword != m_curPassword)
            {
                ret = sqlite3_rekey(m_db, m_newPassword.c_str(), (int)m_newPassword.size()); /* 变更密钥 */
            }
        }
    }
    if (SQLITE_OK != ret)
    {
        sqlite3_close_v2(m_db);
        m_db = nullptr;
        return false;
    }
#endif
    return true;
}

void Sqlite::disconnect()
{
    std::lock_guard<std::mutex> locker(m_mutex);
    if (m_db)
    {
        sqlite3_close_v2(m_db);
        m_db = nullptr;
    }
}

std::shared_ptr<Sqlite::Stmt> Sqlite::createStmt(const std::string& sql)
{
    if (sql.empty())
    {
        return nullptr;
    }
    std::lock_guard<std::mutex> locker(m_mutex);
    if (!m_db)
    {
        return nullptr;
    }
    return std::make_shared<Stmt>(m_db, sql);
}

bool Sqlite::execSql(const std::string& sql,
                     const std::function<bool(const std::unordered_map<std::string, std::string>& columns)>& callback,
                     std::string* errorMsg)
{
    std::lock_guard<std::mutex> locker(m_mutex);
    if (0 == execImpl(m_db, sql, callback, errorMsg))
    {
        return true;
    }
    return false;
}

bool Sqlite::beginTransaction(std::string* errorMsg)
{
    std::lock_guard<std::mutex> locker(m_mutex);
    if (m_inTransaction)
    {
        return false;
    }
    if (0 == execImpl(m_db, "BEGIN TRANSACTION", nullptr, errorMsg))
    {
        m_inTransaction = true;
        return true;
    }
    return false;
}

bool Sqlite::commitTransaction(std::string* errorMsg)
{
    std::lock_guard<std::mutex> locker(m_mutex);
    if (!m_inTransaction)
    {
        return false;
    }
    if (0 == execImpl(m_db, "COMMIT", nullptr, errorMsg))
    {
        m_inTransaction = false;
        return true;
    }
    return false;
}

bool Sqlite::rollbackTransaction(std::string* errorMsg)
{
    std::lock_guard<std::mutex> locker(m_mutex);
    if (0 == execImpl(m_db, "ROLLBACK", nullptr, errorMsg))
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
        return value;
    }
    std::lock_guard<std::mutex> locker(m_mutex);
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
        return false;
    }
    std::lock_guard<std::mutex> locker(m_mutex);
    char* szSql = sqlite3_mprintf("PRAGMA %q = '%q'", key.c_str(), value.c_str());
    int ret = execImpl(m_db, szSql, nullptr, errorMsg);
    sqlite3_free(szSql);
    return (0 == ret);
}

int64_t Sqlite::getLastInsertRowId()
{
    std::lock_guard<std::mutex> locker(m_mutex);
    if (!m_db)
    {
        return -1;
    }
    return sqlite3_last_insert_rowid(m_db);
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
    int ret = sqlite3_exec(
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
                columns[header[i]] = values[i];
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
