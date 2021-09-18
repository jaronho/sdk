#include "sqlite.h"

namespace database
{
Sqlite::Sqlite(const std::string& path, const std::string& password) : m_db(nullptr), m_path(path), m_password(password) {}

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
    if (!m_password.empty())
    {
        ret = sqlite3_rekey_v2(m_db, "main", m_password.c_str(), (int)m_password.size());
        if (SQLITE_OK != ret)
        {
            sqlite3_close_v2(m_db);
            m_db = nullptr;
            return false;
        }
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

int64_t Sqlite::execSql(const std::string& sql,
                        const std::function<bool(const std::unordered_map<std::string, std::string>& columns)>& callback,
                        std::string* errorMsg)
{
    if (errorMsg)
    {
        (*errorMsg).clear();
    }
    if (sql.empty())
    {
        return -1;
    }
    std::lock_guard<std::mutex> locker(m_mutex);
    if (!m_db)
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
            auto& callbackRef = *(std::function<bool(const std::unordered_map<std::string, std::string>&)>*)data;
            if (!callbackRef)
            {
                return 0;
            }
            std::unordered_map<std::string, std::string> columns; /* 每一行的列数据, key: 字段名, value: 字段值 */
            for (int i = 0; i < colCount; ++i)
            {
                columns[header[i]] = values[i];
            }
            if (callbackRef(columns))
            {
                return 0;
            }
            return 1;
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
    if (SQLITE_OK != ret)
    {
        return -1;
    }
    return sqlite3_last_insert_rowid(m_db);
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
} // namespace database
