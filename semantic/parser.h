#pragma once

#include "lexer.h"
#include <string>
#include <vector>
#include <memory>

// ? using namespace std;  ? УДАЛЕНО

// Все типы узлов
enum class NodeType
{
    PROGRAM,
    FUNCTION_DECL,
    PARAM_DECL,
    BLOCK_STMT,
    VAR_DECL_STMT,
    ASSIGN_STMT,
    IF_STMT,
    WHILE_STMT,
    FOR_STMT,
    RETURN_STMT,
    EXPR_STMT,
    IDENTIFIER_EXPR,
    LITERAL_EXPR,
    BINARY_EXPR,
    UNARY_EXPR,
    CALL_EXPR,
    MEMBER_CALL_EXPR,
    ARRAY_ACCESS_EXPR
};

std::string nodeTypeName(NodeType t);

// Базовый класс ASTNode
struct ASTNode
{
    NodeType type;
    int      line = 0;

    explicit ASTNode(NodeType t, int ln = 0) : type(t), line(ln) {}
    virtual ~ASTNode() = default;

    virtual void print(const std::string& prefix = "", bool isLast = true) const = 0;
};

using NodePtr = std::shared_ptr<ASTNode>;

inline std::string childPrefix(const std::string& prefix, bool isLast)
{
    return prefix + (isLast ? "    " : "|   ");
}

// ===== Expressions =====
struct IdentifierExpr : ASTNode
{
    std::string name;
    IdentifierExpr(const std::string& n, int ln)
        : ASTNode(NodeType::IDENTIFIER_EXPR, ln), name(n) {}
    void print(const std::string& prefix, bool isLast) const override;
};

struct LiteralExpr : ASTNode
{
    std::string value;
    LiteralExpr(const std::string& v, int ln)
        : ASTNode(NodeType::LITERAL_EXPR, ln), value(v) {}
    void print(const std::string& prefix, bool isLast) const override;
};

struct BinaryExpr : ASTNode
{
    std::string op;
    NodePtr left;
    NodePtr right;
    BinaryExpr(const std::string& o, NodePtr l, NodePtr r, int ln)
        : ASTNode(NodeType::BINARY_EXPR, ln), op(o),
        left(std::move(l)), right(std::move(r)) {}
    void print(const std::string& prefix, bool isLast) const override;
};

struct UnaryExpr : ASTNode
{
    std::string op;
    NodePtr operand;
    UnaryExpr(const std::string& o, NodePtr operand, int ln)
        : ASTNode(NodeType::UNARY_EXPR, ln), op(o), operand(std::move(operand)) {}
    void print(const std::string& prefix, bool isLast) const override;
};

struct CallExpr : ASTNode
{
    std::string callee;
    std::vector<NodePtr> arguments;
    CallExpr(const std::string& c, std::vector<NodePtr> args, int ln)
        : ASTNode(NodeType::CALL_EXPR, ln), callee(c), arguments(std::move(args)) {}
    void print(const std::string& prefix, bool isLast) const override;
};

struct MemberCallExpr : ASTNode
{
    NodePtr object;
    std::string method;
    std::vector<NodePtr> arguments;
    MemberCallExpr(NodePtr obj, const std::string& m, std::vector<NodePtr> args, int ln)
        : ASTNode(NodeType::MEMBER_CALL_EXPR, ln),
        object(std::move(obj)), method(m), arguments(std::move(args)) {}
    void print(const std::string& prefix, bool isLast) const override;
};

struct ArrayAccessExpr : ASTNode
{
    NodePtr array;
    NodePtr index;
    ArrayAccessExpr(NodePtr arr, NodePtr idx, int ln)
        : ASTNode(NodeType::ARRAY_ACCESS_EXPR, ln),
        array(std::move(arr)), index(std::move(idx)) {}
    void print(const std::string& prefix, bool isLast) const override;
};

// ===== Statements =====
struct BlockStmt : ASTNode
{
    std::vector<NodePtr> statements;
    explicit BlockStmt(int ln) : ASTNode(NodeType::BLOCK_STMT, ln) {}
    void print(const std::string& prefix, bool isLast) const override;
};

struct VarDeclStmt : ASTNode
{
    std::string varType;
    std::string name;
    NodePtr initExpr;
    VarDeclStmt(const std::string& t, const std::string& n, NodePtr init, int ln)
        : ASTNode(NodeType::VAR_DECL_STMT, ln),
        varType(t), name(n), initExpr(std::move(init)) {}
    void print(const std::string& prefix, bool isLast) const override;
};

struct AssignStmt : ASTNode
{
    std::string op;
    NodePtr left;
    NodePtr right;
    AssignStmt(const std::string& o, NodePtr l, NodePtr r, int ln)
        : ASTNode(NodeType::ASSIGN_STMT, ln), op(o),
        left(std::move(l)), right(std::move(r)) {}
    void print(const std::string& prefix, bool isLast) const override;
};

struct IfStmt : ASTNode
{
    NodePtr condition;
    NodePtr thenBranch;
    NodePtr elseBranch;
    IfStmt(NodePtr cond, NodePtr thenB, NodePtr elseB, int ln)
        : ASTNode(NodeType::IF_STMT, ln),
        condition(std::move(cond)), thenBranch(std::move(thenB)), elseBranch(std::move(elseB)) {}
    void print(const std::string& prefix, bool isLast) const override;
};

struct WhileStmt : ASTNode
{
    NodePtr condition;
    NodePtr body;
    WhileStmt(NodePtr cond, NodePtr body, int ln)
        : ASTNode(NodeType::WHILE_STMT, ln),
        condition(std::move(cond)), body(std::move(body)) {}
    void print(const std::string& prefix, bool isLast) const override;
};

struct ForStmt : ASTNode
{
    NodePtr init;
    NodePtr condition;
    NodePtr update;
    NodePtr body;
    ForStmt(NodePtr i, NodePtr c, NodePtr u, NodePtr b, int ln)
        : ASTNode(NodeType::FOR_STMT, ln),
        init(std::move(i)), condition(std::move(c)),
        update(std::move(u)), body(std::move(b)) {}
    void print(const std::string& prefix, bool isLast) const override;
};

struct ReturnStmt : ASTNode
{
    NodePtr value;
    ReturnStmt(NodePtr v, int ln)
        : ASTNode(NodeType::RETURN_STMT, ln), value(std::move(v)) {}
    void print(const std::string& prefix, bool isLast) const override;
};

struct ExprStmt : ASTNode
{
    NodePtr expression;
    ExprStmt(NodePtr expr, int ln)
        : ASTNode(NodeType::EXPR_STMT, ln), expression(std::move(expr)) {}
    void print(const std::string& prefix, bool isLast) const override;
};

// ===== Functions =====
struct ParamDecl : ASTNode
{
    std::string paramType;
    std::string name;
    ParamDecl(const std::string& t, const std::string& n, int ln)
        : ASTNode(NodeType::PARAM_DECL, ln), paramType(t), name(n) {}
    void print(const std::string& prefix, bool isLast) const override;
};

struct FunctionDecl : ASTNode
{
    std::string returnType;
    std::string name;
    std::vector<NodePtr> parameters;
    NodePtr body;

    FunctionDecl(const std::string& rt, const std::string& n,
        std::vector<NodePtr> params, NodePtr b, int ln)
        : ASTNode(NodeType::FUNCTION_DECL, ln),
        returnType(rt), name(n),
        parameters(std::move(params)), body(std::move(b)) {}

    void print(const std::string& prefix, bool isLast) const override;
};

struct ProgramNode : ASTNode
{
    std::vector<NodePtr> functions;
    explicit ProgramNode(int ln) : ASTNode(NodeType::PROGRAM, ln) {}
    void print(const std::string& prefix, bool isLast) const override;
};

// ===== Errors =====
struct ParseError
{
    std::string message;
    int line;
};

// ===== Parser =====
class Parser
{
public:
    explicit Parser(const std::vector<Token>& tokens);
    void parse();
    const std::vector<ParseError>& getErrors() const { return errors; }

private:
    const std::vector<Token>& tokens;
    size_t pos;
    std::vector<ParseError> errors;

    const Token& cur() const;
    const Token& peek(int off = 1) const;
    bool atEnd() const;
    Token consume();
    bool match(TokenType t, const std::string& val = "") const;
    Token expect(TokenType t, const std::string& val = "");
    bool isType(const std::string& v) const;

    void addError(const std::string& msg, int line);
    void skipTo(std::initializer_list<std::string> stopValues);

    NodePtr parseProgram();
    NodePtr parseFunctionDecl();
    NodePtr parseParamDecl();
    NodePtr parseBlock();
    NodePtr parseStatement();
    NodePtr parseVarDeclStmt();
    NodePtr parseIfStmt();
    NodePtr parseWhileStmt();
    NodePtr parseForStmt();
    NodePtr parseReturnStmt();
    NodePtr parseExprStmt();
    NodePtr parseForInit();

    NodePtr parseExpr();
    NodePtr parseAssignExpr();
    NodePtr parseOrExpr();
    NodePtr parseAndExpr();
    NodePtr parseEqualityExpr();
    NodePtr parseRelExpr();
    NodePtr parseAddExpr();
    NodePtr parseMulExpr();
    NodePtr parseUnaryExpr();
    NodePtr parsePostfixExpr();
    NodePtr parsePrimaryExpr();
    std::vector<NodePtr> parseArgList();
};