/**********************************************************************
* Author:	jaron.ho
* Date:		2018-09-25
* Brief:	rgb24数据处理,PS:rgb24坐标系原点在左上角
**********************************************************************/
#ifndef RGB24_H
#define RGB24_H

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C"
{
#endif

/*
 * 功  能:    裁剪rgb24区域
 * 参  数:    rgb24 - rgb24数据
 *			  width - 图像宽,PS.每1个像素占3个数据位
 *            height - 图像高
 *            x - 横向起始坐标(原点为左上角)
 *            y - 竖向起始坐标(原点为左上角)
 *            w - 需要裁剪的宽
 *            h - 需要裁剪的高
 * 返回值:    unsigned char*
 */
extern unsigned char* rgb24Clip(const unsigned char* rgb24, int width, int height, int x, int y, int w, int h);

/*
 * 功  能:    把rgb24灰度化
 * 参  数:    rgb24 - rgb24数据
 *			  width - 图像宽,PS.每1个像素占3个数据位
 *            height - 图像高
 *            clone - 是否深度拷贝,0.直接修改rgb24,1.新分配内存
 *            min - 最小灰度值
 *            max - 最大灰度值
 * 返回值:    unsigned char*
 */
extern unsigned char* rgb24Grayscale(unsigned char* rgb24, int width, int height, int clone, int* min, int* max);

/*
 * 功  能:    计算rgb24灰度直方图
 * 参  数:    rgb24 - rgb24数据
 *			  width - 图像宽,PS.每1个像素占3个数据位
 *            height - 图像高
 *            grayscaled - 0.rgb24未灰度化,1.rgb24已灰度化
 * 返回值:    int*,256长度
 */
extern int* rgb24GrayscaleHistogram(const unsigned char* rgb24, int width, int height, int grayscaled);

/*
 * 功  能:    计算rgb24二值化阀值(OTSU算法)
 * 参  数:    hist - 灰度直方图,必须为256长度
 * 返回值:    int
 */
extern int rgb24BinarizationThresholdOTSU(const int* hist);

/*
 * 功  能:    计算rgb24二值化阀值
 * 参  数:    minGray - 最小灰度值,值范围[0,255]
 *            maxGray - 最大灰度值,值范围[0,255]
 *            t -  最大灰度值的权重系数,值范围(0,1),值越大,阀值越靠近maxGray
 * 返回值:    int
 */
extern int rgb24BinarizationThreshold(int minGray, int maxGray, float t);

/*
 * 功  能:    把rgb24二值化
 * 参  数:    rgb24 - rgb24数据
 *            width - 图像宽,PS.每1个像素占3个数据位
 *            height - 图像高
 *            threshold - 阀值:(0, 255)
 *            clone - 是否深度拷贝,0.直接修改rgb24,1.新分配内存
 *            count - 值为255的数量
 * 返回值:    unsigned char*
 */
extern unsigned char* rgb24Binarization(unsigned char* rgb24, int width, int height, int threshold, int clone, int* count);

/*
 * 功  能:    根据坐标点生成rgb24数据
 * 参  数:    positions - 坐标点数组
 *            count - 坐标个数,PS.每1个坐标占2个数据位
 *            coord - 坐标系:1.原点在左上角(同rgb24坐标系),2.原点在左下角
 *            width - 图像宽,PS.每1个像素占3个数据位
 *            height - 图像高
 *            bgR - 背景颜色,R分量
 *            bgG - 背景颜色,G分量
 *            bgB - 背景颜色,B分量
 *            r - 坐标点颜色,R分量
 *            g - 坐标点颜色,G分量
 *            b - 坐标点颜色,B分量
 * 返回值:    unsigned char*
 */
extern unsigned char* rgb24ByPositions(const int* positions, int count, int coord, int width, int height, int bgR, int bgG, int bgB, int r, int g, int b);

#ifdef __cplusplus
}
#endif

#endif // RGB24_H
