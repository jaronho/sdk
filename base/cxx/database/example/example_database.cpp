#include <iostream>
#include <vector>

#include "../database/sqlite.h"
#include "../winq/abstract.h"

void testDb1()
{
    std::string name = "test1.db";
    database::Sqlite db(name, "123456");
    if (db.connect())
    {
        printf("===== connect database %s ok\n", name.c_str());
        bool ret;
        /* 删除表 */
        ret = db.dropTable("version");
        if (!ret)
        {
            printf("drop table 'version' fail\n");
        }
        /* 创建表 */
        ret = db.execSql("CREATE TABLE IF NOT EXISTS `version`(`key` TEXT PRIMARY KEY NOT NULL, `value` INTEGER NOT NULL)");
        if (!ret)
        {
            printf("create table 'version' fail\n");
            return;
        }
        ret = db.execSql("CREATE TABLE IF NOT EXISTS `department`"
                         "(`id` TEXT PRIMARY KEY NOT NULL,"
                         " `type` INTEGER NOT NULL,"
                         " `name` TEXT NOT NULL,"
                         " `member_count` INTEGER DEFAULT 0 NOT NULL)");
        if (!ret)
        {
            printf("create table 'department' fail\n");
            return;
        }
        /* 设置表数据 */
        {
            static const std::string sql = "INSERT INTO `version`(`key`,`value`) VALUES('major',3)";
            db.execSql(sql);
        }
        {
            static const std::string sql = "REPLACE INTO `version`(`key`,`value`) VALUES(?,?)";
            auto st = db.createStmt(sql);
            if (st)
            {
                st->bind(0, "minor");
                st->bind(1, 0);
                st->step();

                st->reset();
                st->bind(0, "revise");
                st->bind(1, 1);
                st->step();

                st->reset();
                st->bind(0, "build");
                st->bind(1, 813);
                st->step();
            }
        }
        {
            static const std::string sql = "INSERT INTO `department`(`id`,`type`,`name`,`member_count`) VALUES(?,?,?,?)";
            auto st = db.createStmt(sql);
            if (st)
            {
                st->bind(0, "10001");
                st->bind(1, 1);
                st->bind(2, "depart 1");
                st->bind(3, "10");
                st->step();

                st->reset();
                st->bind(0, "10002");
                st->bind(1, 2);
                st->bind(2, "depart 2");
                st->bind(3, "38");
                st->step();

                st->reset();
                st->bind(0, "10003");
                st->bind(1, 3);
                st->bind(2, "depart 3");
                st->bind(3, "14");
                st->step();

                st->reset();
                st->bind(0, "10004");
                st->bind(1, 2);
                st->bind(2, "depart 4");
                st->bind(3, "213");
                st->step();
            }
        }
        /* 查询表数据 */
        {
            printf("===== query version table\n");
            static const std::string sql = "SELECT * from `version`";
            db.execSql(sql, [&](const std::unordered_map<std::string, std::string>& columns) {
                printf("----------------------------------------\n");
                for (auto iter = columns.begin(); columns.end() != iter; ++iter)
                {
                    const auto& key = iter->first;
                    const auto& value = iter->second;
                    if ("build" == value)
                    {
                        return false;
                    }
                    printf("%s: %s\n", key.c_str(), value.c_str());
                }
                return true;
            });
        }
        {
            printf("===== query department table\n");
            static const std::string sql = "SELECT * from `department`";
            auto st = db.createStmt(sql);
            if (st)
            {
                while (st->step())
                {
                    printf("----------------------------------------\n");
                    auto id = st->getColumnString(0);
                    auto type = st->getColumnInt(1);
                    auto name = st->getColumnString(2);
                    auto memberCount = st->getColumnInt(3);
                    printf("          id: %s\n", id.c_str());
                    printf("        type: %d\n", type);
                    printf("        name: %s\n", name.c_str());
                    printf("member count: %d\n", memberCount);
                }
            }
        }
    }
    else
    {
        printf("===== connect database %s fail\n", name.c_str());
    }
}

const std::string g_dbName = "test2.db";
const std::string g_dbPwd = "123456";
const std::string g_tableName("people");
const WCDB::Column g_columnId("id");
const WCDB::Column g_columnName("name");
const WCDB::Column g_columnSex("sex");
const WCDB::Column g_columnAge("age");
const WCDB::Column g_columnCountry("country");
WCDB::ColumnResultList g_peopleResultList{{g_columnId}, {g_columnName}, {g_columnSex}, {g_columnAge}, {g_columnCountry}};

void testWinq()
{
    printf("========================= test winq =========================\n");
    database::Sqlite db(g_dbName, g_dbPwd);
    if (db.connect())
    {
        printf("===== connect database %s ok\n", g_dbName.c_str());
        /* 删除表 */
        {
            auto stmt = WCDB::StatementDropTable().drop(g_tableName);
            printf("SQL: %s\n", stmt.getDescription().c_str());
            bool ret;
            ret = db.execSql(stmt.getDescription());
            if (!ret)
            {
                printf("drop table '%s' fail\n", g_tableName.c_str());
                return;
            }
        }
        /* 创建表 */
        {
            auto stmt = WCDB::StatementCreateTable().create(
                g_tableName,
                std::list<WCDB::ColumnDef>{
                    WCDB::ColumnDef(g_columnId, WCDB::ColumnType::Integer32).makePrimary(WCDB::OrderTerm::NotSet, true),
                    WCDB::ColumnDef(g_columnName, WCDB::ColumnType::Text),
                    WCDB::ColumnDef(g_columnSex, WCDB::ColumnType::Integer32),
                    WCDB::ColumnDef(g_columnAge, WCDB::ColumnType::Integer32),
                    WCDB::ColumnDef(g_columnCountry, WCDB::ColumnType::Text),
                },
                true);
            printf("SQL: %s\n", stmt.getDescription().c_str());
            bool ret;
            ret = db.execSql(stmt.getDescription());
            if (!ret)
            {
                printf("create table '%s' fail\n", g_tableName.c_str());
                return;
            }
        }
        /* 添加表数据 */
        {
            std::vector<std::string> peopleNames{"Zhang San", "Li Si", "Qian Wu", "Wu Liu"};
            for (size_t i = 0; i < peopleNames.size(); ++i)
            {
                auto stmt = WCDB::StatementInsert()
                                .insert(g_tableName, WCDB::Conflict::Replace)
                                .values({nullptr, peopleNames[i], 1, 30 + i, "china"});
                printf("SQL: %s\n", stmt.getDescription().c_str());
                bool ret;
                ret = db.execSql(stmt.getDescription());
                if (!ret)
                {
                    printf("insert table '%s' fail\n", g_tableName.c_str());
                }
            }
        }
        /* 查询表数据 */
        {
            auto stmt = WCDB::StatementSelect().select(g_peopleResultList).from(g_tableName).where(WCDB::Expr(g_columnAge) == 31);
            printf("SQL: %s\n", stmt.getDescription().c_str());
            bool ret;
            ret = db.execSql(stmt.getDescription(), [&](const std::unordered_map<std::string, std::string>& columns) {
                printf("----------------------------------------\n");
                for (auto iter = columns.begin(); columns.end() != iter; ++iter)
                {
                    const auto& key = iter->first;
                    const auto& value = iter->second;
                    printf("%s: %s\n", key.c_str(), value.c_str());
                }
                return true;
            });
            if (!ret)
            {
                printf("select table '%s' fail\n", g_tableName.c_str());
            }
        }
        /* 更新表数据 */
        {
            auto stmt = WCDB::StatementUpdate()
                            .update(g_tableName)
                            .set(std::list<std::pair<WCDB::Column, WCDB::Expr>>{{g_columnAge, 36}})
                            .where(WCDB::Expr(g_columnAge) == 33);
            printf("SQL: %s\n", stmt.getDescription().c_str());
            bool ret;
            ret = db.execSql(stmt.getDescription());
            if (!ret)
            {
                printf("update table '%s' fail\n", g_tableName.c_str());
            }
        }
        /* 删除表数据 */
        {
            auto stmt = WCDB::StatementDelete().deleteFrom(g_tableName).where(WCDB::Expr(g_columnAge) == 32);
            printf("SQL: %s\n", stmt.getDescription().c_str());
            bool ret;
            ret = db.execSql(stmt.getDescription());
            if (!ret)
            {
                printf("delete table '%s' fail\n", g_tableName.c_str());
            }
        }
        /* 查询表数据 */
        {
            auto stmt = WCDB::StatementSelect().select(g_peopleResultList).from(g_tableName);
            printf("SQL: %s\n", stmt.getDescription().c_str());
            bool ret;
            ret = db.execSql(stmt.getDescription(), [&](const std::unordered_map<std::string, std::string>& columns) {
                printf("----------------------------------------\n");
                for (auto iter = columns.begin(); columns.end() != iter; ++iter)
                {
                    const auto& key = iter->first;
                    const auto& value = iter->second;
                    printf("%s: %s\n", key.c_str(), value.c_str());
                }
                return true;
            });
            if (!ret)
            {
                printf("select table '%s' fail\n", g_tableName.c_str());
            }
        }
    }
    else
    {
        printf("===== connect database %s fail\n", g_dbName.c_str());
    }
}

int main()
{
    testDb1();
    printf("\n\n\n\n\n");
    testWinq();
    return 0;
}
