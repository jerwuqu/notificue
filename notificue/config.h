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

	// Font
	char fontName[256];
	uint8_t fontSize;

	// Margin
	int32_t textBoxMargin, titleBodyMargin;

	// Color
	uint32_t backgroundColor, borderColor, textColor;

	// Display
	int32_t displayTime; // todo
} NotificueConfig;

NotificueConfig* config_load();
NotificueConfig* config_get();

#define CONFIG_DEFAULT "# notificue configuration\n\n"\
		"[notificue]\n"\
		"version="CONFIG_VERSION"\n\n"\
		"[location]\n"\
		"screen_x=25\n"\
		"screen_y=25\n"\
		"notification_margin=5\n\n"\
		"[size]\n"\
		"min_width=200\n"\
		"max_width=400\n\n"\
		"[text]\n"\
		"font_name=Arial\n"\
		"font_size=-14\n"\
		"box_margin=15\n"\
		"title_body_margin=5\n\n"\
		"[color]\n"\
		"background=00AAFF\n"\
		"border=000000\n"\
		"text=000000\n\n"\
		"[time]\n"\
		"display_time=0\n"

#define CONFIG_DEFAULT_LEN (sizeof(CONFIG_DEFAULT) - 1)
