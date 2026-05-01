#include "lexer.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <cctype>

using namespace std;

static const unordered_set<string> ALL_TYPES =
{
    "int", "float", "double", "long", "short", "unsigned",
    "string", "bool", "char", "void"
};

// Таблицы
// Ключевые слова
static const unordered_set<string> KEYWORDS =
{
    "int", "bool", "char", "string", "void",
    "return", "if", "else", "for", "do", "while",
    "continue", "using", "namespace", "const",
    "true", "false", "include", "endl", "LC_ALL", "float"
};

// Булевы константы
static const unordered_set<string> BOOL_CONSTANTS = { "true", "false" };

// Двухсимвольные операторы
static const vector<string> OPERATORS2 =
{
    "+=", "++", "<=", ">=", "==", "!=",
    "&&", "||", "<<", ">>", "::"
};

// Односимвольные операторы
static const unordered_set<char> OPERATORS1 =
{
    '=', '+', '%', '<', '>', '!', '&', '|', '-', '*', '/'
};

// Разделители
static const unordered_set<char> DELIMITERS =
{
    '(', ')', '{', '}', '[', ']', ';', ',', '.'
};

// Идентификаторы
static const unordered_set<string> VALID_IDENTIFIERS =
{
    "generatePassword", "showPasswordStrength", "main",
    "length", "useUpper", "useDigits", "useSymbols",
    "password", "lowerChars", "upperChars", "digitChars", "symbolChars",
    "availableChars", "charSetSize", "randomIndex",
    "hasUpper", "hasDigit", "hasSymbol", "strengthScore",
    "passwordLength", "useUpperInput", "useDigitsInput", "useSymbolsInput",
    "i", "c",
    "cout", "cin", "setlocale", "srand", "rand", "time", "std", "clear", "ignore",
    "string", "vector", "a"
};

string tokenTypeName(TokenType t)
{
    switch (t)
    {
    case TokenType::KEYWORD:         return "KEYWORD";
    case TokenType::IDENTIFIER:      return "IDENTIFIER";
    case TokenType::CONSTANT_INT:    return "CONSTANT_INT";
    case TokenType::CONSTANT_FLOAT:  return "CONSTANT_FLOAT";
    case TokenType::CONSTANT_STRING: return "CONSTANT_STRING";
    case TokenType::CONSTANT_BOOL:   return "CONSTANT_BOOL";
    case TokenType::OPERATOR:        return "OPERATOR";
    case TokenType::DELIMITER:       return "DELIMITER";
    case TokenType::PREPROCESSOR:    return "PREPROCESSOR";
    default:                         return "UNKNOWN";
    }
}

// Лексический анализатор

Lexer::Lexer(const string& src) : src(src), pos(0), line(1) {}

const vector<Token>& Lexer::getTokens() const { return tokens; }
const vector<LexError>& Lexer::getErrors() const { return errors; }

void Lexer::tokenize()
{
    while (pos < src.size())
    {
        char c = src[pos];

        // 1. Пропуск однострочных комментариев //
        if (c == '/' && pos + 1 < src.size() && src[pos + 1] == '/')
        {
            while (pos < src.size() && src[pos] != '\n')
            {
                pos++;
            }
            continue;
        }

        // 2. Обработка переноса строки
        if (c == '\n')
        {
            line++;
            pos++;
            continue;
        }

        // 3. Пропуск пробельных символов
        if (isspace((unsigned char)c))
        {
            pos++;
            continue;
        }

        // 4. Директивы препроцессора (#include и т.д.)
        if (c == '#')
        {
            readPreprocessor();
            continue;
        }

        // 5. Строковые литералы ("...")
        if (c == '"')
        {
            readString();
            continue;
        }

        // 6. Символьные литералы ('a')
        if (c == '\'')
        {
            readChar();
            continue;
        }

        // 7. Числа (целые и дробные)
        if (isdigit((unsigned char)c))
        {
            readNumber();
            continue;
        }

        // 8. Идентификаторы и ключевые слова
        if (isalpha((unsigned char)c) || c == '_')
        {
            readIdent();
            continue;
        }

        // 9. Обработка запятой (с проверкой на ошибку в дробных числах)
        if (c == ',')
        {
            if (!tokens.empty() &&
                (tokens.back().type == TokenType::CONSTANT_INT ||
                    tokens.back().type == TokenType::CONSTANT_FLOAT))
            {
                size_t look = pos + 1;
                while (look < src.size() && isspace((unsigned char)src[look])) look++;
                if (look < src.size() && isdigit((unsigned char)src[look]))
                {
                    errors.push_back({ "Error: Comma used instead of dot in float", line });
                    pos++;
                    continue;
                }
            }
            tokens.push_back({ TokenType::DELIMITER, string(1, c), line });
            pos++;
            continue;
        }

        // 10. Двухсимвольные операторы (==, <=, << и т.д.)
        if (tryReadOp2())
            continue;

        // 11. Односимвольные операторы
        if (OPERATORS1.count(c))
        {
            tokens.push_back({ TokenType::OPERATOR, string(1, c), line });
            pos++;
            continue;
        }

        // 12. Разделители
        if (DELIMITERS.count(c))
        {
            tokens.push_back({ TokenType::DELIMITER, string(1, c), line });
            pos++;
            continue;
        }

        // 13. Если символ не распознан
        string unknownChar = "";
        unknownChar += c;
        errors.push_back({ "Invalid character: " + unknownChar, line });
        pos++;
    }
}

void Lexer::readPreprocessor()
{
    size_t start = pos;
    pos++;

    while (pos < src.size() && src[pos] == ' ') pos++;
    size_t wordStart = pos;

    while (pos < src.size() && (isalnum((unsigned char)src[pos]) || src[pos] == '_'))
        pos++;

    string dirWord = src.substr(wordStart, pos - wordStart);

    while (pos < src.size() && src[pos] != '\n') pos++;
    string directive = src.substr(start, pos - start);

    if (!KEYWORDS.count(dirWord))
    {
        errors.push_back({ "Недопустимая директива препроцессора: " + directive, line });
        return;
    }

    tokens.push_back({ TokenType::PREPROCESSOR, directive, line });
}

void Lexer::readString()
{
    int startLine = line;
    pos++;
    string val = "\"";
    bool closed = false;

    while (pos < src.size())
    {
        char c = src[pos];
        if (c == '\\' && pos + 1 < src.size())
        {
            val += c;
            val += src[pos + 1];
            pos += 2;
            continue;
        }
        if (c == '"')
        {
            val += '"';
            pos++;
            closed = true;
            break;
        }
        if (c == '\n')
        {
            errors.push_back({ "Незакрытый строковый литерал: " + val, startLine });
            line++;
            pos++;
            return;
        }
        val += c;
        pos++;
    }

    if (!closed)
    {
        errors.push_back({ "Незакрытый строковый литерал: " + val, startLine });
        return;
    }

    tokens.push_back({ TokenType::CONSTANT_STRING, val, startLine });
}

void Lexer::readChar()
{
    int startLine = line;
    pos++;
    string val = "'";

    while (pos < src.size() && src[pos] != '\'')
    {
        if (src[pos] == '\\' && pos + 1 < src.size())
        {
            val += src[pos];
            val += src[pos + 1];
            pos += 2;
            continue;
        }
        if (src[pos] == '\n')
        {
            errors.push_back({ "Незакрытый символьный литерал: " + val, startLine });
            line++;
            pos++;
            return;
        }
        val += src[pos++];
    }

    if (pos >= src.size())
    {
        errors.push_back({ "Незакрытый символьный литерал: " + val, startLine });
        return;
    }

    val += '\'';
    pos++;
    tokens.push_back({ TokenType::CONSTANT_INT, val, startLine });
}

static bool prevTokenIsType(const vector<Token>& tokens)
{
    for (int i = (int)tokens.size() - 1; i >= 0; i--)
    {
        if (tokens[i].type == TokenType::PREPROCESSOR) continue;
        return tokens[i].type == TokenType::KEYWORD
            && ALL_TYPES.count(tokens[i].value);
    }
    return false;
}

void Lexer::readNumber()
{
    int startLine = line;
    size_t start = pos;
    bool hasDecimal = false;
    bool hasError = false;
    bool hasLetters = false;

    while (pos < src.size())
    {
        char c = src[pos];

        if (isdigit((unsigned char)c))
        {
            pos++;
            continue;
        }

        if (c == '.')
        {
            if (hasDecimal)
            {
                errors.push_back({ "Incorrect number : double dot", startLine });
                hasError = true;
            }
            hasDecimal = true;
            pos++;
            continue;
        }

        if (isalpha((unsigned char)c) || c == '_')
        {
            hasLetters = true;
            hasError = true;
            pos++;
            continue;
        }

        break;
    }

    if (hasLetters)
    {
        string val = src.substr(start, pos - start);

        if (prevTokenIsType(tokens))
            errors.push_back({
                "Invalid identifier: " + val + " — Identifier cannot start with a digit",
                startLine });
        else
            errors.push_back({
                "Incorrect number: \"" + val + "\" with letters",
                startLine });
        return;
    }

    if (!hasError)
    {
        string val = src.substr(start, pos - start);
        TokenType t = hasDecimal ? TokenType::CONSTANT_FLOAT : TokenType::CONSTANT_INT;
        tokens.push_back({ t, val, startLine });
    }
}

void Lexer::readIdent()
{
    int startLine = line;
    size_t start = pos;
    while (pos < src.size() && (isalnum((unsigned char)src[pos]) || src[pos] == '_'))
        pos++;
    string val = src.substr(start, pos - start);

    if (BOOL_CONSTANTS.count(val))
    {
        tokens.push_back({ TokenType::CONSTANT_BOOL, val, startLine });
    }
    else if (KEYWORDS.count(val))
    {
        tokens.push_back({ TokenType::KEYWORD, val, startLine });
    }
    else if (VALID_IDENTIFIERS.count(val))
    {
        tokens.push_back({ TokenType::IDENTIFIER, val, startLine });
    }
    else
    {
        errors.push_back({
            "Недопустимый идентификатор: " + val,
            startLine });
    }
}

bool Lexer::tryReadOp2()
{
    if (pos + 1 >= src.size())
        return false;
    string two = src.substr(pos, 2);
    for (const string& op : OPERATORS2)
    {
        if (two == op)
        {
            tokens.push_back({ TokenType::OPERATOR, op, line });
            pos += 2;
            return true;
        }
    }
    return false;
}
// Функции вывода

static int visibleWidth(const string& s)
{
    int w = 0;
    for (unsigned char ch : s)
        if ((ch & 0xC0) != 0x80)
            w++;
    return w;
}

static string padRight(const string& s, int width)
{
    int pad = width - visibleWidth(s);
    return s + string(max(pad, 0), ' ');
}



void printTable(const vector<Token>& tokens) {
    cout << "\n=== TABLE ===\n"; 
    for (const auto& tok : tokens) {
       
        cout << tok.value << "\t | " << tokenTypeName(tok.type) << endl;
    }
}

void printSequence(const vector<Token>& tokens)
{
    cout << "\n--- ПОСЛЕДОВАТЕЛЬНОСТЬ ---\n[";
    for (size_t i = 0; i < tokens.size(); i++)
    {
        cout << "(" << tokenTypeName(tokens[i].type) << ", " << tokens[i].value << ")";
        if (i + 1 < tokens.size()) cout << ", ";
    }
    cout << "]\n";
}

void printErrors(const vector<LexError>& errors) {
    if (errors.empty()) return;
    cout << "\n!!! ERRORS !!!\n";
    for (const auto& e : errors) {
        
        cout << "Line " << e.line << ": " << e.message << endl;
    }
}