#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <windows.h>
#include "log_entry.h"
#include "json_parser.h"
#include "analyzer.h"
#include "formatter.h"
#include "windows_utils.h"

using namespace std;

// Глобальные переменные
vector<LogEntry> currentLogs;
LogAnalyzer* analyzer = nullptr;
string currentFileName;

// Настройка консоли Windows
void setupConsole() {
    WindowsUtils::setupConsole();
    LogFormatter::setupConsole();
    LogFormatter::setUTF8Encoding();
    LogFormatter::enableVirtualTerminal();

    // Скрыть курсор для более чистого вывода
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hOut, &cursorInfo);
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(hOut, &cursorInfo);
}

// Восстановление консоли
void restoreConsole() {
    // Показать курсор
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hOut, &cursorInfo);
    cursorInfo.bVisible = TRUE;
    SetConsoleCursorInfo(hOut, &cursorInfo);

    LogFormatter::restoreConsole();
    WindowsUtils::restoreConsole();
}

// Очистка экрана
void clearScreen() {
    system("cls");
}

// Ожидание нажатия клавиши
void waitForKey() {
    cout << "\nНажмите любую клавишу для продолжения...";
    cin.ignore();
    cin.get();
}

// Загрузка логов из файла
bool loadLogs(const string& filename) {
    clearScreen();
    cout << "╔════════════════════════════════════════════════════════════════╗\n";
    cout << "║                    ЗАГРУЗКА ЛОГОВ ИЗ ФАЙЛА                    ║\n";
    cout << "╚════════════════════════════════════════════════════════════════╝\n\n";

    try {
        WindowsUtils::HighResolutionTimer timer;
        timer.start();

        JsonValue jsonData = JsonParser::loadFromFile(filename);
        vector<LogEntry> logs = jsonData.asLogEntries();

        double loadTime = timer.elapsedMilliseconds();

        if (logs.empty()) {
            cout << "Файл загружен, но не содержит корректных записей логов.\n";
            return false;
        }

        // Сохраняем данные
        currentLogs = logs;
        if (analyzer) delete analyzer;
        analyzer = new LogAnalyzer(currentLogs);
        currentFileName = filename;

        cout << "✓ Успешно загружено " << logs.size() << " записей логов\n";
        cout << "✓ Время загрузки: " << loadTime << " мс\n";
        cout << "✓ Средняя скорость: "
            << (WindowsUtils::getFileSize(filename) / 1024.0 / 1024.0) / (loadTime / 1000.0)
            << " МБ/с\n\n";

        // Показываем краткую статистику
        auto stats = analyzer->getDetailedStatistics();
        cout << "Краткая статистика:\n";
        cout << "• Временной диапазон: " << stats.timeRangeStart << " - " << stats.timeRangeEnd << "\n";
        cout << "• Уникальных IP: " << stats.uniqueIPs << "\n";
        cout << "• Уникальных URL: " << stats.uniqueURLs << "\n";

        return true;

    }
    catch (const exception& e) {
        cout << "✗ Ошибка загрузки файла: " << e.what() << "\n";
        return false;
    }
}

// Показать общую статистику
void showStatistics() {
    clearScreen();
    cout << "╔════════════════════════════════════════════════════════════════╗\n";
    cout << "║                      ОБЩАЯ СТАТИСТИКА                         ║\n";
    cout << "╚════════════════════════════════════════════════════════════════╝\n\n";

    if (!analyzer || analyzer->getTotalRequests() == 0) {
        cout << "Нет данных для анализа. Загрузите логи сначала.\n";
        waitForKey();
        return;
    }

    auto stats = analyzer->getDetailedStatistics();

    // Используем форматтер для красивого вывода
    LogFormatter::TableConfig config;
    config.headerColor = LogFormatter::Color::BrightCyan;
    config.showBorder = true;

    vector<vector<string>> data;
    vector<string> headers = { "Параметр", "Значение" };

    data.push_back({ "Загружено из файла", currentFileName });
    data.push_back({ "Всего запросов", to_string(stats.totalRequests) });
    data.push_back({ "Уникальных IP", to_string(stats.uniqueIPs) });
    data.push_back({ "Уникальных URL", to_string(stats.uniqueURLs) });
    data.push_back({ "Начало периода", stats.timeRangeStart });
    data.push_back({ "Конец периода", stats.timeRangeEnd });
    data.push_back({ "Средняя нагрузка", to_string(stats.requestsPerSecond) + " запр/сек" });

    cout << LogFormatter::formatTable(data, headers, config) << "\n";

    // Распределение по статусам
    if (!stats.statusCounts.empty()) {
        cout << "\nРаспределение по статусам:\n";
        for (const auto& [status, count] : stats.statusCounts) {
            double percentage = (count * 100.0) / stats.totalRequests;
            cout << "  Статус " << status << ": " << count << " ("
                << fixed << setprecision(1) << percentage << "%)\n";
        }
    }

    waitForKey();
}

// Показать топ IP-адресов
void showTopIPs() {
    clearScreen();
    cout << "╔════════════════════════════════════════════════════════════════╗\n";
    cout << "║                    ТОП IP-АДРЕСОВ                             ║\n";
    cout << "╚════════════════════════════════════════════════════════════════╝\n\n";

    if (!analyzer || analyzer->getTotalRequests() == 0) {
        cout << "Нет данных для анализа. Загрузите логи сначала.\n";
        waitForKey();
        return;
    }

    cout << "Сколько IP-адресов показать (по умолчанию 10)? ";
    string input;
    getline(cin, input);

    int n = 10;
    if (!input.empty()) {
        try {
            n = stoi(input);
            if (n <= 0) n = 10;
            if (n > 50) n = 50; // Ограничение
        }
        catch (...) {
            n = 10;
        }
    }

    // Измеряем время выполнения
    WindowsUtils::HighResolutionTimer timer;
    timer.start();

    auto topIPs = analyzer->getTopIPs(n);

    double analysisTime = timer.elapsedMilliseconds();

    // Используем форматтер
    LogFormatter::TableConfig config;
    config.headerColor = LogFormatter::Color::BrightYellow;
    config.showBorder = true;
    config.altRowColor = LogFormatter::Color::BrightBlue;

    cout << "\n" << LogFormatter::formatTopTable(topIPs,
        "Топ-" + to_string(n) + " IP-адресов по количеству запросов",
        "IP-адрес", "Запросы", config);

    cout << "✓ Анализ выполнен за " << analysisTime << " мс\n";
    cout << "✓ Обработано " << analyzer->getTotalRequests() << " записей\n";

    waitForKey();
}

// Показать топ URL
void showTopURLs() {
    clearScreen();
    cout << "╔════════════════════════════════════════════════════════════════╗\n";
    cout << "║                       ТОП URL                                 ║\n";
    cout << "╚════════════════════════════════════════════════════════════════╝\n\n";

    if (!analyzer || analyzer->getTotalRequests() == 0) {
        cout << "Нет данных для анализа. Загрузите логи сначала.\n";
        waitForKey();
        return;
    }

    cout << "Сколько URL показать (по умолчанию 10)? ";
    string input;
    getline(cin, input);

    int n = 10;
    if (!input.empty()) {
        try {
            n = stoi(input);
            if (n <= 0) n = 10;
            if (n > 30) n = 30; // URL могут быть длинными
        }
        catch (...) {
            n = 10;
        }
    }

    WindowsUtils::HighResolutionTimer timer;
    timer.start();

    auto topURLs = analyzer->getTopURLs(n);

    double analysisTime = timer.elapsedMilliseconds();

    LogFormatter::TableConfig config;
    config.headerColor = LogFormatter::Color::BrightGreen;
    config.showBorder = true;
    config.altRowColor = LogFormatter::Color::BrightMagenta;
    config.maxColumnWidth = 60; // URL могут быть длинными

    cout << "\n" << LogFormatter::formatTopTable(topURLs,
        "Топ-" + to_string(n) + " URL по количеству обращений",
        "URL", "Запросы", config);

    cout << "✓ Анализ выполнен за " << analysisTime << " мс\n";
    cout << "✓ Обработано " << analyzer->getTotalRequests() << " записей\n";

    waitForKey();
}

// Фильтр по статусу
void filterByStatus() {
    clearScreen();
    cout << "╔════════════════════════════════════════════════════════════════╗\n";
    cout << "║                   ФИЛЬТР ПО СТАТУСУ                          ║\n";
    cout << "╚════════════════════════════════════════════════════════════════╝\n\n";

    if (!analyzer || analyzer->getTotalRequests() == 0) {
        cout << "Нет данных для анализа. Загрузите логи сначала.\n";
        waitForKey();
        return;
    }

    cout << "Введите статус код для фильтрации (например, 404): ";
    string input;
    getline(cin, input);

    if (input.empty()) {
        cout << "Отменено.\n";
        waitForKey();
        return;
    }

    try {
        int status = stoi(input);

        if (status < 100 || status > 599) {
            cout << "Неверный статус код. Допустимый диапазон: 100-599\n";
            waitForKey();
            return;
        }

        WindowsUtils::HighResolutionTimer timer;
        timer.start();

        auto filtered = analyzer->filterByStatus(status);

        double filterTime = timer.elapsedMilliseconds();

        clearScreen();
        cout << "╔════════════════════════════════════════════════════════════════╗\n";
        cout << "║          РЕЗУЛЬТАТЫ ФИЛЬТРАЦИИ ПО СТАТУСУ " << status << "             ║\n";
        cout << "╚════════════════════════════════════════════════════════════════╝\n\n";

        if (filtered.empty()) {
            cout << "Записей со статусом " << status << " не найдено.\n";
        }
        else {
            cout << "Найдено " << filtered.size() << " записей со статусом " << status << "\n";
            cout << "Время фильтрации: " << filterTime << " мс\n\n";

            // Показываем первые 20 записей
            int showCount = min(20, static_cast<int>(filtered.size()));
            cout << "Первые " << showCount << " записей:\n\n";

            for (int i = 0; i < showCount; i++) {
                const auto& log = filtered[i];
                cout << (i + 1) << ". " << log.timestamp.substr(11, 8)
                    << " | " << log.ip << " | " << log.method
                    << " | " << log.url << " | " << log.status << "\n";
            }

            if (filtered.size() > showCount) {
                cout << "... и еще " << (filtered.size() - showCount) << " записей\n";
            }

            cout << "\nЭкспортировать результаты в файл? (y/n): ";
            getline(cin, input);
            if (input == "y" || input == "Y") {
                cout << "Введите имя файла (например, status_" << status << ".csv): ";
                getline(cin, input);
                if (!input.empty()) {
                    if (analyzer->exportToCSV(input)) {
                        cout << "Результаты сохранены в " << input << "\n";
                    }
                    else {
                        cout << "Ошибка сохранения файла.\n";
                    }
                }
            }
        }

    }
    catch (...) {
        cout << "Неверный формат статуса.\n";
    }

    waitForKey();
}

// Фильтр по методу
void filterByMethod() {
    clearScreen();
    cout << "╔════════════════════════════════════════════════════════════════╗\n";
    cout << "║                   ФИЛЬТР ПО МЕТОДУ                           ║\n";
    cout << "╚════════════════════════════════════════════════════════════════╝\n\n";

    if (!analyzer || analyzer->getTotalRequests() == 0) {
        cout << "Нет данных для анализа. Загрузите логи сначала.\n";
        waitForKey();
        return;
    }

    cout << "Доступные методы: GET, POST, PUT, DELETE, HEAD, OPTIONS, PATCH\n";
    cout << "Введите метод для фильтрации (например, GET): ";
    string method;
    getline(cin, method);

    if (method.empty()) {
        cout << "Отменено.\n";
        waitForKey();
        return;
    }

    // Проверяем валидность метода
    string upperMethod = method;
    transform(upperMethod.begin(), upperMethod.end(), upperMethod.begin(), ::toupper);
    vector<string> validMethods = { "GET", "POST", "PUT", "DELETE", "HEAD", "OPTIONS", "PATCH" };

    if (find(validMethods.begin(), validMethods.end(), upperMethod) == validMethods.end()) {
        cout << "Неверный метод HTTP.\n";
        waitForKey();
        return;
    }

    WindowsUtils::HighResolutionTimer timer;
    timer.start();

    auto filtered = analyzer->filterByMethod(upperMethod);

    double filterTime = timer.elapsedMilliseconds();

    clearScreen();
    cout << "╔════════════════════════════════════════════════════════════════╗\n";
    cout << "║         РЕЗУЛЬТАТЫ ФИЛЬТРАЦИИ ПО МЕТОДУ " << upperMethod << "               ║\n";
    cout << "╚════════════════════════════════════════════════════════════════╝\n\n";

    if (filtered.empty()) {
        cout << "Записей с методом " << upperMethod << " не найдено.\n";
    }
    else {
        cout << "Найдено " << filtered.size() << " записей с методом " << upperMethod << "\n";
        cout << "Время фильтрации: " << filterTime << " мс\n\n";

        // Показываем статистику по статусам для этого метода
        map<int, int> statusCounts;
        for (const auto& log : filtered) {
            statusCounts[log.status]++;
        }

        cout << "Распределение по статусам для метода " << upperMethod << ":\n";
        for (const auto& [status, count] : statusCounts) {
            double percentage = (count * 100.0) / filtered.size();
            cout << "  Статус " << status << ": " << count << " ("
                << fixed << setprecision(1) << percentage << "%)\n";
        }

        cout << "\nПоказать записи? (y/n): ";
        string input;
        getline(cin, input);
        if (input == "y" || input == "Y") {
            int showCount = min(20, static_cast<int>(filtered.size()));
            cout << "\nПервые " << showCount << " записей:\n\n";

            for (int i = 0; i < showCount; i++) {
                const auto& log = filtered[i];
                cout << (i + 1) << ". " << log.timestamp.substr(11, 8)
                    << " | " << log.ip << " | " << log.url
                    << " | " << log.status << "\n";
            }
        }
    }

    waitForKey();
}

// Экспорт результатов
void exportResults() {
    clearScreen();
    cout << "╔════════════════════════════════════════════════════════════════╗\n";
    cout << "║                    ЭКСПОРТ РЕЗУЛЬТАТОВ                        ║\n";
    cout << "╚════════════════════════════════════════════════════════════════╝\n\n";

    if (!analyzer || analyzer->getTotalRequests() == 0) {
        cout << "Нет данных для экспорта. Загрузите логи сначала.\n";
        waitForKey();
        return;
    }

    cout << "1. Экспортировать все логи в CSV\n";
    cout << "2. Экспортировать все логи в JSON\n";
    cout << "3. Экспортировать топ IP-адресов\n";
    cout << "4. Экспортировать топ URL\n";
    cout << "0. Отмена\n\n";

    cout << "Выберите опцию: ";
    string choice;
    getline(cin, choice);

    if (choice == "0") return;

    cout << "Введите имя файла для экспорта: ";
    string filename;
    getline(cin, filename);

    if (filename.empty()) {
        cout << "Отменено.\n";
        waitForKey();
        return;
    }

    bool success = false;
    WindowsUtils::HighResolutionTimer timer;
    timer.start();

    if (choice == "1") {
        success = analyzer->exportToCSV(filename);
        if (!filename.ends_with(".csv")) filename += ".csv";
    }
    else if (choice == "2") {
        success = analyzer->exportToJson(filename);
        if (!filename.ends_with(".json")) filename += ".json";
    }
    else if (choice == "3") {
        auto topIPs = analyzer->getTopIPs(50);
        ofstream file(filename);
        if (file.is_open()) {
            file << "IP,Requests\n";
            for (const auto& [ip, count] : topIPs) {
                file << ip << "," << count << "\n";
            }
            file.close();
            success = true;
        }
    }
    else if (choice == "4") {
        auto topURLs = analyzer->getTopURLs(50);
        ofstream file(filename);
        if (file.is_open()) {
            file << "URL,Requests\n";
            for (const auto& [url, count] : topURLs) {
                file << "\"" << url << "\"," << count << "\n";
            }
            file.close();
            success = true;
        }
    }

    double exportTime = timer.elapsedMilliseconds();

    if (success) {
        cout << "✓ Данные успешно экспортированы в " << filename << "\n";
        cout << "✓ Время экспорта: " << exportTime << " мс\n";

        cout << "\nОткрыть файл? (y/n): ";
        string open;
        getline(cin, open);
        if (open == "y" || open == "Y") {
            WindowsUtils::openFileWithDefaultApp(filename);
        }
    }
    else {
        cout << "✗ Ошибка экспорта данных.\n";
    }

    waitForKey();
}

// Главное меню
void showMainMenu() {
    while (true) {
        clearScreen();
        cout << "╔════════════════════════════════════════════════════════════════╗\n";
        cout << "║           АНАЛИЗ ЛОГОВ ВЕБ-СЕРВЕРА - ВАРИАНТ 1                ║\n";
        cout << "║                      Windows версия                          ║\n";
        cout << "╚════════════════════════════════════════════════════════════════╝\n\n";

        if (analyzer && analyzer->getTotalRequests() > 0) {
            cout << "Текущий файл: " << currentFileName << "\n";
            cout << "Загружено записей: " << analyzer->getTotalRequests() << "\n\n";
        }

        cout << "┌──────────────────────────────────────────────────────────────┐\n";
        cout << "│                       ГЛАВНОЕ МЕНЮ                          │\n";
        cout << "├──────────────────────────────────────────────────────────────┤\n";
        cout << "│ 1 - Загрузить логи из JSON файла                            │\n";
        cout << "│ 2 - Показать общую статистику                               │\n";
        cout << "│ 3 - Топ IP-адресов                                          │\n";
        cout << "│ 4 - Топ URL                                                 │\n";
        cout << "│ 5 - Фильтр по статусу                                       │\n";
        cout << "│ 6 - Фильтр по методу                                        │\n";
        cout << "│ 7 - Экспорт результатов                                     │\n";
        cout << "│ 8 - Бенчмарк производительности                             │\n";
        cout << "│ 0 - Выход                                                   │\n";
        cout << "└──────────────────────────────────────────────────────────────┘\n\n";

        cout << "Выберите пункт (0-8): ";
        string choice;
        getline(cin, choice);

        if (choice == "0") {
            break;
        }
        else if (choice == "1") {
            cout << "Введите имя файла: ";
            string filename;
            getline(cin, filename);
            if (!filename.empty()) {
                loadLogs(filename);
            }
        }
        else if (choice == "2") {
            showStatistics();
        }
        else if (choice == "3") {
            showTopIPs();
        }
        else if (choice == "4") {
            showTopURLs();
        }
        else if (choice == "5") {
            filterByStatus();
        }
        else if (choice == "6") {
            filterByMethod();
        }
        else if (choice == "7") {
            exportResults();
        }
        else if (choice == "8") {
            // Бенчмарк
            clearScreen();
            cout << "╔════════════════════════════════════════════════════════════════╗\n";
            cout << "║                БЕНЧМАРК ПРОИЗВОДИТЕЛЬНОСТИ                    ║\n";
            cout << "╚════════════════════════════════════════════════════════════════╝\n\n";

            if (!analyzer || analyzer->getTotalRequests() == 0) {
                cout << "Нет данных для тестирования. Загрузите логи сначала.\n";
                waitForKey();
                continue;
            }

            cout << "Запуск тестов производительности...\n\n";

            WindowsUtils::HighResolutionTimer totalTimer;
            totalTimer.start();

            // Тест 1: Загрузка
            cout << "1. Тест загрузки... ";
            WindowsUtils::HighResolutionTimer timer1;
            timer1.start();
            JsonParser::loadFromFile(currentFileName);
            double loadTime = timer1.elapsedMilliseconds();
            cout << loadTime << " мс\n";

            // Тест 2: Топ IP
            cout << "2. Тест топ IP... ";
            WindowsUtils::HighResolutionTimer timer2;
            timer2.start();
            analyzer->getTopIPs(10);
            double ipTime = timer2.elapsedMilliseconds();
            cout << ipTime << " мс\n";

            // Тест 3: Топ URL
            cout << "3. Тест топ URL... ";
            WindowsUtils::HighResolutionTimer timer3;
            timer3.start();
            analyzer->getTopURLs(10);
            double urlTime = timer3.elapsedMilliseconds();
            cout << urlTime << " мс\n";

            // Тест 4: Фильтрация
            cout << "4. Тест фильтрации... ";
            WindowsUtils::HighResolutionTimer timer4;
            timer4.start();
            analyzer->filterByStatus(404);
            double filterTime = timer4.elapsedMilliseconds();
            cout << filterTime << " мс\n";

            double totalTime = totalTimer.elapsedMilliseconds();

            cout << "\n════════════════════════════════════════════════════════════════\n";
            cout << "Итого:\n";
            cout << "• Всего записей: " << analyzer->getTotalRequests() << "\n";
            cout << "• Общее время тестов: " << totalTime << " мс\n";
            cout << "• Среднее время на запись: " << (totalTime / analyzer->getTotalRequests()) << " мс\n";
            cout << "• Оценка производительности: "
                << (analyzer->getTotalRequests() / (totalTime / 1000.0)) << " запросов/сек\n";

            waitForKey();
        }
    }
}

// Точка входа
int main() {
    // Настройка консоли
    setupConsole();

    try {
        showMainMenu();
    }
    catch (const exception& e) {
        cerr << "Критическая ошибка: " << e.what() << endl;
        waitForKey();
    }

    // Очистка ресурсов
    if (analyzer) {
        delete analyzer;
        analyzer = nullptr;
    }

    // Восстановление консоли
    restoreConsole();

    return 0;
}