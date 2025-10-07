package com.zhuchao.android.persist;

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;

import androidx.annotation.Nullable;

import com.zhuchao.android.fbase.MMLog;

import java.util.ArrayList;

public class DBHelper extends SQLiteOpenHelper {
    private final String TAG = "DBHelper";
    private static DBHelper dbHelper;
    private SQLiteDatabase db;
    private String DefaultDatabaseName;
    private String DefaultTableName;

    public DBHelper(@Nullable Context context, @Nullable String name, @Nullable SQLiteDatabase.CursorFactory factory, int version) {
        super(context, name, factory, version);
    }

    public static DBHelper getInstance() {
        // (dbHelper == null) {
        //dbHelper = new DBHelper();
        //     return dbHelper;
        // }
        return dbHelper;
    }

    public void readDatabase(Context context, String databaseName, String tableName) {
        DefaultDatabaseName = databaseName;
        DefaultTableName = tableName;

        if (DefaultDatabaseName == null) DefaultDatabaseName = "CommonDB.db";
        if (DefaultTableName == null) DefaultTableName = "MyTable";

        String path = context.getCacheDir().getPath() + "/" + DefaultDatabaseName;

        db = SQLiteDatabase.openOrCreateDatabase(path, null);

        String sql = "create table if not exists " + DefaultTableName + "(id integer primary key autoincrement," + "name text(50),p1 text(50),p2 text(50)," + "p3 text(50),p4 text(50),p5 text(50),p6 text(50)," + "p7 text(50),p8 text(50),p9 text(50),p10 text(50),p11 text(50),p12 text(50),p13 text(50),p14 text(50))";

        db.execSQL(sql);//创建表
    }


    public ArrayList<String> selectAllData(String tableName) {
        ArrayList<String> list = new ArrayList<>();
        Cursor cursor = db.query(tableName, null, null, null, null, null, null);

        if (cursor != null) {
            // 先获取列索引
            int idIndex = cursor.getColumnIndex("id");
            int nameIndex = cursor.getColumnIndex("name");
            int[] pIndexes = new int[14];
            for (int i = 0; i < 14; i++) {
                pIndexes[i] = cursor.getColumnIndex("p" + (i + 1));
            }

            while (cursor.moveToNext()) {
                int id = (idIndex != -1) ? cursor.getInt(idIndex) : -1;
                String name = (nameIndex != -1) ? cursor.getString(nameIndex) : "";

                String[] pValues = new String[14];
                for (int i = 0; i < 14; i++) {
                    pValues[i] = (pIndexes[i] != -1) ? cursor.getString(pIndexes[i]) : "";
                }

                StringBuilder sb = new StringBuilder();
                sb.append(id).append(',').append(name);
                for (String p : pValues) {
                    sb.append(',').append(p);
                }

                list.add(sb.toString());
            }
            cursor.close();
        }

        return list;
    }

    public ArrayList<String> selectData(String tableName, String name) {
        ArrayList<String> list = new ArrayList<>();
        Cursor cursor = db.query(tableName, null, "name = ?", new String[]{name}, null, null, null);

        if (cursor != null) {
            int idIndex = cursor.getColumnIndex("id");
            int nameIndex = cursor.getColumnIndex("name");
            int[] pIndexes = new int[14];
            for (int i = 0; i < 14; i++) {
                pIndexes[i] = cursor.getColumnIndex("p" + (i + 1));
            }

            while (cursor.moveToNext()) {
                int id = (idIndex != -1) ? cursor.getInt(idIndex) : -1;
                String nameStr = (nameIndex != -1) ? cursor.getString(nameIndex) : "";

                StringBuilder sb = new StringBuilder();
                sb.append(id).append(',').append(nameStr);

                for (int i = 0; i < 14; i++) {
                    String p = (pIndexes[i] != -1) ? cursor.getString(pIndexes[i]) : "";
                    sb.append(',').append(p);
                }

                list.add(sb.toString());
            }
            cursor.close();
        }

        return list;
    }

    public ArrayList<String> selectData(String tableName, int Userid) {
        ArrayList<String> list = new ArrayList<>();
        Cursor cursor = db.query(tableName, null, "id = ?", new String[]{String.valueOf(Userid)}, null, null, null);

        if (cursor != null) {
            int idIndex = cursor.getColumnIndex("id");
            int nameIndex = cursor.getColumnIndex("name");
            int[] pIndexes = new int[14];
            for (int i = 0; i < 14; i++) {
                pIndexes[i] = cursor.getColumnIndex("p" + (i + 1));
            }

            while (cursor.moveToNext()) {
                int id = (idIndex != -1) ? cursor.getInt(idIndex) : -1;
                String name = (nameIndex != -1) ? cursor.getString(nameIndex) : "";

                StringBuilder sb = new StringBuilder();
                sb.append(id).append(',').append(name);

                for (int i = 0; i < 14; i++) {
                    String p = (pIndexes[i] != -1) ? cursor.getString(pIndexes[i]) : "";
                    sb.append(',').append(p);
                }

                list.add(sb.toString());
            }

            cursor.close();
        }

        return list;
    }

    public ArrayList<String> selectDataNameP1(String tableName, String name, String p1) {
        ArrayList<String> list = new ArrayList<>();
        Cursor cursor = db.query(tableName, null, "name = ? and p1 = ?", new String[]{name, p1}, null, null, null);

        if (cursor != null) {
            int idIndex = cursor.getColumnIndex("id");
            int nameIndex = cursor.getColumnIndex("name");
            int p1Index = cursor.getColumnIndex("p1");
            int[] pIndexes = new int[13]; // p2~p14
            for (int i = 0; i < 13; i++) {
                pIndexes[i] = cursor.getColumnIndex("p" + (i + 2));
            }

            while (cursor.moveToNext()) {
                int id = (idIndex != -1) ? cursor.getInt(idIndex) : -1;
                String rowName = (nameIndex != -1) ? cursor.getString(nameIndex) : "";
                String rowP1 = (p1Index != -1) ? cursor.getString(p1Index) : "";

                StringBuilder sb = new StringBuilder();
                sb.append(id).append(',').append(rowName).append(',').append(rowP1);

                for (int i = 0; i < 13; i++) {
                    String p = (pIndexes[i] != -1) ? cursor.getString(pIndexes[i]) : "";
                    sb.append(',').append(p);
                }

                list.add(sb.toString());
            }
            cursor.close();
        }

        return list;
    }

    /**
     * 根据ID删除数据
     * id 删除id
     */
    public int delData(String tableName, int id) {
        int index = db.delete(tableName, "id = ?", new String[]{String.valueOf(id)});
        //MLog("--Main--", "删除了==============" + index);
        return index;
    }

    public int delData(String tableName, String name) {
        int index = db.delete(tableName, "name = ?", new String[]{name});
        MMLog.log("--Main--", "删除了==============" + index);
        return index;
    }

    public int delDataNameP1(String tableName, String name, String p1) {
        int index = db.delete(tableName, "name = ? and p1 = ?", new String[]{name, p1});
        MMLog.log("--Main--", "删除了==============" + index);
        return index;
    }

    public int modifyData(int id, String tableName, String name, String p1, String p2, String p3, String p4, String p5, String p6, String p7, String p8, String p9, String p10, String p11, String p12, String p13, String p14) {
        ContentValues contentValues = new ContentValues();
        contentValues.put("name", name);
        contentValues.put("p1", p1);
        contentValues.put("p2", p2);
        contentValues.put("p3", p3);
        contentValues.put("p4", p4);
        contentValues.put("p5", p5);
        contentValues.put("p6", p6);
        contentValues.put("p7", p7);
        contentValues.put("p8", p8);
        contentValues.put("p9", p9);
        contentValues.put("p10", p10);
        contentValues.put("p11", p11);
        contentValues.put("p12", p12);
        contentValues.put("p13", p13);
        contentValues.put("p14", p14);
        int index = db.update(tableName, contentValues, "id = ?", new String[]{String.valueOf(id)});
        //MLog("--Main--", "修改了===============" + index);
        return index;
    }

    public int modifyData(String tableName, String name, String p1, String p2, String p3, String p4, String p5, String p6, String p7, String p8, String p9, String p10, String p11, String p12, String p13, String p14) {
        ContentValues contentValues = new ContentValues();
        //contentValues.put("name", name);
        contentValues.put("p1", p1);
        contentValues.put("p2", p2);
        contentValues.put("p3", p3);
        contentValues.put("p4", p4);
        contentValues.put("p5", p5);
        contentValues.put("p6", p6);
        contentValues.put("p7", p7);
        contentValues.put("p8", p8);
        contentValues.put("p9", p9);
        contentValues.put("p10", p10);
        contentValues.put("p11", p11);
        contentValues.put("p12", p12);
        contentValues.put("p13", p13);
        contentValues.put("p14", p14);
        int index = db.update(tableName, contentValues, "name = ?", new String[]{name});
        //MLog("--Main--", "修改了===============" + index);
        return index;
    }

    public int modifyDataByNameP1(String tableName, String name, String p1, String p2, String p3, String p4, String p5, String p6, String p7, String p8, String p9, String p10, String p11, String p12, String p13, String p14) {
        ContentValues contentValues = new ContentValues();
        //contentValues.put("name", name);
        //contentValues.put("p1", p1);
        contentValues.put("p2", p2);
        contentValues.put("p3", p3);
        contentValues.put("p4", p4);
        contentValues.put("p5", p5);
        contentValues.put("p6", p6);
        contentValues.put("p7", p7);
        contentValues.put("p8", p8);
        contentValues.put("p9", p9);
        contentValues.put("p10", p10);
        contentValues.put("p11", p11);
        contentValues.put("p12", p12);
        contentValues.put("p13", p13);
        contentValues.put("p14", p14);
        int index = db.update(tableName, contentValues, "name = ? and p1 = ?", new String[]{name, p1});
        //MLog("--Main--", "修改了===============" + index);
        return index;
    }

    /**
     * 添加数据
     * bsid 添加的数据ID
     * name 添加数据名称
     */
    public long insertData(String tableName, String name, String p1, String p2, String p3, String p4, String p5, String p6, String p7, String p8, String p9, String p10, String p11, String p12, String p13, String p14) {
        ContentValues contentValues = new ContentValues();
        contentValues.put("name", name);
        contentValues.put("p1", p1);
        contentValues.put("p2", p2);
        contentValues.put("p3", p3);
        contentValues.put("p4", p4);
        contentValues.put("p5", p5);
        contentValues.put("p6", p6);
        contentValues.put("p7", p7);
        contentValues.put("p8", p8);
        contentValues.put("p9", p9);
        contentValues.put("p10", p10);
        contentValues.put("p11", p11);
        contentValues.put("p12", p12);
        contentValues.put("p13", p13);
        contentValues.put("p14", p14);
        long dataSize = db.insert(tableName, null, contentValues);

        MMLog.log("--insertData--", "insertData====" + name);
        return dataSize;
    }

    public void updateData(String tableName, String name, String p1, String p2, String p3, String p4, String p5, String p6, String p7, String p8, String p9, String p10, String p11, String p12, String p13, String p14) {
        if (existsData(tableName, name))
            modifyData(tableName, name, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14);
        else
            insertData(tableName, name, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14);
    }

    public void updateData2(String tableName, String name, String p1, String p2, String p3, String p4, String p5, String p6, String p7, String p8, String p9, String p10, String p11, String p12, String p13, String p14) {
        if (existsDataNameP1(tableName, name, p1))
            modifyDataByNameP1(tableName, name, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14);
        else
            insertData(tableName, name, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14);
    }

    /*
     * 查询名字单个数据
     * @param name
     * @return
     */
    public boolean existsData(String tableName, String name) {
        //查询数据库
        Cursor cursor = db.query(tableName, null, "name = ?", new String[]{name}, null, null, null);
        while (cursor.moveToNext()) {
            return true;
        }
        return false;
    }

    public boolean existsDataNameP1(String tableName, String name, String p1) {
        //查询数据库
        Cursor cursor = db.query(tableName, null, "name = ? and p1 = ?", new String[]{name, p1}, null, null, null);
        while (cursor.moveToNext()) {
            return true;
        }
        return false;
    }

    @Override
    public void onCreate(SQLiteDatabase db) {

    }

    @Override
    public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {

    }
}
