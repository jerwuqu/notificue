#include "config.h"

#include <stdio.h>
#include <stdlib.h>

static char inited = 0;
static NotificueConfig config;

static void setDefaults()
{
	// Location
	config.screenX = config.screenY = 25;

	// Size
	config.minWidth = 200;
	config.maxWidth = 400;

	// Font
	sprintf_s(config.fontName, 256, "Arial");
	config.fontSize = -14;

	// Margin
	config.textMargin = 15;
	config.titleBodyMargin = config.notificationMargin = 5;

	// Color
	config.backgroundColor = 0xffcc00;
	config.borderColor = config.textColor = 0xffcc00;

	// Display
	config.displayTime = 0;

	inited = 1;
}

NotificueConfig* config_load()
{
	// todo
	inited = 1;
	return &config;
}

NotificueConfig* config_get()
{
	if (!inited) setDefaults();
	return &config;
}

void config_save()
{
	if (!inited) setDefaults();
	// todo
}
