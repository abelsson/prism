#include <iostream>
#include <vector>
#include <typeinfo>
#include "codegen.h"

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
    virtual void print(int indent) const = 0;
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
    void print(int indent) const { INDENT; printf("%lld\n", m_value); }
    void codeGen(CodeGenContext& context);
};

class Double : public Expression {
public:
    double value;
    Double(double value) : value(value) { }
    void codeGen(CodeGenContext& context);
};

class String : public Expression {
public:
    std::string value;
    String(const std::string* value) : value(*value) { }
    void codeGen(CodeGenContext& context);
};

class Identifier : public Expression {
public:
    std::string name;
    Identifier(const std::string& name) : name(name) { }
    void print(int indent) const {
        printf("%s", name.c_str());
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
    void print(int indent) const {
        PRINT_ID("binop\n");
        lhs->print(indent+1);
        rhs->print(indent+1);
    }
    void codeGen(CodeGenContext& context);
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
};

class ExpressionStatement : public Statement {
public:
    Expression& expression;
    ExpressionStatement(Expression& expression) :
        expression(expression) { }
    void codeGen(CodeGenContext& context);
};

class VariableDeclaration : public Statement {
public:
    const Identifier* type;
    Identifier* id;
    Expression* assignmentExpr;
    Value *v;
    VariableDeclaration(const Identifier* type, Identifier* id) :
        type(type), id(id), v(0)
    {
    }

    VariableDeclaration(const Identifier* type, Identifier* id, Expression *assignmentExpr) :
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
        PRINT_ID("variable declaration: ");
        id->print(indent + 1);
        printf(" = \n");
        assignmentExpr->print(indent + 1);

    }

    void codeGen(CodeGenContext& context);
};

class FunctionDeclaration : public Statement {
public:
    const Identifier* type;
    const Identifier* id;
    const VariableList* arguments;
    Block* block;
    FunctionDeclaration(const Identifier* type,
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
};

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
};

class ReturnStatement : public Statement
{
public:
    ReturnStatement(Expression *expr):
        expr(expr)
    {

    }

    void codeGen(CodeGenContext &context);
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

    void codeGen(CodeGenContext &context);
private:
    Expression *expr;
};
