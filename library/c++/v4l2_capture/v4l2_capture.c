/**********************************************************************
* Author:	jaron.ho
* Date:		2018-09-29
* Brief:	v4l2 capture
**********************************************************************/
#include "v4l2_capture.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
/*********************************************************************/
static const char* pix_fmt_str(unsigned int pixelformat) {
	switch (pixelformat) {
		case V4L2_PIX_FMT_RGB332: return "V4L2_PIX_FMT_RGB332";
		case V4L2_PIX_FMT_RGB444: return "V4L2_PIX_FMT_RGB444";
		case V4L2_PIX_FMT_RGB555: return "V4L2_PIX_FMT_RGB555";
		case V4L2_PIX_FMT_RGB565: return "V4L2_PIX_FMT_RGB565";
		case V4L2_PIX_FMT_RGB555X: return "V4L2_PIX_FMT_RGB555X";
		case V4L2_PIX_FMT_RGB565X: return "V4L2_PIX_FMT_RGB565X";
		case V4L2_PIX_FMT_BGR666: return "V4L2_PIX_FMT_BGR666";
		case V4L2_PIX_FMT_BGR24: return "V4L2_PIX_FMT_BGR24";
		case V4L2_PIX_FMT_RGB24: return "V4L2_PIX_FMT_RGB24";
		case V4L2_PIX_FMT_BGR32: return "V4L2_PIX_FMT_BGR32";
		case V4L2_PIX_FMT_RGB32: return "V4L2_PIX_FMT_RGB32";
		case V4L2_PIX_FMT_GREY: return "V4L2_PIX_FMT_GREY";
		case V4L2_PIX_FMT_Y4: return "V4L2_PIX_FMT_Y4";
		case V4L2_PIX_FMT_Y6: return "V4L2_PIX_FMT_Y6";
		case V4L2_PIX_FMT_Y10: return "V4L2_PIX_FMT_Y10";
		case V4L2_PIX_FMT_Y12: return "V4L2_PIX_FMT_Y12";
		case V4L2_PIX_FMT_Y16: return "V4L2_PIX_FMT_Y16";
		case V4L2_PIX_FMT_Y10BPACK: return "V4L2_PIX_FMT_Y10BPACK";
		case V4L2_PIX_FMT_PAL8: return "V4L2_PIX_FMT_PAL8";
		case V4L2_PIX_FMT_UV8: return "V4L2_PIX_FMT_UV8";
		case V4L2_PIX_FMT_YVU410: return "V4L2_PIX_FMT_YVU410";
		case V4L2_PIX_FMT_YVU420: return "V4L2_PIX_FMT_YVU420";
		case V4L2_PIX_FMT_YUYV: return "V4L2_PIX_FMT_YUYV";
		case V4L2_PIX_FMT_YYUV: return "V4L2_PIX_FMT_YYUV";
		case V4L2_PIX_FMT_YVYU: return "V4L2_PIX_FMT_YVYU";
		case V4L2_PIX_FMT_UYVY: return "V4L2_PIX_FMT_UYVY";
		case V4L2_PIX_FMT_VYUY: return "V4L2_PIX_FMT_VYUY";
		case V4L2_PIX_FMT_YUV422P: return "V4L2_PIX_FMT_YUV422P";
		case V4L2_PIX_FMT_YUV411P: return "V4L2_PIX_FMT_YUV411P";
		case V4L2_PIX_FMT_Y41P: return "V4L2_PIX_FMT_Y41P";
		case V4L2_PIX_FMT_YUV444: return "V4L2_PIX_FMT_YUV444";
		case V4L2_PIX_FMT_YUV555: return "V4L2_PIX_FMT_YUV555";
		case V4L2_PIX_FMT_YUV565: return "V4L2_PIX_FMT_YUV565";
		case V4L2_PIX_FMT_YUV32: return "V4L2_PIX_FMT_YUV32";
		case V4L2_PIX_FMT_YUV410: return "V4L2_PIX_FMT_YUV410";
		case V4L2_PIX_FMT_YUV420: return "V4L2_PIX_FMT_YUV420";
		case V4L2_PIX_FMT_HI240: return "V4L2_PIX_FMT_HI240";
		case V4L2_PIX_FMT_HM12: return "V4L2_PIX_FMT_HM12";
		case V4L2_PIX_FMT_M420: return "V4L2_PIX_FMT_M420";
		case V4L2_PIX_FMT_NV12: return "V4L2_PIX_FMT_NV12";
		case V4L2_PIX_FMT_NV21: return "V4L2_PIX_FMT_NV21";
		case V4L2_PIX_FMT_NV16: return "V4L2_PIX_FMT_NV16";
		case V4L2_PIX_FMT_NV61: return "V4L2_PIX_FMT_NV61";
		case V4L2_PIX_FMT_NV24: return "V4L2_PIX_FMT_NV24";
		case V4L2_PIX_FMT_NV42: return "V4L2_PIX_FMT_NV42";
		case V4L2_PIX_FMT_NV12M: return "V4L2_PIX_FMT_NV12M";
		case V4L2_PIX_FMT_NV21M: return "V4L2_PIX_FMT_NV21M";
		case V4L2_PIX_FMT_NV16M: return "V4L2_PIX_FMT_NV16M";
		case V4L2_PIX_FMT_NV61M: return "V4L2_PIX_FMT_NV61M";
		case V4L2_PIX_FMT_NV12MT: return "V4L2_PIX_FMT_NV12MT";
		case V4L2_PIX_FMT_NV12MT_16X16: return "V4L2_PIX_FMT_NV12MT_16X16";
		case V4L2_PIX_FMT_YUV420M: return "V4L2_PIX_FMT_YUV420M";
		case V4L2_PIX_FMT_YVU420M: return "V4L2_PIX_FMT_YVU420M";
		case V4L2_PIX_FMT_SBGGR8: return "V4L2_PIX_FMT_SBGGR8";
		case V4L2_PIX_FMT_SGBRG8: return "V4L2_PIX_FMT_SGBRG8";
		case V4L2_PIX_FMT_SGRBG8: return "V4L2_PIX_FMT_SGRBG8";
		case V4L2_PIX_FMT_SRGGB8: return "V4L2_PIX_FMT_SRGGB8";
		case V4L2_PIX_FMT_SBGGR10: return "V4L2_PIX_FMT_SBGGR10";
		case V4L2_PIX_FMT_SGBRG10: return "V4L2_PIX_FMT_SGBRG10";
		case V4L2_PIX_FMT_SGRBG10: return "V4L2_PIX_FMT_SGRBG10";
		case V4L2_PIX_FMT_SRGGB10: return "V4L2_PIX_FMT_SRGGB10";
		case V4L2_PIX_FMT_SBGGR12: return "V4L2_PIX_FMT_SBGGR12";
		case V4L2_PIX_FMT_SGBRG12: return "V4L2_PIX_FMT_SGBRG12";
		case V4L2_PIX_FMT_SGRBG12: return "V4L2_PIX_FMT_SGRBG12";
		case V4L2_PIX_FMT_SRGGB12: return "V4L2_PIX_FMT_SRGGB12";
		case V4L2_PIX_FMT_SBGGR10ALAW8: return "V4L2_PIX_FMT_SBGGR10ALAW8";
		case V4L2_PIX_FMT_SGBRG10ALAW8: return "V4L2_PIX_FMT_SGBRG10ALAW8";
		case V4L2_PIX_FMT_SGRBG10ALAW8: return "V4L2_PIX_FMT_SGRBG10ALAW8";
		case V4L2_PIX_FMT_SRGGB10ALAW8: return "V4L2_PIX_FMT_SRGGB10ALAW8";
		case V4L2_PIX_FMT_SBGGR10DPCM8: return "V4L2_PIX_FMT_SBGGR10DPCM8";
		case V4L2_PIX_FMT_SGBRG10DPCM8: return "V4L2_PIX_FMT_SGBRG10DPCM8";
		case V4L2_PIX_FMT_SGRBG10DPCM8: return "V4L2_PIX_FMT_SGRBG10DPCM8";
		case V4L2_PIX_FMT_SRGGB10DPCM8: return "V4L2_PIX_FMT_SRGGB10DPCM8";
		case V4L2_PIX_FMT_SBGGR16: return "V4L2_PIX_FMT_SBGGR16";
		case V4L2_PIX_FMT_MJPEG: return "V4L2_PIX_FMT_MJPEG";
		case V4L2_PIX_FMT_JPEG: return "V4L2_PIX_FMT_JPEG";
		case V4L2_PIX_FMT_DV: return "V4L2_PIX_FMT_DV";
		case V4L2_PIX_FMT_MPEG: return "V4L2_PIX_FMT_MPEG";
		case V4L2_PIX_FMT_H264: return "V4L2_PIX_FMT_H264";
		case V4L2_PIX_FMT_H264_NO_SC: return "V4L2_PIX_FMT_H264_NO_SC";
		case V4L2_PIX_FMT_H264_MVC: return "V4L2_PIX_FMT_H264_MVC";
		case V4L2_PIX_FMT_H263: return "V4L2_PIX_FMT_H263";
		case V4L2_PIX_FMT_MPEG1: return "V4L2_PIX_FMT_MPEG1";
		case V4L2_PIX_FMT_MPEG2: return "V4L2_PIX_FMT_MPEG2";
		case V4L2_PIX_FMT_MPEG4: return "V4L2_PIX_FMT_MPEG4";
		case V4L2_PIX_FMT_XVID: return "V4L2_PIX_FMT_XVID";
		case V4L2_PIX_FMT_VC1_ANNEX_G: return "V4L2_PIX_FMT_VC1_ANNEX_G";
		case V4L2_PIX_FMT_VC1_ANNEX_L: return "V4L2_PIX_FMT_VC1_ANNEX_L";
		case V4L2_PIX_FMT_VP8: return "V4L2_PIX_FMT_VP8";
		case V4L2_PIX_FMT_CPIA1: return "V4L2_PIX_FMT_CPIA1";
		case V4L2_PIX_FMT_WNVA: return "V4L2_PIX_FMT_WNVA";
		case V4L2_PIX_FMT_SN9C10X: return "V4L2_PIX_FMT_SN9C10X";
		case V4L2_PIX_FMT_SN9C20X_I420: return "V4L2_PIX_FMT_SN9C20X_I420";
		case V4L2_PIX_FMT_PWC1: return "V4L2_PIX_FMT_PWC1";
		case V4L2_PIX_FMT_PWC2: return "V4L2_PIX_FMT_PWC2";
		case V4L2_PIX_FMT_ET61X251: return "V4L2_PIX_FMT_ET61X251";
		case V4L2_PIX_FMT_SPCA501: return "V4L2_PIX_FMT_SPCA501";
		case V4L2_PIX_FMT_SPCA505: return "V4L2_PIX_FMT_SPCA505";
		case V4L2_PIX_FMT_SPCA508: return "V4L2_PIX_FMT_SPCA508";
		case V4L2_PIX_FMT_SPCA561: return "V4L2_PIX_FMT_SPCA561";
		case V4L2_PIX_FMT_PAC207: return "V4L2_PIX_FMT_PAC207";
		case V4L2_PIX_FMT_MR97310A: return "V4L2_PIX_FMT_MR97310A";
		case V4L2_PIX_FMT_JL2005BCD: return "V4L2_PIX_FMT_JL2005BCD";
		case V4L2_PIX_FMT_SN9C2028: return "V4L2_PIX_FMT_SN9C2028";
		case V4L2_PIX_FMT_SQ905C: return "V4L2_PIX_FMT_SQ905C";
		case V4L2_PIX_FMT_PJPG: return "V4L2_PIX_FMT_PJPG";
		case V4L2_PIX_FMT_OV511: return "V4L2_PIX_FMT_OV511";
		case V4L2_PIX_FMT_OV518: return "V4L2_PIX_FMT_OV518";
		case V4L2_PIX_FMT_STV0680: return "V4L2_PIX_FMT_STV0680";
		case V4L2_PIX_FMT_TM6000: return "V4L2_PIX_FMT_TM6000";
		case V4L2_PIX_FMT_CIT_YYVYUY: return "V4L2_PIX_FMT_CIT_YYVYUY";
		case V4L2_PIX_FMT_KONICA420: return "V4L2_PIX_FMT_KONICA420";
		case V4L2_PIX_FMT_JPGL: return "V4L2_PIX_FMT_JPGL";
		case V4L2_PIX_FMT_SE401: return "V4L2_PIX_FMT_SE401";
		case V4L2_PIX_FMT_S5C_UYVY_JPG: return "V4L2_PIX_FMT_S5C_UYVY_JPG";
	}
	static char str[64] = { 0 };
	memset(str, 0, sizeof(str));
	sprintf(str, "%u", pixelformat);
	return str;
}
/*********************************************************************/
static const char* buf_type_str(unsigned int type) {
	switch ((v4l2_buf_type)type) {
		case V4L2_BUF_TYPE_VIDEO_CAPTURE: return "V4L2_BUF_TYPE_VIDEO_CAPTURE";
		case V4L2_BUF_TYPE_VIDEO_OUTPUT: return "V4L2_BUF_TYPE_VIDEO_OUTPUT";
		case V4L2_BUF_TYPE_VIDEO_OVERLAY: return "V4L2_BUF_TYPE_VIDEO_OVERLAY";
		case V4L2_BUF_TYPE_VBI_CAPTURE: return "V4L2_BUF_TYPE_VBI_CAPTURE";
		case V4L2_BUF_TYPE_VBI_OUTPUT: return "V4L2_BUF_TYPE_VBI_OUTPUT";
		case V4L2_BUF_TYPE_SLICED_VBI_CAPTURE: return "V4L2_BUF_TYPE_SLICED_VBI_CAPTURE";
		case V4L2_BUF_TYPE_SLICED_VBI_OUTPUT: return "V4L2_BUF_TYPE_SLICED_VBI_OUTPUT";
		case V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY: return "V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY";
		case V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE: return "V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE";
		case V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE: return "V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE";
		case V4L2_BUF_TYPE_PRIVATE: return "V4L2_BUF_TYPE_PRIVATE";
	}
	static char str[64] = { 0 };
	memset(str, 0, sizeof(str));
	sprintf(str, "%u", type);
	return str;
}
/*********************************************************************/
static const char* field_str(unsigned int field) {
	switch ((v4l2_field)field) {
		case V4L2_FIELD_ANY: return "V4L2_FIELD_ANY";
		case V4L2_FIELD_NONE: return "V4L2_FIELD_NONE";
		case V4L2_FIELD_TOP: return "V4L2_FIELD_TOP";
		case V4L2_FIELD_BOTTOM: return "V4L2_FIELD_BOTTOM";
		case V4L2_FIELD_INTERLACED: return "V4L2_FIELD_INTERLACED";
		case V4L2_FIELD_SEQ_TB: return "V4L2_FIELD_SEQ_TB";
		case V4L2_FIELD_SEQ_BT: return "V4L2_FIELD_SEQ_BT";
		case V4L2_FIELD_ALTERNATE: return "V4L2_FIELD_ALTERNATE";
		case V4L2_FIELD_INTERLACED_TB: return "V4L2_V4L2_FIELD_INTERLACED_TBFIELD_ANY";
		case V4L2_FIELD_INTERLACED_BT: return "V4L2_FIELD_INTERLACED_BT";
	}
	static char str[64] = { 0 };
	memset(str, 0, sizeof(str));
	sprintf(str, "%u", field);
	return str;
}
/*********************************************************************/
static const char* colorspace_str(unsigned int colorspace) {
	switch ((v4l2_colorspace)colorspace) {
		case V4L2_COLORSPACE_SMPTE170M: return "V4L2_COLORSPACE_SMPTE170M";
		case V4L2_COLORSPACE_SMPTE240M: return "V4L2_COLORSPACE_SMPTE240M";
		case V4L2_COLORSPACE_REC709: return "V4L2_COLORSPACE_REC709";
		case V4L2_COLORSPACE_BT878: return "V4L2_COLORSPACE_BT878";
		case V4L2_COLORSPACE_470_SYSTEM_M: return "V4L2_COLORSPACE_470_SYSTEM_M";
		case V4L2_COLORSPACE_470_SYSTEM_BG: return "V4L2_COLORSPACE_470_SYSTEM_BG";
		case V4L2_COLORSPACE_JPEG: return "V4L2_COLORSPACE_JPEG";
		case V4L2_COLORSPACE_SRGB: return "V4L2_COLORSPACE_SRGB";
	}
	static char str[64] = { 0 };
	memset(str, 0, sizeof(str));
	sprintf(str, "%u", colorspace);
	return str;
}
/*********************************************************************/
static const char* buf_flag_str(unsigned int flag) {
	switch (flag) {
		case V4L2_BUF_FLAG_MAPPED: return "V4L2_BUF_FLAG_MAPPED";
		case V4L2_BUF_FLAG_QUEUED: return "V4L2_BUF_FLAG_QUEUED";
		case V4L2_BUF_FLAG_DONE: return "V4L2_BUF_FLAG_DONE";
		case V4L2_BUF_FLAG_KEYFRAME: return "V4L2_BUF_FLAG_KEYFRAME";
		case V4L2_BUF_FLAG_PFRAME: return "V4L2_BUF_FLAG_PFRAME";
		case V4L2_BUF_FLAG_BFRAME: return "V4L2_BUF_FLAG_BFRAME";
		case V4L2_BUF_FLAG_ERROR: return "V4L2_BUF_FLAG_ERROR";
		case V4L2_BUF_FLAG_TIMECODE: return "V4L2_BUF_FLAG_TIMECODE";
		case V4L2_BUF_FLAG_PREPARED: return "V4L2_BUF_FLAG_PREPARED";
		case V4L2_BUF_FLAG_NO_CACHE_INVALIDATE: return "V4L2_BUF_FLAG_NO_CACHE_INVALIDATE";
		case V4L2_BUF_FLAG_NO_CACHE_CLEAN: return "V4L2_BUF_FLAG_NO_CACHE_CLEAN";
		case V4L2_BUF_FLAG_TIMESTAMP_MASK: return "V4L2_BUF_FLAG_TIMESTAMP_MASK";
		case V4L2_BUF_FLAG_TIMESTAMP_UNKNOWN: return "V4L2_BUF_FLAG_TIMESTAMP_UNKNOWN";
		case V4L2_BUF_FLAG_TIMESTAMP_MONOTONIC: return "V4L2_BUF_FLAG_TIMESTAMP_MONOTONIC";
		case V4L2_BUF_FLAG_TIMESTAMP_COPY: return "V4L2_BUF_FLAG_TIMESTAMP_COPY";
	}
	static char str[64] = { 0 };
	memset(str, 0, sizeof(str));
	sprintf(str, "%u", flag);
	return str;
}
/*********************************************************************/
static void v4l2_information(int fd) {
	struct v4l2_capability cap;
	struct v4l2_fmtdesc fmtdesc;
	struct v4l2_format fmt;
	assert(fd > 0);
	printf("==================== v4l2 information ====================\n");
	memset(&cap, 0, sizeof(struct v4l2_capability));
    if (-1 == ioctl(fd, VIDIOC_QUERYCAP, &cap)) {
		printf("VIDIOC_QUERYCAP error %d, %s\n", errno, strerror(errno));
	} else {
        printf("--------------- Capability ---------------\n");
		printf("- driver:         %s\n", cap.driver);
		printf("- card:           %s\n", cap.card);
		printf("- bus_info:       %s\n", cap.bus_info);
		printf("- version:        %u\n", cap.version);
		printf("- capabilities:   %u\n", cap.capabilities  & 0xFF);
		printf("- device_caps:    %u\n", cap.device_caps);
    }
	memset(&fmtdesc, 0, sizeof(struct v4l2_fmtdesc));
    fmtdesc.index = 0;
    fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == ioctl(fd, VIDIOC_ENUM_FMT, &fmtdesc)) {
		printf("VIDIOC_ENUM_FMT error %d, %s\n", errno, strerror(errno));
	} else {
		printf("------------- Support Format -------------\n");
		printf("- type:           %s\n", buf_type_str(fmtdesc.type));
		printf("- flags:          %s\n", buf_flag_str(fmtdesc.flags));
		printf("- description:    %s\n", fmtdesc.description);
		printf("- pixelformat:    %s\n", pix_fmt_str(fmtdesc.pixelformat));
	}
	memset(&fmt, 0, sizeof(struct v4l2_format));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == ioctl(fd, VIDIOC_G_FMT, &fmt)) {
		printf("VIDIOC_G_FMT error %d, %s\n", errno, strerror(errno));
	} else {
		printf("-------------- Video Format --------------\n");
		printf("- type:           %s\n", buf_type_str(fmt.type));
		printf("- width:          %u\n", fmt.fmt.pix.width);
		printf("- height:         %u\n", fmt.fmt.pix.height);
		printf("- pixelformat:    %s\n", pix_fmt_str(fmt.fmt.pix.pixelformat));
		printf("- field:          %s\n", field_str(fmt.fmt.pix.field));
		printf("- bytesperline:   %u\n", fmt.fmt.pix.bytesperline);
		printf("- sizeimage:      %u\n", fmt.fmt.pix.sizeimage);
		printf("- colorspace:     %s\n", colorspace_str(fmt.fmt.pix.colorspace));
		printf("- priv:           %u\n", fmt.fmt.pix.priv);
	}
}
/*********************************************************************/
typedef struct v4l2_video_buffer {
	void* start;
	size_t length;
} v4l2_video_buffer;
/*********************************************************************/
static int v4l2_open_device(const char* devName, unsigned int* width, unsigned int* height, unsigned int bufferCount, v4l2_video_buffer** buffers) {
	int fd;
	struct stat st;
	struct v4l2_capability cap;
	struct v4l2_fmtdesc fmtdesc;
	struct v4l2_format format;
	struct v4l2_requestbuffers reqbuf;
	struct v4l2_buffer buf;
	struct v4l2_video_buffer* videobufs;
	unsigned int i;
	assert(devName && strlen(devName) > 0);
	assert(width && *width > 0 && height && *height > 0);
	assert(bufferCount > 0);
	assert(buffers && !(*buffers));
	/* step1:check device */
	if (-1 == stat(devName, &st)) {
		printf("Cannot identify '%s': %d, %s\n", devName, errno, strerror(errno));
		return -1;
	}
	if (!S_ISCHR(st.st_mode)) {
		printf("'%s' is no device\n", devName);
		return -2;
	}
	fd = open(devName, O_RDWR, 0);
	if (-1 == fd) {
		printf("Cannot open '%s': %d, %s\n", devName, errno, strerror(errno));
		return -3;
	}
	/* step2:query capability */
	memset(&cap, 0, sizeof(struct v4l2_capability));
	if (-1 == ioctl(fd, VIDIOC_QUERYCAP, &cap)) {
		if (EINVAL == errno) {
			printf("'%s' is no v4l2 device\n", devName);
		} else {
			printf("VIDIOC_QUERYCAP error %d, %s\n", errno, strerror(errno));
		}
		close(fd);
		return -4;
	}
	if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
		printf("'%s' is no video capture device\n", devName);
		close(fd);
		return -5;
	}
	if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
		printf("'%s' does not support streaming i/o\n", devName);
		close(fd);
		return -6;
	}
	/* step3:query video supported format */
	memset(&fmtdesc, 0, sizeof(struct v4l2_fmtdesc));
	fmtdesc.index = 0;
    fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == ioctl(fd, VIDIOC_ENUM_FMT, &fmtdesc)) {
		printf("VIDIOC_ENUM_FMT error %d, %s\n", errno, strerror(errno));
		close(fd);
		return -7;
	}
	/* step4:set format */
	memset(&format, 0, sizeof(struct v4l2_format));
	format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	format.fmt.pix.width = *width;
	format.fmt.pix.height = *height;
	format.fmt.pix.pixelformat = fmtdesc.pixelformat;
	format.fmt.pix.field = V4L2_FIELD_INTERLACED;
	if (-1 == ioctl(fd, VIDIOC_S_FMT, &format)) {
		printf("VIDIOC_S_FMT error %d, %s\n", errno, strerror(errno));
		close(fd);
		return -8;
	}
	*width = format.fmt.pix.width;
	*height = format.fmt.pix.height;
	/* step5:request buffer */
	memset(&reqbuf, 0, sizeof(struct v4l2_requestbuffers));
	reqbuf.count = bufferCount;
	reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	reqbuf.memory = V4L2_MEMORY_MMAP;
	if (-1 == ioctl(fd, VIDIOC_REQBUFS, &reqbuf)) {
		if (EINVAL == errno) {
			printf("'%s' does not support memory mapping\n", devName);
		} else {
			printf("VIDIOC_REQBUFS error %d, %s\n", errno, strerror(errno));
		}
		close(fd);
		return -9;
	}
	/* step6:allocation video buffer */
	videobufs = (struct v4l2_video_buffer*)calloc(bufferCount, sizeof(struct v4l2_video_buffer));
	if (!videobufs) {
		printf("Out of memory\n");
		close(fd);
		return -10;
	}
	for (i = 0; i < bufferCount; ++i) {
		memset(&buf, 0, sizeof(struct v4l2_buffer));
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = i;
		/* step6-1:query buffer */
		if (-1 == ioctl(fd, VIDIOC_QUERYBUF, &buf)) {
			printf("VIDIOC_QUERYBUF error %d, %s\n", errno, strerror(errno));
			free(videobufs);
			close(fd);
			return -11;
		}
		/* step6-2:map memory address */
		videobufs[i].start = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);
		if (MAP_FAILED == videobufs[i].start) {
			printf("mmap error %d, %s\n", errno, strerror(errno));
			free(videobufs);
			close(fd);
			return -12;
		}
		videobufs[i].length = buf.length;
	}
	*buffers = videobufs;
	return fd;
}
/*********************************************************************/
static int v4l2_close_device(int fd, unsigned int bufferCount, v4l2_video_buffer* buffers) {
	unsigned int i;
	assert(fd > 0);
	assert(bufferCount > 0);
	assert(buffers);
	/* step1:recover video buffer */
	for (i = 0; i < bufferCount; ++i) {
		if (buffers[i].start) {
			if (-1 == munmap(buffers[i].start, buffers[i].length)) {
				printf("munmap error %d, %s\n", errno, strerror(errno));
			}
		}
	}
	free(buffers);
	buffers = NULL;
	/* step2:close device */
	if (-1 == close(fd)) {
		printf("close error %d, %s\n", errno, strerror(errno));
		return 0;
	}
	return 1;
}
/*********************************************************************/
static int v4l2_enable_capture(int fd, int enable) {
	enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	assert(fd > 0);
	if (enable) {
		if (-1 == ioctl(fd, VIDIOC_STREAMON, &type)) {
			printf("VIDIOC_STREAMON error %d, %s\n", errno, strerror(errno));
			return 0;
		}
	} else {
		if (-1 == ioctl(fd, VIDIOC_STREAMOFF, &type)) {
			printf("VIDIOC_STREAMOFF error %d, %s\n", errno, strerror(errno));
			return 0;
		}
	}
	return 1;
}
/*********************************************************************/
static void* v4l2_get_frame(int fd, unsigned int bufferCount, v4l2_video_buffer* buffers, size_t* length) {
	struct v4l2_buffer buf;
	void* frame = NULL;
	assert(fd > 0);
	assert(bufferCount > 0);
	assert(buffers);
	assert(length);
	/* step1:get from buffer queue*/
	memset(&buf, 0, sizeof(struct v4l2_buffer));
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;
	if (-1 == ioctl(fd, VIDIOC_QBUF, &buf)) {
		printf("VIDIOC_QBUF error %d, %s\n", errno, strerror(errno));
		return NULL;
	}
	if (-1 == ioctl(fd, VIDIOC_DQBUF, &buf)) {
		printf("VIDIOC_DQBUF error %d, %s\n", errno, strerror(errno));
		return NULL;
	}
	assert(buf.index < bufferCount);
	/* step2:allocation frame memory */
	frame = calloc(buf.length, 1);
	memcpy(frame, buffers[buf.index].start, buf.length);
	*length = buf.length;
	return frame;
}
/*********************************************************************/
static char* s_devName = NULL;
static int s_fd = -1;
static unsigned int s_bufferCount = 1;
static v4l2_video_buffer* s_buffers = NULL;
static int s_enabled = 0;
void v4l2_open(const char* devName, unsigned int* width, unsigned int* height) {
	assert(devName && strlen(devName) > 0);
	assert(width && *width > 0 && height && *height > 0);
	if (s_fd > 0) {
		printf("Current existing device '%s' is opened, can not open new device '%s'\n", s_devName, devName);
		return;
	}
	s_fd = v4l2_open_device(devName, width, height, s_bufferCount, &s_buffers);
	if (s_fd > 0) {
		s_devName = (char*)calloc(strlen(devName) + 1, sizeof(char));
		sprintf(s_devName, "%s", devName);
		s_enabled = 0;
	}
}
void v4l2_close(void) {
	if (s_fd <= 0) {
		return;
	}
	if (s_enabled) {
		if (v4l2_enable_capture(s_fd, 0)) {
			s_enabled = 0;
		}
	}
	if (v4l2_close_device(s_fd, s_bufferCount, s_buffers)) {
		s_fd = -1;
		free(s_devName);
		s_devName = NULL;
	}
}
void v4l2_enable(int enable) {
	if (s_fd <= 0) {
		return;
	}
	if (enable) {
		if (!s_enabled) {
			if (v4l2_enable_capture(s_fd, 1)) {
				s_enabled = 1;
			}
		}
	} else {
		if (s_enabled) {
			if (v4l2_enable_capture(s_fd, 0)) {
				s_enabled = 0;
			}
		}
	}
}
void* v4l2_capture(size_t* length) {
	assert(length);
	if (s_fd <= 0 || !s_enabled) {
		return NULL;
	}
	return v4l2_get_frame(s_fd, s_bufferCount, s_buffers, length);
}
void v4l2_info(void) {
	if (s_fd > 0) {
		v4l2_information(s_fd);
	}
}
/*********************************************************************/
