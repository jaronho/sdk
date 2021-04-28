package com.jaronho.sdk.third.okhttpwrap.bean;

import android.os.Message;

import java.io.Serializable;

/**
 * Handler信息体基类
 */

public class OkMessage implements Serializable {
    public int what;

    public Message build(){
        Message msg = new Message();
        msg.what = this.what;
        msg.obj = this;
        return msg;
    }
}
