#pragma execution_character_set("utf-8")
#include "lexer.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <windows.h>
#include <clocale>
int main() {
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
    setlocale(LC_ALL, "ru_RU.UTF-8");
    // Название файла
    std::string filename = "test.cpp";
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Не удалось открыть файл: " << filename << std::endl;
        return 1;
    }

    // Читаем весь файл в одну строку
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string sourceCode = buffer.str();
    file.close();

    // Далее ваш существующий код
    Lexer lexer(sourceCode);
    lexer.tokenize();

    const auto& tokens = lexer.getTokens();
    const auto& errors = lexer.getErrors();

    if (!tokens.empty()) {
        std::cout << "--- Таблица лексем ---" << std::endl;
        printTable(tokens);
        std::cout << "\n--- Последовательность ---" << std::endl;
        printSequence(tokens);
    }

    if (!errors.empty()) {
        std::cout << "\n--- Ошибки ---" << std::endl;
        printErrors(errors);
    }
    else {
        std::cout << "\nЛексический анализ завершен успешно." << std::endl;
    }

    return 0;
}