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
#define NUM_OF_ARRAY(x) (sizeof(x) / sizeof(x[0]))

using namespace Xbyak;

class ToyVm : public Xbyak::CodeGenerator {
    typedef std::vector<uint32> Buffer;
public:
    enum Reg {
        A, B
    };
    enum Code {
        LD, LDI, ST, ADD, MUL, CMP, SUB, SUBI, PUT, JNZ,
        PUSHI, PUSH, PUSHM, POP, POPM,
        CALL, RET, ASSERT, JE, JNE, JMP,
        END_OF_CODE
    };
    ToyVm()
         : m_mark(0)
    {
        ::memset(m_mem, 0, sizeof(m_mem));
    }

    void vpush(uint16 imm)
    {
        encode(PUSHI, A, imm);
    }

    void vpushm(uint16 idx)
    {
        encode(PUSHM, A, idx);
    }

    void vcall(uint16 idx)
    {
        encode(CALL, A, idx);
    }

    void vret()
    {
        encode(RET);
    }

    void vpop(uint16 idx)
    {
        encode(POPM, A, idx);
    }

    void vldi(uint16 imm, Reg r)
    {
        encode(LDI, r, imm);
    }

    void vld(uint16 idx, Reg r)
    {
        encode(LD, r, idx);
    }

    void vst(Reg r, uint16 idx) {
        encode(ST, r, idx);
    }

    void vadd() {
        encode(ADD);
    }

    void vmul() {
        encode(MUL);
    }

    void vassert() {
        encode(ASSERT);
    }

    void vjnz(Reg r, int offset) {
        encode(JNZ, r, static_cast<uint16>(offset));
    }

    void vput(Reg r) {
        encode(PUT, r);
    }

    void vje(uint16 imm) {
        encode(JE, A, imm);
    }

    void vjne(uint16 imm) {
        encode(JNE, A, imm);
    }

    void vjmp(uint16 imm) {
        encode(JMP, A, imm);
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
            printf("%04x ", m_mem[i]);
        printf("\nsp %d : [ ", m_sp);
        for(int i=m_sp+1;i<65536;i++)
            printf("%x ", m_mem[i]);
        printf("]\n");
    }

    void encode(Code code, Reg r = A, uint16 imm = 0)
    {
        uint32 x = (code << 24) | (r << 16) | imm;
        m_code.push_back(x);
    }

    void encode_at(int idx, Code code, Reg r = A, uint16 imm = 0)
    {
        uint32 x = (code << 24) | (r << 16) | imm;
        m_code[idx] = x;
    }

    void print_code(int code, int imm);
private:

    uint32 m_mem[65536];
    uint32 m_callstack[128];
    Buffer m_code;
    int m_sp;
    int m_mark;
    std::map<std::string, int> m_labels;

    void decode(uint32& code, uint32& r, uint32& imm, uint32 x)
    {
        code = x >> 24;
        r = (x >> 16) & 0xff;
        imm = x & 0xffff;

    }
};
