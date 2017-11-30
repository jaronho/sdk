package com.jaronho.sdk.utils;

import android.annotation.TargetApi;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.graphics.Color;
import android.graphics.drawable.AnimationDrawable;
import android.net.Uri;
import android.os.Build;
import android.os.SystemClock;
import android.provider.Browser;
import android.support.v4.content.ContextCompat;
import android.text.Html;
import android.text.SpannableStringBuilder;
import android.text.Spanned;
import android.text.TextPaint;
import android.text.method.LinkMovementMethod;
import android.text.style.ClickableSpan;
import android.text.style.URLSpan;
import android.util.Log;
import android.view.SurfaceView;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;
import android.widget.Toast;

import com.jaronho.sdk.library.timer.Timer;
import com.jaronho.sdk.library.timer.TimerManager;
import com.jaronho.sdk.utils.view.VideoPlayer;

import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * Author:  jaron.ho
 * Date:    2017-02-08
 * Brief:   视图工具集
 */

public final class UtilView {
    private static long mCostStartTime = 0;         // 耗时计算开始时间(毫秒)
    private static Toast mToast = null;             // 提示组件

    /**
     * 功  能: 耗时开始,与接口costTimeCalc配对使用
     * 参  数: 无
     * 返回值: 无
     */
    public static void costTimeBegin() {
        mCostStartTime = SystemClock.elapsedRealtime();
    }

    /**
     * 功  能: 耗时计算,与接口costTimeBegin配对使用
     * 参  数: resetFlag - 耗时是否复位
     *         tag - 打印信息的标签,当为null或为空字符串时,不打印
     *         header - 打印信息的头部,当为null或为空字符串时,不打印
     * 返回值: long,耗时(毫秒)
     */
    public static long costTimeCalc(boolean resetFlag, String tag, String header) {
        long deltaTime = 0;
        if (mCostStartTime > 0) {
            long currentTime = SystemClock.elapsedRealtime();
            deltaTime = currentTime - mCostStartTime;
            mCostStartTime = resetFlag ? 0 : currentTime;
            if (null != tag && tag.length() > 0 && null != header && header.length() > 0) {
                Log.d(tag, "[" + header + "] cost time " + deltaTime + " millisconds");
            }
        }
        return deltaTime;
    }

    /**
     * 功  能: 创建帧动画
     * 参  数: activity - 活动
     *         frameIds - 帧图片资源id
     *         durations - 每帧时间间隔(毫秒)
     *         isLoop - 是否循环播放
     * 返回值: AnimationDrawable
     */
    public static AnimationDrawable createFrameDrawable(Activity activity, int[] frameIds, int[] durations, boolean isLoop) {
        AnimationDrawable ani = new AnimationDrawable();
        for (int i = 0, len = frameIds.length; i < len; ++i) {
            ani.addFrame(ContextCompat.getDrawable(activity, frameIds[i]), durations[i]);
        }
        ani.setOneShot(!isLoop);
        return ani;
    }

    /**
     * 功  能: 创建帧动画
     * 参  数: activity - 活动
     *         frameIds - 帧图片资源id
     *         duration - 每帧时间间隔(毫秒)
     *         isLoop - 是否循环播放
     * 返回值: AnimationDrawable
     */
    public static AnimationDrawable createFrameDrawable(Activity activity, int[] frameIds, int duration, boolean isLoop) {
        AnimationDrawable ani = new AnimationDrawable();
        for (int frameId : frameIds) {
            ani.addFrame(ContextCompat.getDrawable(activity, frameId), duration);
        }
        ani.setOneShot(!isLoop);
        return ani;
    }

    /**
     * 功  能: 开始闪烁
     * 参  数: view - 视图
     *         showTime - 显示时间(毫秒)
     *         hideTime - 隐藏时间(毫秒)
     *         count - 闪烁次数,<=0 时一直闪烁
     *         handler - 闪烁结束回调
     * 返回值: 无
     */
    @TargetApi(Build.VERSION_CODES.JELLY_BEAN_MR1)
    public static void startBlink(final View view, final int showTime, final int hideTime, final int count, final Timer.OverHandler handler) {
        if (null != view) {
            if (showTime <= 0) {
                view.setVisibility(View.INVISIBLE);
                return;
            }
            if (View.NO_ID == view.getId()) {
                view.setId(View.generateViewId());
            }
            view.setVisibility(View.VISIBLE);
            if (hideTime > 0) {
                TimerManager.getInstance().run(showTime, count * 2, new Timer.RunHandler() {
                    private boolean mShowFlag = true;
                    @Override
                    public void onCallback(Timer tm, int runCount, Object param) {
                        mShowFlag = !mShowFlag;
                        tm.setInterval(mShowFlag ? showTime : hideTime);
                        view.setVisibility(mShowFlag ? View.VISIBLE : (hideTime > 0 ? View.INVISIBLE : View.VISIBLE));
                    }
                }, handler, null, false, "DefaultTimerBlink_" + view.getId());
            } else if (count > 0) {
                TimerManager.getInstance().run(showTime * count, 1, handler, "DefaultTimerBlink_" + view.getId());
            }
        }
    }

    /**
     * 功  能: 开始闪烁
     * 参  数: view - 视图
     *         showTime - 显示时间(毫秒)
     *         hideTime - 隐藏时间(毫秒)
     *         count - 闪烁次数,<=0 时一直闪烁
     *         showAtLast - 当闪烁次数到达后,是否显示(true:显示,false:隐藏)
     * 返回值: 无
     */
    public static void startBlink(final View view, final int showTime, final int hideTime, final int count, final boolean showAtLast) {
        startBlink(view, showTime, hideTime, count, new Timer.OverHandler() {
            @Override
            public void onCallback(Timer tm, Object param) {
                view.setVisibility(showAtLast ? View.VISIBLE : View.INVISIBLE);
            }
        });
    }

    /**
     * 功  能: 停止闪烁
     * 参  数: view - 视图
     *         showAtLast - 是否显示(true:显示,false:隐藏)
     * 返回值: 无
     */
    public static void stopBlink(View view, boolean showAtLast) {
        if (null != view) {
            if (View.NO_ID != view.getId()) {
                TimerManager.getInstance().stop("DefaultTimerBlink_" + view.getId(), false);
            }
            view.setVisibility(showAtLast ? View.VISIBLE : View.INVISIBLE);
        }
    }

    /**
     * 功  能: 显示提示文本
     * 参  数: context - 上下文
     *         msg - 提示信息
     * 返回值: 无
     */
    public static void showToast(Context context, String msg) {
        if (null == context || null == msg || msg.isEmpty()) {
            return;
        }
        if (null == mToast) {
            mToast = Toast.makeText(context, msg, Toast.LENGTH_SHORT);
        } else {
            mToast.setText(msg);
        }
        mToast.show();
    }

    /**
     * 功  能: 显示提示文本
     * 参  数: context - 上下文
     *         resId - 资源id
     * 返回值: 无
     */
    public static void showToast(Context context, int resId) {
        if (null != context) {
            showToast(context, context.getString(resId));
        }
    }

    /**
     * 功  能: 设置链接跳转与高亮样式
     * 参  数: url - 链接
     *         color - 链接颜色
     *         underline - 是否显示下划线
     * 返回值: ClickableSpan
     */
    public static ClickableSpan createClickableSpan(final String url, final int color, final boolean underline) {
        return new ClickableSpan() {
            @Override
            public void onClick(View widget) {
                Uri uri = Uri.parse(url);
                Context context = widget.getContext();
                Intent intent = new Intent(Intent.ACTION_VIEW, uri);
                intent.putExtra(Browser.EXTRA_APPLICATION_ID, context.getPackageName());
                context.startActivity(intent);
            }

            @Override
            public void updateDrawState(TextPaint ds) {
                ds.setColor(color);
                ds.setUnderlineText(underline);
            }
        };
    }

    /**
     * 功  能: 让TextView自动解析URL并高亮设置点击链接(链接不支持中文)
     * 参  数: tv - 文本控件
     *         content - 内容
     * 返回值: TextView
     */
    public static TextView setTextViewContent(TextView tv, String content) {
        SpannableStringBuilder sp = new SpannableStringBuilder(content);
        // 又碰上一个坑,Android居然还不支持POSIX字符,在Android中的\p{Alnum}和Java中的\p{Alnum}不是同一个值,非得要换成[a-zA-Z0-9]才行
        Pattern pattern = Pattern.compile("(http|https|ftp|svn)://([a-zA-Z0-9]+[/?.?])" + "+[a-zA-Z0-9]*\\??([a-zA-Z0-9]*=[a-zA-Z0-9]*&?)*");
        Matcher matcher = pattern.matcher(content);
        while (matcher.find()) {
            String url = matcher.group();
            int start = content.indexOf(url);
            if (start >= 0) {
                int end = start + url.length();
                sp.setSpan(new URLSpan(url), start, end, Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
                sp.setSpan(createClickableSpan(url, Color.BLUE, false), start, end, Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
            }
        }
        tv.setText(sp);
        tv.setMovementMethod(LinkMovementMethod.getInstance());
        return tv;
    }

    /**
     * 功  能: 处理html数据的高亮与响应
     * 参  数: tv - 文本控件
     *         content - 内容
     * 返回值: TextView
     */
    public static TextView setTextViewHtml(TextView tv, String content) {
        SpannableStringBuilder sp = new SpannableStringBuilder(Html.fromHtml(content));
        URLSpan[] urlSpans = sp.getSpans(0, sp.length(), URLSpan.class);
        for (final URLSpan span : urlSpans) {
            int start = sp.getSpanStart(span);
            int end = sp.getSpanEnd(span);
            sp.setSpan(createClickableSpan(span.getURL(), Color.BLUE, false), start, end, Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
        }
        tv.setText(sp);
        tv.setMovementMethod(LinkMovementMethod.getInstance());
        return tv;
    }

    /**
     * 功  能: 创建视频播放器(默认大小)
     * 参  数: activity - 活动
     * 返回值: VideoPlayer
     */
    public static VideoPlayer createVideoPlayer(Activity activity) {
        SurfaceView surfaceView = new SurfaceView(activity);
        surfaceView.setZOrderOnTop(true);
        surfaceView.setZOrderMediaOverlay(true);
        return new VideoPlayer(activity, surfaceView, true);
    }

    /**
     * 功  能: 创建视频播放器(固定宽高)
     * 参  数: activity - 活动
     *         width - 宽度
     *         height - 高度
     * 返回值: VideoPlayer
     */
    public static VideoPlayer createVideoPlayer(Activity activity, int width, int height) {
        VideoPlayer player = createVideoPlayer(activity);
        player.setFitType(VideoPlayer.FitType.FIXED_SIZE);
        player.getView().setLayoutParams(new ViewGroup.LayoutParams(width, height));
        return player;
    }
}
