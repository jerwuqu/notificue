#include "log.h"

void ntfu_log(const char* format, ...)
{
	va_list vargs;
	va_start(vargs, format);

	vprintf(format, vargs);

	FILE* file;
	fopen_s(&file, LOG_PATH, "a+");
	vfprintf(file, format, vargs);
	fclose(file);

	va_end(vargs);
}

void ntfu_log_win32_error()
{
	wchar_t buf[256];
	FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buf, 256, NULL);
	ntfu_log("%ls\n", buf);
}

void ntfu_log_shell32_version()
{
	// todo: this
}