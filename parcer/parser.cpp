#include "parser.h"
#include <iostream>
#include <initializer_list>
#include <stdexcept>

using namespace std;

string nodeTypeName(NodeType t)
{
    switch (t)
    {
    case NodeType::PROGRAM:           return "Program";
    case NodeType::FUNCTION_DECL:     return "FunctionDecl";
    case NodeType::PARAM_DECL:        return "ParamDecl";
    case NodeType::BLOCK_STMT:        return "BlockStmt";
    case NodeType::VAR_DECL_STMT:     return "VarDeclStmt";
    case NodeType::ASSIGN_STMT:       return "AssignStmt";
    case NodeType::IF_STMT:           return "IfStmt";
    case NodeType::WHILE_STMT:        return "WhileStmt";
    case NodeType::FOR_STMT:          return "ForStmt";
    case NodeType::RETURN_STMT:       return "ReturnStmt";
    case NodeType::EXPR_STMT:         return "ExprStmt";
    case NodeType::IDENTIFIER_EXPR:   return "IdentifierExpr";
    case NodeType::LITERAL_EXPR:      return "LiteralExpr";
    case NodeType::BINARY_EXPR:       return "BinaryExpr";
    case NodeType::UNARY_EXPR:        return "UnaryExpr";
    case NodeType::CALL_EXPR:         return "CallExpr";
    case NodeType::MEMBER_CALL_EXPR:  return "MemberCallExpr";
    case NodeType::ARRAY_ACCESS_EXPR: return "ArrayAccessExpr";
    default:                          return "Unknown";
    }
}


// Построение AST
void IdentifierExpr::print(const string& prefix, bool isLast) const
{
    cout << prefix << (isLast ? "'-- " : "+-- ")
        << "IdentifierExpr: " << name << "\n";
}

void LiteralExpr::print(const string& prefix, bool isLast) const
{
    cout << prefix << (isLast ? "'-- " : "+-- ")
        << "LiteralExpr: " << value << "\n";
}

void BinaryExpr::print(const string& prefix, bool isLast) const
{
    cout << prefix << (isLast ? "'-- " : "+-- ")
        << "BinaryExpr [" << op << "]\n";
    string p = childPrefix(prefix, isLast);
    cout << p << "+-- left:\n";
    left->print(p + "|   ", true);
    cout << p << "'-- right:\n";
    right->print(p + "    ", true);
}

void UnaryExpr::print(const string& prefix, bool isLast) const
{
    cout << prefix << (isLast ? "'-- " : "+-- ")
        << "UnaryExpr [" << op << "]\n";
    string p = childPrefix(prefix, isLast);
    operand->print(p, true);
}

void CallExpr::print(const string& prefix, bool isLast) const
{
    cout << prefix << (isLast ? "'-- " : "+-- ")
        << "CallExpr: " << callee << "\n";
    string p = childPrefix(prefix, isLast);
    for (size_t i = 0; i < arguments.size(); i++)
    {
        bool last = (i + 1 == arguments.size());
        cout << p << (last ? "'-- " : "+-- ") << "arg[" << i << "]:\n";
        arguments[i]->print(childPrefix(p, last), true);
    }
}

void MemberCallExpr::print(const string& prefix, bool isLast) const
{
    cout << prefix << (isLast ? "'-- " : "+-- ")
        << "MemberCallExpr: ." << method << "\n";
    string p = childPrefix(prefix, isLast);
    cout << p << "+-- object:\n";
    object->print(p + "|   ", true);
    for (size_t i = 0; i < arguments.size(); i++)
    {
        bool last = (i + 1 == arguments.size());
        cout << p << (last ? "'-- " : "+-- ") << "arg[" << i << "]:\n";
        arguments[i]->print(childPrefix(p, last), true);
    }
}

void ArrayAccessExpr::print(const string& prefix, bool isLast) const
{
    cout << prefix << (isLast ? "'-- " : "+-- ")
        << "ArrayAccessExpr\n";
    string p = childPrefix(prefix, isLast);
    cout << p << "+-- array:\n";
    array->print(p + "|   ", true);
    cout << p << "'-- index:\n";
    index->print(p + "    ", true);
}

void BlockStmt::print(const string& prefix, bool isLast) const
{
    cout << prefix << (isLast ? "'-- " : "+-- ") << "BlockStmt\n";
    string p = childPrefix(prefix, isLast);
    for (size_t i = 0; i < statements.size(); i++)
        statements[i]->print(p, i + 1 == statements.size());
}

void VarDeclStmt::print(const string& prefix, bool isLast) const
{
    cout << prefix << (isLast ? "'-- " : "+-- ")
        << "VarDeclStmt\n";
    string p = childPrefix(prefix, isLast);
    bool hasInit = (initExpr != nullptr);
    cout << p << "+-- type: " << varType << "\n";
    cout << p << (hasInit ? "+-- " : "'-- ") << "name: " << name << "\n";
    if (hasInit)
    {
        cout << p << "'-- initExpr:\n";
        initExpr->print(p + "    ", true);
    }
}

void AssignStmt::print(const string& prefix, bool isLast) const
{
    cout << prefix << (isLast ? "'-- " : "+-- ")
        << "AssignStmt [" << op << "]\n";
    string p = childPrefix(prefix, isLast);
    cout << p << "+-- left:\n";
    left->print(p + "|   ", true);
    cout << p << "'-- right:\n";
    right->print(p + "    ", true);
}

void IfStmt::print(const string& prefix, bool isLast) const
{
    cout << prefix << (isLast ? "'-- " : "+-- ") << "IfStmt\n";
    string p = childPrefix(prefix, isLast);
    bool hasElse = (elseBranch != nullptr);
    cout << p << "+-- condition:\n";
    condition->print(p + "|   ", true);
    cout << p << (hasElse ? "+-- " : "'-- ") << "thenBranch:\n";
    thenBranch->print(childPrefix(p, !hasElse), true);
    if (hasElse)
    {
        cout << p << "'-- elseBranch:\n";
        elseBranch->print(p + "    ", true);
    }
}

void WhileStmt::print(const string& prefix, bool isLast) const
{
    cout << prefix << (isLast ? "'-- " : "+-- ") << "WhileStmt\n";
    string p = childPrefix(prefix, isLast);
    cout << p << "+-- condition:\n";
    condition->print(p + "|   ", true);
    cout << p << "'-- body:\n";
    body->print(p + "    ", true);
}

void ForStmt::print(const string& prefix, bool isLast) const
{
    cout << prefix << (isLast ? "'-- " : "+-- ") << "ForStmt\n";
    string p = childPrefix(prefix, isLast);
    cout << p << "+-- init:\n";
    if (init) init->print(p + "|   ", true);
    else cout << p << "|   '-- (empty)\n";
    cout << p << "+-- condition:\n";
    if (condition) condition->print(p + "|   ", true);
    else cout << p << "|   '-- (empty)\n";
    cout << p << "+-- update:\n";
    if (update) update->print(p + "|   ", true);
    else cout << p << "|   '-- (empty)\n";
    cout << p << "'-- body:\n";
    body->print(p + "    ", true);
}

void ReturnStmt::print(const string& prefix, bool isLast) const
{
    cout << prefix << (isLast ? "'-- " : "+-- ") << "ReturnStmt\n";
    string p = childPrefix(prefix, isLast);
    if (value)
        value->print(p, true);
    else
        cout << p << "'-- (void)\n";
}

void ExprStmt::print(const string& prefix, bool isLast) const
{
    cout << prefix << (isLast ? "'-- " : "+-- ") << "ExprStmt\n";
    string p = childPrefix(prefix, isLast);
    expression->print(p, true);
}

void ParamDecl::print(const string& prefix, bool isLast) const
{
    cout << prefix << (isLast ? "'-- " : "+-- ")
        << "ParamDecl: " << paramType << " " << name << "\n";
}

void FunctionDecl::print(const string& prefix, bool isLast) const
{
    cout << prefix << (isLast ? "'-- " : "+-- ")
        << "FunctionDecl: " << returnType << " " << name << "\n";
    string p = childPrefix(prefix, isLast);
    if (!parameters.empty())
    {
        cout << p << "+-- parameters\n";
        string pp = p + "|   ";
        for (size_t i = 0; i < parameters.size(); i++)
            parameters[i]->print(pp, i + 1 == parameters.size());
    }
    cout << p << "'-- body:\n";
    body->print(p + "    ", true);
}

void ProgramNode::print(const string& prefix, bool isLast) const
{
    cout << "Program\n";
    for (size_t i = 0; i < functions.size(); i++)
        functions[i]->print("", i + 1 == functions.size());
}

// Реализация методов парсера
Parser::Parser(const vector<Token>& tokens) : tokens(tokens), pos(0) {}

static const Token EOF_TOKEN{ TokenType::DELIMITER, "$EOF$", -1 };

const Token& Parser::cur() const // просмотр текущего токена
{
    return pos < tokens.size() ? tokens[pos] : EOF_TOKEN;
}
const Token& Parser::peek(int off) const // просмотр токена впереда на off
{
    size_t idx = pos + off;
    return idx < tokens.size() ? tokens[idx] : EOF_TOKEN;
}
bool Parser::atEnd() const { return pos >= tokens.size(); } // проверка, что токены закончились
Token Parser::consume() { Token t = cur(); if (!atEnd()) ++pos; return t; } // возвращает текущий токен и сдвигает индекс

bool Parser::match(TokenType t, const string& val) const // проверка типа и значения токена
{
    if (cur().type != t) return false;
    return val.empty() || cur().value == val;
}

Token Parser::expect(TokenType t, const string& val) // match() + consume() обязательного токена
{
    if (match(t, val)) return consume();
    string expected = val.empty() ? tokenTypeName(t) : ("'" + val + "'");
    string got = "'" + cur().value + "' (" + tokenTypeName(cur().type) + ")";
    addError("Ожидался " + expected + ", получен " + got, cur().line);
    return { t, val.empty() ? "" : val, cur().line };
}

static const unordered_set<string> TYPE_KEYWORDS =
{
    "int", "float", "double", "long", "short",
    "string", "bool", "char", "void", "unsigned"
};

bool Parser::isType(const string& v) const // проверка типа данных
{
    return cur().type == TokenType::KEYWORD && TYPE_KEYWORDS.count(cur().value);
}

void Parser::addError(const string& msg, int line) // добавление ошибки
{
    errors.push_back({ msg, line });
    cerr << "[Синтаксическая ошибка] Строка " << line << ": " << msg << "\n";
}

void Parser::skipTo(initializer_list<string> stopValues) // пропуск токенов
{
    while (!atEnd())
    {
        for (const string& sv : stopValues)
            if (cur().value == sv) return;
        consume();
    }
}

// Главный метод синтаксического анализатора
void Parser::parse()
{
    auto tree = parseProgram();

    cout << "\n=== AST ===\n";
    tree->print("", true);

    cout << "\n";
    if (errors.empty())
        cout << "Синтаксический анализ завершён успешно. Ошибок не найдено.\n";
    else
        cout << "Синтаксический анализ завершён. Найдено ошибок: " << errors.size() << ".\n";
}

// Методы разбора
// Разбор всей программы
NodePtr Parser::parseProgram()
{
    auto prog = make_shared<ProgramNode>(0);

    while (match(TokenType::PREPROCESSOR))
        consume();

    while (!atEnd())
    {
        if (match(TokenType::PREPROCESSOR)) { consume(); continue; }

        if (match(TokenType::KEYWORD, "using"))
        {
            while (!atEnd() && !match(TokenType::DELIMITER, ";"))
                consume();
            if (match(TokenType::DELIMITER, ";")) consume();
            continue;
        }

        // проверка того, что на верхнем уровне только функции
        if (!isType(cur().value) && !atEnd())
        {
            addError("Ожидалось объявление функции (тип возвращаемого значения), получен '" +
                cur().value + "'", cur().line);
            consume();
            continue;
        }

        prog->functions.push_back(parseFunctionDecl());
    }

    return prog;
}

// Разбор функции
NodePtr Parser::parseFunctionDecl()
{
    int ln = cur().line;

    string retType = cur().value;
    consume();

    // Имя функции
    if (!match(TokenType::IDENTIFIER))
    {
        addError("Ожидалось имя функции (идентификатор)", cur().line);
        skipTo({ "{", "}" });
    }
    string name = cur().value;

    if (match(TokenType::IDENTIFIER)) consume();

    // Обязательная скобка после имени
    expect(TokenType::DELIMITER, "(");

    // Цикл разбора параметров
    vector<NodePtr> params;
    while (!match(TokenType::DELIMITER, ")") && !atEnd())
    {
        params.push_back(parseParamDecl());
        if (match(TokenType::DELIMITER, ",")) consume();
    }
    expect(TokenType::DELIMITER, ")");

    // Тело функции
    if (!match(TokenType::DELIMITER, "{"))
    {
        addError("Ожидалось '{' -- начало тела функции '" + name + "'", cur().line);
        skipTo({ "{" });
    }
    NodePtr body = parseBlock();

    return make_shared<FunctionDecl>(retType, name, move(params), move(body), ln);
}

// Разбор параметра функции
NodePtr Parser::parseParamDecl()
{
    int ln = cur().line;
    if (!isType(cur().value))
    {
        addError("Ожидался тип параметра, получен '" + cur().value + "'", ln);
        return make_shared<ParamDecl>("?", "?", ln);
    }
    string pType = cur().value; consume();

    if (!match(TokenType::IDENTIFIER))
    {
        addError("Ожидалось имя параметра (идентификатор), получен '" +
            cur().value + "'", cur().line);
        return make_shared<ParamDecl>(pType, "?", ln);
    }
    string pName = cur().value; consume();
    return make_shared<ParamDecl>(pType, pName, ln);
}

// Разбор блока
NodePtr Parser::parseBlock()
{
    int ln = cur().line;
    auto block = make_shared<BlockStmt>(ln);

    if (!match(TokenType::DELIMITER, "{"))
    {
        addError("Ожидался '{' (начало блока), получен '" + cur().value + "'", ln);
        return block;
    }
    consume();

    while (!match(TokenType::DELIMITER, "}") && !atEnd())
        block->statements.push_back(parseStatement());

    if (!match(TokenType::DELIMITER, "}"))
        addError("Незакрытый блок: ожидался '}' (незакрытая операторная скобка)", cur().line);
    else
        consume();

    return block;
}

// Разбор оператора
NodePtr Parser::parseStatement()
{
    int ln = cur().line;

    if (match(TokenType::PREPROCESSOR)) { consume(); return parseStatement(); }

    // объявление переменной
    if (isType(cur().value))
        return parseVarDeclStmt();

    if (match(TokenType::KEYWORD, "if"))     return parseIfStmt();
    if (match(TokenType::KEYWORD, "while"))  return parseWhileStmt();
    if (match(TokenType::KEYWORD, "for"))    return parseForStmt();
    if (match(TokenType::KEYWORD, "return")) return parseReturnStmt();

    if (match(TokenType::DELIMITER, "{"))    return parseBlock();

    if (match(TokenType::DELIMITER, ";"))
    {
        consume();
        return make_shared<ExprStmt>(
            make_shared<LiteralExpr>("(empty)", ln), ln);
    }

    return parseExprStmt();
}

// Разбор объявления переменной
NodePtr Parser::parseVarDeclStmt()
{
    int ln = cur().line;
    string vType = cur().value; consume();

    if (!match(TokenType::IDENTIFIER))
    {
        addError("Ожидалось имя переменной после типа '" + vType +
            "', получен '" + cur().value + "'", cur().line);
        skipTo({ ";", "}" });
        if (match(TokenType::DELIMITER, ";")) consume();
        return make_shared<VarDeclStmt>(vType, "?", nullptr, ln);
    }
    string vName = cur().value; consume();

    NodePtr init;
    if (match(TokenType::OPERATOR, "="))
    {
        consume();
        init = parseExpr();
    }
    else if (cur().type == TokenType::OPERATOR &&
        (cur().value == "+=" || cur().value == "-="))
    {
        addError("Недопустимый оператор '" + cur().value +
            "' при объявлении переменной '" + vName + "'", cur().line);
        consume();
        init = parseExpr();
    }

    if (!match(TokenType::DELIMITER, ";"))
        addError("Ожидался ';' после объявления переменной '" + vName +
            "', получен '" + cur().value + "'", cur().line);
    else
        consume();

    return make_shared<VarDeclStmt>(vType, vName, move(init), ln);
}

// Разбор if
NodePtr Parser::parseIfStmt()
{
    int ln = cur().line;
    consume();
    expect(TokenType::DELIMITER, "(");
    NodePtr cond = parseExpr();
    expect(TokenType::DELIMITER, ")");
    NodePtr thenB = parseStatement();
    NodePtr elseB;
    if (match(TokenType::KEYWORD, "else"))
    {
        consume();
        elseB = parseStatement();
    }
    return make_shared<IfStmt>(move(cond), move(thenB), move(elseB), ln);
}

// Разбор while
NodePtr Parser::parseWhileStmt()
{
    int ln = cur().line;
    consume();
    expect(TokenType::DELIMITER, "(");
    NodePtr cond = parseExpr();
    expect(TokenType::DELIMITER, ")");
    NodePtr body = parseStatement();
    return make_shared<WhileStmt>(move(cond), move(body), ln);
}

// Разбор for
NodePtr Parser::parseForStmt()
{
    int ln = cur().line;
    consume();
    expect(TokenType::DELIMITER, "(");

    NodePtr init;
    if (!match(TokenType::DELIMITER, ";"))
        init = parseForInit();
    if (match(TokenType::DELIMITER, ";")) consume();
    else addError("Ожидался ';' после инициализации цикла for", cur().line);

    NodePtr cond;
    if (!match(TokenType::DELIMITER, ";"))
        cond = parseExpr();
    if (match(TokenType::DELIMITER, ";")) consume();
    else addError("Ожидался ';' после условия цикла for", cur().line);

    NodePtr upd;
    if (!match(TokenType::DELIMITER, ")"))
        upd = parseExpr();

    expect(TokenType::DELIMITER, ")");
    NodePtr body = parseStatement();
    return make_shared<ForStmt>(move(init), move(cond), move(upd), move(body), ln);
}

// Разбор объявления переменной в for
NodePtr Parser::parseForInit()
{
    int ln = cur().line;
    if (isType(cur().value))
    {
        string vType = cur().value; consume();
        if (!match(TokenType::IDENTIFIER))
        {
            addError("Ожидалось имя переменной в инициализации for", cur().line);
            return make_shared<VarDeclStmt>(vType, "?", nullptr, ln);
        }
        string vName = cur().value; consume();
        NodePtr init;
        if (match(TokenType::OPERATOR, "=")) { consume(); init = parseExpr(); }
        return make_shared<VarDeclStmt>(vType, vName, move(init), ln);
    }
    return parseExpr();
}

// Разбор return
NodePtr Parser::parseReturnStmt()
{
    int ln = cur().line;
    consume();
    NodePtr val;
    if (!match(TokenType::DELIMITER, ";"))
        val = parseExpr();
    if (!match(TokenType::DELIMITER, ";"))
        addError("Ожидался ';' после return", cur().line);
    else
        consume();
    return make_shared<ReturnStmt>(move(val), ln);
}

// Разбор выражения, как оператора
NodePtr Parser::parseExprStmt()
{
    int ln = cur().line;
    NodePtr expr = parseExpr();
    if (!match(TokenType::DELIMITER, ";"))
        addError("Ожидался ';' после выражения, получен '" +
            cur().value + "'", cur().line);
    else
        consume();
    return make_shared<ExprStmt>(move(expr), ln);
}

// Разбор выражения
NodePtr Parser::parseExpr() { return parseAssignExpr(); }

// Разбор выражений присваивания
NodePtr Parser::parseAssignExpr()
{
    int ln = cur().line;
    NodePtr left = parseOrExpr();

    static const unordered_set<string> ASSIGN_OPS =
    { "=", "+=", "-=", "*=", "/=", "%=" };

    if (cur().type == TokenType::OPERATOR && ASSIGN_OPS.count(cur().value))
    {
        string op = cur().value; consume();
        NodePtr right = parseAssignExpr();
        return make_shared<AssignStmt>(op, move(left), move(right), ln);
    }
    return left;
}

// Разбор or
NodePtr Parser::parseOrExpr()
{
    int ln = cur().line;
    NodePtr left = parseAndExpr();
    while (match(TokenType::OPERATOR, "||"))
    {
        string op = cur().value; consume();
        NodePtr right = parseAndExpr();
        left = make_shared<BinaryExpr>(op, move(left), move(right), ln);
    }
    return left;
}

// Разбор and
NodePtr Parser::parseAndExpr()
{
    int ln = cur().line;
    NodePtr left = parseEqualityExpr();
    while (match(TokenType::OPERATOR, "&&"))
    {
        string op = cur().value; consume();
        NodePtr right = parseEqualityExpr();
        left = make_shared<BinaryExpr>(op, move(left), move(right), ln);
    }
    return left;
}

// Разбор == и !=
NodePtr Parser::parseEqualityExpr()
{
    int ln = cur().line;
    NodePtr left = parseRelExpr();
    while (cur().type == TokenType::OPERATOR &&
        (cur().value == "==" || cur().value == "!="))
    {
        string op = cur().value; consume();
        NodePtr right = parseRelExpr();
        left = make_shared<BinaryExpr>(op, move(left), move(right), ln);
    }
    return left;
}

// Разбор операторов сравнения
NodePtr Parser::parseRelExpr()
{
    int ln = cur().line;
    NodePtr left = parseAddExpr();
    while (cur().type == TokenType::OPERATOR &&
        (cur().value == "<" || cur().value == ">" ||
            cur().value == "<=" || cur().value == ">="))
    {
        string op = cur().value; consume();
        NodePtr right = parseAddExpr();
        left = make_shared<BinaryExpr>(op, move(left), move(right), ln);
    }
    return left;
}

// Разбор сложения и вычитания
NodePtr Parser::parseAddExpr()
{
    int ln = cur().line;
    NodePtr left = parseMulExpr();
    while (cur().type == TokenType::OPERATOR &&
        (cur().value == "+" || cur().value == "-"))
    {
        string op = cur().value; consume();
        NodePtr right = parseMulExpr();
        left = make_shared<BinaryExpr>(op, move(left), move(right), ln);
    }
    return left;
}

// Разбор умножения и деления
NodePtr Parser::parseMulExpr()
{
    int ln = cur().line;
    NodePtr left = parseUnaryExpr();
    while (cur().type == TokenType::OPERATOR &&
        (cur().value == "*" || cur().value == "/" || cur().value == "%"))
    {
        string op = cur().value; consume();
        NodePtr right = parseUnaryExpr();
        left = make_shared<BinaryExpr>(op, move(left), move(right), ln);
    }
    return left;
}

// Разбор унарных операций
NodePtr Parser::parseUnaryExpr()
{
    int ln = cur().line;
    if (cur().type == TokenType::OPERATOR &&
        (cur().value == "!" || cur().value == "-" ||
            cur().value == "++" || cur().value == "--"))
    {
        string op = cur().value; consume();
        NodePtr operand = parseUnaryExpr();
        return make_shared<UnaryExpr>(op, move(operand), ln);
    }
    return parsePostfixExpr();
}

// Разбор постфиксных операций
NodePtr Parser::parsePostfixExpr()
{
    int ln = cur().line;
    NodePtr node = parsePrimaryExpr();

    while (true)
    {
        // [] для массива
        if (match(TokenType::DELIMITER, "["))
        {
            consume();
            NodePtr idx = parseExpr();
            expect(TokenType::DELIMITER, "]");
            node = make_shared<ArrayAccessExpr>(move(node), move(idx), ln);
        }
        // доступ к полю или методу объекта
        else if (match(TokenType::DELIMITER, "."))
        {
            consume();
            if (!match(TokenType::IDENTIFIER) && cur().type != TokenType::KEYWORD)
            {
                addError("Ожидалось имя члена после '.', получен '" +
                    cur().value + "'", cur().line);
                break;
            }
            string memberName = cur().value; consume();

            // метод
            if (match(TokenType::DELIMITER, "("))
            {
                vector<NodePtr> args = parseArgList();
                node = make_shared<MemberCallExpr>(move(node), memberName, move(args), ln);
            }
            else
            {
                auto memberNode = make_shared<IdentifierExpr>(memberName, ln);
                node = make_shared<BinaryExpr>(".", move(node), move(memberNode), ln);
            }
        }
        // cin и cout
        else if (match(TokenType::OPERATOR, "<<") || match(TokenType::OPERATOR, ">>"))
        {
            string op = cur().value; consume();
            NodePtr right = parseUnaryExpr();
            node = make_shared<BinaryExpr>(op, move(node), move(right), ln);
        }
        else if (cur().type == TokenType::OPERATOR &&
            (cur().value == "++" || cur().value == "--"))
        {
            string op = cur().value + "(post)"; consume();
            node = make_shared<UnaryExpr>(op, move(node), ln);
        }
        else break;
    }
    return node;
}

// Разбор атомарных выражений
NodePtr Parser::parsePrimaryExpr()
{
    int ln = cur().line;

    // скобочное выражение
    if (match(TokenType::DELIMITER, "("))
    {
        consume();
        NodePtr inner = parseExpr();
        expect(TokenType::DELIMITER, ")");
        return inner;
    }

    // числа, строки, булевы литералы
    if (cur().type == TokenType::CONSTANT_INT ||
        cur().type == TokenType::CONSTANT_FLOAT ||
        cur().type == TokenType::CONSTANT_STRING ||
        cur().type == TokenType::CONSTANT_BOOL)
    {
        string v = cur().value; consume();
        return make_shared<LiteralExpr>(v, ln);
    }

    // литералы из ключевых слов
    if (cur().type == TokenType::KEYWORD &&
        (cur().value == "true" || cur().value == "false" ||
            cur().value == "endl" || cur().value == "LC_ALL"))
    {
        string v = cur().value; consume();
        return make_shared<LiteralExpr>(v, ln);
    }

    // идентификатор или вызов функции
    if (cur().type == TokenType::IDENTIFIER)
    {
        string name = cur().value; consume();
        if (match(TokenType::DELIMITER, "("))
        {
            vector<NodePtr> args = parseArgList();
            return make_shared<CallExpr>(name, move(args), ln);
        }
        return make_shared<IdentifierExpr>(name, ln);
    }

    // аварийная обработка
    if (cur().type == TokenType::OPERATOR && cur().value == "::")
    {
        consume();
        if (cur().type == TokenType::IDENTIFIER)
        {
            string name = cur().value; consume();
            return make_shared<IdentifierExpr>(name, ln);
        }
    }

    // ничего не подошло
    addError("Неожиданный токен '" + cur().value +
        "' (" + tokenTypeName(cur().type) + ") в позиции выражения", ln);
    if (!atEnd()) consume();
    return make_shared<LiteralExpr>("?", ln);
}

// Разбор списка аргументов
vector<NodePtr> Parser::parseArgList()
{
    vector<NodePtr> args;
    expect(TokenType::DELIMITER, "(");
    while (!match(TokenType::DELIMITER, ")") && !atEnd())
    {
        args.push_back(parseExpr());
        if (match(TokenType::DELIMITER, ",")) consume();
        else break;
    }
    if (!match(TokenType::DELIMITER, ")"))
        addError("Ожидался ')' -- незакрытый список аргументов", cur().line);
    else
        consume();
    return args;
}
