#pragma once

#include <stack>
#include <map>

#include "vm.h"

class Block;

class Value
{
public:
    Value(int addr): addr(addr) {}
    int addr;
};

class ConstantValue : public Value
{
public:
    ConstantValue(int value): Value(-1)
    {
        m_value.integer = value;
    }

    union {
        const char *str;
        int integer;
        double dbl;
    } m_value;
};

class Type
{
public:
    Type(int id): id(id) {}
    int id;
};

class BasicBlock
{

};

class Function
{
public:
    int pm_addr;
    std::vector<int> arguments;
};

class CodeGenBlock {
public:
    BasicBlock *block;
    std::map<std::string , Value*> locals;
    std::map<std::string , Function*> functions;
};

class CodeGenContext : public ToyVm {
    std::deque<CodeGenBlock *> blocks;

public:

    CodeGenContext() { }

    void generate_code(Block& root);
    void run_code();

    std::map<std::string, Value*>&locals()
    {
        return blocks.front()->locals;
    }

    std::map<std::string, Function*>&functions()
    {
        return blocks.front()->functions;
    }

    Function* find_function(const std::string &name)
    {
        std::deque<CodeGenBlock*>::iterator it;
        for(it = blocks.begin(); it != blocks.end(); ++it) {
            if ((*it)->functions.count(name))
                return (*it)->functions[name];
        }
        return 0;
    }

    Value* find_variable(const std::string &name)
    {
        std::deque<CodeGenBlock*>::iterator it;
        for(it = blocks.begin(); it != blocks.end(); ++it) {
            if ((*it)->locals.count(name))
                return (*it)->locals[name];
        }
        return 0;
    }

    BasicBlock *current_block()
    {
        return blocks.front()->block;
    }

    void push_block(BasicBlock *block)
    {
        blocks.push_front(new CodeGenBlock());
        blocks.front()->block = block;
    }

    void pop_block()
    {
        CodeGenBlock *top = blocks.front();
        blocks.pop_front();
        delete top;
    }

    std::deque<CodeGenBlock *> &blockStack()
    {
        return blocks;
    }
};
