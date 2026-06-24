#include "trayWin32.h"
#include "appUi.h"
#include <windows.h>
#include <shellapi.h>

static HWND trayMsgHwnd = NULL;
static ClipboardToolApp* cachedApp = nullptr; // 从 GtkWindow* 改为 ClipboardToolApp*
#define WM_TRAY_CLICK (WM_USER + 101)

#define ID_TRAY_SHOW_WINDOW 10001
#define ID_TRAY_EXIT 10002
#define ID_TRAY_TOGGLE_ENABLED 10003

void SystemTrayWin32::update_menu_state() {
    HMENU hMenu = CreatePopupMenu();
    AppendMenuW(hMenu, MF_STRING | (cachedApp->isCleanerEnabled ? MF_CHECKED : MF_UNCHECKED), ID_TRAY_TOGGLE_ENABLED, L"开启净化");
    // 此处仅为更新状态，不实际显示，所以不需要 TrackPopupMenu
    DestroyMenu(hMenu);
}
static LRESULT CALLBACK tray_wnd_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_TRAY_CLICK:
            switch (lParam) {
                case WM_LBUTTONDBLCLK:
                    if (cachedApp && cachedApp->win) {
                        // 由Win32消息泵在GLib主线程中触发，安全调用GTK API呼出窗口
                        gtk_window_present(GTK_WINDOW(cachedApp->win));
                    }
                    break;
                case WM_RBUTTONUP: {
                    POINT pt;
                    GetCursorPos(&pt);
                    HMENU hMenu = CreatePopupMenu();
                    AppendMenuW(hMenu, MF_STRING | (cachedApp->isCleanerEnabled ? MF_CHECKED : MF_UNCHECKED), ID_TRAY_TOGGLE_ENABLED, L"开启净化");
                    AppendMenuW(hMenu, MF_STRING, ID_TRAY_SHOW_WINDOW, L"显示主窗口");
                    AppendMenuW(hMenu, MF_STRING, ID_TRAY_EXIT, L"退出");

                    SetForegroundWindow(hwnd);
                    TrackPopupMenu(hMenu, TPM_RIGHTALIGN | TPM_BOTTOMALIGN, pt.x, pt.y, 0, hwnd, NULL);
                    DestroyMenu(hMenu);
                    break;
                }
            }
            break;
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case ID_TRAY_SHOW_WINDOW:
                    if (cachedApp && cachedApp->win) {
                        gtk_window_present(GTK_WINDOW(cachedApp->win));
                    }
                    break;
                case ID_TRAY_EXIT:
                    if (cachedApp && cachedApp->app) {
                        GtkApplication* app = cachedApp->app;
                        if (G_IS_APPLICATION(app)) {
                            g_application_quit(G_APPLICATION(app));
                        }
                    }
                    break;
                case ID_TRAY_TOGGLE_ENABLED:
                    if (cachedApp) {
                        cachedApp->isCleanerEnabled = !cachedApp->isCleanerEnabled;
                        cachedApp->sync_ui_state();
                    }
                    break;
            }
            break;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

void SystemTrayWin32::init_tray(ClipboardToolApp* app) {
    cachedApp = app;

    WNDCLASSW wc = {0};
    wc.cbWndExtra = sizeof(ClipboardToolApp*);
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
    wcscpy_s(nid.szTip, L"剪贴板净化工具 (双击唤醒)");

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