#pragma once
#include <gtk/gtk.h>
#include <string>
#include "config.h"
#include <memory>

// 前向声明，避免在头文件中互相#include
class SystemTrayWin32;

class ClipboardToolApp {
public:
    GtkApplication* app = nullptr;
    GtkWidget* win = nullptr;
    GtkWidget* enableSwitch = nullptr; // 新增：UI开关控件
    bool isCleanerEnabled = true;      // 修改：设为公有并初始化
    gulong clip_handler_id = 0;
    std::string lastStr;
    ClipboardConfig config;
    std::unique_ptr<UIStrings> uiStrings;

    ClipboardToolApp();
    int run(int argc, char* argv[]);
    void sync_ui_state();

private:
    GtkTextBuffer* logBuffer = nullptr;

    static void on_activate(GtkApplication* app, gpointer user_data);
    static void on_check_toggled(GtkCheckButton* btn, gpointer user_data);
    static void on_entry_changed(GtkEditable* edt, gpointer user_data);
    static void on_quit_clicked(GtkButton* btn, gpointer user_data);
    static void on_clip_changed(GdkClipboard* clip, gpointer user_data);
    static void on_clip_read(GObject* src, GAsyncResult* res, gpointer user_data);
    static void on_enable_switch_toggled(GObject* sw, GParamSpec* pspec, gpointer user_data);

    void append_log(const std::string& msg);
};