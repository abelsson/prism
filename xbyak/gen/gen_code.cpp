#define XBYAK_DONT_READ_LIST
#include <stdio.h>
#include <string.h>
#include "xbyak.h"
#define NUM_OF_ARRAY(x) (sizeof(x) / sizeof(x[0]))

using namespace Xbyak;
#ifdef _MSC_VER
	#pragma warning(disable : 4996) // scanf
	#define snprintf _snprintf_s
#endif

enum {
	PP_66 = 1 << 0,
	PP_F3 = 1 << 1,
	PP_F2 = 1 << 2,
	MM_0F = 1 << 5,
	MM_0F38 = 1 << 6,
	MM_0F3A = 1 << 7
};

const int NONE = 256; // same as Xbyak::CodeGenerator::NONE

std::string type2String(int type)
{
	std::string str;
	if (type & MM_0F) {
		str = "MM_0F";
	} else if (type & MM_0F38) {
		str = "MM_0F38";
	} else if (type & MM_0F3A) {
		str = "MM_0F3A";
	}
	if (type & PP_66) {
		str += " | PP_66";
	} else if (type & PP_F3) {
		str += " | PP_F3";
	} else if (type & PP_F2) {
		str += " | PP_F2";
	}
	return str;
}

void put()
{
	const int NO = CodeGenerator::NONE;
	{
		char buf[16];
		unsigned int v = VERSION;
		if (v & 0xF) {
			snprintf(buf, sizeof(buf), "%d.%02X%x", v >> 12, (v >> 4) & 0xFF, v & 0xF);
		} else {
			snprintf(buf, sizeof(buf), "%d.%02X", v >> 12, (v >> 4) & 0xFF);
		}
		printf("const char *getVersionString() const { return \"%s\"; }\n", buf);
	}
	const int B = 1 << 0;
	const int W = 1 << 1;
	const int D = 1 << 2;
	const int Q = 1 << 3;
	{
		const struct Tbl {
			uint8 code;
			const char *name;
		} tbl[] = {
			// MMX
			{ B01101011, "packssdw" },
			{ B01100011, "packsswb" },
			{ B01100111, "packuswb" },

			{ B11011011, "pand" },
			{ B11011111, "pandn" },

			{ B11110101, "pmaddwd" },
			{ B11100100, "pmulhuw" },
			{ B11100101, "pmulhw" },
			{ B11010101, "pmullw" },

			{ B11101011, "por" },

			{ B01101000, "punpckhbw" },
			{ B01101001, "punpckhwd" },
			{ B01101010, "punpckhdq" },

			{ B01100000, "punpcklbw" },
			{ B01100001, "punpcklwd" },
			{ B01100010, "punpckldq" },

			{ B11101111, "pxor" },

			// MMX2
			{ B11100000, "pavgb" },
			{ B11100011, "pavgw" },
			{ B11101110, "pmaxsw" },
			{ B11011110, "pmaxub" },
			{ B11101010, "pminsw" },
			{ B11011010, "pminub" },
			{ B11110110, "psadbw" },
			//
			{ B11010100, "paddq" },
			{ B11110100, "pmuludq" },
			{ B11111011, "psubq" },
		};
		for (int i = 0; i < NUM_OF_ARRAY(tbl); i++) {
			const Tbl *p = &tbl[i];
			printf("void %s(const Mmx& mmx, const Operand& op) { opMMX(mmx, op, 0x%02X); }\n"
				, p->name, p->code);
		}
	}

	{
		const struct Tbl {
			uint8 code;
			int mode;
			const char *name;
		} tbl[] = {
			{ B11111100, B|W|D, "padd" },
			{ B11101100, B|W  , "padds" },
			{ B11011100, B|W  , "paddus" },
			{ B01110100, B|W|D, "pcmpeq" },
			{ B01100100, B|W|D, "pcmpgt" },
			{ B11110000, W|D|Q, "psll" },
			{ B11100000, W|D  , "psra" },
			{ B11010000, W|D|Q, "psrl" },
			{ B11111000, B|W|D, "psub" },
			{ B11101000, B|W  , "psubs" },
			{ B11011000, B|W  , "psubus" },
		};
		for (int i = 0; i < NUM_OF_ARRAY(tbl); i++) {
			const Tbl *p = &tbl[i];
			static const char modTbl[][4] = {
				"b", "w", "d", "q"
			};
			for (int j = 0; j < 4; j++) {
				// B(0), W(1), D(2), Q(3)
				if (!(p->mode & (1 << j))) continue;
				printf("void %s%s(const Mmx& mmx, const Operand& op) { opMMX(mmx, op, 0x%02X); }\n"
					, p->name, modTbl[j]
					, p->code | j
				);
			}
		}
	}

	{
		const struct Tbl {
			uint8 code;
			int ext;
			int mode;
			const char *name;
		} tbl[] = {
			{ B01110000, 6, W|D|Q, "psll" },
			{ B01110000, 4, W|D  , "psra" },
			{ B01110000, 2, W|D|Q, "psrl" },
		};
		for (int i = 0; i < NUM_OF_ARRAY(tbl); i++) {
			const Tbl *p = &tbl[i];
			static const char modTbl[][4] = {
				"b", "w", "d", "q"
			};
			for (int j = 0; j < 4; j++) {
				// B(0), W(1), D(2), Q(3)
				if (!(p->mode & (1 << j))) continue;
				printf("void %s%s(const Mmx& mmx, int imm8) { opMMX_IMM(mmx, imm8, 0x%02X, %d); }\n"
					, p->name, modTbl[j]
					, p->code | j
					, p->ext
				);
			}
		}
		printf("void pslldq(const Xmm& xmm, int imm8) { opMMX_IMM(xmm, imm8, 0x%02X, %d); }\n", B01110011, 7);
		printf("void psrldq(const Xmm& xmm, int imm8) { opMMX_IMM(xmm, imm8, 0x%02X, %d); }\n", B01110011, 3);
	}

	{
		const struct Tbl {
			uint8 code;
			uint8 pref;
			const char *name;
		} tbl[] = {
			{ B01110000, 0, "pshufw" },
			{ B01110000, 0xF2, "pshuflw" },
			{ B01110000, 0xF3, "pshufhw" },
			{ B01110000, 0x66, "pshufd" },
		};
		for (int i = 0; i < NUM_OF_ARRAY(tbl); i++) {
			const Tbl *p = &tbl[i];
			printf("void %s(const Mmx& mmx, const Operand& op, uint8 imm8) { opMMX(mmx, op, 0x%02X, 0x%02X, imm8); }\n" , p->name, p->code, p->pref);
		}
	}
	{
		const struct MmxTbl6 {
			uint8 code; // for (reg, reg/[mem])
			uint8 code2; // for ([mem], reg)
			int pref;
			const char *name;
		} mmxTbl6[] = {
			{ B01101111, B01111111, 0x66, "movdqa" },
			{ B01101111, B01111111, 0xF3, "movdqu" },
			// SSE2
			{ B00101000, B00101001, NO, "movaps" },
			{ B00010000, B00010001, 0xF3, "movss" },
			{ B00010000, B00010001, NO, "movups" },
			{ B00101000, B00101001, 0x66, "movapd" },
			{ B00010000, B00010001, 0xF2, "movsd" },
			{ B00010000, B00010001, 0x66, "movupd" },
		};
		for (int i = 0; i < NUM_OF_ARRAY(mmxTbl6); i++) {
			const MmxTbl6 *p = &mmxTbl6[i];
			printf("void %s(const Xmm& xmm, const Operand& op) { opMMX(xmm, op, 0x%02X, 0x%02X); }\n" , p->name, p->code, p->pref);
			printf("void %s(const Address& addr, const Xmm& xmm) { ", p->name);
			if (p->pref != NO) printf("db(0x%02X); ", p->pref);
			printf("opModM(addr, xmm, 0x0F, 0x%02X); }\n" , p->code2);
		}
	}
	{
		enum {
			PS = 1 << 0,
			SS = 1 << 1,
			PD = 1 << 2,
			SD = 1 << 3
		};
		const struct {
			int code;
			const char *name;
		} sufTbl[] = {
			{ NO, "ps" },
			{ 0xF3, "ss" },
			{ 0x66, "pd" },
			{ 0xF2, "sd" },
		};
		const struct Tbl {
			uint8 code;
			int mode;
			const char *name;
			bool hasImm;
		} tbl[] = {
			{ B01011000, PS|SS|PD|SD, "add" },
			{ B01010101, PS|PD      , "andn" },
			{ B01010100, PS|PD      , "and" },
			{ B11000010, PS|SS|PD|SD, "cmp", true },
			{ B01011110, PS|SS|PD|SD, "div" },
			{ B01011111, PS|SS|PD|SD, "max" },
			{ B01011101, PS|SS|PD|SD, "min" },
			{ B01011001, PS|SS|PD|SD, "mul" },
			{ B01010110, PS|PD      , "or" },
			{ B01010011, PS|SS      , "rcp" },
			{ B01010010, PS|SS      , "rsqrt" },
			{ B11000110, PS|PD      , "shuf", true },
			{ B01010001, PS|SS|PD|SD, "sqrt" },
			{ B01011100, PS|SS|PD|SD, "sub" },
			{ B00010101, PS|PD      , "unpckh" },
			{ B00010100, PS|PD      , "unpckl" },
			{ B01010111, PS|PD      , "xor" },
			//
		};
		for (int i = 0; i < NUM_OF_ARRAY(tbl); i++) {
			const Tbl *p = &tbl[i];
			for (int j = 0; j < NUM_OF_ARRAY(sufTbl); j++) {
				if (!(p->mode & (1 << j))) continue;
				if (p->hasImm) {
					// don't change uint8 to int because NO is not in byte
					printf("void %s%s(const Xmm& xmm, const Operand& op, uint8 imm8) { opGen(xmm, op, 0x%2X, 0x%02X, isXMM_XMMorMEM, imm8); }\n", p->name, sufTbl[j].name, p->code, sufTbl[j].code);
				} else {
					printf("void %s%s(const Xmm& xmm, const Operand& op) { opGen(xmm, op, 0x%2X, 0x%02X, isXMM_XMMorMEM); }\n", p->name, sufTbl[j].name, p->code, sufTbl[j].code);
				}
			}
		}
	}
	{
		// (XMM, XMM)
		const struct Tbl {
			uint8 code;
			uint8 pref;
			const char *name;
		} tbl[] = {
			{ B11110111, 0x66, "maskmovdqu" },
			{ B00010010, 0   , "movhlps" },
			{ B00010110, 0   , "movlhps" },
		};
		for (int i = 0; i < NUM_OF_ARRAY(tbl); i++) {
			const Tbl *p = &tbl[i];
			printf("void %s(const Xmm& reg1, const Xmm& reg2) { ", p->name);
			if (p->pref) printf("db(0x%02X); ", p->pref);
			printf(" opModR(reg1, reg2, 0x0F, 0x%02X); }\n", p->code);
		}
	}
	{
		// (XMM, XMM|MEM)
		const struct Tbl {
			uint8 code;
			int pref;
			const char *name;
		} tbl[] = {
			{ B01101101, 0x66, "punpckhqdq" },
			{ B01101100, 0x66, "punpcklqdq" },

			{ B00101111, NO  , "comiss" },
			{ B00101110, NO  , "ucomiss" },
			{ B00101111, 0x66, "comisd" },
			{ B00101110, 0x66, "ucomisd" },

			{ B01011010, 0x66, "cvtpd2ps" },
			{ B01011010, NO  , "cvtps2pd" },
			{ B01011010, 0xF2, "cvtsd2ss" },
			{ B01011010, 0xF3, "cvtss2sd" },
			{ B11100110, 0xF2, "cvtpd2dq" },
			{ B11100110, 0x66, "cvttpd2dq" },
			{ B11100110, 0xF3, "cvtdq2pd" },
			{ B01011011, 0x66, "cvtps2dq" },
			{ B01011011, 0xF3, "cvttps2dq" },
			{ B01011011, NO  , "cvtdq2ps" },

			// SSE3
			{ B11010000, 0x66, "addsubpd" },
			{ B11010000, 0xF2, "addsubps" },
			{ B01111100, 0x66, "haddpd" },
			{ B01111100, 0xF2, "haddps" },
			{ B01111101, 0x66, "hsubpd" },
			{ B01111101, 0xF2, "hsubps" },

			{ B00010010, 0xF2, "movddup" },
			{ B00010110, 0xF3, "movshdup" },
			{ B00010010, 0xF3, "movsldup" },
		};
		for (int i = 0; i < NUM_OF_ARRAY(tbl); i++) {
			const Tbl *p = &tbl[i];
			printf("void %s(const Xmm& xmm, const Operand& op) { opGen(xmm, op, 0x%02X, 0x%02X, isXMM_XMMorMEM); }\n", p->name, p->code, p->pref);
		}
	}

	{
		// special type
		const struct Tbl {
			uint8 code;
			int pref;
			const char *name;
			const char *cond;
		} tbl[] = {
			{ B00101010, NO  , "cvtpi2ps",  "isXMM_MMXorMEM" },
			{ B00101101, NO  , "cvtps2pi",  "isMMX_XMMorMEM" },
			{ B00101010, 0xF3, "cvtsi2ss",  "isXMM_REG32orMEM" },
			{ B00101101, 0xF3, "cvtss2si",  "isREG32_XMMorMEM" },
			{ B00101100, NO  , "cvttps2pi", "isMMX_XMMorMEM" },
			{ B00101100, 0xF3, "cvttss2si", "isREG32_XMMorMEM" },
			{ B00101010, 0x66, "cvtpi2pd",  "isXMM_MMXorMEM" },
			{ B00101101, 0x66, "cvtpd2pi",  "isMMX_XMMorMEM" },
			{ B00101010, 0xF2, "cvtsi2sd",  "isXMM_REG32orMEM" },
			{ B00101101, 0xF2, "cvtsd2si",  "isREG32_XMMorMEM" },
			{ B00101100, 0x66, "cvttpd2pi", "isMMX_XMMorMEM" },
			{ B00101100, 0xF2, "cvttsd2si", "isREG32_XMMorMEM" },
		};
		for (int i = 0; i < NUM_OF_ARRAY(tbl); i++) {
			const Tbl *p = &tbl[i];
			printf("void %s(const Operand& reg, const Operand& op) { opGen(reg, op, 0x%02X, 0x%02X, %s); }\n", p->name, p->code, p->pref, p->cond);
		}
	}
	{
		// prefetch
		const struct Tbl {
			int ext;
			const char *name;
		} tbl[] = {
			{ 1, "t0" },
			{ 2, "t1" },
			{ 3, "t2" },
			{ 0, "nta" },
		};
		for (int i = 0; i < NUM_OF_ARRAY(tbl); i++) {
			const Tbl *p = &tbl[i];
			printf("void prefetch%s(const Address& addr) { opModM(addr, Reg32(%d), 0x0F, B00011000); }\n", p->name, p->ext);
		}
	}
	{
		const struct Tbl {
			uint8 code;
			int pref;
			const char *name;
		} tbl[] = {
			{ B00010110, NO,   "movhps" },
			{ B00010010, NO,   "movlps" },
			{ B00010110, 0x66, "movhpd" },
			{ B00010010, 0x66, "movlpd" },
		};
		for (int i = 0; i < NUM_OF_ARRAY(tbl); i++) {
			const Tbl *p = &tbl[i];
			printf("void %s(const Operand& op1, const Operand& op2) { opMovXMM(op1, op2, 0x%02X, 0x%02X); }\n", p->name, p->code, p->pref);
		}
	}
	{
		// cmov
		const struct Tbl {
			uint8 ext;
			const char *name;
		} tbl[] = {
			{ 0, "o" },
			{ 1, "no" },
			{ 2, "b" },
			{ 2, "c" },
			{ 2, "nae" },
			{ 3, "nb" },
			{ 3, "ae" },
			{ 3, "nc" },
			{ 4, "e" },
			{ 4, "z" },
			{ 5, "ne" },
			{ 5, "nz" },
			{ 6, "be" },
			{ 6, "na" },
			{ 7, "nbe" },
			{ 7, "a" },
			{ 8, "s" },
			{ 9, "ns" },
			{ 10, "p" },
			{ 10, "pe" },
			{ 11, "np" },
			{ 11, "po" },
			{ 12, "l" },
			{ 12, "nge" },
			{ 13, "nl" },
			{ 13, "ge" },
			{ 14, "le" },
			{ 14, "ng" },
			{ 15, "nle" },
			{ 15, "g" },
		};
		for (int i = 0; i < NUM_OF_ARRAY(tbl); i++) {
			const Tbl *p = &tbl[i];
			printf("void cmov%s(const Reg32e& reg, const Operand& op) { opModRM(reg, op, op.isREG(i32e), op.isMEM(), 0x0F, B01000000 | %d); }\n", p->name, p->ext);
			printf("void j%s(const char *label, LabelType type = T_AUTO) { opJmp(label, type, 0x%02X, 0x%02X, 0x%02X); }\n", p->name, p->ext | B01110000, p->ext | B10000000, 0x0F);
			printf("void set%s(const Operand& op) { opR_ModM(op, 8, 0, 0x0F, B10010000 | %d); }\n", p->name, p->ext);
		}
	}
	////////////////////////////////////////////////////////////////
	{
		const struct Tbl {
			const char *name;
			uint8 code1;
			uint8 code2;
			uint8 code3;
		} tbl[] = {
			// only 64-bit mode(from)
			{ "cdqe", B01001000, B10011000 },
			{ "@@@" }, /// here
			// only 32-bit mode(from)
			{ "aaa", B00110111 },
			{ "aad", B11010101, B00001010 },
			{ "aam", B11010100, B00001010 },
			{ "aas", B00111111 },
			{ "daa", B00100111 },
			{ "das", B00101111 },
			{ "popad", B01100001 },
			{ "popfd", B10011101 },
			{ "pusha", B01100000 },
			{ "pushad", B01100000 },
			{ "pushfd", B10011100 },
			{ "popa", B01100001 },
			{ "@@@" }, /// here

			{ "cbw", 0x66, B10011000 },
			{ "cdq", B10011001 },
			{ "clc", B11111000 },
			{ "cld", B11111100 },
			{ "cli", B11111010 },
			{ "cmc", B11110101 },

			{ "cpuid", 0x0F, B10100010 },
			{ "cwd", 0x66, B10011001 },
			{ "cwde", B10011000 },

			{ "lahf", B10011111 },
			{ "lock", B11110000 },
			{ "nop", B10010000 },

			{ "sahf", B10011110 },
			{ "stc", B11111001 },
			{ "std", B11111101 },
			{ "sti", B11111011 },

			{ "emms", 0x0F, B01110111 },
			{ "pause", 0xF3, B10010000 },
			{ "sfence", 0x0F, B10101110, B11111000 },
			{ "lfence", 0x0F, B10101110, B11101000 },
			{ "mfence", 0x0F, B10101110, B11110000 },
			{ "monitor", 0x0F, B00000001, B11001000 },
			{ "mwait", 0x0F, B00000001, B11001001 },

			{ "rdmsr", 0x0F, B00110010 },
			{ "rdpmc", 0x0F, B00110011 },
			{ "rdtsc", 0x0F, B00110001 },
			{ "rdtscp", 0x0F, 0x01, 0xF9 },
			{ "wait", 0x9B },
			{ "fwait", 0x9B },
			{ "wbinvd", 0x0F, B00001001 },
			{ "wrmsr", 0x0F, B00110000 },
			{ "xlatb", 0xD7 },

			{ "popf", B10011101 },
			{ "pushf", B10011100 },

			{ "vzeroall", 0xC5, 0xFC, 0x77 },
			{ "vzeroupper", 0xC5, 0xF8, 0x77 },
			{ "xgetbv", 0x0F, 0x01, 0xD0 },

			// FPU
			{ "f2xm1", 0xD9, 0xF0 },
			{ "fabs", 0xD9, 0xE1 },
			{ "faddp", 0xDE, 0xC1 },
			{ "fchs", 0xD9, 0xE0 },

			{ "fcom", 0xD8, 0xD1 },
			{ "fcomp", 0xD8, 0xD9 },
			{ "fcompp", 0xDE, 0xD9 },
			{ "fcos", 0xD9, 0xFF },
			{ "fdecstp", 0xD9, 0xF6 },
			{ "fdivp", 0xDE, 0xF9 },
			{ "fdivrp", 0xDE, 0xF1 },
			{ "fincstp", 0xD9, 0xF7 },
			{ "finit", 0x9B, 0xDB, 0xE3 },
			{ "fninit", 0xDB, 0xE3 },
			{ "fld1", 0xD9, 0xE8 },
			{ "fldl2t", 0xD9, 0xE9 },
			{ "fldl2e", 0xD9, 0xEA },
			{ "fldpi", 0xD9, 0xEB },
			{ "fldlg2", 0xD9, 0xEC },
			{ "fldln2", 0xD9, 0xED },
			{ "fldz", 0xD9, 0xEE },
			{ "fmulp", 0xDE, 0xC9 },
			{ "fnop", 0xD9, 0xD0 },
			{ "fpatan", 0xD9, 0xF3 },
			{ "fprem", 0xD9, 0xF8 },
			{ "fprem1", 0xD9, 0xF5 },
			{ "fptan", 0xD9, 0xF2 },
			{ "frndint", 0xD9, 0xFC },
			{ "fscale", 0xD9, 0xFD },
			{ "fsin", 0xD9, 0xFE },
			{ "fsincos", 0xD9, 0xFB },
			{ "fsqrt", 0xD9, 0xFA },
			{ "fsubp", 0xDE, 0xE9 },
			{ "fsubrp", 0xDE, 0xE1 },
			{ "ftst", 0xD9, 0xE4 },
			{ "fucom", 0xDD, 0xE1 },
			{ "fucomp", 0xDD, 0xE9 },
			{ "fucompp", 0xDA, 0xE9 },
			{ "fxam", 0xD9, 0xE5 },
			{ "fxch", 0xD9, 0xC9 },
			{ "fxtract", 0xD9, 0xF4 },
			{ "fyl2x", 0xD9, 0xF1 },
			{ "fyl2xp1", 0xD9, 0xF9 },
		};
		printf("#ifdef XBYAK64\n");
		bool inOnly64Bit = true;
		for (int i = 0; i < NUM_OF_ARRAY(tbl); i++) {
			const Tbl *p = &tbl[i];
			if (strcmp(p->name, "@@@") == 0) {
				if (inOnly64Bit) {
					printf("#else\n");
					inOnly64Bit = false;
				} else {
					printf("#endif\n");
				}
				continue;
			}
			printf("void %s() { db(0x%02X); ", p->name, p->code1);
			if (p->code2) printf("db(0x%02X); ", p->code2);
			if (p->code3) printf("db(0x%02X); ", p->code3);
			printf("}\n");
		}
	}
	{
		const struct Tbl {
			uint8 code; // (reg, reg)
			uint8 ext; // (reg, imm)
			const char *name;
		} tbl[] = {
			{ B00010000, 2, "adc" },
			{ B00000000, 0, "add" },
			{ B00100000, 4, "and" },
			{ B00111000, 7, "cmp" },
			{ B00001000, 1, "or" },
			{ B00011000, 3, "sbb" },
			{ B00101000, 5, "sub" },
			{ B00110000, 6, "xor" },
		};
		for (int i = 0; i < NUM_OF_ARRAY(tbl); i++) {
			const Tbl *p = &tbl[i];
			const std::string name = p->name;
			bool isOpName = name == "and" || name == "or" || name == "xor";
			if (isOpName) {
				printf("#ifdef XBYAK_NO_OP_NAMES\n");
				printf("void %s_(const Operand& op1, const Operand& op2) { opRM_RM(op1, op2, 0x%02X); }\n", p->name, p->code);
				printf("void %s_(const Operand& op, uint32 imm) { opRM_I(op, imm, 0x%02X, %d); }\n", p->name, p->code, p->ext);
				printf("#else\n");
			}
			printf("void %s(const Operand& op1, const Operand& op2) { opRM_RM(op1, op2, 0x%02X); }\n", p->name, p->code);
			printf("void %s(const Operand& op, uint32 imm) { opRM_I(op, imm, 0x%02X, %d); }\n", p->name, p->code, p->ext);
			if (isOpName) {
				printf("#endif\n");
			}
		}
	}

	{
		const struct Tbl {
			uint8 code;
			uint8 ext;
			const char *name;
		} tbl[] = {
			{ B01001000, 1, "dec" },
			{ B01000000, 0, "inc" },
		};
		for (int i = 0; i < NUM_OF_ARRAY(tbl); i++) {
			const Tbl *p = &tbl[i];
			printf("void %s(const Operand& op) { opIncDec(op, 0x%02X, %d); }\n", p->name, p->code, p->ext);
		}
	}
	{
		const struct Tbl {
			uint8 code;
			uint8 ext;
			const char *name;
		} tbl[] = {
			{ B11110110, 6, "div" },
			{ B11110110, 7, "idiv" },
			{ B11110110, 5, "imul" },
			{ B11110110, 4, "mul" },
			{ B11110110, 3, "neg" },
			{ B11110110, 2, "not" },
		};
		for (int i = 0; i < NUM_OF_ARRAY(tbl); i++) {
			const Tbl *p = &tbl[i];
			const std::string name = p->name;
			bool isOpName = name == "not";
			if (isOpName) {
				printf("#ifdef XBYAK_NO_OP_NAMES\n");
				printf("void %s_(const Operand& op) { opR_ModM(op, 0, %d, 0x%02X); }\n", p->name, p->ext, p->code);
				printf("#else\n");
			}
			printf("void %s(const Operand& op) { opR_ModM(op, 0, %d, 0x%02X); }\n", p->name, p->ext, p->code);
			if (isOpName) {
				printf("#endif\n");
			}
		}
	}
	{
		const struct Tbl {
			const char *name;
			uint8 ext;
		} tbl[] = {
			{ "rcl", 2 },
			{ "rcr", 3 },
			{ "rol", 0 },
			{ "ror", 1 },
			{ "sar", 7 },
			{ "shl", 4 },
			{ "shr", 5 },

			{ "sal", 4 },
		};
		for (int i = 0; i < NUM_OF_ARRAY(tbl); i++) {
			const Tbl *p = &tbl[i];
			printf("void %s(const Operand& op, int imm) { opShift(op, imm, %d); }\n", p->name, p->ext);
			printf("void %s(const Operand& op, const Reg8& cl) { opShift(op, cl, %d); }\n", p->name, p->ext);
		}
	}
	{
		const struct Tbl {
			const char *name;
			uint8 code;
		} tbl[] = {
			{ "shld", B10100100 },
			{ "shrd", B10101100 },
		};
		for (int i = 0; i < NUM_OF_ARRAY(tbl); i++) {
			const Tbl *p = &tbl[i];
			printf("void %s(const Operand& op, const Reg& reg, uint8 imm) { opShxd(op, reg, imm, 0x%02X); }\n", p->name, p->code);
			printf("void %s(const Operand& op, const Reg& reg, const Reg8& cl) { opShxd(op, reg, 0, 0x%02X, &cl); }\n", p->name, p->code);
		}
	}
	{
		const struct Tbl {
			const char *name;
			uint8 code;
		} tbl[] = {
			{ "bsf", B10111100 },
			{ "bsr", B10111101 },
		};
		for (int i = 0; i < NUM_OF_ARRAY(tbl); i++) {
			const Tbl *p = &tbl[i];
			printf("void %s(const Reg&reg, const Operand& op) { opModRM(reg, op, op.isREG(16 | i32e), op.isMEM(), 0x0F, 0x%02X); }\n", p->name, p->code);
		}
	}
	// SSSE3
	{
		const struct Tbl {
			uint8 code;
			const char *name;
		} tbl[] = {
			{ 0x00, "pshufb" },
			{ 0x01, "phaddw" },
			{ 0x02, "phaddd" },
			{ 0x03, "phaddsw" },
			{ 0x04, "pmaddubsw" },
			{ 0x05, "phsubw" },
			{ 0x06, "phsubd" },
			{ 0x07, "phsubsw" },
			{ 0x08, "psignb" },
			{ 0x09, "psignw" },
			{ 0x0a, "psignd" },
			{ 0x0b, "pmulhrsw" },
			{ 0x1c, "pabsb" },
			{ 0x1d, "pabsw" },
			{ 0x1e, "pabsd" },
		};
		for (int i = 0; i < NUM_OF_ARRAY(tbl); i++) {
			const Tbl *p = &tbl[i];
			printf("void %s(const Mmx& mmx, const Operand& op) { opMMX(mmx, op, 0x%02X, 0x66, NONE, 0x38); }\n", p->name, p->code);
		}
		printf("void palignr(const Mmx& mmx, const Operand& op, int imm) { opMMX(mmx, op, 0x0f, 0x66, static_cast<uint8>(imm), 0x3a); }\n");
	}
	// SSE4
	{
		const struct Tbl {
			uint8 code;
			const char *name;
		} tbl[] = {
			// SSE4.1
			{ 0x15, "blendvpd" },
			{ 0x14, "blendvps" },
			{ 0x2B, "packusdw" },
			{ 0x10, "pblendvb" },
			{ 0x29, "pcmpeqq" },
			{ 0x17, "ptest" },
			{ 0x20, "pmovsxbw" },
			{ 0x21, "pmovsxbd" },
			{ 0x22, "pmovsxbq" },
			{ 0x23, "pmovsxwd" },
			{ 0x24, "pmovsxwq" },
			{ 0x25, "pmovsxdq" },
			{ 0x30, "pmovzxbw" },
			{ 0x31, "pmovzxbd" },
			{ 0x32, "pmovzxbq" },
			{ 0x33, "pmovzxwd" },
			{ 0x34, "pmovzxwq" },
			{ 0x35, "pmovzxdq" },
			{ 0x38, "pminsb" },
			{ 0x39, "pminsd" },
			{ 0x3A, "pminuw" },
			{ 0x3B, "pminud" },
			{ 0x3C, "pmaxsb" },
			{ 0x3D, "pmaxsd" },
			{ 0x3E, "pmaxuw" },
			{ 0x3F, "pmaxud" },
			{ 0x28, "pmuldq" },
			{ 0x40, "pmulld" },
			{ 0x41, "phminposuw"},
			// SSE4.2
			{ 0x37, "pcmpgtq" },
			{ 0xde, "aesdec" },
			{ 0xdf, "aesdeclast" },
			{ 0xdc, "aesenc" },
			{ 0xdd, "aesenclast" },
			{ 0xdb, "aesimc" },
		};
		for (int i = 0; i < NUM_OF_ARRAY(tbl); i++) {
			const Tbl *p = &tbl[i];
			printf("void %s(const Xmm& xmm, const Operand& op) { opGen(xmm, op, 0x%02X, 0x66, isXMM_XMMorMEM, NONE, 0x38); }\n", p->name, p->code);
		}
	}
	{
		const struct Tbl {
			uint8 code;
			const char *name;
		} tbl[] = {
			// SSE4.1
			{ 0x0D, "blendpd" },
			{ 0x0C, "blendps" },
			{ 0x41, "dppd" },
			{ 0x40, "dpps" },
			{ 0x42, "mpsadbw" },
			{ 0x0E, "pblendw" },
			{ 0x08, "roundps" },
			{ 0x09, "roundpd" },
			{ 0x0A, "roundss" },
			{ 0x0B, "roundsd" },
			// SSE4.2
			{ 0x60, "pcmpestrm" },
			{ 0x61, "pcmpestri" },
			{ 0x62, "pcmpistrm" },
			{ 0x63, "pcmpistri" },
			{ 0x44, "pclmulqdq" },
			{ 0xdf, "aeskeygenassist" },
		};
		for (int i = 0; i < NUM_OF_ARRAY(tbl); i++) {
			const Tbl *p = &tbl[i];
			printf("void %s(const Xmm& xmm, const Operand& op, int imm) { opGen(xmm, op, 0x%02X, 0x66, isXMM_XMMorMEM, static_cast<uint8>(imm), 0x3A); }\n", p->name, p->code);
		}
	}
	{
		const struct Tbl {
			const char *name;
			uint8 code;
		} tbl[] = {
			{ "pclmullqlqdq", 0 },
			{ "pclmulhqlqdq", 1 },
			{ "pclmullqhdq", 0x10 },
			{ "pclmulhqhdq", 0x11 },
		};
		for (int i = 0; i < NUM_OF_ARRAY(tbl); i++) {
			const Tbl *p = &tbl[i];
			printf("void %s(const Xmm& xmm, const Operand& op) { pclmulqdq(xmm, op, 0x%02X); }\n", p->name, p->code);
		}
	}
	{
		const struct Tbl {
			uint8 code1;
			int code2;
			uint8 ext;
			const char *name;
		} tbl[] = {
			{ 0x0F, B10101110, 2, "ldmxcsr" },
			{ 0x0F, B10101110, 3, "stmxcsr" },
			{ 0x0F, B10101110, 7, "clflush" }, // 0x80 is bug of nasm ?
			{ 0xD9, NONE, 5, "fldcw" },
//			{ 0x9B, 0xD9, 7, "fstcw" }, // not correct order for fstcw [eax] on 64bit OS
		};
		for (int i = 0; i < NUM_OF_ARRAY(tbl); i++) {
			const Tbl *p = &tbl[i];
			printf("void %s(const Address& addr) { opModM(addr, Reg32(%d), 0x%02X, 0x%02X); }\n", p->name, p->ext, p->code1, p->code2);
		}
		printf("void fstcw(const Address& addr) { db(0x9B); opModM(addr, Reg32(7), 0xD9, NONE); }\n");
	}
	{
		const struct Tbl {
			uint8 code;
			const char *name;
		} tbl[] = {
			{ B00101011, "movntpd" },
			{ B11100111, "movntdq" },
		};
		for (int i = 0; i < NUM_OF_ARRAY(tbl); i++) {
			const Tbl *p = &tbl[i];
			// cast xmm register to 16bit register to put 0x66
			printf("void %s(const Address& addr, const Xmm& reg) { opModM(addr, Reg16(reg.getIdx()), 0x0F, 0x%02X); }\n", p->name, p->code);
		}
	}
	{
		const struct Tbl {
			uint8 code;
			const char *name;
		} tbl[] = {
			{ B10111110, "movsx" },
			{ B10110110, "movzx" },
		};
		for (int i = 0; i < NUM_OF_ARRAY(tbl); i++) {
			const Tbl *p = &tbl[i];
			printf("void %s(const Reg& reg, const Operand& op) { opMovxx(reg, op, 0x%02X); }\n", p->name, p->code);
		}
	}
	{
		const struct Tbl {
			uint8 m16;
			uint8 m32;
			uint8 m64;
			uint8 ext;
			const char *name;
			uint8 m64ext;
		} tbl[] = {
			{ 0x00, 0xD8, 0xDC, 0, "fadd" },
			{ 0xDE, 0xDA, 0x00, 0, "fiadd" },
			{ 0x00, 0xD8, 0xDC, 2, "fcom" },
			{ 0x00, 0xD8, 0xDC, 3, "fcomp" },
			{ 0x00, 0xD8, 0xDC, 6, "fdiv" },
			{ 0xDE, 0xDA, 0x00, 6, "fidiv" },
			{ 0x00, 0xD8, 0xDC, 7, "fdivr" },
			{ 0xDE, 0xDA, 0x00, 7, "fidivr" },
			{ 0xDE, 0xDA, 0x00, 2, "ficom" },
			{ 0xDE, 0xDA, 0x00, 3, "ficomp" },
			{ 0xDF, 0xDB, 0xDF, 0, "fild", 5 },
			{ 0xDF, 0xDB, 0x00, 2, "fist" },
			{ 0xDF, 0xDB, 0xDF, 3, "fistp", 7 },
			{ 0xDF, 0xDB, 0xDD, 1, "fisttp" },
			{ 0x00, 0xD9, 0xDD, 0, "fld" },
			{ 0x00, 0xD8, 0xDC, 1, "fmul" },
			{ 0xDE, 0xDA, 0x00, 1, "fimul" },
			{ 0x00, 0xD9, 0xDD, 2, "fst" },
			{ 0x00, 0xD9, 0xDD, 3, "fstp" },
			{ 0x00, 0xD8, 0xDC, 4, "fsub" },
			{ 0xDE, 0xDA, 0x00, 4, "fisub" },
			{ 0x00, 0xD8, 0xDC, 5, "fsubr" },
			{ 0xDE, 0xDA, 0x00, 5, "fisubr" },
		};
		for (int i = 0; i < NUM_OF_ARRAY(tbl); i++) {
			const Tbl *p = &tbl[i];
			printf("void %s(const Address& addr) { opFpuMem(addr, 0x%02X, 0x%02X, 0x%02X, %d, %d); }\n", p->name, p->m16, p->m32, p->m64, p->ext, p->m64ext);
		}
	}
	{
		const struct Tbl {
			uint32 code1;
			uint32 code2;
			const char *name;
		} tbl[] = {
			{ 0xD8C0, 0xDCC0, "fadd" },
			{ 0x0000, 0xDEC0, "faddp" },

			{ 0xDAC0, 0x00C0, "fcmovb" },
			{ 0xDAC8, 0x00C8, "fcmove" },
			{ 0xDAD0, 0x00D0, "fcmovbe" },
			{ 0xDAD8, 0x00D8, "fcmovu" },
			{ 0xDBC0, 0x00C0, "fcmovnb" },
			{ 0xDBC8, 0x00C8, "fcmovne" },
			{ 0xDBD0, 0x00D0, "fcmovnbe" },
			{ 0xDBD8, 0x00D8, "fcmovnu" },

			{ 0xDBF0, 0x00F0, "fcomi" },
			{ 0xDFF0, 0x00F0, "fcomip" },
			{ 0xDBE8, 0x00E8, "fucomi" },
			{ 0xDFE8, 0x00E8, "fucomip" },

			{ 0xD8F0, 0xDCF8, "fdiv" },
			{ 0x0000, 0xDEF8, "fdivp" },
			{ 0xD8F8, 0xDCF0, "fdivr" },
			{ 0x0000, 0xDEF0, "fdivrp" },
			{ 0xD8C8, 0xDCC8, "fmul" },
			{ 0x0000, 0xDEC8, "fmulp" },
			{ 0xD8E0, 0xDCE8, "fsub" },
			{ 0x0000, 0xDEE8, "fsubp" },
			{ 0xD8E8, 0xDCE0, "fsubr" },
			{ 0x0000, 0xDEE0, "fsubrp" },
		};
		for (int i = 0; i < NUM_OF_ARRAY(tbl); i++) {
			const Tbl *p = &tbl[i];
			printf("void %s(const Fpu& reg1, const Fpu& reg2) { opFpuFpu(reg1, reg2, 0x%04X, 0x%04X); }\n", p->name, p->code1, p->code2);
			// omit st0 version(like nasm)
			if (p->code1) {
				printf("void %s(const Fpu& reg1) { opFpuFpu(st0, reg1, 0x%04X, 0x%04X); }\n", p->name, p->code1, p->code2);
			} else {
				printf("void %s(const Fpu& reg1) { opFpuFpu(reg1, st0, 0x%04X, 0x%04X); }\n", p->name, p->code1, p->code2);
			}
		}
	}
	{
		const struct Tbl {
			uint8 code1;
			uint8 code2;
			const char *name;
		} tbl[] = {
			{ 0xD8, 0xD0, "fcom" },
			{ 0xD8, 0xD8, "fcomp" },
			{ 0xDD, 0xC0, "ffree" },
			{ 0xD9, 0xC0, "fld" },
			{ 0xDD, 0xD0, "fst" },
			{ 0xDD, 0xD8, "fstp" },
			{ 0xDD, 0xE0, "fucom" },
			{ 0xDD, 0xE8, "fucomp" },
			{ 0xD9, 0xC8, "fxch" },
		};
		for (int i = 0; i < NUM_OF_ARRAY(tbl); i++) {
			const Tbl *p = &tbl[i];
			printf("void %s(const Fpu& reg) { opFpu(reg, 0x%02X, 0x%02X); }\n", p->name, p->code1, p->code2);
		}
	}
	// AVX
	{ // pd, ps, sd, ss
		const struct Tbl {
			uint8 code;
			const char *name;
			bool only_pd_ps;
		} tbl[] = {
			{ 0x58, "add", false },
			{ 0x5C, "sub", false },
			{ 0x59, "mul", false },
			{ 0x5E, "div", false },
			{ 0x5F, "max", false },
			{ 0x5D, "min", false },
			{ 0x54, "and", true },
			{ 0x55, "andn", true },
			{ 0x56, "or", true },
			{ 0x57, "xor", true },
		};
		for (int i = 0; i < NUM_OF_ARRAY(tbl); i++) {
			const Tbl *p = &tbl[i];
			printf("void v%spd(const Xmm& xmm, const Operand& op1, const Operand& op2 = Operand()) { opAVX_X_X_XM(xmm, op1, op2, MM_0F | PP_66, 0x%02X, true); }\n", p->name, p->code);
			printf("void v%sps(const Xmm& xmm, const Operand& op1, const Operand& op2 = Operand()) { opAVX_X_X_XM(xmm, op1, op2, MM_0F, 0x%02X, true); }\n", p->name, p->code);
			if (p->only_pd_ps) continue;
			printf("void v%ssd(const Xmm& xmm, const Operand& op1, const Operand& op2 = Operand()) { opAVX_X_X_XM(xmm, op1, op2, MM_0F | PP_F2, 0x%02X, false); }\n", p->name, p->code);
			printf("void v%sss(const Xmm& xmm, const Operand& op1, const Operand& op2 = Operand()) { opAVX_X_X_XM(xmm, op1, op2, MM_0F | PP_F3, 0x%02X, false); }\n", p->name, p->code);
		}
	}
	// (x, x, x/m[, imm]) or (y, y, y/m[, imm])
	{
		const struct Tbl {
			uint8 code;
			const char *name;
			int type;
			bool supportYMM;
			int w;
			bool hasIMM;
			bool enableOmit;
		} tbl[] = {
			{ 0x0D, "blendpd", MM_0F3A | PP_66, true, 0, true, true },
			{ 0x0C, "blendps", MM_0F3A | PP_66, true, 0, true, true },
			{ 0x41, "dppd", MM_0F3A | PP_66, false, 0, true, true },
			{ 0x40, "dpps", MM_0F3A | PP_66, true, 0, true, true },
			{ 0x42, "mpsadbw", MM_0F3A | PP_66, false, 0, true, true },
			{ 0x0E, "pblendw", MM_0F3A | PP_66, false, 0, true, true },
			{ 0x0B, "roundsd", MM_0F3A | PP_66, false, 0, true, true },
			{ 0x0A, "roundss", MM_0F3A | PP_66, false, 0, true, true },
			{ 0x44, "pclmulqdq", MM_0F3A | PP_66, false, 0, true, true },
			{ 0x0C, "permilps", MM_0F38 | PP_66, true, 0, false, false },
			{ 0x0D, "permilpd", MM_0F38 | PP_66, true, 0, false, false },
			{ 0xC2, "cmppd", MM_0F | PP_66, true, -1, true, true },
			{ 0xC2, "cmpps", MM_0F, true, -1, true, true },
			{ 0xC2, "cmpsd", MM_0F | PP_F2, false, -1, true, true },
			{ 0xC2, "cmpss", MM_0F | PP_F3, false, -1, true, true },
			{ 0x5A, "cvtsd2ss", MM_0F | PP_F2, false, -1, false, true },
			{ 0x5A, "cvtss2sd", MM_0F | PP_F3, false, -1, false, true },
			{ 0x21, "insertps", MM_0F3A | PP_66, false, 0, true, true },
			{ 0x63, "packsswb", MM_0F | PP_66, false, -1, false, true },
			{ 0x6B, "packssdw", MM_0F | PP_66, false, -1, false, true },
			{ 0x67, "packuswb", MM_0F | PP_66, false, -1, false, true },
			{ 0x2B, "packusdw", MM_0F38 | PP_66, false, -1, false, true },

			{ 0xFC, "paddb", MM_0F | PP_66, false, -1, false, true },
			{ 0xFD, "paddw", MM_0F | PP_66, false, -1, false, true },
			{ 0xFE, "paddd", MM_0F | PP_66, false, -1, false, true },
			{ 0xD4, "paddq", MM_0F | PP_66, false, -1, false, true },

			{ 0xEC, "paddsb", MM_0F | PP_66, false, -1, false, true },
			{ 0xED, "paddsw", MM_0F | PP_66, false, -1, false, true },

			{ 0xDC, "paddusb", MM_0F | PP_66, false, -1, false, true },
			{ 0xDD, "paddusw", MM_0F | PP_66, false, -1, false, true },

			{ 0x0F, "palignr", MM_0F3A | PP_66, false, -1, true, true },

			{ 0xDB, "pand", MM_0F | PP_66, false, -1, false, true },
			{ 0xDF, "pandn", MM_0F | PP_66, false, -1, false, true },

			{ 0xE0, "pavgb", MM_0F | PP_66, false, -1, false, true },
			{ 0xE3, "pavgw", MM_0F | PP_66, false, -1, false, true },

			{ 0x74, "pcmpeqb", MM_0F | PP_66, false, -1, false, true },
			{ 0x75, "pcmpeqw", MM_0F | PP_66, false, -1, false, true },
			{ 0x76, "pcmpeqd", MM_0F | PP_66, false, -1, false, true },
			{ 0x29, "pcmpeqq", MM_0F38 | PP_66, false, -1, false, true },

			{ 0x64, "pcmpgtb", MM_0F | PP_66, false, -1, false, true },
			{ 0x65, "pcmpgtw", MM_0F | PP_66, false, -1, false, true },
			{ 0x66, "pcmpgtd", MM_0F | PP_66, false, -1, false, true },
			{ 0x37, "pcmpgtq", MM_0F38 | PP_66, false, -1, false, true },

			{ 0x01, "phaddw", MM_0F38 | PP_66, false, -1, false, true },
			{ 0x02, "phaddd", MM_0F38 | PP_66, false, -1, false, true },
			{ 0x03, "phaddsw", MM_0F38 | PP_66, false, -1, false, true },

			{ 0x05, "phsubw", MM_0F38 | PP_66, false, -1, false, true },
			{ 0x06, "phsubd", MM_0F38 | PP_66, false, -1, false, true },
			{ 0x07, "phsubsw", MM_0F38 | PP_66, false, -1, false, true },
			{ 0xF5, "pmaddwd", MM_0F | PP_66, false, -1, false, true },
			{ 0x04, "pmaddubsw", MM_0F38 | PP_66, false, -1, false, true },

			{ 0x3C, "pmaxsb", MM_0F38 | PP_66, false, -1, false, true },
			{ 0xEE, "pmaxsw", MM_0F | PP_66, false, -1, false, true },
			{ 0x3D, "pmaxsd", MM_0F38 | PP_66, false, -1, false, true },

			{ 0xDE, "pmaxub", MM_0F | PP_66, false, -1, false, true },
			{ 0x3E, "pmaxuw", MM_0F38 | PP_66, false, -1, false, true },
			{ 0x3F, "pmaxud", MM_0F38 | PP_66, false, -1, false, true },

			{ 0x38, "pminsb", MM_0F38 | PP_66, false, -1, false, true },
			{ 0xEA, "pminsw", MM_0F | PP_66, false, -1, false, true },
			{ 0x39, "pminsd", MM_0F38 | PP_66, false, -1, false, true },

			{ 0xDA, "pminub", MM_0F | PP_66, false, -1, false, true },
			{ 0x3A, "pminuw", MM_0F38 | PP_66, false, -1, false, true },
			{ 0x3B, "pminud", MM_0F38 | PP_66, false, -1, false, true },

			{ 0xE4, "pmulhuw", MM_0F | PP_66, false, -1, false, true },
			{ 0x0B, "pmulhrsw", MM_0F38 | PP_66, false, -1, false, true },
			{ 0xE5, "pmulhw", MM_0F | PP_66, false, -1, false, true },
			{ 0xD5, "pmullw", MM_0F | PP_66, false, -1, false, true },
			{ 0x40, "pmulld", MM_0F38 | PP_66, false, -1, false, true },

			{ 0xF4, "pmuludq", MM_0F | PP_66, false, -1, false, true },
			{ 0x28, "pmuldq", MM_0F38 | PP_66, false, -1, false, true },

			{ 0xEB, "por", MM_0F | PP_66, false, -1, false, true },
			{ 0xF6, "psadbw", MM_0F | PP_66, false, -1, false, true },

			{ 0x00, "pshufb", MM_0F38 | PP_66, false, -1, false, false },

			{ 0x08, "psignb", MM_0F38 | PP_66, false, -1, false, true },
			{ 0x09, "psignw", MM_0F38 | PP_66, false, -1, false, true },
			{ 0x0A, "psignd", MM_0F38 | PP_66, false, -1, false, true },

			{ 0xF1, "psllw", MM_0F | PP_66, false, -1, false, true },
			{ 0xF2, "pslld", MM_0F | PP_66, false, -1, false, true },
			{ 0xF3, "psllq", MM_0F | PP_66, false, -1, false, true },

			{ 0xE1, "psraw", MM_0F | PP_66, false, -1, false, true },
			{ 0xE2, "psrad", MM_0F | PP_66, false, -1, false, true },
			{ 0xD1, "psrlw", MM_0F | PP_66, false, -1, false, true },
			{ 0xD2, "psrld", MM_0F | PP_66, false, -1, false, true },
			{ 0xD3, "psrlq", MM_0F | PP_66, false, -1, false, true },

			{ 0xF8, "psubb", MM_0F | PP_66, false, -1, false, true },
			{ 0xF9, "psubw", MM_0F | PP_66, false, -1, false, true },
			{ 0xFA, "psubd", MM_0F | PP_66, false, -1, false, true },
			{ 0xFB, "psubq", MM_0F | PP_66, false, -1, false, true },

			{ 0xE8, "psubsb", MM_0F | PP_66, false, -1, false, true },
			{ 0xE9, "psubsw", MM_0F | PP_66, false, -1, false, true },

			{ 0xD8, "psubusb", MM_0F | PP_66, false, -1, false, true },
			{ 0xD9, "psubusw", MM_0F | PP_66, false, -1, false, true },

			{ 0x68, "punpckhbw", MM_0F | PP_66, false, -1, false, true },
			{ 0x69, "punpckhwd", MM_0F | PP_66, false, -1, false, true },
			{ 0x6A, "punpckhdq", MM_0F | PP_66, false, -1, false, true },
			{ 0x6D, "punpckhqdq", MM_0F | PP_66, false, -1, false, true },

			{ 0x60, "punpcklbw", MM_0F | PP_66, false, -1, false, true },
			{ 0x61, "punpcklwd", MM_0F | PP_66, false, -1, false, true },
			{ 0x62, "punpckldq", MM_0F | PP_66, false, -1, false, true },
			{ 0x6C, "punpcklqdq", MM_0F | PP_66, false, -1, false, true },

			{ 0xEF, "pxor", MM_0F | PP_66, false, -1, false, true },

			{ 0x53, "rcpss", MM_0F | PP_F3, false, -1, false, true },
			{ 0x52, "rsqrtss", MM_0F | PP_F3, false, -1, false, true },

			{ 0xC6, "shufpd", MM_0F | PP_66, true, -1, true, true },
			{ 0xC6, "shufps", MM_0F, true, -1, true, true },

			{ 0x51, "sqrtsd", MM_0F | PP_F2, false, -1, false, true },
			{ 0x51, "sqrtss", MM_0F | PP_F3, false, -1, false, true },

			{ 0x15, "unpckhpd", MM_0F | PP_66, true, -1, false, true },
			{ 0x15, "unpckhps", MM_0F, true, -1, false, true },

			{ 0x14, "unpcklpd", MM_0F | PP_66, true, -1, false, true },
			{ 0x14, "unpcklps", MM_0F, true, -1, false, true },
		};
		for (size_t i = 0; i < NUM_OF_ARRAY(tbl); i++) {
			const Tbl *p = &tbl[i];
			std::string type = type2String(p->type);
			printf("void v%s(const Xmm& xm1, const Xmm& xm2, const Operand& op%s) { opAVX_X_X_XM(xm1, xm2, op, %s, 0x%02X, %s, %d)%s; }\n"
				, p->name, p->hasIMM ? ", uint8 imm" : "", type.c_str(), p->code, p->supportYMM ? "true" : "false", p->w, p->hasIMM ? "; db(imm)" : "");
			if (!p->enableOmit) continue;
			printf("void v%s(const Xmm& xmm, const Operand& op%s) { opAVX_X_X_XM(xmm, xmm, op, %s, 0x%02X, %s, %d)%s; }\n"
				, p->name, p->hasIMM ? ", uint8 imm" : "", type.c_str(), p->code, p->supportYMM ? "true" : "false", p->w, p->hasIMM ? "; db(imm)" : "");
		}
	}
	// (x, x/m[, imm]) or (y, y/m[, imm])
	{
		const struct Tbl {
			uint8 code;
			const char *name;
			int type;
			bool supportYMM;
			int w;
			bool hasIMM;
		} tbl[] = {
			{ 0xDF, "aeskeygenassist", MM_0F3A | PP_66, false, 0, true },
			{ 0x09, "roundpd", MM_0F3A | PP_66, true, 0, true },
			{ 0x08, "roundps", MM_0F3A | PP_66, true, 0, true },
			{ 0x05, "permilpd", MM_0F3A | PP_66, true, 0, true },
			{ 0x04, "permilps", MM_0F3A | PP_66, true, 0, true },
			{ 0x61, "pcmpestri", MM_0F3A | PP_66, false, 0, true },
			{ 0x60, "pcmpestrm", MM_0F3A | PP_66, false, 0, true },
			{ 0x63, "pcmpistri", MM_0F3A | PP_66, false, 0, true },
			{ 0x62, "pcmpistrm", MM_0F3A | PP_66, false, 0, true },
			{ 0x0E, "testps", MM_0F38 | PP_66, true, 0, false },
			{ 0x0F, "testpd", MM_0F38 | PP_66, true, 0, false },
			{ 0x2F, "comisd", MM_0F | PP_66, false, -1, false },
			{ 0x2F, "comiss", MM_0F, false, -1, false },
			{ 0x5B, "cvtdq2ps", MM_0F, true, -1, false },
			{ 0x5B, "cvtps2dq", MM_0F | PP_66, true, -1, false },
			{ 0x5B, "cvttps2dq", MM_0F | PP_F3, true, -1, false },
			{ 0x28, "movapd", MM_0F | PP_66, true, -1, false },
			{ 0x28, "movaps", MM_0F, true, -1, false },
			{ 0x12, "movddup", MM_0F | PP_F2, true, -1, false },
			{ 0x6F, "movdqa", MM_0F | PP_66, true, -1, false },
			{ 0x6F, "movdqu", MM_0F | PP_F3, true, -1, false },
			{ 0x16, "movshdup", MM_0F | PP_F3, true, -1, false },
			{ 0x12, "movsldup", MM_0F | PP_F3, true, -1, false },
			{ 0x10, "movupd", MM_0F | PP_66, true, -1, false },
			{ 0x10, "movups", MM_0F, true, -1, false },

			{ 0x1C, "pabsb", MM_0F38 | PP_66, false, -1, false },
			{ 0x1D, "pabsw", MM_0F38 | PP_66, false, -1, false },
			{ 0x1E, "pabsd", MM_0F38 | PP_66, false, -1, false },
			{ 0x41, "phminposuw", MM_0F38 | PP_66, false, -1, false },

			{ 0x20, "pmovsxbw", MM_0F38 | PP_66, false, -1, false },
			{ 0x21, "pmovsxbd", MM_0F38 | PP_66, false, -1, false },
			{ 0x22, "pmovsxbq", MM_0F38 | PP_66, false, -1, false },
			{ 0x23, "pmovsxwd", MM_0F38 | PP_66, false, -1, false },
			{ 0x24, "pmovsxwq", MM_0F38 | PP_66, false, -1, false },
			{ 0x25, "pmovsxdq", MM_0F38 | PP_66, false, -1, false },

			{ 0x30, "pmovzxbw", MM_0F38 | PP_66, false, -1, false },
			{ 0x31, "pmovzxbd", MM_0F38 | PP_66, false, -1, false },
			{ 0x32, "pmovzxbq", MM_0F38 | PP_66, false, -1, false },
			{ 0x33, "pmovzxwd", MM_0F38 | PP_66, false, -1, false },
			{ 0x34, "pmovzxwq", MM_0F38 | PP_66, false, -1, false },
			{ 0x35, "pmovzxdq", MM_0F38 | PP_66, false, -1, false },

			{ 0x70, "pshufd", MM_0F | PP_66, false, -1, true },
			{ 0x70, "pshufhw", MM_0F | PP_F3, false, -1, true },
			{ 0x70, "pshuflw", MM_0F | PP_F2, false, -1, true },

			{ 0x17, "ptest", MM_0F38 | PP_66, false, -1, false },
			{ 0x53, "rcpps", MM_0F, true, -1, false },
			{ 0x52, "rsqrtps", MM_0F, true, -1, false },

			{ 0x51, "sqrtpd", MM_0F | PP_66, true, -1, false },
			{ 0x51, "sqrtps", MM_0F, true, -1, false },

			{ 0x2E, "ucomisd", MM_0F | PP_66, false, -1, false },
			{ 0x2E, "ucomiss", MM_0F, false, -1, false },
		};
		for (size_t i = 0; i < NUM_OF_ARRAY(tbl); i++) {
			const Tbl *p = &tbl[i];
			std::string type = type2String(p->type);
			printf("void v%s(const Xmm& xm, const Operand& op%s) { opAVX_X_XM_IMM(xm, op, %s, 0x%02X, %s, %d%s); }\n"
				, p->name, p->hasIMM ? ", uint8 imm" : "", type.c_str(), p->code, p->supportYMM ? "true" : "false", p->w, p->hasIMM ? ", imm" : "");
		}
	}
	// (m, x), (m, y)
	{
		const struct Tbl {
			uint8 code;
			const char *name;
			int type;
			bool supportYMM;
			int w;
		} tbl[] = {
			{ 0x29, "movapd", MM_0F | PP_66, true, -1 },
			{ 0x29, "movaps", MM_0F, true, -1 },
			{ 0x7F, "movdqa", MM_0F | PP_66, true, -1 },
			{ 0x7F, "movdqu", MM_0F | PP_F3, true, -1 },
			{ 0x11, "movupd", MM_0F | PP_66, true, -1 },
			{ 0x11, "movups", MM_0F, true, -1 },
		};
		for (size_t i = 0; i < NUM_OF_ARRAY(tbl); i++) {
			const Tbl *p = &tbl[i];
			std::string type = type2String(p->type);
			printf("void v%s(const Address& addr, const Xmm& xmm) { opAVX_X_XM_IMM(xmm, addr, %s, 0x%02X, %s, %d); }\n"
				, p->name, type.c_str(), p->code, p->supportYMM ? "true" : "false", p->w);
		}
	}
	// (x, x/m), (y, y/m), (x, x, x/m), (y, y, y/m)
	{
		const struct Tbl {
			uint8 code;
			const char *name;
			int type;
			bool supportYMM;
			int w;
		} tbl[] = {
			{ 0xD0, "addsubpd", MM_0F | PP_66, true, -1 },
			{ 0xD0, "addsubps", MM_0F | PP_F2, true, -1 },
			{ 0x7C, "haddpd", MM_0F | PP_66, true, -1 },
			{ 0x7C, "haddps", MM_0F | PP_F2, true, -1 },
			{ 0x7D, "hsubpd", MM_0F | PP_66, true, -1 },
			{ 0x7D, "hsubps", MM_0F | PP_F2, true, -1 },

			{ 0xDC, "aesenc", MM_0F38 | PP_66, false, 0 },
			{ 0xDD, "aesenclast", MM_0F38 | PP_66, false, 0 },
			{ 0xDE, "aesdec", MM_0F38 | PP_66, false, 0 },
			{ 0xDF, "aesdeclast", MM_0F38 | PP_66, false, 0 },
		};
		for (size_t i = 0; i < NUM_OF_ARRAY(tbl); i++) {
			const Tbl *p = &tbl[i];
			std::string type = type2String(p->type);
			printf("void v%s(const Xmm& xmm, const Operand& op1, const Operand& op2 = Operand()) { opAVX_X_X_XM(xmm, op1, op2, %s, 0x%02X, %s, %d); }\n"
				, p->name, type.c_str(), p->code, p->supportYMM ? "true" : "false", p->w);
		}
	}
	// vmaskmov
	{
		const char suf[][8] = { "ps", "pd" };
		for (int i = 0; i < 2; i++) {
			printf("void vmaskmov%s(const Xmm& xm1, const Xmm& xm2, const Address& addr) { opAVX_X_X_XM(xm1, xm2, addr, MM_0F38 | PP_66, 0x%02X, true, 0); }\n", suf[i], 0x2C + i);
			printf("void vmaskmov%s(const Address& addr, const Xmm& xm1, const Xmm& xm2) { opAVX_X_X_XM(xm2, xm1, addr, MM_0F38 | PP_66, 0x%02X, true, 0); }\n", suf[i], 0x2E + i);
		}
	}
	// vcmpeqps
	{
		const char pred[32][16] = {
			"eq", "lt", "le", "unord", "neq", "nlt", "nle", "ord",
			"eq_uq", "nge", "ngt", "false", "neq_oq", "ge", "gt",
			"true", "eq_os", "lt_oq", "le_oq", "unord_s", "neq_us", "nlt_uq", "nle_uq", "ord_s",
			"eq_us", "nge_uq", "ngt_uq", "false_os", "neq_os", "ge_oq", "gt_oq", "true_us"
		};
		const char suf[][4] = { "pd", "ps", "sd", "ss" };
		for (int i = 0; i < 4; i++) {
			const char *s = suf[i];
			for (int j = 0; j < 32; j++) {
				if (j < 8) {
					printf("void cmp%s%s(const Xmm& x, const Operand& op) { cmp%s(x, op, %d); }\n", pred[j], s, s, j);
				}
				printf("void vcmp%s%s(const Xmm& x1, const Xmm& x2, const Operand& op) { vcmp%s(x1, x2, op, %d); }\n", pred[j], s, s, j);
				printf("void vcmp%s%s(const Xmm& x, const Operand& op) { vcmp%s(x, op, %d); }\n", pred[j], s, s, j);
			}
		}
	}
	// vmov(h|l)(pd|ps)
	{
		const struct Tbl {
			bool isH;
			bool isPd;
			uint8 code;
		} tbl[] = {
			{ true, true, 0x16 },
			{ true, false, 0x16 },
			{ false, true, 0x12 },
			{ false, false, 0x12 },
		};
		for (size_t i = 0; i < NUM_OF_ARRAY(tbl); i++) {
			const Tbl& p = tbl[i];
			char c = p.isH ? 'h' : 'l';
			const char *suf = p.isPd ? "pd" : "ps";
			const char *type = p.isPd ? "MM_0F | PP_66" : "MM_0F";
			printf("void vmov%c%s(const Xmm& x, const Operand& op1, const Operand& op2 = Operand()) { if (!op2.isNone() && !op2.isMEM()) throw ERR_BAD_COMBINATION; opAVX_X_X_XM(x, op1, op2, %s, 0x%02X, false); }\n"
				, c, suf, type, p.code);
			printf("void vmov%c%s(const Address& addr, const Xmm& x) { opAVX_X_X_XM(x, xm0, addr, %s, 0x%02X, false); }\n"
				, c, suf, type, p.code + 1);
		}
	}
	// FMA
	{
		const struct Tbl {
			uint8 code;
			const char *name;
			bool supportYMM;
		} tbl[] = {
			{ 0x08, "vfmadd", true },
			{ 0x09, "vfmadd", false },
			{ 0x06, "vfmaddsub", true },
			{ 0x07, "vfmsubadd", true },
			{ 0x0A, "vfmsub", true },
			{ 0x0B, "vfmsub", false },
			{ 0x0C, "vfnmadd", true },
			{ 0x0D, "vfnmadd", false },
			{ 0x0E, "vfnmsub", true },
			{ 0x0F, "vfnmsub", false },
		};
		for (size_t i = 0; i < NUM_OF_ARRAY(tbl); i++) {
			for (int j = 0; j < 2; j++) {
				const char suf[][2][8] = {
					{ "pd", "ps" },
					{ "sd", "ss" },
				};
				for (int k = 0; k < 3; k++) {
					const struct Ord {
						const char *str;
						uint8 code;
					} ord[] = {
						{ "132", 0x90 },
						{ "213", 0xA0 },
						{ "231", 0xB0 },
					};
					printf("void %s%s%s(const Xmm& xmm, const Xmm& op1, const Operand& op2 = Operand()) { opAVX_X_X_XM(xmm, op1, op2, MM_0F38 | PP_66, 0x%02X, %s, %d); }\n"
						, tbl[i].name, ord[k].str, suf[tbl[i].supportYMM ? 0 : 1][j], tbl[i].code + ord[k].code, tbl[i].supportYMM ? "true" : "false", j == 0 ? 1 : 0);
				}
			}
		}
	}
	// FMA others
	{
		printf("void vaesimc(const Xmm& x, const Operand& op) { opAVX_X_XM_IMM(x, op, MM_0F38 | PP_66, 0xDB, false, 0); }\n");

		printf("void vbroadcastf128(const Ymm& y, const Address& addr) { opAVX_X_XM_IMM(y, addr, MM_0F38 | PP_66, 0x1A, true, 0); }\n");
		printf("void vbroadcastsd(const Ymm& y, const Address& addr) { opAVX_X_XM_IMM(y, addr, MM_0F38 | PP_66, 0x19, true, 0); }\n");
		printf("void vbroadcastss(const Xmm& x, const Address& addr) { opAVX_X_XM_IMM(x, addr, MM_0F38 | PP_66, 0x18, true, 0); }\n");

		printf("void vextractf128(const Operand& op, const Ymm& y, uint8 imm) { opAVX_X_X_XMcvt(y, y.isXMM() ? xm0 : ym0, op, op.isXMM(), Operand::YMM, MM_0F3A | PP_66, 0x19, true, 0); db(imm); }\n");
		printf("void vextractps(const Operand& op, const Xmm& x, uint8 imm) { if (!(op.isREG(32) || op.isMEM()) || x.isYMM()) throw ERR_BAD_COMBINATION; opAVX_X_X_XMcvt(x, x.isXMM() ? xm0 : ym0, op, op.isREG(), Operand::XMM, MM_0F3A | PP_66, 0x17, false, 0); db(imm); }\n");
		printf("void vinsertf128(const Ymm& y1, const Ymm& y2, const Operand& op, uint8 imm) { opAVX_X_X_XMcvt(y1, y2, op, op.isXMM(), Operand::YMM, MM_0F3A | PP_66, 0x18, true, 0); db(imm); }\n");
		printf("void vperm2f128(const Ymm& y1, const Ymm& y2, const Operand& op, uint8 imm) { opAVX_X_X_XM(y1, y2, op, MM_0F3A | PP_66, 0x06, true, 0); db(imm); }\n");

		printf("void vlddqu(const Xmm& x, const Address& addr) { opAVX_X_X_XM(x, x.isXMM() ? xm0 : ym0, addr, MM_0F | PP_F2, 0xF0, true, 0); }\n");
		printf("void vldmxcsr(const Address& addr) { opAVX_X_X_XM(xm2, xm0, addr, MM_0F, 0xAE, false, -1); }\n");
		printf("void vstmxcsr(const Address& addr) { opAVX_X_X_XM(xm3, xm0, addr, MM_0F, 0xAE, false, -1); }\n");
		printf("void vmaskmovdqu(const Xmm& x1, const Xmm& x2) { opAVX_X_X_XM(x1, xm0, x2, MM_0F | PP_66, 0xF7, false, -1); }\n");

		printf("void vpextrb(const Operand& op, const Xmm& x, uint8 imm) { if (!op.isREG(i32e) && !op.isMEM()) throw ERR_BAD_COMBINATION; opAVX_X_X_XMcvt(x, xm0, op, !op.isMEM(), Operand::XMM, MM_0F3A | PP_66, 0x14, false); db(imm); }\n");
		// according to Intel' manual, VEX.W1 is ignored in 64-bit mode, then always VEX.W = 0, but I follow yasm encoding.
		printf("void vpextrw(const Reg& r, const Xmm& x, uint8 imm) { opAVX_X_X_XM(Xmm(r.getIdx()), xm0, x, MM_0F | PP_66, 0xC5, false, r.isBit(64) ? 1 : 0); db(imm); }\n");
		printf("void vpextrw(const Address& addr, const Xmm& x, uint8 imm) { opAVX_X_X_XM(x, xm0, addr, MM_0F3A | PP_66, 0x15, false); db(imm); }\n");
		printf("void vpextrd(const Operand& op, const Xmm& x, uint8 imm) { if (!op.isREG(32) && !op.isMEM()) throw ERR_BAD_COMBINATION; opAVX_X_X_XMcvt(x, xm0, op, !op.isMEM(), Operand::XMM, MM_0F3A | PP_66, 0x16, false, 0); db(imm); }\n");

		printf("void vpinsrb(const Xmm& x1, const Xmm& x2, const Operand& op, uint8 imm) { if (!op.isREG(32) && !op.isMEM()) throw ERR_BAD_COMBINATION; opAVX_X_X_XMcvt(x1, x2, op, !op.isMEM(), Operand::XMM, MM_0F3A | PP_66, 0x20, false); db(imm); }\n");
		printf("void vpinsrb(const Xmm& x, const Operand& op, uint8 imm) { if (!op.isREG(32) && !op.isMEM()) throw ERR_BAD_COMBINATION; opAVX_X_X_XMcvt(x, x, op, !op.isMEM(), Operand::XMM, MM_0F3A | PP_66, 0x20, false); db(imm); }\n");

		printf("void vpinsrw(const Xmm& x1, const Xmm& x2, const Operand& op, uint8 imm) { if (!op.isREG(32) && !op.isMEM()) throw ERR_BAD_COMBINATION; opAVX_X_X_XMcvt(x1, x2, op, !op.isMEM(), Operand::XMM, MM_0F | PP_66, 0xC4, false); db(imm); }\n");
		printf("void vpinsrw(const Xmm& x, const Operand& op, uint8 imm) { if (!op.isREG(32) && !op.isMEM()) throw ERR_BAD_COMBINATION; opAVX_X_X_XMcvt(x, x, op, !op.isMEM(), Operand::XMM, MM_0F | PP_66, 0xC4, false); db(imm); }\n");

		printf("void vpinsrd(const Xmm& x1, const Xmm& x2, const Operand& op, uint8 imm) { if (!op.isREG(32) && !op.isMEM()) throw ERR_BAD_COMBINATION; opAVX_X_X_XMcvt(x1, x2, op, !op.isMEM(), Operand::XMM, MM_0F3A | PP_66, 0x22, false, 0); db(imm); }\n");
		printf("void vpinsrd(const Xmm& x, const Operand& op, uint8 imm) { if (!op.isREG(32) && !op.isMEM()) throw ERR_BAD_COMBINATION; opAVX_X_X_XMcvt(x, x, op, !op.isMEM(), Operand::XMM, MM_0F3A | PP_66, 0x22, false, 0); db(imm); }\n");

		printf("void vpmovmskb(const Reg32e& r, const Xmm& x) { if (x.isYMM()) throw ERR_BAD_COMBINATION; opAVX_X_X_XM(Xmm(r.getIdx()), xm0, x, MM_0F | PP_66, 0xD7, false); }\n");

	}
	// (x, x, imm), (x, imm)
	{
		const struct Tbl {
			const char *name;
			uint8 code;
			int idx;
		} tbl[] = {
			{ "pslldq", 0x73, 7 },
			{ "psrldq", 0x73, 3 },
			{ "psllw", 0x71, 6 },
			{ "pslld", 0x72, 6 },
			{ "psllq", 0x73, 6 },
			{ "psraw", 0x71, 4 },
			{ "psrad", 0x72, 4 },
			{ "psrlw", 0x71, 2 },
			{ "psrld", 0x72, 2 },
			{ "psrlq", 0x73, 2 },
		};
		for (size_t i = 0; i < NUM_OF_ARRAY(tbl); i++) {
			const Tbl& p = tbl[i];
			printf("void v%s(const Xmm& x1, const Xmm& x2, uint8 imm) { opAVX_X_X_XM(xm%d, x1, x2, MM_0F | PP_66, 0x%02X, false); db(imm); }\n", p.name, p.idx, p.code);
			printf("void v%s(const Xmm& x, uint8 imm) { opAVX_X_X_XM(xm%d, x, x, MM_0F | PP_66, 0x%02X, false); db(imm); }\n", p.name, p.idx, p.code);
		}
	}
	// 4-op
	{
		printf("void vblendvpd(const Xmm& x1, const Xmm& x2, const Operand& op, const Xmm& x4) { opAVX_X_X_XM(x1, x2, op, MM_0F3A | PP_66, 0x4B, true); db(x4.getIdx() << 4); }\n");
		printf("void vblendvpd(const Xmm& x1, const Operand& op, const Xmm& x4) { opAVX_X_X_XM(x1, x1, op, MM_0F3A | PP_66, 0x4B, true); db(x4.getIdx() << 4); }\n");

		printf("void vblendvps(const Xmm& x1, const Xmm& x2, const Operand& op, const Xmm& x4) { opAVX_X_X_XM(x1, x2, op, MM_0F3A | PP_66, 0x4A, true); db(x4.getIdx() << 4); }\n");
		printf("void vblendvps(const Xmm& x1, const Operand& op, const Xmm& x4) { opAVX_X_X_XM(x1, x1, op, MM_0F3A | PP_66, 0x4A, true); db(x4.getIdx() << 4); }\n");

		printf("void vpblendvb(const Xmm& x1, const Xmm& x2, const Operand& op, const Xmm& x4) { opAVX_X_X_XM(x1, x2, op, MM_0F3A | PP_66, 0x4C, false); db(x4.getIdx() << 4); }\n");
		printf("void vpblendvb(const Xmm& x1, const Operand& op, const Xmm& x4) { opAVX_X_X_XM(x1, x1, op, MM_0F3A | PP_66, 0x4C, false); db(x4.getIdx() << 4); }\n");
	}
	// mov
	{
		printf("void vmovd(const Xmm& x, const Reg32& reg) { opAVX_X_X_XM(x, xm0, Xmm(reg.getIdx()), MM_0F | PP_66, 0x6E, false, 0); }\n");
		printf("void vmovd(const Xmm& x, const Address& addr) { opAVX_X_X_XM(x, xm0, addr, MM_0F | PP_66, 0x6E, false, 0); }\n");
		printf("void vmovd(const Reg32& reg, const Xmm& x) { opAVX_X_X_XM(x, xm0, Xmm(reg.getIdx()), MM_0F | PP_66, 0x7E, false, 0); }\n");
		printf("void vmovd(const Address& addr, const Xmm& x) { opAVX_X_X_XM(x, xm0, addr, MM_0F | PP_66, 0x7E, false, 0); }\n");

		printf("void vmovq(const Xmm& x, const Address& addr) { opAVX_X_X_XM(x, xm0, addr, MM_0F | PP_F3, 0x7E, false, -1); }\n");
		printf("void vmovq(const Address& addr, const Xmm& x) { opAVX_X_X_XM(x, xm0, addr, MM_0F | PP_66, 0xD6, false, -1); }\n");
		printf("void vmovq(const Xmm& x1, const Xmm& x2) { opAVX_X_X_XM(x1, xm0, x2, MM_0F | PP_F3, 0x7E, false, -1); }\n");

		printf("void vmovhlps(const Xmm& x1, const Xmm& x2, const Operand& op = Operand()) { if (!op.isNone() && !op.isXMM()) throw ERR_BAD_COMBINATION; opAVX_X_X_XM(x1, x2, op, MM_0F, 0x12, false); }\n");
		printf("void vmovlhps(const Xmm& x1, const Xmm& x2, const Operand& op = Operand()) { if (!op.isNone() && !op.isXMM()) throw ERR_BAD_COMBINATION; opAVX_X_X_XM(x1, x2, op, MM_0F, 0x16, false); }\n");

		printf("void vmovmskpd(const Reg& r, const Xmm& x) { if (!r.isBit(i32e)) throw ERR_BAD_COMBINATION; opAVX_X_X_XM(x.isXMM() ? Xmm(r.getIdx()) : Ymm(r.getIdx()), x.isXMM() ? xm0 : ym0, x, MM_0F | PP_66, 0x50, true, 0); }\n");
		printf("void vmovmskps(const Reg& r, const Xmm& x) { if (!r.isBit(i32e)) throw ERR_BAD_COMBINATION; opAVX_X_X_XM(x.isXMM() ? Xmm(r.getIdx()) : Ymm(r.getIdx()), x.isXMM() ? xm0 : ym0, x, MM_0F, 0x50, true, 0); }\n");

		printf("void vmovntdq(const Address& addr, const Xmm& x) { opAVX_X_X_XM(x, x.isXMM() ? xm0 : ym0, addr, MM_0F | PP_66, 0xE7, true); }\n");
		printf("void vmovntpd(const Address& addr, const Xmm& x) { opAVX_X_X_XM(x, x.isXMM() ? xm0 : ym0, addr, MM_0F | PP_66, 0x2B, true); }\n");
		printf("void vmovntps(const Address& addr, const Xmm& x) { opAVX_X_X_XM(x, x.isXMM() ? xm0 : ym0, addr, MM_0F, 0x2B, true); }\n");
		printf("void vmovntdqa(const Xmm& x, const Address& addr) { opAVX_X_X_XM(x, xm0, addr, MM_0F38 | PP_66, 0x2A, false); }\n");

		// vmovsd, vmovss
		for (int i = 0; i < 2; i++) {
			char c1 = i == 0 ? 'd' : 's';
			char c2 = i == 0 ? '2' : '3';
			printf("void vmovs%c(const Xmm& x1, const Xmm& x2, const Operand& op = Operand()) { if (!op.isNone() && !op.isXMM()) throw ERR_BAD_COMBINATION; opAVX_X_X_XM(x1, x2, op, MM_0F | PP_F%c, 0x10, false); }\n", c1, c2);
			printf("void vmovs%c(const Xmm& x, const Address& addr) { opAVX_X_X_XM(x, xm0, addr, MM_0F | PP_F%c, 0x10, false); }\n", c1, c2);
			printf("void vmovs%c(const Address& addr, const Xmm& x) { opAVX_X_X_XM(x, xm0, addr, MM_0F | PP_F%c, 0x11, false); }\n", c1, c2);
		}
	}
	// cvt
	{
		printf("void vcvtss2si(const Reg32& r, const Operand& op) { opAVX_X_X_XM(Xmm(r.getIdx()), xm0, op, MM_0F | PP_F3, 0x2D, false, 0); }\n");
		printf("void vcvttss2si(const Reg32& r, const Operand& op) { opAVX_X_X_XM(Xmm(r.getIdx()), xm0, op, MM_0F | PP_F3, 0x2C, false, 0); }\n");
		printf("void vcvtsd2si(const Reg32& r, const Operand& op) { opAVX_X_X_XM(Xmm(r.getIdx()), xm0, op, MM_0F | PP_F2, 0x2D, false, 0); }\n");
		printf("void vcvttsd2si(const Reg32& r, const Operand& op) { opAVX_X_X_XM(Xmm(r.getIdx()), xm0, op, MM_0F | PP_F2, 0x2C, false, 0); }\n");

		printf("void vcvtsi2ss(const Xmm& x, const Operand& op1, const Operand& op2 = Operand()) { if (!op2.isNone() && !(op2.isREG(i32e) || op2.isMEM())) throw ERR_BAD_COMBINATION; opAVX_X_X_XMcvt(x, op1, op2, op2.isREG(), Operand::XMM, MM_0F | PP_F3, 0x2A, false, (op1.isMEM() || op2.isMEM()) ? -1 : (op1.isREG(32) || op2.isREG(32)) ? 0 : 1); }\n");
		printf("void vcvtsi2sd(const Xmm& x, const Operand& op1, const Operand& op2 = Operand()) { if (!op2.isNone() && !(op2.isREG(i32e) || op2.isMEM())) throw ERR_BAD_COMBINATION; opAVX_X_X_XMcvt(x, op1, op2, op2.isREG(), Operand::XMM, MM_0F | PP_F2, 0x2A, false, (op1.isMEM() || op2.isMEM()) ? -1 : (op1.isREG(32) || op2.isREG(32)) ? 0 : 1); }\n");

		printf("void vcvtps2pd(const Xmm& x, const Operand& op) { if (!op.isMEM() && !op.isXMM()) throw ERR_BAD_COMBINATION; opAVX_X_X_XMcvt(x, x.isXMM() ? xm0 : ym0, op, !op.isMEM(), x.isXMM() ? Operand::XMM : Operand::YMM, MM_0F, 0x5A, true); }\n");
		printf("void vcvtdq2pd(const Xmm& x, const Operand& op) { if (!op.isMEM() && !op.isXMM()) throw ERR_BAD_COMBINATION; opAVX_X_X_XMcvt(x, x.isXMM() ? xm0 : ym0, op, !op.isMEM(), x.isXMM() ? Operand::XMM : Operand::YMM, MM_0F | PP_F3, 0xE6, true); }\n");

		printf("void vcvtpd2ps(const Xmm& x, const Operand& op) { if (x.isYMM()) throw ERR_BAD_COMBINATION; opAVX_X_X_XM(op.isYMM() ? Ymm(x.getIdx()) : x, op.isYMM() ? ym0 : xm0, op, MM_0F | PP_66, 0x5A, true); }\n");
		printf("void vcvtpd2dq(const Xmm& x, const Operand& op) { if (x.isYMM()) throw ERR_BAD_COMBINATION; opAVX_X_X_XM(op.isYMM() ? Ymm(x.getIdx()) : x, op.isYMM() ? ym0 : xm0, op, MM_0F | PP_F2, 0xE6, true); }\n");
		printf("void vcvttpd2dq(const Xmm& x, const Operand& op) { if (x.isYMM()) throw ERR_BAD_COMBINATION; opAVX_X_X_XM(op.isYMM() ? Ymm(x.getIdx()) : x, op.isYMM() ? ym0 : xm0, op, MM_0F | PP_66, 0xE6, true); }\n");
	}
	// x64
	{

		printf("#ifdef XBYAK64\n");
		printf("void vmovq(const Xmm& x, const Reg64& reg) { opAVX_X_X_XM(x, xm0, Xmm(reg.getIdx()), MM_0F | PP_66, 0x6E, false, 1); }\n");
		printf("void vmovq(const Reg64& reg, const Xmm& x) { opAVX_X_X_XM(x, xm0, Xmm(reg.getIdx()), MM_0F | PP_66, 0x7E, false, 1); }\n");

		printf("void vpextrq(const Operand& op, const Xmm& x, uint8 imm) { if (!op.isREG(64) && !op.isMEM()) throw ERR_BAD_COMBINATION; opAVX_X_X_XMcvt(x, xm0, op, !op.isMEM(), Operand::XMM, MM_0F3A | PP_66, 0x16, false, 1); db(imm); }\n");

		printf("void vpinsrq(const Xmm& x1, const Xmm& x2, const Operand& op, uint8 imm) { if (!op.isREG(64) && !op.isMEM()) throw ERR_BAD_COMBINATION; opAVX_X_X_XMcvt(x1, x2, op, !op.isMEM(), Operand::XMM, MM_0F3A | PP_66, 0x22, false, 1); db(imm); }\n");
		printf("void vpinsrq(const Xmm& x, const Operand& op, uint8 imm) { if (!op.isREG(64) && !op.isMEM()) throw ERR_BAD_COMBINATION; opAVX_X_X_XMcvt(x, x, op, !op.isMEM(), Operand::XMM, MM_0F3A | PP_66, 0x22, false, 1); db(imm); }\n");

		printf("void vcvtss2si(const Reg64& r, const Operand& op) { opAVX_X_X_XM(Xmm(r.getIdx()), xm0, op, MM_0F | PP_F3, 0x2D, false, 1); }\n");
		printf("void vcvttss2si(const Reg64& r, const Operand& op) { opAVX_X_X_XM(Xmm(r.getIdx()), xm0, op, MM_0F | PP_F3, 0x2C, false, 1); }\n");
		printf("void vcvtsd2si(const Reg64& r, const Operand& op) { opAVX_X_X_XM(Xmm(r.getIdx()), xm0, op, MM_0F | PP_F2, 0x2D, false, 1); }\n");
		printf("void vcvttsd2si(const Reg64& r, const Operand& op) { opAVX_X_X_XM(Xmm(r.getIdx()), xm0, op, MM_0F | PP_F2, 0x2C, false, 1); }\n");
		printf("#endif\n");
	}
}

int main()
{
	put();
	return 0;
}
