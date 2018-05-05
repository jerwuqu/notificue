#pragma once

#include <stdint.h>

#define CONFIG_PATH "notificue.cfg"

typedef struct _NotificueConfig
{
	// Location (negative value for opposite side)
	int32_t screenX, screenY;

	// Size
	int32_t minWidth, maxWidth;

	// Font
	char fontName[256];
	uint8_t fontSize;

	// Margin
	int32_t textMargin, titleBodyMargin, notificationMargin;

	// Color
	uint32_t backgroundColor, borderColor, textColor;

	// Display
	uint32_t displayTime; // todo
} NotificueConfig;

NotificueConfig* config_load();
NotificueConfig* config_get();
void config_save();
