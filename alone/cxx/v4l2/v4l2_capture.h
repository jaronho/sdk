#ifndef V4L2_CAPTURE_H
#define V4L2_CAPTURE_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C"
{
#endif
/* 错误码 */
#define V4L2_OK 0
#define V4L2_ERR_STAT -1
#define V4L2_ERR_NOT_DEV -2
#define V4L2_ERR_OPEN -3
#define V4L2_ERR_QUERYCAP -4
#define V4L2_ERR_NO_CAP -5
#define V4L2_ERR_NO_STREAM -6
#define V4L2_ERR_SET_FMT -8
#define V4L2_ERR_REQBUFS -9
#define V4L2_ERR_NOMEM -10
#define V4L2_ERR_QUERYBUF -11
#define V4L2_ERR_MMAP -12
#define V4L2_ERR_QBUF -13
#define V4L2_ERR_STREAMON -14
#define V4L2_ERR_STREAMOFF -15
#define V4L2_ERR_DQBUF -16

#ifndef v4l2_fourcc
#define v4l2_fourcc(a, b, c, d) ((unsigned int)(a) | ((unsigned int)(b) << 8) | ((unsigned int)(c) << 16) | ((unsigned int)(d) << 24))
#endif

/* 像素格式定义(仅在未包含V4L2头文件时生效, 用于非Linux平台或文档目的) */
#ifndef V4L2_PIX_FMT_YUYV
#define V4L2_PIX_FMT_YUYV ((unsigned int)v4l2_fourcc('Y', 'U', 'Y', 'V'))
#endif
#ifndef V4L2_PIX_FMT_MJPEG
#define V4L2_PIX_FMT_MJPEG ((unsigned int)v4l2_fourcc('M', 'J', 'P', 'G'))
#endif
#ifndef V4L2_PIX_FMT_RGB24
#define V4L2_PIX_FMT_RGB24 ((unsigned int)v4l2_fourcc('R', 'G', '2', '4'))
#endif
#ifndef V4L2_PIX_FMT_BGR24
#define V4L2_PIX_FMT_BGR24 ((unsigned int)v4l2_fourcc('B', 'G', '2', '4'))
#endif
#ifndef V4L2_PIX_FMT_YUV420
#define V4L2_PIX_FMT_YUV420 ((unsigned int)v4l2_fourcc('Y', 'U', '1', '2'))
#endif

    /**
     * @brief 打开视频设备
     * @param devName 设备名, 如 "/dev/video0"
     * @param width [输入/输出]请求宽度(会被修改为实际值)
     * @param height [输入/输出]请求高度(会被修改为实际值)
     * @param reqFmt 请求的像素格式, 如: V4L2_PIX_FMT_YUYV
     * @param actualFmt [输出]实际使用的像素格式, 可为 NULL
     * @return >0=设备描述符, <=0=错误码
     */
    int v4l2_open_device(const char* devName, unsigned int* width, unsigned int* height, unsigned int reqFmt, unsigned int* actualFmt);

    /**
     * @brief 分配并映射V4L2缓冲区
     * @param fd 设备描述符
     * @param bufferCount 请求缓冲区数量(建议2-4)
     * @param outBuffers [输出]缓冲区指针数组
     * @param outLengths [输出]缓冲区大小数组
     * @return >0=实际分配的缓冲区数量, <=0=错误码
     */
    int v4l2_alloc_buffers(int fd, unsigned int bufferCount, void*** outBuffers, size_t** outLengths);

    /**
     * @brief 释放V4L2缓冲区
     * @param bufferCount 缓冲区数量
     * @param buffers 缓冲区指针数组
     * @param lengths 缓冲区大小数组
     */
    void v4l2_free_buffers(unsigned int bufferCount, void** buffers, size_t* lengths);

    /**
     * @brief 关闭设备
     * @param fd 设备描述符
     */
    void v4l2_close_device(int fd);

    /**
     * @brief 启动/停止视频流
     * @param fd 设备描述符
     * @param buffers 缓冲区数组
     * @param bufferCount 缓冲区数量
     * @param enable 1=启动, 0=停止
     * @return V4L2_OK=成功, <0=错误码
     */
    int v4l2_stream(int fd, void** buffers, unsigned int bufferCount, int enable);

    /**
     * @brief 捕获一帧(带超时)
     * @param fd 设备描述符
     * @param buffers 缓冲区数组
     * @param bufferCount 缓冲区数量
     * @param outBufIndex [输出]缓冲区索引(用于后续归还)
     * @param timeoutMs 超时毫秒, -1=无限等待
     * @return 指向帧数据的指针(mmap地址, 不要free), NULL=失败或超时
     */
    void* v4l2_dqbuf(int fd, void** buffers, unsigned int bufferCount, unsigned int* outBufIndex, int timeoutMs);

    /**
     * @brief 归还缓冲区到驱动队列
     * @param fd 设备描述符
     * @param bufIndex 缓冲区索引
     * @return 1=成功, 0=失败
     */
    int v4l2_qbuf(int fd, unsigned int bufIndex);

    /**
     * @brief 查询设备信息
     * @param fd 设备描述符
     */
    void v4l2_print_info(int fd);

    /**
     * @brief YUYV转RGB24
     * @param yuyv YUYV数据
     * @param rgb [输出]RGB缓冲区(调用方分配 width*height*3 字节)
     * @param width 图像宽度
     * @param height 图像高度
     */
    void v4l2_yuyv_to_rgb24(const unsigned char* yuyv, unsigned char* rgb, int width, int height);
#ifdef __cplusplus
}
#endif
#endif /* V4L2_CAPTURE_H */

/**
int main(int argc, char* argv[])
{
    const char* device = (argc > 1) ? argv[1] : "/dev/video0";
    unsigned int width = 640;
    unsigned int height = 480;
    unsigned int actualFmt = 0;
    int fd = -1;
    void** buffers = NULL;
    size_t* lengths = NULL;
    int bufferCount = 0;
    int i;
    // ========== 1. 打开设备 ==========
    printf("=== Step 1: Open device ===\n");
    fd = v4l2_open_device(device, &width, &height, V4L2_PIX_FMT_YUYV, &actualFmt);
    if (fd <= 0)
    {
        printf("Failed to open device, error: %d\n", fd);
        return -1;
    }
    printf("Device opened: fd=%d, resolution=%dx%d\n", fd, width, height);
    // 打印设备信息
    v4l2_print_info(fd);
    // ========== 2. 分配缓冲区 ==========
    printf("\n=== Step 2: Allocate buffers ===\n");
    bufferCount = v4l2_alloc_buffers(fd, 2, &buffers, &lengths);  // 申请2个缓冲区
    if (bufferCount <= 0)
    {
        printf("Failed to allocate buffers, error: %d\n", bufferCount);
        v4l2_close_device(fd);
        return -1;
    }
    printf("Allocated %d buffers\n", bufferCount);
    // ========== 3. 启动视频流 ==========
    printf("\n=== Step 3: Start streaming ===\n");
    int ret = v4l2_stream(fd, buffers, bufferCount, 1);
    if (ret != V4L2_OK)
    {
        printf("Failed to start streaming, error: %d\n", ret);
        v4l2_free_buffers( bufferCount, buffers, lengths);
        v4l2_close_device(fd);
        return -1;
    }
    printf("Streaming started\n");
    // ========== 4. 捕获图像 ==========
    printf("\n=== Step 4: Capture frames (press Ctrl+C to stop) ===\n");
    // 分配 RGB 缓冲区
    unsigned char* rgb = (unsigned char*)malloc(width * height * 3);
    if (!rgb)
    {
        printf("Failed to allocate RGB buffer\n");
        goto cleanup;
    }
    for (i = 0; i < 100; ++i)  // 捕获100帧
    {
        unsigned int bufIndex = 0;
        void* frame = v4l2_dqbuf(fd, buffers, bufferCount, &bufIndex, 1000);  // 1秒超时
        if (!frame)
        {
            printf("Frame %d: timeout or error\n", i);
            continue;
        }
        printf("Frame %d: captured, buffer index=%u\n", i, bufIndex);
        // 如果是 YUYV 格式，转换为 RGB
        if (actualFmt == V4L2_PIX_FMT_YUYV)
        {
            v4l2_yuyv_to_rgb24((const unsigned char*)frame, rgb, width, height);
            // 此时 rgb 中就是 RGB24 数据，可以保存为 BMP 或显示
        }
        // ========== 关键：归还缓冲区 ==========
        if (!v4l2_qbuf(fd, bufIndex))
        {
            printf("Failed to queue buffer back\n");
            break;
        }
        usleep(33000);  // 约30 FPS
    }
    free(rgb);
    // ========== 5. 清理 ==========
cleanup:
    printf("\n=== Step 5: Cleanup ===\n");
    // 停止流
    v4l2_stream(fd, buffers, bufferCount, 0);
    printf("Streaming stopped\n");
    // 释放缓冲区
    v4l2_free_buffers(bufferCount, buffers, lengths);
    printf("Buffers freed\n");
    // 关闭设备
    v4l2_close_device(fd);
    printf("Device closed\n");
    return 0;
}
 */
