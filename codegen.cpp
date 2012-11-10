#include <typeinfo>
#include "node.h"
#include "codegen.h"
#include "parser.hpp"

using namespace std;

int addr = 0;

/* Compile the AST into a module */
void CodeGenContext::generateCode(Block& root)
{
    std::cout << "Generating code...\n";
    BasicBlock *bblock = new BasicBlock();
    pushBlock(bblock);
    root.codeGen(*this, true);
    //popBlock();
}

/* Executes the AST by running the main function */
void CodeGenContext::runCode()
{
    dump();
    for(std::map<std::string , Value*>::const_iterator it = locals().begin(); it != locals().end(); it++) {
        std::cout << "Local " << it->first << " addr: " << it->second->addr << std::endl;
    }

    for(std::map<std::string , Function*>::const_iterator it = functions().begin(); it != functions().end(); it++) {
        std::cout << "Function " << it->first << " addr: " << it->second->pm_addr << std::endl;
        for(int i = 0; i < it->second->arguments.size(); it++)
            std::cout << "arg " << i << " at " << it->second->arguments[i] << endl;
    }
    run(functions()["main"]->pm_addr);
    //recompile();
    //runByJIT();

}

/* -- Code Generation -- */

Value* NInteger::codeGen(CodeGenContext& context)
{
    std::cout << "Creating integer: " << value << endl;
    context.vpush(value);
    return 0;
}

Value* NDouble::codeGen(CodeGenContext& context)
{
    std::cout << "Creating double: " << value << endl;
}


Value *NString::codeGen(CodeGenContext &context)
{
}

Value* NIdentifier::codeGen(CodeGenContext& context)
{
    std::cout << "Creating identifier reference: " << name << endl;

    Value *v = context.find_variable(name);
    if (!v) {
        std::cout << "Reference to unknown variable " << name << std::endl;
        exit(0);
    }
    context.vpushm(v->addr);
    return 0;
}

Value* NMethodCall::codeGen(CodeGenContext& context)
{
    std::cout << "Calling function " << id.name << std::endl;

    Function *f = context.find_function(id.name);
    if (!f) {
        std::cout << "Error, no matching call to function " << id.name << std::endl;
        exit(0);
    }

    ExpressionList::const_iterator it;
    int i = 0;
    for (it = arguments.begin(); it != arguments.end(); it++) {
        (**it).codeGen(context);
        context.vpop(f->arguments[i++]);
    }
    int idx = f->pm_addr;
    context.vcall(idx);
}

Value* BinaryOperator::codeGen(CodeGenContext& context)
{
    std::cout << "Creating binary operation " << endl;

    lhs.codeGen(context);
    rhs.codeGen(context);

    switch (op) {
        case TPLUS:
          context.vadd();
          break;
        case TMINUS:
          break;
        case TMUL:
          context.vmul();
          break;
        case TDIV:
          break;
        break;
        /* TODO comparison */
    }
    return 0;

}

Value* Assignment::codeGen(CodeGenContext& context)
{
    std::cout << "Creating assignment for " << lhs.name << endl;
    if (context.locals().find(lhs.name) == context.locals().end()) {
        std::cerr << "undeclared variable " << lhs.name << endl;
        return NULL;
    }

    rhs.codeGen(context);
    context.vpop(context.locals()[lhs.name]->addr);
    return context.locals()[lhs.name];
}


Value* Block::codeGen(CodeGenContext& context, int local_funcs_only)
{
    StatementList::const_iterator it;
    Value *last = NULL;
    for (it = statements.begin(); it != statements.end(); it++) {
        if (local_funcs_only && typeid(**it) == typeid(FunctionDeclaration)) {
            std::cout << "Generating code for " << typeid(**it).name() << endl;
            last = (**it).codeGen(context);
        }
        if (!local_funcs_only && typeid(**it) != typeid(FunctionDeclaration)) {
            std::cout << "Generating code for " << typeid(**it).name() << endl;
            last = (**it).codeGen(context);
        }
    }

    std::cout << "Creating block" << endl;
    return last;
}

Value* ExpressionStatement::codeGen(CodeGenContext& context)
{
    std::cout << "Generating code for " << typeid(expression).name() << endl;
    return expression.codeGen(context);
}

Value* VariableDeclaration::codeGen(CodeGenContext& context)
{
    std::cout << "Creating variable declaration " << type.name << " " << id.name << endl;

    Value *v = new Value(addr++);
    context.locals()[id.name] = v;
    if (assignmentExpr != NULL) {
        Assignment assn(id, *assignmentExpr);
        assn.codeGen(context);
    }
    return v;
}

Value* ExternDeclaration::codeGen(CodeGenContext& context)
{
    VariableList::const_iterator it;
    for (it = arguments.begin(); it != arguments.end(); it++) {
       cout << (**it).type.name << endl;
    }

    std::cout << "Creating external function: " << id.name << endl;
    return 0;
}

Value* FunctionDeclaration::codeGen(CodeGenContext& context)
{
    BasicBlock *bblock = new BasicBlock();

    Function *f = new Function();

    context.pushBlock(bblock);
    VariableList::const_iterator it;
    for (it = arguments.begin(); it != arguments.end(); it++) {
        cout << "Argument : " << (**it).type.name ;
        Value *v = (**it).codeGen(context);
        cout << " at: " << v->addr << endl;
        f->arguments.push_back(v->addr);
    }
    block.codeGen(context, true);
    f->pm_addr = context.getCurrent();
    block.codeGen(context, false);
    context.vret();
    context.popBlock();

    context.functions().insert(std::make_pair(id.name, f));

    std::cout << "Creating function: " << id.name << endl;

    return 0;
}

Value* IfStatement::codeGen(CodeGenContext& context)
{
   return 0;
}


Value *ReturnStatement::codeGen(CodeGenContext &context)
{
    expr->codeGen(context);
}
