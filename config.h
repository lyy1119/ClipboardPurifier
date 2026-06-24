#pragma once
#include <string>

/*
 * 类名：ClipboardConfig
 * 主要作用：保存用户设定的净化规则配置
 */
struct ClipboardConfig {
    bool replaceNewlinesWithSpaces = true;
    std::string preservedSymbols = "， 。 “ ”";
};

/*
 * 结构体名：UIStrings
 * 主要作用：保存应用界面和托盘菜单的所有文本，方便修改和国际化
 */
struct UIStrings {
    // --- 通用 ---
    const std::string app_id = "com.coder.winclipboard";
    const std::string title = "剪贴板净化工具 v1.0.0";
    const std::string enable_purification = "开启净化";
    const std::string exit_app = "退出";

    // --- 主窗口 ---
    const std::string enable_purification_label = "开启剪贴板净化：";
    const std::string replace_newlines_label = "自动将换行符替换为空格";
    const std::string preserved_symbols_label = "保留符号：";
    const std::string quit_button_label = "退出程序";
    const std::string log_prefix = "✅ [已净化] 长度: ";
    const std::string log_suffix_preview = "\n预览: ";
    const std::string log_suffix_end = "...\n\n";

    // --- 系统托盘 ---
    const std::string tray_show_window = "显示主窗口";
};