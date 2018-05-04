#include "shellhook.h"

static HMODULE dll = NULL;
static HHOOK hook = NULL;

int shellhook_inject()
{
	dll = LoadLibraryA("notificue-dll.dll");
	if (!dll) {
		log_text("DLL failed to load!\n");
		log_win32_error();
		return 1;
	}

	HOOKPROC addr = (HOOKPROC)GetProcAddress(dll, "wndProcHook");
	if (!addr) {
		log_text("DLL function not found!\n");
		log_win32_error();
		return 1;
	}

	HWND wnd = FindWindowA("Shell_TrayWnd", "");
	if (!wnd) {
		log_text("Shell_TrayWnd window not found!\n");
		log_win32_error();
		return 1;
	}

	DWORD tid = GetWindowThreadProcessId(wnd, 0);
	if (!tid) {
		log_text("Window Thread ID not found!\n");
		log_win32_error();
		return 1;
	}

	hook = SetWindowsHookEx(WH_CALLWNDPROC, addr, dll, tid);
	if (!hook) {
		log_text("WH_CALLWNDPROC hook failed!\n");
		log_win32_error();
		return 1;
	}

	return 0;
}

void shellhook_remove()
{
	FreeLibrary(dll);
	UnhookWindowsHookEx(hook);
	dll = hook = NULL;
}
