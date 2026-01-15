#ifndef ANALYZER_H
#define ANALYZER_H

#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <functional>
#include "log_entry.h"

// Класс для анализа логов веб-сервера

class LogAnalyzer {
private:
    std::vector<LogEntry> logs;

    // Оптимизированная хэш-функция для Windows
    struct WindowsStringHash {
        size_t operator()(const std::string& s) const {
            // FNV-1a hash - эффективная для Windows
            size_t hash = 2166136261U;
            for (char c : s) {
                hash ^= static_cast<unsigned char>(c);
                hash *= 16777619U;
            }
            return hash;
        }
    };

    // Использование оптимизированной хэш-таблицы
    using FastHashMap = std::unordered_map<std::string, int, WindowsStringHash>;

public:
    // Конструкторы
    LogAnalyzer() = default;
    explicit LogAnalyzer(const std::vector<LogEntry>& logEntries);

    // Загрузка данных
    bool loadFromJson(const JsonValue& json);
    bool loadFromFile(const std::string& filename);

    // Основные операции анализа
    std::vector<std::pair<std::string, int>> getTopIPs(int n = 10);
    std::vector<std::pair<std::string, int>> getTopURLs(int n = 10);

    // Фильтрация
    std::vector<LogEntry> filterByStatus(int status) const;
    std::vector<LogEntry> filterByMethod(const std::string& method) const;
    std::vector<LogEntry> filterByTimeRange(const std::string& startTime,
        const std::string& endTime) const;
    std::vector<LogEntry> filterByIP(const std::string& ip) const;
    std::vector<LogEntry> filterByURL(const std::string& urlPattern) const;

    // Комбинированные фильтры
    std::vector<LogEntry> filter(const std::function<bool(const LogEntry&)>& predicate) const;

    // Статистика
    int getTotalRequests() const { return static_cast<int>(logs.size()); }
    std::pair<std::string, std::string> getTimeRange() const;

    std::map<int, int> getStatusDistribution() const;
    std::map<std::string, int> getMethodDistribution() const;

    // Детальная статистика
    struct Statistics {
        int totalRequests;
        int uniqueIPs;
        int uniqueURLs;
        std::string timeRangeStart;
        std::string timeRangeEnd;
        std::map<int, int> statusCounts;
        std::map<std::string, int> methodCounts;
        double requestsPerSecond; // среднее количество запросов в секунду
    };

    Statistics getDetailedStatistics() const;

    // Анализ аномалий
    std::vector<LogEntry> findFailedRequests(int threshold = 400) const; // статус >= threshold
    std::vector<std::string> findSuspiciousIPs(int threshold = 100) const; // IP с > threshold запросов
    std::vector<LogEntry> findSlowPeriods(int windowSeconds = 60,
        int threshold = 1000) const; // периоды высокой нагрузки

    // Экспорт результатов
    bool exportToCSV(const std::string& filename) const;
    bool exportToJson(const std::string& filename) const;

    // Windows-специфичные методы
    bool exportToExcel(const std::string& filename) const; // через CSV
    bool openInDefaultViewer(const std::string& filename) const;

    // Доступ к данным
    const std::vector<LogEntry>& getLogs() const { return logs; }
    void clear() { logs.clear(); }
    void addLog(const LogEntry& entry) { logs.push_back(entry); }

    // Вспомогательные методы
    static bool isInTimeRange(const std::string& timestamp,
        const std::string& start,
        const std::string& end);
    static std::string extractDomain(const std::string& url);
    static std::string extractPath(const std::string& url);

private:
    // Внутренние методы для оптимизации
    void buildIPIndex() const;
    void buildURLIndex() const;
    void buildTimeIndex() const;

    // Кэши для производительности
    mutable std::map<std::string, std::vector<size_t>> ipIndex;
    mutable std::map<std::string, std::vector<size_t>> urlIndex;
    mutable std::map<std::string, std::vector<size_t>> timeIndex;
    mutable bool indexesBuilt = false;

    void ensureIndexesBuilt() const;
};

// Утилиты для работы со временем (Windows-совместимые)
namespace TimeUtils {
    // Парсинг ISO 8601 timestamp
    std::tm parseISOTimestamp(const std::string& timestamp);

    // Форматирование времени в ISO 8601
    std::string formatISOTimestamp(const std::tm& time);

    // Сравнение timestamp
    bool isEarlier(const std::string& t1, const std::string& t2);
    bool isLater(const std::string& t1, const std::string& t2);

    // Разница в секундах
    int secondsDifference(const std::string& t1, const std::string& t2);

    // Windows-специфичные функции
    std::string getCurrentTimeISO();
    std::string convertToLocalTime(const std::string& isoTime);
};

#endif // ANALYZER_H