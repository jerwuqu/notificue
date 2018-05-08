// Notificue Logging

#include "log.h"
#include <time.h>

void log_text(const char* format, ...)
{
	va_list vargs;
	va_start(vargs, format);

	vprintf(format, vargs);

#ifdef LOG_TO_FILE
	FILE* file;
	if (fopen_s(&file, LOG_PATH, "a+")) {
		printf("Error opening log file!\n");
		va_end(vargs);
		return;
	}
	vfprintf(file, format, vargs);
	fclose(file);
#endif

	va_end(vargs);
}

void log_dump(char* data, size_t size)
{
	char fileName[MAX_PATH];
	GetTempFileNameA(LOG_DUMP_PATH, "dmp", 0, fileName);

	FILE* file;
	if (fopen_s(&file, fileName, "w")) {
		log_text("Error opening dump file!\n");
		log_win32_error();
		return;
	}
	fwrite(data, size, 1, file);
	fclose(file);
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
