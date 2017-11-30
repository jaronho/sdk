package com.jaronho.sdk.third.okhttpwrap.bean;

import com.jaronho.sdk.third.okhttpwrap.annotation.DownloadStatus;
import com.jaronho.sdk.third.okhttpwrap.callback.ProgressCallback;

/**
 * 下载文件信息体
 */

public class DownloadFileInfo {
    private String url;             // 下载文件接口地址
    private String saveFileDir;     // 文件保存目录
    private String saveFileName;    // 文件保存名称
    private ProgressCallback progressCallback;  // 下载进度回调接口
    private String downloadStatus = DownloadStatus.INIT;    // 下载状态
    private long completedSize; // 已下载字节数
    private String saveFileNameWithExtension;// 保存文件名称: 包含扩展名
    private String saveFileNameCopy;    // 保存文件备用名称: 用于文件名称冲突
    private String saveFileNameEncrypt; // 保存文件名称(加密后)

    public DownloadFileInfo(String url, String saveFileName, ProgressCallback progressCallback) {
        this.url = url;
        this.saveFileName = saveFileName;
        this.progressCallback = progressCallback;
    }

    public DownloadFileInfo(String url, String saveFileDir, String saveFileName, ProgressCallback progressCallback) {
        this.url = url;
        this.saveFileDir = saveFileDir;
        this.saveFileName = saveFileName;
        this.progressCallback = progressCallback;
    }

    public ProgressCallback getProgressCallback() {
        return progressCallback;
    }

    public void setProgressCallback(ProgressCallback progressCallback) {
        this.progressCallback = progressCallback;
    }

    public String getUrl() {
        return url;
    }

    public void setUrl(String url) {
        this.url = url;
    }

    public String getSaveFileDir() {
        return saveFileDir;
    }

    public void setSaveFileDir(String saveFileDir) {
        this.saveFileDir = saveFileDir;
    }

    public String getSaveFileName() {
        return saveFileName;
    }

    public void setSaveFileName(String saveFileName) {
        this.saveFileName = saveFileName;
    }

    public String getDownloadStatus() {
        return downloadStatus;
    }

    public void setDownloadStatus(@DownloadStatus String downloadStatus) {
        this.downloadStatus = downloadStatus;
    }

    public long getCompletedSize() {
        return completedSize;
    }

    public void setCompletedSize(long completedSize) {
        this.completedSize = completedSize;
    }


    public String getSaveFileNameWithExtension() {
        return saveFileNameWithExtension;
    }

    public void setSaveFileNameWithExtension(String saveFileNameWithExtension) {
        this.saveFileNameWithExtension = saveFileNameWithExtension;
    }

    public String getSaveFileNameCopy() {
        return saveFileNameCopy;
    }

    public void setSaveFileNameCopy(String saveFileNameCopy) {
        this.saveFileNameCopy = saveFileNameCopy;
    }

    public String getSaveFileNameEncrypt() {
        return saveFileNameEncrypt;
    }

    public void setSaveFileNameEncrypt(String saveFileNameEncrypt) {
        this.saveFileNameEncrypt = saveFileNameEncrypt;
    }
}
