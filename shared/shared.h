// Class name used for referring to main program
#define HOOK_WND_CLASSNAME "notificue_wcs"

// From Windows.h
#define WM_USER 0x0400

// Custom messages for talking between main program and dll
#define WM_NOTIFICUE_PING (WM_USER + 1)
#define WM_NOTIFICUE_PONG (WM_USER + 2)

// Found in decompilation of Shell_NotifyIconW in shell32.dll
#define CDS_NID_DATA_SIZE 0x5CC
#define CDS_NID_MAGIC "\x23\x34\x75\x34"

#define CDS_NID_CHECK(buff, len) (\
    len == CDS_NID_DATA_SIZE && \
    buff[0] == CDS_NID_MAGIC[0] && \
    buff[1] == CDS_NID_MAGIC[1] && \
    buff[2] == CDS_NID_MAGIC[2] && \
    buff[3] == CDS_NID_MAGIC[3])
