package com.jaronho.sdk.third.okhttpwrap.annotation;

import android.support.annotation.IntDef;

import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;

/**
 * 请求方法
 */

@IntDef({RequestMethod.POST,RequestMethod.GET})
@Retention(RetentionPolicy.SOURCE)
public @interface RequestMethod {
    int POST = 1;
    int GET = 2;
}