 /*
  *  The scanner definition for seal.
  */

 /*
  *  Stuff enclosed in %{ %} in the first section is copied verbatim to the
  *  output, so headers and global definitions are placed here to be visible
  * to the code in the file.  Don't remove anything that was here initially
  */
%{

#include <seal-parse.h>
#include <stringtab.h>
#include <string.h>
#include <utilities.h>
#include <stdint.h>
#include <stdlib.h>
#include <cstdio>
#include <cstdlib>
#include <stdio.h>

using namespace std;
/* The compiler assumes these identifiers. */
#define yylval seal_yylval
#define yylex  seal_yylex

/* Max size of string constants */
#define MAX_STR_CONST 256
#define YY_NO_UNPUT   /* keep g++ happy */

extern FILE *fin; /* we read from this file */

/* define YY_INPUT so we read from the FILE fin:
 * This change makes it possible to use this scanner in
 * the seal compiler.
 */
#undef YY_INPUT
#define YY_INPUT(buf,result,max_size) \
	if ( (result = fread( (char*)buf, sizeof(char), max_size, fin)) < 0) \
		YY_FATAL_ERROR( "read() in flex scanner failed");

char string_buf[MAX_STR_CONST]; /* to assemble string constants */
char *string_buf_ptr;

extern int curr_lineno;
extern int verbose_flag;

extern YYSTYPE seal_yylval;
static int comment_num=0;
static bool flag=false;
static bool flag2=false;
/*
 *  Add Your own definitions here
 */

%}
 /*
  * Define names for regular expressions here.
  */
%option noyywrap

DIGIT  [0-9]
DEC [1-9]*[0-9]+
HEX 0[xX][0-9a-fA-F]+
FLOAT {DIGIT}+"."{DIGIT}*
STR1 [^\n]
STR2 [^\n]*"\n"
STR3 [^n]*"\\n"
VOID "void"
OBJECTID [a-z][0-9a-zA-Z_]*
TYPEID ("Int"|"Float"|"String"|"Bool"|"Void") /*是否需要hex*/
IF "if"
ELSE "else"
WHILE "while"
FOR "for"
CONTINUE "contine"
FUNC "func"
RETURN "return"
VAR "var"
STRUCT "struct"
AND "&&"
OR "||"
EQUAL "=="
LP "{"
RP "}"
LB "("
RB ")"
PLUS "+"
MINUS "-" /*还有一种单目取负*/
CHENG "*"
DIV "/"
MOD "%"
YU "&"
OOR "|"
XOR "^"
YOR "~"
DENGYU "="
NE "!="
GE ">="
LE "<="
DAYU ">"
XIAOYU "<"
NOT "!"
FEN ";"
DO ","
ERRORA [A-Z0-9_][a-zA-Z0-9_]*
ERRORB [^A-Za-z0-9_][a-zA-Z0-9_]+
%Start          COMMENTS
%Start          INSTANTSTRING
%Start          STRING


%%
 /* 单行注释 */
"//"[^\n]* { }

 /* 多行注释 */
<INITIAL,COMMENTS>"/*" {
    comment_num++;
    BEGIN COMMENTS;
}

 /* 跳过注释里面的内容 */
<COMMENTS>[^\n/*]* { }

<COMMENTS>[/*] { }

 /* 注释结束 */
<COMMENTS>"*/" {
    comment_num--;
    if (comment_num == 0) {
        BEGIN 0;
    }
}
 /* 注释未结束遇到文件结束 */
<COMMENTS><<EOF>> {
    strcpy(yylval.error_msg,"Error: Meeting EOF when in comment");
    BEGIN 0;
    return ERROR;
}
 /* 多行注释不匹配 */
"*/" {
    strcpy(yylval.error_msg,"Error: */ don't have matched /*");
    return ERROR;
}


 /* 字符串 */

 /* 用反撇号`开始的字符串 */
<INITIAL>(`) {
    BEGIN INSTANTSTRING;
    yymore();
}

 

 /* 字符串未闭合遇到文件结束  */
<INSTANTSTRING><<EOF>> {
    strcpy(yylval.error_msg, "Error: Meeting EOF when in string");
    BEGIN 0;
    yyrestart(yyin);
    return ERROR;
}

 /* 记录字符串内容除了单独处理的 */
<INSTANTSTRING>[^`\n]* { yymore(); }


 /* 换行符 */
<INSTANTSTRING>\n {
    curr_lineno++;
    yymore();
}

 /* 字符串结束 */
<INSTANTSTRING>(`) {
 /*  int i,j;
    char str[400];
    for(i = j = 0;i<yyleng-1; i++)
        if(yytext[i] != '`')
            str[j] = yytext[i];
            j++;
    str[j]='\0';
    if(j>255) {
         strcpy(yylval.error_msg, "Error: the length of String is more than 256");
         BEGIN 0;
         return ERROR;
    }
    char *out=new char[j];
    strncpy(out,str,j);
    seal_yylval.symbol = stringtable.add_string(out);
    BEGIN 0;
    return CONST_STRING;
    */
    int i = 0,j = 0;
    for(i = j = 0;*(yytext + i) != '\0'; i++)
        if(yytext[i] != '`')
            yytext[j++] = yytext[i];
    yytext[j]='\0';

    seal_yylval.symbol = stringtable.add_string(yytext);
    BEGIN 0;
    return CONST_STRING;
}

 /* 引号类型字符串 */
<INITIAL>\" {
    BEGIN STRING;
    yymore();
}

 /* 记录字符串内容除了特殊处理的 */
<STRING>[^\\\"\n]* { 
    yymore(); 
    }


<STRING>\\[^\n0] { 
    yymore(); 
    }

 /* 正确的转义换行符 */
<STRING>\\\n {
    curr_lineno++;
    yymore();
}

 /* 
<STRING>\n {
    strcpy(yylval.error_msg, "Error: String Contains illegal \n Character");
    BEGIN 0;
    curr_lineno++;
    return ERROR;
}
 */

 /* 遇到不正确的换行符 */
<STRING>\n {
    curr_lineno++;
    flag2=true;
    yymore();
}

 /* 遇到空字符 */
<STRING>\\0 {
   flag=true;
   yymore();
}

 /*
<STRING>\\0\"{
   strcpy(yylval.error_msg,"Error: String contains illegal \0 character");
   BEGIN 0;
   return ERROR;
}
 */

 /* 未闭合字符串遇到文件结束 */
<STRING><<EOF>> {
    strcpy(yylval.error_msg, "Error: Meeting EOF when in string");
    BEGIN 0;
    yyrestart(yyin);
    return ERROR;
}
 /*  
<STRING>[^\n"]*"\\\n"[^"]*\" {
  string input(yytext,yyleng);
  input = input.substr(1, input.length() - 2);
  string output = "";
  string::size_type k;
  BEGIN 0;
  if(yyleng>256) {
     strcpy(seal_yylval.error_msg,"Error: The length of the string exceeds a maximum of 256");
     return ERROR;
  }
  char str[400];
  int j=0;
  int i;
  while ((k = input.find_first_of("\\")) != string::npos) {
        output += input.substr(0, k);
        switch (input[k + 1]) {
        
        case 'n':
            output += "\n";
            break;
        default:
            output += input[k + 1];
            break;
        }
        
            
       
        }

        input = input.substr(k + 2, input.length() - 2);
    }
    output += input;
    
   
   char *out = (char*)output.c_str();
  

   seal_yylval.symbol = stringtable.add_string(out);
   return CONST_STRING;
}
 

<STRING>[^\n"]*"\n"[^"]*\" {
   curr_lineno++;
   strcpy(yylval.error_msg,"Error: String contains \n character");
   BEGIN 0;
   return ERROR;
}

<STRING>[^"]*<EOF> {
    strcpy(yylval.error_msg, "Error: EOF in string");
    BEGIN 0;
    return ERROR;
}
 */

 /* 字符串结束 */
<STRING>\" {
  
  string input(yytext,yyleng);
  input = input.substr(1, input.length() - 2);
  if(flag&&flag2){
        strcpy(yylval.error_msg, "Error: String Contains illegal \\0 and \n Character");
        flag = false;
        flag2= false;
        BEGIN 0;
        return ERROR;
  }
  if(flag){
        strcpy(yylval.error_msg, "Error: String Contains illegal \\0 Character");
        flag = false;
        
        BEGIN 0;
        return ERROR;
  }
  if(flag2){
        strcpy(yylval.error_msg, "Error: String Contains illegal \n Character");
        flag2 = false;
        
        BEGIN 0;
        return ERROR;
  }
  string output = "";
  string::size_type k;
  BEGIN 0;
  if(yyleng>256) {
     strcpy(seal_yylval.error_msg,"Error: The length of the string is more than 256");
     return ERROR;
  }
  char str[400];
  int j=0;
  int i;
  while ((k = input.find_first_of("\\")) != string::npos) {
        output += input.substr(0, k);

        switch (input[k + 1]) {
        case 'b':
            output += "\b";
            break;
        case 't':
            output += "\t";
            break;
        case 'n':
            output += "\n";
            break;
        case 'f':
            output += "\f";
            break;
        default:
            output += input[k + 1];
            break;
        }

        input = input.substr(k + 2, input.length() - 2);
    }
    output += input;
    
   
   char *out = (char*)output.c_str();
   seal_yylval.symbol = stringtable.add_string(out);
   return CONST_STRING;
}
 /*
<STRING>[^\n"]*\" {
   string input(yytext,yyleng);
   input = input.substr(1, input.length() - 2);
   BEGIN 0;
   if(yyleng>256) {
    strcpy(seal_yylval.error_msg,"Error: The length of the string is more than 256");
    return ERROR;
   }
   char str[400];
   int j=0;
   int i;
   for(i=0;i<yyleng-1;i++){
     
     
     str[j]=input[i];
     
     j++;
   }
   char *out = new char[j];
   strncpy(out,str,j);
   seal_yylval.symbol = stringtable.add_string(out);
    
   return CONST_STRING;
}
 */





 /* 整数常量 */
[0-9]+ {
   seal_yylval.symbol = inttable.add_string(yytext);
   return CONST_INT;
}

 /* 十六进制常量 */
0[xX][0-9a-fA-F]* {
   char text[1000];
   char* s;
   int ans = strtol(yytext, &s, 16);
   snprintf(text, sizeof(text), "%d", ans);
   seal_yylval.symbol = inttable.add_string(text);
   return CONST_INT;
}

  /* 浮点数常量 */
[0-9]+"."[0-9]+ {
    seal_yylval.symbol = floattable.add_string(yytext);
    return CONST_FLOAT;
}


 /* 关键词 */
"if" {return IF;}
"else" {return ELSE;}
"while" {return WHILE;}
"for" {return FOR;}
"break" {return BREAK;}
"continue" {return CONTINUE;}
"func" {return FUNC;}
"return" {return RETURN;}
"var" { return VAR; }
"struct" {return STRUCT;}

 /*布尔*/
t(?i:rue) {
    seal_yylval.boolean = 1;
    return CONST_BOOL;
}

f(?i:alse) {
    seal_yylval.boolean = 0;
    return CONST_BOOL;
}

 /*变量*/
[a-z][A-Za-z0-9_]* {
    seal_yylval.symbol = idtable.add_string(yytext);
    return OBJECTID;
}


 /*类型*/
"Int"|"Float"|"String"|"Bool"|"Void" {
   seal_yylval.symbol = idtable.add_string(yytext);
   return TYPEID;
}

 /* 运算符 */
"&&" {return AND;}
"||" {return OR;}
"==" {return EQUAL;}
"{" {return int('{');}
"}" {return int('}');}
"(" {return int('(');}
")" {return int(')');}
"+" { return int('+'); }
"-" {return int('-');}
"*" {return int('*');}
"/" {return int('/');}
"%" {return int('%');}
"&" {return int('&');}
"|" {return int('|');}
"^" {return int('^');}
"~" {return int('~');}
"=" {return int('=');}
"!=" {return NE;}
">=" {return GE;}
"<=" {return LE;}
">" {return int('>');} 
"<" {return int('<');} 
"!" {return int('!');} 
";" {return int(';');} 
"," {return int(',');} 

 /*空格*/
[ \f\r\t\v]+ { }

 /* 错误 */
[A-Z0-9_][a-zA-Z0-9_]* {

   strcpy(yylval.error_msg,"Error: illegal TYPEID");
   return ERROR;
}




 /*换行*/
"\n" {
   curr_lineno++;
}






.	{
	strcpy(seal_yylval.error_msg, yytext); 
	return (ERROR); 
}

%%
int getIndexOfSigns(char ch){    
   if(ch >= '0' && ch <= '9'){        
     return ch - '0';    
     }   
   if(ch >= 'A' && ch <='F'){   
      return ch - 'A' + 10;    
    }    
   if(ch >= 'a' && ch <= 'f'){        
     return ch - 'a' + 10;    
    }    
    return -1;
}
int hexToDec(char *source){    
  int  sum = 0;    
  int t = 1;    
  int i, len;     
  len = strlen(source);    
  for(i=len-1; i>=0; i--){        
    sum += t * getIndexOfSigns(*(source + i));        
    t *= 16;    }        
    return sum;
}
