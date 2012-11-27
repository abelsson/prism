#include <iostream>
#include <cstdio>

#include "node.h"
#include "codegen.h"


extern Block* programBlock;
extern int yyparse();
extern int error_count;

extern "C"
double print(double x) {
  printf("%f\n",x);
  return x;
}

extern FILE *yyin;

int main(int argc, char **argv)
{
    if (argc == 2)
        yyin = fopen(argv[1], "r");
    else
        yyin = fopen("../prism/test.prism","r");
    yyparse();

    if (error_count == 0) {
        std::cout << programBlock << std::endl;
        CodeGenContext context;
        context.generateCode(*programBlock);

        programBlock->print(0);

        context.runCode();
    }

    return 0;
}
