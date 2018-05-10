#pragma once

#include <Windows.h>

int shellhook_inject();
HWND shellhook_getShellWnd();
void shellhook_remove();
