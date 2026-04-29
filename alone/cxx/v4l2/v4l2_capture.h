#pragma once
#include <atomic>
#include <functional>
#include <string>

#include "v4l2_device.h"

/**
 * @brief V4L2捕获(非线程安全)
 */
class V4L2Capture
{
public:
    V4L2Capture() = default;
    ~V4L2Capture();

    /**
     * @brief 打开视频设备
     * @param device 设备路径, 如 "/dev/video0"
     * @param width 请求宽度
     * @param height 请求高度
     * @param pixFmt 像素格式, 如: V4L2_PIX_FMT_YUYV, V4L2_PIX_FMT_MJPEG, V4L2_PIX_FMT_RGB24, V4L2_PIX_FMT_BGR24, V4L2_PIX_FMT_YUV420
     * @return true-成功, false-失败
     */
    bool open(const std::string& device, uint32_t width = 640, uint32_t height = 480, uint32_t pixFmt = V4L2_PIX_FMT_YUYV);

    /**
     * @brief 关闭视频设备
     */
    void close();

    /**
     * @brief 是否已打开
     * @return true=已打开
     */
    bool isOpened() const;

    /**
     * @brief 获取宽度
     * @return 宽度
     */
    uint32_t getWidth() const;

    /**
     * @brief 获取高度
     * @return 高度
     */
    uint32_t getHeight() const;

    /**
     * @brief 获像素格式
     * @return 像素格式
     */
    uint32_t getPixFmt() const;

    /**
     * @brief 捕获单帧数据
     * @param frameCb 帧回调函数, 参数: data-帧数据, dataLen-数据长度, width-宽度, height-高度, pixFmt-像素格式
     * @param timeout 超时时间(单位: 毫秒), <=0表示无限等待
     * @return true=成功捕获并回调
     */
    bool captureFrame(const std::function<void(void* data, size_t dataLen, uint32_t width, uint32_t height, uint32_t pixFmt)>& frameCb,
                      int timeout = 100);

private:
    int m_fd = -1; /* 设备描述符 */
    void** m_buffers = nullptr; /* 视频缓冲区 */
    size_t* m_lengths = nullptr; /* 视频缓冲区长度 */
    uint32_t m_bufferCount = 0; /* 视频缓冲区数量 */
    std::atomic_uint32_t m_width{0}; /* 视频宽度 */
    std::atomic_uint32_t m_height{0}; /* 视频高度 */
    std::atomic_uint32_t m_pixFmt{0}; /* 视频像素格式 */
    std::atomic_bool m_streaming{false}; /* 是否正在流式采集 */
};
