package com.jaronho.sdk.third.okhttpwrap.interceptor;

import com.jaronho.sdk.third.okhttpwrap.HttpInfo;

/**
 * 请求结果拦截器
 */
public interface ResultInterceptor {
    HttpInfo intercept(HttpInfo info) throws Exception;
}
