package com.jaronho.sdk.third.okhttpwrap.util;

import java.security.MessageDigest;

/**
 * 加密工具类
 * 支持MD5加密
 */

public class EncryptUtil {
    /**
     * MD5加密：生成16位密文
     * @param originString 加密字符串
     * @param isUpperCase 是否生成大写密文
     */
    public static String MD5StringTo16Bit(String originString,boolean isUpperCase) throws Exception {
        String result = MD5StringTo32Bit(originString,isUpperCase);
        if (32 == result.length()) {
            return result.substring(8,24);
        }
        return "";
    }

    /**
     * MD5加密：生成32位密文
     * @param originString 加密字符串
     * @param isUpperCase 是否生成大写密文
     */
    public static String MD5StringTo32Bit(String originString,boolean isUpperCase) throws Exception {
        String result = "";
        if (null != originString) {
            MessageDigest md = MessageDigest.getInstance("MD5");
            byte bytes[] = md.digest(originString.getBytes());
            for (byte aByte : bytes) {
                String str = Integer.toHexString(aByte & 0xFF);
                if (1 == str.length()) {
                    str += "F";
                }
                result += str;
            }
        }
        if (isUpperCase) {
            return result.toUpperCase();
        } else {
            return result.toLowerCase();
        }
    }
}
