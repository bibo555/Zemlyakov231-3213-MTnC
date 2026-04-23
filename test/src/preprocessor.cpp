#include <string>
#include <regex>
#include <sstream>
#include <iostream>
#include <algorithm>

std::string preprocessCode(const std::string& code)
{
    std::string result = code;
    bool hasError = false;

    //  Проверка комментариев (/* */)
    bool insideComment = false;

    for (size_t i = 0; i + 1 < result.size(); i++)
    {
        if (result[i] == '/' && result[i + 1] == '*')
        {
            if (insideComment)
            {
                std::cerr << "Ошибка: вложенные комментарии запрещены!\n";
                hasError = true;
            }
            insideComment = true;
        }

        if (result[i] == '*' && result[i + 1] == '/')
        {
            if (!insideComment)
            {
                std::cerr << "Ошибка: найдено закрытие комментария без открытия!\n";
                hasError = true;
            }
            insideComment = false;
        }
    }

    if (insideComment)
    {
        std::cerr << "Ошибка: незакрытый многострочный комментарий!\n";
        hasError = true;
    }

    //  Удаление комментариев
    result = std::regex_replace(result, std::regex("//.*"), "");
    result = std::regex_replace(result, std::regex("/\\*[\\s\\S]*?\\*/"), "");

    // Очистка пробелов и пустых строк
    std::stringstream input(result);
    std::stringstream output;
    std::string line;

    while (std::getline(input, line))
    {
        // удалить пробелы в начале и конце строки
        line = std::regex_replace(line, std::regex("^\\s+|\\s+$"), "");

        if (!line.empty())
        {
            output << line << "\n";
        }
    }

    result = output.str();

    //  Проверка недопустимых символов
    std::regex invalidChars("[^a-zA-Z0-9_\\s\\{\\}\\(\\);=+\\-*/.,<>!&|\\[\\]#\"]");

    if (std::regex_search(result, invalidChars))
    {
        std::cerr << "Ошибка: обнаружены недопустимые символы!\n";
        hasError = true;
    }

    //  Проверка незакрытых строк
    auto quoteCount = std::count(result.begin(), result.end(), '"');
    if (quoteCount % 2 != 0)
    {
        std::cerr << "Ошибка: незакрытая строка!\n";
        hasError = true;
    }

    //  Проверка пустого результата
    if (result.empty())
    {
        std::cerr << "Предупреждение: файл пуст после обработки!\n";
    }

    //  Сообщение об успехе
    if (!hasError)
    {
        std::cout << "Ошибок не выявлено\n";
    }

    return result;
}