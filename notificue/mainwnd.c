#include "mainwnd.h"
#include "ntfshow.h"
#include "shellhook.h"

#define ONE_SECOND 1
#define HOOK_DEAD_TIME (ONE_SECOND * 3)

static HWND mainWnd = NULL;
static time_t lastPong;

static int processPathFromWnd(HWND hwnd, WCHAR* processNameOut, DWORD processNameLength)
{
	DWORD processId;
	GetWindowThreadProcessId(hwnd, &processId);
	HANDLE processHandle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processId);
	if (!processHandle) {
		log_text("Failed to open process!\n");
		log_win32_error();
		return 0;
	}

	if (!QueryFullProcessImageNameW(processHandle, 0, processNameOut, &processNameLength)) {
		log_text("Failed to get process name!\n");
		log_win32_error();
		return 0;
	}
	CloseHandle(processHandle);

	return 1;
}

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

			// Get data with reliable offsets
			sni.nid_hWnd = *((HWND*)(buff + 0xC));
			sni.nid_hIcon = *((HICON*)(buff + 0x1C));
			sni.nid_szTip = (WCHAR*)(buff + 0x20);

			// Ignore message if icon is being deleted
			if (sni.dwMessage == NIM_DELETE) goto defwndproc;

			// Get remaning data depending on NID struct size (which varies depending on OS version)
			if (sni.nid_cbSize == 0x3BC) {
				sni.nid_uFlags = *((DWORD*)(buff + 0x14));
				sni.nid_szInfo = (WCHAR*)(buff + 0x128);
				sni.nid_szInfoTitle = (WCHAR*)(buff + 0x32C);
				sni.ext_exePath = (WCHAR*)(buff + 0x3C4);
			} else {
				log_text("Incorrect NID cbSize! (%d) CDS dumped!\n", sni.nid_cbSize);
				log_dump(buff, bufflen);
				goto defwndproc;
			}

			// Check if this is a notification
			if (sni.nid_uFlags & NIF_INFO) {
				if (sni.nid_hWnd) {
					WCHAR processName[MAX_PATH];
					if (processPathFromWnd(sni.nid_hWnd, processName, sizeof(processName))) {
						ntfshow_create(sni.nid_szInfoTitle, sni.nid_szInfo, processName);
					} else {
						ntfshow_create(sni.nid_szInfoTitle, sni.nid_szInfo, NULL);
					}
				} else if (sni.ext_exePath && sni.ext_exePath[0]) {
					ntfshow_create(sni.nid_szInfoTitle, sni.nid_szInfo, sni.ext_exePath);
				} else {
					ntfshow_create(sni.nid_szInfoTitle, sni.nid_szInfo, NULL);
				}
			}
		}
	} else if (msg == NOTIFICUE_PONG_MESSAGE) {
		lastPong = time(0);
	}

defwndproc:
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

static VOID CALLBACK aliveTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	// Flush log
	log_flush();

	// Get window
	HWND wnd = shellhook_getShellWnd();
	if (!wnd) return;

	// Check when we got last pong
	if (time(0) - lastPong >= HOOK_DEAD_TIME) {
		log_text("Hook dead! Reinjecting...\n");
		shellhook_remove();
		if (shellhook_inject()) return;
		log_text("OK!\n");
	}

	// Send ping
	SendMessage(wnd, NOTIFICUE_PING_MESSAGE, 0, 0);
}

int mainwnd_isRunning()
{
	return FindWindowA(NOTIFICUE_HOOK_WND_CLASSNAME, "") != NULL;
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

	// Create repeating timer
	lastPong = time(0);
	SetTimer(mainWnd, 0, ONE_SECOND * 1000, aliveTimerProc);

	return 0;
}

void mainwnd_destroy()
{
	DestroyWindow(mainWnd);
	mainWnd = NULL;
}
