#include "formatter.h"
#include <windows.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cmath>

using namespace std;

// Статические переменные
bool LogFormatter::consoleInitialized = false;
void* LogFormatter::originalConsoleMode = nullptr;

// Настройка консоли Windows
void LogFormatter::setupConsole() {
    if (consoleInitialized) return;

    // Сохраняем оригинальный режим консоли
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD originalMode;
    GetConsoleMode(hOut, &originalMode);
    originalConsoleMode = reinterpret_cast<void*>(static_cast<size_t>(originalMode));

    // Устанавливаем UTF-8 кодировку
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    // Включаем виртуальный терминал для ANSI escape-последовательностей
    DWORD mode;
    GetConsoleMode(hOut, &mode);
    mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, mode);

    consoleInitialized = true;
}

// Включение UTF-8 кодировки
void LogFormatter::setUTF8Encoding() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
}

// Включение виртуального терминала
void LogFormatter::enableVirtualTerminal() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode;
    GetConsoleMode(hOut, &mode);
    mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, mode);
}

// Восстановление оригинальных настроек
void LogFormatter::restoreConsole() {
    if (!consoleInitialized || !originalConsoleMode) return;

    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD originalMode = static_cast<DWORD>(reinterpret_cast<size_t>(originalConsoleMode));
    SetConsoleMode(hOut, originalMode);

    consoleInitialized = false;
    originalConsoleMode = nullptr;
}

// Установка цвета текста
void LogFormatter::setColor(Color color) {
    string code = getColorCode(color);
    if (!code.empty()) {
        cout << code;
    }
}

// Сброс цвета
void LogFormatter::resetColor() {
    cout << getResetCode();
}

// Получение кода цвета для Windows
string LogFormatter::getColorCode(Color color) {
    switch (color) {
    case Color::Default:    return "\033[0m";
    case Color::Black:      return "\033[30m";
    case Color::Red:        return "\033[31m";
    case Color::Green:      return "\033[32m";
    case Color::Yellow:     return "\033[33m";
    case Color::Blue:       return "\033[34m";
    case Color::Magenta:    return "\033[35m";
    case Color::Cyan:       return "\033[36m";
    case Color::White:      return "\033[37m";
    case Color::BrightRed:  return "\033[91m";
    case Color::BrightGreen:return "\033[92m";
    case Color::BrightYellow:return "\033[93m";
    case Color::BrightBlue: return "\033[94m";
    case Color::BrightMagenta:return "\033[95m";
    case Color::BrightCyan: return "\033[96m";
    case Color::BrightWhite:return "\033[97m";
    default:               return "\033[0m";
    }
}

// Код сброса
string LogFormatter::getResetCode() {
    return "\033[0m";
}

// Форматирование таблицы
string LogFormatter::formatTable(const vector<vector<string>>& data,
    const vector<string>& headers,
    const TableConfig& config) {
    if (data.empty() && headers.empty()) {
        return "";
    }

    // Вычисляем ширину колонок
    vector<int> colWidths = calculateColumnWidths(data, headers, config);

    ostringstream oss;
    string borderChar = config.showBorder ? "─" : " ";
    string cornerChar = config.showBorder ? "┼" : " ";

    // Верхняя граница
    if (config.showBorder) {
        oss << "┌";
        for (size_t i = 0; i < colWidths.size(); i++) {
            oss << repeatChar('─', colWidths[i] + 2);
            if (i < colWidths.size() - 1) oss << "┬";
        }
        oss << "┐\n";
    }

    // Заголовок
    if (config.showHeader && !headers.empty()) {
        if (config.showBorder) oss << "│";

        for (size_t i = 0; i < headers.size(); i++) {
            oss << " " << alignText(headers[i], colWidths[i], 'c') << " ";
            if (config.showBorder) oss << "│";
        }
        oss << "\n";

        // Разделитель заголовка
        if (config.showBorder) {
            oss << "├";
            for (size_t i = 0; i < colWidths.size(); i++) {
                oss << repeatChar('─', colWidths[i] + 2);
                if (i < colWidths.size() - 1) oss << "┼";
            }
            oss << "┤\n";
        }
    }

    // Данные
    for (size_t row = 0; row < data.size(); row++) {
        if (config.showBorder) oss << "│";

        // Альтернативный цвет строк
        if (config.altRowColor != Color::Default && row % 2 == 1) {
            oss << getColorCode(config.altRowColor);
        }

        for (size_t col = 0; col < data[row].size(); col++) {
            oss << " " << alignText(data[row][col], colWidths[col], 'l') << " ";
            if (config.showBorder) oss << "│";
        }

        if (config.altRowColor != Color::Default && row % 2 == 1) {
            oss << getResetCode();
        }

        oss << "\n";
    }

    // Нижняя граница
    if (config.showBorder) {
        oss << "└";
        for (size_t i = 0; i < colWidths.size(); i++) {
            oss << repeatChar('─', colWidths[i] + 2);
            if (i < colWidths.size() - 1) oss << "┴";
        }
        oss << "┘\n";
    }

    return oss.str();
}

// Форматирование таблицы логов
string LogFormatter::formatLogsTable(const vector<LogEntry>& logs,
    int maxRows,
    const TableConfig& config) {
    if (logs.empty()) {
        return "Нет данных для отображения\n";
    }

    // Ограничиваем количество строк
    size_t displayCount = min(logs.size(), static_cast<size_t>(maxRows));

    vector<vector<string>> data;
    vector<string> headers = { "Время", "IP", "Метод", "URL", "Статус" };

    for (size_t i = 0; i < displayCount; i++) {
        const auto& log = logs[i];
        vector<string> row = {
            log.timestamp.substr(11, 8), // только время
            log.ip,
            log.method,
            log.url.length() > 30 ? log.url.substr(0, 27) + "..." : log.url,
            to_string(log.status)
        };
        data.push_back(row);
    }

    string result = formatTable(data, headers, config);

    if (logs.size() > displayCount) {
        ostringstream oss;
        oss << result << "\n... и еще " << (logs.size() - displayCount)
            << " записей (показано: " << displayCount << ")\n";
        return oss.str();
    }

    return result;
}

// Форматирование топ-таблицы
string LogFormatter::formatTopTable(const vector<pair<string, int>>& data,
    const string& title,
    const string& keyColumn,
    const string& valueColumn,
    const TableConfig& config) {
    vector<vector<string>> tableData;
    vector<string> headers = { keyColumn, valueColumn };

    for (const auto& [key, value] : data) {
        tableData.push_back({ key, to_string(value) });
    }

    ostringstream oss;
    oss << title << "\n\n";
    oss << formatTable(tableData, headers, config);

    return oss.str();
}

// Форматирование статистики
string LogFormatter::formatStatisticsTable(const LogAnalyzer::Statistics& stats,
    const TableConfig& config) {
    vector<vector<string>> data;
    vector<string> headers = { "Параметр", "Значение" };

    data.push_back({ "Общее количество запросов", to_string(stats.totalRequests) });
    data.push_back({ "Уникальных IP-адресов", to_string(stats.uniqueIPs) });
    data.push_back({ "Уникальных URL", to_string(stats.uniqueURLs) });
    data.push_back({ "Начало периода", stats.timeRangeStart });
    data.push_back({ "Конец периода", stats.timeRangeEnd });
    data.push_back({ "Средняя нагрузка", to_string(stats.requestsPerSecond) + " запр/сек" });

    // Распределение по статусам
    for (const auto& [status, count] : stats.statusCounts) {
        double percentage = (count * 100.0) / stats.totalRequests;
        ostringstream oss;
        oss << "Статус " << status << ": " << count << " ("
            << fixed << setprecision(1) << percentage << "%)";
        data.push_back({ "", oss.str() });
    }

    return formatTable(data, headers, config);
}

// Форматирование сводки
string LogFormatter::formatSummary(const LogAnalyzer& analyzer) {
    auto stats = analyzer.getDetailedStatistics();

    ostringstream oss;
    oss << "╔════════════════════════════════════════════════════════════════╗\n";
    oss << "║                     СВОДКА АНАЛИЗА ЛОГОВ                      ║\n";
    oss << "╚════════════════════════════════════════════════════════════════╝\n\n";

    oss << "Общая информация:\n";
    oss << "────────────────\n";
    oss << "• Запросов всего: " << stats.totalRequests << "\n";
    oss << "• Уникальных IP: " << stats.uniqueIPs << "\n";
    oss << "• Уникальных URL: " << stats.uniqueURLs << "\n";
    oss << "• Период: " << stats.timeRangeStart << " - " << stats.timeRangeEnd << "\n";
    oss << "• Средняя нагрузка: " << fixed << setprecision(2)
        << stats.requestsPerSecond << " запросов/сек\n\n";

    oss << "Распределение по статусам:\n";
    oss << "─────────────────────────\n";
    for (const auto& [status, count] : stats.statusCounts) {
        double percentage = (count * 100.0) / stats.totalRequests;
        oss << "• " << status << ": " << count << " ("
            << fixed << setprecision(1) << percentage << "%)\n";
    }

    return oss.str();
}

// Вспомогательные функции
string LogFormatter::repeatChar(char ch, int count) {
    return string(max(0, count), ch);
}

string LogFormatter::alignText(const string& text, int width, char align) {
    if (static_cast<int>(text.length()) >= width) {
        return text.substr(0, width);
    }

    int padding = width - static_cast<int>(text.length());

    switch (align) {
    case 'c': // center
        return string(padding / 2, ' ') + text + string(padding - padding / 2, ' ');
    case 'r': // right
        return string(padding, ' ') + text;
    case 'l': // left
    default:
        return text + string(padding, ' ');
    }
}

// Расчет ширины колонок
vector<int> LogFormatter::calculateColumnWidths(const vector<vector<string>>& data,
    const vector<string>& headers,
    const TableConfig& config) {
    vector<int> widths(headers.size(), 0);

    // Минимальная ширина по заголовкам
    for (size_t i = 0; i < headers.size(); i++) {
        widths[i] = max(config.minColumnWidth,
            min(config.maxColumnWidth, static_cast<int>(headers[i].length())));
    }

    // Учитываем данные
    for (const auto& row : data) {
        for (size_t i = 0; i < min(row.size(), widths.size()); i++) {
            int cellWidth = min(config.maxColumnWidth, static_cast<int>(row[i].length()));
            widths[i] = max(widths[i], cellWidth);
        }
    }

    return widths;
}

// Конструктор ProgressBar
LogFormatter::ProgressBar::ProgressBar(int total, int width, const string& message)
    : total(total), current(0), width(width), message(message),
    showPercentage(true), showTime(true) {
}

// Обновление прогресс-бара
void LogFormatter::ProgressBar::update(int value) {
    current = min(max(value, 0), total);
}

// Инкремент прогресс-бара
void LogFormatter::ProgressBar::increment(int step) {
    update(current + step);
}

// Завершение прогресс-бара
void LogFormatter::ProgressBar::finish(const string& finalMessage) {
    current = total;
    message = finalMessage;
    display();
}

// Отображение прогресс-бара
void LogFormatter::ProgressBar::display() const {
    cout << toString() << flush;
}

// Преобразование прогресс-бара в строку
string LogFormatter::ProgressBar::toString() const {
    ostringstream oss;

    if (!message.empty()) {
        oss << message << " ";
    }

    int filled = static_cast<int>((current * width) / total);
    int percent = static_cast<int>((current * 100) / total);

    oss << "[";
    for (int i = 0; i < width; i++) {
        if (i < filled) {
            oss << "█";
        }
        else if (i == filled) {
            oss << "▌";
        }
        else {
            oss << " ";
        }
    }
    oss << "]";

    if (showPercentage) {
        oss << " " << setw(3) << percent << "%";
    }

    if (showTime && total > 0 && current > 0) {
        // Примерное время до завершения
        double progress = static_cast<double>(current) / total;
        if (progress > 0.01) {
            int estimatedTotal = static_cast<int>(time(nullptr) * (1.0 / progress));
            int remaining = estimatedTotal - static_cast<int>(time(nullptr));
            if (remaining > 0) {
                oss << " (~" << remaining << "с)";
            }
        }
    }

    oss << "\r";
    return oss.str();
}