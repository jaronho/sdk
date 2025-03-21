﻿#include "GuiLite.h"
#include <stdlib.h>
#include <string.h>

const int UI_WIDTH = 240;
const int UI_HEIGHT = 320;
const unsigned char	s_wave_data1[] =
{
	0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D,	0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D,	0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D,	0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D,
	0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D,	0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D,	0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D,	0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D,
	0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D,	0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D,	0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D,	0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D,
	0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D,	0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D,	0x7E, 0x7E, 0x7F, 0x81, 0x81, 0x82, 0x82, 0x83,	0x84, 0x84, 0x84, 0x84, 0x83, 0x82, 0x82, 0x81,
	0x81, 0x7F, 0x7F, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D,	0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D,	0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D,	0x7D, 0x7D, 0x7C, 0x7B, 0x7A, 0x79, 0x7A, 0x82,
	0x89, 0x91, 0x98, 0x9F, 0xA2, 0x9B, 0x94, 0x8C,	0x85, 0x7E, 0x78, 0x79, 0x7A, 0x7B, 0x7D, 0x7D,	0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D,	0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D,
	0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D,	0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D,	0x7E, 0x7F, 0x7F, 0x81, 0x81, 0x82, 0x83, 0x84,	0x84, 0x85, 0x86, 0x87, 0x87, 0x87, 0x88, 0x88,
	0x89, 0x89, 0x89, 0x8A, 0x8A, 0x8A, 0x8A, 0x89,	0x89, 0x88, 0x88, 0x87, 0x87, 0x85, 0x85, 0x85,	0x83, 0x82, 0x81, 0x81, 0x7F, 0x7E, 0x7E, 0x7D,	0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D
};
const char s_wave_data2[] =
{
	0x21, 0x22, 0x23, 0x22, 0x21, 0x20, 0x1E, 0x1C,	0x1B, 0x19, 0x18, 0x16, 0x14, 0x12, 0x11, 0x10,	0x0F, 0x0D, 0x0C, 0x0A, 0x09, 0x07, 0x06, 0x05,	0x04, 0x03, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00,
	0x01, 0x06, 0x0E, 0x18, 0x23, 0x2D, 0x37, 0x40,	0x49, 0x51, 0x57, 0x5A, 0x5A, 0x59, 0x55, 0x50,	0x49, 0x42, 0x3B, 0x34, 0x2F, 0x29, 0x24, 0x20,	0x1F, 0x1e, 0x1d, 0x1f
};
const unsigned char s_wave_data3[] =
{
	0x53, 0x53, 0x53, 0x52, 0x53, 0x53, 0x52, 0x52,	0x52, 0x52, 0x52, 0x52, 0x52, 0x52, 0x52, 0x52,	0x53, 0x53, 0x53, 0x53, 0x53, 0x53, 0x53, 0x53,	0x53, 0x54, 0x54, 0x54, 0x54, 0x55, 0x55, 0x55,
	0x55, 0x56, 0x57, 0x57, 0x58, 0x59, 0x5A, 0x5B,	0x5C, 0x5D, 0x5F, 0x60, 0x62, 0x64, 0x66, 0x68,	0x6A, 0x6C, 0x6E, 0x70, 0x72, 0x75, 0x77, 0x79,	0x7B, 0x7D, 0x80, 0x82, 0x84, 0x87, 0x89, 0x8B,
	0x8D, 0x8F, 0x91, 0x93, 0x94, 0x96, 0x97, 0x98,	0x9A, 0x9B, 0x9C, 0x9C, 0x9D, 0x9E, 0x9E, 0x9E,	0x9E, 0x9F, 0x9F, 0x9F, 0x9F, 0x9F, 0x9F, 0x9F,	0x9F, 0x9E, 0x9F, 0x9E, 0x9E, 0x9E, 0x9E, 0x9E,
	0x9E, 0x9E, 0x9D, 0x9D, 0x9D, 0x9D, 0x9C, 0x9C,	0x9C, 0x9C, 0x9B, 0x9B, 0x9A, 0x9A, 0x9A, 0x99,	0x99, 0x98, 0x98, 0x98, 0x97, 0x97, 0x96, 0x96,	0x96, 0x95, 0x95, 0x94, 0x94, 0x93, 0x93, 0x92,
	0x92, 0x91, 0x91, 0x90, 0x90, 0x8F, 0x8F, 0x8E,	0x8E, 0x8D, 0x8D, 0x8C, 0x8B, 0x8B, 0x8A, 0x8A,	0x89, 0x89, 0x88, 0x88, 0x87, 0x87, 0x86, 0x85,	0x85, 0x85, 0x84, 0x83, 0x83, 0x82, 0x82, 0x81,
	0x81, 0x80, 0x7F, 0x7F, 0x7E, 0x7D, 0x7D, 0x7C,	0x7B, 0x7B, 0x7A, 0x7A, 0x79, 0x78, 0x77, 0x77,	0x76, 0x75, 0x74, 0x74, 0x73, 0x72, 0x71, 0x71,	0x70, 0x6F, 0x6F, 0x6E, 0x6D, 0x6D, 0x6C, 0x6B,
	0x6A, 0x6A, 0x69, 0x69, 0x68, 0x67, 0x66, 0x66,	0x65, 0x65, 0x64, 0x64, 0x63, 0x63, 0x62, 0x62,	0x61, 0x61, 0x61, 0x60, 0x60, 0x5F, 0x5F, 0x5F,	0x5E, 0x5E, 0x5D, 0x5D, 0x5D, 0x5C, 0x5C, 0x5C,
	0x5B, 0x5B, 0x5B, 0x5B, 0x5B, 0x5A, 0x5A, 0x5A,	0x59, 0x59, 0x59, 0x58, 0x58, 0x57, 0x57, 0x57,	0x56, 0x56, 0x55, 0x55, 0x55, 0x55, 0x54, 0x54,	0x54, 0x54, 0x54, 0x54, 0x53, 0x54, 0x53, 0x53,
};

//////////////////////// define widgets & map message ////////////////////////
enum WND_ID
{
	ID_ROOT = 1,
	ID_WAVE1,
	ID_WAVE2,
	ID_WAVE3,
	ID_BUTTON
};

class c_myUI : public c_wnd
{
	virtual c_wnd* clone() { return new c_myUI(); }
	virtual void on_init_children(void) {
		c_wave_ctrl *p_wave = (c_wave_ctrl*)get_wnd_ptr(ID_WAVE1);
		p_wave->set_wave_speed(1);
		p_wave->set_wave_color(GL_RGB(0, 255, 0));
		p_wave->set_wave_in_out_rate(256, 17);
		p_wave->set_wave(&m_wave_buffer1);
		p_wave->set_max_min(170, 110);

		p_wave = (c_wave_ctrl*)get_wnd_ptr(ID_WAVE2);
		p_wave->set_wave_speed(1);
		p_wave->set_wave_color(GL_RGB(41, 189, 189));
		p_wave->set_wave_in_out_rate(60, 17);
		p_wave->set_wave(&m_wave_buffer2);
		p_wave->set_max_min(100, 0);

		p_wave = (c_wave_ctrl*)get_wnd_ptr(ID_WAVE3);
		p_wave->set_wave_speed(1);
		p_wave->set_wave_color(GL_RGB(255, 255, 0));
		p_wave->set_wave_in_out_rate(128, 17);
		p_wave->set_wave(&m_wave_buffer3);
		p_wave->set_max_min(180, 80);
	}
	virtual void on_paint(void)
	{
		c_rect rect;
		get_screen_rect(rect);
		m_surface->fill_rect(rect, GL_RGB(33, 33, 33), m_z_order);
	}
	void on_clicked(unsigned int ctrl_id) {
		static int index;
		c_rect rect;
		get_screen_rect(rect);
		c_button* button = (c_button*)get_wnd_ptr(ID_BUTTON);
		switch (++index % 3)
		{
		case 0:
			button->set_str("GuiLite");
			break;
		case 1:
			button->set_str("\xe2\x9d\xa4");
			break;
		default:
			button->set_str("\xe5\x8d\x95\xe7\x89\x87\xe6\x9c\xba");
			break;
		}
		button->show_window();
	}
public:
	c_wave_buffer m_wave_buffer1;
	c_wave_buffer m_wave_buffer2;
	c_wave_buffer m_wave_buffer3;
	GL_DECLARE_MESSAGE_MAP()//delcare message
};

//map message
GL_BEGIN_MESSAGE_MAP(c_myUI)
ON_GL_BN_CLICKED(ID_BUTTON, c_myUI::on_clicked)
GL_END_MESSAGE_MAP()

//////////////////////// layout UI ////////////////////////
static c_myUI s_myUI;
static c_wave_ctrl s_wave1;
static c_wave_ctrl s_wave2;
static c_wave_ctrl s_wave3;
static c_button s_button;
static WND_TREE s_myUI_children[] =
{
	{&s_wave1, ID_WAVE1, 0, 0, 0, 240, 90, NULL},
	{&s_wave2, ID_WAVE2, 0, 0, 95, 240, 90, NULL},
	{&s_wave3, ID_WAVE3, 0, 0, 190, 240, 90, NULL},
	{&s_button, ID_BUTTON, "GuiLite", 60, 280, 120, 40, NULL},
	{ NULL,0,0,0,0,0,0 }
};

//////////////////////// start UI ////////////////////////
extern const FONT_INFO Lucida_Console_27;
void load_resource()
{
	c_theme::add_font(FONT_DEFAULT, &Lucida_Console_27);
	//for button
	c_theme::add_color(COLOR_WND_FONT, GL_RGB(255, 0, 0));
	c_theme::add_color(COLOR_WND_NORMAL, GL_RGB(59, 75, 94));
	c_theme::add_color(COLOR_WND_PUSHED, GL_RGB(33, 42, 53));
	c_theme::add_color(COLOR_WND_FOCUS, GL_RGB(33, 42, 53));
	c_theme::add_color(COLOR_WND_BORDER, GL_RGB(46, 59, 73));
}

static c_display* s_display;
void create_ui(void* phy_fb, int screen_width, int screen_height, int color_bytes, struct EXTERNAL_GFX_OP* gfx_op)
{
	load_resource();
	s_display = new c_display(phy_fb, screen_width, screen_height, UI_WIDTH, UI_HEIGHT, color_bytes, 1, gfx_op);
	c_surface* surface = s_display->alloc_surface(Z_ORDER_LEVEL_0);
	surface->set_active(true);
	s_myUI.set_surface(surface);
	s_myUI.connect(NULL, ID_ROOT, 0, 0, 0, UI_WIDTH, UI_HEIGHT, s_myUI_children);
	s_myUI.show_window();

	unsigned int data_index1, data_index2, data_index3;
	data_index1 = data_index2 = data_index3 = 0;
	unsigned int wave_index = 0;
	while(1)
	{
		s_myUI.m_wave_buffer1.write_wave_data(s_wave_data1[data_index1++%sizeof(s_wave_data1)]);
		s_myUI.m_wave_buffer1.write_wave_data(s_wave_data1[data_index1++ % sizeof(s_wave_data1)]);
		s_myUI.m_wave_buffer1.write_wave_data(s_wave_data1[data_index1++ % sizeof(s_wave_data1)]);
		s_myUI.m_wave_buffer1.write_wave_data(s_wave_data1[data_index1++ % sizeof(s_wave_data1)]);

		s_myUI.m_wave_buffer2.write_wave_data(s_wave_data2[data_index2++%sizeof(s_wave_data2)]);
		
		s_myUI.m_wave_buffer3.write_wave_data(s_wave_data3[data_index3++ % sizeof(s_wave_data3)]);
		s_myUI.m_wave_buffer3.write_wave_data(s_wave_data3[data_index3++ % sizeof(s_wave_data3)]);
		s_wave1.refresh_wave(wave_index);
		s_wave2.refresh_wave(wave_index);
		s_wave3.refresh_wave(wave_index);
		wave_index++;
		thread_sleep(17);
	}
}

//////////////////////// interface for all platform ////////////////////////
extern "C" void startHelloWave(void* phy_fb, int width, int height, int color_bytes, struct EXTERNAL_GFX_OP* gfx_op)
{
	create_ui(phy_fb, width, height, color_bytes, gfx_op);
}

void sendTouch2HelloWave(int x, int y, bool is_down)
{
	is_down ? s_myUI.on_touch(x, y, TOUCH_DOWN) : s_myUI.on_touch(x, y, TOUCH_UP);
}

int captureUiOfHelloWave()
{
	return s_display->snap_shot("snap_short.bmp");
}
