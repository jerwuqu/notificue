#include "notificue.h"
#include "ntfls.h"
#include "ntfshow.h"
#include "shellhook.h"
#include "mainwnd.h"

void notificue_exit(int errorCode)
{
	shellhook_remove();
	mainwnd_destroy();
	ntfshow_quit();
	log_flush();
	exit(errorCode);
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,  LPSTR lpCmdLine, int nCmdShow)
{
	// Check if already running
	if (mainwnd_isRunning()) {
		MessageBox(0, "notificue is already running!\n", "notificue", MB_OK | MB_ICONERROR);
		return 1;
	}

	// Init
	if (ntfshow_init()) {
		log_flush();
		return 1;
	}

	if (mainwnd_create(hInstance)) {
		log_flush();
		return 1;
	}

	if (shellhook_inject()) {
		log_flush();
		return 1;
	}

	log_text("OK!\n");

	// Message loop
	MSG message;
	while (GetMessage(&message, NULL, 0, 0)) {
		TranslateMessage(&message);
		DispatchMessage(&message);
	}

	// Quit
	notificue_exit(0);

	return 0;
}
