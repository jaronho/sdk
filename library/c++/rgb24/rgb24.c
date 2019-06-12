/**********************************************************************
* Author:	jaron.ho
* Date:		2018-09-25
* Brief:	rgb24数据处理,PS:rgb24坐标系原点在左上角
**********************************************************************/
#include "rgb24.h"

unsigned char* rgb24Clip(const unsigned char* rgb24, int width, int height, int x, int y, int w, int h) {
    assert(rgb24);
    assert(width > 0);
    assert(height > 0);
    assert(x >= 0 && x < width && y >= 0 && y < height);
    assert(w > 0 && w <= width && h > 0 && h <= height);
    assert(x + w <= width && y + h <= height);
    int i = 0, j = 0, index = 0, idx = 0;
    unsigned char* clip = (unsigned char*)calloc(w * h * 3, sizeof(unsigned char));
    for (i = 0; i < h; ++i) {
        for (j = 0; j < w; ++j) {
            index = (i * w + j) * 3;
            idx = ((i + y) * width + j + x) * 3;
            clip[index] = rgb24[idx];
            clip[index + 1] = rgb24[idx + 1];
            clip[index + 2] = rgb24[idx + 2];
        }
    }
    return clip;
}

unsigned char* rgb24Grayscale(unsigned char* rgb24, int width, int height, int clone, int* min, int* max) {
	assert(rgb24);
	assert(width > 0);
	assert(height > 0);
	int i = 0, j = 0, index = 0;
    int r = 0, g = 0, b = 0, grayvalue = 0, minValue = -1, maxValue = -1;
    unsigned char* tmp = rgb24;
    if (clone) {
        tmp = (unsigned char*)calloc(width * height * 3, sizeof(unsigned char));
    }
    for (i = 0; i < height; ++i) {
        for (j = 0; j < width; ++j) {
            index = (i * width + j) * 3;
            r = (int)rgb24[index];
            g = (int)rgb24[index + 1];
            b = (int)rgb24[index + 2];
            grayvalue = (int)(0.299 * r + 0.587 * g + 0.114 * b);	/* 灰度值 */
            tmp[index] = tmp[index + 1] = tmp[index + 2] = (unsigned char)grayvalue;
            if (-1 == minValue || grayvalue < minValue) {
                minValue = grayvalue;
            }
            if (-1 == maxValue || grayvalue > maxValue) {
                maxValue = grayvalue;
            }
        }
    }
    if (min) {
        *min = minValue;
    }
    if (max) {
        *max = maxValue;
    }
    return tmp;
}

int* rgb24GrayscaleHistogram(const unsigned char* rgb24, int width, int height, int grayscaled) {
    assert(rgb24);
	assert(width > 0);
	assert(height > 0);
	int i = 0, j = 0, index = 0;
	int r = 0, g = 0, b = 0, grayvalue = 0;
    int* hist = (int*)calloc(256, sizeof(int));
	if (grayscaled) {	/* 已灰度化 */
		for (i = 0; i < height; ++i) {
			for (j = 0; j < width; ++j) {
                index = (i * width + j) * 3;
                grayvalue = rgb24[index];
				hist[grayvalue]++;
			}
		}
	} else {	/* 未灰度化 */
		for (i = 0; i < height; ++i) {
			for (j = 0; j < width; ++j) {
                index = (i * width + j) * 3;
                r = (int)rgb24[index];
                g = (int)rgb24[index + 1];
                b = (int)rgb24[index + 2];
				grayvalue = (int)(0.299 * r + 0.587 * g + 0.114 * b);	// 灰度值
				hist[grayvalue]++;
			}
		}
	}
    return hist;
}

int rgb24BinarizationThresholdOTSU(const int* hist) {
    assert(hist);
	int i = 0, t = 0, maxT = 0;
	int sum = 0, count = 0;
    float u0 = 0, u1 = 0, w0 = 0, w1 = 0;
	float devi, maxDevi = 0; /* 方差及最大方差 */
	for (i = 0; i < 256; ++i) {
		sum += hist[i];
	}
	for (t = 0; t < 255; t++) {
		count = 0;
		u0 = 0;
		/* 阈值为t时,c0组的均值及产生的概率 */
		for (i = 0; i <= t; ++i) {
			count += hist[i];
			u0 += i * hist[i];
		}
		u0 = u0 / count;
		w0 = (float)count/sum;
		/* 阈值为t时,c1组的均值及产生的概率 */
		u1 = 0;
		for (i = t + 1; i < 256; ++i) {
			u1 += i * hist[i];
		}
		u1 = u1 / (sum - count);
		w1 = 1 - w0;
		/* 两类间方差 */
		devi = w0 * w1 * (u1 - u0) * (u1 - u0);
		/* 记录最大的方差及最佳位置 */
		if (devi > maxDevi) {
			maxDevi = devi;
			maxT = t;
		}
	}
	return maxT;
}

int rgb24BinarizationThreshold(int minGray, int maxGray, float t) {
    assert(minGray >= 0 && minGray <= 255);
    assert(maxGray >= 0 && maxGray <= 255);
    assert(t > 0 && t < 1);
    return (int)(minGray * (1 - t) + maxGray * t);
}

unsigned char* rgb24Binarization(unsigned char* rgb24, int width, int height, int threshold, int clone, int* count) {
	assert(rgb24);
    assert(width > 0);
    assert(height > 0);
    assert(threshold > 0 && threshold < 255);
    assert(count);
	int i = 0, j = 0, index = 0;
	int r = 0, g = 0, b = 0, value = 0;
    unsigned char* tmp = rgb24;
    if (clone) {
        tmp = (unsigned char*)calloc(width * height * 3, sizeof(unsigned char));
    }
    for (i = 0; i < height; ++i) {
        for (j = 0; j < width; ++j) {
            index = (i * width + j) * 3;
            r = (int)rgb24[index];
            g = (int)rgb24[index + 1];
            b = (int)rgb24[index + 2];
            value = 0;
            if ((r + g + b) / 3 >= threshold) {
                value = 255;
                ++(*count);
            }
            tmp[index] = tmp[index + 1] = tmp[index + 2] = (unsigned char)value;
        }
    }
    return tmp;
}

unsigned char* rgb24ByPositions(const int* positions, int count, int csys, int width, int height, int bgR, int bgG, int bgB, int r, int g, int b) {
    assert(positions);
    assert(count > 0);
    assert(1 == csys || 2 == csys);
    assert(width > 0);
    assert(height > 0);
    assert(bgR >= 0 && bgR <= 255);
    assert(bgG >= 0 && bgG <= 255);
    assert(bgB >= 0 && bgB <= 255);
    assert(r >= 0 && r <= 255);
    assert(g >= 0 && g <= 255);
    assert(b >= 0 && b <= 255);
    int i = 0, j = 0, index = 0;
    int hit = 0, k = 0, idx = 0, x = 0, y = 0;
    unsigned char* rgb24 = (unsigned char*)calloc(width * height * 3, sizeof(unsigned char));
    for (i = 0; i < height; ++i) {
        for (j = 0; j < width; ++j) {
            index = (i * width + j) * 3;
            hit = 0;
            for (k = 0; k < count; ++k) {
                idx = k * 2;
                x = positions[idx];
                y = positions[idx + 1];
                if (2 == csys) {
                    y = height - y;
                }
                if (x == j && y == i) {
                    hit = 1;
                    break;
                }
            }
            if (hit) {
                rgb24[index] = (unsigned char)r;
                rgb24[index + 1] = (unsigned char)g;
                rgb24[index + 2] = (unsigned char)b;
            } else {
                rgb24[index] = (unsigned char)bgR;
                rgb24[index + 1] = (unsigned char)bgG;
                rgb24[index + 2] = (unsigned char)bgB;
            }
        }
    }
    return rgb24;
}
