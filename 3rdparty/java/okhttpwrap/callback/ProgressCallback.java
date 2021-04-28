package com.jaronho.sdk.third.okhttpwrap.callback;

import com.jaronho.sdk.third.okhttpwrap.HttpInfo;

/**
 * 进度回调
 */

public abstract class ProgressCallback  {
    /**
     * 异步UI线程：返回请求结果
     * @param filePath 文件路径
     * @param info 结果信息类
     */
    public abstract void onResponseMain(String filePath, HttpInfo info);

    /**
     * 同步非UI线程：返回请求结果
     * @param filePath 文件路径
     * @param info 结果信息类
     */
    public abstract void onResponseSync(String filePath, HttpInfo info);

    /**
     * 非UI线程：除了更新ProgressBar进度外不要进行其他UI操作
     * @param percent 已经写入的百分比
     * @param bytesWritten 已经写入的字节数
     * @param contentLength 文件总长度
     * @param done 是否完成即：bytesWritten==contentLength
     */
    public abstract void onProgressAsync(int percent, long bytesWritten, long contentLength, boolean done);

    /**
     * UI线程：可以直接操作UI
     * @param percent 已经写入的百分比
     * @param bytesWritten 已经写入的字节数
     * @param contentLength 文件总长度
     * @param done 是否完成即：bytesWritten==contentLength
     */
    public abstract void onProgressMain(int percent, long bytesWritten, long contentLength, boolean done);
}
