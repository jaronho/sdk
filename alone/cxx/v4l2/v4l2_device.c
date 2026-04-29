#include "v4l2_device.h"

#include <errno.h>
#include <fcntl.h>
#ifdef __linux__
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

int v4l2_open_device(const char* devName, unsigned int* width, unsigned int* height, unsigned int reqFmt, unsigned int* actualFmt)
{
#ifdef __linux__
    int fd;
    struct stat st;
    struct v4l2_capability cap;
    struct v4l2_format format;
    if (!devName || strlen(devName) <= 0 || !width || 0 == *width || !height || 0 == *height)
    {
        return V4L2_ERR_STAT;
    }
    /* step1: 检查并打开设备 */
    if (-1 == stat(devName, &st))
    {
        printf("Cannot identify '%s': %d, %s\n", devName, errno, strerror(errno));
        return V4L2_ERR_STAT;
    }
    if (!S_ISCHR(st.st_mode))
    {
        printf("'%s' is not a device\n", devName);
        return V4L2_ERR_NOT_DEV;
    }
    fd = open(devName, O_RDWR | O_NONBLOCK, 0);
    if (-1 == fd)
    {
        printf("Cannot open '%s': %d, %s\n", devName, errno, strerror(errno));
        return V4L2_ERR_OPEN;
    }
    /* step2: 查询能力 */
    memset(&cap, 0, sizeof(cap));
    if (-1 == ioctl(fd, VIDIOC_QUERYCAP, &cap))
    {
        printf("VIDIOC_QUERYCAP error %d, %s\n", errno, strerror(errno));
        close(fd);
        return V4L2_ERR_QUERYCAP;
    }
    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
    {
        printf("'%s' is no video capture device\n", devName);
        close(fd);
        return V4L2_ERR_NO_CAP;
    }
    if (!(cap.capabilities & V4L2_CAP_STREAMING))
    {
        printf("'%s' does not support streaming i/o\n", devName);
        close(fd);
        return V4L2_ERR_NO_STREAM;
    }
    /* step3: 设置格式(优先用户请求, 否则用YUYV) */
    memset(&format, 0, sizeof(format));
    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    format.fmt.pix.width = *width;
    format.fmt.pix.height = *height;
    format.fmt.pix.pixelformat = reqFmt ? reqFmt : V4L2_PIX_FMT_YUYV;
    format.fmt.pix.field = V4L2_FIELD_NONE;
    if (-1 == ioctl(fd, VIDIOC_S_FMT, &format))
    {
        printf("VIDIOC_S_FMT error %d, %s\n", errno, strerror(errno));
        close(fd);
        return V4L2_ERR_SET_FMT;
    }
    *width = format.fmt.pix.width;
    *height = format.fmt.pix.height;
    if (actualFmt)
    {
        *actualFmt = format.fmt.pix.pixelformat;
    }
    printf("V4L2 format set: %dx%d, fourcc=%c%c%c%c\n", *width, *height, (format.fmt.pix.pixelformat >> 0) & 0xFF,
           (format.fmt.pix.pixelformat >> 8) & 0xFF, (format.fmt.pix.pixelformat >> 16) & 0xFF, (format.fmt.pix.pixelformat >> 24) & 0xFF);
    return fd;
#else
    return V4L2_ERR_STAT;
#endif
}

int v4l2_alloc_buffers(int fd, unsigned int bufferCount, void*** outBuffers, size_t** outLengths)
{
#ifdef __linux__
    struct v4l2_requestbuffers reqbuf;
    struct v4l2_buffer buf;
    unsigned int i;
    void** buffers;
    size_t* lengths;
    unsigned int actualCount;
    if (fd <= 0 || bufferCount < 2 || !outBuffers || !outLengths)
    {
        return V4L2_ERR_STAT;
    }
    memset(&reqbuf, 0, sizeof(reqbuf));
    reqbuf.count = bufferCount;
    reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    reqbuf.memory = V4L2_MEMORY_MMAP;
    if (-1 == ioctl(fd, VIDIOC_REQBUFS, &reqbuf))
    {
        printf("VIDIOC_REQBUFS error %d, %s\n", errno, strerror(errno));
        return V4L2_ERR_REQBUFS;
    }
    actualCount = reqbuf.count;
    if (actualCount < bufferCount)
    {
        printf("Insufficient buffer memory, requested %u, got %u\n", bufferCount, actualCount);
    }
    if (actualCount < 2)
    {
        printf("Driver only provided %u buffers, need at least 2\n", actualCount);
        return V4L2_ERR_REQBUFS;
    }
    buffers = (void**)calloc(actualCount, sizeof(void*));
    lengths = (size_t*)calloc(actualCount, sizeof(size_t));
    if (!buffers || !lengths)
    {
        free(buffers);
        free(lengths);
        return V4L2_ERR_NOMEM;
    }
    for (i = 0; i < actualCount; ++i)
    {
        memset(&buf, 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;
        if (-1 == ioctl(fd, VIDIOC_QUERYBUF, &buf))
        {
            printf("VIDIOC_QUERYBUF error %d, %s\n", errno, strerror(errno));
            for (unsigned int j = 0; j < i; ++j)
            {
                if (buffers[j] && lengths[j] > 0)
                    munmap(buffers[j], lengths[j]);
            }
            free(buffers);
            free(lengths);
            return V4L2_ERR_QUERYBUF;
        }
        buffers[i] = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);
        if (MAP_FAILED == buffers[i])
        {
            printf("mmap error %d, %s\n", errno, strerror(errno));
            for (unsigned int j = 0; j < i; ++j)
            {
                if (buffers[j] && lengths[j] > 0)
                    munmap(buffers[j], lengths[j]);
            }
            free(buffers);
            free(lengths);
            return V4L2_ERR_MMAP;
        }
        lengths[i] = buf.length;
    }
    *outBuffers = buffers;
    *outLengths = lengths; /* ← 修复：解引用赋值 */
    return (int)actualCount;
#else
    return V4L2_ERR_STAT;
#endif
}

void v4l2_free_buffers(unsigned int bufferCount, void** buffers, size_t* lengths)
{
#ifdef __linux__
    unsigned int i;
    if (!buffers || !lengths)
    {
        return;
    }
    for (i = 0; i < bufferCount; ++i)
    {
        if (buffers[i] && lengths[i] > 0)
        {
            if (-1 == munmap(buffers[i], lengths[i]))
            {
                printf("munmap error %d, %s\n", errno, strerror(errno));
            }
        }
    }
    free(buffers);
    free(lengths);
#endif
}

void v4l2_close_device(int fd)
{
#ifdef __linux__
    if (fd > 0)
    {
        if (-1 == close(fd))
        {
            printf("v4l2_close_device error %d, %s\n", errno, strerror(errno));
        }
    }
#endif
}

int v4l2_stream(int fd, void** buffers, unsigned int bufferCount, int enable)
{
#ifdef __linux__
    struct v4l2_buffer buf;
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    unsigned int i;
    if (fd <= 0)
    {
        return V4L2_ERR_STAT;
    }
    if (enable)
    {
        /* 启动前先把所有缓冲区入队 */
        for (i = 0; i < bufferCount; ++i)
        {
            memset(&buf, 0, sizeof(buf));
            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_MMAP;
            buf.index = i;
            if (-1 == ioctl(fd, VIDIOC_QBUF, &buf))
            {
                printf("VIDIOC_QBUF error %d, %s\n", errno, strerror(errno));
                return V4L2_ERR_QBUF;
            }
        }
        if (-1 == ioctl(fd, VIDIOC_STREAMON, &type))
        {
            printf("VIDIOC_STREAMON error %d, %s\n", errno, strerror(errno));
            return V4L2_ERR_STREAMON;
        }
    }
    else
    {
        if (-1 == ioctl(fd, VIDIOC_STREAMOFF, &type))
        {
            printf("VIDIOC_STREAMOFF error %d, %s\n", errno, strerror(errno));
            return V4L2_ERR_STREAMOFF;
        }
    }
    return V4L2_OK;
#else
    return V4L2_ERR_STAT;
#endif
}

void* v4l2_dqbuf(int fd, void** buffers, unsigned int bufferCount, unsigned int* outBufIndex, int timeoutMs)
{
#ifdef __linux__
    struct v4l2_buffer buf;
    fd_set fds;
    struct timeval tv;
    int r;
    if (fd <= 0 || !buffers || 0 == bufferCount || !outBufIndex)
    {
        return NULL;
    }
    /* 等待数据 */
    if (timeoutMs >= 0)
    {
        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        tv.tv_sec = timeoutMs / 1000;
        tv.tv_usec = (timeoutMs % 1000) * 1000;
        r = select(fd + 1, &fds, NULL, NULL, &tv);
        if (r < 0)
        {
            printf("select error %d, %s\n", errno, strerror(errno));
            return NULL;
        }
        if (0 == r)
        {
            return NULL; /* 超时 */
        }
    }
    memset(&buf, 0, sizeof(buf));
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    if (-1 == ioctl(fd, VIDIOC_DQBUF, &buf))
    {
        if (errno != EAGAIN)
        {
            printf("VIDIOC_DQBUF error %d, %s\n", errno, strerror(errno));
        }
        return NULL;
    }
    if (buf.index >= bufferCount)
    {
        printf("Invalid buffer index %u >= %u\n", buf.index, bufferCount);
        return NULL;
    }
    *outBufIndex = buf.index;
    return buffers[buf.index];
#else
    return NULL;
#endif
}

int v4l2_qbuf(int fd, unsigned int bufIndex)
{
#ifdef __linux__
    struct v4l2_buffer buf;
    if (fd <= 0)
    {
        return 0;
    }
    memset(&buf, 0, sizeof(buf));
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = bufIndex;
    if (-1 == ioctl(fd, VIDIOC_QBUF, &buf))
    {
        printf("VIDIOC_QBUF(return) error %d, %s\n", errno, strerror(errno));
        return 0;
    }
    return 1;
#else
    return 0;
#endif
}

void v4l2_print_info(int fd)
{
#ifdef __linux__
    struct v4l2_capability cap;
    struct v4l2_fmtdesc fmtdesc;
    struct v4l2_format fmt;
    if (fd <= 0)
    {
        return;
    }
    printf("==================== V4L2 Info ====================\n");
    memset(&cap, 0, sizeof(cap));
    if (0 == ioctl(fd, VIDIOC_QUERYCAP, &cap))
    {
        printf("Driver:       %s\n", cap.driver);
        printf("Card:         %s\n", cap.card);
        printf("Bus:          %s\n", cap.bus_info);
        printf("Version:      %u.%u.%u\n", (cap.version >> 16) & 0xFF, (cap.version >> 8) & 0xFF, cap.version & 0xFF);
    }
    printf("Support formats:\n");
    memset(&fmtdesc, 0, sizeof(fmtdesc));
    fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    while (0 == ioctl(fd, VIDIOC_ENUM_FMT, &fmtdesc))
    {
        printf("  [%d] %c%c%c%c - %s\n", fmtdesc.index, (fmtdesc.pixelformat >> 0) & 0xFF, (fmtdesc.pixelformat >> 8) & 0xFF,
               (fmtdesc.pixelformat >> 16) & 0xFF, (fmtdesc.pixelformat >> 24) & 0xFF, fmtdesc.description);
        fmtdesc.index++;
    }
    memset(&fmt, 0, sizeof(fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (0 == ioctl(fd, VIDIOC_G_FMT, &fmt))
    {
        printf("Current:      %dx%d, %c%c%c%c\n", fmt.fmt.pix.width, fmt.fmt.pix.height, (fmt.fmt.pix.pixelformat >> 0) & 0xFF,
               (fmt.fmt.pix.pixelformat >> 8) & 0xFF, (fmt.fmt.pix.pixelformat >> 16) & 0xFF, (fmt.fmt.pix.pixelformat >> 24) & 0xFF);
    }
#endif
}

void v4l2_yuyv_to_rgb24(const unsigned char* yuyv, unsigned char* rgb, int width, int height)
{
    int i, j;
    int y0, y1, u, v;
    int r, g, b;
    int rgbIdx = 0;
    if (!yuyv || !rgb || width <= 0 || height <= 0)
    {
        return;
    }
    for (i = 0; i < height; ++i)
    {
        for (j = 0; j < width; j += 2)
        {
            int idx = (i * width + j) * 2;
            y0 = yuyv[idx];
            u = yuyv[idx + 1] - 128;
            y1 = yuyv[idx + 2];
            v = yuyv[idx + 3] - 128;
            r = (298 * y0 + 409 * v + 128) >> 8;
            g = (298 * y0 - 100 * u - 208 * v + 128) >> 8;
            b = (298 * y0 + 516 * u + 128) >> 8;
            rgb[rgbIdx++] = r < 0 ? 0 : (r > 255 ? 255 : r);
            rgb[rgbIdx++] = g < 0 ? 0 : (g > 255 ? 255 : g);
            rgb[rgbIdx++] = b < 0 ? 0 : (b > 255 ? 255 : b);
            r = (298 * y1 + 409 * v + 128) >> 8;
            g = (298 * y1 - 100 * u - 208 * v + 128) >> 8;
            b = (298 * y1 + 516 * u + 128) >> 8;
            rgb[rgbIdx++] = r < 0 ? 0 : (r > 255 ? 255 : r);
            rgb[rgbIdx++] = g < 0 ? 0 : (g > 255 ? 255 : g);
            rgb[rgbIdx++] = b < 0 ? 0 : (b > 255 ? 255 : b);
        }
    }
}
