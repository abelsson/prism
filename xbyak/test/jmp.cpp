#include <stdio.h>
#include <string.h>
#include <string>
#include <xbyak/xbyak.h>
#define NUM_OF_ARRAY(x) (sizeof(x) / sizeof(x[0]))

#if !defined(_WIN64) && !defined(__x86_64__)
	#define ONLY_32BIT
#endif

using namespace Xbyak;

struct TestJmp : public Xbyak::CodeGenerator {
	void putNop(int n)
	{
		for (int i = 0; i < n; i++) {
			nop();
		}
	}
/*
     4                                  X0:
     5 00000004 EBFE                    jmp short X0
     6
     7                                  X1:
     8 00000006 <res 00000001>          dummyX1 resb 1
     9 00000007 EBFD                    jmp short X1
    10
    11                                  X126:
    12 00000009 <res 0000007E>          dummyX126 resb 126
    13 00000087 EB80                    jmp short X126
    14
    15                                  X127:
    16 00000089 <res 0000007F>          dummyX127 resb 127
    17 00000108 E97CFFFFFF              jmp near X127
    18
    19 0000010D EB00                    jmp short Y0
    20                                  Y0:
    21
    22 0000010F EB01                    jmp short Y1
    23 00000111 <res 00000001>          dummyY1 resb 1
    24                                  Y1:
    25
    26 00000112 EB7F                    jmp short Y127
    27 00000114 <res 0000007F>          dummyY127 resb 127
    28                                  Y127:
    29
    30 00000193 E980000000              jmp near Y128
    31 00000198 <res 00000080>          dummyY128 resb 128
    32                                  Y128:
*/
	TestJmp(int offset, bool isBack, bool isShort)
	{
		if (isBack) {
			L("@@");
			putNop(offset);
			jmp("@b");
		} else {
			if (isShort) {
				jmp("@f");
			} else {
				jmp("@f", T_NEAR);
			}
			putNop(offset);
			L("@@");
		}
	}
};

void test1()
{
	static const struct Tbl {
		int offset;
		bool isBack;
		bool isShort;
		uint8 result[6];
		int size;
	} tbl[] = {
		{ 0, true, true, { 0xeb, 0xfe }, 2 },
		{ 1, true, true, { 0xeb, 0xfd }, 2 },
		{ 126, true, true, { 0xeb, 0x80 }, 2 },
		{ 127, true, false, {0xe9, 0x7c, 0xff, 0xff, 0xff }, 5 },
		{ 0, false, true, { 0xeb, 0x00 }, 2 },
		{ 1, false, true, { 0xeb, 0x01 }, 2 },
		{ 127, false, true, { 0xeb, 0x7f }, 2 },
		{ 128, false, false, { 0xe9, 0x80, 0x00, 0x00, 0x00 }, 5 },
	};
	for (size_t i = 0; i < NUM_OF_ARRAY(tbl); i++) {
		const Tbl *p = &tbl[i];
		TestJmp jmp(p->offset, p->isBack, p->isShort);
		const uint8 *q = (const uint8*)jmp.getCode();
		if (p->isBack) q += p->offset; /* skip nop */
		for (int j = 0; j < p->size; j++) {
			if (q[j] != p->result[j]) {
				printf("err (%d,%d) %02x assume=%02x\n", (int)i, j, q[j], p->result[j]);
			}
		}
	}
	puts("ok");
}

struct TestJmp2 : public CodeGenerator {
	void putNop(int n)
	{
		for (int i = 0; i < n; i++) {
			nop();
		}
	}
/*
  1 00000000 90                      nop
  2 00000001 90                      nop
  3                                  f1:
  4 00000002 <res 0000007E>          dummyX1 resb 126
  6 00000080 EB80                     jmp f1
  7
  8                                  f2:
  9 00000082 <res 0000007F>          dummyX2 resb 127
 11 00000101 E97CFFFFFF               jmp f2
 12
 13
 14 00000106 EB7F                    jmp f3
 15 00000108 <res 0000007F>          dummyX3 resb 127
 17                                  f3:
 18
 19 00000187 E980000000              jmp f4
 20 0000018C <res 00000080>          dummyX4 resb 128
 22                                  f4:
*/
	explicit TestJmp2(void *p)
		: Xbyak::CodeGenerator(8192, p)
	{
		inLocalLabel();
		nop();
		nop();
	L(".f1");
		putNop(126);
		jmp(".f1");
	L(".f2");
		putNop(127);
		jmp(".f2", T_NEAR);

		jmp(".f3");
		putNop(127);
	L(".f3");
		jmp(".f4", T_NEAR);
		putNop(128);
	L(".f4");
		outLocalLabel();
	}
};

void test2()
{
	puts("test2");
	std::string ok;
	ok.resize(0x18C + 128, (char)0x90);
	ok[0x080] = (char)0xeb;
	ok[0x081] = (char)0x80;

	ok[0x101] = (char)0xe9;
	ok[0x102] = (char)0x7c;
	ok[0x103] = (char)0xff;
	ok[0x104] = (char)0xff;
	ok[0x105] = (char)0xff;

	ok[0x106] = (char)0xeb;
	ok[0x107] = (char)0x7f;

	ok[0x187] = (char)0xe9;
	ok[0x188] = (char)0x80;
	ok[0x189] = (char)0x00;
	ok[0x18a] = (char)0x00;
	ok[0x18b] = (char)0x00;
	for (int j = 0; j < 2; j++) {
		TestJmp2 c(j == 0 ? 0 : Xbyak::AutoGrow);
		c.ready();
		std::string m((const char*)c.getCode(), c.getSize());
		if (m.size() != ok.size()) {
			printf("test2 err %d %d\n", (int)m.size(), (int)ok.size());
		} else {
			if (m != ok) {
				for (size_t i = 0; i < m.size(); i++) {
					if (m[i] != ok[i]) {
						printf("diff 0x%03x %02x %02x\n", (int)i, (unsigned char)m[i], (unsigned char)ok[i]);
					}
				}
			} else {
				puts("ok");
			}
		}
	}
}

#ifdef ONLY_32BIT
int add5(int x) { return x + 5; }
int add2(int x) { return x + 2; }

struct Grow : Xbyak::CodeGenerator {
	Grow(int dummySize)
		: Xbyak::CodeGenerator(128, Xbyak::AutoGrow)
	{
		mov(eax, 100);
		push(eax);
		call((void*)add5);
		add(esp, 4);
		push(eax);
		call((void*)add2);
		add(esp, 4);
		ret();
		for (int i = 0; i < dummySize; i++) {
			db(0);
		}
	}
};

void test3()
{
	for (int dummySize = 0; dummySize < 40000; dummySize += 10000) {
		printf("dummySize=%d\n", dummySize);
		Grow g(dummySize);
		g.ready();
		int (*f)() = (int (*)())g.getCode();
		int x = f();
		const int ok = 107;
		if (x != ok) {
			printf("err %d assume %d\n", x, ok);
		} else {
			printf("ok\n");
		}
	}
}
#endif

Xbyak::uint8 bufL[4096 * 32];
Xbyak::uint8 bufS[4096 * 2];

struct MyAllocator : Xbyak::Allocator {
	Xbyak::uint8 *alloc(size_t size)
	{
		if (size < sizeof(bufS)) {
			printf("use bufS(%d)\n", (int)size);
			return bufS;
		}
		if (size < sizeof(bufL)) {
			printf("use bufL(%d)\n", (int)size);
			return bufL;
		}
		fprintf(stderr, "no memory %d\n", (int)size);
		exit(1);
	}
	void free(Xbyak::uint8 *)
	{
	}
} myAlloc;

void dump(const std::string& m)
{
	printf("size=%d\n     ", (int)m.size());
	for (int i = 0; i < 16; i++) {
		printf("%02x ", i);
	}
	printf("\n     ");
	for (int i = 0; i < 16; i++) {
		printf("---");
	}
	printf("\n");
	for (size_t i = 0; i < m.size(); i++) {
		if ((i % 16) == 0) printf("%04x ", (int)(i / 16));
		printf("%02x ", (unsigned char)m[i]);
		if ((i % 16) == 15) putchar('\n');
	}
	putchar('\n');
}

void diff(const std::string& a, const std::string& b)
{
	puts("diff");
	if (a.size() != b.size()) printf("size diff %d %d\n", (int)a.size(), (int)b.size());
	for (size_t i = 0; i < a.size(); i++) {
		if (a[i] != b[i]) {
			printf("diff %d(%04x) %02x %02x\n", (int)i, (int)i, (unsigned char)a[i], (unsigned char)b[i]);
		}
	}
	puts("end");
}

struct Test4 : Xbyak::CodeGenerator {
	explicit Test4(int size, void *mode)
		: CodeGenerator(size, mode)
	{
		using namespace Xbyak;
		inLocalLabel();
		outLocalLabel();
		jmp(".x");
		for (int i = 0; i < 10; i++) {
			nop();
		}
	L(".x");
		ret();
	}
};
void test4()
{
	std::string fm, gm;
	Test4 fc(1024, 0);
	Test4 gc(5, Xbyak::AutoGrow);
	gc.ready();
	fm.assign((const char*)fc.getCode(), fc.getSize());
	gm.assign((const char*)gc.getCode(), gc.getSize());
//	dump(fm);
//	dump(gm);
	diff(gm, gm);
}

struct Test5 : Xbyak::CodeGenerator {
	explicit Test5(int size, int count, void *mode)
		: CodeGenerator(size, mode, &myAlloc)
	{
		using namespace Xbyak;
		inLocalLabel();
		mov(ecx, count);
		xor(eax, eax);
	L(".lp");
		for (int i = 0; i < count; i++) {
			L(Label::toStr(i).c_str());
			add(eax, 1);
			int to = 0;
			if (i < count / 2) {
				to = count - 1 - i;
			} else {
				to = count  - i;
			}
			if (i == count / 2) {
				jmp(".exit", T_NEAR);
			} else {
				jmp(Label::toStr(to).c_str(), T_NEAR);
			}
		}
	L(".exit");
		sub(ecx, 1);
		jnz(".lp", T_NEAR);
		ret();
		outLocalLabel();
	}
};

void test5()
{
	std::string fm, gm;
	const int count = 50;
	int ret;
	Test5 fc(1024 * 64, count, 0);
	ret = ((int (*)())fc.getCode())();
	if (ret != count * count) {
		printf("err ret=%d, %d\n", ret, count * count);
	} else {
		puts("ok");
	}
	fm.assign((const char*)fc.getCode(), fc.getSize());
	Test5 gc(10, count, Xbyak::AutoGrow);
	gc.ready();
#if 0
	ret = ((int (*)())gc.getCode())();
	if (ret != count * count) {
		printf("err ret=%d, %d\n", ret, count * count);
	} else {
		puts("ok");
	}
#endif
	gm.assign((const char*)gc.getCode(), gc.getSize());
	diff(fm, gm);
}

int main()
{
	try {
		test1();
		test2();
#ifdef ONLY_32BIT
		test3();
#endif
		test4();
		test5();
	} catch (Xbyak::Error err) {
		printf("ERR:%s(%d)\n", Xbyak::ConvertErrorToString(err), err);
	} catch (...) {
		printf("unknown error\n");
	}
}
