#pragma once
#include <gtk/gtk.h>

class SystemTrayWin32 {
public:
    static void init_tray(GtkWindow* mainWindow);
    /*
     * 函数名：init_tray
     * 主要作用：调用Win32 API在Windows任务栏生成托盘图标。
     */

    static void destroy_tray();
    /*
     * 函数名：destroy_tray
     * 主要作用：安全拔除任务栏图标，防止产生幽灵残留。
     */
};