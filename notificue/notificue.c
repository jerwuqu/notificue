#include "ntfls.h"
#include "ntfshow.h"
#include "shellhook.h"
#include "mainwnd.h"

int main(int argc, char** argv)
{
	// Check if already running
	if (mainwnd_isRunning()) {
		MessageBox(0, "notificue is already running!\n", "notificue", MB_OK | MB_ICONERROR);
		return 1;
	}

	// Init
	if (ntfshow_init()) return 1;
	if (mainwnd_create()) return 1;
	if (shellhook_inject()) return 1;

	log_text("OK!\n");

	// Message loop
	MSG message;
	while (GetMessage(&message, NULL, 0, 0)) {
		TranslateMessage(&message);
		DispatchMessage(&message);
	}

	// Quit
	shellhook_remove();
	mainwnd_destroy();
	ntfshow_quit();
	log_flush();

	return 0;
}
