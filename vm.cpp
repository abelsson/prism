#include <iostream>
#include "vm.h"

extern int debug;

std::string replace_all(const std::string &str, const std::string &search_val, const std::string &replace_val)
{
    size_t start = 0;
    std::string result = str;
    while(1) {
        size_t where = result.find(search_val, start);
        if(where == std::string::npos) {
            break;
        }
        result.replace(where, search_val.size(), replace_val);
        start = where + replace_val.size();
    }
    return result;
}



void ToyVm::setMark()
{
    m_mark = (int)m_code.size();
}

int ToyVm::getMarkOffset()
{
    return m_mark - (int)m_code.size() - 1;
}

int ToyVm::getCurrent()
{
    return m_code.size();
}

void ToyVm::run(int start_pc)
{
    uint32 reg[2] = { 0, 0 };
    const size_t end = m_code.size();
    uint32 pc = start_pc;
    uint32 sp = 65535;
    uint32 call_sp = 0;

    for (;;) {
        uint32 x = m_code[pc];
        uint32 code, r, imm;
        ::Type t;
        decode(code, t.id, imm, x);
        switch (code) {
        case ADD: {
            switch (t.id) {
            case ::Type::INT: {
                int a = m_mem[++sp].int_value;
                int b = m_mem[++sp].int_value;
                m_mem[sp--].int_value = a + b;
            } break;
            case ::Type::DOUBLE: {
                double a = m_mem[++sp].float_value;
                double b = m_mem[++sp].float_value;
                m_mem[sp--].float_value = a + b;
            } break;
            case ::Type::STRING: {
                const std::string* a = m_mem[++sp].str_value;
                const std::string* b = m_mem[++sp].str_value;
                m_mem[sp--].str_value = new std::string(*b + *a);
            } break;
            default:
                assert(false);
            }
            } break;

        case SUB: {
            switch (t.id) {
            case ::Type::INT: {
                int a = m_mem[++sp].int_value;
                int b = m_mem[++sp].int_value;
                m_mem[sp--].int_value = b - a;
            } break;
            case ::Type::DOUBLE: {
                double a = m_mem[++sp].float_value;
                double b = m_mem[++sp].float_value;
                m_mem[sp--].float_value = b - a;
            } break;
            default:
                assert(false);
            }
            } break;
        case ::Type::STRING: {
            } break;
        case PUSHI:
            m_mem[sp--].int_value = imm;
            break;
        case PUSH_CONSTANT:
            m_mem[sp--] = m_constants[imm];
            break;
        case PUSHM:
            m_mem[sp--] = m_mem[imm];
            break;
        case POPM:
            m_mem[imm] = m_mem[++sp];
            break;
        case MUL: {
            switch (t.id) {
            case ::Type::INT: {
                int a = m_mem[++sp].int_value;
                int b = m_mem[++sp].int_value;
                m_mem[sp--].int_value = a * b;
                } break;
            case ::Type::DOUBLE: {
                double a = m_mem[++sp].float_value;
                double b = m_mem[++sp].float_value;
                m_mem[sp--].float_value = a * b;
                } break;
            default:
                assert(false);
            }
            } break;
        case AND: {
            switch (t.id) {
            case ::Type::INT: {
                int a = m_mem[++sp].int_value;
                int b = m_mem[++sp].int_value;
                m_mem[sp--].int_value = a && b;
                } break;
            default:
                assert(false);
            }
            } break;
        case DIV: {
            switch (t.id) {
            case ::Type::INT: {
                int a = m_mem[++sp].int_value;
                int b = m_mem[++sp].int_value;
                m_mem[sp--].int_value = b / a;
                } break;
            case ::Type::DOUBLE: {
                double a = m_mem[++sp].float_value;
                double b = m_mem[++sp].float_value;
                m_mem[sp--].float_value = b / a;
                } break;
            default:
                assert(false);
            }
            } break;
        case CMP: {
            switch (t.id) {
            case ::Type::INT: {
                int a = m_mem[++sp].int_value;
                int b = m_mem[++sp].int_value;
                m_mem[sp--].int_value = a == b;
                } break;
            case ::Type::DOUBLE: {
                double a = m_mem[++sp].float_value;
                double b = m_mem[++sp].float_value;
                m_mem[sp--].int_value = a == b;
                } break;
            case ::Type::STRING: {
                const std::string* a = m_mem[++sp].str_value;
                const std::string* b = m_mem[++sp].str_value;
                m_mem[sp--].int_value = *a == *b;
                } break;
            default:
                assert(false);
            }
            } break;
        case CLT: {
            switch (t.id) {
            case ::Type::INT: {
                int a = m_mem[++sp].int_value;
                int b = m_mem[++sp].int_value;
                m_mem[sp--].int_value = b < a;
                } break;
            case ::Type::DOUBLE: {
                double a = m_mem[++sp].float_value;
                double b = m_mem[++sp].float_value;
                m_mem[sp--].int_value = b < a;
                } break;
            default:
                assert(false);
            }
            } break;
        case JE: {
            int val =  m_mem[++sp].int_value;
            if (val == 1)
                pc = imm - 1;
            }
            break;
        case JNE: {
            int val =  m_mem[++sp].int_value;
            if (val == 0)
                pc = imm - 1;
            }
            break;
        case JMP:
            pc = imm - 1;
            break;
        case CALL:
            m_callstack[call_sp++] = pc;
            pc = imm - 1;
            break;

        case RET:
            if (call_sp == 0)
                break;
            pc = m_callstack[--call_sp];
            break;
        case ASSERT: {
            int val =  m_mem[++sp].int_value;
            assert(val != 0);
            } break;
        case PRINT: {
            switch (t.id) {
            case ::Type::INT:
                printf("%d", m_mem[++sp].int_value);
                break;
            case ::Type::STRING:
                printf("%s", replace_all(*m_mem[++sp].str_value,"\\n","\n").c_str());
                break;
            case ::Type::DOUBLE:
                printf("%f", m_mem[++sp].float_value);
                break;
            case ::Type::LIST: {
                std::list<IValue>* val =  m_mem[++sp].list_value;
                for(auto it : *val)
                    printf("%d, ", it.int_value);
                printf("");
                } break;
            default:
                assert(false);
            }
            } break;
        case MAKE_ITER:
            m_mem[sp].list_iter = new std::list<IValue>::iterator();
            *m_mem[sp].list_iter = m_mem[sp + 1].list_value->begin();
            sp--;
            break;
        case LOOP_ITER: {
            std::list<IValue>::iterator *it = m_mem[sp + 1].list_iter;
            std::list<IValue> *list = m_mem[sp + 2].list_value;
            (*it)++;
            if (*it != list->end()) {
                pc = imm - 1;
            }
            } break;
        case ITER_VALUE: {
            std::list<IValue>::iterator *it = m_mem[sp + 1].list_iter;
            m_mem[sp--] = (**it);
            } break;
        case MAKE_LIST: {
            std::list<IValue> *lst = new std::list<IValue>;
            for(int i=0; i < imm; i++) {
                IValue& value = m_mem[++sp];
                lst->push_front(value);
            }
            m_mem[sp--].list_value = lst;
            } break;
        default:
            assert(0);
            break;
        }
        if (debug) {
            printf("---\n");
            printf("m_mem[] = ");
            for(int i=0;i<8;i++)
                printf("%04x ", m_mem[i].int_value);
            printf("\nsp %d : [ ", sp);
            for(int i=sp+1;i<65536;i++)
                printf("%x ", m_mem[i].int_value);

            printf("]\npc= %d: ", pc);
            print_code(code, imm);
            printf("\n");
        }
        pc++;
        if (pc >= end) break;
    } // for (;;)
}

void ToyVm::dump()
{
    const size_t end = m_code.size();
    for (int i=0;i<end;i++) {
        uint32 x = m_code[i];
        ::Type t;
        uint32 code, imm;
        decode(code, t.id, imm, x);
        printf("%02d: ", i);
        print_code(code, imm);
        printf("\n");
    }
}

void ToyVm::print_code(int code, int imm)
{
    switch (code) {
    case PUSHI:
        printf("push %d", imm);
        break;
    case PUSH_CONSTANT:
        printf("push-constant %d", imm);
        break;
    case PUSHM:
        printf("push [%d]", imm);
        break;
    case POPM:
        printf("pop [%d]", imm);
        break;
    case CALL:
        printf("call %d", imm);
        break;
    case RET:
        printf("ret");
        break;
    case ADD:
        printf("add");
        break;
    case SUB:
        printf("add");
        break;
    case MUL:
        printf("mul");
        break;
    case DIV:
        printf("div");
        break;
    case AND:
        printf("and");
        break;
    case CMP:
        printf("cmp");
        break;
    case CLT:
        printf("clt");
        break;
    case ASSERT:
        printf("assert");
        break;
    case PRINT:
        printf("print");
        break;
    case JE:
        printf("je %d", imm);
        break;
    case JNE:
        printf("jne %d", imm);
        break;
    case JMP:
        printf("jmp %d", imm);
        break;
    case MAKE_LIST:
        printf("make-list %d", imm);
        break;
    case MAKE_ITER:
        printf("make-iter");
        break;
    case LOOP_ITER:
        printf("loop-iter: %d", imm);
        break;
    case ITER_VALUE:
        printf("push *iter");
        break;
    default:
        printf("Unknown code %d\n", code);
        assert(0);
        break;
    }
}

void ToyVm::recompile()
{
    using namespace Xbyak;
    /*
            esi : A
            edi : B
            ebx : m_mem
            for speed up
            m_mem[0] : eax
            m_mem[1] : ecx
            m_mem[2] : edx
        */
    push(rbp);
    push(rbx);
    push(r12);
    push(r13);
    push(r14);
    push(r15);
    mov(rbp, rsp);

    const Reg32e reg[7] = { esi, edi, r8d, r9d, r10d, r12d, r13d };
    const Reg32e mem(r15);
    const Reg32e &vsp = r14;
    int sp = 0;
    mov(mem, (size_t)m_mem);
    mov(vsp, (size_t)65535);
    const size_t end = m_code.size();
    uint32 pc = 0;
    uint32 labelNum = 0;
    for (;;) {
        uint32 x = m_code[pc];
        ::Type t;
        uint32 code, imm;
        decode(code, t.id, imm, x);
        L(Label::toStr(labelNum++).c_str());
        switch (code) {
        case PUSHI:
            if (sp < 7) {
                mov(reg[sp++], imm);
            } else {
                mov(dword[mem + vsp * 4], imm);
                dec(vsp);
            }
            break;
        case PUSHM:
            if (sp < 7) {
                mov(reg[sp++], dword[mem + imm * 4]);
            } else {
                mov(eax, dword[mem + imm * 4]);
                mov(dword[mem + vsp * 4], eax);
                dec(vsp);
            }
            break;
        case ADD:
            if (sp < 7) {
                const Reg32e& a=reg[--sp];
                const Reg32e& b=reg[sp - 1];
                add(b, a);
            } else {
                mov(eax, dword[mem + vsp * 4 + 4]);
                mov(ebx, dword[mem + vsp * 4 + 8]);
                add(eax, ebx);
                mov(dword[mem + vsp * 4 + 8], eax);
                inc(vsp);
            }
            break;
        case MUL:
            if (sp < 7) {
                const Reg32e& a=reg[--sp];
                const Reg32e& b=reg[sp - 1];
                imul(b, a);
            } else {
                mov(eax, dword[mem + vsp * 4 + 4]);
                mov(ebx, dword[mem + vsp * 4 + 8]);
                imul(eax, ebx);
                mov(dword[mem + vsp * 4 + 8], eax);
                inc(vsp);
            }
            break;
        case POPM:
            if (sp < 7) {
                mov(dword[mem + imm * 4], reg[--sp]);
            } else {
                inc(vsp);
                mov(eax, dword[mem + vsp * 4]);
                mov(dword[mem + imm * 4], eax);
            }
            break;
        case RET:
            break;
        default:
            assert(0);
            break;
        }
        pc++;
        if (pc >= end) break;
    } // for (;;)


    L("exit");
    mov(rax, (size_t)&m_sp);
    mov(dword[rax], r14d);
    mov(rsp, rbp);
    pop(r15);
    pop(r14);
    pop(r13);
    pop(r12);
    pop(rbx);
    pop(rbp);
    ret();
}



/*
    the code generated by Xbyak
   push        ebx
   push        esi
   push        edi
   xor         eax,eax
   xor         ecx,ecx
   xor         edx,edx
   xor         esi,esi
   xor         edi,edi
   mov         ebx,0EFF58h
   mov         esi,1
   mov         eax,esi
   mov         edi,2710h
   mov         edx,edi
.lp:
   mov         ecx,esi
   add         esi,eax
   mov         edi,ecx
   mov         eax,edi
   mov         edi,edx
   sub         edi,1
   mov         edx,edi
   test        edi,edi
   jne         .lp
   push        eax
   push        edx
   push        ecx
   push        esi
   push        esi
   push        41h
   push        42C434h
   call        printf (409342h)
   add         esp,10h
   pop         ecx
   pop         edx
   pop         eax
   pop         edi
   pop         esi
   pop         ebx
   ret
*/
