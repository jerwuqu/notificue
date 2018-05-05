#include "mainwnd.h"
#include "ntfshow.h"

static HWND mainWnd = NULL;

static LRESULT CALLBACK wndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_COPYDATA) {
		// Get data pointed to in message
		COPYDATASTRUCT* cds = (COPYDATASTRUCT*)lParam;
		char* buff = (char*)cds->lpData;
		DWORD bufflen = cds->cbData;

		// Verify length and magic
		if (CDS_NID_CHECK(buff, bufflen)) {
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
				log_text("Incorrect NID cbSize! (%d)\n", sni.nid_cbSize);
				log_shell32_version();
				goto defwndproc;
			}

			// Check if this is a notification
			if (sni.nid_uFlags & NIF_INFO) {
				log_text("Notification! Title: %ls, Body: %ls\n", sni.nid_szInfoTitle, sni.nid_szInfo);
				ntfshow_create(sni.nid_szInfoTitle, sni.nid_szInfo);
			}
		}
	}

defwndproc:
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

int mainwnd_create()
{
	// Register main class
	WNDCLASSEX wc;
	memset(&wc, 0, sizeof(WNDCLASSEX));
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.lpfnWndProc = wndProc;
	wc.lpszClassName = NOTIFICUE_HOOK_WND_CLASSNAME;
	wc.style = CS_VREDRAW | CS_HREDRAW;
	if (!RegisterClassEx(&wc)) {
		log_text("Failed to register main class!\n");
		log_win32_error();
		return 1;
	}

	// Create window
	mainWnd = CreateWindowEx(0, NOTIFICUE_HOOK_WND_CLASSNAME, "", 0, 0, 0, 100, 100, 0, 0, 0, 0);
	if (!mainWnd) {
		log_text("Failed to create window!\n");
		log_win32_error();
		return 1;
	}

	ShowWindow(mainWnd, SW_HIDE);
	if (!UpdateWindow(mainWnd)) {
		log_text("Failed to update window!\n");
		log_win32_error();
		return 1;
	}

	return 0;
}

void mainwnd_destroy()
{
	DestroyWindow(mainWnd);
	mainWnd = NULL;
}
