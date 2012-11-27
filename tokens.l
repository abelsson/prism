%{
#include <string>
#include "node.h"
#include "parser.hpp"
#define SAVE_TOKEN yylval.string = new std::string(yytext, yyleng)
#define TOKEN(t) (yylval.token = t)
extern "C" int yywrap() { }
extern int line_num;
%}

%%

[ \t]                   ;
\n                      line_num++;
[0-9]+\.[0-9]*          SAVE_TOKEN; return TDOUBLE;
[0-9]+                  SAVE_TOKEN; return TINTEGER;
L?\"(\\.|[^\\"])*\"     SAVE_TOKEN; return TSTRING;
L?'(\\.|[^\\'])*'       SAVE_TOKEN; return TSTRING;
"="                     return TOKEN(TEQUAL);
"=="                    return TOKEN(TCEQ);
"!="                    return TOKEN(TCNE);
"<"                     return TOKEN(TCLT);
"<="                    return TOKEN(TCLE);
">"                     return TOKEN(TCGT);
">="                    return TOKEN(TCGE);
"("                     return TOKEN(TLPAREN);
")"                     return TOKEN(TRPAREN);
"{"                     return TOKEN(TLBRACE);
"}"                     return TOKEN(TRBRACE);
"."                     return TOKEN(TDOT);
","                     return TOKEN(TCOMMA);
"+"                     return TOKEN(TPLUS);
"-"                     return TOKEN(TMINUS);
"*"                     return TOKEN(TMUL);
"/"                     return TOKEN(TDIV);
";"                     return TOKEN(TSEMICOLON);
"extern"                return TOKEN(TEXTERN);
"if"                    return TOKEN(TIF);
"else"                  return TOKEN(TELSE);
"return"                return TOKEN(TRETURN);
"func"                  return TOKEN(TFUNC);
"assert"                return TOKEN(TASSERT);
[a-zA-Z_][a-zA-Z0-9_]*  SAVE_TOKEN; return TIDENTIFIER;
.                       printf("Unknown token!\n"); yyterminate();

%%
