package com.jh.utils.videoplayer;

import android.app.Activity;
import android.view.SurfaceView;
import android.view.ViewGroup.LayoutParams;

/**
 * Author:  jaron.ho
 * Date:    2017-02-14
 * Brief:   视频播放器工厂
 */

public class VideoPlayerFactory {
    // 创建视频播放器
    public static VideoPlayer create(Activity activity, int width, int height) {
        SurfaceView surfaceView = new SurfaceView(activity);
        surfaceView.setLayoutParams(new LayoutParams(width, height));
        surfaceView.setZOrderOnTop(true);
        surfaceView.setZOrderMediaOverlay(true);
        return new VideoPlayer(activity, surfaceView, true);
    }

    // 创建全屏视频播放器(结束回调,错误回调)
    public static VideoPlayer createFull(Activity activity, VideoPlayer.CompleteHandler completeHandler, VideoPlayer.ErrorHandler errorHandler) {
        VideoPlayer player = create(activity, LayoutParams.MATCH_PARENT, LayoutParams.MATCH_PARENT);
        player.setCompleteHandler(completeHandler);
        player.setErrorHandler(errorHandler);
        player.setFitType(VideoPlayer.FitType.SHOW_ALL);
        return player;
    }

    // 创建全屏视频播放器(结束回调)
    public static VideoPlayer createFull(Activity activity, VideoPlayer.CompleteHandler completeHandler) {
        return createFull(activity, completeHandler, null);
    }

    // 创建全屏视频播放器
    public static VideoPlayer createFull(Activity activity) {
        return createFull(activity, null, null);
    }
}
