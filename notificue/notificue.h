#pragma once

#include <Windows.h>
#include <stdio.h>

#include "notificationlist.h"

struct SNIDATA
{
	// From Shell_NotifyIcon
	DWORD dwMessage;

	// From NOTIFYICONDATA
	DWORD nid_cbSize;
	DWORD nid_uFlags;
	WCHAR* nid_szInfo;
	WCHAR* nid_szInfoTitle;
};