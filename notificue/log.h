#pragma once

#include <Windows.h>
#include <stdio.h>

#define LOG_PATH "C:\\Users\\JerwuQu\\Desktop\\notificue\\notificue.log"

void ntfu_log(const char* format, ...);
void ntfu_log_win32_error();
void ntfu_log_shell32_version();
