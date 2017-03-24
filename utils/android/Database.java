package com.jaronho.sdk.utils;

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteException;
import android.database.sqlite.SQLiteOpenHelper;
import android.database.sqlite.SQLiteDatabase.CursorFactory;
import android.util.Log;

public class Database {
	private static String TAG = "Database";
	// SQLite
	private static class SQLiteHelper extends SQLiteOpenHelper {
		public SQLiteHelper(Context context, String name, CursorFactory factory, int version) {
		    super(context, name, factory, version);
		}
		@Override
		public void onCreate(SQLiteDatabase db) {
		}
		@Override
		public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion){
		}
	}
	
	// 当前使用的用户数据库
	private static SQLiteOpenHelper mSQLiteHelper = null;
	
	/**
     * 创建数据库
     * @param context 上下文
     * @param name 数据库名
     * @param factory 游标工厂
     * @param version 版本号
     */
	public static SQLiteOpenHelper create(Context context, String name, CursorFactory factory, int version) {
		if (null != mSQLiteHelper) {
			return mSQLiteHelper;
		}
		try {
			mSQLiteHelper = new SQLiteHelper(context, name, factory, version);
		} catch (SQLiteException e) {
			Log.e(TAG, "create -> name: " + name + ", version: " + version + "\n" + e.toString());
		}
		return mSQLiteHelper;
	}
	
	/**
     * 关闭当前使用数据库
     */
	public static void close() {
		if (null == mSQLiteHelper) {
			return;
		}
		mSQLiteHelper.getWritableDatabase().close();
		mSQLiteHelper.getReadableDatabase().close();
		mSQLiteHelper.close();
		mSQLiteHelper = null;
	}
	
	/**
     * 执行增、删、改操作SQL
     * @param sql SQL语句
     */
	public static void execute(String sql) {
		if (null == mSQLiteHelper) {
			return;
		}
		SQLiteDatabase database = null;
		try {
			database = mSQLiteHelper.getWritableDatabase();
		}
		catch (SQLiteException e) {
			return;
		}
		database.execSQL(sql);
	}
	
	/**
     * 插入数据
     * @param table 表名
     * @param values 插入的数据内容
     * @return 返回值 -1 为失败
     */
	public static long insert(String table, ContentValues values) {
		if (null == mSQLiteHelper) {
			return -1L;
		}
		SQLiteDatabase database = null;
		try {
			database = mSQLiteHelper.getWritableDatabase();
		} catch (SQLiteException e) {
			return -1L;
		}
		return database.insert(table, null, values);
	}
	
	/**
     * 删除
     * @param table 表名
     * @param whereClause 条件
     * @param whereArgs 条件参数值
     * @return 返回值 -1 为失败
     */
	public static int delete(String table, String whereClause, String[] whereArgs) {
		if (null == mSQLiteHelper) {
			return -1;
		}
		SQLiteDatabase database = null;
		try {
			database = mSQLiteHelper.getWritableDatabase();
		} catch (SQLiteException e) {
			return -1;
		}
		return database.delete(table, whereClause, whereArgs);
	}
	
	/**
     * 更新数据
     * @param table 表名
     * @param values 更新数据内容
     * @param whereClause 条件
     * @param whereArgs 条件参数
     * @return 返回值 -1 为失败
     */
	public static int update(String table, ContentValues values, String whereClause, String[] whereArgs) {
		if (null == mSQLiteHelper) {
			return -1;
		}
		SQLiteDatabase database = null;
		try {
			database = mSQLiteHelper.getWritableDatabase();
		} catch (SQLiteException e) {
			return -1;
		}
		return database.update(table, values, whereClause, whereArgs);
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
     * @return 返回数据库查询Cursor
     */
	public static Cursor query(String table, String[] columns, String selection, String[] selectionArgs, String groupBy, String having, String orderBy) {
		if (null == mSQLiteHelper) {
			return null;
		}
		SQLiteDatabase database = null;
		try {
			database = mSQLiteHelper.getReadableDatabase();
		} catch (SQLiteException e) {
			return null;
		}
		return database.query(table, columns, selection, selectionArgs, groupBy, having, orderBy);
	}
	
	/**
     * 查询
     * @param sql 查询条件SQL语句
     * @param selectionArgs 查询条件参数
     * @return 返回查询的  Cursor
     */
	public static Cursor query(String sql, String[] selectionArgs) {
		if (null == mSQLiteHelper) {
			return null;
		}
		SQLiteDatabase database = null;
		try {
			database = mSQLiteHelper.getWritableDatabase();
		} catch (SQLiteException e) {
			return null;
		}
		return database.rawQuery(sql, selectionArgs);
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
     * @return 返回查询的  Cursor
     */
	public static Cursor query(String table, String[] columns, String selection, String[] selectionArgs, String groupBy, String having, String orderBy, String limit) {
		if (null == mSQLiteHelper) {
			return null;
		}
		SQLiteDatabase database = null;
		try {
			database = mSQLiteHelper.getWritableDatabase();
		} catch (SQLiteException e) {
			return null;
		}
		return database.query(table, columns, selection, selectionArgs, groupBy, having, orderBy, limit);
	}
}
