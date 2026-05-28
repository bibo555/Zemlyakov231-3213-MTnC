#pragma once

#include "parser.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <optional>

using namespace std;

// Таблица символов
struct SymbolEntry
{
    string name;
    string type;
    string scope;
    int    declLine;
    bool   initialized;
    string initValue;
};

// Триада
struct Triad
{
    int    index;
    string op;
    string arg1;
    string arg2;
};

// Ошибка
struct SemanticError
{
    string message;
    int    line;
};

// Семантический анализатор
class SemanticAnalyzer
{
public:
    explicit SemanticAnalyzer(const NodePtr& ast);

    void analyze();

    const vector<SymbolEntry>& getSymbolTable() const { return symbolTable; }
    const vector<Triad>& getTriads()       const { return triads; }
    const vector<SemanticError>& getErrors()      const { return errors; }

    void printSymbolTable()  const;
    void printTriads()       const;
    void printErrors()       const;

private:
    const NodePtr& root;

    vector<SymbolEntry>   symbolTable;
    vector<Triad>         triads;
    vector<SemanticError> errors;

    // Область видимости
    struct Scope
    {
        string scopeName;
        unordered_map<string, string> vars; // (имя, тип)
    };
    vector<Scope> scopes;

    // (функция, возвращаемый тип)
    unordered_map<string, string> functions;

    // Текущая функция (для return)
    string currentFunction;

    // Счётчик триад
    int triadCounter = 0;

    // Добавление ошибок
    void addError(const string& msg, int line);

    // Работа с областями видимости
    void   pushScope(const string& name);
    void   popScope();
    bool   declareVar(const string& name, const string& type, int line, bool initialized, const string& initVal);
    bool   lookupVar(const string& name, string& outType) const;
    string currentScopeName() const;

    // Генерация триад
    int  emitTriad(const string& op, const string& a1, const string& a2 = "-");
    string triadRef(int idx) const; // "^N"

    // Определение типа выражения
    string inferType(const NodePtr& node);

    // Обход узлов
    void visitProgram(const ProgramNode& prog);
    void visitFunctionDecl(const FunctionDecl& fn);
    void visitBlock(const BlockStmt& blk);
    void visitStatement(const NodePtr& stmt);
    void visitVarDecl(const VarDeclStmt& decl);
    void visitAssign(const AssignStmt& asgn);
    void visitIf(const IfStmt& stmt);
    void visitWhile(const WhileStmt& stmt);
    void visitFor(const ForStmt& stmt);
    void visitReturn(const ReturnStmt& stmt);
    void visitExprStmt(const ExprStmt& stmt);

    string visitExpr(const NodePtr& expr);
    string visitIdentifier(const IdentifierExpr& id);
    string visitLiteral(const LiteralExpr& lit);
    string visitBinary(const BinaryExpr& bin);
    string visitUnary(const UnaryExpr& un);
    string visitCall(const CallExpr& call);
    string visitMemberCall(const MemberCallExpr& mc);
    string visitArrayAccess(const ArrayAccessExpr& acc);
};

int semVisibleWidth(const string& s);
string semPadRight(const string& s, int w);
#pragma once
