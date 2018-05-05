#pragma once

#include <stdint.h>

#define CONFIG_PATH "notificue.ini"
#define CONFIG_VERSION "DEV"

typedef struct _NotificueConfig
{
	// Notificue
	char version[16];

	// Location (negative value for opposite side)
	int32_t screenX, screenY;
	int32_t notificationMargin;

	// Size
	int32_t minWidth, maxWidth;

	// Text
	char fontName[256];
	uint8_t fontSize;
	int32_t textBoxMargin, titleBodyMargin;

	// Color
	uint32_t backgroundColor, borderColor, textColor;

	// Time
	int32_t displayTime;

	// Sound
	char soundFile[256];
} NotificueConfig;

NotificueConfig* config_load();
NotificueConfig* config_get();

#define CONFIG_DEFAULT "# notificue configuration\n\n"\
		"[notificue]\n"\
		"version="CONFIG_VERSION"\n\n"\
		"[location]\n"\
		"screen_x=-25\n"\
		"screen_y=25\n"\
		"notification_margin=5\n\n"\
		"[size]\n"\
		"min_width=200\n"\
		"max_width=400\n\n"\
		"[text]\n"\
		"font_name=Arial\n"\
		"font_size=18\n"\
		"box_margin=15\n"\
		"title_body_margin=0\n\n"\
		"[color]\n"\
		"background=FFAA00\n"\
		"border=000000\n"\
		"text=000000\n\n"\
		"[time]\n"\
		"display_time=5000\n\n"\
		"[sound]\n"\
		"filename=\n"

#define CONFIG_DEFAULT_LEN (sizeof(CONFIG_DEFAULT) - 1)
