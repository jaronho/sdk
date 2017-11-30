package com.jaronho.sdk.third.okhttpwrap.helper;

import android.os.Message;
import android.os.NetworkOnMainThreadException;
import android.text.TextUtils;

import com.jaronho.sdk.third.okhttpwrap.HttpInfo;
import com.jaronho.sdk.third.okhttpwrap.annotation.RequestMethod;
import com.jaronho.sdk.third.okhttpwrap.bean.CallbackMessage;
import com.jaronho.sdk.third.okhttpwrap.bean.DownloadMessage;
import com.jaronho.sdk.third.okhttpwrap.callback.BaseActivityLifecycleCallbacks;
import com.jaronho.sdk.third.okhttpwrap.callback.CallbackOk;
import com.jaronho.sdk.third.okhttpwrap.callback.ProgressCallback;
import com.jaronho.sdk.third.okhttpwrap.handler.OkMainHandler;
import com.jaronho.sdk.third.okhttpwrap.interceptor.ExceptionInterceptor;
import com.jaronho.sdk.third.okhttpwrap.interceptor.ResultInterceptor;

import java.io.IOException;
import java.net.SocketTimeoutException;
import java.net.UnknownHostException;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import okhttp3.Call;
import okhttp3.Callback;
import okhttp3.FormBody;
import okhttp3.HttpUrl;
import okhttp3.OkHttpClient;
import okhttp3.Request;
import okhttp3.Response;

/**
 * Http网络请求业务类
 */

class HttpHelper extends BaseHelper{
    private List<ResultInterceptor> resultInterceptors;         // 请求结果拦截器
    private List<ExceptionInterceptor> exceptionInterceptors;   // 请求链路异常拦截器

    HttpHelper(HelperInfo helperInfo) {
        super(helperInfo);
        resultInterceptors = helperInfo.getResultInterceptors();
        exceptionInterceptors = helperInfo.getExceptionInterceptors();
    }

    /**
     * 同步请求
     */
    HttpInfo doRequestSync(OkHttpHelper helper) {
        Call call = null;
        final HttpInfo info = helper.getHttpInfo();
        Request request = helper.getRequest();
        String url = info.getUrl();
        if (!checkUrl(url)) {
            return retInfo(info,HttpInfo.CheckURL);
        }
        request = request == null ? buildRequest(info,helper.getRequestMethod()) : request;
        helper.setRequest(request);
        OkHttpClient httpClient = helper.getHttpClient();
        try {
            httpClient = httpClient == null ? super.httpClient : httpClient;
            call = httpClient.newCall(request);
            BaseActivityLifecycleCallbacks.putCall(requestTag,call);
            Response res = call.execute();
            return dealResponse(helper, res, call);
        } catch (IllegalArgumentException e) {
            return retInfo(info,HttpInfo.ProtocolException);
        } catch (SocketTimeoutException e) {
            if (null != e.getMessage()) {
                if (e.getMessage().toLowerCase().contains("failed to connect to")) {
                    return retInfo(info, HttpInfo.ConnectionTimeOut);
                }
                if (e.getMessage().toLowerCase().equals("timeout")) {
                    return retInfo(info, HttpInfo.WriteAndReadTimeOut);
                }
            }
            return retInfo(info,HttpInfo.WriteAndReadTimeOut);
        } catch (UnknownHostException e) {
            return retInfo(info,HttpInfo.CheckURL);
        } catch(NetworkOnMainThreadException e) {
            return retInfo(info,HttpInfo.NetworkOnMainThreadException);
        } catch(Exception e) {
            e.printStackTrace();
            return retInfo(info,HttpInfo.NoResult);
        } finally {
            BaseActivityLifecycleCallbacks.cancel(requestTag,call);
        }
    }

    /**
     * 异步请求
     */
    void doRequestAsync(final OkHttpHelper helper) {
        final HttpInfo info = helper.getHttpInfo();
        final CallbackOk callback = helper.getCallback();
        Request request = helper.getRequest();
        if (null == callback) {
            throw new NullPointerException("CallbackOk is null!");
        }
        String url = info.getUrl();
        if (!checkUrl(url)) {
            // 主线程回调
            Message msg = new CallbackMessage(OkMainHandler.RESPONSE_CALLBACK,
                    callback,
                    retInfo(info,HttpInfo.CheckURL))
                    .build();
            OkMainHandler.getInstance().sendMessage(msg);
            return ;
        }
        Call call = httpClient.newCall(null == request ? buildRequest(info,helper.getRequestMethod()) : request);
        BaseActivityLifecycleCallbacks.putCall(requestTag,call);
        call.enqueue(new Callback() {
            @Override
            public void onFailure(Call call, IOException e) {
                e.printStackTrace();
                // 主线程回调
                Message msg = new CallbackMessage(OkMainHandler.RESPONSE_CALLBACK,
                        callback,
                        retInfo(info,HttpInfo.NoResult,"["+e.getMessage()+"]"))
                        .build();
                OkMainHandler.getInstance().sendMessage(msg);
                BaseActivityLifecycleCallbacks.cancel(requestTag,call);
            }

            @Override
            public void onResponse(Call call, Response res) throws IOException {
                // 主线程回调
                Message msg = new CallbackMessage(OkMainHandler.RESPONSE_CALLBACK,
                        callback,
                        dealResponse(helper,res,call))
                        .build();
                OkMainHandler.getInstance().sendMessage(msg);
                if (!call.isCanceled()) {
                    call.cancel();
                }
                BaseActivityLifecycleCallbacks.cancel(requestTag,call);
            }
        });
    }

    /**
     * 检查请求URL
     */
    private boolean checkUrl(String url) {
        HttpUrl parsed = HttpUrl.parse(url);
        return parsed != null && !TextUtils.isEmpty(url);
    }

    /**
     * 构建Request
     */
    private Request buildRequest(HttpInfo info, @RequestMethod int method) {
        Request request;
        Request.Builder requestBuilder = new Request.Builder();
        final String url = info.getUrl();
        if (RequestMethod.POST == method) {
            FormBody.Builder builder = new FormBody.Builder();
            if (null != info.getParams() && !info.getParams().isEmpty()) {
                StringBuilder log = new StringBuilder("PostParams: ");
                String logInfo;
                String value;
                for (String key : info.getParams().keySet()) {
                    value = info.getParams().get(key);
                    value = value == null ? "" : value;
                    builder.add(key, value);
                    logInfo = key+"="+value+", ";
                    log.append(logInfo);
                }
                showLog(log.toString());
            }
            requestBuilder.url(url).post(builder.build());
        } else if (RequestMethod.GET == method) {
            StringBuilder params = new StringBuilder();
            params.append(url);
            if (null != info.getParams() && !info.getParams().isEmpty()) {
                if (!url.contains("?") && !url.endsWith("?")) {
                    params.append("?");
                }
                String logInfo;
                String value;
                boolean isFirst = params.toString().endsWith("?");
                for (String name : info.getParams().keySet()) {
                    value = info.getParams().get(name);
                    value = value == null ? "" : value;
                    if (isFirst) {
                        logInfo = name + "=" + value;
                        isFirst = false;
                    } else {
                        logInfo = "&" + name + "=" + value;
                    }
                    params.append(logInfo);
                }
            }
            requestBuilder.url(params.toString()).get();
        } else {
            requestBuilder.url(url).get();
        }
        requestBuilder.addHeader("Connection", "close");
        addHeadsToRequest(info,requestBuilder);
        request = requestBuilder.build();
        return request;
    }

    /**
     * 处理HTTP响应
     */
    private HttpInfo dealResponse(OkHttpHelper helper,Response res,Call call) {
        final HttpInfo info = helper.getHttpInfo();
        try {
            if(null != res){
                final int netCode = res.code();
                if (res.isSuccessful()) {
                    if (null == helper.getDownloadFileInfo()) {
                        return retInfo(info,netCode,HttpInfo.SUCCESS,res.body().string());
                    } else { // 下载文件
                        return helper.getDownUpLoadHelper().downloadingFile(helper,res,call);
                    }
                } else {
                    showLog("HttpStatus: "+res.code());
                    if (404 == netCode) {   // 请求页面路径错误
                        return retInfo(info, netCode, HttpInfo.CheckURL);
                    }
                    if (416 == netCode) {   // 请求数据流范围错误
                        return retInfo(info, netCode, HttpInfo.Message, "请求Http数据流范围错误\n" + res.body().string());
                    }
                    if (500 == netCode) {   // 服务器内部错误
                        return retInfo(info, netCode, HttpInfo.NoResult);
                    }
                    if (502 == netCode) {   // 错误网关
                        return retInfo(info, netCode, HttpInfo.CheckNet);
                    }
                    if (504 == netCode) {   // 网关超时
                        return retInfo(info, netCode, HttpInfo.CheckNet);
                    }
                }
            }
            return retInfo(info,HttpInfo.CheckURL);
        } catch (Exception e) {
            e.printStackTrace();
            return retInfo(info,HttpInfo.NoResult);
        } finally {
            if (null != res) {
                res.close();
            }
        }
    }

    HttpInfo retInfo(HttpInfo info, int code) {
        return retInfo(info,code,code,null);
    }

    HttpInfo retInfo(HttpInfo info, int netCode, int code) {
        return retInfo(info,netCode,code,null);
    }

    HttpInfo retInfo(HttpInfo info, int code, String resDetail) {
        return retInfo(info,code,code,resDetail);
    }

    /**
     * 封装请求结果
     */
    HttpInfo retInfo(HttpInfo info, int netCode, int code, String resDetail) {
        info.packInfo(netCode,code,unicodeToString(resDetail));
        // 拦截请求结果
        dealInterceptor(info);
        showLog("Response: "+info.getRetDetail());
        return info;
    }

    /**
     * unicode中文转码
     */
    private String unicodeToString(String str) {
        if(TextUtils.isEmpty(str))
            return "";
        Pattern pattern = Pattern.compile("(\\\\u(\\p{XDigit}{4}))");
        Matcher matcher = pattern.matcher(str);
        char ch;
        while (matcher.find()) {
            ch = (char) Integer.parseInt(matcher.group(2), 16);
            str = str.replace(matcher.group(1), ch + "");
        }
        return str;
    }

    /**
     * 处理拦截器
     */
    private void dealInterceptor(HttpInfo info) {
        try {
            if (info.isSuccessful() && null != resultInterceptors) { //请求结果拦截器
                for (ResultInterceptor interceptor : resultInterceptors) {
                    interceptor.intercept(info);
                }
            } else { //请求链路异常拦截器
                if (null != exceptionInterceptors) {
                    for (ExceptionInterceptor interceptor : exceptionInterceptors) {
                        interceptor.intercept(info);
                    }
                }
            }
        } catch (Exception e) {
            showLog("拦截器处理异常："+e.getMessage());
        }
    }

    /**
     * 请求结果回调
     */
    void responseCallback(HttpInfo info, ProgressCallback progressCallback, int code) {
        // 同步回调
        if (null != progressCallback) {
            progressCallback.onResponseSync(info.getUrl(), info);
        }
        // 异步主线程回调
        Message msg = new DownloadMessage(
                code,
                info.getUrl(),
                info,
                progressCallback)
                .build();
        OkMainHandler.getInstance().sendMessage(msg);
    }

    /**
     * 添加请求头参数
     */
    Request.Builder addHeadsToRequest(HttpInfo info, Request.Builder requestBuilder) {
        if (null != info.getHeads() && !info.getHeads().isEmpty()) {
            for (String key : info.getHeads().keySet()) {
                requestBuilder.addHeader(key,info.getHeads().get(key));
            }
        }
        return requestBuilder;
    }
}
