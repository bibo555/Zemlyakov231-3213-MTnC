#pragma once

#include <string>
#include <vector>
#include <unordered_set>

using namespace std;

// Типы токенов
enum class TokenType
{
    KEYWORD,
    IDENTIFIER,
    CONSTANT_INT,
    CONSTANT_FLOAT,
    CONSTANT_STRING,
    CONSTANT_BOOL,
    OPERATOR,
    DELIMITER,
    PREPROCESSOR,
    UNKNOWN
};

// Преобразует тип токена в строку для вывода
string tokenTypeName(TokenType t);

// Структура токена
struct Token
{
    TokenType type;
    string value;
    int line;
};

// Структура ошибки
struct LexError
{
    string message;
    int line;
};

// Лексер
class Lexer
{
public:
    explicit Lexer(const string& src);

    void tokenize();

    const vector<Token>& getTokens() const;
    const vector<LexError>& getErrors() const;

private:
    string src;
    size_t pos;
    int line;

    vector<Token> tokens;
    vector<LexError> errors;

    void readPreprocessor();
    void readString();
    void readChar();
    void readNumber();
    void readIdent();
    bool tryReadOp2();
};

// Функции вывода
void printTable(const vector<Token>& tokens);
void printSequence(const vector<Token>& tokens);
void printErrors(const vector<LexError>& errors);
#pragma once
