#pragma once
#include <gtk/gtk.h>
#include <string>
#include "config.h"

class ClipboardToolApp {
private:
    GtkApplication* app;
    GtkWidget* win;
    GtkTextBuffer* logBuffer;
    ClipboardConfig config;
    std::string lastStr;
    bool writeLock;

    static void on_activate(GtkApplication* app, gpointer user_data);
    static void on_check_toggled(GtkCheckButton* btn, gpointer user_data);
    static void on_entry_changed(GtkEditable* edt, gpointer user_data);
    static void on_quit_clicked(GtkButton* btn, gpointer user_data);
    static void on_clip_changed(GdkClipboard* clip, gpointer user_data);
    static void on_clip_read(GObject* src, GAsyncResult* res, gpointer user_data);

    void append_log(const std::string& msg);

public:
    ClipboardToolApp();
    int run(int argc, char* argv[]);
};