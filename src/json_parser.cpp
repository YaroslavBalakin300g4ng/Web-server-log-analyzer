#include "json_parser.h"
#include <fstream>
#include <sstream>
#include <cctype>
#include <stdexcept>
#include <windows.h>

using namespace std;

// Вспомогательная функция для пропуска пробелов
void JsonParser::skipWhitespace(const string& jsonStr, size_t& pos) {
    while (pos < jsonStr.size() && isspace(static_cast<unsigned char>(jsonStr[pos]))) {
        pos++;
    }
}

// Парсинг строки JSON
JsonValue JsonParser::parse(const string& jsonStr) {
    size_t pos = 0;
    JsonValue result = parseValue(jsonStr, pos);
    skipWhitespace(jsonStr, pos);

    if (pos != jsonStr.size()) {
        throw JsonParseException("Лишние символы после JSON", pos);
    }

    return result;
}

// Парсинг значения
JsonValue JsonParser::parseValue(const string& jsonStr, size_t& pos) {
    skipWhitespace(jsonStr, pos);

    if (pos >= jsonStr.size()) {
        throw JsonParseException("Неожиданный конец JSON", pos);
    }

    char c = jsonStr[pos];

    if (c == '{') {
        return parseObject(jsonStr, pos);
    }
    else if (c == '[') {
        return parseArray(jsonStr, pos);
    }
    else if (c == '"') {
        return parseString(jsonStr, pos);
    }
    else if (c == '-' || isdigit(c)) {
        return parseNumber(jsonStr, pos);
    }
    else if (c == 't' || c == 'f' || c == 'n') {
        return parseKeyword(jsonStr, pos);
    }
    else {
        throw JsonParseException("Неожиданный символ", pos, string(1, c));
    }
}

// Парсинг объекта
JsonValue JsonParser::parseObject(const string& jsonStr, size_t& pos) {
    pos++; // пропускаем '{'
    skipWhitespace(jsonStr, pos);

    map<string, JsonValue> obj;

    if (jsonStr[pos] == '}') {
        pos++; // пустой объект
        return JsonValue(obj);
    }

    while (pos < jsonStr.size()) {
        // Парсинг ключа
        skipWhitespace(jsonStr, pos);
        if (jsonStr[pos] != '"') {
            throw JsonParseException("Ожидалась строка (ключ)", pos);
        }

        JsonValue keyValue = parseString(jsonStr, pos);
        string key = keyValue.asString();

        // Пропускаем ':'
        skipWhitespace(jsonStr, pos);
        if (jsonStr[pos] != ':') {
            throw JsonParseException("Ожидалось ':' после ключа", pos);
        }
        pos++;

        // Парсинг значения
        JsonValue value = parseValue(jsonStr, pos);
        obj[key] = value;

        // Проверяем следующий символ
        skipWhitespace(jsonStr, pos);
        if (jsonStr[pos] == '}') {
            pos++;
            break;
        }
        else if (jsonStr[pos] == ',') {
            pos++;
            continue;
        }
        else {
            throw JsonParseException("Ожидалось ',' или '}'", pos);
        }
    }

    return JsonValue(obj);
}

// Парсинг массива
JsonValue JsonParser::parseArray(const string& jsonStr, size_t& pos) {
    pos++; // пропускаем '['
    skipWhitespace(jsonStr, pos);

    vector<JsonValue> arr;

    if (jsonStr[pos] == ']') {
        pos++; // пустой массив
        return JsonValue(arr);
    }

    while (pos < jsonStr.size()) {
        // Парсинг элемента
        JsonValue element = parseValue(jsonStr, pos);
        arr.push_back(element);

        // Проверяем следующий символ
        skipWhitespace(jsonStr, pos);
        if (jsonStr[pos] == ']') {
            pos++;
            break;
        }
        else if (jsonStr[pos] == ',') {
            pos++;
            continue;
        }
        else {
            throw JsonParseException("Ожидалось ',' или ']'", pos);
        }
    }

    return JsonValue(arr);
}

// Парсинг строки с обработкой экранированных символов
JsonValue JsonParser::parseString(const string& jsonStr, size_t& pos) {
    pos++; // пропускаем открывающую кавычку

    string str;
    while (pos < jsonStr.size() && jsonStr[pos] != '"') {
        if (jsonStr[pos] == '\\') {
            pos++;
            if (pos >= jsonStr.size()) {
                throw JsonParseException("Незавершенная escape-последовательность", pos);
            }
            str += parseEscapeSequence(jsonStr, pos);
        }
        else {
            str += jsonStr[pos];
            pos++;
        }
    }

    if (pos >= jsonStr.size() || jsonStr[pos] != '"') {
        throw JsonParseException("Незавершенная строка", pos);
    }
    pos++; // пропускаем закрывающую кавычку

    return JsonValue(str);
}

// Обработка escape-последовательностей
char JsonParser::parseEscapeSequence(const string& jsonStr, size_t& pos) {
    char c = jsonStr[pos];
    pos++;

    switch (c) {
    case '"': return '"';
    case '\\': return '\\';
    case '/': return '/';
    case 'b': return '\b';
    case 'f': return '\f';
    case 'n': return '\n';
    case 'r': return '\r';
    case 't': return '\t';
    case 'u': {
        // Простая обработка \uXXXX (без полной поддержки Unicode)
        if (pos + 3 >= jsonStr.size()) {
            throw JsonParseException("Неполная Unicode последовательность", pos);
        }
        // Пропускаем Unicode для простоты
        pos += 4;
        return '?';
    }
    default:
        throw JsonParseException("Неизвестная escape-последовательность", pos - 1, string(1, c));
    }
}

// Парсинг числа
JsonValue JsonParser::parseNumber(const string& jsonStr, size_t& pos) {
    size_t start = pos;

    // Обрабатываем знак
    if (jsonStr[pos] == '-') {
        pos++;
    }

    // Целая часть
    while (pos < jsonStr.size() && isdigit(jsonStr[pos])) {
        pos++;
    }

    // Дробная часть
    if (pos < jsonStr.size() && jsonStr[pos] == '.') {
        pos++;
        while (pos < jsonStr.size() && isdigit(jsonStr[pos])) {
            pos++;
        }
    }

    // Экспоненциальная часть
    if (pos < jsonStr.size() && (jsonStr[pos] == 'e' || jsonStr[pos] == 'E')) {
        pos++;
        if (pos < jsonStr.size() && (jsonStr[pos] == '+' || jsonStr[pos] == '-')) {
            pos++;
        }
        while (pos < jsonStr.size() && isdigit(jsonStr[pos])) {
            pos++;
        }
    }

    string numStr = jsonStr.substr(start, pos - start);
    try {
        double num = stod(numStr);
        return JsonValue(num);
    }
    catch (const exception&) {
        throw JsonParseException("Некорректное число", start, numStr);
    }
}

// Парсинг ключевых слов (true, false, null)
JsonValue JsonParser::parseKeyword(const string& jsonStr, size_t& pos) {
    if (jsonStr.compare(pos, 4, "true") == 0) {
        pos += 4;
        return JsonValue(true);
    }
    else if (jsonStr.compare(pos, 5, "false") == 0) {
        pos += 5;
        return JsonValue(false);
    }
    else if (jsonStr.compare(pos, 4, "null") == 0) {
        pos += 4;
        return JsonValue();
    }
    else {
        throw JsonParseException("Неизвестное ключевое слово", pos);
    }
}

// Загрузка JSON из файла с обработкой BOM для Windows
JsonValue JsonParser::loadFromFile(const string& filename) {
    ifstream file(filename, ios::binary);
    if (!file.is_open()) {
        throw JsonFileException("Не удалось открыть файл: " + filename);
    }

    // Определяем размер файла
    file.seekg(0, ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, ios::beg);

    // Читаем файл
    vector<char> buffer(fileSize + 1);
    file.read(buffer.data(), fileSize);
    buffer[fileSize] = '\0';

    // Проверяем BOM для UTF-8
    string content;
    if (fileSize >= 3 &&
        static_cast<unsigned char>(buffer[0]) == 0xEF &&
        static_cast<unsigned char>(buffer[1]) == 0xBB &&
        static_cast<unsigned char>(buffer[2]) == 0xBF) {
        // Пропускаем BOM
        content = string(buffer.data() + 3, fileSize - 3);
    }
    else {
        content = string(buffer.data(), fileSize);
    }

    file.close();
    return parse(content);
}

// Сохранение JSON в файл
void JsonParser::saveToFile(const string& filename, const JsonValue& value, bool pretty) {
    ofstream file(filename);
    if (!file.is_open()) {
        throw JsonFileException("Не удалось создать файл: " + filename);
    }

    string jsonStr = toString(value, pretty);
    file << jsonStr;
    file.close();
}

// Преобразование в строку
string JsonParser::toString(const JsonValue& value, bool pretty) {
    ostringstream oss;
    int indentLevel = 0;

    function<void(const JsonValue&, int)> writeValue;

    writeValue = [&](const JsonValue& val, int indent) {
        if (val.isNull()) {
            oss << "null";
        }
        else if (val.isBoolean()) {
            oss << (val.asBoolean() ? "true" : "false");
        }
        else if (val.isNumber()) {
            oss << val.asNumber();
        }
        else if (val.isString()) {
            oss << '"' << val.asString() << '"';
        }
        else if (val.isArray()) {
            oss << '[';
            if (pretty && !val.arrayValue.empty()) oss << '\n';

            for (size_t i = 0; i < val.arrayValue.size(); i++) {
                if (pretty) {
                    oss << string(indent + 2, ' ');
                }
                writeValue(val.arrayValue[i], indent + 2);
                if (i != val.arrayValue.size() - 1) {
                    oss << ',';
                }
                if (pretty) oss << '\n';
            }

            if (pretty && !val.arrayValue.empty()) {
                oss << string(indent, ' ');
            }
            oss << ']';
        }
        else if (val.isObject()) {
            oss << '{';
            if (pretty && !val.objectValue.empty()) oss << '\n';

            size_t i = 0;
            for (const auto& [key, val2] : val.objectValue) {
                if (pretty) {
                    oss << string(indent + 2, ' ');
                }
                oss << '"' << key << "\":";
                if (pretty) oss << ' ';
                writeValue(val2, indent + 2);
                if (++i != val.objectValue.size()) {
                    oss << ',';
                }
                if (pretty) oss << '\n';
            }

            if (pretty && !val.objectValue.empty()) {
                oss << string(indent, ' ');
            }
            oss << '}';
        }
        };

    writeValue(value, 0);
    return oss.str();
}

// Валидация JSON строки
bool JsonParser::isValid(const string& jsonStr, string& errorMsg) {
    try {
        parse(jsonStr);
        errorMsg = "";
        return true;
    }
    catch (const JsonParseException& e) {
        errorMsg = e.what();
        return false;
    }
}

// Методы JsonValue
string JsonValue::asString() const {
    if (type != JsonType::String) {
        throw runtime_error("Не строковый тип");
    }
    return stringValue;
}

double JsonValue::asNumber() const {
    if (type != JsonType::Number) {
        throw runtime_error("Не числовой тип");
    }
    return numberValue;
}

bool JsonValue::asBoolean() const {
    if (type != JsonType::Boolean) {
        throw runtime_error("Не логический тип");
    }
    return boolValue;
}

vector<LogEntry> JsonValue::asLogEntries() const {
    vector<LogEntry> entries;

    if (!isArray()) {
        throw runtime_error("Не массив");
    }

    for (const auto& val : arrayValue) {
        if (!val.isObject()) {
            continue;
        }

        try {
            string timestamp = val.objectValue.at("ts").asString();
            string ip = val.objectValue.at("ip").asString();
            string method = val.objectValue.at("method").asString();
            string url = val.objectValue.at("url").asString();
            int status = static_cast<int>(val.objectValue.at("status").asNumber());

            entries.push_back(LogEntry(timestamp, ip, method, url, status));
        }
        catch (const exception&) {
            // Пропускаем некорректные записи
        }
    }

    return entries;
}

JsonValue& JsonValue::operator[](const string& key) {
    if (type != JsonType::Object) {
        throw runtime_error("Не объектный тип");
    }
    return objectValue[key];
}

const JsonValue& JsonValue::operator[](const string& key) const {
    if (type != JsonType::Object) {
        throw runtime_error("Не объектный тип");
    }
    auto it = objectValue.find(key);
    if (it == objectValue.end()) {
        throw runtime_error("Ключ не найден: " + key);
    }
    return it->second;
}

JsonValue& JsonValue::operator[](size_t index) {
    if (type != JsonType::Array) {
        throw runtime_error("Не массив");
    }
    if (index >= arrayValue.size()) {
        throw runtime_error("Индекс вне диапазона");
    }
    return arrayValue[index];
}

const JsonValue& JsonValue::operator[](size_t index) const {
    if (type != JsonType::Array) {
        throw runtime_error("Не массив");
    }
    if (index >= arrayValue.size()) {
        throw runtime_error("Индекс вне диапазона");
    }
    return arrayValue[index];
}

size_t JsonValue::size() const {
    if (type == JsonType::Array) {
        return arrayValue.size();
    }
    else if (type == JsonType::Object) {
        return objectValue.size();
    }
    return 0;
}

bool JsonValue::empty() const {
    return size() == 0;
}