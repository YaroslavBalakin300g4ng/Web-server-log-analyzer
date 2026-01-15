#include <iostream>
#include <string>
#include <vector>
#include <cassert>
#include <chrono>
#include "analyzer.h"
#include "log_entry.h"

using namespace std;
using namespace chrono;

// Генерация тестовых данных для анализатора
vector<LogEntry> generateTestLogsForAnalyzer(int count) {
    vector<LogEntry> logs;
    logs.reserve(count);

    vector<string> methods = { "GET", "POST", "PUT", "DELETE", "GET", "GET", "POST" }; // GET чаще
    vector<int> statuses = { 200, 200, 200, 404, 500, 301, 200, 200 }; // 200 чаще
    vector<string> urls = {
        "/index.html", "/api/data", "/users", "/products",
        "/login", "/admin", "/dashboard", "/api/v1/users",
        "/api/v1/products", "/static/css/style.css"
    };
    vector<string> ips = {
        "192.168.1.1", "10.0.0.1", "172.16.0.1", "192.168.1.2",
        "10.0.0.2", "192.168.1.100", "192.168.1.101", "203.0.113.7"
    };

    // Делаем некоторые IP более частыми
    vector<string> frequentIPs = { "192.168.1.1", "10.0.0.1", "203.0.113.7" };
    vector<string> frequentURLs = { "/index.html", "/api/data", "/users" };

    for (int i = 0; i < count; i++) {
        // Генерация timestamp с прогрессией времени
        int day = 14 + (i / 1000) % 7; // 7 дней данных
        int hour = 8 + (i / 100) % 12; // с 8:00 до 20:00
        int minute = i % 60;
        int second = (i * 7) % 60;

        string timestamp = "2025-03-" + (day < 10 ? "0" : "") + to_string(day) + "T"
            + (hour < 10 ? "0" : "") + to_string(hour) + ":"
            + (minute < 10 ? "0" : "") + to_string(minute) + ":"
            + (second < 10 ? "0" : "") + to_string(second) + "Z";

        // Выбор IP (некоторые IP чаще)
        string ip;
        if (i % 10 < 3) { // 30% запросов от частых IP
            ip = frequentIPs[i % frequentIPs.size()];
        }
        else {
            ip = ips[i % ips.size()];
        }

        // Выбор URL (некоторые URL чаще)
        string url;
        if (i % 8 < 3) { // 37.5% запросов к частым URL
            url = frequentURLs[i % frequentURLs.size()];
        }
        else {
            url = urls[i % urls.size()];
            if (i % 5 == 0) {
                url += "?id=" + to_string(i);
            }
        }

        string method = methods[i % methods.size()];
        int status = statuses[i % statuses.size()];

        logs.emplace_back(timestamp, ip, method, url, status);
    }

    return logs;
}

// Тестирование загрузки данных
void testDataLoading() {
    cout << "Тестирование загрузки данных...\n";

    // Генерация тестовых данных
    vector<LogEntry> logs = generateTestLogsForAnalyzer(100);
    LogAnalyzer analyzer(logs);

    assert(analyzer.getTotalRequests() == 100);
    assert(analyzer.getLogs().size() == 100);

    // Проверка, что все записи валидны
    int validCount = 0;
    for (const auto& log : analyzer.getLogs()) {
        if (log.isValid()) validCount++;
    }
    assert(validCount == 100);

    cout << "✓ Загружено " << analyzer.getTotalRequests() << " записей\n";
    cout << "✓ Все записи валидны\n\n";
}

// Тестирование топ IP-адресов
void testTopIPs() {
    cout << "Тестирование топ IP-адресов...\n";

    vector<LogEntry> logs = generateTestLogsForAnalyzer(1000);
    LogAnalyzer analyzer(logs);

    auto start = high_resolution_clock::now();
    auto topIPs = analyzer.getTopIPs(5);
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);

    assert(!topIPs.empty());
    assert(topIPs.size() <= 5);

    // Проверяем, что IP отсортированы по убыванию количества запросов
    for (size_t i = 1; i < topIPs.size(); i++) {
        assert(topIPs[i - 1].second >= topIPs[i].second);
    }

    cout << "✓ Найдено " << topIPs.size() << " топ IP-адресов\n";
    cout << "✓ Время выполнения: " << duration.count() << " мс\n";

    // Выводим результаты для отладки
    cout << "Топ-3 IP-адреса:\n";
    for (size_t i = 0; i < min(topIPs.size(), size_t(3)); i++) {
        cout << "  " << (i + 1) << ". " << topIPs[i].first
            << " - " << topIPs[i].second << " запросов\n";
    }
    cout << "\n";
}

// Тестирование топ URL
void testTopURLs() {
    cout << "Тестирование топ URL...\n";

    vector<LogEntry> logs = generateTestLogsForAnalyzer(1000);
    LogAnalyzer analyzer(logs);

    auto start = high_resolution_clock::now();
    auto topURLs = analyzer.getTopURLs(5);
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);

    assert(!topURLs.empty());
    assert(topURLs.size() <= 5);

    // Проверяем сортировку
    for (size_t i = 1; i < topURLs.size(); i++) {
        assert(topURLs[i - 1].second >= topURLs[i].second);
    }

    cout << "✓ Найдено " << topURLs.size() << " топ URL\n";
    cout << "✓ Время выполнения: " << duration.count() << " мс\n";

    cout << "Топ-3 URL:\n";
    for (size_t i = 0; i < min(topURLs.size(), size_t(3)); i++) {
        cout << "  " << (i + 1) << ". " << topURLs[i].first
            << " - " << topURLs[i].second << " запросов\n";
    }
    cout << "\n";
}

// Тестирование фильтрации по статусу
void testStatusFiltering() {
    cout << "Тестирование фильтрации по статусу...\n";

    vector<LogEntry> logs = generateTestLogsForAnalyzer(1000);
    LogAnalyzer analyzer(logs);

    // Фильтруем успешные запросы (200)
    auto start = high_resolution_clock::now();
    auto successful = analyzer.filterByStatus(200);
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);

    // Проверяем, что все отфильтрованные записи имеют статус 200
    for (const auto& log : successful) {
        assert(log.status == 200);
    }

    cout << "✓ Найдено " << successful.size() << " записей со статусом 200\n";
    cout << "✓ Время выполнения: " << duration.count() << " мс\n";

    // Фильтруем ошибки (404)
    auto notFound = analyzer.filterByStatus(404);
    for (const auto& log : notFound) {
        assert(log.status == 404);
    }

    cout << "✓ Найдено " << notFound.size() << " записей со статусом 404\n\n";
}

// Тестирование фильтрации по методу
void testMethodFiltering() {
    cout << "Тестирование фильтрации по методу...\n";

    vector<LogEntry> logs = generateTestLogsForAnalyzer(1000);
    LogAnalyzer analyzer(logs);

    // Фильтруем GET запросы
    auto start = high_resolution_clock::now();
    auto getRequests = analyzer.filterByMethod("GET");
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);

    // Проверяем, что все отфильтрованные записи имеют метод GET
    for (const auto& log : getRequests) {
        assert(log.method == "GET" || log.method == "get");
    }

    cout << "✓ Найдено " << getRequests.size() << " GET запросов\n";
    cout << "✓ Время выполнения: " << duration.count() << " мс\n";

    // Фильтруем POST запросы
    auto postRequests = analyzer.filterByMethod("POST");
    for (const auto& log : postRequests) {
        assert(log.method == "POST" || log.method == "post");
    }

    cout << "✓ Найдено " << postRequests.size() << " POST запросов\n\n";
}

// Тестирование фильтрации по времени
void testTimeFiltering() {
    cout << "Тестирование фильтрации по времени...\n";

    vector<LogEntry> logs = generateTestLogsForAnalyzer(1000);
    LogAnalyzer analyzer(logs);

    // Получаем временной диапазон
    auto timeRange = analyzer.getTimeRange();
    assert(!timeRange.first.empty());
    assert(!timeRange.second.empty());

    cout << "✓ Временной диапазон: " << timeRange.first
        << " - " << timeRange.second << "\n";

    // Фильтруем по произвольному диапазону
    string startTime = "2025-03-14T08:00:00Z";
    string endTime = "2025-03-14T12:00:00Z";

    auto start = high_resolution_clock::now();
    auto filtered = analyzer.filterByTimeRange(startTime, endTime);
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);

    // Проверяем, что все записи входят в диапазон
    for (const auto& log : filtered) {
        assert(log.timestamp >= startTime);
        assert(log.timestamp <= endTime);
    }

    cout << "✓ Найдено " << filtered.size() << " записей в указанном диапазоне\n";
    cout << "✓ Время выполнения: " << duration.count() << " мс\n\n";
}

// Тестирование статистики
void testStatistics() {
    cout << "Тестирование статистики...\n";

    vector<LogEntry> logs = generateTestLogsForAnalyzer(500);
    LogAnalyzer analyzer(logs);

    auto start = high_resolution_clock::now();
    auto stats = analyzer.getDetailedStatistics();
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);

    assert(stats.totalRequests == 500);
    assert(stats.uniqueIPs > 0);
    assert(stats.uniqueURLs > 0);
    assert(!stats.timeRangeStart.empty());
    assert(!stats.timeRangeEnd.empty());

    // Проверяем распределение по статусам
    int totalFromStatuses = 0;
    for (const auto& [status, count] : stats.statusCounts) {
        totalFromStatuses += count;
        assert(status >= 100 && status <= 599);
        assert(count > 0);
    }
    assert(totalFromStatuses == stats.totalRequests);

    // Проверяем распределение по методам
    int totalFromMethods = 0;
    for (const auto& [method, count] : stats.methodCounts) {
        totalFromMethods += count;
        assert(!method.empty());
        assert(count > 0);
    }
    assert(totalFromMethods == stats.totalRequests);

    cout << "✓ Всего запросов: " << stats.totalRequests << "\n";
    cout << "✓ Уникальных IP: " << stats.uniqueIPs << "\n";
    cout << "✓ Уникальных URL: " << stats.uniqueURLs << "\n";
    cout << "✓ Распределений по статусам: " << stats.statusCounts.size() << "\n";
    cout << "✓ Распределений по методам: " << stats.methodCounts.size() << "\n";
    cout << "✓ Время выполнения: " << duration.count() << " мс\n\n";
}

// Тестирование поиска аномалий
void testAnomalyDetection() {
    cout << "Тестирование поиска аномалий...\n";

    vector<LogEntry> logs = generateTestLogsForAnalyzer(1000);
    LogAnalyzer analyzer(logs);

    // Ищем неудачные запросы (статус >= 400)
    auto failedRequests = analyzer.findFailedRequests(400);
    for (const auto& log : failedRequests) {
        assert(log.status >= 400);
    }

    // Ищем подозрительные IP (более 50 запросов)
    auto suspiciousIPs = analyzer.findSuspiciousIPs(50);

    cout << "✓ Найдено " << failedRequests.size() << " неудачных запросов\n";
    cout << "✓ Найдено " << suspiciousIPs.size() << " подозрительных IP\n\n";
}

// Тестирование производительности с большим объемом данных
void testPerformance() {
    cout << "Тестирование производительности...\n";

    // Тестируем с разными объемами данных
    vector<int> testSizes = { 1000, 5000, 10000 };

    for (int size : testSizes) {
        cout << "\nТестирование с " << size << " записями:\n";

        // Генерация данных
        auto genStart = high_resolution_clock::now();
        vector<LogEntry> logs = generateTestLogsForAnalyzer(size);
        auto genEnd = high_resolution_clock::now();
        auto genDuration = duration_cast<milliseconds>(genEnd - genStart);

        cout << "  Генерация: " << genDuration.count() << " мс\n";

        LogAnalyzer analyzer(logs);

        // Тест топ IP
        auto start = high_resolution_clock::now();
        analyzer.getTopIPs(10);
        auto end = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(end - start);
        cout << "  Топ IP: " << duration.count() << " мс\n";

        // Тест топ URL
        start = high_resolution_clock::now();
        analyzer.getTopURLs(10);
        end = high_resolution_clock::now();
        duration = duration_cast<milliseconds>(end - start);
        cout << "  Топ URL: " << duration.count() << " мс\n";

        // Тест фильтрации
        start = high_resolution_clock::now();
        analyzer.filterByStatus(200);
        end = high_resolution_clock::now();
        duration = duration_cast<milliseconds>(end - start);
        cout << "  Фильтрация: " << duration.count() << " мс\n";

        // Тест статистики
        start = high_resolution_clock::now();
        analyzer.getDetailedStatistics();
        end = high_resolution_clock::now();
        duration = duration_cast<milliseconds>(end - start);
        cout << "  Статистика: " << duration.count() << " мс\n";
    }

    cout << "\n";
}

// Тестирование экспорта данных
void testExport() {
    cout << "Тестирование экспорта данных...\n";

    vector<LogEntry> logs = generateTestLogsForAnalyzer(100);
    LogAnalyzer analyzer(logs);

    // Экспорт в CSV
    string csvFile = "test_export.csv";
    bool csvSuccess = analyzer.exportToCSV(csvFile);
    assert(csvSuccess == true);

    // Проверяем существование файла
    ifstream csvCheck(csvFile);
    assert(csvCheck.is_open());
    csvCheck.close();

    // Удаляем тестовый файл
    remove(csvFile.c_str());

    // Экспорт в JSON
    string jsonFile = "test_export.json";
    bool jsonSuccess = analyzer.exportToJson(jsonFile);
    assert(jsonSuccess == true);

    ifstream jsonCheck(jsonFile);
    assert(jsonCheck.is_open());
    jsonCheck.close();

    remove(jsonFile.c_str());

    cout << "✓ CSV экспорт успешен\n";
    cout << "✓ JSON экспорт успешен\n\n";
}

// Тестирование вспомогательных функций
void testUtilityFunctions() {
    cout << "Тестирование вспомогательных функций...\n";

    // Тест extractDomain
    assert(LogAnalyzer::extractDomain("http://example.com/path") == "example.com");
    assert(LogAnalyzer::extractDomain("https://www.google.com/search") == "www.google.com");
    assert(LogAnalyzer::extractDomain("/relative/path") == "/relative/path"); // без протокола

    // Тест extractPath
    assert(LogAnalyzer::extractPath("http://example.com/path/to/page") == "/path/to/page");
    assert(LogAnalyzer::extractPath("https://google.com/search?q=test") == "/search");
    assert(LogAnalyzer::extractPath("/api/v1/users") == "/api/v1/users");

    // Тест isInTimeRange
    assert(LogAnalyzer::isInTimeRange("2025-03-14T10:00:00Z",
        "2025-03-14T09:00:00Z",
        "2025-03-14T11:00:00Z") == true);
    assert(LogAnalyzer::isInTimeRange("2025-03-14T08:00:00Z",
        "2025-03-14T09:00:00Z",
        "2025-03-14T11:00:00Z") == false);
    assert(LogAnalyzer::isInTimeRange("2025-03-14T12:00:00Z",
        "2025-03-14T09:00:00Z",
        "") == true); // без верхней границы

    cout << "✓ Все вспомогательные функции работают корректно\n\n";
}

// Главная функция тестирования
int main() {
    cout << "========================================\n";
    cout << "   ТЕСТИРОВАНИЕ LogAnalyzer (Windows)   \n";
    cout << "========================================\n\n";

    try {
        cout << "Запуск тестов...\n\n";

        testDataLoading();
        testTopIPs();
        testTopURLs();
        testStatusFiltering();
        testMethodFiltering();
        testTimeFiltering();
        testStatistics();
        testAnomalyDetection();
        testPerformance();
        testExport();
        testUtilityFunctions();

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