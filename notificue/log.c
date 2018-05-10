// Notificue Logging

#include "log.h"
#include <time.h>

static char logBuffer[1024 * 1024];
static int logBufferHead = 0;

void log_flush()
{
#ifdef LOG_TO_FILE
	if (logBufferHead < 1) return;
	FILE* file;
	if (fopen_s(&file, LOG_PATH, "a+")) {
		printf("Error opening log file!\n");
		return;
	}
	fwrite(logBuffer, logBufferHead, 1, file);
	logBufferHead = 0;
	fflush(file);
	fclose(file);
#endif
}

void log_text(const char* format, ...)
{
	va_list vargs;
	va_start(vargs, format);
	vprintf(format, vargs);
#ifdef LOG_TO_FILE
	if (logBufferHead < sizeof(logBuffer)) {
		int written = vsprintf_s(&logBuffer[logBufferHead], sizeof(logBuffer) - logBufferHead, format, vargs);
		if (written < 0) {
			printf("Failed to write to log buffer!\n");
		} else {
			logBufferHead += written;
		}
	}
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
