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
extern int debug;

int main(int argc, char **argv)
{
    if (argc == 2)
        yyin = fopen(argv[1], "r");
    else
        yyin = fopen("../prism/test.prism","r");
    yyparse();

    if (error_count == 0) {
        CodeGenContext context;

        Context root(0);

        Visitor *tv = new TypeVisitor;
        programBlock->set_context(&root);
        programBlock->accept(tv);

        std::cout << programBlock->yaml() << std::endl;

        context.generate_code(*programBlock);
        context.run_code();
    }

    return 0;
}
