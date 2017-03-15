package com.yaxon.hudmain.jh.library;

import android.os.Debug;
import android.util.Log;

import java.util.Arrays;

/**
 * Author:  jaron.ho
 * Date:    2017-03-15
 * Brief:   字节流类(网络协议序列化)
 */

public class ByteArray {
    private static final int MAX_MSG_SIZE = 1024 * 1024;    // 单个网络消息最大长度(超过极易导致物理服务器收发队列阻塞)
    private byte[] mContent;            // 字节流内容
    private int mTotalSize;             // 字节流允许大小
    private int mReadIndex;             // 读取位置
    private int mWriteIndex;            // 写入位置

    /**
     * 功  能: 大小端转换(短整型数字转为短整型数字)
     * 参  数: x - 短整型数字
     * 返回值: short
     */
    public static short swab16(short x) {
        return (short)(((x & 0x00FF) << 8) | ((x & 0xFF00) >> 8));
    }

    /**
     * 功  能: 大小端转换(字节流转为短整型数字)
     * 参  数: buf - 字节流
     * 返回值: short
     */
    public static short swab16_array(byte[] buf) {
        return (short)((buf[0] << 8) | buf[1]);
    }

    /**
     * 功  能: 大小端转换(整型数字转为整型数字)
     * 参  数: x - 整型数字
     * 返回值: int
     */
    public static int swab32(int x) {
        return (((x & 0x000000FF) << 24) | ((x & 0x0000FF00) << 8) | ((x & 0x00FF0000) >> 8) | ((x & 0xFF000000) >> 24));
    }

    /**
     * 功  能: 大小端转换(字节流转为整型数字)
     * 参  数: buf - 字节流
     * 返回值: int
     */
    public static int swab32_array(byte[] buf) {
        return ((buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3]);
    }

    /**
     * 功  能: 网络消息最大长度
     * 参  数: 无
     * 返回值: int
     */
    public static int max_size() {
        return MAX_MSG_SIZE;
    };

    public ByteArray(int size) {
        if (size <= 0 || size > MAX_MSG_SIZE) {
            size = MAX_MSG_SIZE;
        }
        mContent = new byte[size];
        for (int i = 0; i < size; ++i) {
            mContent[i] = 0;
        }
        mTotalSize = size;
        mReadIndex = 0;
        mWriteIndex = 0;
    }

    /**
     * 功  能: 打印当前内容
     * 参  数: 无
     * 返回值: 无
     */
    public void print() {
        String TAG = "ByteArray";
        Log.d(TAG, String.format("==================== ByteArray: max=%d, length=%d, space=%d",  mTotalSize, mContent.length, getSpaceLength()));
        Log.d(TAG, Arrays.toString(mContent));
        Log.d(TAG, "========================================");
    }

    /**
     * 功  能: 清空,重新使用当前字节流
     * 参  数: 无
     * 返回值: 无
     */
    public void reuse() {
        for (int i = 0; i < mTotalSize; ++i) {
            mContent[i] = 0;
        }
        mReadIndex = 0;
        mWriteIndex = 0;
    }

    /**
     * 功  能: 获取字节流总长度
     * 参  数: 无
     * 返回值: int
     */
    public int getTotalLength() {
        return mTotalSize;
    }

    /**
     * 功  能: 获取字节流当前长度
     * 参  数: 无
     * 返回值: int
     */
    public int getCurrentLength() {
        return Math.abs(mWriteIndex - mReadIndex);
    }

    /**
     * 功  能: 获取字节流剩余可写长度
     * 参  数: 无
     * 返回值: int
     */
    public int getSpaceLength() {
        return mTotalSize - mWriteIndex;
    }

    /**
     * 功  能: 获取字节流内容
     * 参  数: 无
     * 返回值: byte[]
     */
    public final byte[] getContent() {
        return mContent;
    }

    /**
     * 功  能: 设置字节流内容
     * 参  数: content - 字节流
     *         len - 长度
     * 返回值: boolean
     */
    public boolean setContent(final byte[] content, int len) {
        if (len > mTotalSize) {
            return false;
        }
        for (int i = 0; i < mTotalSize; ++i) {
            if (i < len) {
                mContent[i] = content[i];
            } else {
                mContent[i] = 0;
            }
        }
        mWriteIndex = len;
        return true;
    }

    /**
     * 功  能: 从字节流读取布尔型
     * 参  数: 无
     * 返回值: boolean
     */
    public boolean read_bool() {
        byte[] p = read(1);
        return null != p && 0x01 == p[0];
    }

    /**
     * 功  能: 向字节流写入布尔型
     * 参  数: value - 值
     * 返回值: boolean
     */
    public boolean write_bool(boolean value) {
        int index  = writeIndex(1);
        if (-1 == index) {
            return false;
        }
        mContent[index] = (byte)(value ? 0x01 : 0x00);
        return true;
    }

    /**
     * 功  能: 从字节流读取字节型
     * 参  数: 无
     * 返回值: byte
     */
    public byte read_byte() {
        byte[] p = read(1);
        if (null == p) {
            return 0;
        }
        return p[0];
    }

    /**
     * 功  能: 向字节流写入字节型
     * 参  数: value - 值
     * 返回值: boolean
     */
    public boolean write_byte(byte value) {
        int index  = writeIndex(1);
        if (-1 == index) {
            return false;
        }
        mContent[index] = value;
        return true;
    }

    /**
     * 功  能: 从字节流读取字符型
     * 参  数: 无
     * 返回值: byte
     */
    public char read_char() {
        byte[] p = read(2);
        if (null == p) {
            return 0;
        }
        return (char)(p[0] | (p[1] << 8));
    }

    /**
     * 功  能: 向字节流写入字符型
     * 参  数: value - 值
     * 返回值: boolean
     */
    public boolean write_char(char value) {
        int index  = writeIndex(2);
        if (-1 == index) {
            return false;
        }
        mContent[index] = (byte)value;
        mContent[index + 1] = (byte)(value >> 8);
        return true;
    }

    /**
     * 功  能: 从字节流读取16位整型
     * 参  数: 无
     * 返回值: short
     */
    public short read_int16() {
        byte[] p = read(2);
        if (null == p) {
            return 0;
        }
        return (short)(p[0] | (p[1] << 8));
    }

    /**
     * 功  能: 向字节流写入16位整型
     * 参  数: value - 值
     * 返回值: boolean
     */
    public boolean write_int16(short value) {
        int index  = writeIndex(2);
        if (-1 == index) {
            return false;
        }
        mContent[index] = (byte)value;
        mContent[index + 1] = (byte)(value >> 8);
        return true;
    }

    /**
     * 功  能: 从字节流读取32位整型
     * 参  数: 无
     * 返回值: int
     */
    public int read_int() {
        byte[] p = read(4);
        if (null == p) {
            return 0;
        }
        return p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
    }

    /**
     * 功  能: 向字节流写入32位整型
     * 参  数: value - 值
     * 返回值: boolean
     */
    public boolean write_int(int value) {
        int index  = writeIndex(4);
        if (-1 == index) {
            return false;
        }
        mContent[index] = (byte)value;
        mContent[index + 1] = (byte)(value >> 8);
        mContent[index + 2] = (byte)(value >> 16);
        mContent[index + 3] = (byte)(value >> 24);
        return true;
    }

    /**
     * 功  能: 从字节流读取长整型
     * 参  数: 无
     * 返回值: long
     */
    public long read_long() {
        byte[] p = read(8);
        if (null == p) {
            return 0;
        }
        return (long)p[0] | ((long)p[1] << 8) | ((long)p[2] << 16) | ((long)p[3] << 24)
                | ((long)p[4] << 32) | ((long)p[5] << 40) | ((long)p[6] << 48) | ((long)p[7] << 56);
    }

    /**
     * 功  能: 向字节流写入长整型
     * 参  数: value - 值
     * 返回值: boolean
     */
    public boolean write_long(long value) {
        int index  = writeIndex(8);
        if (-1 == index) {
            return false;
        }
        mContent[index] = (byte)value;
        mContent[index + 1] = (byte)(value >> 8);
        mContent[index + 2] = (byte)(value >> 16);
        mContent[index + 3] = (byte)(value >> 24);
        mContent[index + 4] = (byte)(value >> 32);
        mContent[index + 5] = (byte)(value >> 40);
        mContent[index + 6] = (byte)(value >> 48);
        mContent[index + 7] = (byte)(value >> 56);
        return true;
    }

    /**
     * 功  能: 从字节流读取浮点型
     * 参  数: 无
     * 返回值: float
     */
    public float read_float() {
        byte[] p = read(4);
        if (null == p) {
            return 0;
        }
        int r = p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
        return Float.intBitsToFloat(r);
    }

    /**
     * 功  能: 向字节流写入浮点型
     * 参  数: value - 值
     * 返回值: boolean
     */
    public boolean write_float(float value) {
        int index  = writeIndex(4);
        if (-1 == index) {
            return false;
        }
        int w = Float.floatToIntBits(value);
        for (int i = 0; i < 4; ++i) {
            mContent[index + i] = Float.valueOf(w).byteValue();
            w = w >> 8;
        }
        return true;
    }

    /**
     * 功  能: 从字节流读取双精度浮点型
     * 参  数: 无
     * 返回值: double
     */
    public double read_double() {
        byte[] p = read(4);
        if (null == p) {
            return 0;
        }
        long r = (long)p[0] | ((long)p[1] << 8) | ((long)p[2] << 16) | ((long)p[3] << 24)
                | ((long)p[4] << 32) | ((long)p[5] << 40) | ((long)p[6] << 48) | ((long)p[7] << 56);
        return Double.longBitsToDouble(r);
    }

    /**
     * 功  能: 向字节流写入双精度浮点型
     * 参  数: value - 值
     * 返回值: boolean
     */
    public boolean write_double(double value) {
        int index  = writeIndex(8);
        if (-1 == index) {
            return false;
        }
        long w = Double.doubleToLongBits(value);
        for (int i = 0; i < 8; ++i) {
            mContent[index + i] = Long.valueOf(w).byteValue();
            w = w >> 8;
        }
        return true;
    }

    /**
     * 功  能: 从字节流读取字符串
     * 参  数: 无
     * 返回值: String
     */
    public String read_string() {
        int len = read_int();
        if (0 == len) {
            return "";
        }
        byte[] p = read(len);
        if (null == p) {
            return "";
        }
        return Arrays.toString(p);
    }

    /**
     * 功  能: 向字节流写入字符串
     * 参  数: value - 值
     * 返回值: boolean
     */
    public boolean write_string(String str) {
        byte[] bs = str.getBytes();
        if (!write_int(bs.length)) {
            return false;
        }
        int index  = writeIndex(bs.length);
        if (-1 == index) {
            return false;
        }
        System.arraycopy(bs, 0, mContent, index, bs.length);
        return true;
    }

    private boolean copy(final byte[] buf, int n) {
        if (getSpaceLength() < n) {
            return false;
        }
        int index = writeIndex(n);
        if (-1 == index) {
            return false;
        }
        System.arraycopy(buf, 0, mContent, index, n);
        return true;
    }

    private byte[] read(int n) {
        if (mReadIndex + n > mTotalSize) {
            return null;
        }
        byte[] p = new byte[n];
        System.arraycopy(mContent, mReadIndex, p, 0, n);
        mReadIndex += n;
        return p;
    }

    private int writeIndex(int n) {
        if (mWriteIndex + n > mTotalSize) {
            return -1;
        }
        int index = mWriteIndex;
        mWriteIndex += n;
        return index;
    }
}
