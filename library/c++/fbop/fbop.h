/**********************************************************************
* Author:	jaron.ho
* Date:		2019-11-05
* Brief:	封装了基于linux中framebuffer的操作接口
**********************************************************************/
#ifndef FBOP_H
#define FBOP_H

#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define GL_ARGB(a, r, g, b)     ((((unsigned int)(a)) << 24) | (((unsigned int)(r)) << 16) | (((unsigned int)(g)) << 8) | ((unsigned int)(b)))
#define GL_ARGB_A(rgb)          ((((unsigned int)(rgb)) >> 24) & 0xFF)
#define GL_RGB(r, g, b)         ((0xFF << 24) | (((unsigned int)(r)) << 16) | (((unsigned int)(g)) << 8) | ((unsigned int)(b)))
#define GL_RGB_R(rgb)           ((((unsigned int)(rgb)) >> 16) & 0xFF)
#define GL_RGB_G(rgb)           ((((unsigned int)(rgb)) >> 8) & 0xFF)
#define GL_RGB_B(rgb)           (((unsigned int)(rgb)) & 0xFF)
#define GL_RGB_32_to_16(rgb)    (((((unsigned int)(rgb)) & 0xFF) >> 3) | ((((unsigned int)(rgb)) & 0xFC00) >> 5) | ((((unsigned int)(rgb)) & 0xF80000) >> 8))
#define GL_RGB_16_to_32(rgb)    ((0xFF << 24) | ((((unsigned int)(rgb)) & 0x1F) << 3) | ((((unsigned int)(rgb)) & 0x7E0) << 5) | ((((unsigned int)(rgb)) & 0xF800) << 8))

/*
 * 功  能:    打开fb设备并初始化
 * 参  数:    fbPath - fb设备路径, 例如: /dev/fb0
 * 返回值:    0.成功, 1.参数错误, 2.无法打开fb设备, 3.无法读取屏幕固定信息, 4.无法读取屏幕可变信息, 5.无法映射fb设备到内存
 */
int fbop_open(const char* fbPath);

/*
 * 功  能:    关闭fb设备
 * 参  数:    无
 * 返回值:    无
 */
void fbop_close(void);

/*
 * 功  能:    获取fb设备宽度
 * 参  数:    无
 * 返回值:    int
 */
int fbop_screen_width(void);

/*
 * 功  能:    获取fb设备高度
 * 参  数:    无
 * 返回值:    int
 */
int fbop_screen_height(void);

/*
 * 功  能:    获取单个像素所占位数, 例如: 16位, 24位
 * 参  数:    无
 * 返回值:    int
 */
int fbop_pixel_bits(void);

/*
 * 功  能:    获取单个颜色所占字节数, 例如: 2字节, 3字节
 * 参  数:    无
 * 返回值:    int
 */
int fbop_color_bytes(void);

/*
 * 功  能:    获取fb缓冲区大小
 * 参  数:    无
 * 返回值:    unsigned long
 */
unsigned long fbop_size(void);

/*
 * 功  能:    获取fb缓冲区指针
 * 参  数:    无
 * 返回值:    void*
 */
void* fbop_buffer(void);

/*
 * 功  能:    获取像素点
 * 参  数:    x - 像素点x坐标
 *            y - 像素点y坐标
 *            rgb - 32位像素点颜色值(输出)
 * 返回值:    0.成功, 1.设备未打开, 2.坐标溢出, 3.不支持颜色所占字节数
 */
int fbop_get_pixel(int x, int y, unsigned int* rgb);

/*
 * 功  能:    设置16位像素点
 * 参  数:    x - 像素点x坐标
 *            y - 像素点y坐标
 *            rgb - 16位像素点颜色值
 * 返回值:    0.成功, 1.设备未打开, 2.坐标溢出, 3.不支持颜色所占字节数
 */
int fbop_set_pixel16(int x, int y, unsigned short rgb);

/*
 * 功  能:    设置32位像素点
 * 参  数:    x - 像素点x坐标
 *            y - 像素点y坐标
 *            rgb - 32位像素点颜色值
 * 返回值:    0.成功, 1.设备未打开, 2.坐标溢出, 3.不支持颜色所占字节数
 */
int fbop_set_pixel32(int x, int y, unsigned int rgb);

/*
 * 功  能:    快照
 * 参  数:    filename - 快照文件名, 例如: snap1.bmp, /home/dev/snap2.bmp
 * 返回值:    0.成功, 1.参数错误, 2.设备未打开, 3.不支持颜色所占字节数, 4.文件保存失败
 */
int fbop_snapshot(const char* filename);

/*********************************************************************
****************************** 扩展接口 ******************************
**********************************************************************/
/*
 * 功  能:    设置16位背景图像数据
 * 参  数:    rgb - 16位图像数据
 * 返回值:    void
 */
void fbop_set_draw_bg16(unsigned short rgb);

/*
 * 功  能:    设置32位背景图像数据
 * 参  数:    rgb - 32位图像数据
 * 返回值:    void
 */
void fbop_set_draw_bg32(unsigned int rgb);

/*
 * 功  能:    绘制16位图像数据
 * 参  数:    x - 像素点x坐标
 *            y - 像素点y坐标
 *            data - 16位图像数据
 *            width - 图像宽
 *            height - 图像高
 * 返回值:    0.成功, 1.设备未打开, 2.坐标溢出, 3.不支持颜色所占字节数
 */
int fbop_draw_raw_data16(int x, int y, const unsigned short* data, int width, int height);

/*
 * 功  能:    绘制32位图像数据
 * 参  数:    x - 像素点x坐标
 *            y - 像素点y坐标
 *            data - 32位图像数据
 *            width - 图像宽
 *            height - 图像高
 * 返回值:    0.成功, 1.设备未打开, 2.坐标溢出, 3.不支持颜色所占字节数
 */
int fbop_draw_raw_data32(int x, int y, const unsigned int* data, int width, int height);

#ifdef __cplusplus
}
#endif

#endif

