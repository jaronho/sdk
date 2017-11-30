package com.jaronho.sdk.third.okhttpwrap.handler;

import android.os.Handler;
import android.os.Looper;
import android.os.Message;

import com.jaronho.sdk.third.okhttpwrap.bean.CallbackMessage;
import com.jaronho.sdk.third.okhttpwrap.bean.DownloadMessage;
import com.jaronho.sdk.third.okhttpwrap.bean.ProgressMessage;
import com.jaronho.sdk.third.okhttpwrap.bean.UploadMessage;

/**
 * 主线程Handler
 */
public class OkMainHandler extends Handler {
    public static final int RESPONSE_CALLBACK = 0x01;            // 网络请求回调标识
    public static final int PROGRESS_CALLBACK = 0x02;            // 进度回调标识
    public static final int RESPONSE_UPLOAD_CALLBACK = 0x03;    // 上传结果回调标识
    public static final int RESPONSE_DOWNLOAD_CALLBACK = 0x04;  // 下载结果回调标识

    private static OkMainHandler singleton;

    public static OkMainHandler getInstance(){
        if (null == singleton){
            synchronized (OkMainHandler.class){
                if (null == singleton) {
                    singleton = new OkMainHandler();
                }
            }
        }
        return singleton;
    }

    public OkMainHandler() {
        super(Looper.getMainLooper());
    }

    @Override
    public void handleMessage(Message msg) {
        final int what = msg.what;
        try {
            switch (what){
                case RESPONSE_CALLBACK:
                    CallbackMessage callMsg = (CallbackMessage)msg.obj;
                    if (null != callMsg.callback) {
                        callMsg.callback.onResponse(callMsg.info);
                    }
                    break;
                case PROGRESS_CALLBACK:
                    ProgressMessage proMsg = (ProgressMessage)msg.obj;
                    if (null != proMsg.progressCallback) {
                        proMsg.progressCallback.onProgressMain(proMsg.percent, proMsg.bytesWritten, proMsg.contentLength, proMsg.done);
                    }
                    break;
                case RESPONSE_UPLOAD_CALLBACK:
                    UploadMessage uploadMsg = (UploadMessage)msg.obj;
                    if (null != uploadMsg.progressCallback) {
                        uploadMsg.progressCallback.onResponseMain(uploadMsg.filePath, uploadMsg.info);
                    }
                    break;
                case RESPONSE_DOWNLOAD_CALLBACK:
                    DownloadMessage downloadMsg = (DownloadMessage)msg.obj;
                    if (null != downloadMsg) {
                        downloadMsg.progressCallback.onResponseMain(downloadMsg.filePath, downloadMsg.info);
                    }
                    break;
                default:
                    super.handleMessage(msg);
                    break;
            }
        } catch (Exception e){
        }
    }
}
