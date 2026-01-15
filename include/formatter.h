#ifndef FORMATTER_H
#define FORMATTER_H

#include <string>
#include <vector>
#include <map>
#include "log_entry.h"
#include "analyzer.h"

// Класс для форматированного вывода результатов анализа

class LogFormatter {
public:
    // Настройка вывода для Windows Console
    static void setupConsole();
    static void setUTF8Encoding();
    static void enableVirtualTerminal();
    static void restoreConsole();

    // Цветной вывод
    enum class Color {
        Default,
        Black,
        Red,
        Green,
        Yellow,
        Blue,
        Magenta,
        Cyan,
        White,
        BrightRed,
        BrightGreen,
        BrightYellow,
        BrightBlue,
        BrightMagenta,
        BrightCyan,
        BrightWhite
    };

    static void setColor(Color color);
    static void resetColor();

    // Форматирование таблиц
    struct TableConfig {
        bool showBorder = true;
        bool showHeader = true;
        bool showRowNumbers = false;
        Color headerColor = Color::BrightCyan;
        Color borderColor = Color::White;
        Color altRowColor = Color::Default;
        int minColumnWidth = 10;
        int maxColumnWidth = 50;
    };

    static std::string formatTable(
        const std::vector<std::vector<std::string>>& data,
        const std::vector<std::string>& headers,
        const TableConfig& config = TableConfig()
    );

    // Специализированные таблицы для логов
    static std::string formatLogsTable(
        const std::vector<LogEntry>& logs,
        int maxRows = 50,
        const TableConfig& config = TableConfig()
    );

    static std::string formatTopTable(
        const std::vector<std::pair<std::string, int>>& data,
        const std::string& title,
        const std::string& keyColumn,
        const std::string& valueColumn,
        const TableConfig& config = TableConfig()
    );

    static std::string formatStatisticsTable(
        const LogAnalyzer::Statistics& stats,
        const TableConfig& config = TableConfig()
    );

    // Форматирование сводок
    static std::string formatSummary(const LogAnalyzer& analyzer);
    static std::string formatStatusSummary(const LogAnalyzer& analyzer);
    static std::string formatMethodSummary(const LogAnalyzer& analyzer);
    static std::string formatTimeSummary(const LogAnalyzer& analyzer);

    // Форматирование деталей
    static std::string formatLogEntryDetails(const LogEntry& entry);
    static std::string formatFilterResults(const std::vector<LogEntry>& results,
        const std::string& filterDescription);

    // Графики ASCII
    static std::string formatBarChart(
        const std::map<std::string, int>& data,
        int width = 50,
        int height = 10
    );

    static std::string formatHistogram(
        const std::map<int, int>& data,
        int width = 50,
        int height = 10
    );

    // Прогресс-бар для Windows
    class ProgressBar {
    private:
        int total;
        int current;
        int width;
        std::string message;
        bool showPercentage;
        bool showTime;

    public:
        ProgressBar(int total, int width = 50,
            const std::string& message = "Обработка...");

        void update(int value);
        void increment(int step = 1);
        void finish(const std::string& finalMessage = "Готово!");

        void setMessage(const std::string& msg) { message = msg; }
        void showPercentage(bool show) { showPercentage = show; }
        void showTime(bool show) { showTime = show; }

        std::string toString() const;
        void display() const;
    };

    // Утилиты для текста
    static std::string wrapText(const std::string& text, int width);
    static std::string alignText(const std::string& text, int width,
        char align = 'l'); // l=left, c=center, r=right
    static std::string truncateText(const std::string& text, int maxLength,
        const std::string& suffix = "...");

    // Windows-специфичные методы
    static std::string formatForClipboard(const std::string& text);
    static bool copyToClipboard(const std::string& text);
    static std::string getConsoleSize(); // возвращает "ширинаxвысота"

private:
    // Вспомогательные методы
    static std::vector<int> calculateColumnWidths(
        const std::vector<std::vector<std::string>>& data,
        const std::vector<std::string>& headers,
        const TableConfig& config
    );

    static std::string repeatChar(char ch, int count);
    static std::string formatCell(const std::string& content, int width, char align);

    // Цветовые коды для Windows
    static std::string getColorCode(Color color);
    static std::string getResetCode();

    // Состояние консоли
    static bool consoleInitialized;
    static void* originalConsoleMode; // HANDLE для Windows
};

#endif // FORMATTER_H