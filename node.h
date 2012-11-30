#include <iostream>
#include <vector>
#include <typeinfo>
#include "codegen.h"

#include "yaml-cpp/yaml.h"

class CodeGenContext;
class Statement;
class Expression;
class VariableDeclaration;

typedef std::vector<Statement*> StatementList;
typedef std::vector<Expression*> ExpressionList;
typedef std::vector<VariableDeclaration*> VariableList;

extern int addr;


#define PRINT_ID(x) printf("%*s %s", indent, "", x );
#define INDENT printf("%*s", indent, "");
class Node {
public:
    virtual ~Node() {}
    virtual Value *value() { return NULL; }
    virtual Type *type() { return NULL; }
    virtual YAML::Node yaml() const = 0;
    virtual void codeGen(CodeGenContext& context) { }
};

class Expression : public Node {
public:
    virtual void print(int indent) const { printf("node\n"); }
};

class Statement : public Node {
public:
 virtual void print(int indent) const { printf("statement\n"); }
};

class Integer : public Expression {
public:
    long long m_value;

    Value *value() { return new Value(m_value); }
    Integer(long long value) : m_value(value) { }
    void print(int indent) const
    {
        INDENT;
        printf(" %lld\n", m_value);
    }
    YAML::Node yaml() const
    {
        YAML::Node node;
        node["type"] = "integer";
        node["value"] = m_value;
        return node;
    }

    void codeGen(CodeGenContext& context);
};

class Double : public Expression {
public:
    double value;
    Double(double value) : value(value) { }
    YAML::Node yaml() const
    {
        YAML::Node node;
        node["type"] = "double";
        node["value"] = value;
        return node;
    }
    void codeGen(CodeGenContext& context);
};

class String : public Expression {
public:
    std::string value;
    String(const std::string* value) : value(*value) { }
    void codeGen(CodeGenContext& context);
    YAML::Node yaml() const
    {
        YAML::Node node;
        node["type"] = "string";
        node["value"] = value;
        return node;
    }
};

class Typename : public Expression {
public:
    std::string name;
    Typename(const std::string& name) : name(name) { }

    YAML::Node yaml() const
    {
        YAML::Node node;
        node["type"] = "typename";
        node["value"] = name;
        return node;
    }

    void codeGen(CodeGenContext& context) {};
};


class Identifier : public Expression {
public:
    std::string name;
    Identifier(const std::string& name) : name(name) { }
    void print(int indent) const
    {
        printf("%*s %s\n", indent, "", name.c_str());
    }

    YAML::Node yaml() const
    {
        YAML::Node node;
        node["type"] = "identifier";
        node["value"] = name;
        return node;
    }

    void codeGen(CodeGenContext& context);
};

class MethodCall : public Expression {
public:
    const Identifier* id;

    ExpressionList* arguments;

    MethodCall(const Identifier* id, ExpressionList* arguments) :
        id(id), arguments(arguments) { }
    MethodCall(const Identifier* id) : id(id) { }
    void codeGen(CodeGenContext& context);
    YAML::Node yaml() const
    {
        YAML::Node node;
        node["type"] = "methodcall";

        YAML::Node args = node["arguments"];
        for(auto it : *arguments)
            args.push_back(it->yaml());

        node["value"] = id->yaml();

        return node;
    }
};

class BinaryOperator : public Expression {
public:
    int op;
    Expression* lhs;
    Expression* rhs;

    BinaryOperator(Expression* lhs, int op, Expression* rhs) :
        lhs(lhs), rhs(rhs), op(op)
    {
    }
    void print(int indent) const;
    void codeGen(CodeGenContext& context);
    YAML::Node yaml() const
    {
        YAML::Node node;
        node["type"] = "binop";
        node["left"] = lhs->yaml();
        node["right"] = rhs->yaml();
        return node;
    }
};

class Assignment : public Expression {
public:
    Identifier* lhs;
    Expression* rhs;
    Assignment(Identifier* lhs, Expression* rhs) :
        lhs(lhs), rhs(rhs) { }
    void print(int indent) const {
        PRINT_ID("assign\n");
        lhs->print(indent+1);
        rhs->print(indent+1);
    }
    void codeGen(CodeGenContext& context);
    YAML::Node yaml() const
    {
        YAML::Node node;
        node["type"] = "assign";
        node["left"] = lhs->yaml();
        node["right"] = rhs->yaml();
        return node;
    }
};

class Block : public Expression {
public:
    StatementList statements;
    Block() { }
    void codeGen(CodeGenContext& context, int local_funcs_only);
    void print(int indent) const {
        PRINT_ID("block\n");
        for(size_t i = 0; i < statements.size(); i++) {
            statements[i]->print(indent+1);
        }
    }
    YAML::Node yaml() const
    {
        YAML::Node node;
        node["type"] = "block";

        YAML::Node stmnts = node["statements"];
        for(auto it : statements)
            stmnts.push_back(it->yaml());
        return node;
    }
};

#if 0
class ExpressionStatement : public Statement {
public:
    Expression& expression;
    ExpressionStatement(Expression& expression) :
        expression(expression) { }
    void codeGen(CodeGenContext& context);
};
#endif

class VariableDeclaration : public Statement {
public:
    const Typename* type;
    Identifier* id;
    Expression* assignmentExpr;
    Value *v;
    VariableDeclaration(const Typename* type, Identifier* id) :
        type(type), id(id), v(0)
    {
    }

    VariableDeclaration(const Typename* type, Identifier* id, Expression *assignmentExpr) :
        type(type), id(id), assignmentExpr(assignmentExpr), v(0)
    {

    }

    Value *value()
    {
        if (v == NULL)
            v = new Value(addr++);
        return v;
    }

    void print(int indent) const
    {
        PRINT_ID("variable declaration:\n");
        id->print(indent + 1);
        INDENT; printf(" = \n");
        assignmentExpr->print(indent + 1);

    }

    YAML::Node yaml() const
    {
        YAML::Node node;
        node["type"] = "vardecl";
        node["variable-type"] = type->yaml();
        node["identifier"] = id->yaml();
        if (assignmentExpr)
            node["assign"] = assignmentExpr->yaml();
        return node;
    }

    void codeGen(CodeGenContext& context);
};

class FunctionDeclaration : public Statement {
public:
    const Typename* type;
    const Identifier* id;
    const VariableList* arguments;
    Block* block;
    FunctionDeclaration(const Typename* type,
                        const Identifier* id,
                        const VariableList* arguments,
                        Block* block) :
        type(type), id(id), arguments(arguments), block(block)
    {

    }

    void print(int indent) const {
        PRINT_ID("function declaration\n");
        type->print(indent + 1);
        id->print(indent + 1);
        block->print(indent + 1);
    }

    void codeGen(CodeGenContext& context);

    YAML::Node yaml() const
    {
        YAML::Node node;
        node["type"] = "function";
        node["name"] = id->yaml();
        node["return-type"] = type->yaml();
        YAML::Node args = node["arguments"];
        for(auto it : *arguments)
            args.push_back(it->yaml());

        node["block"] = block->yaml();

        return node;
    }
};

#if 0
class ExternDeclaration : public Statement
{
public:
    const Identifier* type;
    const Identifier* id;
    const VariableList* arguments;

    ExternDeclaration(const Identifier* type, const Identifier* id, const VariableList* arguments) :
        type(type), id(id), arguments(arguments)
    {

    }

    void codeGen(CodeGenContext& context);
};
#endif


class IfStatement : public Statement
{
public:
    Expression *evalExpr;
    Block* thenBlock;
    Block* elseBlock;

    IfStatement(Expression *evalExpr, Block* thenBlock, Block *elseBlock) :
        evalExpr(evalExpr), thenBlock(thenBlock), elseBlock(elseBlock)
    {
    }

    void codeGen(CodeGenContext& context);
    YAML::Node yaml() const
    {
        YAML::Node node;
        node["type"] = "if";
        node["expr"] = evalExpr->yaml();
        node["true-branch"] = thenBlock->yaml();
        if (elseBlock)
            node["false-branch"] = elseBlock->yaml();
        return node;
    }
};

class ReturnStatement : public Statement
{
public:
    ReturnStatement(Expression *expr):
        expr(expr)
    {

    }

    void codeGen(CodeGenContext &context);
    YAML::Node yaml() const
    {
        YAML::Node node;
        node["type"] = "return";
        node["expr"] = expr->yaml();
        return node;
    }
private:
    Expression *expr;
};


class AssertStatement : public Statement
{
public:
    AssertStatement(Expression *expr):
        expr(expr)
    {

    }

    void print(int indent) const
    {
        PRINT_ID("assert\n");
        expr->print(indent + 1);
    }

    void codeGen(CodeGenContext &context);
    YAML::Node yaml() const
    {
        YAML::Node node;
        node["type"] = "assert";
        node["expr"] = expr->yaml();
        return node;
    }
private:
    Expression *expr;
};

class PrintStatement : public Statement
{
public:
    PrintStatement(Expression *expr):
        expr(expr)
    {

    }

    void print(int indent) const
    {
        PRINT_ID("print\n");
        expr->print(indent + 1);
    }
    void codeGen(CodeGenContext &context);
    YAML::Node yaml() const
    {
        YAML::Node node;
        node["type"] = "print";
        node["expr"] = expr->yaml();
        return node;
    }
private:
    Expression *expr;
};

