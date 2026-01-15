#include "windows_utils.h"
#include <windows.h>
#include <shlobj.h>
#include <psapi.h>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <direct.h>
#include <io.h>

using namespace std;

namespace WindowsUtils {

    // Проверка существования файла
    bool fileExists(const string& path) {
        DWORD attrs = GetFileAttributesA(path.c_str());
        return (attrs != INVALID_FILE_ATTRIBUTES && !(attrs & FILE_ATTRIBUTE_DIRECTORY));
    }

    // Проверка существования директории
    bool directoryExists(const string& path) {
        DWORD attrs = GetFileAttributesA(path.c_str());
        return (attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY));
    }

    // Создание директории
    bool createDirectory(const string& path) {
        // Рекурсивное создание директорий
        for (size_t i = 0; i < path.length(); i++) {
            if (path[i] == '\\' || path[i] == '/') {
                string subdir = path.substr(0, i);
                if (!subdir.empty() && !directoryExists(subdir)) {
                    if (!CreateDirectoryA(subdir.c_str(), NULL)) {
                        if (GetLastError() != ERROR_ALREADY_EXISTS) {
                            return false;
                        }
                    }
                }
            }
        }
        return CreateDirectoryA(path.c_str(), NULL) != 0 ||
            GetLastError() == ERROR_ALREADY_EXISTS;
    }

    // Получение размера файла
    long long getFileSize(const string& path) {
        WIN32_FILE_ATTRIBUTE_DATA fileInfo;
        if (GetFileAttributesExA(path.c_str(), GetFileExInfoStandard, &fileInfo)) {
            LARGE_INTEGER size;
            size.HighPart = fileInfo.nFileSizeHigh;
            size.LowPart = fileInfo.nFileSizeLow;
            return size.QuadPart;
        }
        return -1;
    }

    // Чтение файла в строку
    string readFileToString(const string& path) {
        ifstream file(path, ios::binary | ios::ate);
        if (!file.is_open()) {
            throw WindowsException("Не удалось открыть файл: " + path);
        }

        streamsize size = file.tellg();
        file.seekg(0, ios::beg);

        vector<char> buffer(size);
        if (!file.read(buffer.data(), size)) {
            throw WindowsException("Ошибка чтения файла: " + path);
        }

        return string(buffer.data(), size);
    }

    // Запись строки в файл
    bool writeStringToFile(const string& path, const string& content) {
        ofstream file(path, ios::binary);
        if (!file.is_open()) {
            return false;
        }

        file.write(content.c_str(), content.size());
        return file.good();
    }

    // Получение текущей директории
    string getCurrentDirectory() {
        char buffer[MAX_PATH];
        if (GetCurrentDirectoryA(MAX_PATH, buffer)) {
            return string(buffer);
        }
        return "";
    }

    // Установка текущей директории
    bool setCurrentDirectory(const string& path) {
        return SetCurrentDirectoryA(path.c_str()) != 0;
    }

    // Получение абсолютного пути
    string getAbsolutePath(const string& relativePath) {
        char absolutePath[MAX_PATH];
        if (_fullpath(absolutePath, relativePath.c_str(), MAX_PATH)) {
            return string(absolutePath);
        }
        return relativePath;
    }

    // Разделение пути на компоненты
    PathComponents splitPath(const string& path) {
        PathComponents components;

        // Разделение диска
        size_t colonPos = path.find(':');
        if (colonPos != string::npos && colonPos < 2) {
            components.drive = path.substr(0, colonPos + 1);
        }

        // Поиск последнего разделителя
        size_t lastSlash = path.find_last_of("/\\");
        if (lastSlash != string::npos) {
            components.directory = path.substr(0, lastSlash + 1);
            string filename = path.substr(lastSlash + 1);

            // Разделение имени файла и расширения
            size_t dotPos = filename.find_last_of('.');
            if (dotPos != string::npos) {
                components.filename = filename.substr(0, dotPos);
                components.extension = filename.substr(dotPos);
            }
            else {
                components.filename = filename;
            }
        }
        else {
            // Нет пути, только имя файла
            components.directory = "";
            components.filename = path;
        }

        return components;
    }

    // Объединение пути
    string joinPath(const string& dir, const string& filename) {
        if (dir.empty()) return filename;
        if (filename.empty()) return dir;

        char lastChar = dir.back();
        if (lastChar == '\\' || lastChar == '/') {
            return dir + filename;
        }
        else {
            return dir + "\\" + filename;
        }
    }

    // Настройка консоли
    bool setupConsole() {
        // Устанавливаем UTF-8 кодировку
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);

        // Включаем поддержку виртуального терминала для цветов
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD dwMode = 0;
        GetConsoleMode(hOut, &dwMode);
        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        return SetConsoleMode(hOut, dwMode) != 0;
    }

    // Восстановление консоли
    void restoreConsole() {
        // Возвращаем стандартную кодировку (Windows-1251 для русской Windows)
        SetConsoleOutputCP(866); // DOS Cyrillic для совместимости
        SetConsoleCP(866);
    }

    // Получение размеров консоли
    ConsoleSize getConsoleSize() {
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        ConsoleSize size = { 80, 25 }; // значения по умолчанию

        if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
            size.width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
            size.height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
        }

        return size;
    }

    // Преобразование UTF-8 в UTF-16
    wstring utf8ToWide(const string& utf8) {
        if (utf8.empty()) return wstring();

        int size_needed = MultiByteToWideChar(CP_UTF8, 0,
            utf8.c_str(), (int)utf8.size(), NULL, 0);
        wstring wstr(size_needed, 0);
        MultiByteToWideChar(CP_UTF8, 0,
            utf8.c_str(), (int)utf8.size(), &wstr[0], size_needed);
        return wstr;
    }

    // Преобразование UTF-16 в UTF-8
    string wideToUtf8(const wstring& wide) {
        if (wide.empty()) return string();

        int size_needed = WideCharToMultiByte(CP_UTF8, 0,
            wide.c_str(), (int)wide.size(), NULL, 0, NULL, NULL);
        string str(size_needed, 0);
        WideCharToMultiByte(CP_UTF8, 0,
            wide.c_str(), (int)wide.size(), &str[0], size_needed, NULL, NULL);
        return str;
    }

    // Получение текста ошибки Windows
    string getLastErrorString() {
        DWORD error = GetLastError();
        if (error == 0) {
            return "Нет ошибки";
        }

        LPSTR messageBuffer = nullptr;
        size_t size = FormatMessageA(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPSTR)&messageBuffer, 0, NULL);

        string message(messageBuffer, size);
        LocalFree(messageBuffer);

        // Удаляем лишние пробелы и символы новой строки
        while (!message.empty() && (message.back() == '\n' || message.back() == '\r' || message.back() == ' ')) {
            message.pop_back();
        }

        return "Код ошибки: " + to_string(error) + " - " + message;
    }

    // Конструктор WindowsException
    WindowsException::WindowsException(const string& message, unsigned long code)
        : runtime_error(message + " (" + getLastErrorString() + ")"), errorCode(code) {
    }

    // Получение описания ошибки
    string WindowsException::getErrorDescription() const {
        return getLastErrorString();
    }

    // Конструктор HighResolutionTimer
    HighResolutionTimer::HighResolutionTimer() {
        LARGE_INTEGER freq;
        QueryPerformanceFrequency(&freq);
        frequency = freq.QuadPart;
        startTime = 0;
    }

    // Запуск таймера
    void HighResolutionTimer::start() {
        LARGE_INTEGER start;
        QueryPerformanceCounter(&start);
        startTime = start.QuadPart;
    }

    // Прошедшее время в миллисекундах
    double HighResolutionTimer::elapsedMilliseconds() const {
        LARGE_INTEGER now;
        QueryPerformanceCounter(&now);
        return ((now.QuadPart - startTime) * 1000.0) / frequency;
    }

    // Прошедшее время в микросекундах
    double HighResolutionTimer::elapsedMicroseconds() const {
        LARGE_INTEGER now;
        QueryPerformanceCounter(&now);
        return ((now.QuadPart - startTime) * 1000000.0) / frequency;
    }

    // Прошедшее время в секундах
    double HighResolutionTimer::elapsedSeconds() const {
        LARGE_INTEGER now;
        QueryPerformanceCounter(&now);
        return static_cast<double>(now.QuadPart - startTime) / frequency;
    }

    // Получение текущего времени в высоком разрешении
    long long getCurrentTimestamp() {
        LARGE_INTEGER time;
        QueryPerformanceCounter(&time);
        return time.QuadPart;
    }
};