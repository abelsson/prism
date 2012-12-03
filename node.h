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
extern int line_num;

#define PRINT_ID(x) printf("%*s %s", indent, "", x );
#define INDENT printf("%*s", indent, "");

class Node;

class Visitor
{
  public:
    virtual void visit(Node*) = 0;
};

class Context;

class Context
{
public:
    Context(Context* parent): m_parent(parent) {}

    std::map<std::string , Type> m_locals;
    std::map<std::string , Type> m_functions;
    Context* m_parent;
};

class Node {
public:
    Node()
    {
        m_line_num = line_num;
    }

    virtual ~Node() {}
    virtual Value *value() { return NULL; }
    virtual Type type() const = 0;
    virtual YAML::Node yaml() const = 0;
    virtual void codeGen(CodeGenContext& context) { }
    virtual void set_context(Context* parent) = 0;

    virtual void accept(Visitor* v)
    {
        v->visit(this);
        for(auto it : m_children)
            it->accept(v);
    }

    void add_child(Node* child) { m_children.push_back(child); }
protected:
    Context* m_context;
    int m_line_num;
    std::list<Node*> m_children;
};

class Expression : public Node {
public:
};

class Statement : public Node {
public:
};

class Integer : public Expression {
public:
    long long m_value;

    Value *value() { return new Value(m_value); }
    Integer(long long value) : m_value(value) { }

    Type type() const
    {
        return Type::INT;
    }

    YAML::Node yaml() const
    {
        YAML::Node node;
        node["id"] = "integer";
        node["value"] = m_value;
        return node;
    }

    void set_context(Context* parent)
    {
        m_context = parent;
    }

    void codeGen(CodeGenContext& context);
};

class Double : public Expression {
public:
    double m_value;
    Double(double value) : m_value(value) { }

    Type type() const
    {
        return Type::DOUBLE;
    }

    YAML::Node yaml() const
    {
        YAML::Node node;
        node["id"] = "double";
        node["value"] = m_value;
        return node;
    }

    void set_context(Context* parent)
    {
        m_context = parent;
    }

    void codeGen(CodeGenContext& context);
};

class String : public Expression {
public:
    std::string m_value;
    String(const std::string* value) :
        m_value(*value)
    {
        m_value = m_value.substr(1, m_value.length() - 2);
    }

    Type type() const
    {
        return Type::STRING;
    }

    YAML::Node yaml() const
    {
        YAML::Node node;
        node["id"] = "string";
        node["value"] = m_value;
        return node;
    }

    void set_context(Context* parent)
    {
        m_context = parent;
    }

    void codeGen(CodeGenContext& context);

};

class Typename : public Expression {
public:
    std::string m_name;
    Typename(const std::string& name) : m_name(name) { }

    Type type() const
    {
        if (m_name == "Int")
            return Type::INT;
        else if (m_name == "String")
            return Type::STRING;
        else if (m_name == "Double")
            return Type::DOUBLE;
        else if (m_name == "List")
            return Type::LIST;

        return Type::UNKNOWN;
    }

    YAML::Node yaml() const
    {
        YAML::Node node;
        node["id"] = "typename";
        node["value"] = m_name;
        return node;
    }

    void set_context(Context* parent)
    {
        m_context = parent;
    }

    void codeGen(CodeGenContext& context) {};
};

class ListLiteral : public Expression {

    std::vector<Expression*> m_elements;
public:
    ListLiteral()
    {
        for(auto it : m_elements)
            add_child(it);
    }

    Type type() const
    {
        return Type::UNKNOWN;
    }

    YAML::Node yaml() const
    {
        YAML::Node node;
        node["id"] = "listliteral";
        YAML::Node elements = node["elements"];
        for(auto it : m_elements)
            elements.push_back(it->yaml());
        return node;
    }

    void set_context(Context* parent)
    {
        m_context = parent;
        for(auto it : m_elements)
            it->set_context(m_context);
    }

    void add(Expression *x)
    {
        m_elements.push_back(x);
    }

    void codeGen(CodeGenContext& context);
};

class Identifier : public Expression {
public:
    std::string m_name;
    Identifier(const std::string& name) : m_name(name) { }
    void print(int indent) const
    {
        printf("%*s %s\n", indent, "", m_name.c_str());
    }

    Type type() const
    {
        std::cout << "Locals: " << m_context->m_locals.size() << std::endl;

        Context *ctx = m_context;
        while (ctx) {
            if (ctx->m_locals.count(m_name)) {
                std::cout << " type= " << m_name << " "  << ctx->m_locals[m_name].name() << std::endl;
                return ctx->m_locals[m_name];
            }
            ctx = ctx->m_parent;
        }

        return Type::UNKNOWN;

    }

    YAML::Node yaml() const
    {
        YAML::Node node;
        node["id"] = "identifier";
        node["context"] = (size_t)m_context;
        node["type"] = type().name();
        node["value"] = m_name;
        return node;
    }

    void set_context(Context* parent)
    {
        m_context = parent;
    }

    void codeGen(CodeGenContext& context);
};

class MethodCall : public Expression {
public:
    Identifier* m_id;

    ExpressionList* m_arguments;

    MethodCall(Identifier* id, ExpressionList* arguments) :
        m_id(id), m_arguments(arguments)
    {
        add_child(m_id);
        for(auto it : *m_arguments)
            add_child(it);
    }
    MethodCall(Identifier* id) : m_id(id)
    {
        add_child(m_id);
    }

    void codeGen(CodeGenContext& context);

    Type type() const
    {
        std::string name = m_id->m_name;
        std::cout << "Locals: " << m_context->m_functions.size() << std::endl;

        Context *ctx = m_context;
        while (ctx) {
            if (ctx->m_functions.count(name)) {
                std::cout << " type= " << name << " "  << ctx->m_functions[name].name() << std::endl;
                return ctx->m_functions[name];
            }
            ctx = ctx->m_parent;
        }

        return Type::UNKNOWN;
    }

    void set_context(Context* parent)
    {
        m_context = parent;
        m_id->set_context(parent);
        for(auto it : *m_arguments)
            it->set_context(parent);
    }

    YAML::Node yaml() const
    {
        YAML::Node node;
        node["id"] = "methodcall";

        YAML::Node args = node["arguments"];
        for(auto it : *m_arguments)
            args.push_back(it->yaml());

        node["value"] = m_id->yaml();

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
        add_child(lhs);
        add_child(rhs);
    }
    void print(int indent) const;
    void codeGen(CodeGenContext& context);

    Type type() const;

    YAML::Node yaml() const
    {
        YAML::Node node;
        node["id"] = "binop";
        node["left"] = lhs->yaml();
        node["right"] = rhs->yaml();
        return node;
    }

    void set_context(Context* parent)
    {
        m_context = parent;
        lhs->set_context(parent);
        rhs->set_context(parent);
    }

};

class Assignment : public Expression {
public:
    Identifier* lhs;
    Expression* rhs;
    Assignment(Identifier* lhs, Expression* rhs) :
        lhs(lhs), rhs(rhs)
    {
        add_child(lhs);
        add_child(rhs);
    }

    void codeGen(CodeGenContext& context);

    Type type() const
    {
        return lhs->type();
    }

    YAML::Node yaml() const
    {
        YAML::Node node;
        node["id"] = "assign";
        node["left"] = lhs->yaml();
        node["right"] = rhs->yaml();
        return node;
    }

    void set_context(Context* parent)
    {
        m_context = parent;
        lhs->set_context(m_context);
        rhs->set_context(m_context);
    }

};

class Block : public Expression {
    StatementList statements;
public:
    Block()
    {
        for(auto it : statements)
            add_child(it);
    }

    void add_statement(Statement *stmt)
    {
        statements.push_back(stmt);
        add_child(stmt);
    }

    void codeGen(CodeGenContext& context, int local_funcs_only);

    Type type() const
    {
        return Type::VOID;
    }

    YAML::Node yaml() const
    {
        YAML::Node node;
        node["id"] = "block";
        node["context"] = (size_t)m_context;
        YAML::Node stmnts = node["statements"];
        for(auto it : statements)
            stmnts.push_back(it->yaml());
        return node;
    }

    void set_context(Context* parent)
    {
        m_context = new Context(parent);
        for(auto it : statements)
            it->set_context(m_context);
    }

};

class ExpressionStatement : public Statement {
public:
    Expression& m_expression;
    ExpressionStatement(Expression& expression) :
        m_expression(expression) { }

    Type type() const
    {
        return m_expression.type();
    }

    YAML::Node yaml() const
    {
        YAML::Node node;
        node["id"] = "expression";
        node["expression"] = m_expression.yaml();
        return node;
    }

    void set_context(Context* parent)
    {
        m_context = new Context(parent);
        m_expression.set_context(parent);
    }

    void codeGen(CodeGenContext& context);
};


class VariableDeclaration : public Statement {
public:
    Typename* m_type;
    Identifier* m_name;
    Expression* m_assignment_expr;
    Value *v;
    VariableDeclaration(Typename* type, Identifier* id) :
        m_type(type), m_name(id), v(0)
    {
        add_child(m_type);
        add_child(m_name);
    }

    VariableDeclaration(Typename* type, Identifier* id, Expression *assignmentExpr) :
        m_type(type), m_name(id), m_assignment_expr(assignmentExpr), v(0)
    {
        add_child(m_type);
        add_child(m_name);
        add_child(m_assignment_expr);
    }

    Value *value()
    {
        if (v == NULL)
            v = new Value(addr++);
        return v;
    }

    Type type() const
    {
        m_context->m_locals[m_name->m_name] = m_type->type();
        return Type::VOID;
    }

    YAML::Node yaml() const
    {
        YAML::Node node;
        node["id"] = "vardecl";
        node["variable-type"] = m_type->yaml();
        node["identifier"] = m_name->yaml();
        if (m_assignment_expr)
            node["assign"] = m_assignment_expr->yaml();
        return node;
    }

    void set_context(Context* parent)
    {
        m_context = parent;
        m_type->set_context(m_context);
        m_name->set_context(m_context);
        if (m_assignment_expr)
            m_assignment_expr->set_context(m_context);
    }

    void codeGen(CodeGenContext& context);
};

class FunctionDeclaration : public Statement {
public:
    Typename* m_type;
    Identifier* m_id;
    VariableList* m_arguments;
    Block* m_block;
    FunctionDeclaration(Typename* type,
                        Identifier* id,
                        VariableList* arguments,
                        Block* block) :
        m_type(type), m_id(id), m_arguments(arguments), m_block(block)
    {
        add_child(m_type);
        add_child(m_id);
        add_child(m_block);
        for(auto it : *m_arguments)
            add_child(it);
    }

    void codeGen(CodeGenContext& context);

    Type type() const
    {
        m_context->m_functions[m_id->m_name] = m_type->type();
        return m_type->type();
    }

    YAML::Node yaml() const
    {
        YAML::Node node;
        node["id"] = "function";
        node["name"] = m_id->yaml();
        node["return-type"] = m_type->yaml();
        YAML::Node args = node["arguments"];
        for(auto it : *m_arguments)
            args.push_back(it->yaml());

        node["block"] = m_block->yaml();

        return node;
    }

    void set_context(Context* parent)
    {
        m_context = parent;
        m_type->set_context(m_context);
        m_id->set_context(m_context);
        m_block->set_context(m_context);
        for(auto it : *m_arguments)
            it->set_context(m_context);
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
    Expression *m_eval_expr;
    Block* m_then_block;
    Block* m_else_block;

    IfStatement(Expression *evalExpr, Block* thenBlock, Block *elseBlock) :
        m_eval_expr(evalExpr), m_then_block(thenBlock), m_else_block(elseBlock)
    {
        add_child(m_eval_expr);
        add_child(m_then_block);
        if (m_else_block)
            add_child(m_else_block);
    }

    Type type() const
    {
        return Type::VOID;
    }

    void codeGen(CodeGenContext& context);
    YAML::Node yaml() const
    {
        YAML::Node node;
        node["id"] = "if";
        node["expr"] = m_eval_expr->yaml();
        node["true-branch"] = m_then_block->yaml();
        if (m_else_block)
            node["false-branch"] = m_else_block->yaml();
        return node;
    }

    void set_context(Context* parent)
    {
        m_context = parent;
        m_eval_expr->set_context(parent);
        m_then_block->set_context(parent);
        if (m_else_block)
            m_else_block->set_context(parent);
    }

};

class ReturnStatement : public Statement
{
public:
    ReturnStatement(Expression *expr):
        m_expr(expr)
    {
        add_child(m_expr);
    }

    Type type() const
    {
        return Type::VOID;
    }

    void codeGen(CodeGenContext &context);
    YAML::Node yaml() const
    {
        YAML::Node node;
        node["id"] = "return";
        node["expr"] = m_expr->yaml();
        return node;
    }

    void set_context(Context* parent)
    {
        m_context = parent;
        m_expr->set_context(parent);
    }

private:
    Expression *m_expr;
};


class AssertStatement : public Statement
{
public:
    AssertStatement(Expression *expr):
        m_expr(expr)
    {
        add_child(m_expr);
    }


    Type type() const
    {
        return Type::VOID;
    }

    void codeGen(CodeGenContext &context);
    YAML::Node yaml() const
    {
        YAML::Node node;
        node["id"] = "assert";
        node["expr"] = m_expr->yaml();
        return node;
    }

    void set_context(Context* parent)
    {
        m_context = parent;
        m_expr->set_context(parent);
    }

private:
    Expression *m_expr;
};

class PrintStatement : public Statement
{
public:
    PrintStatement(Expression *expr):
        m_expr(expr)
    {
        add_child(m_expr);
    }

    Type type() const
    {
        return Type::VOID;
    }

    void codeGen(CodeGenContext &context);
    YAML::Node yaml() const
    {
        YAML::Node node;
        node["id"] = "print";
        node["expr"] = m_expr->yaml();
        return node;
    }

    void set_context(Context* parent)
    {
        m_context = parent;
        m_expr->set_context(parent);
    }

private:
    Expression *m_expr;
};


class ForeachStatement : public Statement
{
    VariableDeclaration *m_var_decl;
    Expression *m_expr;
    Block *m_block;
public:
    ForeachStatement(VariableDeclaration *var, Expression *expr, Block *block):
        m_var_decl(var), m_expr(expr), m_block(block)
    {
        add_child(m_var_decl);
        add_child(m_expr);
        add_child(m_block);
    }

    Type type() const
    {
        return Type::VOID;
    }

    void codeGen(CodeGenContext &context);

    YAML::Node yaml() const
    {
        YAML::Node node;
        node["id"] = "foreach";
        node["var"] = m_var_decl->yaml();
        node["iter"] = m_expr->yaml();
        node["block"] = m_block->yaml();
        return node;
    }

    void set_context(Context* parent)
    {
        m_context = parent;
        m_var_decl->set_context(parent);
        m_expr->set_context(parent);
        m_block->set_context(parent);
    }
};

class WhileStatement : public Statement
{
    Expression *m_expr;
    Block *m_block;
public:
    WhileStatement(Expression *expr, Block *block):
       m_expr(expr), m_block(block)
    {
        add_child(m_expr);
        add_child(m_block);
    }

    Type type() const
    {
        return Type::VOID;
    }

    void codeGen(CodeGenContext &context);

    YAML::Node yaml() const
    {
        YAML::Node node;
        node["id"] = "foreach";
        node["iter"] = m_expr->yaml();
        node["block"] = m_block->yaml();
        return node;
    }

    void set_context(Context* parent)
    {
        m_context = parent;
        m_expr->set_context(parent);
        m_block->set_context(parent);
    }
};

class TypeVisitor : public Visitor
{
public:
    void visit(Node* n)
    {
        n->type();
    }
};

class PtrVisitor : public Visitor
{
public:
    void visit(Node* n)
    {
        std::cout << n << std::endl;
    }
};

class SetContextVisitor : public Visitor
{
public:
    void visit(Node *)
    {

    }
};
