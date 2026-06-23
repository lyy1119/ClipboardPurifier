#pragma once
#include <string>
#include "config.h"

std::string clean_pdf_text(const std::string& rawText, const ClipboardConfig& config);
/*
 * 函数名：clean_pdf_text
 * 主要作用：执行全角转半角、回车空格净化及白名单符号保护。
 */

void run_cleaner_unit_tests();
/*
 * 函数名：run_cleaner_unit_tests
 * 主要作用：在应用启动前断言拦截清洗算法的正确性。
 */