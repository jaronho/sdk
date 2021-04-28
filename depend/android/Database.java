package com.jaronho.sdk.utils;

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.DatabaseErrorHandler;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteException;
import android.database.sqlite.SQLiteOpenHelper;
import android.database.sqlite.SQLiteDatabase.CursorFactory;
import android.util.Log;

/**
 * Author:  jaron.ho
 * Date:    2017-05-22
 * Brief:   SQLite数据库操作类
 */

public class Database extends SQLiteOpenHelper {
	private static final String TAG = "Database";

	/**
	 * 构造函数
	 * @param context 上下文
	 * @param name 数据库名
	 * @param factory 游标工厂
	 * @param version 版本号
	 * @param errorHandler 错误处理句柄
	 */
	public Database(Context context, String name, CursorFactory factory, int version, DatabaseErrorHandler errorHandler) {
		super(context, name, factory, version, errorHandler);
	}

	public Database(Context context, String name, CursorFactory factory, int version) {
		this(context, name, factory, version, null);
	}

	@Override
	public void onCreate(SQLiteDatabase db) {
	}

	@Override
	public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
	}

	 /**
     * 关闭当前使用数据库
     */
	@Override
	public synchronized void close() {
		try {
			getWritableDatabase().close();
			getReadableDatabase().close();
			super.close();
		} catch (Exception e) {
			Log.e(TAG, "close: " + e.toString());
		}
	}

	/**
     * 执行增、删、改操作SQL
     * @param sql SQL语句
     */
	public void execute(String sql) {
		try {
			getWritableDatabase().execSQL(sql);;
		} catch (SQLiteException e) {
			Log.e(TAG, "execute -> sql: " + sql + "\n" + e.toString());
		}
	}
	
	/**
     * 插入数据
     * @param table 表名
     * @param values 插入的数据内容
     * @return 返回值:-1为失败
     */
	public long insert(String table, ContentValues values) {
		try {
			return getWritableDatabase().insert(table, null, values);
		} catch (SQLiteException e) {
			Log.e(TAG, "insert -> table: " + table + "\n" + e.toString());
		}
		return -1L;
	}
	
	/**
     * 删除
     * @param table 表名
     * @param whereClause 条件
     * @param whereArgs 条件参数值
     * @return 返回值:-1为失败
     */
	public int delete(String table, String whereClause, String[] whereArgs) {
		try {
			getWritableDatabase().delete(table, whereClause, whereArgs);
		} catch (SQLiteException e) {
			Log.e(TAG, "delete -> table: " + table + "\n" + e.toString());
		}
		return -1;
	}
	
	/**
     * 更新数据
     * @param table 表名
     * @param values 更新数据内容
     * @param whereClause 条件
     * @param whereArgs 条件参数
     * @return 返回值:-1为失败
     */
	public int update(String table, ContentValues values, String whereClause, String[] whereArgs) {
		try {
			return getWritableDatabase().update(table, values, whereClause, whereArgs);
		} catch (SQLiteException e) {
			Log.e(TAG, "update -> table: " + table + "\n" + e.toString());
		}
		return -1;
	}
	
	/**
     * 查询
     * @param table 数据库名称
     * @param columns  列名(数组)
     * @param selection 查询条件 (可为空)
     * @param selectionArgs 条件参数值(可为空)
     * @param groupBy 分组字段(可为空)
     * @param having 分组条件字段(可为空)
     * @param orderBy 排序字段(可为空)  
     * @return 返回查询的Cursor
     */
	public Cursor query(String table, String[] columns, String selection, String[] selectionArgs, String groupBy, String having, String orderBy) {
		try {
			return getReadableDatabase().query(table, columns, selection, selectionArgs, groupBy, having, orderBy);
		} catch (SQLiteException e) {
			Log.e(TAG, "query -> table: " + table + "\n" + e.toString());
		}
		return null;
	}
	
	/**
     * 查询
     * @param sql 查询条件SQL语句
     * @param selectionArgs 查询条件参数
     * @return 返回查询的Cursor
     */
	public Cursor query(String sql, String[] selectionArgs) {
		try {
			return getWritableDatabase().rawQuery(sql, selectionArgs);
		} catch (SQLiteException e) {
			Log.e(TAG, "query -> sql: " + sql + "\n" + e.toString());
		}
		return null;
	}
	
	/**
     * 查询
     * @param table 数据库名称
     * @param columns 列名(数组)
     * @param selection 查询条件 (可为空)
     * @param selectionArgs 条件参数值(可为空)
     * @param groupBy 分组字段(可为空)
     * @param having 分组条件字段(可为空)
     * @param orderBy 排序字段(可为空)
     * @param limit 指定偏移量和获取的记录数(可为空)
     * @return 返回查询的Cursor
     */
	public Cursor query(String table, String[] columns, String selection, String[] selectionArgs, String groupBy, String having, String orderBy, String limit) {
		try {
			return getWritableDatabase().query(table, columns, selection, selectionArgs, groupBy, having, orderBy, limit);
		} catch (SQLiteException e) {
			Log.e(TAG, "query -> table: " + table + "\n" + e.toString());
		}
		return null;
	}
}
