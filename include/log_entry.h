#ifndef LOG_ENTRY_H
#define LOG_ENTRY_H

#include <string>
#include <stdexcept>
#include <ctime>

// Структура для хранения записи лога веб-сервера

struct LogEntry {
    std::string timestamp;  // ISO 8601 формат: YYYY-MM-DDTHH:MM:SSZ
    std::string ip;         // IPv4 адрес
    std::string method;     // HTTP метод: GET, POST, PUT, DELETE, etc.
    std::string url;        // URL запроса
    int status;             // HTTP статус код (100-599)

    // Конструкторы
    LogEntry() : status(0) {}
    LogEntry(const std::string& ts, const std::string& ip,
        const std::string& m, const std::string& u, int s)
        : timestamp(ts), ip(ip), method(m), url(u), status(s) {
    }

    // Валидация записи
    bool isValid() const;

    // Windows-специфичные методы для работы с кодировками
    static std::wstring toWideString(const std::string& str);
    static std::string fromWideString(const std::wstring& wstr);

    // Статические методы валидации
    static bool validateTimestamp(const std::string& ts);
    static bool validateIP(const std::string& ip);
    static bool validateMethod(const std::string& method);
    static bool validateStatus(int status);

    // Парсинг из строки JSON
    static LogEntry fromJsonString(const std::string& jsonStr);

    // Преобразование в строку для отладки
    std::string toString() const;
};

// Исключения для валидации логов
class LogValidationException : public std::runtime_error {
public:
    explicit LogValidationException(const std::string& msg)
        : std::runtime_error(msg) {}
};

class TimestampFormatException : public LogValidationException {
public:
    explicit TimestampFormatException(const std::string& ts)
        : LogValidationException("Неверный формат времени: " + ts) {
    }
};

class IPFormatException : public LogValidationException {
public:
    explicit IPFormatException(const std::string& ip)
        : LogValidationException("Некорректный IP-адрес: " + ip) {
    }
};

class StatusRangeException : public LogValidationException {
public:
    explicit StatusRangeException(int status)
        : LogValidationException("Недопустимый статус код: " + std::to_string(status)) {
    }
};

class MethodException : public LogValidationException {
public:
    explicit MethodException(const std::string& method)
        : LogValidationException("Неизвестный HTTP метод: " + method) {
    }
};

#endif // LOG_ENTRY_H