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