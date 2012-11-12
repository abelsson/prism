#include <iostream>
#include <vector>

#include "codegen.h"

class CodeGenContext;
class Statement;
class Expression;
class VariableDeclaration;

typedef std::vector<Statement*> StatementList;
typedef std::vector<Expression*> ExpressionList;
typedef std::vector<VariableDeclaration*> VariableList;

extern int addr;

class Node {
public:
    virtual ~Node() {}
    virtual Value *value() { return NULL; }
    virtual Type *type() { return NULL; }
    virtual void codeGen(CodeGenContext& context) { }
};

class Expression : public Node {
};

class Statement : public Node {
};

class Integer : public Expression {
public:
    long long m_value;

    Value *value() { return new Value(m_value); }
    Integer(long long value) : m_value(value) { }
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
    void codeGen(CodeGenContext& context);
};

class Assignment : public Expression {
public:
    Identifier* lhs;
    Expression* rhs;
    Assignment(Identifier* lhs, Expression* rhs) :
        lhs(lhs), rhs(rhs) { }
    void codeGen(CodeGenContext& context);
};

class Block : public Expression {
public:
    StatementList statements;
    Block() { }
    void codeGen(CodeGenContext& context, int local_funcs_only);
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

    IfStatement(Expression *evalExpr, Block* elseBlock, Block *thenBlock) :
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

