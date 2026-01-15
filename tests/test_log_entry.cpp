#include <iostream>
#include <string>
#include <vector>
#include <cassert>
#include "log_entry.h"

using namespace std;

// Тестирование валидации timestamp
void testTimestampValidation() {
    cout << "Тестирование валидации timestamp...\n";

    // Корректные timestamp
    assert(LogEntry::validateTimestamp("2025-03-14T12:03:21Z") == true);
    assert(LogEntry::validateTimestamp("2024-12-31T23:59:59Z") == true);
    assert(LogEntry::validateTimestamp("2023-01-01T00:00:00Z") == true);

    // Некорректные timestamp
    assert(LogEntry::validateTimestamp("2025-03-14T12:03:21") == false); // нет Z
    assert(LogEntry::validateTimestamp("2025-03-14 12:03:21Z") == false); // пробел вместо T
    assert(LogEntry::validateTimestamp("2025-13-14T12:03:21Z") == false); // неверный месяц
    assert(LogEntry::validateTimestamp("2025-03-14T25:03:21Z") == false); // неверный час
    assert(LogEntry::validateTimestamp("2025-03-14T12:60:21Z") == false); // неверная минута
    assert(LogEntry::validateTimestamp("2025-03-14T12:03:61Z") == false); // неверная секунда
    assert(LogEntry::validateTimestamp("") == false); // пустая строка
    assert(LogEntry::validateTimestamp("invalid") == false); // произвольная строка

    cout << "✓ Все тесты валидации timestamp пройдены\n\n";
}

// Тестирование валидации IP-адресов
void testIPValidation() {
    cout << "Тестирование валидации IP-адресов...\n";

    // Корректные IP
    assert(LogEntry::validateIP("192.168.1.1") == true);
    assert(LogEntry::validateIP("10.0.0.1") == true);
    assert(LogEntry::validateIP("172.16.0.1") == true);
    assert(LogEntry::validateIP("255.255.255.255") == true);
    assert(LogEntry::validateIP("0.0.0.0") == true);

    // Некорректные IP
    assert(LogEntry::validateIP("256.168.1.1") == false); // октет > 255
    assert(LogEntry::validateIP("192.168.1") == false);   // не хватает октета
    assert(LogEntry::validateIP("192.168.1.1.1") == false); // лишний октет
    assert(LogEntry::validateIP("192.168.1.1a") == false); // буквы в октете
    assert(LogEntry::validateIP("") == false); // пустая строка
    assert(LogEntry::validateIP("localhost") == false); // доменное имя

    cout << "✓ Все тесты валидации IP пройдены\n\n";
}

// Тестирование валидации методов HTTP
void testMethodValidation() {
    cout << "Тестирование валидации методов HTTP...\n";

    // Корректные методы (регистр не важен)
    assert(LogEntry::validateMethod("GET") == true);
    assert(LogEntry::validateMethod("get") == true);
    assert(LogEntry::validateMethod("Get") == true);
    assert(LogEntry::validateMethod("POST") == true);
    assert(LogEntry::validateMethod("PUT") == true);
    assert(LogEntry::validateMethod("DELETE") == true);
    assert(LogEntry::validateMethod("HEAD") == true);
    assert(LogEntry::validateMethod("OPTIONS") == true);
    assert(LogEntry::validateMethod("PATCH") == true);
    assert(LogEntry::validateMethod("CONNECT") == true);
    assert(LogEntry::validateMethod("TRACE") == true);

    // Некорректные методы
    assert(LogEntry::validateMethod("") == false);
    assert(LogEntry::validateMethod("GETS") == false);
    assert(LogEntry::validateMethod("POSTS") == false);
    assert(LogEntry::validateMethod("PUTT") == false);
    assert(LogEntry::validateMethod("INVALID") == false);

    cout << "✓ Все тесты валидации методов пройдены\n\n";
}

// Тестирование валидации статус кодов
void testStatusValidation() {
    cout << "Тестирование валидации статус кодов...\n";

    // Корректные статусы
    assert(LogEntry::validateStatus(100) == true);
    assert(LogEntry::validateStatus(200) == true);
    assert(LogEntry::validateStatus(301) == true);
    assert(LogEntry::validateStatus(404) == true);
    assert(LogEntry::validateStatus(500) == true);
    assert(LogEntry::validateStatus(599) == true);

    // Некорректные статусы
    assert(LogEntry::validateStatus(99) == false);   // меньше 100
    assert(LogEntry::validateStatus(600) == false);  // больше 599
    assert(LogEntry::validateStatus(0) == false);
    assert(LogEntry::validateStatus(-1) == false);
    assert(LogEntry::validateStatus(999) == false);

    cout << "✓ Все тесты валидации статусов пройдены\n\n";
}

// Тестирование создания и валидации LogEntry
void testLogEntryCreation() {
    cout << "Тестирование создания LogEntry...\n";

    // Корректная запись
    LogEntry validEntry("2025-03-14T12:03:21Z", "192.168.1.1", "GET", "/index.html", 200);
    assert(validEntry.isValid() == true);
    assert(validEntry.timestamp == "2025-03-14T12:03:21Z");
    assert(validEntry.ip == "192.168.1.1");
    assert(validEntry.method == "GET");
    assert(validEntry.url == "/index.html");
    assert(validEntry.status == 200);

    // Запись с неверным timestamp
    LogEntry invalidTimestamp("2025-13-14T12:03:21Z", "192.168.1.1", "GET", "/index.html", 200);
    assert(invalidTimestamp.isValid() == false);

    // Запись с неверным IP
    LogEntry invalidIP("2025-03-14T12:03:21Z", "256.168.1.1", "GET", "/index.html", 200);
    assert(invalidIP.isValid() == false);

    // Запись с неверным методом
    LogEntry invalidMethod("2025-03-14T12:03:21Z", "192.168.1.1", "INVALID", "/index.html", 200);
    assert(invalidMethod.isValid() == false);

    // Запись с неверным статусом
    LogEntry invalidStatus("2025-03-14T12:03:21Z", "192.168.1.1", "GET", "/index.html", 999);
    assert(invalidStatus.isValid() == false);

    // Проверка метода toString
    string str = validEntry.toString();
    assert(str.find("2025-03-14T12:03:21Z") != string::npos);
    assert(str.find("192.168.1.1") != string::npos);
    assert(str.find("GET") != string::npos);
    assert(str.find("/index.html") != string::npos);
    assert(str.find("200") != string::npos);

    cout << "✓ Все тесты создания LogEntry пройдены\n\n";
}

// Тестирование исключений
void testExceptions() {
    cout << "Тестирование исключений...\n";

    bool exceptionThrown = false;

    // Тест TimestampFormatException
    try {
        throw TimestampFormatException("2025-13-14T12:03:21Z");
        exceptionThrown = false;
    }
    catch (const TimestampFormatException& e) {
        exceptionThrown = true;
        assert(string(e.what()).find("Неверный формат времени") != string::npos);
    }
    assert(exceptionThrown == true);

    // Тест IPFormatException
    exceptionThrown = false;
    try {
        throw IPFormatException("256.168.1.1");
        exceptionThrown = false;
    }
    catch (const IPFormatException& e) {
        exceptionThrown = true;
        assert(string(e.what()).find("Некорректный IP-адрес") != string::npos);
    }
    assert(exceptionThrown == true);

    // Тест StatusRangeException
    exceptionThrown = false;
    try {
        throw StatusRangeException(999);
        exceptionThrown = false;
    }
    catch (const StatusRangeException& e) {
        exceptionThrown = true;
        assert(string(e.what()).find("Недопустимый статус код") != string::npos);
    }
    assert(exceptionThrown == true);

    // Тест MethodException
    exceptionThrown = false;
    try {
        throw MethodException("INVALID");
        exceptionThrown = false;
    }
    catch (const MethodException& e) {
        exceptionThrown = true;
        assert(string(e.what()).find("Неизвестный HTTP метод") != string::npos);
    }
    assert(exceptionThrown == true);

    cout << "✓ Все тесты исключений пройдены\n\n";
}

// Тестирование преобразований строк для Windows
void testStringConversions() {
    cout << "Тестирование преобразований строк (Windows)...\n";

    // Тест ASCII строк
    string asciiStr = "Hello World!";
    wstring wideAscii = LogEntry::toWideString(asciiStr);
    string backAscii = LogEntry::fromWideString(wideAscii);
    assert(asciiStr == backAscii);

    // Тест русских строк
    string russianStr = "Привет, мир!";
    wstring wideRussian = LogEntry::toWideString(russianStr);
    string backRussian = LogEntry::fromWideString(wideRussian);
    assert(russianStr == backRussian);

    // Тест пустой строки
    string emptyStr = "";
    wstring wideEmpty = LogEntry::toWideString(emptyStr);
    string backEmpty = LogEntry::fromWideString(wideEmpty);
    assert(emptyStr == backEmpty);
    assert(wideEmpty.empty() == true);

    // Тест строки с символами UTF-8
    string utf8Str = "Hello 世界! Привет!";
    wstring wideUtf8 = LogEntry::toWideString(utf8Str);
    string backUtf8 = LogEntry::fromWideString(wideUtf8);
    assert(utf8Str == backUtf8);

    cout << "✓ Все тесты преобразования строк пройдены\n\n";
}

// Тестирование парсинга из JSON строки
void testFromJsonString() {
    cout << "Тестирование парсинга из JSON строки...\n";

    // Корректный JSON
    string jsonStr = R"({"ts":"2025-03-14T12:03:21Z","ip":"192.168.1.1","method":"GET","url":"/index.html","status":200})";

    LogEntry entry = LogEntry::fromJsonString(jsonStr);
    assert(entry.timestamp == "2025-03-14T12:03:21Z");
    assert(entry.ip == "192.168.1.1");
    assert(entry.method == "GET");
    assert(entry.url == "/index.html");
    assert(entry.status == 200);
    assert(entry.isValid() == true);

    // Некорректный JSON
    bool exceptionThrown = false;
    try {
        string invalidJson = R"({"ts":"invalid","ip":"192.168.1.1","method":"GET","url":"/index.html","status":200})";
        LogEntry invalidEntry = LogEntry::fromJsonString(invalidJson);
        exceptionThrown = false;
    }
    catch (const LogValidationException&) {
        exceptionThrown = true;
    }
    assert(exceptionThrown == true);

    cout << "✓ Все тесты парсинга JSON строки пройдены\n\n";
}

// Генерация тестовых данных
vector<LogEntry> generateTestLogs(int count) {
    vector<LogEntry> logs;
    logs.reserve(count);

    vector<string> methods = { "GET", "POST", "PUT", "DELETE" };
    vector<int> statuses = { 200, 201, 404, 500, 301 };
    vector<string> urls = { "/index.html", "/api/data", "/users", "/products", "/login" };

    for (int i = 0; i < count; i++) {
        string timestamp = "2025-03-" + to_string(15 + i % 10) + "T"
            + to_string(10 + i % 10) + ":"
            + to_string(30 + i % 30) + ":"
            + to_string(i % 60) + "Z";

        string ip = "192.168.1." + to_string(1 + i % 255);
        string method = methods[i % methods.size()];
        string url = urls[i % urls.size()] + "?id=" + to_string(i);
        int status = statuses[i % statuses.size()];

        logs.emplace_back(timestamp, ip, method, url, status);
    }

    return logs;
}

// Тестирование с тестовыми данными
void testWithGeneratedData() {
    cout << "Тестирование с генерированными данными...\n";

    int testCount = 1000;
    vector<LogEntry> logs = generateTestLogs(testCount);

    // Проверяем, что все записи валидны
    int validCount = 0;
    for (const auto& log : logs) {
        if (log.isValid()) {
            validCount++;
        }
    }
    assert(validCount == testCount);

    // Проверяем уникальность некоторых данных
    int uniqueIPs = 0;
    int uniqueURLs = 0;

    // Простая проверка уникальности
    for (size_t i = 0; i < logs.size(); i++) {
        bool ipUnique = true;
        bool urlUnique = true;

        for (size_t j = 0; j < i; j++) {
            if (logs[i].ip == logs[j].ip) ipUnique = false;
            if (logs[i].url == logs[j].url) urlUnique = false;
        }

        if (ipUnique) uniqueIPs++;
        if (urlUnique) uniqueURLs++;
    }

    assert(uniqueIPs > 0);
    assert(uniqueURLs > 0);

    cout << "✓ Сгенерировано " << testCount << " тестовых записей\n";
    cout << "✓ Уникальных IP: " << uniqueIPs << "\n";
    cout << "✓ Уникальных URL: " << uniqueURLs << "\n";
    cout << "✓ Все тесты с генерированными данными пройдены\n\n";
}

// Главная функция тестирования
int main() {
    cout << "========================================\n";
    cout << "     ТЕСТИРОВАНИЕ LogEntry (Windows)     \n";
    cout << "========================================\n\n";

    try {
        testTimestampValidation();
        testIPValidation();
        testMethodValidation();
        testStatusValidation();
        testLogEntryCreation();
        testExceptions();
        testStringConversions();
        testFromJsonString();
        testWithGeneratedData();

        cout << "========================================\n";
        cout << "  ВСЕ ТЕСТЫ УСПЕШНО ПРОЙДЕНЫ! 🎉\n";
        cout << "========================================\n";

        return 0;

    }
    catch (const exception& e) {
        cerr << "\n✗ ОШИБКА ТЕСТИРОВАНИЯ: " << e.what() << endl;
        return 1;
    }
    catch (...) {
        cerr << "\n✗ НЕИЗВЕСТНАЯ ОШИБКА ТЕСТИРОВАНИЯ" << endl;
        return 1;
    }
}