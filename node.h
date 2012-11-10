#include <iostream>
#include <vector>

#include "codegen.h"

class CodeGenContext;
class NStatement;
class Expression;
class VariableDeclaration;

typedef std::vector<NStatement*> StatementList;
typedef std::vector<Expression*> ExpressionList;
typedef std::vector<VariableDeclaration*> VariableList;


class Node {
public:
    virtual ~Node() {}
    virtual Value* codeGen(CodeGenContext& context) { }
};


class Expression : public Node {
};

class NStatement : public Node {
};

class NInteger : public Expression {
public:
    long long value;
    NInteger(long long value) : value(value) { }
    virtual Value* codeGen(CodeGenContext& context);
};

class NDouble : public Expression {
public:
    double value;
    NDouble(double value) : value(value) { }
    virtual Value* codeGen(CodeGenContext& context);
};

class NString : public Expression {
public:
    std::string value;
    NString(const std::string* value) : value(*value) { }
    virtual Value* codeGen(CodeGenContext& context);
};

class NIdentifier : public Expression {
public:
    std::string name;
    NIdentifier(const std::string& name) : name(name) { }
    virtual Value* codeGen(CodeGenContext& context);
};

class NMethodCall : public Expression {
public:
    const NIdentifier& id;
    ExpressionList arguments;
    NMethodCall(const NIdentifier& id, ExpressionList& arguments) :
        id(id), arguments(arguments) { }
    NMethodCall(const NIdentifier& id) : id(id) { }
    virtual Value* codeGen(CodeGenContext& context);
};

class BinaryOperator : public Expression {
public:
    int op;
    Expression& lhs;
    Expression& rhs;
    BinaryOperator(Expression& lhs, int op, Expression& rhs) :
        lhs(lhs), rhs(rhs), op(op) { }
    virtual Value* codeGen(CodeGenContext& context);
};

class Assignment : public Expression {
public:
    NIdentifier& lhs;
    Expression& rhs;
    Assignment(NIdentifier& lhs, Expression& rhs) :
        lhs(lhs), rhs(rhs) { }
    virtual Value* codeGen(CodeGenContext& context);
};

class Block : public Expression {
public:
    StatementList statements;
    Block() { }
    virtual Value* codeGen(CodeGenContext& context, int local_funcs_only);
};

class ExpressionStatement : public NStatement {
public:
    Expression& expression;
    ExpressionStatement(Expression& expression) :
        expression(expression) { }
    virtual Value* codeGen(CodeGenContext& context);
};

class VariableDeclaration : public NStatement {
public:
    const NIdentifier& type;
    NIdentifier& id;
    Expression *assignmentExpr;

    VariableDeclaration(const NIdentifier& type, NIdentifier& id) :
        type(type), id(id)
    {
    }

    VariableDeclaration(const NIdentifier& type, NIdentifier& id, Expression *assignmentExpr) :
        type(type), id(id), assignmentExpr(assignmentExpr)
    {

    }

    virtual Value* codeGen(CodeGenContext& context);
};

class FunctionDeclaration : public NStatement {
public:
    const NIdentifier& type;
    const NIdentifier& id;
    VariableList arguments;
    Block& block;
    FunctionDeclaration(const NIdentifier& type,
                        const NIdentifier& id,
                        const VariableList& arguments,
                        Block& block) :
        type(type), id(id), arguments(arguments), block(block)
    {

    }

    virtual Value* codeGen(CodeGenContext& context);
};

class ExternDeclaration : public NStatement
{
public:
    const NIdentifier& type;
    const NIdentifier& id;
    VariableList arguments;

    ExternDeclaration(const NIdentifier& type, const NIdentifier& id, const VariableList& arguments) :
        type(type), id(id), arguments(arguments)
    {

    }

    virtual Value* codeGen(CodeGenContext& context);
};


class IfStatement : public NStatement
{
public:
    Expression *evalExpr;
    Block& thenBlock;
    Block& elseBlock;

    IfStatement(Expression *evalExpr, Block& elseBlock, Block &thenBlock) :
        evalExpr(evalExpr), thenBlock(thenBlock), elseBlock(elseBlock)
    {

    }

    virtual Value* codeGen(CodeGenContext& context);
};

class ReturnStatement : public NStatement
{
public:
    ReturnStatement(Expression *expr):
        expr(expr)
    {

    }

    virtual Value* codeGen(CodeGenContext &context);
private:
    Expression *expr;
};

