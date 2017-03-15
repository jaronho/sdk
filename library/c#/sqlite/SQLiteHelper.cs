using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Data;
using System.Data.Common;
using System.Data.SQLite;

namespace SQLiteNameSpace
{
    class SQLiteHelper
    {
        private SQLiteConnection mConn = null;

        /* 构造函数
         * dbPath：SQLite数据库文件路径
        */
        public SQLiteHelper(string dbPath)
        {
            mConn = new SQLiteConnection("Data Source=" + dbPath);
            mConn.Open();
        }

        /* 对SQLite数据库执行增删改操作，返回受影响的行数
         * sql：要执行的增删改的SQL语句
         * parameters：执行SQL增删改语句所需要的参数，参数必须以它们在SQL语句中的顺序为准
        */
        public int excuteNonQuery(string sql, SQLiteParameter[] parameters)
        {
            SQLiteCommand cmd = new SQLiteCommand(sql, mConn);
            if (null != parameters)
            {
                cmd.Parameters.AddRange(parameters);
            }
            return cmd.ExecuteNonQuery();
        }

        /* 对数据库执行查询语句，返回一个关联的SQLiteDataReader实例
         * sql：要执行的查询的SQL语句
         * parameters：执行SQL查询语句所需要的参数，参数必须以它们在SQL语句中的顺序为准
        */
        public SQLiteDataReader excuteQuery(string sql, SQLiteParameter[] parameters)
        {
            SQLiteCommand cmd = new SQLiteCommand(sql, mConn);
            if (null != parameters)
            {
                cmd.Parameters.AddRange(parameters);
            }
            return cmd.ExecuteReader();
        }

        /* 创建SQLite数据库文件
         * dbPath：要创建的SQLite数据库文件路径
        */
        public static void createDB(string dbPath)
        {
            if (File.Exists((dbPath)))
                return;

            SQLiteConnection.CreateFile(dbPath);
        }
    }
}
