#ifndef JSON_PARSER_H
#define JSON_PARSER_H

#include <string>
#include <vector>
#include <map>
#include <stdexcept>
#include "log_entry.h"

// JSON-парсер для работы с логами веб-сервера

enum class JsonType {
    Null,
    Boolean,
    Number,
    String,
    Array,
    Object
};

struct JsonValue {
    JsonType type;

    bool boolValue;
    double numberValue;
    std::string stringValue;
    std::vector<JsonValue> arrayValue;
    std::map<std::string, JsonValue> objectValue;

    // Конструкторы
    JsonValue() : type(JsonType::Null), boolValue(false), numberValue(0.0) {}
    JsonValue(bool b) : type(JsonType::Boolean), boolValue(b), numberValue(0.0) {}
    JsonValue(double n) : type(JsonType::Number), boolValue(false), numberValue(n) {}
    JsonValue(const std::string& s)
        : type(JsonType::String), boolValue(false), numberValue(0.0), stringValue(s) {
    }
    JsonValue(const std::vector<JsonValue>& arr)
        : type(JsonType::Array), boolValue(false), numberValue(0.0), arrayValue(arr) {
    }
    JsonValue(const std::map<std::string, JsonValue>& obj)
        : type(JsonType::Object), boolValue(false), numberValue(0.0), objectValue(obj) {
    }

    // Методы преобразования
    std::string asString() const;
    double asNumber() const;
    bool asBoolean() const;
    std::vector<LogEntry> asLogEntries() const;

    // Проверки типов
    bool isNull() const { return type == JsonType::Null; }
    bool isBoolean() const { return type == JsonType::Boolean; }
    bool isNumber() const { return type == JsonType::Number; }
    bool isString() const { return type == JsonType::String; }
    bool isArray() const { return type == JsonType::Array; }
    bool isObject() const { return type == JsonType::Object; }

    // Доступ к элементам
    JsonValue& operator[](const std::string& key);
    const JsonValue& operator[](const std::string& key) const;
    JsonValue& operator[](size_t index);
    const JsonValue& operator[](size_t index) const;

    // Размеры
    size_t size() const;
    bool empty() const;
};

// Класс для парсинга JSON с поддержкой Windows кодировок
class JsonParser {
public:
    // Основной парсинг
    static JsonValue parse(const std::string& jsonStr);

    // Загрузка из файла с обработкой BOM для Windows
    static JsonValue loadFromFile(const std::string& filename);

    // Сохранение в файл
    static void saveToFile(const std::string& filename, const JsonValue& value, bool pretty = true);

    // Преобразование в строку
    static std::string toString(const JsonValue& value, bool pretty = true);

    // Валидация JSON строки
    static bool isValid(const std::string& jsonStr, std::string& errorMsg);

private:
    // Вспомогательные методы парсинга
    static JsonValue parseValue(const std::string& jsonStr, size_t& pos);
    static JsonValue parseObject(const std::string& jsonStr, size_t& pos);
    static JsonValue parseArray(const std::string& jsonStr, size_t& pos);
    static JsonValue parseString(const std::string& jsonStr, size_t& pos);
    static JsonValue parseNumber(const std::string& jsonStr, size_t& pos);
    static JsonValue parseKeyword(const std::string& jsonStr, size_t& pos);

    // Пропуск пробелов
    static void skipWhitespace(const std::string& jsonStr, size_t& pos);

    // Обработка экранированных символов (Windows-совместимая)
    static char parseEscapeSequence(const std::string& jsonStr, size_t& pos);
};

// Исключения парсера
class JsonParseException : public std::runtime_error {
private:
    size_t position;
    std::string context;

public:
    JsonParseException(const std::string& msg, size_t pos, const std::string& ctx = "")
        : std::runtime_error(msg + " (позиция: " + std::to_string(pos) + ")")
        , position(pos), context(ctx) {
    }

    size_t getPosition() const { return position; }
    std::string getContext() const { return context; }
};

class JsonFileException : public std::runtime_error {
public:
    explicit JsonFileException(const std::string& filename)
        : std::runtime_error("Ошибка работы с файлом: " + filename) {}
};

#endif // JSON_PARSER_H