#include "analyzer.h"
#include <algorithm>
#include <numeric>
#include <cmath>
#include <unordered_set>
#include <windows.h>
#include "json_parser.h"

using namespace std;

// Конструктор с загрузкой логов
LogAnalyzer::LogAnalyzer(const vector<LogEntry>& logEntries) : logs(logEntries) {
    indexesBuilt = false;
}

// Загрузка из JsonValue
bool LogAnalyzer::loadFromJson(const JsonValue& json) {
    try {
        logs = json.asLogEntries();
        indexesBuilt = false;
        return true;
    }
    catch (const exception& e) {
        return false;
    }
}

// Загрузка из файла
bool LogAnalyzer::loadFromFile(const string& filename) {
    try {
        JsonValue json = JsonParser::loadFromFile(filename);
        return loadFromJson(json);
    }
    catch (const exception&) {
        return false;
    }
}

// Получение топ IP-адресов
vector<pair<string, int>> LogAnalyzer::getTopIPs(int n) {
    ensureIndexesBuilt();

    FastHashMap ipCounts;
    ipCounts.reserve(logs.size() / 10);

    for (const auto& log : logs) {
        ipCounts[log.ip]++;
    }

    vector<pair<string, int>> sortedIPs(ipCounts.begin(), ipCounts.end());

    sort(sortedIPs.begin(), sortedIPs.end(),
        [](const pair<string, int>& a, const pair<string, int>& b) {
            return a.second > b.second;
        });

    if (n > 0 && static_cast<size_t>(n) < sortedIPs.size()) {
        sortedIPs.resize(n);
    }

    return sortedIPs;
}

// Получение топ URL
vector<pair<string, int>> LogAnalyzer::getTopURLs(int n) {
    ensureIndexesBuilt();

    FastHashMap urlCounts;
    urlCounts.reserve(logs.size() / 20);

    for (const auto& log : logs) {
        urlCounts[log.url]++;
    }

    vector<pair<string, int>> sortedURLs(urlCounts.begin(), urlCounts.end());

    sort(sortedURLs.begin(), sortedURLs.end(),
        [](const pair<string, int>& a, const pair<string, int>& b) {
            return a.second > b.second;
        });

    if (n > 0 && static_cast<size_t>(n) < sortedURLs.size()) {
        sortedURLs.resize(n);
    }

    return sortedURLs;
}

// Фильтрация по статусу
vector<LogEntry> LogAnalyzer::filterByStatus(int status) const {
    vector<LogEntry> result;
    result.reserve(logs.size() / 10); // Ожидаем примерно 10% записей

    copy_if(logs.begin(), logs.end(), back_inserter(result),
        [status](const LogEntry& log) {
            return log.status == status;
        });

    result.shrink_to_fit();
    return result;
}

// Фильтрация по методу
vector<LogEntry> LogAnalyzer::filterByMethod(const string& method) const {
    vector<LogEntry> result;
    result.reserve(logs.size() / 5); // Ожидаем примерно 20% записей

    string upperMethod = method;
    transform(upperMethod.begin(), upperMethod.end(), upperMethod.begin(), ::toupper);

    copy_if(logs.begin(), logs.end(), back_inserter(result),
        [&upperMethod](const LogEntry& log) {
            string logMethod = log.method;
            transform(logMethod.begin(), logMethod.end(), logMethod.begin(), ::toupper);
            return logMethod == upperMethod;
        });

    result.shrink_to_fit();
    return result;
}

// Фильтрация по временному диапазону
vector<LogEntry> LogAnalyzer::filterByTimeRange(const string& startTime,
    const string& endTime) const {
    vector<LogEntry> result;
    result.reserve(logs.size());

    copy_if(logs.begin(), logs.end(), back_inserter(result),
        [&startTime, &endTime](const LogEntry& log) {
            return isInTimeRange(log.timestamp, startTime, endTime);
        });

    result.shrink_to_fit();
    return result;
}

// Фильтрация по IP
vector<LogEntry> LogAnalyzer::filterByIP(const string& ip) const {
    vector<LogEntry> result;

    copy_if(logs.begin(), logs.end(), back_inserter(result),
        [&ip](const LogEntry& log) {
            return log.ip == ip;
        });

    return result;
}

// Фильтрация по шаблону URL
vector<LogEntry> LogAnalyzer::filterByURL(const string& urlPattern) const {
    vector<LogEntry> result;

    copy_if(logs.begin(), logs.end(), back_inserter(result),
        [&urlPattern](const LogEntry& log) {
            return log.url.find(urlPattern) != string::npos;
        });

    return result;
}

// Фильтрация с пользовательским предикатом
vector<LogEntry> LogAnalyzer::filter(const function<bool(const LogEntry&)>& predicate) const {
    vector<LogEntry> result;

    copy_if(logs.begin(), logs.end(), back_inserter(result), predicate);

    return result;
}

// Получение временного диапазона
pair<string, string> LogAnalyzer::getTimeRange() const {
    if (logs.empty()) {
        return { "", "" };
    }

    string minTime = logs[0].timestamp;
    string maxTime = logs[0].timestamp;

    for (const auto& log : logs) {
        if (log.timestamp < minTime) {
            minTime = log.timestamp;
        }
        if (log.timestamp > maxTime) {
            maxTime = log.timestamp;
        }
    }

    return { minTime, maxTime };
}

// Распределение по статусам
map<int, int> LogAnalyzer::getStatusDistribution() const {
    map<int, int> distribution;

    for (const auto& log : logs) {
        distribution[log.status]++;
    }

    return distribution;
}

// Распределение по методам
map<string, int> LogAnalyzer::getMethodDistribution() const {
    map<string, int> distribution;

    for (const auto& log : logs) {
        distribution[log.method]++;
    }

    return distribution;
}

// Детальная статистика
LogAnalyzer::Statistics LogAnalyzer::getDetailedStatistics() const {
    Statistics stats;
    stats.totalRequests = getTotalRequests();

    // Уникальные IP и URL
    unordered_set<string> uniqueIPs, uniqueURLs;
    for (const auto& log : logs) {
        uniqueIPs.insert(log.ip);
        uniqueURLs.insert(log.url);
    }
    stats.uniqueIPs = static_cast<int>(uniqueIPs.size());
    stats.uniqueURLs = static_cast<int>(uniqueURLs.size());

    // Временной диапазон
    auto timeRange = getTimeRange();
    stats.timeRangeStart = timeRange.first;
    stats.timeRangeEnd = timeRange.second;

    // Распределение по статусам
    stats.statusCounts = getStatusDistribution();

    // Распределение по методам
    stats.methodCounts = getMethodDistribution();

    // Расчет средней нагрузки (запросов в секунду)
    if (!logs.empty() && timeRange.first != timeRange.second) {
        try {
            // Простой расчет: предполагаем равномерное распределение
            // В реальности нужно парсить timestamp и вычислять разницу
            stats.requestsPerSecond = static_cast<double>(stats.totalRequests) / 3600.0; // пример: за час
        }
        catch (...) {
            stats.requestsPerSecond = 0.0;
        }
    }
    else {
        stats.requestsPerSecond = 0.0;
    }

    return stats;
}

// Поиск неудачных запросов
vector<LogEntry> LogAnalyzer::findFailedRequests(int threshold) const {
    vector<LogEntry> result;

    copy_if(logs.begin(), logs.end(), back_inserter(result),
        [threshold](const LogEntry& log) {
            return log.status >= threshold;
        });

    return result;
}

// Поиск подозрительных IP
vector<string> LogAnalyzer::findSuspiciousIPs(int threshold) const {
    FastHashMap ipCounts;

    for (const auto& log : logs) {
        ipCounts[log.ip]++;
    }

    vector<string> suspiciousIPs;

    for (const auto& [ip, count] : ipCounts) {
        if (count > threshold) {
            suspiciousIPs.push_back(ip);
        }
    }

    return suspiciousIPs;
}

// Экспорт в CSV
bool LogAnalyzer::exportToCSV(const string& filename) const {
    ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }

    // Заголовок
    file << "timestamp,ip,method,url,status\n";

    // Данные
    for (const auto& log : logs) {
        file << log.timestamp << ","
            << log.ip << ","
            << log.method << ","
            << "\"" << log.url << "\","
            << log.status << "\n";
    }

    file.close();
    return true;
}

// Экспорт в JSON
bool LogAnalyzer::exportToJson(const string& filename) const {
    try {
        JsonValue jsonArray(JsonType::Array);

        for (const auto& log : logs) {
            map<string, JsonValue> obj;
            obj["ts"] = JsonValue(log.timestamp);
            obj["ip"] = JsonValue(log.ip);
            obj["method"] = JsonValue(log.method);
            obj["url"] = JsonValue(log.url);
            obj["status"] = JsonValue(static_cast<double>(log.status));

            jsonArray.arrayValue.push_back(JsonValue(obj));
        }

        JsonParser::saveToFile(filename, jsonArray, true);
        return true;
    }
    catch (const exception&) {
        return false;
    }
}

// Вспомогательная функция для проверки временного диапазона
bool LogAnalyzer::isInTimeRange(const string& timestamp,
    const string& start,
    const string& end) {
    if (start.empty() && end.empty()) return true;
    if (start.empty()) return timestamp <= end;
    if (end.empty()) return timestamp >= start;
    return timestamp >= start && timestamp <= end;
}

// Извлечение домена из URL
string LogAnalyzer::extractDomain(const string& url) {
    // Простая реализация
    size_t start = url.find("://");
    if (start != string::npos) {
        start += 3;
    }
    else {
        start = 0;
    }

    size_t end = url.find('/', start);
    if (end == string::npos) {
        end = url.length();
    }

    return url.substr(start, end - start);
}

// Извлечение пути из URL
string LogAnalyzer::extractPath(const string& url) {
    size_t start = url.find("://");
    if (start != string::npos) {
        start += 3;
    }
    else {
        start = 0;
    }

    size_t slashPos = url.find('/', start);
    if (slashPos == string::npos) {
        return "/";
    }

    size_t queryPos = url.find('?', slashPos);
    if (queryPos == string::npos) {
        return url.substr(slashPos);
    }

    return url.substr(slashPos, queryPos - slashPos);
}

// Построение индексов для оптимизации
void LogAnalyzer::ensureIndexesBuilt() const {
    if (indexesBuilt) return;

    ipIndex.clear();
    urlIndex.clear();
    timeIndex.clear();

    for (size_t i = 0; i < logs.size(); i++) {
        ipIndex[logs[i].ip].push_back(i);
        urlIndex[logs[i].url].push_back(i);
        timeIndex[logs[i].timestamp].push_back(i);
    }

    indexesBuilt = true;
}

// Утилиты для работы со временем
namespace TimeUtils {

    tm parseISOTimestamp(const string& timestamp) {
        tm time = {};

        // Парсинг формата: YYYY-MM-DDTHH:MM:SSZ
        sscanf_s(timestamp.c_str(), "%d-%d-%dT%d:%d:%dZ",
            &time.tm_year, &time.tm_mon, &time.tm_mday,
            &time.tm_hour, &time.tm_min, &time.tm_sec);

        time.tm_year -= 1900;
        time.tm_mon -= 1;

        return time;
    }

    string formatISOTimestamp(const tm& time) {
        char buffer[20];
        snprintf(buffer, sizeof(buffer), "%04d-%02d-%02dT%02d:%02d:%02dZ",
            time.tm_year + 1900, time.tm_mon + 1, time.tm_mday,
            time.tm_hour, time.tm_min, time.tm_sec);
        return string(buffer);
    }

    bool isEarlier(const string& t1, const string& t2) {
        return t1 < t2;
    }

    bool isLater(const string& t1, const string& t2) {
        return t1 > t2;
    }

    string getCurrentTimeISO() {
        time_t now = time(nullptr);
        tm gmtm;
        gmtime_s(&gmtm, &now);

        char buffer[20];
        strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", &gmtm);
        return string(buffer);
    }
};