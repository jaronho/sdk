package com.jaronho.sdk.third.okhttpwrap.bean;

import com.jaronho.sdk.third.okhttpwrap.HttpInfo;
import com.jaronho.sdk.third.okhttpwrap.callback.CallbackOk;

/**
 * 响应回调信息体
 */

public class CallbackMessage extends OkMessage{
    public CallbackOk callback;
    public HttpInfo info;

    public CallbackMessage(int what, CallbackOk callback, HttpInfo info) {
        this.what = what;
        this.callback = callback;
        this.info = info;
    }
}
