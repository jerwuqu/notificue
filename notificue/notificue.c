#include "ntfls.h"
#include "ntfshow.h"
#include "shellhook.h"
#include "mainwnd.h"

int main(int argc, char** argv)
{
	// Init
	if (ntfshow_init()) return 1;
	if (mainwnd_create()) return 1;
	if (shellhook_inject()) return 1;

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
	
	return 0;
}
