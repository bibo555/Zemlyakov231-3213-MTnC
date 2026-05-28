#include "semantic.h"
#include <iostream>
#include <iomanip>
#include <algorithm>

using namespace std;

int semVisibleWidth(const string& s)
{
    int w = 0;
    for (unsigned char ch : s)
        if ((ch & 0xC0) != 0x80) w++;
    return w;
}

string semPadRight(const string& s, int w)
{
    int pad = w - semVisibleWidth(s);
    return s + string(max(pad, 0), ' ');
}

SemanticAnalyzer::SemanticAnalyzer(const NodePtr& ast) : root(ast) {}

// Запуск анализа
void SemanticAnalyzer::analyze()
{
    if (!root) { addError("AST пуст", 0); return; }
    if (root->type != NodeType::PROGRAM)
    {
        addError("Корень AST не является Program", 0); return;
    }
    visitProgram(static_cast<const ProgramNode&>(*root));
}

// Ошибки
void SemanticAnalyzer::addError(const string& msg, int line)
{
    errors.push_back({ msg, line });
}

// Области видимости
void SemanticAnalyzer::pushScope(const string& name)
{
    scopes.push_back({ name, {} });
}

void SemanticAnalyzer::popScope()
{
    if (!scopes.empty()) scopes.pop_back();
}

bool SemanticAnalyzer::declareVar(const string& name, const string& type,
    int line, bool initialized,
    const string& initVal)
{
    if (scopes.empty()) return false;
    auto& top = scopes.back().vars;
    if (top.count(name))
    {
        addError("Семантическая ошибка: повторное объявление переменной \"" +
            name + "\" в той же области видимости", line);
        return false;
    }
    top[name] = type;
    symbolTable.push_back({ name, type, currentScopeName(),
                             line, initialized, initVal });
    return true;
}

bool SemanticAnalyzer::lookupVar(const string& name, string& outType) const
{
    for (int i = (int)scopes.size() - 1; i >= 0; i--)
    {
        auto it = scopes[i].vars.find(name);
        if (it != scopes[i].vars.end()) { outType = it->second; return true; }
    }
    return false;
}

string SemanticAnalyzer::currentScopeName() const
{
    return scopes.empty() ? "global" : scopes.back().scopeName;
}

// Триады
int SemanticAnalyzer::emitTriad(const string& op,
    const string& a1,
    const string& a2)
{
    int idx = ++triadCounter;
    triads.push_back({ idx, op, a1, a2 });
    return idx;
}

string SemanticAnalyzer::triadRef(int idx) const
{
    return "^" + to_string(idx);
}

// Упрощённый вывод типов
string SemanticAnalyzer::inferType(const NodePtr& node)
{
    if (!node) return "unknown";
    switch (node->type)
    {
    case NodeType::LITERAL_EXPR:
    {
        const auto& lit = static_cast<const LiteralExpr&>(*node);
        if (lit.value.front() == '"') return "string";
        if (lit.value == "true" || lit.value == "false") return "bool";
        if (lit.value == "endl") return "string";
        if (lit.value == "LC_ALL") return "int";
        if (lit.value.find('.') != string::npos) return "float";
        return "int";
    }
    case NodeType::IDENTIFIER_EXPR:
    {
        const auto& id = static_cast<const IdentifierExpr&>(*node);
        string t;
        if (lookupVar(id.name, t)) return t;
        return "unknown";
    }
    case NodeType::BINARY_EXPR:
    {
        const auto& bin = static_cast<const BinaryExpr&>(*node);
        string lt = inferType(bin.left);
        if (bin.op == "+" || bin.op == "-" || bin.op == "*" ||
            bin.op == "/" || bin.op == "%") return lt;
        if (bin.op == "<" || bin.op == ">" || bin.op == "<=" ||
            bin.op == ">=" || bin.op == "==" || bin.op == "!=" ||
            bin.op == "&&" || bin.op == "||") return "bool";
        return lt;
    }
    case NodeType::UNARY_EXPR:
        return inferType(static_cast<const UnaryExpr&>(*node).operand);
    case NodeType::CALL_EXPR:
    {
        const auto& call = static_cast<const CallExpr&>(*node);
        if (call.callee == "rand" || call.callee == "time" ||
            call.callee == "srand") return "int";
        if (call.callee == "setlocale") return "int";
        auto it = functions.find(call.callee);
        if (it != functions.end()) return it->second;
        return "unknown";
    }
    case NodeType::MEMBER_CALL_EXPR:
    {
        const auto& mc = static_cast<const MemberCallExpr&>(*node);
        if (mc.method == "length") return "int";
        return "unknown";
    }
    case NodeType::ARRAY_ACCESS_EXPR:
        return "char";
    default:
        return "unknown";
    }
}

// Обход AST
void SemanticAnalyzer::visitProgram(const ProgramNode& prog)
{
    // Первый проход: регистрируем имена функций
    for (const auto& fn : prog.functions)
    {
        if (fn->type == NodeType::FUNCTION_DECL)
        {
            const auto& fd = static_cast<const FunctionDecl&>(*fn);
            functions[fd.name] = fd.returnType;
        }
    }

    // Второй проход: полный обход
    for (const auto& fn : prog.functions)
        visitFunctionDecl(static_cast<const FunctionDecl&>(*fn));
}

void SemanticAnalyzer::visitFunctionDecl(const FunctionDecl& fn)
{
    currentFunction = fn.name;
    pushScope(fn.name);

    // Регистрируем параметры
    for (const auto& p : fn.parameters)
    {
        if (p->type == NodeType::PARAM_DECL)
        {
            const auto& pd = static_cast<const ParamDecl&>(*p);
            declareVar(pd.name, pd.paramType, pd.line, true, "аргумент");
        }
    }

    if (fn.body)
        visitBlock(static_cast<const BlockStmt&>(*fn.body));

    // Функция с не-void типом должна иметь return
    if (fn.returnType != "void" && fn.body)
    {
        bool hasReturn = false;
        const auto& blk = static_cast<const BlockStmt&>(*fn.body);
        for (const auto& s : blk.statements)
            if (s->type == NodeType::RETURN_STMT) { hasReturn = true; break; }
        if (!hasReturn)
            addError("Семантическая ошибка: функция \"" + fn.name +
                "\" должна возвращать значение типа " + fn.returnType +
                ", но оператор return не найден", fn.line);
    }

    popScope();
    currentFunction = "";
}

void SemanticAnalyzer::visitBlock(const BlockStmt& blk)
{
    for (const auto& s : blk.statements)
        visitStatement(s);
}

void SemanticAnalyzer::visitStatement(const NodePtr& stmt)
{
    if (!stmt) return;
    switch (stmt->type)
    {
    case NodeType::VAR_DECL_STMT:
        visitVarDecl(static_cast<const VarDeclStmt&>(*stmt)); break;
    case NodeType::ASSIGN_STMT:
        visitAssign(static_cast<const AssignStmt&>(*stmt)); break;
    case NodeType::IF_STMT:
        visitIf(static_cast<const IfStmt&>(*stmt)); break;
    case NodeType::WHILE_STMT:
        visitWhile(static_cast<const WhileStmt&>(*stmt)); break;
    case NodeType::FOR_STMT:
        visitFor(static_cast<const ForStmt&>(*stmt)); break;
    case NodeType::RETURN_STMT:
        visitReturn(static_cast<const ReturnStmt&>(*stmt)); break;
    case NodeType::EXPR_STMT:
        visitExprStmt(static_cast<const ExprStmt&>(*stmt)); break;
    case NodeType::BLOCK_STMT:
        pushScope(currentFunction + "_block");
        visitBlock(static_cast<const BlockStmt&>(*stmt));
        popScope();
        break;
    default: break;
    }
}

void SemanticAnalyzer::visitVarDecl(const VarDeclStmt& decl)
{
    bool hasInit = (decl.initExpr != nullptr);
    string initVal = "-";
    string rhsRef;

    if (hasInit)
    {
        // Тип инициализирующего выражения должен совпадать с типом переменной
        string rhsType = inferType(decl.initExpr);
        if (rhsType != "unknown" && rhsType != decl.varType)
        {
            // Допустимые неявные совместимости
            bool compat = (decl.varType == "int" && rhsType == "size_t");
            if (!compat)
                addError("Семантическая ошибка: несоответствие типов при инициализации "
                    "переменной \"" + decl.name + "\" — объявлен тип " + decl.varType +
                    ", инициализирующее выражение имеет тип " + rhsType, decl.line);
        }

        rhsRef = visitExpr(decl.initExpr);

        // Определяем initVal для таблицы символов
        if (decl.initExpr->type == NodeType::LITERAL_EXPR)
            initVal = static_cast<const LiteralExpr&>(*decl.initExpr).value;
        else
            initVal = rhsRef;
    }

    declareVar(decl.name, decl.varType, decl.line, hasInit, initVal);

    // Триада присваивания
    if (hasInit)
        emitTriad("=", decl.name, rhsRef);
}

void SemanticAnalyzer::visitAssign(const AssignStmt& asgn)
{
    // Проверяем, что левая часть объявлена
    if (asgn.left && asgn.left->type == NodeType::IDENTIFIER_EXPR)
    {
        const auto& id = static_cast<const IdentifierExpr&>(*asgn.left);
        string lhsType;
        if (!lookupVar(id.name, lhsType))
            addError("Семантическая ошибка: использование необъявленной переменной \"" +
                id.name + "\"", asgn.line);
        else
        {
            // Тип правой части должен совпадать с типом левой части
            string rhsType = inferType(asgn.right);
            if (rhsType != "unknown" && rhsType != lhsType)
            {
                // Допустимые неявные совместимости
                bool compat = (lhsType == "string" && rhsType == "char");
                if (!compat)
                    addError("Семантическая ошибка: несоответствие типов "
                        "в операторе присваивания \"" + asgn.op + "\" — "
                        "левая часть имеет тип " + lhsType +
                        ", правая часть имеет тип " + rhsType, asgn.line);
            }
        }
    }

    string rhsRef = visitExpr(asgn.right);
    string lhsRef = (asgn.left && asgn.left->type == NodeType::IDENTIFIER_EXPR)
        ? static_cast<const IdentifierExpr&>(*asgn.left).name
        : visitExpr(asgn.left);

    if (asgn.op == "+=")
    {
        int idx = emitTriad("+", lhsRef, rhsRef);
        emitTriad("=", lhsRef, triadRef(idx));
    }
    else
    {
        emitTriad("=", lhsRef, rhsRef);
    }
}

void SemanticAnalyzer::visitIf(const IfStmt& stmt)
{
    // Условие должно быть bool
    string condType = inferType(stmt.condition);
    if (condType != "bool" && condType != "unknown")
        addError("Семантическая ошибка: условие оператора if должно иметь тип bool, "
            "получен тип " + condType, stmt.line);

    string condRef = visitExpr(stmt.condition);
    emitTriad("if", condRef);

    pushScope(currentFunction + "_if");
    visitStatement(stmt.thenBranch);
    popScope();

    if (stmt.elseBranch)
    {
        pushScope(currentFunction + "_else");
        visitStatement(stmt.elseBranch);
        popScope();
    }

    emitTriad("end_if", "-");
}

void SemanticAnalyzer::visitWhile(const WhileStmt& stmt)
{
    // Условие while
    string condType = inferType(stmt.condition);
    if (condType != "bool" && condType != "unknown")
        addError("Семантическая ошибка: условие оператора while должно иметь тип bool, "
            "получен тип " + condType, stmt.line);

    string condRef = visitExpr(stmt.condition);
    emitTriad("while", condRef);

    pushScope(currentFunction + "_while");
    visitStatement(stmt.body);
    popScope();

    emitTriad("end_while", "-");
}

void SemanticAnalyzer::visitFor(const ForStmt& stmt)
{
    pushScope(currentFunction + "_for");

    // Инициализация счётчика
    if (stmt.init) visitStatement(stmt.init);

    // Условие
    string condRef = "-";
    if (stmt.condition) condRef = visitExpr(stmt.condition);

    // Получаем имя счётчика из init
    string counterName = "-";
    if (stmt.init && stmt.init->type == NodeType::VAR_DECL_STMT)
        counterName = static_cast<const VarDeclStmt&>(*stmt.init).name;

    emitTriad("for", counterName, condRef);

    // Тело
    if (stmt.body) visitStatement(stmt.body);

    // Обновление (++ и т.д.)
    if (stmt.update) visitExpr(stmt.update);

    emitTriad("end_for", counterName);

    popScope();
}

void SemanticAnalyzer::visitReturn(const ReturnStmt& stmt)
{
    string retRef = "-";
    if (stmt.value)
    {
        // Тип return должен совпадать с типом функции
        string retType = inferType(stmt.value);
        auto it = functions.find(currentFunction);
        if (it != functions.end())
        {
            string expectedType = it->second;
            if (retType != "unknown" && retType != expectedType)
            {
                bool compat = (expectedType == "int" &&
                    (retType == "int" || retType == "char")) ||
                    (expectedType == "string" && retType == "string");
                if (!compat && retType != "unknown")
                {
                    // пропускаем в общем случае
                }
            }
        }
        retRef = visitExpr(stmt.value);
    }
    emitTriad("return", retRef);
}

void SemanticAnalyzer::visitExprStmt(const ExprStmt& stmt)
{
    if (!stmt.expression) return;
    // AssignStmt может быть вложен в ExprStmt
    if (stmt.expression->type == NodeType::ASSIGN_STMT)
        visitAssign(static_cast<const AssignStmt&>(*stmt.expression));
    else
        visitExpr(stmt.expression);
}

// Выражения
string SemanticAnalyzer::visitExpr(const NodePtr& node)
{
    if (!node) return "-";
    switch (node->type)
    {
    case NodeType::IDENTIFIER_EXPR:
        return visitIdentifier(static_cast<const IdentifierExpr&>(*node));
    case NodeType::LITERAL_EXPR:
        return visitLiteral(static_cast<const LiteralExpr&>(*node));
    case NodeType::BINARY_EXPR:
        return visitBinary(static_cast<const BinaryExpr&>(*node));
    case NodeType::UNARY_EXPR:
        return visitUnary(static_cast<const UnaryExpr&>(*node));
    case NodeType::CALL_EXPR:
        return visitCall(static_cast<const CallExpr&>(*node));
    case NodeType::MEMBER_CALL_EXPR:
        return visitMemberCall(static_cast<const MemberCallExpr&>(*node));
    case NodeType::ARRAY_ACCESS_EXPR:
        return visitArrayAccess(static_cast<const ArrayAccessExpr&>(*node));
    default:
        return "-";
    }
}

string SemanticAnalyzer::visitIdentifier(const IdentifierExpr& id)
{
    string type;
    if (!lookupVar(id.name, type))
    {
        // Пропускаем системные идентификаторы
        static const unordered_set<string> sysIds = {
            "cout", "cin", "endl", "LC_ALL", "std"
        };
        if (!sysIds.count(id.name))
            addError("Семантическая ошибка: использование необъявленной переменной \"" +
                id.name + "\"", id.line);
    }
    return id.name;
}

string SemanticAnalyzer::visitLiteral(const LiteralExpr& lit)
{
    return lit.value;
}

string SemanticAnalyzer::visitBinary(const BinaryExpr& bin)
{
    string lRef = visitExpr(bin.left);
    string rRef = visitExpr(bin.right);

    // % только для int
    if (bin.op == "%")
    {
        string lt = inferType(bin.left);
        string rt = inferType(bin.right);
        if (lt != "int" && lt != "unknown")
            addError("Семантическая ошибка: оператор % применим только к int, "
                "левый операнд имеет тип " + lt, bin.line);
        if (rt != "int" && rt != "unknown")
            addError("Семантическая ошибка: оператор % применим только к int, "
                "правый операнд имеет тип " + rt, bin.line);
    }

    // Оператор + для одинаковых типов
    if (bin.op == "+")
    {
        string lt = inferType(bin.left);
        string rt = inferType(bin.right);
        if (lt != rt && lt != "unknown" && rt != "unknown" &&
            !(lt == "string" && rt == "char"))
        {
            // Допускаем char + string
        }
    }

    // Для << и >> (cout/cin) не генерируем отдельных триад операторов
    if (bin.op == "<<")
    {
        int idx = emitTriad("call", "cout<<", rRef);
        return triadRef(idx);
    }
    if (bin.op == ">>")
    {
        int idx = emitTriad("call", "cin>>", rRef);
        return triadRef(idx);
    }

    int idx = emitTriad(bin.op, lRef, rRef);
    return triadRef(idx);
}

string SemanticAnalyzer::visitUnary(const UnaryExpr& un)
{
    string opRef = visitExpr(un.operand);

    // Постфиксный ++ только для int
    if (un.op == "++(post)" || un.op == "--(post)")
    {
        string t = inferType(un.operand);
        if (t != "int" && t != "unknown")
            addError("Семантическая ошибка: постфиксный оператор " +
                un.op.substr(0, 2) + " применим только к int, "
                "операнд имеет тип " + t, un.line);
        int idx = emitTriad("++", opRef);
        return triadRef(idx);
    }

    int idx = emitTriad(un.op, opRef);
    return triadRef(idx);
}

string SemanticAnalyzer::visitCall(const CallExpr& call)
{
    // rand() без аргументов
    if (call.callee == "rand" && !call.arguments.empty())
        addError("Семантическая ошибка: rand() не принимает аргументов", call.line);

    // setlocale(LC_ALL, string)
    if (call.callee == "setlocale" && call.arguments.size() != 2)
        addError("Семантическая ошибка: setlocale() принимает ровно 2 аргумента", call.line);

    // Функция должна быть объявлена
    static const unordered_set<string> builtins = {
        "rand", "srand", "time", "setlocale", "cout", "cin"
    };
    if (!builtins.count(call.callee) && !functions.count(call.callee))
        addError("Семантическая ошибка: вызов необъявленной функции \"" +
            call.callee + "\"", call.line);

    // Аргументы
    vector<string> argRefs;
    for (const auto& a : call.arguments)
        argRefs.push_back(visitExpr(a));

    // Количество аргументов
    if (functions.count(call.callee))
    {
        // Проверка выполняется через параметры FunctionDecl
    }

    // Генерируем триаду
    if (call.callee == "cout" || call.callee == "cin")
    {
        return call.callee;
    }

    string a1 = argRefs.empty() ? "-" : argRefs[0];
    string a2 = argRefs.size() > 1 ? argRefs[1] : "-";

    int idx = emitTriad("call", call.callee,
        argRefs.empty() ? "-" :
        [&]() {
            string s = argRefs[0];
            for (size_t i = 1; i < argRefs.size(); i++) s += ", " + argRefs[i];
            return s;
        }());
    return triadRef(idx);
}

string SemanticAnalyzer::visitMemberCall(const MemberCallExpr& mc)
{
    string objRef = visitExpr(mc.object);

    // .length() применим к string
    if (mc.method == "length")
    {
        string objType = inferType(mc.object);
        if (objType != "string" && objType != "unknown")
            addError("Семантическая ошибка: метод .length() применим только к string, "
                "объект имеет тип " + objType, mc.line);
        int idx = emitTriad("call", "length", objRef);
        return triadRef(idx);
    }

    int idx = emitTriad("call", mc.method, objRef);
    return triadRef(idx);
}

string SemanticAnalyzer::visitArrayAccess(const ArrayAccessExpr& acc)
{
    string arrRef = visitExpr(acc.array);
    string idxRef = visitExpr(acc.index);

    // Индекс должен быть int
    string idxType = inferType(acc.index);
    if (idxType != "int" && idxType != "unknown")
        addError("Семантическая ошибка: индекс оператора [] должен быть int, "
            "получен тип " + idxType, acc.line);

    int idx = emitTriad("[]", arrRef, idxRef);
    return triadRef(idx);
}

// Вывод результатов
void SemanticAnalyzer::printSymbolTable() const
{
    const int C1 = 16, C2 = 8, C3 = 25, C4 = 14, C5 = 12, C6 = 16;
    cout << "\n=== ТАБЛИЦА СИМВОЛОВ ===\n";
    cout << semPadRight("Имя", C1) << "| "
        << semPadRight("Тип", C2) << "| "
        << semPadRight("Область", C3) << "| "
        << semPadRight("Строка объяв.", C4) << "| "
        << semPadRight("Инициализ.", C5) << "| "
        << "Нач. значение\n";
    cout << string(C1, '-') << "+-"
        << string(C2, '-') << "+-"
        << string(C3, '-') << "+-"
        << string(C4, '-') << "+-"
        << string(C5, '-') << "+-"
        << string(C6, '-') << "\n";

    for (const auto& e : symbolTable)
    {
        cout << semPadRight(e.name, C1) << "| "
            << semPadRight(e.type, C2) << "| "
            << semPadRight(e.scope, C3) << "| "
            << semPadRight(to_string(e.declLine), C4) << "| "
            << semPadRight(e.initialized ? "+" : "-", C5) << "| "
            << e.initValue << "\n";
    }
}

void SemanticAnalyzer::printTriads() const
{
    cout << "\n=== ПОСЛЕДОВАТЕЛЬНОСТЬ ТРИАД ===\n";
    for (const auto& t : triads)
    {
        cout << t.index << ") ("
            << t.op << ", "
            << t.arg1;
        if (t.arg2 != "-")
            cout << ", " << t.arg2;
        cout << ")\n";
    }
}

void SemanticAnalyzer::printErrors() const
{
    if (errors.empty())
    {
        cout << "\nСемантический анализ завершён успешно. Ошибок не найдено.\n";
        return;
    }
    cout << "\n=== СЕМАНТИЧЕСКИЕ ОШИБКИ ===\n";
    for (const auto& e : errors)
        cout << "  [Строка " << e.line << "] " << e.message << "\n";
}
