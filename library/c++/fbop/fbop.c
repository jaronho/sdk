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
****************************** 变量定义 ******************************
**********************************************************************/
static int l_opened = 0;
static int l_screen_width = 0;
static int l_screen_height = 0;
static int l_pixel_bits = 0;
static int l_color_bytes = 0;
static unsigned long l_buffer_size = 0;
static void* l_buffer = 0;

/*********************************************************************
****************************** 接口实现 ******************************
**********************************************************************/
int fbop_open(const char* fbPath) {
    int fbfd;
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
    if (1 == l_opened) {
        return 0;
    }
    if (!fbPath || 0 == strlen(fbPath)) {
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
    l_screen_width = vinfo.xres;
    l_screen_height = vinfo.yres;
    l_pixel_bits = vinfo.bits_per_pixel;
    l_color_bytes = l_pixel_bits / 8;
    l_buffer_size = (unsigned long)l_screen_width * l_screen_height * l_color_bytes;
    l_buffer = mmap(0, l_buffer_size, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
    if (-1 == (int)l_buffer) {
        printf("Cannot map framebuffer device to memory, fb device: %s\n", fbPath);
        close(fbfd);
        return 5;
    }
    close(fbfd);
    l_opened = 1;
    return 0;
}

void fbop_close(void) {
    if (1 == l_opened) {
        munmap(l_buffer, l_buffer_size);
        l_buffer = 0;
        l_screen_width = 0;
        l_screen_height = 0;
        l_pixel_bits = 0;
        l_color_bytes = 0;
        l_buffer_size = 0;
        l_opened = 0;
    }
}

int fbop_screen_width(void) {
    return l_screen_width;
}

int fbop_screen_height(void) {
    return l_screen_height;
}

int fbop_pixel_bits(void) {
    return l_pixel_bits;
}

int fbop_color_bytes(void) {
    return l_color_bytes;
}

unsigned long fbop_size(void) {
    return l_buffer_size;
}

void* fbop_buffer(void) {
    return l_buffer;
}

int fbop_get_pixel(int x, int y, unsigned int* rgb) {
    if (0 == l_opened) {
        printf("Cannot get pixel, device not open\n");
        return 1;
    }
    if (x >= l_screen_width || y >= l_screen_height) {
        printf("Cannot get pixel: (%d, %d), overflow screen area: (%d, %d)\n", x, y, l_screen_width, l_screen_height);
        return 2;
    }
    if (2 == l_color_bytes) {
        if (rgb) {
            *rgb = GL_RGB_16_to_32(((unsigned short*)l_buffer)[y * l_screen_width + x]);
        }
	} else if (4 == l_color_bytes) {
        if (rgb) {
		    *rgb = ((unsigned int*)l_buffer)[y * l_screen_width + x];
        }
	} else {
        printf("Cannot get pixel, not support color bytes: %d\n", l_color_bytes);
        return 3;
    }
    return 0;
}

int fbop_set_pixel16(int x, int y, unsigned short rgb) {
    if (0 == l_opened) {
        printf("Cannot set pixel 16, device not open\n");
        return 1;
    }
    if (x < 0 || x >= l_screen_width || y < 0 || y >= l_screen_height) {
        printf("Cannot set pixel 16: (%d, %d), overflow screen area: (%d, %d)\n", x, y, l_screen_width, l_screen_height);
        return 2;
    }
    if (2 == l_color_bytes) {
		((unsigned short*)l_buffer)[y * l_screen_width + x] = rgb;
	} else if (4 == l_color_bytes) {
		((unsigned int*)l_buffer)[y * l_screen_width + x] = GL_RGB_16_to_32(rgb);
	} else {
        printf("Cannot set pixel 16, not support color bytes: %d\n", l_color_bytes);
        return 3;
    }
    return 0;
}

int fbop_set_pixel32(int x, int y, unsigned int rgb) {
    if (0 == l_opened) {
        printf("Cannot set pixel 32, device not open\n");
        return 1;
    }
    if (x < 0 || x >= l_screen_width || y < 0 || y >= l_screen_height) {
        printf("Cannot set pixel 32: (%d, %d), overflow screen area: (%d, %d)\n", x, y, l_screen_width, l_screen_height);
        return 2;
    }
    if (2 == l_color_bytes) {
		((unsigned short*)l_buffer)[y * l_screen_width + x] = GL_RGB_32_to_16(rgb);
	} else if (4 == l_color_bytes) {
		((unsigned int*)l_buffer)[y * l_screen_width + x] = rgb;
	} else {
        printf("Cannot set pixel 32, not support color bytes: %d\n", l_color_bytes);
        return 3;
    }
    return 0;
}

int fbop_snapshot(const char* filename) {
    unsigned short* p_bmp565_data;
    unsigned int* p_raw_data;
    int i;
    int ret;
    unsigned short rgb;
    if (0 == l_opened) {
        printf("Cannot snapshot, device not open\n");
        return 1;
    }
    if (!filename || 0 == strlen(filename)) {
        return 2;
    }
	if (2 == l_color_bytes) {   //16 bits framebuffer
		if (-1 == generateBmpFile(filename, l_screen_width, l_screen_height, (unsigned char*)l_buffer)) {
            printf("Cannot snapshot, open file: %s fail\n", filename);
            return 4;
        }
	} else if (4 == l_color_bytes) {    //32 bits framebuffer
	    p_bmp565_data =  (unsigned short*)malloc(l_screen_width * l_screen_height * sizeof(unsigned short));
	    p_raw_data = (unsigned int*)l_buffer;
	    for (i = 0; i < l_screen_width * l_screen_height; ++i) {
            rgb = *p_raw_data++;
		    p_bmp565_data[i] = GL_RGB_32_to_16(rgb);
	    }
	    ret = generateBmpFile(filename, l_screen_width, l_screen_height, (unsigned char*)p_bmp565_data);
        free(p_bmp565_data);
        if (-1 == ret) {
            printf("Cannot snapshot, open file: %s fail\n", filename);
            return 4;
        }
    } else {
        printf("Cannot snapshot, not support color bytes: %d\n", l_color_bytes);
        return 3;
    }
	return 0;
}

/*********************************************************************
****************************** 扩展接口 ******************************
**********************************************************************/
static unsigned short l_draw_bg16 = 0;
static unsigned int l_draw_bg32 = 0;
static int l_draw_bg_bit = 16;

void fbop_set_draw_bg16(unsigned short rgb) {
    l_draw_bg16 = rgb;
    l_draw_bg_bit = 16;
}

void fbop_set_draw_bg32(unsigned int rgb) {
    l_draw_bg32 = rgb;
    l_draw_bg_bit = 32;
}

int fbop_draw_raw_data16(int x, int y, const unsigned short* data, int width, int height) {
    int px, py;
    int dx, dy;
    int ret;
    for (py = 0; py < l_screen_height; ++py) {
        for (px = 0; px < l_screen_width; ++px) {
            if (py < y || py >= y + height || px < x || px >= x + width) {
                if (16 == l_draw_bg_bit) {
                    ret = fbop_set_pixel16(px, py, l_draw_bg16);
                } else {
                    ret = fbop_set_pixel32(px, py, l_draw_bg32);
                }
                if (0 != ret) {
                    return ret;
                }
            } else {
                dx = px - x;
                dy = py - y;
                ret = fbop_set_pixel16(px, py, data[dy * width + dx]);
                if (0 != ret) {
                    return ret;
                }
            }
        }
    }
    return 0;
}

int fbop_draw_raw_data32(int x, int y, const unsigned int* data, int width, int height) {
    int px, py;
    int dx, dy;
    int ret;
    for (py = 0; py < l_screen_height; ++py) {
        for (px = 0; px < l_screen_width; ++px) {
            if (py < y || py >= y + height || px < x || px >= x + width) {
                if (16 == l_draw_bg_bit) {
                    ret = fbop_set_pixel16(px, py, l_draw_bg16);
                } else {
                    ret = fbop_set_pixel32(px, py, l_draw_bg32);
                }
                if (0 != ret) {
                    return ret;
                }
            } else {
                dx = px - x;
                dy = py - y;
                ret = fbop_set_pixel32(px, py, data[dy * width + dx]);
                if (0 != ret) {
                    return ret;
                }
            }
        }
    }
    return 0;
}

