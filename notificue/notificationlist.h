#pragma once

#include <Windows.h>
#include <stdlib.h>
#include <time.h>

#include "log.h"

#define NOTIFICATION_LIMIT 3

typedef struct _Notification
{
	int index;
	time_t created;
	wchar_t* title;
	wchar_t* body;
} Notification;

Notification* ntls_create(const time_t created, const wchar_t* title, const wchar_t* body);
void ntls_remove(Notification* notification);
