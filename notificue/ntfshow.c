#include "ntfshow.h"
#include "config.h"
#pragma comment(lib, "Winmm.lib")

#define NOTIFICATION_CLASS "notificue_ntf"
#define NTF_DRAW_TEXT_FLAGS (DT_NOCLIP | DT_NOPREFIX | DT_WORDBREAK | DT_EDITCONTROL)

static HCURSOR cursor = NULL;
static HFONT titleFont = NULL;
static HFONT bodyFont = NULL;
static NotificueConfig* config = NULL;
static HDC dummyDC = NULL;
static DWORD screenWidth, screenHeight;

static SIZE calculateBoxSize(const wchar_t* title, const wchar_t* body)
{
	// Calculate textbox width
	SIZE textSize;
	GetTextExtentPoint32W(dummyDC, title, lstrlenW(title), &textSize);
	int titleWidth = textSize.cx;
	GetTextExtentPoint32W(dummyDC, body, lstrlenW(body), &textSize);
	int textWidth = titleWidth > textSize.cx ? titleWidth : textSize.cx;

	if (textWidth < config->minWidth) textWidth = config->minWidth;
	if (textWidth > config->maxWidth - config->textBoxMargin * 2) textWidth = config->maxWidth - config->textBoxMargin * 2;

	// Get title height
	RECT rect;
	rect.left = rect.top = 0;
	rect.right = textWidth;
	rect.bottom = 1000;
	SelectObject(dummyDC, titleFont);
	int boxHeight = config->textBoxMargin * 2 + config->titleBodyMargin;
	boxHeight += DrawTextW(dummyDC, title, -1, &rect, NTF_DRAW_TEXT_FLAGS);

	// Get body height
	rect.left = rect.top = 0;
	rect.right = textWidth;
	rect.bottom = 1000;
	SelectObject(dummyDC, bodyFont);
	boxHeight += DrawTextW(dummyDC, body, -1, &rect, NTF_DRAW_TEXT_FLAGS);

	// Set properties
	SIZE boxSize;
	boxSize.cx = textWidth + config->textBoxMargin * 2;
	boxSize.cy = boxHeight;

	return boxSize;
}

static LRESULT CALLBACK wndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	Notification* ntf = (Notification*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if (ntf) {
		if (msg == WM_PAINT) {
			// Begin
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);

			// Rect
			RECT textRect;
			textRect.left = textRect.top = config->textBoxMargin;
			textRect.right = ntf->boxSize.cx - config->textBoxMargin;
			textRect.bottom = ntf->boxSize.cy - config->textBoxMargin;

			// Clear
			SelectObject(hdc, GetStockObject(DC_PEN));
			SelectObject(hdc, GetStockObject(DC_BRUSH));
			SetDCPenColor(hdc, config->borderColor);
			SetDCBrushColor(hdc, config->backgroundColor);
			Rectangle(hdc, 0, 0, ntf->boxSize.cx, ntf->boxSize.cy);

			// Draw text
			SetBkMode(hdc, TRANSPARENT);
			SelectObject(hdc, titleFont);
			SetTextColor(hdc, config->textColor);
			int titleHeight = DrawTextW(hdc, ntf->title, -1, &textRect, NTF_DRAW_TEXT_FLAGS);

			textRect.top += titleHeight + config->titleBodyMargin;
			SelectObject(hdc, bodyFont);
			DrawTextW(hdc, ntf->body, -1, &textRect, NTF_DRAW_TEXT_FLAGS);

			// End
			EndPaint(hwnd, &ps);
			return FALSE;
		} else if (msg == WM_LBUTTONDOWN) {
			ntfshow_remove(ntf);
		}
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

static VOID CALLBACK dismissTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	Notification* ntf = (Notification*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	KillTimer(hwnd, idEvent);
	if (ntf) ntfshow_remove(ntf);
}

int ntfshow_init()
{
	// Get config
	config = config_get();

	// Create dummy DC
	HDC displayDC = CreateDC("DISPLAY", NULL, NULL, NULL);
	dummyDC = CreateCompatibleDC(displayDC);
	DeleteDC(displayDC);

	// Get screen size
	screenWidth = GetSystemMetrics(SM_CXSCREEN);
	screenHeight = GetSystemMetrics(SM_CYSCREEN);

	// Load resources
	cursor = LoadCursor(NULL, IDC_HAND);
	if (!cursor) {
		log_text("Failed to load cursor!\n");
		log_win32_error();
		return 1;
	}

	titleFont = CreateFont(config->fontSize, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, config->fontName);
	if (!titleFont) {
		log_text("Failed to load title font!\n");
		log_win32_error();
		return 1;
	}

	bodyFont = CreateFont(config->fontSize, 0, 0, 0, FW_NORMAL, 0, 0, 0, 0, 0, 0, 0, 0, config->fontName);
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
	wc.lpszClassName = NOTIFICATION_CLASS;
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
	cursor = NULL;
	DeleteObject(titleFont);
	DeleteObject(bodyFont);
	titleFont = bodyFont = NULL;

	DeleteDC(dummyDC);
	dummyDC = NULL;
}

Notification* ntfshow_create(wchar_t* title, wchar_t* body)
{
	// Don't show if both title and body are empty
	if (lstrlenW(title) == 0 && lstrlenW(body) == 0) {
		log_text("Empty notification, not displaying...\n");
		return 0;
	}

	// Calculate size
	SIZE boxSize = calculateBoxSize(title, body);

	// Create notification in list
	time_t created = time(0);
	Notification* ntf = ntfls_create(created, title, body, boxSize);
	if (ntf == NULL) {
		log_text("Failed to create notification!\n");
		return 0;
	}

	// Create notification window
	HWND hwnd = CreateWindowEx(WS_EX_TOPMOST | WS_EX_TOOLWINDOW, NOTIFICATION_CLASS, "", 0, 0, 0, ntf->boxSize.cx, ntf->boxSize.cy, 0, 0, 0, 0);
	if (!hwnd) {
		log_text("Failed to create window!\n");
		log_win32_error();
		return 0;
	}

	// Link notification and window
	ntf->hwnd = hwnd;
	SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)ntf);

	// Remove default window styling
	LONG lStyle = GetWindowLong(hwnd, GWL_STYLE);
	lStyle &= ~WS_OVERLAPPEDWINDOW;
	SetWindowLong(hwnd, GWL_STYLE, lStyle);

	// Show window
	ShowWindow(hwnd, SW_SHOW);
	if (!UpdateWindow(hwnd)) {
		log_text("Failed to update window!\n");
		log_win32_error();
		return 0;
	}

	// Set position
	ntfshow_reposition(ntf);

	// Set auto-dismiss timer
	if (config->displayTime > 0) SetTimer(hwnd, 0, config->displayTime, dismissTimerProc);

	// Play sound
	if (config->soundFile[0]) PlaySound(config->soundFile, NULL, SND_FILENAME | SND_ASYNC);

	return ntf;
}

void ntfshow_reposition(Notification* ntf)
{
	if (!ntf->hwnd) return;
	int boxX, boxY;
	if (config->screenX >= 0) {
		boxX = config->screenX;
	} else {
		boxX = screenWidth + config->screenX - ntf->boxSize.cx;
	}
	if (config->screenY >= 0) {
		boxY = config->screenY + ntf->boxYOffset + ntf->index * config->notificationMargin;
	} else {
		boxY = screenHeight + config->screenY - ntf->boxYOffset - ntf->index * config->notificationMargin - ntf->boxSize.cy;
	}
	SetWindowPos(ntf->hwnd, NULL, boxX, boxY, 0, 0, SWP_FRAMECHANGED | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER);
}

void ntfshow_remove(Notification* ntf)
{
	ntfls_remove(ntf);
	if (!ntf->hwnd) return;
	SetWindowLongPtr(ntf->hwnd, GWLP_USERDATA, 0);
	DestroyWindow(ntf->hwnd);
}
