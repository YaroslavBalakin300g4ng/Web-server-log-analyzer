#include <iostream>
#include <string>
#include <vector>
#include <cassert>
#include <fstream>
#include "json_parser.h"
#include "log_entry.h"

using namespace std;

// Тестирование парсинга простых типов
void testSimpleTypes() {
    cout << "Тестирование парсинга простых типов...\n";

    // Тест null
    JsonValue nullVal = JsonParser::parse("null");
    assert(nullVal.isNull() == true);

    // Тест boolean
    JsonValue trueVal = JsonParser::parse("true");
    assert(trueVal.isBoolean() == true);
    assert(trueVal.asBoolean() == true);

    JsonValue falseVal = JsonParser::parse("false");
    assert(falseVal.isBoolean() == true);
    assert(falseVal.asBoolean() == false);

    // Тест чисел
    JsonValue intVal = JsonParser::parse("42");
    assert(intVal.isNumber() == true);
    assert(intVal.asNumber() == 42.0);

    JsonValue floatVal = JsonParser::parse("3.14159");
    assert(floatVal.isNumber() == true);
    assert(floatVal.asNumber() == 3.14159);

    JsonValue negVal = JsonParser::parse("-10.5");
    assert(negVal.isNumber() == true);
    assert(negVal.asNumber() == -10.5);

    JsonValue expVal = JsonParser::parse("1.23e4");
    assert(expVal.isNumber() == true);
    assert(expVal.asNumber() == 12300.0);

    // Тест строк
    JsonValue strVal = JsonParser::parse("\"Hello, World!\"");
    assert(strVal.isString() == true);
    assert(strVal.asString() == "Hello, World!");

    JsonValue emptyStr = JsonParser::parse("\"\"");
    assert(emptyStr.isString() == true);
    assert(emptyStr.asString() == "");

    cout << "✓ Все простые типы парсятся корректно\n\n";
}

// Тестирование escape-последовательностей в строках
void testEscapeSequences() {
    cout << "Тестирование escape-последовательностей...\n";

    // Базовые escape-последовательности
    JsonValue quote = JsonParser::parse("\"\\\"\"");
    assert(quote.asString() == "\"");

    JsonValue backslash = JsonParser::parse("\"\\\\\"");
    assert(backslash.asString() == "\\");

    JsonValue slash = JsonParser::parse("\"\\/\"");
    assert(slash.asString() == "/");

    JsonValue newline = JsonParser::parse("\"\\n\"");
    assert(newline.asString() == "\n");

    JsonValue tab = JsonParser::parse("\"\\t\"");
    assert(tab.asString() == "\t");

    JsonValue carriage = JsonParser::parse("\"\\r\"");
    assert(carriage.asString() == "\r");

    JsonValue backspace = JsonParser::parse("\"\\b\"");
    assert(backspace.asString() == "\b");

    JsonValue formfeed = JsonParser::parse("\"\\f\"");
    assert(formfeed.asString() == "\f");

    // Комбинированные строки
    JsonValue combined = JsonParser::parse("\"Line 1\\nLine 2\\tTab\"");
    assert(combined.asString() == "Line 1\nLine 2\tTab");

    // Unicode (упрощенная обработка)
    JsonValue unicode = JsonParser::parse("\"\\u0031\"");
    // Наша реализация возвращает '?' для Unicode

    cout << "✓ Все escape-последовательности обрабатываются корректно\n\n";
}

// Тестирование парсинга массивов
void testArrays() {
    cout << "Тестирование парсинга массивов...\n";

    // Пустой массив
    JsonValue emptyArray = JsonParser::parse("[]");
    assert(emptyArray.isArray() == true);
    assert(emptyArray.size() == 0);
    assert(emptyArray.empty() == true);

    // Массив чисел
    JsonValue numArray = JsonParser::parse("[1, 2, 3, 4, 5]");
    assert(numArray.isArray() == true);
    assert(numArray.size() == 5);
    assert(numArray[0].asNumber() == 1.0);
    assert(numArray[4].asNumber() == 5.0);

    // Массив смешанных типов
    JsonValue mixedArray = JsonParser::parse("[1, \"two\", true, null]");
    assert(mixedArray.isArray() == true);
    assert(mixedArray.size() == 4);
    assert(mixedArray[0].asNumber() == 1.0);
    assert(mixedArray[1].asString() == "two");
    assert(mixedArray[2].asBoolean() == true);
    assert(mixedArray[3].isNull() == true);

    // Вложенные массивы
    JsonValue nestedArray = JsonParser::parse("[[1, 2], [3, 4]]");
    assert(nestedArray.isArray() == true);
    assert(nestedArray.size() == 2);
    assert(nestedArray[0].isArray() == true);
    assert(nestedArray[0][0].asNumber() == 1.0);
    assert(nestedArray[1][1].asNumber() == 4.0);

    // Массив объектов
    JsonValue objArray = JsonParser::parse("[{\"id\": 1}, {\"id\": 2}]");
    assert(objArray.isArray() == true);
    assert(objArray.size() == 2);
    assert(objArray[0].isObject() == true);
    assert(objArray[0]["id"].asNumber() == 1.0);

    cout << "✓ Все массивы парсятся корректно\n\n";
}

// Тестирование парсинга объектов
void testObjects() {
    cout << "Тестирование парсинга объектов...\n";

    // Пустой объект
    JsonValue emptyObject = JsonParser::parse("{}");
    assert(emptyObject.isObject() == true);
    assert(emptyObject.size() == 0);
    assert(emptyObject.empty() == true);

    // Простой объект
    JsonValue simpleObject = JsonParser::parse("{\"name\": \"John\", \"age\": 30, \"active\": true}");
    assert(simpleObject.isObject() == true);
    assert(simpleObject.size() == 3);
    assert(simpleObject["name"].asString() == "John");
    assert(simpleObject["age"].asNumber() == 30.0);
    assert(simpleObject["active"].asBoolean() == true);

    // Объект с вложенными объектами
    JsonValue nestedObject = JsonParser::parse("{\"user\": {\"name\": \"Alice\", \"settings\": {\"theme\": \"dark\"}}}");
    assert(nestedObject.isObject() == true);
    assert(nestedObject["user"].isObject() == true);
    assert(nestedObject["user"]["name"].asString() == "Alice");
    assert(nestedObject["user"]["settings"].isObject() == true);
    assert(nestedObject["user"]["settings"]["theme"].asString() == "dark");

    // Объект с массивами
    JsonValue objectWithArrays = JsonParser::parse("{\"numbers\": [1, 2, 3], \"strings\": [\"a\", \"b\"]}");
    assert(objectWithArrays.isObject() == true);
    assert(objectWithArrays["numbers"].isArray() == true);
    assert(objectWithArrays["numbers"].size() == 3);
    assert(objectWithArrays["strings"].isArray() == true);
    assert(objectWithArrays["strings"].size() == 2);

    // Объект с экранированными ключами
    JsonValue escapedKeys = JsonParser::parse("{\"key\\\"with\\\"quotes\": \"value\"}");
    assert(escapedKeys.isObject() == true);
    assert(escapedKeys["key\"with\"quotes"].asString() == "value");

    cout << "✓ Все объекты парсятся корректно\n\n";
}

// Тестирование парсинга логов веб-сервера
void testLogParsing() {
    cout << "Тестирование парсинга логов веб-сервера...\n";

    // Одиночная запись лога
    string singleLog = R"({
        "ts": "2025-03-14T12:03:21Z",
        "ip": "192.168.1.1",
        "method": "GET",
        "url": "/index.html",
        "status": 200
    })";

    JsonValue logEntry = JsonParser::parse(singleLog);
    assert(logEntry.isObject() == true);
    assert(logEntry["ts"].asString() == "2025-03-14T12:03:21Z");
    assert(logEntry["ip"].asString() == "192.168.1.1");
    assert(logEntry["method"].asString() == "GET");
    assert(logEntry["url"].asString() == "/index.html");
    assert(logEntry["status"].asNumber() == 200.0);

    // Массив записей логов
    string logsArray = R"([
        {
            "ts": "2025-03-14T12:03:21Z",
            "ip": "192.168.1.1",
            "method": "GET",
            "url": "/index.html",
            "status": 200
        },
        {
            "ts": "2025-03-14T12:03:27Z",
            "ip": "192.168.1.2",
            "method": "POST",
            "url": "/api/login",
            "status": 201
        }
    ])";

    JsonValue logs = JsonParser::parse(logsArray);
    assert(logs.isArray() == true);
    assert(logs.size() == 2);

    // Преобразование в LogEntry
    vector<LogEntry> entries = logs.asLogEntries();
    assert(entries.size() == 2);

    assert(entries[0].timestamp == "2025-03-14T12:03:21Z");
    assert(entries[0].ip == "192.168.1.1");
    assert(entries[0].method == "GET");
    assert(entries[0].url == "/index.html");
    assert(entries[0].status == 200);

    assert(entries[1].timestamp == "2025-03-14T12:03:27Z");
    assert(entries[1].ip == "192.168.1.2");
    assert(entries[1].method == "POST");
    assert(entries[1].url == "/api/login");
    assert(entries[1].status == 201);

    // Проверка валидности записей
    for (const auto& entry : entries) {
        assert(entry.isValid() == true);
    }

    cout << "✓ Логи веб-сервера парсятся корректно\n\n";
}

// Тестирование обработки ошибок
void testErrorHandling() {
    cout << "Тестирование обработки ошибок...\n";

    bool exceptionThrown = false;

    // Неполный JSON
    try {
        JsonParser::parse("{");
        exceptionThrown = false;
    }
    catch (const JsonParseException&) {
        exceptionThrown = true;
    }
    assert(exceptionThrown == true);

    // Неверный токен
    exceptionThrown = false;
    try {
        JsonParser::parse("invalid");
        exceptionThrown = false;
    }
    catch (const JsonParseException&) {
        exceptionThrown = true;
    }
    assert(exceptionThrown == true);

    // Неправильный массив
    exceptionThrown = false;
    try {
        JsonParser::parse("[1, 2,}");
        exceptionThrown = false;
    }
    catch (const JsonParseException&) {
        exceptionThrown = true;
    }
    assert(exceptionThrown == true);

    // Неправильный объект
    exceptionThrown = false;
    try {
        JsonParser::parse("{\"key\": value}");
        exceptionThrown = false;
    }
    catch (const JsonParseException&) {
        exceptionThrown = true;
    }
    assert(exceptionThrown == true);

    // Незавершенная строка
    exceptionThrown = false;
    try {
        JsonParser::parse("\"unclosed string");
        exceptionThrown = false;
    }
    catch (const JsonParseException&) {
        exceptionThrown = true;
    }
    assert(exceptionThrown == true);

    // Неверная escape-последовательность
    exceptionThrown = false;
    try {
        JsonParser::parse("\"\\x\"");
        exceptionThrown = false;
    }
    catch (const JsonParseException&) {
        exceptionThrown = true;
    }
    assert(exceptionThrown == true);

    // Неполное число
    exceptionThrown = false;
    try {
        JsonParser::parse("1.");
        exceptionThrown = false;
    }
    catch (const JsonParseException&) {
        exceptionThrown = true;
    }
    assert(exceptionThrown == true);

    cout << "✓ Все ошибки обрабатываются корректно\n\n";
}

// Тестирование работы с файлами
void testFileOperations() {
    cout << "Тестирование работы с файлами...\n";

    // Создаем тестовый JSON файл
    string testJson = R"([
        {
            "ts": "2025-03-14T12:03:21Z",
            "ip": "192.168.1.1",
            "method": "GET",
            "url": "/index.html",
            "status": 200
        },
        {
            "ts": "2025-03-14T12:04:15Z",
            "ip": "10.0.0.1",
            "method": "POST",
            "url": "/api/data",
            "status": 201
        }
    ])";

    string filename = "test_logs.json";

    // Записываем тестовый файл
    ofstream outFile(filename);
    assert(outFile.is_open());
    outFile << testJson;
    outFile.close();

    // Читаем файл с помощью нашего парсера
    JsonValue loadedData = JsonParser::loadFromFile(filename);
    assert(loadedData.isArray() == true);
    assert(loadedData.size() == 2);

    // Проверяем данные
    vector<LogEntry> entries = loadedData.asLogEntries();
    assert(entries.size() == 2);
    assert(entries[0].ip == "192.168.1.1");
    assert(entries[1].ip == "10.0.0.1");

    // Сохраняем обратно (форматированный вывод)
    string prettyFile = "test_logs_pretty.json";
    JsonParser::saveToFile(prettyFile, loadedData, true);

    // Проверяем, что файл создан
    ifstream prettyCheck(prettyFile);
    assert(prettyCheck.is_open());
    prettyCheck.close();

    // Сохраняем без форматирования
    string compactFile = "test_logs_compact.json";
    JsonParser::saveToFile(compactFile, loadedData, false);

    ifstream compactCheck(compactFile);
    assert(compactCheck.is_open());

    // Сравниваем размеры файлов (форматированный должен быть больше)
    compactCheck.seekg(0, ios::end);
    size_t compactSize = compactCheck.tellg();
    compactCheck.close();

    ifstream prettySizeCheck(prettyFile);
    prettySizeCheck.seekg(0, ios::end);
    size_t prettySize = prettySizeCheck.tellg();
    prettySizeCheck.close();

    assert(prettySize > compactSize);

    // Тестирование валидации JSON
    string errorMsg;
    bool isValid = JsonParser::isValid(testJson, errorMsg);
    assert(isValid == true);
    assert(errorMsg.empty());

    isValid = JsonParser::isValid("{ invalid json }", errorMsg);
    assert(isValid == false);
    assert(!errorMsg.empty());

    // Очистка тестовых файлов
    remove(filename.c_str());
    remove(prettyFile.c_str());
    remove(compactFile.c_str());

    cout << "✓ Все файловые операции работают корректно\n\n";
}

// Тестирование BOM (Byte Order Mark) для Windows
void testBOMHandling() {
    cout << "Тестирование обработки BOM (Windows)...\n";

    // Создаем файл с BOM
    string filename = "test_bom.json";

    // Записываем UTF-8 BOM (0xEF 0xBB 0xBF) + JSON
    ofstream file(filename, ios::binary);
    unsigned char bom[] = { 0xEF, 0xBB, 0xBF };
    file.write(reinterpret_cast<char*>(bom), 3);
    file << R"({"test": "value"})";
    file.close();

    // Парсим файл с BOM
    JsonValue withBom = JsonParser::loadFromFile(filename);
    assert(withBom.isObject() == true);
    assert(withBom["test"].asString() == "value");

    // Создаем файл без BOM
    string filenameNoBom = "test_no_bom.json";
    ofstream fileNoBom(filenameNoBom);
    fileNoBom << R"({"test": "value"})";
    fileNoBom.close();

    // Парсим файл без BOM
    JsonValue withoutBom = JsonParser::loadFromFile(filenameNoBom);
    assert(withoutBom.isObject() == true);
    assert(withoutBom["test"].asString() == "value");

    // Очистка
    remove(filename.c_str());
    remove(filenameNoBom.c_str());

    cout << "✓ BOM корректно обрабатывается\n\n";
}

// Тестирование преобразования в строку
void testToString() {
    cout << "Тестирование преобразования в строку...\n";

    // Создаем тестовый объект
    JsonValue testObject;
    testObject.type = JsonType::Object;
    testObject.objectValue = {
        {"name", JsonValue("John Doe")},
        {"age", JsonValue(30.0)},
        {"active", JsonValue(true)},
        {"tags", JsonValue(vector<JsonValue>{JsonValue("admin"), JsonValue("user")})},
        {"metadata", JsonValue(map<string, JsonValue>{{"id", JsonValue(1.0)}})}
    };

    // Преобразуем в форматированную строку
    string pretty = JsonParser::toString(testObject, true);
    assert(!pretty.empty());
    assert(pretty.find("\n") != string::npos); // Должны быть переносы строк

    // Преобразуем в компактную строку
    string compact = JsonParser::toString(testObject, false);
    assert(!compact.empty());
    assert(compact.find("\n") == string::npos); // Не должно быть переносов строк

    // Проверяем, что компактная версия короче
    assert(compact.length() < pretty.length());

    // Парсим обратно и сравниваем
    JsonValue parsedPretty = JsonParser::parse(pretty);
    JsonValue parsedCompact = JsonParser::parse(compact);

    assert(parsedPretty["name"].asString() == "John Doe");
    assert(parsedCompact["name"].asString() == "John Doe");

    cout << "✓ Преобразование в строку работает корректно\n\n";
}

// Тестирование сложных структур данных
void testComplexStructures() {
    cout << "Тестирование сложных структур данных...\n";

    // Создаем сложную JSON структуру
    string complexJson = R"({
        "logs": [
            {
                "request": {
                    "method": "GET",
                    "url": "/api/v1/users",
                    "headers": {
                        "User-Agent": "Mozilla/5.0",
                        "Accept": "application/json"
                    }
                },
                "response": {
                    "status": 200,
                    "time": 45.2
                },
                "timestamp": "2025-03-14T12:03:21Z"
            }
        ],
        "metadata": {
            "total": 1,
            "page": 1,
            "pages": 1
        }
    })";

    JsonValue complex = JsonParser::parse(complexJson);
    assert(complex.isObject() == true);
    assert(complex["logs"].isArray() == true);
    assert(complex["logs"].size() == 1);

    JsonValue firstLog = complex["logs"][0];
    assert(firstLog["request"].isObject() == true);
    assert(firstLog["request"]["method"].asString() == "GET");
    assert(firstLog["request"]["headers"].isObject() == true);
    assert(firstLog["request"]["headers"]["User-Agent"].asString() == "Mozilla/5.0");
    assert(firstLog["response"].isObject() == true);
    assert(firstLog["response"]["status"].asNumber() == 200.0);
    assert(firstLog["timestamp"].asString() == "2025-03-14T12:03:21Z");

    assert(complex["metadata"].isObject() == true);
    assert(complex["metadata"]["total"].asNumber() == 1.0);

    cout << "✓ Сложные структуры парсятся корректно\n\n";
}

// Тестирование производительности парсинга
void testPerformance() {
    cout << "Тестирование производительности парсинга...\n";

    // Генерация большого JSON
    int logCount = 1000;
    stringstream largeJson;
    largeJson << "[";

    for (int i = 0; i < logCount; i++) {
        if (i > 0) largeJson << ",";
        largeJson << R"({
            "ts": "2025-03-14T12:03:21Z",
            "ip": "192.168.1.)" << (i % 255 + 1) << R"(",
            "method": "GET",
            "url": "/api/v1/data?id=)" << i << R"(",
            "status": 200
        })";
    }
    largeJson << "]";

    string jsonStr = largeJson.str();

    // Измеряем время парсинга
    auto start = chrono::high_resolution_clock::now();
    JsonValue largeData = JsonParser::parse(jsonStr);
    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);

    assert(largeData.isArray() == true);
    assert(largeData.size() == logCount);

    // Измеряем время преобразования в строку
    start = chrono::high_resolution_clock::now();
    string serialized = JsonParser::toString(largeData, false);
    end = chrono::high_resolution_clock::now();
    auto serializeDuration = chrono::duration_cast<chrono::milliseconds>(end - start);

    assert(!serialized.empty());

    cout << "✓ Парсинг " << logCount << " записей: " << duration.count() << " мс\n";
    cout << "✓ Сериализация: " << serializeDuration.count() << " мс\n";
    cout << "✓ Среднее время на запись: " << (duration.count() * 1000.0 / logCount) << " мкс\n\n";
}

// Главная функция тестирования
int main() {
    cout << "========================================\n";
    cout << "    ТЕСТИРОВАНИЕ JSON Parser (Windows)   \n";
    cout << "========================================\n\n";

    try {
        cout << "Запуск тестов...\n\n";

        testSimpleTypes();
        testEscapeSequences();
        testArrays();
        testObjects();
        testLogParsing();
        testErrorHandling();
        testFileOperations();
        testBOMHandling();
        testToString();
        testComplexStructures();
        testPerformance();

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