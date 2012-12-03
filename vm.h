#include <iostream>
/*
    toy vm
    register A, B : 32bit
    PC : program counter

    mem_  4byte x 65536

    R = A or B
    vldiR, imm  ; R = imm
    vldR, idx   ; R = mem_[idx]
    vstR, idx   ; mem_[idx] = R
    vaddiR, imm ; R += imm
    vsubiR, imm ; R -= imm
    vaddR, idx  ; R += mem_[idx]
    vsubR, idx  ; R -= mem_[idx]
    vputR       ; print R
    vjnzR, offset; if (R != 0) then jmp(PC += offset(signed))
*/
#if defined(_MSC_VER) && (_MSC_VER <= 1200)
    #pragma warning(disable:4514)
    #pragma warning(disable:4786)
#endif
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <vector>

#include "xbyak/xbyak.h"
#include "xbyak/xbyak_util.h"

#include "type.h"
#define NUM_OF_ARRAY(x) (sizeof(x) / sizeof(x[0]))

using namespace Xbyak;

struct IValue;

struct IValue {
    union {
        int int_value;
        double float_value;
        const std::string *str_value;
        std::list<IValue> *list_value;
        std::list<IValue>::iterator *list_iter;
    };
};

class ToyVm : public Xbyak::CodeGenerator {
    typedef std::vector<uint32> Buffer;
public:
    enum Code {
        LD, LDI, ST, ADD, MUL, DIV, SUB,
        AND,
        CMP, CLT,
        PUSHI, PUSH, PUSHM, POP, POPM,
        PUSH_CONSTANT,
        MAKE_LIST, MAKE_ITER, LOOP_ITER, ITER_VALUE,
        CALL, RET,
        ASSERT, PRINT,
        JE, JNE, JMP,
        END_OF_CODE
    };

    ToyVm()
         : m_mark(0)
    {
        m_mem = new IValue[65536];
    }

    void vpush(uint16 imm)
    {
        encode(PUSHI, imm);
    }

    void vpush_constant(uint16 imm)
    {
        encode(PUSH_CONSTANT, imm);
    }

    void vpushm(uint16 idx)
    {
        encode(PUSHM, idx);
    }

    void vcall(uint16 idx)
    {
        encode(CALL, idx);
    }

    void vret()
    {
        encode(RET);
    }

    void vpop(uint16 idx)
    {
        encode(POPM, idx);
    }

    void vmake_list(int num)
    {
        encode(MAKE_LIST, num);
    }

    void vmake_iter()
    {
        encode(MAKE_ITER);
    }

    void vloop_iter(int pc)
    {
        encode(LOOP_ITER, pc);
    }

    void viter_value()
    {
        encode(ITER_VALUE);
    }

    void vadd(::Type t) {
        encode(ADD, t);
    }

    void vmul(::Type t) {
        encode(MUL, t);
    }

    void vassert() {
        encode(ASSERT);
    }

    void vprint(::Type t) {
        encode(PRINT, t);
    }

    void vje(uint16 imm) {
        encode(JE, imm);
    }

    void vjne(uint16 imm) {
        encode(JNE, imm);
    }

    void vjmp(uint16 imm) {
        encode(JMP,imm);
    }

    void set_label(const std::string& id) {
        m_labels.insert(std::make_pair(id, getCurrent()));
    }

    int reserve() {
        m_code.push_back(0);
        return m_code.size()-1;
    }

    void setMark();
    int getMarkOffset();
    int getCurrent();

    void run(int start_pc);
    void recompile();

    void dump();

    void runByJIT()
    {
        printf("Size = %ld\n", getSize());
        {
          FILE *fp = fopen("distmp.tmp", "wb");
          fwrite(getCode(), 1, getSize(), fp);
          fclose(fp);
          system("objdump -m i386 -M x86-64,intel -b binary -D distmp.tmp; rm distmp.tmp");
        }
        ((void (*)())(const void*)getCode())();


        printf("---\n");
        printf("m_mem[] = ");
        for(int i=0;i<8;i++)
            printf("%04x ", m_mem[i].int_value);
        printf("\nsp %d : [ ", m_sp);
        for(int i=m_sp+1;i<65536;i++)
            printf("%x ", m_mem[i].int_value);
        printf("]\n");
    }

    void encode(Code code, uint16 imm)
    {
        uint32 x = (code << 24) | (0 << 16) | imm;
        m_code.push_back(x);
    }

    void encode(Code code, ::Type t = ::Type::UNKNOWN, uint16 imm = 0)
    {
        uint32 x = (code << 24) | (t.id << 16) | imm;
        m_code.push_back(x);
    }


    void encode_at(int idx, Code code, uint16 imm)
    {
        uint32 x = (code << 24) | (0 << 16) | imm;
        m_code[idx] = x;
    }

    void encode_at(int idx, Code code, ::Type t = ::Type::UNKNOWN, uint16 imm = 0)
    {
        uint32 x = (code << 24) | (t.id << 16) | imm;
        m_code[idx] = x;
    }

    void print_code(int code, int imm);
    int add_constant(const std::string* str)
    {
        IValue value;
        value.str_value = str;
        m_constants.push_back(value);
        printf("Add string constant at %ld\n", m_constants.size() - 1);
        return m_constants.size() - 1;
    }

    int add_constant(double d)
    {
        IValue value;
        value.float_value = d;
        m_constants.push_back(value);
        printf("Add double constant at %ld\n", m_constants.size() - 1);
        return m_constants.size() - 1;
    }

private:

    IValue* m_mem;

    std::vector<IValue> m_constants;

    uint32 m_callstack[128];
    Buffer m_code;
    int m_sp;
    int m_mark;
    std::map<std::string, int> m_labels;

    void decode(uint32& code, int& t, uint32& imm, uint32 x)
    {
        code = x >> 24;
        t = (x >> 16) & 0xff;
        imm = x & 0xffff;

    }
};
