// Notificue Logging

#include "log.h"

void log_text(const char* format, ...)
{
	va_list vargs;
	va_start(vargs, format);

	vprintf(format, vargs);

#ifdef LOG_TO_FILE
	FILE* file;
	fopen_s(&file, LOG_PATH, "a+");
	vfprintf(file, format, vargs);
	fclose(file);
#endif

	va_end(vargs);
}

void log_win32_error()
{
	wchar_t buf[256];
	FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buf, 256, NULL);
	log_text("%ls\n", buf);
}

void log_shell32_version()
{
	// todo: this
}
