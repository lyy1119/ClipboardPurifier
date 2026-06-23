#include "appUi.h"
#include "cleaner.h"
#include "trayWin32.h"

ClipboardToolApp::ClipboardToolApp() {
    app = gtk_application_new("com.coder.winclipboard", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), this);
}

int ClipboardToolApp::run(int argc, char* argv[]) {
    int res = g_application_run(G_APPLICATION(app), argc, argv);
    SystemTrayWin32::destroy_tray(); // 退出时拔除托盘图标
    g_object_unref(app);
    return res;
}

void ClipboardToolApp::append_log(const std::string& msg) {
    GtkTextIter iter;
    gtk_text_buffer_get_end_iter(logBuffer, &iter);
    gtk_text_buffer_insert(logBuffer, &iter, msg.c_str(), -1);
}

void ClipboardToolApp::on_check_toggled(GtkCheckButton* btn, gpointer user_data) {
    static_cast<ClipboardToolApp*>(user_data)->config.replaceNewlinesWithSpaces = gtk_check_button_get_active(btn);
}

void ClipboardToolApp::on_entry_changed(GtkEditable* edt, gpointer user_data) {
    const char* txt = gtk_editable_get_text(edt);
    static_cast<ClipboardToolApp*>(user_data)->config.preservedSymbols = txt ? txt : "";
}

void ClipboardToolApp::on_quit_clicked(GtkButton* btn, gpointer user_data) {
    g_application_quit(G_APPLICATION(static_cast<ClipboardToolApp*>(user_data)->app));
}

void ClipboardToolApp::on_clip_changed(GdkClipboard* clip, gpointer user_data) {
    auto* self = static_cast<ClipboardToolApp*>(user_data);
    gdk_clipboard_read_text_async(clip, nullptr, on_clip_read, self);
}

void ClipboardToolApp::on_clip_read(GObject* src, GAsyncResult* res, gpointer user_data) {
    auto* self = static_cast<ClipboardToolApp*>(user_data);
    char* txt = gdk_clipboard_read_text_finish(GDK_CLIPBOARD(src), res, nullptr);
    if (!txt) return;

    std::string curr(txt);
    g_free(txt);

    if (curr != self->lastStr && !curr.empty()) {
        std::string fixed = clean_pdf_text(curr, self->config);
        if (fixed != curr && !fixed.empty()) {
            // Block the "changed" signal handler to prevent re-entrancy.
            g_signal_handler_block(GDK_CLIPBOARD(src), self->clip_handler_id);

            self->lastStr = fixed;
            gdk_clipboard_set_text(GDK_CLIPBOARD(src), fixed.c_str());

            // Unblock the handler immediately after setting the text.
            g_signal_handler_unblock(GDK_CLIPBOARD(src), self->clip_handler_id);

            std::string log = "✅ [已净化] 长度: " + std::to_string(fixed.length()) + "\n预览: " + fixed.substr(0, 36) + "...\n\n";
            self->append_log(log);

        } else { self->lastStr = curr; }
    }
}

void ClipboardToolApp::on_activate(GtkApplication* app, gpointer user_data) {
    auto* self = static_cast<ClipboardToolApp*>(user_data);
    self->win = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(self->win), "剪贴板文献净化工具");
    gtk_window_set_default_size(GTK_WINDOW(self->win), 500, 350);
    gtk_window_set_hide_on_close(GTK_WINDOW(self->win), TRUE); // 点X转入后台

    GtkWidget* box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_start(box, 15); gtk_widget_set_margin_end(box, 15);
    gtk_widget_set_margin_top(box, 15); gtk_widget_set_margin_bottom(box, 15);
    gtk_window_set_child(GTK_WINDOW(self->win), box);

    GtkWidget* chk = gtk_check_button_new_with_label("自动将换行符替换为空格");
    gtk_check_button_set_active(GTK_CHECK_BUTTON(chk), self->config.replaceNewlinesWithSpaces);
    g_signal_connect(chk, "toggled", G_CALLBACK(on_check_toggled), self);
    gtk_box_append(GTK_BOX(box), chk);

    GtkWidget* hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget* edt = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(edt), self->config.preservedSymbols.c_str());
    gtk_widget_set_hexpand(edt, TRUE);
    g_signal_connect(edt, "changed", G_CALLBACK(on_entry_changed), self);
    gtk_box_append(GTK_BOX(hbox), gtk_label_new("保留符号："));
    gtk_box_append(GTK_BOX(hbox), edt);
    gtk_box_append(GTK_BOX(box), hbox);

    // 增设：日志输出面板（弥补-mwindows看不到控制台的缺陷）
    GtkWidget* scroll = gtk_scrolled_window_new();
    gtk_widget_set_vexpand(scroll, TRUE);
    self->logBuffer = gtk_text_buffer_new(nullptr);
    GtkWidget* tv = gtk_text_view_new_with_buffer(self->logBuffer);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(tv), FALSE);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), tv);
    gtk_box_append(GTK_BOX(box), scroll);

    GtkWidget* btn = gtk_button_new_with_label("彻底退出程序");
    g_signal_connect(btn, "clicked", G_CALLBACK(on_quit_clicked), self);
    gtk_box_append(GTK_BOX(box), btn);

    GdkClipboard* clip = gdk_display_get_clipboard(gdk_display_get_default());
    self->clip_handler_id = g_signal_connect(clip, "changed", G_CALLBACK(on_clip_changed), self);

    SystemTrayWin32::init_tray(GTK_WINDOW(self->win));
    gtk_window_present(GTK_WINDOW(self->win));
}