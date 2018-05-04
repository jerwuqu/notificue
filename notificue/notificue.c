#include "notificue.h"

HCURSOR cursor;
HFONT titleFont, bodyFont;
HWND mainWnd;

HMODULE dll;
HHOOK hook;

#define NOTIFICATION_X 25
#define NOTIFICATION_Y 25
#define NOTIFICATION_WIDTH 200
#define NOTIFICATION_HEIGHT 60
#define NOTIFICATION_MARGIN 5

void displayNotification(wchar_t* title, wchar_t* body)
{
	// Create notification in list
	time_t created = time(0);
	Notification* ntf = ntls_create(created, title, body);
	if (ntf == NULL) {
		ntfu_log("Failed to create notification!\n");
		return;
	}

	// Create notification window
	HWND hwnd = CreateWindowEx(WS_EX_TOPMOST, "notificue_ntf", "", CS_VREDRAW | CS_HREDRAW, 0, 0, NOTIFICATION_WIDTH, NOTIFICATION_HEIGHT, 0, 0, 0, 0);
	if (!hwnd) {
		ntfu_log("Failed to create window!\n");
		ntfu_log_win32_error();
		return;
	}

	// Assign notification
	SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)ntf);

	// Remove styling (https://stackoverflow.com/questions/2398746/removing-window-border)
	LONG lStyle = GetWindowLong(hwnd, GWL_STYLE);
	lStyle &= ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU);
	SetWindowLong(hwnd, GWL_STYLE, lStyle);
	LONG lExStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
	lExStyle &= ~(WS_EX_DLGMODALFRAME | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE);
	SetWindowLong(hwnd, GWL_EXSTYLE, lExStyle);

	// Show window
	ShowWindow(hwnd, SW_SHOW);
	if (!UpdateWindow(hwnd)) {
		ntfu_log("Failed to update window!\n");
		ntfu_log_win32_error();
		return;
	}

	// Set position
	int y = NOTIFICATION_Y + (ntf->index) * (NOTIFICATION_HEIGHT + NOTIFICATION_MARGIN);
	SetWindowPos(hwnd, NULL, NOTIFICATION_X, y, 0, 0, SWP_FRAMECHANGED | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER);
}

LRESULT CALLBACK mainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
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
				ntfu_log("Notification! Title: %ls, Body: %ls\n", sni.nid_szInfoTitle, sni.nid_szInfo);
				displayNotification(sni.nid_szInfoTitle, sni.nid_szInfo);
			}
		}
	}

defwndproc:
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK ntfWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	Notification* ntf = (Notification*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if (ntf) {
		if (msg == WM_PAINT) {
			// Rect
			RECT rect;
			rect.left = 15;
			rect.top = 15;
			rect.right = NOTIFICATION_WIDTH - rect.left;
			rect.bottom = NOTIFICATION_HEIGHT - rect.top;

			// Begin
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);

			// Clear
			SelectObject(hdc, GetStockObject(DC_PEN));
			SelectObject(hdc, GetStockObject(DC_BRUSH));
			SetDCPenColor(hdc, 0x000000);
			SetDCBrushColor(hdc, 0xffcc00);
			Rectangle(hdc, 0, 0, NOTIFICATION_WIDTH, NOTIFICATION_HEIGHT);

			// Draw text
			SetBkMode(hdc, TRANSPARENT);
			SelectObject(hdc, titleFont);
			SetTextColor(hdc, 0xffffff);
			DrawTextW(hdc, ntf->title, -1, &rect, DT_SINGLELINE | DT_NOCLIP | DT_NOPREFIX);

			rect.top = 30;
			SelectObject(hdc, bodyFont);
			DrawTextW(hdc, ntf->body, -1, &rect, DT_NOCLIP | DT_NOPREFIX);

			// End
			EndPaint(hwnd, &ps);
			return FALSE;
		} else if (msg == WM_LBUTTONDOWN) {
			ntls_remove(ntf);
			SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
			DestroyWindow(hwnd);
		}
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

int createWindow()
{
	// Load resources
	cursor = LoadCursor(NULL, IDC_ARROW);
	if (!cursor) {
		ntfu_log("Failed to load cursor!\n");
		ntfu_log_win32_error();
		return 1;
	}

	titleFont = CreateFont(-14, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, "Arial");
	if (!titleFont) {
		ntfu_log("Failed to load title font!\n");
		ntfu_log_win32_error();
		return 1;
	}

	bodyFont = CreateFont(-14, 0, 0, 0, FW_NORMAL, 0, 0, 0, 0, 0, 0, 0, 0, "Arial");
	if (!bodyFont) {
		ntfu_log("Failed to load body font!\n");
		ntfu_log_win32_error();
		return 1;
	}

	// Register main class
	WNDCLASSEX wc;
	memset(&wc, 0, sizeof(WNDCLASSEX));
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.lpfnWndProc = mainWndProc;
	wc.lpszClassName = "notificue_main";
	wc.style = CS_VREDRAW | CS_HREDRAW;
	if (!RegisterClassEx(&wc)) {
		ntfu_log("Failed to register main class!\n");
		ntfu_log_win32_error();
		return 1;
	}

	// Register notification class
	memset(&wc, 0, sizeof(WNDCLASSEX));
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.lpfnWndProc = ntfWndProc;
	wc.lpszClassName = "notificue_ntf";
	wc.hCursor = cursor;
	if (!RegisterClassEx(&wc)) {
		ntfu_log("Failed to register ntf class!\n");
		ntfu_log_win32_error();
		return 1;
	}

	// Create window
	mainWnd = CreateWindowEx(0, "notificue_main", "", 0, 0, 0, 100, 100, 0, 0, 0, 0);
	if (!mainWnd) {
		ntfu_log("Failed to create window!\n");
		ntfu_log_win32_error();
		return 1;
	}

	ShowWindow(mainWnd, SW_HIDE);
	if (!UpdateWindow(mainWnd)) {
		ntfu_log("Failed to update window!\n");
		ntfu_log_win32_error();
		return 1;
	}

	return 0;
}

void destroyWindow()
{
	DestroyWindow(mainWnd);
	DeleteObject(cursor);
	DeleteObject(titleFont);
	DeleteObject(bodyFont);
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

int main(int argc, char** argv)
{
	if (createWindow()) return 1;
	if (hookNotifications()) return 1;

	MSG message;
	while (GetMessage(&message, NULL, 0, 0)) {
		TranslateMessage(&message);
		DispatchMessage(&message);
	}
	
	unhookNotifications();

	return 0;
}
