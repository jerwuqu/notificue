#pragma once

#include <Windows.h>
#include <stdlib.h>
#include "log.h"
#include "ntfls.h"

int ntfshow_init();
void ntfshow_quit();
Notification* ntfshow_create(wchar_t* title, wchar_t* body, wchar_t* process);
void ntfshow_reposition(Notification* ntf);
void ntfshow_remove(Notification* ntf);
