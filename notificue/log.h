// Notificue Logging

#pragma once

#include <Windows.h>
#include <stdio.h>

#ifdef LOG_TO_FILE
#define LOG_PATH "notificue.log"
#endif

void log_text(const char* format, ...);
void log_win32_error();
void log_shell32_version();
