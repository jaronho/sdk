#include "v4l2_capture.h"

#include <cerrno>

V4L2Capture::~V4L2Capture()
{
    close();
}

bool V4L2Capture::open(const std::string& device, uint32_t width, uint32_t height, uint32_t pixFmt)
{
    close();
    m_deviceGone = false;
    /* 打开过程中任一环节失败时统一走这里释放缓冲区和句柄 */
    auto cleanup = [&]() {
        if (m_fd > 0)
        {
            v4l2_close_device(m_fd); /* 先关fd, 确保内核已停止数据流并归还缓冲区 */
        }
        m_fd = -1;
        if (m_buffers)
        {
            v4l2_free_buffers(m_bufferCount, m_buffers, m_lengths);
            m_buffers = nullptr;
            m_lengths = nullptr;
        }
        m_bufferCount = 0;
    };
    /* step0. 前置检查: 摄像头拔出后节点会被内核摘除, 节点不存在时不能对该设备发起任何ioctl */
    if (V4L2_OK != v4l2_device_node_exist(device.c_str()))
    {
        m_deviceGone = true; /* 明确标记为设备已拔出, 便于上层区分"设备不存在"与"其它打开失败" */
        return false;
    }
    /* step1. 打开设备 */
    m_fd = v4l2_open_device(device.c_str(), &width, &height, pixFmt, &pixFmt);
    if (m_fd <= 0)
    {
        m_fd = -1;
        return false;
    }
    m_width = width;
    m_height = height;
    m_pixFmt = pixFmt;
    /* step2. 分配缓冲区 */
    auto bufferCount = v4l2_alloc_buffers(m_fd, 2, &m_buffers, &m_lengths);
    if (bufferCount <= 0)
    {
        cleanup();
        return false;
    }
    m_bufferCount = bufferCount;
    /* step2.5. STREAMON前再次确认设备节点仍存在, 收窄"节点已摘除但仍发起STREAMON"的竞态窗口 */
    if (V4L2_OK != v4l2_device_node_exist(device.c_str()))
    {
        m_deviceGone = true;
        cleanup();
        return false;
    }
    /* step3. 启动视频流 */
    auto ret = v4l2_stream(m_fd, m_buffers, m_bufferCount, 1);
    if (V4L2_OK != ret)
    {
        cleanup();
        return false;
    }
    m_streaming = true;
    m_deviceGone = false;
    return true;
}

void V4L2Capture::close()
{
    if (m_fd <= 0)
    {
        return;
    }
    /* step1. 关闭设备: 这里刻意不再主动发送STREAMOFF, 原因如下:
       1) 内核释放fd时走vb2_core_queue_release -> __vb2_queue_cancel, 与STREAMOFF内部调用的是同一个函数,
          同样会调用驱动的stop_streaming并归还全部缓冲区, 即close(fd)本身已等价于STREAMOFF, 并非必需;
       2) STREAMOFF在设备已拔出时可能永久阻塞甚至触发内核异常, 而"设备是否真的没了"在应用层无法可靠判定
          (poll返回TIMEOUT不代表设备还在, 设备节点摘除和udev事件也都有滞后窗口),
          任何基于这些判据的取舍都只是缩小窗口而非消除;
       3) fd无论如何都要关闭, 与其多发一次危险的可选ioctl, 不如直接依赖内核的释放路径 */
    v4l2_close_device(m_fd);
    m_fd = -1;
    m_streaming = false;
    /* step2. 释放缓冲区(放在close之后, 确保内核已停止数据流并归还所有缓冲区) */
    if (m_buffers)
    {
        v4l2_free_buffers(m_bufferCount, m_buffers, m_lengths);
        m_buffers = nullptr;
        m_lengths = nullptr;
    }
    /* 资源清理 */
    m_bufferCount = 0;
    m_width = 0;
    m_height = 0;
    m_pixFmt = 0;
    m_deviceGone = false;
}

bool V4L2Capture::isOpened() const
{
    return (m_fd > 0);
}

uint32_t V4L2Capture::getWidth() const
{
    return m_width.load();
}

uint32_t V4L2Capture::getHeight() const
{
    return m_height.load();
}

uint32_t V4L2Capture::getPixFmt() const
{
    return m_pixFmt.load();
}

bool V4L2Capture::isDeviceGone() const
{
    return m_deviceGone.load();
}

V4L2CaptureResult V4L2Capture::captureFrame(
    const std::function<void(const void* frame, size_t dataLen, uint32_t width, uint32_t height, uint32_t pixFmt)>& frameCb, int timeout)
{
    if (m_fd <= 0 || !m_streaming)
    {
        return V4L2CaptureResult::Error;
    }
    uint32_t bufIndex = 0;
    errno = 0;
    /* 取出帧 */
    const void* data = v4l2_dqbuf(m_fd, m_buffers, m_bufferCount, &bufIndex, timeout);
    if (!data)
    {
        /* EAGAIN=超时暂无数据, ENODEV=设备已拔出, 其余为其它错误 */
        if (EAGAIN == errno)
        {
            return V4L2CaptureResult::Timeout;
        }
        if (ENODEV == errno)
        {
            m_deviceGone = true;
            return V4L2CaptureResult::DeviceGone;
        }
        return V4L2CaptureResult::Error;
    }
    size_t dataLen = m_lengths[bufIndex];
    /* 回调通知 */
    if (frameCb)
    {
        frameCb(data, dataLen, m_width.load(), m_height.load(), m_pixFmt.load());
    }
    /* 归还缓冲区 */
    if (!v4l2_qbuf(m_fd, bufIndex))
    {
        /* 归还失败: v4l2_qbuf已把"设备已拔出"类errno归一化为ENODEV, 未归一化到的再复查一次设备状态 */
        if (ENODEV == errno || V4L2_ERR_DEV_GONE == v4l2_check_device(m_fd, 0))
        {
            m_deviceGone = true;
            return V4L2CaptureResult::DeviceGone;
        }
        return V4L2CaptureResult::Error;
    }
    return V4L2CaptureResult::Ok;
}
