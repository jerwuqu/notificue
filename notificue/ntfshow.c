#include "ntfshow.h"

#define NOTIFICATION_X 25
#define NOTIFICATION_Y 25
#define NOTIFICATION_WIDTH 200
#define NOTIFICATION_HEIGHT 60
#define NOTIFICATION_MARGIN 5

static HCURSOR cursor = NULL;
static HFONT titleFont = NULL;
static HFONT bodyFont = NULL;

static LRESULT CALLBACK wndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
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
			ntfls_remove(ntf);
			SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
			DestroyWindow(hwnd);
		}
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

int ntfshow_init()
{
	// Load resources
	cursor = LoadCursor(NULL, IDC_HAND);
	if (!cursor) {
		log_text("Failed to load cursor!\n");
		log_win32_error();
		return 1;
	}

	titleFont = CreateFont(-14, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, "Arial");
	if (!titleFont) {
		log_text("Failed to load title font!\n");
		log_win32_error();
		return 1;
	}

	bodyFont = CreateFont(-14, 0, 0, 0, FW_NORMAL, 0, 0, 0, 0, 0, 0, 0, 0, "Arial");
	if (!bodyFont) {
		log_text("Failed to load body font!\n");
		log_win32_error();
		return 1;
	}

	// Register notification class
	WNDCLASSEX wc;
	memset(&wc, 0, sizeof(WNDCLASSEX));
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.lpfnWndProc = wndProc;
	wc.lpszClassName = "notificue_ntf";
	wc.hCursor = cursor;
	if (!RegisterClassEx(&wc)) {
		log_text("Failed to register ntf class!\n");
		log_win32_error();
		return 1;
	}

	return 0;
}

void ntfshow_quit()
{
	DeleteObject(cursor);
	DeleteObject(titleFont);
	DeleteObject(bodyFont);
	cursor = titleFont = bodyFont = NULL;
}

void ntfshow_display(wchar_t* title, wchar_t* body)
{
	// Create notification in list
	time_t created = time(0);
	Notification* ntf = ntfls_create(created, title, body);
	if (ntf == NULL) {
		log_text("Failed to create notification!\n");
		return;
	}

	// Create notification window
	HWND hwnd = CreateWindowEx(WS_EX_TOPMOST, "notificue_ntf", "", CS_VREDRAW | CS_HREDRAW, 0, 0, NOTIFICATION_WIDTH, NOTIFICATION_HEIGHT, 0, 0, 0, 0);
	if (!hwnd) {
		log_text("Failed to create window!\n");
		log_win32_error();
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
		log_text("Failed to update window!\n");
		log_win32_error();
		return;
	}

	// Set position
	int y = NOTIFICATION_Y + (ntf->index) * (NOTIFICATION_HEIGHT + NOTIFICATION_MARGIN);
	SetWindowPos(hwnd, NULL, NOTIFICATION_X, y, 0, 0, SWP_FRAMECHANGED | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER);
}
