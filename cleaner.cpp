#include "cleaner.h"
#include <glib.h>
#include <vector>
#include <regex>
#include <iostream>
#include <cassert>

void replace_all_substrings(std::string& targetStr, const std::string& searchStr, const std::string& replaceStr) {
    if (searchStr.empty()) return;
    size_t pos = 0;
    while ((pos = targetStr.find(searchStr, pos)) != std::string::npos) {
        targetStr.replace(pos, searchStr.length(), replaceStr);
        pos += replaceStr.length();
    }
}

std::string clean_pdf_text(const std::string& rawText, const ClipboardConfig& config) {
    if (rawText.empty()) return "";
    std::string text = rawText;

    std::vector<std::string> whiteList;
    std::string tempSym;
    for (char c : config.preservedSymbols) {
        if (c == ' ') {
            if (!tempSym.empty()) { whiteList.push_back(tempSym); tempSym.clear(); }
        } else { tempSym += c; }
    }
    if (!tempSym.empty()) whiteList.push_back(tempSym);

    std::vector<std::string> tags;
    for (size_t i = 0; i < whiteList.size(); ++i) {
        std::string t = "\x01TAG" + std::to_string(i) + "\x02";
        tags.push_back(t);
        replace_all_substrings(text, whiteList[i], t);
    }

    char* normStr = g_utf8_normalize(text.c_str(), -1, G_NORMALIZE_NFKC);
    if (normStr) { text = normStr; g_free(normStr); }

    for (size_t i = 0; i < whiteList.size(); ++i) {
        replace_all_substrings(text, tags[i], whiteList[i]);
    }

    text = std::regex_replace(text, std::regex(R"(([\w])-[\s\r]*\n[\s\r]*([\w]))"), "$1$2");

    if (config.replaceNewlinesWithSpaces) {
        replace_all_substrings(text, "\r\n", " ");
        replace_all_substrings(text, "\r", " ");
        replace_all_substrings(text, "\n", " ");
    }

    text = std::regex_replace(text, std::regex(R"(\s+)"), " ");

    size_t start = text.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) return "";
    return text.substr(start, text.find_last_not_of(" \t\n\r") - start + 1);
}

void run_cleaner_unit_tests() {
    ClipboardConfig cfg;
    std::string t1 = "Ｐｙｔｈｏｎ ３．１１";
    assert(clean_pdf_text(t1, cfg) == "Python 3.11");

    std::string t2 = "保留中文，句号。和问号？";
    assert(clean_pdf_text(t2, cfg) == "保留中文，句号。和问号?");

    std::string t3 = "Com-\nputer";
    assert(clean_pdf_text(t3, cfg) == "Computer");
}