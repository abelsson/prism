%{
    #include <cstdio>
    #include "node.h"
    Block *programBlock; /* the top level root node of our final AST */

    int line_num = 1;
    int error_count = 0;
    extern int yylex();
    void yyerror(const char *s)
    {
      printf("ERROR: line %d: %s\n", line_num, s);
      error_count++;
    }
%}
%error-verbose


/* Represents the many different ways we can access our data */
%union {
    Node *node;
    Block *block;
    Expression *expr;
    Statement *stmt;
    Identifier *ident;
    VariableDeclaration *var_decl;
    std::vector<VariableDeclaration*> *varvec;
    std::vector<Expression*> *exprvec;
    std::string *string;
    int token;
}

/* Define our terminal symbols (tokens). This should
   match our tokens.l lex file. We also define the node type
   they represent.
 */
%token <string> TIDENTIFIER TINTEGER TDOUBLE TSTRING
%token <token> TCEQ TCNE TCLT TCLE TCGT TCGE TEQUAL
%token <token> TLPAREN TRPAREN TLBRACE TRBRACE TCOMMA TDOT TSEMICOLON TNEWLINE
%token <token> TPLUS TMINUS TMUL TDIV
%token <token> TEXTERN TIF TELSE TRETURN TFUNC TASSERT TPRINT

/* Define the type of node our nonterminal symbols represent.
   The types refer to the %union declaration above. Ex: when
   we call an ident (defined by union type ident) we are really
   calling an (NIdentifier*). It makes the compiler happy.
 */
%type <ident> ident
%type <expr> numeric string expr
%type <varvec> func_decl_args
%type <exprvec> call_args
%type <block> program stmts block
%type <stmt> stmt var_decl func_decl extern_func_decl if_stmt return_stmt assert_stmt print_stmt
%type <token> comparison
/* Operator precedence for mathematical operators */
%left TCEQ TCNE TCLT TCLE TCGT TCGE
%left TPLUS TMINUS
%left TMUL TDIV

%start program

%%

program : stmts { programBlock = $1; }
        ;

stmts : stmt { $$ = new Block(); $$->statements.push_back($<stmt>1); }
      | stmts stmt { $1->statements.push_back($<stmt>2); }
      | stmts TNEWLINE {}
      | stmts error { yyclearin; yyerrok; }
      ;

stmt : var_decl TSEMICOLON
     | func_decl
     | extern_func_decl
     | if_stmt
     | return_stmt TSEMICOLON
     | assert_stmt TSEMICOLON
     | print_stmt TSEMICOLON
     | expr TSEMICOLON { $$ = new ExpressionStatement(*$1); }
     ;

block : TLBRACE stmts TRBRACE { $$ = $2; }
      | TLBRACE TRBRACE { $$ = new Block(); }
      ;

var_decl : ident ident { $$ = new VariableDeclaration($1, $2); }
         | ident ident TEQUAL expr { $$ = new VariableDeclaration($1, $2, $4); }
         ;

if_stmt  : TIF TLPAREN expr TRPAREN block TELSE block { $$ = new IfStatement($3, $5, $7); }
         | TIF TLPAREN expr TRPAREN block  { $$ = new IfStatement($3, $5, 0); }
         ;

assert_stmt: TASSERT expr { $$ = new AssertStatement($2); }
         ;

print_stmt: TPRINT expr { $$ = new PrintStatement($2); }
         ;

return_stmt: TRETURN expr { $$ = new ReturnStatement($2); }
         ;
extern_func_decl: TEXTERN ident ident TLPAREN func_decl_args TRPAREN TSEMICOLON { $$ = new ExternDeclaration($2, $3, $5); }
         ;

func_decl : TFUNC ident TEQUAL ident TLPAREN func_decl_args TRPAREN block
            { $$ = new FunctionDeclaration($4, $2, $6, $8); }
          ;

func_decl_args : /*blank*/  { $$ = new VariableList(); }
          | var_decl { $$ = new VariableList(); $$->push_back($<var_decl>1); }
          | func_decl_args TCOMMA var_decl { $1->push_back($<var_decl>3); }
          ;

ident : TIDENTIFIER { $$ = new Identifier(*$1); delete $1; }
      ;

numeric : TINTEGER { $$ = new Integer(atol($1->c_str())); delete $1; }
        | TDOUBLE { $$ = new Double(atof($1->c_str())); delete $1; }
        ;

string : TSTRING { $$ = new String($1); delete $1; }
       ;

expr : ident TEQUAL expr { $$ = new Assignment($<ident>1, $3); }
     | ident TLPAREN call_args TRPAREN { $$ = new MethodCall($1, $3);  }
     | ident { $<ident>$ = $1; }
     | numeric
     | string
     | expr TPLUS expr { $$ = new BinaryOperator($1, $2, $3); }
     | expr TMINUS expr { $$ = new BinaryOperator($1, $2, $3); }
     | expr TMUL expr { $$ = new BinaryOperator($1, $2, $3); }
     | expr TDIV expr { $$ = new BinaryOperator($1, $2, $3); }
     | expr comparison expr { $$ = new BinaryOperator($1, $2, $3); }
     | TLPAREN expr TRPAREN { $$ = $2; }
     ;

call_args : /*blank*/  { $$ = new ExpressionList(); }
          | expr { $$ = new ExpressionList(); $$->push_back($1); }
          | call_args TCOMMA expr  { $1->push_back($3); }
          ;

comparison : TCEQ | TCNE | TCLT | TCLE | TCGT | TCGE
           ;

%%
