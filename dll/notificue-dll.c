#include <Windows.h>
#include "../shared/shared.h"

static HWND notificue_wnd = 0;

extern __declspec(dllexport) LRESULT WINAPI wndProcHook(int code, WPARAM wparam, LPARAM lparam)
{
    if (code == HC_ACTION && notificue_wnd) {
        CWPSTRUCT *data = (CWPSTRUCT*)lparam;
        if (data->message == WM_COPYDATA) {
            // Get CDS
            COPYDATASTRUCT* cds = (COPYDATASTRUCT*)data->lParam;
            char* buff = (char*)cds->lpData;
            DWORD bufflen = cds->cbData;

            // Verify length and magic
            if (CDS_NID_CHECK(buff, bufflen)) {
                // Forward to notificue
                SendMessage(notificue_wnd, data->message, data->wParam, data->lParam);
            }
        } else if (data->message == WM_NOTIFICUE_PING) {
            // Reply with pong
            PostMessage(notificue_wnd, WM_NOTIFICUE_PONG, 0, 0);
        }
    }

    return CallNextHookEx(0, code, wparam, lparam);
}

BOOL WINAPI DllMain(HINSTANCE _inst, DWORD reason, LPVOID _reserved)
{
    if (reason == DLL_PROCESS_ATTACH) {
        notificue_wnd = FindWindowA(HOOK_WND_CLASSNAME, "");
        if (!notificue_wnd) return FALSE;
    }

    return TRUE;
}
