#include "trayWin32.h"
#include "appUi.h"
#include "config.h" // 引入config.h以使用UIStrings
#include <windows.h>
#include <shellapi.h>
#include <vector>

static HWND trayMsgHwnd = NULL;
static ClipboardToolApp* cachedApp = nullptr; // 从 GtkWindow* 改为 ClipboardToolApp*

// 辅助函数：将 UTF-8 std::string 转换为 std::wstring
static std::wstring utf8_to_wstring(const std::string& str) {
    if (str.empty()) return std::wstring();
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}
#define WM_TRAY_CLICK (WM_USER + 101)

#define ID_TRAY_SHOW_WINDOW 10001
#define ID_TRAY_EXIT 10002
#define ID_TRAY_TOGGLE_ENABLED 10003

void SystemTrayWin32::update_menu_state() {
    if (!cachedApp) return;
    const auto& strings = *cachedApp->uiStrings;
    HMENU hMenu = CreatePopupMenu();
    AppendMenuW(hMenu, MF_STRING | (cachedApp->isCleanerEnabled ? MF_CHECKED : MF_UNCHECKED), ID_TRAY_TOGGLE_ENABLED, utf8_to_wstring(strings.enable_purification).c_str());
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
                    const auto& strings = *cachedApp->uiStrings;
                    GetCursorPos(&pt);
                    HMENU hMenu = CreatePopupMenu();
                    AppendMenuW(hMenu, MF_STRING | (cachedApp->isCleanerEnabled ? MF_CHECKED : MF_UNCHECKED), ID_TRAY_TOGGLE_ENABLED, utf8_to_wstring(strings.enable_purification).c_str());
                    AppendMenuW(hMenu, MF_STRING, ID_TRAY_SHOW_WINDOW, utf8_to_wstring(strings.tray_show_window).c_str());
                    AppendMenuW(hMenu, MF_STRING, ID_TRAY_EXIT, utf8_to_wstring(strings.exit_app).c_str());

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
    const auto& strings = *cachedApp->uiStrings;

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

    std::wstring tip_text = utf8_to_wstring(strings.title);
    wcscpy_s(nid.szTip, tip_text.c_str());
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