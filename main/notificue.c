#include <Windows.h>
#include "log.h"
#include "../shared/shared.h"

static FILE* log_file = 0;
static HWND main_wnd = 0;
static HMODULE dll = NULL;
static HHOOK hook = NULL;

struct SNIDATA
{
    // From Shell_NotifyIcon
    DWORD dwMessage;

    // From NOTIFYICONDATAW
    DWORD nid_cbSize;
    HWND nid_hWnd;
    HICON nid_hIcon;
    WCHAR* nid_szTip;

    // Also NOTIFYICONDATAW though offset varies depending on shell32 version
    DWORD nid_uFlags;
    WCHAR* nid_szInfo;
    WCHAR* nid_szInfoTitle;

    // Other data that is sometimes present in the message
    WCHAR* ext_exePath;
};

int start_logging(void)
{
    if (fopen_s(&log_file, "notificue.log", "a")) {
        log_error("Failed to open 'notificue.log'");
        return 1;
    }
    log_set_fp(log_file);
    return 0;
}

void stop_logging(void)
{
    if (log_file) {
        log_set_fp(0);
        fclose(log_file);
        log_file = 0;
    }
}

int is_running(void)
{
    return !!FindWindowA(HOOK_WND_CLASSNAME, "");
}

void log_win32_fatal(char* text)
{
    if (text) log_fatal(text);
    char buf[512];
    FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, 0, GetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buf, 512, 0);
    log_fatal(buf);
}

void create_notification(wchar_t* title, wchar_t* body)
{
    // todo
    log_info("%ls - %ls", title, body);
}

static LRESULT CALLBACK wndProc(HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    if (msg == WM_COPYDATA) {
        // Get CDS
        COPYDATASTRUCT* cds = (COPYDATASTRUCT*)lparam;
        char* buff = (char*)cds->lpData;
        DWORD bufflen = cds->cbData;

        // Verify length and magic
        if (!CDS_NID_CHECK(buff, bufflen)) goto defwndproc;

        // Get basic information
        struct SNIDATA sni;
        sni.dwMessage = *((DWORD*)(buff + 0x4));
        sni.nid_cbSize = *((DWORD*)(buff + 0x8));

        // Ignore message if icon is being deleted
        if (sni.dwMessage == NIM_DELETE) goto defwndproc;

        // Verify data size (which varies depending on OS version)
        if (sni.nid_cbSize != 0x3BC) {
            log_error("Incorrect NOTIFYICONDATA cbSize: (%d)! Make sure your OS version is compatible.", sni.nid_cbSize);
            goto defwndproc;
        }

        // Get data with reliable offsets
        sni.nid_hWnd = *((HWND*)(buff + 0xC));
        sni.nid_hIcon = *((HICON*)(buff + 0x1C));
        sni.nid_szTip = (WCHAR*)(buff + 0x20);

        // Get remaning data
        sni.nid_uFlags = *((DWORD*)(buff + 0x14));
        sni.nid_szInfo = (WCHAR*)(buff + 0x128);
        sni.nid_szInfoTitle = (WCHAR*)(buff + 0x32C);
        sni.ext_exePath = (WCHAR*)(buff + 0x3C4);

        // Make sure it's a notification
        if (!(sni.nid_uFlags & NIF_INFO)) goto defwndproc;

        // todo: use nid_hWnd and QueryFullProcessImageNameW here to get exe name like in the old version

        create_notification(sni.nid_szInfoTitle, sni.nid_szInfo);
    }

defwndproc:
    return DefWindowProc(wnd, msg, wparam, lparam);
}

int create_main_window(void)
{
    // Register main class
    WNDCLASSEX wc;
    memset(&wc, 0, sizeof(WNDCLASSEX));
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = wndProc;
    wc.lpszClassName = HOOK_WND_CLASSNAME;
    wc.style = CS_VREDRAW | CS_HREDRAW;
    if (!RegisterClassEx(&wc)) {
        log_win32_fatal("Failed to register main class!");
        return 1;
    }

    // Create window
    main_wnd = CreateWindowEx(0, HOOK_WND_CLASSNAME, "", 0, 0, 0, 100, 100, 0, 0, 0, 0);
    if (!main_wnd) {
        log_win32_fatal("Failed to create window!\n");
        return 1;
    }

    ShowWindow(main_wnd, SW_HIDE);
    if (!UpdateWindow(main_wnd)) {
        log_win32_fatal("Failed to update window!\n");
        return 1;
    }

    return 0;
}

void destroy_main_window(void)
{
    if (main_wnd) {
        DestroyWindow(main_wnd);
        main_wnd = 0;
        UnregisterClassA(HOOK_WND_CLASSNAME, 0);
    }
}

int dll_inject()
{
    dll = LoadLibraryA("notificue.dll");
    if (!dll) {
        log_win32_fatal("notificue.dll failed to load!");
        return 1;
    }

    HOOKPROC addr = (HOOKPROC)GetProcAddress(dll, "wndProcHook");
    if (!addr) {
        log_win32_fatal("DLL hook function not found!");
        return 1;
    }

    HWND shell_wnd = FindWindowA("Shell_TrayWnd", "");
    if (!shell_wnd) {
        log_win32_fatal("Shell_TrayWnd window not found!");
        return 1;
    }

    DWORD tid = GetWindowThreadProcessId(shell_wnd, 0);
    if (!tid) {
        log_win32_fatal("Window Thread ID not found!");
        return 1;
    }

    hook = SetWindowsHookEx(WH_CALLWNDPROC, addr, dll, tid);
    if (!hook) {
        log_win32_fatal("Assigning WH_CALLWNDPROC hook failed!");
        return 1;
    }

    return 0;
}

void dll_remove()
{
    UnhookWindowsHookEx(hook);
    hook = 0;
    FreeLibrary(dll);
    dll = 0;
}

int main(void)
{
    // Make sure notificue isn't already running
    if (is_running()) {
        log_error("notificue is already running! Quitting...");
        goto cleanup;
    }

    // Init
    log_debug("Hello! Opening log file...");
    if (start_logging()) goto cleanup;
    if (create_main_window()) goto cleanup;
    if (dll_inject()) goto cleanup;
    log_debug("Started!");

    // Begin main loop
    MSG msg;
    while (GetMessage(&msg, 0, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Clean up
cleanup:
    log_debug("Cleaning up...");
    dll_remove();
    destroy_main_window();
    stop_logging();
    log_debug("Bye!");
    return 0;
}
