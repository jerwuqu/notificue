#include <windows.h>
#include <stdio.h>
#include "../notificue/shared.h"

HWND notificueWnd;

extern __declspec(dllexport) LRESULT WINAPI wndProcHook(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode == HC_ACTION && notificueWnd) {
		CWPSTRUCT *data = (CWPSTRUCT*)lParam;
		if (data->message == WM_COPYDATA) {
			COPYDATASTRUCT* cds = (COPYDATASTRUCT*)data->lParam;
			char* buff = (char*)cds->lpData;
			DWORD bufflen = cds->cbData;

			// Verify length and magic
			if (CDS_NID_CHECK(buff, bufflen)) {
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
		notificueWnd = FindWindowA(NOTIFICUE_HOOK_WND_CLASSNAME, "");
		if (!notificueWnd) return FALSE;
	}

	return TRUE;
}
