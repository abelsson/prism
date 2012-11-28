#include <typeinfo>
#include "node.h"
#include "codegen.h"
#include "parser.hpp"

using namespace std;

int addr = 0;
int debug = 0;

/* Compile the AST into a module */
void CodeGenContext::generateCode(Block& root)
{
    if (debug)
        std::cout << "Generating code...\n";
    BasicBlock *bblock = new BasicBlock();
    pushBlock(bblock);
    root.codeGen(*this, true);
    //popBlock();
}

/* Executes the AST by running the main function */
void CodeGenContext::runCode()
{
    if (debug) {
        dump();
        for(std::map<std::string , Value*>::const_iterator it = locals().begin(); it != locals().end(); it++) {
            std::cout << "Local " << it->first << " addr: " << it->second->addr << std::endl;
        }

        for(std::map<std::string , Function*>::const_iterator it = functions().begin(); it != functions().end(); it++) {
            std::cout << "Function " << it->first << " addr: " << it->second->pm_addr << std::endl;
            for(int i = 0; i < it->second->arguments.size(); it++)
                std::cout << "arg " << i << " at " << it->second->arguments[i] << endl;
        }
    }
    run(functions()["main"]->pm_addr);
    //recompile();
    //runByJIT();

}

void Integer::codeGen(CodeGenContext& context)
{
    if (debug)
        std::cout << "Creating integer: " << m_value << endl;
    context.vpush(m_value);
}

void Double::codeGen(CodeGenContext& context)
{
    if (debug)
        std::cout << "Creating double: " << value << endl;
}


void String::codeGen(CodeGenContext &context)
{
}

void Identifier::codeGen(CodeGenContext& context)
{
    if (debug)
        std::cout << "Creating identifier reference: " << name << endl;

    Value *v = context.find_variable(name);
    if (!v) {
        std::cout << "Reference to unknown variable " << name << std::endl;
        exit(0);
    }
    context.vpushm(v->addr);
}

void MethodCall::codeGen(CodeGenContext& context)
{
    if (debug)
        std::cout << "Calling function " << id->name << std::endl;

    Function *f = context.find_function(id->name);
    if (!f) {
        std::cout << "Error, no matching call to function " << id->name << std::endl;
        exit(0);
    }

    ExpressionList::const_iterator it;
    int i = 0;
    for (it = arguments->begin(); it != arguments->end(); it++) {
        (**it).codeGen(context);
        context.vpop(f->arguments[i++]);
    }
    int idx = f->pm_addr;
    context.vcall(idx);
}

void BinaryOperator::print(int indent) const {
    PRINT_ID("binop: ");
    switch (op) {
    case TPLUS:
        printf("+");
        break;
    case TMINUS:
        break;
    case TMUL:
        printf("*");

        break;
    case TDIV:
        break;
    case TCEQ:
        printf("==");
        break;
        /* TODO comparison */
    }
    printf("\n");
    printf("%*s lhs:\n", indent, ""); lhs->print(indent+1);
    printf("%*s rhs:\n", indent, ""); rhs->print(indent+1);
}
void BinaryOperator::codeGen(CodeGenContext& context)
{
    if (debug)
        std::cout << "Creating binary operation " << endl;

    lhs->codeGen(context);
    rhs->codeGen(context);

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
    case TCEQ:
        context.encode(ToyVm::CMP);
        break;
        /* TODO comparison */
    }

}

void Assignment::codeGen(CodeGenContext& context)
{
    if (debug)
        std::cout << "Creating assignment for " << lhs->name << endl;
    if (context.locals().find(lhs->name) == context.locals().end()) {
        std::cerr << "undeclared variable " << lhs->name << endl;
        return;
    }

    rhs->codeGen(context);
    context.vpop(context.locals()[lhs->name]->addr);
}


void Block::codeGen(CodeGenContext& context, int local_funcs_only)
{
    StatementList::const_iterator it;
    for (it = statements.begin(); it != statements.end(); it++) {
        if (local_funcs_only && typeid(**it) == typeid(FunctionDeclaration)) {
            if (debug)
                std::cout << "Generating code func for " << typeid(**it).name() << endl;
            (**it).codeGen(context);
        }
        if (!local_funcs_only && typeid(**it) != typeid(FunctionDeclaration)) {
            if (debug)
                std::cout << "Generating code for " << typeid(**it).name() << endl;
            (**it).codeGen(context);
        }
    }

    if (debug)
        std::cout << "Creating block" << endl;
}

void ExpressionStatement::codeGen(CodeGenContext& context)
{
    if (debug)
        std::cout << "Generating code for " << typeid(expression).name() << endl;
    return expression.codeGen(context);
}

void VariableDeclaration::codeGen(CodeGenContext& context)
{
    if (debug)
        std::cout << "Creating variable declaration " << type->name << " " << id->name << endl;

    context.locals()[id->name] = value();
    if (assignmentExpr != NULL) {
        Assignment assn(id, assignmentExpr);
        assn.codeGen(context);
    }
}

void ExternDeclaration::codeGen(CodeGenContext& context)
{
    VariableList::const_iterator it;
    for (it = arguments->begin(); it != arguments->end(); it++) {
       cout << (**it).type->name << endl;
    }

    if (debug)
        std::cout << "Creating external function: " << id->name << endl;
}

void FunctionDeclaration::codeGen(CodeGenContext& context)
{
    BasicBlock *bblock = new BasicBlock();

    Function *f = new Function();

    context.pushBlock(bblock);
    VariableList::const_iterator it;
    for (it = arguments->begin(); it != arguments->end(); it++) {
        if (debug)
            cout << "Argument : " << (**it).type->name ;
        Value *v = (*it)->value();
        (**it).codeGen(context);
        if (debug)
            cout << " at: " << v->addr << endl;
        f->arguments.push_back(v->addr);
    }
    block->codeGen(context, true);
    f->pm_addr = context.getCurrent();
    block->codeGen(context, false);
    context.vret();
    context.popBlock();

    context.functions().insert(std::make_pair(id->name, f));

    if (debug)
        std::cout << "Creating function: " << id->name << endl;
}

void IfStatement::codeGen(CodeGenContext& context)
{
    int then_end;
    evalExpr->codeGen(context);
    int then_start = context.reserve();
    thenBlock->codeGen(context, false);

    if (elseBlock) {
        then_end = context.reserve();
        elseBlock->codeGen(context ,false);
    }

    int block_end = context.getCurrent();

    if (elseBlock) {
        context.encode_at(then_end, CodeGenContext::JMP, CodeGenContext::A, block_end);
        context.encode_at(then_start, CodeGenContext::JNE, CodeGenContext::A, then_end);
    } else {
        context.encode_at(then_start, CodeGenContext::JNE, CodeGenContext::A, block_end);
    }
}



void ReturnStatement::codeGen(CodeGenContext &context)
{
    expr->codeGen(context);
}


void AssertStatement::codeGen(CodeGenContext &context)
{
    expr->codeGen(context);
    context.vassert();
}

void PrintStatement::codeGen(CodeGenContext &context)
{
    expr->codeGen(context);
    context.vprint();
}

