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

const void* v4l2_dqbuf(int fd, void** buffers, unsigned int bufferCount, unsigned int* outBufIndex, int timeout)
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
    if (timeout > 0)
    {
        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        tv.tv_sec = timeout / 1000;
        tv.tv_usec = (timeout % 1000) * 1000;
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

static int s_yuv2rgb_init = 0;
static int s_yuv2rgb_r_v[256] = {0};
static int s_yuv2rgb_g_u[256] = {0};
static int s_yuv2rgb_g_v[256] = {0};
static int s_yuv2rgb_b_u[256] = {0};

static int s_yuv420_r_v[256] = {0};
static int s_yuv420_g_u[256] = {0};
static int s_yuv420_g_v[256] = {0};
static int s_yuv420_b_u[256] = {0};
static int s_yuv420_init = 0;

static void v4l2_yuyv_to_rgb24_init(void)
{
    int i;
    if (0 == s_yuv2rgb_init)
    {
        s_yuv2rgb_init = 1;
        for (i = 0; i < 256; ++i)
        {
            int v = i - 128;
            int u = i - 128;
            s_yuv2rgb_r_v[i] = (409 * v + 128) >> 8;
            s_yuv2rgb_g_u[i] = (100 * u + 128) >> 8;
            s_yuv2rgb_g_v[i] = (208 * v + 128) >> 8;
            s_yuv2rgb_b_u[i] = (516 * u + 128) >> 8;
        }
    }
}

static void v4l2_yuv420_to_rgb24_init(void)
{
    int i;
    if (0 == s_yuv420_init)
    {
        s_yuv420_init = 1;
        for (i = 0; i < 256; ++i)
        {
            int v = i - 128;
            int u = i - 128;
            s_yuv420_r_v[i] = (359 * v + 128) >> 8;
            s_yuv420_g_u[i] = (89 * u + 128) >> 8;
            s_yuv420_g_v[i] = (183 * v + 128) >> 8;
            s_yuv420_b_u[i] = (455 * u + 128) >> 8;
        }
    }
}

void v4l2_yuyv_to_rgb24(const unsigned char* yuyv, unsigned char* rgb, int width, int height)
{
    int i, j;
    int rgbIndex = 0;
    v4l2_yuyv_to_rgb24_init();
    for (i = 0; i < height; ++i)
    {
        for (j = 0; j < width; j += 2)
        {
            int idx = (i * width + j) * 2;
            int y0 = yuyv[idx];
            int u = yuyv[idx + 1];
            int y1 = yuyv[idx + 2];
            int v = yuyv[idx + 3];

            int r_v = s_yuv2rgb_r_v[v];
            int g_u = s_yuv2rgb_g_u[u];
            int g_v = s_yuv2rgb_g_v[v];
            int b_u = s_yuv2rgb_b_u[u];

            int y0_base = (298 * (y0 - 16) + 128) >> 8;
            int r0 = y0_base + r_v;
            int g0 = y0_base - g_u - g_v;
            int b0 = y0_base + b_u;

            rgb[rgbIndex++] = r0 < 0 ? 0 : (r0 > 255 ? 255 : r0);
            rgb[rgbIndex++] = g0 < 0 ? 0 : (g0 > 255 ? 255 : g0);
            rgb[rgbIndex++] = b0 < 0 ? 0 : (b0 > 255 ? 255 : b0);

            int y1_base = (298 * (y1 - 16) + 128) >> 8;
            int r1 = y1_base + r_v;
            int g1 = y1_base - g_u - g_v;
            int b1 = y1_base + b_u;

            rgb[rgbIndex++] = r1 < 0 ? 0 : (r1 > 255 ? 255 : r1);
            rgb[rgbIndex++] = g1 < 0 ? 0 : (g1 > 255 ? 255 : g1);
            rgb[rgbIndex++] = b1 < 0 ? 0 : (b1 > 255 ? 255 : b1);
        }
    }
}

void v4l2_yuyv_to_bgr24(const unsigned char* yuyv, unsigned char* bgr, int width, int height)
{
    int i, j;
    int bgrIndex = 0;
    v4l2_yuyv_to_rgb24_init();
    for (i = 0; i < height; ++i)
    {
        for (j = 0; j < width; j += 2)
        {
            int idx = (i * width + j) * 2;
            int y0 = yuyv[idx];
            int u = yuyv[idx + 1];
            int y1 = yuyv[idx + 2];
            int v = yuyv[idx + 3];

            int r_v = s_yuv2rgb_r_v[v];
            int g_u = s_yuv2rgb_g_u[u];
            int g_v = s_yuv2rgb_g_v[v];
            int b_u = s_yuv2rgb_b_u[u];

            int y0_base = (298 * (y0 - 16) + 128) >> 8;
            int r0 = y0_base + r_v;
            int g0 = y0_base - g_u - g_v;
            int b0 = y0_base + b_u;

            bgr[bgrIndex++] = b0 < 0 ? 0 : (b0 > 255 ? 255 : b0);
            bgr[bgrIndex++] = g0 < 0 ? 0 : (g0 > 255 ? 255 : g0);
            bgr[bgrIndex++] = r0 < 0 ? 0 : (r0 > 255 ? 255 : r0);

            int y1_base = (298 * (y1 - 16) + 128) >> 8;
            int r1 = y1_base + r_v;
            int g1 = y1_base - g_u - g_v;
            int b1 = y1_base + b_u;

            bgr[bgrIndex++] = b1 < 0 ? 0 : (b1 > 255 ? 255 : b1);
            bgr[bgrIndex++] = g1 < 0 ? 0 : (g1 > 255 ? 255 : g1);
            bgr[bgrIndex++] = r1 < 0 ? 0 : (r1 > 255 ? 255 : r1);
        }
    }
}

void v4l2_yuv420_to_rgb24(const unsigned char* yuv, unsigned char* rgb, int width, int height)
{
    int i, j;
    int rgbIndex = 0;
    int y_size = width * height;
    const unsigned char* y_buf = yuv;
    const unsigned char* uv_buf = yuv + y_size;
    v4l2_yuv420_to_rgb24_init();
    for (i = 0; i < height; ++i)
    {
        for (j = 0; j < width; ++j)
        {
            int y = y_buf[i * width + j];
            int uv_idx = (i / 2) * (width) + (j & ~1);
            int u = uv_buf[uv_idx];
            int v = uv_buf[uv_idx + 1];

            int rv = s_yuv420_r_v[v];
            int gu = s_yuv420_g_u[u];
            int gv = s_yuv420_g_v[v];
            int bu = s_yuv420_b_u[u];

            int y_base = (298 * (y - 16) + 128) >> 8;
            int r = y_base + rv;
            int g = y_base - gu - gv;
            int b = y_base + bu;

            rgb[rgbIndex++] = r < 0 ? 0 : (r > 255 ? 255 : r);
            rgb[rgbIndex++] = g < 0 ? 0 : (g > 255 ? 255 : g);
            rgb[rgbIndex++] = b < 0 ? 0 : (b > 255 ? 255 : b);
        }
    }
}

void v4l2_yuv420_to_bgr24(const unsigned char* yuv, unsigned char* bgr, int width, int height)
{
    int i, j;
    int bgrIndex = 0;
    int y_size = width * height;
    const unsigned char* y_buf = yuv;
    const unsigned char* uv_buf = yuv + y_size;
    v4l2_yuv420_to_rgb24_init();
    for (i = 0; i < height; ++i)
    {
        for (j = 0; j < width; ++j)
        {
            int y = y_buf[i * width + j];
            int uv_idx = (i / 2) * width + (j & ~1);
            int u = uv_buf[uv_idx];
            int v = uv_buf[uv_idx + 1];

            int rv = s_yuv420_r_v[v];
            int gu = s_yuv420_g_u[u];
            int gv = s_yuv420_g_v[v];
            int bu = s_yuv420_b_u[u];

            int y_base = (298 * (y - 16) + 128) >> 8;
            int r = y_base + rv;
            int g = y_base - gu - gv;
            int b = y_base + bu;

            bgr[bgrIndex++] = b < 0 ? 0 : (b > 255 ? 255 : b);
            bgr[bgrIndex++] = g < 0 ? 0 : (g > 255 ? 255 : g);
            bgr[bgrIndex++] = r < 0 ? 0 : (r > 255 ? 255 : r);
        }
    }
}

void v4l2_rgb24_to_bgr24(const unsigned char* rgb, unsigned char* bgr, int width, int height)
{
    int i, j;
    int index = 0;
    for (i = 0; i < height; ++i)
    {
        for (j = 0; j < width; ++j)
        {
            unsigned char r = rgb[index];
            unsigned char g = rgb[index + 1];
            unsigned char b = rgb[index + 2];
            bgr[index++] = b;
            bgr[index++] = g;
            bgr[index++] = r;
        }
    }
}

void v4l2_bgr24_to_rgb24(const unsigned char* bgr, unsigned char* rgb, int width, int height)
{
    int i, j;
    int index = 0;
    for (i = 0; i < height; ++i)
    {
        for (j = 0; j < width; ++j)
        {
            unsigned char b = bgr[index];
            unsigned char g = bgr[index + 1];
            unsigned char r = bgr[index + 2];
            rgb[index++] = r;
            rgb[index++] = g;
            rgb[index++] = b;
        }
    }
}
