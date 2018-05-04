#pragma once

#include <Windows.h>
#include <stdlib.h>
#include "log.h"
#include "ntfls.h"

int ntfshow_init();
void ntfshow_quit();
void ntfshow_display(wchar_t* title, wchar_t* body);
