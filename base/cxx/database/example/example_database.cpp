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
        ret = db.execSql("CREATE TABLE IF NOT EXISTS version(key TEXT PRIMARY KEY NOT NULL, value INTEGER NOT NULL)");
        if (!ret)
        {
            printf("create table 'version' fail\n");
            return;
        }
        ret = db.execSql("CREATE TABLE IF NOT EXISTS department("
                         "id TEXT PRIMARY KEY NOT NULL,"
                         "type INTEGER NOT NULL,"
                         "name TEXT NOT NULL,"
                         "member_count INTEGER DEFAULT 0 NOT NULL)");
        if (!ret)
        {
            printf("create table 'department' fail\n");
            return;
        }
        /* 设置表数据 */
        {
            static const std::string sql = "INSERT INTO version(key,value) VALUES('major',3)";
            db.execSql(sql);
        }
        {
            static const std::string sql = "REPLACE INTO version(key,value) VALUES(?,?)";
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
            static const std::string sql = "INSERT INTO department(id,type,name,member_count) VALUES(?,?,?,?)";
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
            static const std::string sql = "SELECT * FROM version";
            db.execSql(sql, [&](const std::unordered_map<std::string, std::string>& columns) {
                printf("    ----------------------------------------\n");
                for (auto iter = columns.begin(); columns.end() != iter; ++iter)
                {
                    const auto& key = iter->first;
                    const auto& value = iter->second;
                    if ("build" == value)
                    {
                        return false;
                    }
                    printf("    %s: %s\n", key.c_str(), value.c_str());
                }
                return true;
            });
        }
        {
            printf("===== query department table\n");
            static const std::string sql = "SELECT * FROM department";
            auto st = db.createStmt(sql);
            if (st)
            {
                while (st->step() > 0)
                {
                    printf("    ----------------------------------------\n");
                    auto id = st->getColumnString(0);
                    auto type = st->getColumnInt(1);
                    auto name = st->getColumnString(2);
                    auto memberCount = st->getColumnInt(3);
                    printf("              id: %s\n", id.c_str());
                    printf("            type: %d\n", type);
                    printf("            name: %s\n", name.c_str());
                    printf("    member count: %d\n", memberCount);
                }
            }
        }
    }
    else
    {
        printf("===== connect database %s fail\n", name.c_str());
    }
}

const std::string g_dbName = "winq.db";
const std::string g_dbPwd = "123456";

/* 学生表 */
namespace student_table
{
const std::string tbname = "student"; /* 表名 */
const WCDB::Column column_id("id"); /* 唯一ID */
const WCDB::Column column_name("name"); /* 姓名 */
const WCDB::Column column_sex("sex"); /* 性别 */
const WCDB::Column column_age("age"); /* 年龄 */
const WCDB::Column column_country("country"); /* 国家 */
const WCDB::Column column_class_id("class_id"); /* 班级ID */

/* 删除表 */
std::string dropSql()
{
    return WCDB::StatementDropTable().drop(tbname).getDescription();
}

/* 创建表 */
std::string createSql()
{
    return WCDB::StatementCreateTable()
        .create(tbname,
                std::list<WCDB::ColumnDef>{
                    WCDB::ColumnDef(column_id, WCDB::ColumnType::Integer64)
                        .makePrimary(WCDB::OrderTerm::NotSet, true)
                        .makeUnique()
                        .makeNotNull()
                        .makeDefault(1),
                    WCDB::ColumnDef(column_name, WCDB::ColumnType::Text),
                    WCDB::ColumnDef(column_sex, WCDB::ColumnType::Integer32),
                    WCDB::ColumnDef(column_age, WCDB::ColumnType::Integer32),
                    WCDB::ColumnDef(column_country, WCDB::ColumnType::Text),
                    WCDB::ColumnDef(column_class_id, WCDB::ColumnType::Integer32),
                },
                true)
        .getDescription();
}

/* 插入数据 */
std::string insertSql(const std::string& name, int sex, int age, const std::string& country, int classId)
{
    return WCDB::StatementInsert()
        .insert(tbname, WCDB::Conflict::Replace)
        .values({nullptr, name, sex, age, country, classId})
        .getDescription();
}

/* 根据年龄删除数据 */
std::string deleteByAgeSql(int age)
{
    return WCDB::StatementDelete().deleteFrom(tbname).where(WCDB::Expr(column_age) == age).getDescription();
}

/* 根据ID更新年龄 */
std::string updateAgeByIdSql(int id, int newAge)
{
    return WCDB::StatementUpdate()
        .update(tbname)
        .set(std::list<std::pair<WCDB::Column, WCDB::Expr>>{{column_age, newAge}})
        .where(WCDB::Expr(column_id) == id)
        .getDescription();
}

/* 查询所有数据 */
std::string selectAllSql()
{
    WCDB::ColumnResultList results = {{column_id}, {column_name}, {column_sex}, {column_age}, {column_country}, {column_class_id}};
    return WCDB::StatementSelect().select(results).from(tbname).getDescription();
}
} // namespace student_table

/* 班级表 */
namespace class_table
{
const std::string tbname = "class"; /* 表名 */
const WCDB::Column column_id("id"); /* 唯一ID */
const WCDB::Column column_department("department"); /* 系别 */
const WCDB::Column column_grade("grade"); /* 年级 */
const WCDB::Column column_number("number"); /* 班级 */
const WCDB::Column column_student_count("student_count"); /* 学生数 */

/* 删除表 */
std::string dropSql()
{
    return WCDB::StatementDropTable().drop(tbname).getDescription();
}

/* 创建表 */
std::string createSql()
{
    return WCDB::StatementCreateTable()
        .create(tbname,
                std::list<WCDB::ColumnDef>{
                    WCDB::ColumnDef(column_id, WCDB::ColumnType::Integer64)
                        .makePrimary(WCDB::OrderTerm::NotSet, true)
                        .makeUnique()
                        .makeNotNull()
                        .makeDefault(1),
                    WCDB::ColumnDef(column_department, WCDB::ColumnType::Text),
                    WCDB::ColumnDef(column_grade, WCDB::ColumnType::Integer32),
                    WCDB::ColumnDef(column_number, WCDB::ColumnType::Integer32),
                    WCDB::ColumnDef(column_student_count, WCDB::ColumnType::Integer32),
                },
                true)
        .getDescription();
}

/* 插入数据 */
std::string insertSql(const std::string& department, int grade, int number, int studentCount)
{
    return WCDB::StatementInsert()
        .insert(tbname, WCDB::Conflict::Replace)
        .values({nullptr, department, grade, number, studentCount})
        .getDescription();
}

/* 根据学生年龄获取班级信息 */
std::string selectByStudentAgeSql(int age)
{
    WCDB::ColumnResultList results = {{column_id.inTable(tbname)},
                                      {column_department.inTable(tbname)},
                                      {column_grade.inTable(tbname)},
                                      {column_number.inTable(tbname)},
                                      {column_student_count.inTable(tbname)}};
    return WCDB::StatementSelect()
        .select(results)
        .from(WCDB::JoinClause(tbname)
                  .join(student_table::tbname, WCDB::JoinClause::Type::Inner)
                  .on(WCDB::Expr(column_id.inTable(tbname)) == WCDB::Expr(student_table::column_class_id.inTable(student_table::tbname))
                      && WCDB::Expr(student_table::column_age.inTable(student_table::tbname)) > age))
        .getDescription();
}
} // namespace class_table

void testWinq()
{
    printf("========================= test winq =========================\n");
    database::Sqlite db(g_dbName, g_dbPwd);
    if (!db.connect())
    {
        printf("===== connect database %s fail\n", g_dbName.c_str());
        return;
    }
    printf("===== connect database %s ok\n", g_dbName.c_str());
    /* 删除学生表 */
    {
        auto sql = student_table::dropSql();
        printf("SQL: %s\n", sql.c_str());
        bool ret = db.execSql(sql);
        if (!ret)
        {
            printf("drop table '%s' fail\n", student_table::tbname.c_str());
        }
    }
    /* 创建学生表 */
    {
        auto sql = student_table::createSql();
        printf("SQL: %s\n", sql.c_str());
        bool ret = db.execSql(sql);
        if (!ret)
        {
            printf("create table '%s' fail\n", student_table::tbname.c_str());
            return;
        }
    }
    /* 添加学生数据 */
    {
        std::vector<std::string> peopleNames{"Zhang San", "Li Si",   "Qian Wu",   "Wu Liu",    "Zhao Qi",
                                             "Li Fang",   "Han Lei", "Chen Cong", "Xiang Fei", "Lei Ting"};
        for (size_t i = 0; i < peopleNames.size(); ++i)
        {
            auto sql = student_table::insertSql(peopleNames[i], (i % 2), 30 + i, "china", (peopleNames[i].size() % 2) + 1);
            printf("SQL: %s\n", sql.c_str());
            bool ret = db.execSql(sql);
            if (!ret)
            {
                printf("insert table '%s' fail\n", student_table::tbname.c_str());
            }
        }
    }
    /* 查询学生数据 */
    {
        auto sql = student_table::selectAllSql();
        printf("SQL: %s\n", sql.c_str());
        bool ret = db.execSql(sql, [&](const std::unordered_map<std::string, std::string>& columns) {
            printf("    ----------------------------------------\n");
            for (auto iter = columns.begin(); columns.end() != iter; ++iter)
            {
                const auto& key = iter->first;
                const auto& value = iter->second;
                printf("    %s: %s\n", key.c_str(), value.c_str());
            }
            return true;
        });
        if (!ret)
        {
            printf("select table '%s' fail\n", student_table::tbname.c_str());
        }
    }
    /* 更新学生数据 */
    {
        auto sql = student_table::updateAgeByIdSql(2, 40);
        printf("SQL: %s\n", sql.c_str());
        bool ret;
        ret = db.execSql(sql);
        if (!ret)
        {
            printf("update table '%s' fail\n", student_table::tbname.c_str());
        }
    }
    /* 删除学生数据 */
    {
        auto sql = student_table::deleteByAgeSql(32);
        printf("SQL: %s\n", sql.c_str());
        bool ret;
        ret = db.execSql(sql);
        if (!ret)
        {
            printf("delete table '%s' fail\n", student_table::tbname.c_str());
        }
    }
    /* 删除班级表 */
    {
        auto sql = class_table::dropSql();
        printf("SQL: %s\n", sql.c_str());
        bool ret = db.execSql(sql);
        if (!ret)
        {
            printf("drop table '%s' fail\n", class_table::tbname.c_str());
        }
    }
    /* 创建班级表 */
    {
        auto sql = class_table::createSql();
        printf("SQL: %s\n", sql.c_str());
        bool ret = db.execSql(sql);
        if (!ret)
        {
            printf("create table '%s' fail\n", class_table::tbname.c_str());
            return;
        }
    }
    /* 添加班级数据 */
    {
        std::vector<std::string> departments{"Computer", "Foreign Language", "Physics"};
        for (size_t i = 0; i < departments.size(); ++i)
        {
            auto sql = class_table::insertSql(departments[i], 1, i + 1, departments.size());
            printf("SQL: %s\n", sql.c_str());
            bool ret = db.execSql(sql);
            if (!ret)
            {
                printf("insert table '%s' fail\n", class_table::tbname.c_str());
            }
        }
    }
    /* 查询班级数据 */
    {
        auto sql = class_table::selectByStudentAgeSql(34);
        printf("SQL: %s\n", sql.c_str());
        bool ret = db.execSql(sql, [&](const std::unordered_map<std::string, std::string>& columns) {
            printf("    ----------------------------------------\n");
            for (auto iter = columns.begin(); columns.end() != iter; ++iter)
            {
                const auto& key = iter->first;
                const auto& value = iter->second;
                printf("    %s: %s\n", key.c_str(), value.c_str());
            }
            return true;
        });
        if (!ret)
        {
            printf("select table '%s' fail\n", class_table::tbname.c_str());
        }
    }
}

int main()
{
    testDb1();
    printf("\n\n\n\n\n");
    testWinq();
    return 0;
}
