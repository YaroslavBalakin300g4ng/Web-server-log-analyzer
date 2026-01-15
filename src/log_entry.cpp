#include "log_entry.h"
#include <regex>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <windows.h>

using namespace std;

// Проверка валидности всей записи
bool LogEntry::isValid() const {
    return validateTimestamp(timestamp) &&
        validateIP(ip) &&
        validateMethod(method) &&
        validateStatus(status);
}

// Валидация timestamp в формате ISO 8601
bool LogEntry::validateTimestamp(const string& ts) {
    // Простая проверка формата: YYYY-MM-DDTHH:MM:SSZ
    regex isoPattern(R"(^\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}Z$)");
    return regex_match(ts, isoPattern);
}

// Валидация IPv4 адреса
bool LogEntry::validateIP(const string& ip) {
    regex ipPattern(R"(^(\d{1,3})\.(\d{1,3})\.(\d{1,3})\.(\d{1,3})$)");
    smatch match;

    if (!regex_match(ip, match, ipPattern)) {
        return false;
    }

    // Проверка диапазона октетов
    for (int i = 1; i <= 4; i++) {
        int octet = stoi(match[i].str());
        if (octet < 0 || octet > 255) {
            return false;
        }
    }

    return true;
}

// Валидация HTTP метода
bool LogEntry::validateMethod(const string& method) {
    vector<string> validMethods = { "GET", "POST", "PUT", "DELETE", "HEAD",
                                   "OPTIONS", "PATCH", "CONNECT", "TRACE" };
    string upperMethod = method;
    transform(upperMethod.begin(), upperMethod.end(), upperMethod.begin(), ::toupper);

    return find(validMethods.begin(), validMethods.end(), upperMethod) != validMethods.end();
}

// Валидация статус кода
bool LogEntry::validateStatus(int status) {
    return status >= 100 && status <= 599;
}

// Преобразование string в wstring для Windows
wstring LogEntry::toWideString(const string& str) {
    if (str.empty()) return wstring();

    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), NULL, 0);
    wstring wstr(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), &wstr[0], size_needed);
    return wstr;
}

// Преобразование wstring в string для Windows
string LogEntry::fromWideString(const wstring& wstr) {
    if (wstr.empty()) return string();

    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), NULL, 0, NULL, NULL);
    string str(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), &str[0], size_needed, NULL, NULL);
    return str;
}

// Создание LogEntry из JSON строки
LogEntry LogEntry::fromJsonString(const string& jsonStr) {
    // Парсинг простого JSON формата
    regex pattern(R"(\{"ts":"([^"]+)", "ip":"([^"] + )","method":"([^ "]+)", "url":"([^"] + )","status":(\d+)\})");
    smatch match;

    if (regex_search(jsonStr, match, pattern) && match.size() == 6) {
        string timestamp = match[1].str();
        string ip = match[2].str();
        string method = match[3].str();
        string url = match[4].str();
        int status = stoi(match[5].str());

        return LogEntry(timestamp, ip, method, url, status);
    }

    throw LogValidationException("Неверный формат JSON строки");
}

// Преобразование в строку для отладки
string LogEntry::toString() const {
    ostringstream oss;
    oss << "Timestamp: " << timestamp << "\n"
        << "IP: " << ip << "\n"
        << "Method: " << method << "\n"
        << "URL: " << url << "\n"
        << "Status: " << status;
    return oss.str();
}