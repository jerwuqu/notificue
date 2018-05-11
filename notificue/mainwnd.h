#pragma once

#include <Windows.h>
#include "shared.h"

struct SNIDATA
{
	// From Shell_NotifyIcon
	DWORD dwMessage;

	// From NOTIFYICONDATA
	DWORD nid_cbSize;
	HWND nid_hWnd;
	HICON nid_hIcon;
	WCHAR* nid_szTip;

	// Also NOTIFYICONDATA though offset varies depending on shell32 version
	DWORD nid_uFlags;
	WCHAR* nid_szInfo;
	WCHAR* nid_szInfoTitle;

	// Other data that is sometimes present in the message
	WCHAR* ext_exePath;
};

int mainwnd_isRunning();
int mainwnd_create();
void mainwnd_destroy();
