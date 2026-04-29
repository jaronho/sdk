#include "v4l2_capture.h"

V4L2Capture::~V4L2Capture()
{
    close();
}

bool V4L2Capture::open(const std::string& device, unsigned int width, unsigned int height, unsigned int pixFmt)
{
    close();
    /* step1. 打开设备 */
    m_fd = v4l2_open_device(device.c_str(), &width, &height, pixFmt, &pixFmt);
    if (m_fd <= 0)
    {
        return false;
    }
    m_width = width;
    m_height = height;
    m_pixFmt = pixFmt;
    /* step2. 分配缓冲区 */
    auto bufferCount = v4l2_alloc_buffers(m_fd, 2, &m_buffers, &m_lengths);
    if (bufferCount <= 0)
    {
        v4l2_close_device(m_fd);
        m_fd = -1;
        return false;
    }
    m_bufferCount = bufferCount;
    /* step3. 启动视频流 */
    auto ret = v4l2_stream(m_fd, m_buffers, m_bufferCount, 1);
    if (V4L2_OK != ret)
    {
        v4l2_free_buffers(m_bufferCount, m_buffers, m_lengths);
        m_buffers = nullptr;
        m_lengths = nullptr;
        v4l2_close_device(m_fd);
        m_fd = -1;
        return false;
    }
    m_streaming = true;
    return true;
}

void V4L2Capture::close()
{
    if (m_fd <= 0)
    {
        return;
    }
    /* step1. 停止视频流 */
    if (m_streaming)
    {
        v4l2_stream(m_fd, m_buffers, m_bufferCount, 0);
        m_streaming = false;
    }
    /* step2. 释放缓冲区 */
    if (m_buffers)
    {
        v4l2_free_buffers(m_bufferCount, m_buffers, m_lengths);
        m_buffers = nullptr;
        m_lengths = nullptr;
    }
    /* step3. 关闭设备 */
    v4l2_close_device(m_fd);
    m_fd = -1;
    /* 资源清理 */
    m_bufferCount = 0;
    m_width = 0;
    m_height = 0;
    m_pixFmt = 0;
}

bool V4L2Capture::isOpened() const
{
    return (m_fd > 0);
}

unsigned int V4L2Capture::getWidth() const
{
    return m_width.load();
}

unsigned int V4L2Capture::getHeight() const
{
    return m_height.load();
}

unsigned int V4L2Capture::getPixFmt() const
{
    return m_pixFmt.load();
}

bool V4L2Capture::captureFrame(
    const std::function<void(void* frame, unsigned int width, unsigned int height, unsigned int pixFmt)>& frameCb, int timeout)
{
    if (m_fd <= 0 || !m_streaming)
    {
        return false;
    }
    unsigned int bufIndex = 0;
    /* 取出帧 */
    void* data = v4l2_dqbuf(m_fd, m_buffers, m_bufferCount, &bufIndex, timeout);
    if (!data)
    {
        return false;
    }
    /* 回调通知 */
    if (frameCb)
    {
        frameCb(data, m_width.load(), m_height.load(), m_pixFmt.load());
    }
    /* 归还缓冲区 */
    v4l2_qbuf(m_fd, bufIndex);
    return true;
}
