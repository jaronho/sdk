/**********************************************************************
* Author:	jaron.ho
* Date:		2019-11-05
* Brief:	封装了基于linux中framebuffer的操作接口
**********************************************************************/
#include "fbop.h"

/*********************************************************************
***************************** Bitmap文件 *****************************
**********************************************************************/
typedef struct {
	unsigned short	bfType;
	unsigned int   	bfSize;
	unsigned short  bfReserved1;
	unsigned short  bfReserved2;
	unsigned int   	bfOffBits;
} __attribute__((packed)) BmpFileHead;

typedef struct {
	unsigned int  	biSize;
	int 			biWidth;
	int       		biHeight;
	unsigned short	biPlanes;
	unsigned short  biBitCount;
	unsigned int    biCompress;
	unsigned int    biSizeImage;
	int       		biXPelsPerMeter;
	int       		biYPelsPerMeter;
	unsigned int 	biClrUsed;
	unsigned int    biClrImportant;
	unsigned int 	biRedMask;
	unsigned int 	biGreenMask;
	unsigned int 	biBlueMask;
} __attribute__((packed)) BmpInfoHead;

int generateBmpFile(const char* filename, unsigned int width, unsigned int height, unsigned char* data) {
	BmpFileHead bmp_head;
	BmpInfoHead bmp_info;
	int size;
    FILE* fp;
    int i;
    size = width * height * 2;
	//initialize bmp head.
	bmp_head.bfType = 0x4d42;
	bmp_head.bfSize = size + sizeof(BmpFileHead) + sizeof(BmpInfoHead);
	bmp_head.bfReserved1 = bmp_head.bfReserved2 = 0;
	bmp_head.bfOffBits = bmp_head.bfSize - size;
	//initialize bmp info.
	bmp_info.biSize = 40;
	bmp_info.biWidth = width;
	bmp_info.biHeight = height;
	bmp_info.biPlanes = 1;
	bmp_info.biBitCount = 16;
	bmp_info.biCompress = 3;
	bmp_info.biSizeImage = size;
	bmp_info.biXPelsPerMeter = 0;
	bmp_info.biYPelsPerMeter = 0;
	bmp_info.biClrUsed = 0;
	bmp_info.biClrImportant = 0;
	//RGB565
	bmp_info.biRedMask = 0xF800;
	bmp_info.biGreenMask = 0x07E0;
	bmp_info.biBlueMask = 0x001F;
	//copy the data
	if (!(fp = fopen(filename, "wb"))) {
		return -1;
	}
	fwrite(&bmp_head, 1, sizeof(BmpFileHead), fp);
	fwrite(&bmp_info, 1, sizeof(BmpInfoHead), fp);
	for (i = height - 1; i >= 0; --i) {
		fwrite(&data[i * width * 2], 1, width * 2, fp);
	}
	fclose(fp);
	return 0;
}

/*********************************************************************
****************************** 接口实现 ******************************
**********************************************************************/
int fbop_open(const char* fbPath, st_frame_buffer** fbPtr) {
    int fbfd;
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
    st_frame_buffer* fb;
    if (!fbPath || 0 == strlen(fbPath) || !fbPtr) {
        printf("Param error\n");
        return 1;
    }
    fbfd = open(fbPath, O_RDWR);
    if (fbfd < 0) {
        printf("Cannot open fb device: %s\n", fbPath);
        close(fbfd);
        return 2;
    }
    if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo)) {
        printf("Cannot read fixed information, fb device: %s\n", fbPath);
        close(fbfd);
        return 3;
    }
    if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo)) {
        printf("Cannot read variable information, fb device: %s\n", fbPath);
        close(fbfd);
        return 4;
    }
    fb = (st_frame_buffer*)malloc(sizeof(st_frame_buffer));
    fb->width = vinfo.xres;
    fb->height = vinfo.yres;
    fb->pixel_bits = vinfo.bits_per_pixel;
    fb->color_bytes = fb->pixel_bits / 8;
    fb->size = (unsigned long)fb->width * fb->height * fb->color_bytes;
    fb->data = mmap(0, fb->size, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
    if (-1 == (int)fb->data) {
        printf("Cannot map framebuffer device to memory, fb device: %s\n", fbPath);
        close(fbfd);
        free(fb);
        return 5;
    }
    close(fbfd);
    *fbPtr = fb;
    return 0;
}

void fbop_close(st_frame_buffer** fbPtr) {
    st_frame_buffer* fb;
    if (fbPtr && *fbPtr) {
        fb = *fbPtr;
        if (fb->data) {
            munmap(fb->data, fb->size);
        }
        fb->width = 0;
        fb->height = 0;
        fb->pixel_bits = 0;
        fb->color_bytes = 0;
        fb->size = 0;
        fb->data = 0;
        fb->draw_bg16 = 0;
        fb->draw_bg32 = 0;
        fb->draw_bg_bit = 0;
        free(fb);
        fb = 0;
        *fbPtr = fb;
    }
}

int fbop_get_pixel(st_frame_buffer* fb, int x, int y, unsigned int* rgb) {
    if (!fb || !fb->data || 0 == fb->size) {
        printf("Cannot get pixel, device not open\n");
        return 1;
    }
    if (x < 0 || x >= fb->width || y < 0 || y >= fb->height) {
        printf("Cannot get pixel: (%d, %d), overflow screen area: (%d, %d)\n", x, y, fb->width, fb->height);
        return 2;
    }
    if (2 == fb->color_bytes) {
        if (rgb) {
            *rgb = GL_RGB_16_to_32(((unsigned short*)fb->data)[y * fb->width + x]);
        }
	} else if (4 == fb->color_bytes) {
        if (rgb) {
		    *rgb = ((unsigned int*)fb->data)[y * fb->width + x];
        }
	} else {
        printf("Cannot get pixel, not support color bytes: %d\n", fb->color_bytes);
        return 3;
    }
    return 0;
}

int fbop_set_pixel16(st_frame_buffer* fb, int x, int y, unsigned short rgb) {
    if (!fb || !fb->data || 0 == fb->size) {
        printf("Cannot set pixel 16, device not open\n");
        return 1;
    }
    if (x < 0 || x >= fb->width || y < 0 || y >= fb->height) {
        printf("Cannot set pixel 16: (%d, %d), overflow screen area: (%d, %d)\n", x, y, fb->width, fb->height);
        return 2;
    }
    if (2 == fb->color_bytes) {
		((unsigned short*)fb->data)[y * fb->width + x] = rgb;
	} else if (4 == fb->color_bytes) {
		((unsigned int*)fb->data)[y * fb->width + x] = GL_RGB_16_to_32(rgb);
	} else {
        printf("Cannot set pixel 16, not support color bytes: %d\n", fb->color_bytes);
        return 3;
    }
    return 0;
}

int fbop_set_pixel32(st_frame_buffer* fb, int x, int y, unsigned int rgb) {
    if (!fb || !fb->data || 0 == fb->size) {
        printf("Cannot set pixel 32, device not open\n");
        return 1;
    }
    if (x < 0 || x >= fb->width || y < 0 || y >= fb->height) {
        printf("Cannot set pixel 32: (%d, %d), overflow screen area: (%d, %d)\n", x, y, fb->width, fb->height);
        return 2;
    }
    if (2 == fb->color_bytes) {
		((unsigned short*)fb->data)[y * fb->width + x] = GL_RGB_32_to_16(rgb);
	} else if (4 == fb->color_bytes) {
		((unsigned int*)fb->data)[y * fb->width + x] = rgb;
	} else {
        printf("Cannot set pixel 32, not support color bytes: %d\n", fb->color_bytes);
        return 3;
    }
    return 0;
}

int fbop_snapshot(st_frame_buffer* fb, const char* filename) {
    unsigned short* p_bmp565_data;
    unsigned int* p_raw_data;
    int i;
    int ret;
    unsigned short rgb;
    if (!fb || !fb->data || 0 == fb->size) {
        printf("Cannot snapshot, device not open\n");
        return 1;
    }
    if (!filename || 0 == strlen(filename)) {
        return 2;
    }
	if (2 == fb->color_bytes) {   //16 bits framebuffer
		if (-1 == generateBmpFile(filename, fb->width, fb->height, (unsigned char*)fb->data)) {
            printf("Cannot snapshot, open file: %s fail\n", filename);
            return 4;
        }
	} else if (4 == fb->color_bytes) {    //32 bits framebuffer
	    p_bmp565_data =  (unsigned short*)malloc(fb->width * fb->height * sizeof(unsigned short));
	    p_raw_data = (unsigned int*)fb->data;
	    for (i = 0; i < fb->width * fb->height; ++i) {
            rgb = *p_raw_data++;
		    p_bmp565_data[i] = GL_RGB_32_to_16(rgb);
	    }
	    ret = generateBmpFile(filename, fb->width, fb->height, (unsigned char*)p_bmp565_data);
        free(p_bmp565_data);
        if (-1 == ret) {
            printf("Cannot snapshot, open file: %s fail\n", filename);
            return 4;
        }
    } else {
        printf("Cannot snapshot, not support color bytes: %d\n", fb->color_bytes);
        return 3;
    }
	return 0;
}

/*********************************************************************
****************************** 扩展接口 ******************************
**********************************************************************/
void fbop_set_draw_bg16(st_frame_buffer* fb, unsigned short rgb) {
    if (fb && fb->data) {
        fb->draw_bg16 = rgb;
        fb->draw_bg_bit = 16;
    }
}

void fbop_set_draw_bg32(st_frame_buffer* fb, unsigned int rgb) {
    if (fb && fb->data) {
        fb->draw_bg32 = rgb;
        fb->draw_bg_bit = 32;
    }
}

void fbop_unset_draw_bg(st_frame_buffer* fb) {
    if (fb && fb->data) {
        fb->draw_bg_bit = 0;
    }
}

int fbop_draw_raw_data16(st_frame_buffer* fb, int x, int y, const unsigned short* data, int width, int height) {
    int px, py;
    int dx, dy;
    int ret;
    if (!fb || !fb->data || 0 == fb->size) {
        printf("Cannot draw raw data 16, device not open\n");
        return 1;
    }
    for (py = 0; py < fb->height; ++py) {
        for (px = 0; px < fb->width; ++px) {
            if (py < y || py >= y + height || px < x || px >= x + width) {
                if (16 == fb->draw_bg_bit) {
                    ret = fbop_set_pixel16(fb, px, py, fb->draw_bg16);
                } else if (32 == fb->draw_bg_bit) {
                    ret = fbop_set_pixel32(fb, px, py, fb->draw_bg32);
                } else {
                    continue;
                }
                if (0 != ret) {
                    return ret;
                }
            } else {
                dx = px - x;
                dy = py - y;
                ret = fbop_set_pixel16(fb, px, py, data[dy * width + dx]);
                if (0 != ret) {
                    return ret;
                }
            }
        }
    }
    return 0;
}

int fbop_draw_raw_data32(st_frame_buffer* fb, int x, int y, const unsigned int* data, int width, int height) {
    int px, py;
    int dx, dy;
    int ret;
    if (!fb || !fb->data || 0 == fb->size) {
        printf("Cannot draw raw data 32, device not open\n");
        return 1;
    }
    for (py = 0; py < fb->height; ++py) {
        for (px = 0; px < fb->width; ++px) {
            if (py < y || py >= y + height || px < x || px >= x + width) {
                if (16 == fb->draw_bg_bit) {
                    ret = fbop_set_pixel16(fb, px, py, fb->draw_bg16);
                } else if (32 == fb->draw_bg_bit) {
                    ret = fbop_set_pixel32(fb, px, py, fb->draw_bg32);
                } else {
                    continue;
                }
                if (0 != ret) {
                    return ret;
                }
            } else {
                dx = px - x;
                dy = py - y;
                ret = fbop_set_pixel32(fb, px, py, data[dy * width + dx]);
                if (0 != ret) {
                    return ret;
                }
            }
        }
    }
    return 0;
}

