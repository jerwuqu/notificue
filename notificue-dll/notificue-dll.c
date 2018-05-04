#include <windows.h>
#include <stdio.h>

HWND notificueWnd;

extern __declspec(dllexport) LRESULT WINAPI wndProcHook(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode == HC_ACTION && notificueWnd) {
		CWPSTRUCT *data = (CWPSTRUCT*)lParam;
		if (data->message == WM_COPYDATA) {
			COPYDATASTRUCT* cds = (COPYDATASTRUCT*)data->lParam;
			char* buff = (char*)cds->lpData;
			DWORD bufflen = cds->cbData;

			// Verify length and magic [from Shell_NotifyIconW in Shell32.dll]
			if (bufflen == 0x5CC && buff[0] == 0x23 && buff[1] == 0x34 && buff[2] == 0x75 && buff[3] == 0x34) {
				// Forward to notificue
				SendMessage(notificueWnd, data->message, data->wParam, data->lParam);
			}
		}
	}

	return CallNextHookEx(0, nCode, wParam, lParam);
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH) {
		notificueWnd = FindWindowA("notificue_main", "");
		if (!notificueWnd) return FALSE;
	}

	return TRUE;
}
