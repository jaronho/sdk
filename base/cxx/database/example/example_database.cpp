#include <iostream>

#include "../database/sqlite.h"

void testDb1()
{
    std::string name = "test1.db";
    database::Sqlite db(name, "123456");
    if (db.connect())
    {
        printf("===== connect database %s ok\n", name.c_str());
        /* 创建表 */
        bool ret;
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

int main()
{
    testDb1();
    return 0;
}
