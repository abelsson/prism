#include <typeinfo>
#include "node.h"
#include "codegen.h"
#include "parser.hpp"

using namespace std;

int addr = 0;
int debug = 0;

/* Compile the AST into a module */
void CodeGenContext::generate_code(Block& root)
{
    if (debug)
        std::cout << "Generating code...\n";
    BasicBlock *bblock = new BasicBlock();
    push_block(bblock);
    root.codeGen(*this, true);
    //popBlock();
}

/* Executes the AST by running the main function */
void CodeGenContext::run_code()
{
    if (1) {
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
        std::cout << "Creating double: " << m_value << endl;
}


void String::codeGen(CodeGenContext &context)
{
    int ref_id = context.add_constant(&m_value);
    context.vpushs(ref_id);
}

void ListLiteral::codeGen(CodeGenContext& context)
{
    int count = 0;
    for(auto it : m_elements) {
        it->codeGen(context);
        count++;
    }
    context.vmake_list(count);
}


void Identifier::codeGen(CodeGenContext& context)
{
    if (debug)
        std::cout << "Creating identifier reference: " << m_name << endl;

    Value *v = context.find_variable(m_name);
    if (!v) {
        std::cout << "Reference to unknown variable " << m_name << std::endl;
        exit(0);
    }
    context.vpushm(v->addr);
}

void MethodCall::codeGen(CodeGenContext& context)
{
    if (debug)
        std::cout << "Calling function " << m_id->m_name << std::endl;

    Function *f = context.find_function(m_id->m_name);
    if (!f) {
        std::cout << "Error, no matching call to function " << m_id->m_name << std::endl;
        exit(0);
    }

    ExpressionList::const_iterator it;
    int i = 0;
    for (it = m_arguments->begin(); it != m_arguments->end(); it++) {
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
        std::cout << "Creating assignment for " << lhs->m_name << endl;
    if (context.locals().find(lhs->m_name) == context.locals().end()) {
        std::cerr << "undeclared variable " << lhs->m_name << endl;
        return;
    }

    rhs->codeGen(context);
    context.vpop(context.locals()[lhs->m_name]->addr);
}


void Block::codeGen(CodeGenContext& context, int local_funcs_only)
{
    for (auto it : statements) {
        if (local_funcs_only && typeid(*it) == typeid(FunctionDeclaration)) {
            if (debug)
                std::cout << "Generating code func for " << typeid(*it).name() << endl;
            (*it).codeGen(context);
        }
        if (!local_funcs_only && typeid(*it) != typeid(FunctionDeclaration)) {
            if (debug)
                std::cout << "Generating code for " << typeid(*it).name() << endl;
            (*it).codeGen(context);
        }
    }

    if (debug)
        std::cout << "Creating block" << endl;
}

#if 0
void ExpressionStatement::codeGen(CodeGenContext& context)
{
    if (debug)
        std::cout << "Generating code for " << typeid(expression).name() << endl;
    return expression.codeGen(context);
}
#endif

void VariableDeclaration::codeGen(CodeGenContext& context)
{
    if (debug)
        std::cout << "Creating variable declaration " << m_type->m_name << " " << m_name->m_name << endl;

    context.locals()[m_name->m_name] = value();
    if (m_assignment_expr != NULL) {
        Assignment assn(m_name, m_assignment_expr);
        assn.codeGen(context);
    }
}

#if 0
void ExternDeclaration::codeGen(CodeGenContext& context)
{
    VariableList::const_iterator it;
    for (it = arguments->begin(); it != arguments->end(); it++) {
       cout << (**it).type->name << endl;
    }

    if (debug)
        std::cout << "Creating external function: " << id->name << endl;
}
#endif

void FunctionDeclaration::codeGen(CodeGenContext& context)
{
    BasicBlock *bblock = new BasicBlock();

    Function *f = new Function();

    context.push_block(bblock);
    VariableList::const_iterator it;
    for (it = m_arguments->begin(); it != m_arguments->end(); it++) {
        if (debug)
            cout << "Argument : " << (**it).m_type->m_name ;
        Value *v = (*it)->value();
        (**it).codeGen(context);
        if (debug)
            cout << " at: " << v->addr << endl;
        f->arguments.push_back(v->addr);
    }
    m_block->codeGen(context, true);
    f->pm_addr = context.getCurrent();
    m_block->codeGen(context, false);
    context.vret();
    context.pop_block();

    context.functions().insert(std::make_pair(m_id->m_name, f));

    if (debug)
        std::cout << "Creating function: " << m_id->m_name << endl;
}

void IfStatement::codeGen(CodeGenContext& context)
{
    int then_end;
    m_eval_expr->codeGen(context);
    int then_start = context.reserve();
    m_then_block->codeGen(context, false);

    if (m_else_block) {
        then_end = context.reserve();
        m_else_block->codeGen(context ,false);
    }

    int block_end = context.getCurrent();

    if (m_else_block) {
        context.encode_at(then_end, CodeGenContext::JMP, CodeGenContext::A, block_end);
        context.encode_at(then_start, CodeGenContext::JNE, CodeGenContext::A, then_end);
    } else {
        context.encode_at(then_start, CodeGenContext::JNE, CodeGenContext::A, block_end);
    }
}



void ReturnStatement::codeGen(CodeGenContext &context)
{
    m_expr->codeGen(context);
}


void AssertStatement::codeGen(CodeGenContext &context)
{
    m_expr->codeGen(context);
    context.vassert();
}

void PrintStatement::codeGen(CodeGenContext &context)
{
    m_expr->codeGen(context);

    switch(m_expr->type().id)
    {
    case Type::INT:
        context.vprint();
        break;
    case Type::STRING:
        context.vprints();
        break;
    case Type::LIST:
        context.vprintl();
        break;
    }
}


void ForeachStatement::codeGen(CodeGenContext &context)
{
    m_var_decl->codeGen(context);
    m_expr->codeGen(context);
    context.vmake_iter();
    int pos = context.getCurrent();
    context.viter_value();
    context.vpop(context.locals()[m_var_decl->m_name->m_name]->addr);
    m_block->codeGen(context, false);
    context.vloop_iter(pos);
}
