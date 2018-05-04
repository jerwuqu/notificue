#include <Windows.h>
#include <stdio.h>

#define LOG_PATH "C:\\Users\\JerwuQu\\Desktop\\notificue\\notificue.log"

HCURSOR cursor;
HFONT font;
HWND mainWnd;

HMODULE dll;
HHOOK hook;

void ntfu_log(const char* format, ...)
{
	va_list vargs;
	va_start(vargs, format);

	vprintf(format, vargs);

	FILE* file;
	fopen_s(&file, LOG_PATH, "a+");
	vfprintf(file, format, vargs);
	fclose(file);

	va_end(vargs);
}

void ntfu_log_win32_error()
{
	wchar_t buf[256];
	FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buf, 256, NULL);
	ntfu_log("%ls\n", buf);
}

void ntfu_log_shell32_version()
{
	// todo: this
}

int hookNotifications()
{
	dll = LoadLibraryA("notificue-dll.dll");
	if (!dll) {
		ntfu_log("DLL failed to load!\n");
		ntfu_log_win32_error();
		return 1;
	}

	HOOKPROC addr = (HOOKPROC)GetProcAddress(dll, "wndProcHook");
	if (!addr) {
		ntfu_log("DLL function not found!\n");
		ntfu_log_win32_error();
		return 1;
	}


	HWND wnd = FindWindowA("Shell_TrayWnd", "");
	if (!wnd) {
		ntfu_log("Shell_TrayWnd window not found!\n");
		ntfu_log_win32_error();
		return 1;
	}

	DWORD pid;
	DWORD tid = GetWindowThreadProcessId(wnd, &pid);
	if (!tid) {
		ntfu_log("Window Thread ID not found!\n");
		ntfu_log_win32_error();
		return 1;
	}

	ntfu_log("TID: %d\n", tid);

	hook = SetWindowsHookEx(WH_CALLWNDPROC, addr, dll, tid);
	if (!hook) {
		ntfu_log("WH_CALLWNDPROC hook failed!\n");
		ntfu_log_win32_error();
		return 1;
	}

	return 0;
}

void unhookNotifications()
{
	FreeLibrary(dll);
	UnhookWindowsHookEx(hook);
}

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

LRESULT CALLBACK wndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_COPYDATA) {
		// Get data pointed to in message
		COPYDATASTRUCT* cds = (COPYDATASTRUCT*)lParam;
		char* buff = (char*)cds->lpData;
		DWORD bufflen = cds->cbData;

		// Verify length and magic [found in decompilation of Shell_NotifyIconW in shell32.dll]
		if (bufflen == 0x5CC && buff[0] == 0x23 && buff[1] == 0x34 && buff[2] == 0x75 && buff[3] == 0x34) {
			// Get basic information
			struct SNIDATA sni;
			sni.dwMessage = *((DWORD*)(buff + 0x4));
			sni.nid_cbSize = *((DWORD*)(buff + 0x8));

			// Ignore message if icon is being deleted
			if (sni.dwMessage == NIM_DELETE) goto defwndproc; 

			// Get remaning data depending on NID struct size (which varies depending on OS version)
			if (sni.nid_cbSize == 0x3BC) {
				sni.nid_uFlags = *((DWORD*)(buff + 0x14));
				sni.nid_szInfo = (WCHAR*)(buff + 0x128);
				sni.nid_szInfoTitle = (WCHAR*)(buff + 0x32C);
			} else {
				ntfu_log("Incorrect NID cbSize! (%d)\n", sni.nid_cbSize);
				ntfu_log_shell32_version();
				goto defwndproc;
			}

			// Check if this is a notification
			if (sni.nid_uFlags & NIF_INFO) {
				ntfu_log("Notification! dwMessage: %d, Title: %ls, Body: %ls\n", sni.dwMessage, sni.nid_szInfoTitle, sni.nid_szInfo);
			}
		}
	}

defwndproc:
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

HWND createWindow()
{
	cursor = LoadCursor(NULL, IDC_ARROW);
	if (!cursor) {
		ntfu_log("Failed to load cursor!\n");
		ntfu_log_win32_error();
		return 0;
	}

	font = CreateFont(-14, 0, 0, 0, FW_NORMAL, 0, 0, 0, 0, 0, 0, 0, 0, "Arial");
	if (!font) {
		ntfu_log("Failed to load font!\n");
		ntfu_log_win32_error();
		return;
	}

	WNDCLASSEX wc;
	memset(&wc, 0, sizeof(WNDCLASSEX));
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.lpfnWndProc = wndProc;
	wc.lpszClassName = "notificue_main";
	wc.style = CS_VREDRAW | CS_HREDRAW;
	wc.hCursor = cursor;
	if (!RegisterClassEx(&wc)) {
		ntfu_log("Failed to register class!\n");
		ntfu_log_win32_error();
		return 0;
	}

	HWND wnd = CreateWindowEx(0, "notificue_main", "", 0, 0, 0, 100, 100, 0, 0, 0, 0);
	if (!wnd) {
		ntfu_log("Failed to create window!\n");
		ntfu_log_win32_error();
		return 0;
	}

	ShowWindow(wnd, SW_HIDE);
	if (!UpdateWindow(wnd)) {
		ntfu_log("Failed to update window!\n");
		ntfu_log_win32_error();
		return 0;
	}

	return wnd;
}

void destroyWindow()
{
	DestroyWindow(mainWnd);
	DeleteObject(cursor);
	DeleteObject(font);
}

int main(int argc, char** argv)
{
	mainWnd = createWindow();
	if (!mainWnd) return 1;
	if (hookNotifications()) return 1;

	MSG message;
	while (GetMessage(&message, NULL, 0, 0)) {
		TranslateMessage(&message);
		DispatchMessage(&message);
	}
	
	unhookNotifications();

	return 0;
}
