#include "tray_win32.h"
#include <windows.h>
#include <shellapi.h>

static HWND trayMsgHwnd = NULL;
static GtkWindow* cachedGtkWindow = nullptr;
#define WM_TRAY_CLICK (WM_USER + 101)

static LRESULT CALLBACK tray_wnd_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_TRAY_CLICK) {
        if (lParam == WM_LBUTTONDBLCLK || lParam == WM_RBUTTONUP) {
            if (cachedGtkWindow) {
                // 由Win32消息泵在GLib主线程中触发，安全调用GTK API呼出窗口
                gtk_window_present(cachedGtkWindow);
            }
        }
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

void SystemTrayWin32::init_tray(GtkWindow* mainWindow) {
    cachedGtkWindow = mainWindow;

    WNDCLASSW wc = {0};
    wc.lpfnWndProc = tray_wnd_proc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = L"GtkWin32TrayClass";
    RegisterClassW(&wc);

    // 创建一个不可见的 Message-Only 窗口专职接收托盘双击事件
    trayMsgHwnd = CreateWindowExW(0, wc.lpszClassName, L"", 0,0,0,0,0, HWND_MESSAGE, NULL, wc.hInstance, NULL);

    NOTIFYICONDATAW nid = {0};
    nid.cbSize = sizeof(NOTIFYICONDATAW);
    nid.hWnd = trayMsgHwnd;
    nid.uID = 1001;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAY_CLICK;
    nid.hIcon = LoadIcon(NULL, IDI_INFORMATION);
    wcscpy_s(nid.szTip, L"剪贴板文献净化工具 (双击唤醒)");

    Shell_NotifyIconW(NIM_ADD, &nid);
}

void SystemTrayWin32::destroy_tray() {
    if (trayMsgHwnd) {
        NOTIFYICONDATAW nid = {0};
        nid.cbSize = sizeof(NOTIFYICONDATAW);
        nid.hWnd = trayMsgHwnd;
        nid.uID = 1001;
        Shell_NotifyIconW(NIM_DELETE, &nid);
        DestroyWindow(trayMsgHwnd);
        trayMsgHwnd = NULL;
    }
}