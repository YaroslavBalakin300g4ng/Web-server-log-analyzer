#ifndef MAIN_H
#define MAIN_H

#include <string>
#include <vector>
#include <map>

// Интерфейс командной строки и меню

namespace CLI {

    // Структура для хранения аргументов командной строки
    struct CommandLineArgs {
        std::string inputFile;
        std::string outputFile;
        int topIPs = 10;
        int topURLs = 10;
        std::vector<int> filterStatus;
        std::vector<std::string> filterMethod;
        std::string filterTimeStart;
        std::string filterTimeEnd;
        std::string filterIP;
        std::string filterURL;
        bool showHelp = false;
        bool interactive = false;
        bool verbose = false;
        bool exportCSV = false;
        bool exportJSON = false;
        bool colorOutput = true;

        // Windows-специфичные
        bool useWindowsConsole = true;
        bool enableVT100 = true;
        std::string codepage = "UTF-8";
    };

    // Парсинг аргументов командной строки
    CommandLineArgs parseArguments(int argc, char* argv[]);

    // Отображение справки
    void showHelp();
    void showUsage();
    void showExamples();

    // Интерактивное меню
    class InteractiveMenu {
    private:
        bool running;
        std::vector<LogEntry> currentLogs;
        LogAnalyzer analyzer;
        std::string currentFile;

        // Windows-специфичные
        void* consoleHandle;

    public:
        InteractiveMenu();
        ~InteractiveMenu();

        void run();

    private:
        void showMainMenu();
        void loadLogs();
        void showStatistics();
        void showTopIPs();
        void showTopURLs();
        void applyFilters();
        void exportResults();
        void saveReport();
        void showLogDetails();

        // Вспомогательные методы
        void printHeader(const std::string& title);
        void printSeparator();
        void waitForEnter();
        void clearScreen();

        // Windows-специфичные методы
        void setupWindowsConsole();
        void restoreWindowsConsole();
    };

    // Утилиты для работы с пользовательским вводом
    std::string promptString(const std::string& message,
        const std::string& defaultValue = "");
    int promptInt(const std::string& message,
        int minValue = INT_MIN,
        int maxValue = INT_MAX,
        int defaultValue = 0);
    bool promptYesNo(const std::string& message, bool defaultValue = true);

    // Форматирование вывода для командной строки
    void printError(const std::string& message);
    void printWarning(const std::string& message);
    void printSuccess(const std::string& message);
    void printInfo(const std::string& message);

    // Windows-специфичное форматирование
    void printColor(const std::string& message, int colorCode);
    void printProgressBar(int current, int total, const std::string& message = "");

    // Обработка ошибок ввода/вывода
    class CLIException : public std::runtime_error {
    public:
        explicit CLIException(const std::string& msg) : std::runtime_error(msg) {}
    };
};

#endif // MAIN_H