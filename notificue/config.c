#include "config.h"
#include "../inih/ini.h"
#include "log.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static char loaded = 0;
static NotificueConfig config;

static int saveDefaults()
{
	log_text("Saving defaults...\n");
	FILE* file;
	fopen_s(&file, CONFIG_PATH, "w");
	if (!file) {
		log_text("Failed to open file!\n");
		return 0;
	}
	if (!fwrite(CONFIG_DEFAULT, CONFIG_DEFAULT_LEN, 1, file)) {
		log_text("Failed to write to file!\n");
		return 0;
	}
	fclose(file);
	return 1;
}

static int backupConfig()
{
	char backupName[256];
	sprintf_s(backupName, 256, "%s.bak.%lld", CONFIG_PATH, time(0));
	if (rename(CONFIG_PATH, backupName)) {
		log_text("Failed to rename file!\n");
		return 0;
	}
	log_text("Moved '%s' to '%s'\n", CONFIG_PATH, backupName);
	return 1;
}

static int iniHandler(void* user, const char* section, const char* name, const char* value)
{
#define SECTION(s) strcmp(section, s) == 0
#define PROPERTY(n) strcmp(name, n) == 0

	if (SECTION("notificue")) {
		if (PROPERTY("version")) {
			sprintf_s(config.version, sizeof(config.version), value);
		} else {
			return 0;
		}
	} else if (SECTION("location")) {
		if (PROPERTY("screen_x")) {
			config.screenX = atoi(value);
		} else if (PROPERTY("screen_y")) {
			config.screenY = atoi(value);
		} else if (PROPERTY("notification_margin")) {
			config.notificationMargin = atoi(value);
		} else {
			return 0;
		}
	} else if (SECTION("size")) {
		if (PROPERTY("min_width")) {
			config.minWidth = atoi(value);
		} else if (PROPERTY("max_width")) {
			config.maxWidth = atoi(value);
		} else {
			return 0;
		}
	} else if (SECTION("text")) {
		if (PROPERTY("font_name")) {
			sprintf_s(config.fontName, sizeof(config.fontName), value);
		} else if (PROPERTY("font_size")) {
			config.fontSize = atoi(value);
		} else if (PROPERTY("box_margin")) {
			config.textBoxMargin = atoi(value);
		} else if (PROPERTY("title_body_margin")) {
			config.titleBodyMargin = atoi(value);
		} else {
			return 0;
		}
	} else if (SECTION("color")) {
		if (PROPERTY("background")) {
			config.backgroundColor = strtol(value, NULL, 16);
		} else if (PROPERTY("border")) {
			config.borderColor = strtol(value, NULL, 16);
		} else if (PROPERTY("text")) {
			config.textColor = strtol(value, NULL, 16);
		} else {
			return 0;
		}
	} else if (SECTION("time")) {
		if (PROPERTY("display_time")) {
			config.displayTime = atoi(value);
		} else {
			return 0;
		}
	} else if (SECTION("sound")) {
		if (PROPERTY("filename")) {
			sprintf_s(config.soundFile, sizeof(config.soundFile), value);
		} else {
			return 0;
		}
	} else {
		return 0;
	}

	return 1;
}

NotificueConfig* config_load()
{
	// Load
	if (ini_parse(CONFIG_PATH, iniHandler, 0) < 0) {
		log_text("Can't load config!\n");
		if (saveDefaults()) {
			return config_load();
		} else {
			return 0;
		}
	}

	// Check version
	if (strncmp(config.version, CONFIG_VERSION, sizeof(CONFIG_VERSION))) {
		log_text("Outdated config version!\n");
		if (backupConfig()) {
			saveDefaults();
			return config_load();
		} else {
			return 0;
		}
	}

	log_text("Config loaded!\n");

	loaded = 1;
	return &config;
}

NotificueConfig* config_get()
{
	if (!loaded) config_load();
	return &config;
}
