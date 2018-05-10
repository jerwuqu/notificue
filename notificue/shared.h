// This header is shared between DLL and main application

#pragma once

// Found in decompilation of Shell_NotifyIconW in shell32.dll
#define CDS_NID_DATA_SIZE 0x5CC
#define CDS_NID_MAGIC_BYTE_1 0x23
#define CDS_NID_MAGIC_BYTE_2 0x34
#define CDS_NID_MAGIC_BYTE_3 0x75
#define CDS_NID_MAGIC_BYTE_4 0x34
#define CDS_NID_CHECK(data, len) (len == CDS_NID_DATA_SIZE && \
		data[0] == CDS_NID_MAGIC_BYTE_1 && \
		data[1] == CDS_NID_MAGIC_BYTE_2 && \
		data[2] == CDS_NID_MAGIC_BYTE_3 && \
		data[3] == CDS_NID_MAGIC_BYTE_4)

#define NOTIFICUE_HOOK_WND_CLASSNAME "notificue_mainwnd"

#define NOTIFICUE_PING_INTERVAL 1
#define NOTIFICUE_HOOK_DEAD_TIMEOUT (NOTIFICUE_PING_INTERVAL * 3)
#define NOTIFICUE_PING_MESSAGE (WM_USER + 0)
#define NOTIFICUE_PONG_MESSAGE (WM_USER + 1)
