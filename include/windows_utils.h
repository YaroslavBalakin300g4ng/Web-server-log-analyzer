#ifndef WINDOWS_UTILS_H
#define WINDOWS_UTILS_H

#include <string>
#include <vector>

// Windows-специфичные утилиты для работы с файлами, консолью и системой

namespace WindowsUtils {

    // ==================== Работа с файлами и путями ====================

    // Проверка существования файла (с поддержкой длинных путей)
    bool fileExists(const std::string& path);

    // Проверка существования директории
    bool directoryExists(const std::string& path);

    // Создание директории (создаёт все промежуточные директории)
    bool createDirectory(const std::string& path);

    // Получение размера файла в байтах
    long long getFileSize(const std::string& path);

    // Чтение всего файла в строку (с обработкой BOM)
    std::string readFileToString(const std::string& path);

    // Запись строки в файл
    bool writeStringToFile(const std::string& path, const std::string& content);

    // Получение текущей рабочей директории
    std::string getCurrentDirectory();

    // Установка текущей рабочей директории
    bool setCurrentDirectory(const std::string& path);

    // Получение абсолютного пути
    std::string getAbsolutePath(const std::string& relativePath);

    // Разделение пути на компоненты
    struct PathComponents {
        std::string drive;
        std::string directory;
        std::string filename;
        std::string extension;
    };

    PathComponents splitPath(const std::string& path);

    // Объединение компонентов пути
    std::string joinPath(const std::string& dir, const std::string& filename);

    // Получение списка файлов в директории
    std::vector<std::string> listFiles(const std::string& directory,
        const std::string& pattern = "*.*");

    // ==================== Работа с консолью ====================

    // Настройка консоли для поддержки UTF-8 и русского языка
    bool setupConsole();

    // Восстановление оригинальных настроек консоли
    void restoreConsole();

    // Получение размеров консоли (ширина, высота)
    struct ConsoleSize {
        int width;
        int height;
    };

    ConsoleSize getConsoleSize();

    // Установка размера консоли
    bool setConsoleSize(int width, int height);

    // Получение позиции курсора
    struct CursorPosition {
        int x;
        int y;
    };

    CursorPosition getCursorPosition();

    // Установка позиции курсора
    bool setCursorPosition(int x, int y);

    // Очистка консоли
    void clearConsole();

    // Очистка текущей строки
    void clearLine();

    // Скрыть/показать курсор
    void showCursor(bool show);

    // ==================== Работа с кодировками ====================

    // Преобразование UTF-8 в UTF-16 (Windows native)
    std::wstring utf8ToWide(const std::string& utf8);

    // Преобразование UTF-16 в UTF-8
    std::string wideToUtf8(const std::wstring& wide);

    // Определение кодировки файла
    enum class Encoding {
        Unknown,
        UTF8,
        UTF8_BOM,
        UTF16_LE,
        UTF16_BE,
        CP1251,    // Windows Cyrillic
        CP866,     // DOS Cyrillic
        ISO8859_5  // Latin/Cyrillic
    };

    Encoding detectFileEncoding(const std::string& path);

    // Конвертация между кодировками
    std::string convertEncoding(const std::string& text,
        Encoding from,
        Encoding to = Encoding::UTF8);

    // ==================== Системная информация ====================

    // Получение имени пользователя
    std::string getUsername();

    // Получение имени компьютера
    std::string getComputerName();

    // Получение версии Windows
    std::string getWindowsVersion();

    // Проверка прав администратора
    bool isRunningAsAdmin();

    // Получение доступной памяти в байтах
    long long getAvailableMemory();

    // Получение общего объема памяти в байтах
    long long getTotalMemory();

    // ==================== Работа с процессами ====================

    // Запуск внешней программы
    struct ProcessResult {
        int exitCode;
        std::string output;
        std::string error;
    };

    ProcessResult executeCommand(const std::string& command,
        const std::string& workingDir = "",
        bool waitForCompletion = true);

    // Открытие файла в ассоциированной программе
    bool openFileWithDefaultApp(const std::string& path);

    // Открытие URL в браузере по умолчанию
    bool openUrlInBrowser(const std::string& url);

    // ==================== Утилиты для времени ====================

    // Высокоточный таймер (для бенчмаркинга)
    class HighResolutionTimer {
    private:
        long long startTime;
        long long frequency;

    public:
        HighResolutionTimer();
        void start();
        double elapsedMilliseconds() const;
        double elapsedMicroseconds() const;
        double elapsedSeconds() const;
    };

    // Получение текущего времени в высоком разрешении
    long long getCurrentTimestamp();

    // Форматирование времени для отображения
    std::string formatTime(long long timestamp,
        const std::string& format = "%Y-%m-%d %H:%M:%S");

    // ==================== Обработка ошибок Windows ====================

    // Получение текстового описания ошибки Windows
    std::string getLastErrorString();

    // Класс для Windows исключений
    class WindowsException : public std::runtime_error {
    private:
        unsigned long errorCode;

    public:
        WindowsException(const std::string& message, unsigned long code = 0);
        unsigned long getErrorCode() const { return errorCode; }
        std::string getErrorDescription() const;
    };
};

#endif // WINDOWS_UTILS_H