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
    public static VideoPlayer createPlayer(Activity activity, int width, int height) {
        SurfaceView surfaceView = new SurfaceView(activity);
        surfaceView.setLayoutParams(new LayoutParams(width, height));
        surfaceView.setZOrderOnTop(true);
        surfaceView.setZOrderMediaOverlay(true);
        return new VideoPlayer(activity, surfaceView, true);
    }

    // 创建全屏视频播放器(结束回调,错误回调)
    public static VideoPlayer createPlayerFull(Activity activity, VideoPlayer.CompleteHandler completeHandler, VideoPlayer.ErrorHandler errorHandler) {
        VideoPlayer player = createPlayer(activity, LayoutParams.MATCH_PARENT, LayoutParams.MATCH_PARENT);
        player.setCompleteHandler(completeHandler);
        player.setErrorHandler(errorHandler);
        return player;
    }

    // 创建全屏视频播放器(结束回调)
    public static VideoPlayer createPlayerFull(Activity activity, VideoPlayer.CompleteHandler completeHandler) {
        return createPlayerFull(activity, completeHandler, null);
    }

    // 创建全屏视频播放器
    public static VideoPlayer createPlayerFull(Activity activity) {
        return createPlayerFull(activity, null, null);
    }
}
