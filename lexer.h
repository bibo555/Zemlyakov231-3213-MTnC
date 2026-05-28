#pragma once

#include <string>
#include <vector>
#include <unordered_set>

enum class TokenType
{
    IDENTIFIER,
    KEYWORD,

    OPERATOR,
    DELIMITER,
    PREPROCESSOR,

    CONSTANT_INT,
    CONSTANT_FLOAT,
    CONSTANT_STRING,
    CONSTANT_BOOL,

    END
};

struct Token
{
    TokenType type;
    std::string value;
    int line;

    Token(TokenType t, const std::string& v, int ln)
        : type(t), value(v), line(ln) {}
};

class Lexer
{
public:
    Lexer(const std::string& input);

    std::vector<Token> tokenize();

private:
    std::string input;
    size_t pos = 0;
    int line = 1;

    char peek(int offset = 0) const;
    char get();

    void skipWhitespace();

    Token identifier();
    Token number();
    Token string();
    Token preprocessor();

    Token opOrDelimiter();

    bool isAtEnd() const;
};