%{
#include <ctype.h>
#include <string.h>
#include "parser.h"
extern char *lexString;
extern int line_num, lexInt;
extern float lexFloat;
%}

letter				[a-zA-Z_]
digit				[0-9]
letter_or_digit			[a-zA-Z_0-9]
variable_char 			[\]\[\.\>a-zA-Z_0-9\-]
string_char			[\]\[\\\+\'=\)\(\*&\^\%\$#@!:;,\.\<\>\?\/a-zA-Z_0-9 \t\n\-]
white_space			[ \t\n]
blank				[ \t]

%%

table|TABLE             { return (TABLE); 
		/* FIX:  MAKE ALL KEYWORDS CASE INSENSITIVE */
		}
create|CREATE		{ return (CREATE); }
view|VIEW		{ return (VIEW); }
as|AS			{ return (AS); }
index|INDEX		{ return INDEX;}
unique|UNIQUE	{return UNIQUE;}
hash|HASH	{return HASH;}
ttree|TTREE	{return TTREE;}

select|SELECT		{ return (SELECT); }
into|INTO		{ return (INTO); }
from|FROM		{ return (FROM); } 
where|WHERE		{ return (WHERE); }
insert|INSERT		{ return (INSERT); }
values|VALUES		{ return (VALUES); }
update|UPDATE		{ return (UPDATE); }
delete|DELETE		{ return (DELETE); }
DROP|drop			{ return (DROP);}
DATABASE|database	{ return (DATABASE);}
TABLESPACE|tablespace	{ return (TABLESPACE);}
SCHEMA|schema		{ return (SCHEMA);}

ON|on			{ return (ON);}
SIZE|size		{ return (SIZE);}
group|GROUP		{ return (GROUP); }
order|ORDER		{ return (ORDER); }
by|BY			{ return (BY); }
ASC|asc			{ return (ASC);}
DESC|desc		{ return (DESC);}
all|ALL			{ return (ALL); }
aggregate|AGGREGATE	{ return (AGGREGATE); }

char|CHAR		{ return (CHAR_SYM); }
varchar|VARCHAR		{ return (VARCHAR_SYM); }
int|INT			{ return (INT_SYM); }
integer|INTEGER		{ return (INT_SYM); }
float|FLOAT		{ return (FLOAT_SYM); }
double|DOUBLE		{ return (DOUBLE_SYM); }

count|COUNT  		{ return (COUNT); }
max|MAX			{ return (MAX); }
min|MIN  		{ return (MIN); }
sum|SUM  		{ return (SUM); }
avg|AVG  		{ return (AVG); }
distinct|DISTINCT	{ return (DISTINCT); }

is|IS			{ return (IS); }
not|NOT			{ return (P_NOT); }
true|TRUE		{ return TRUESYM; }
false|FALSE		{ return FALSESYM; }
unknown|UNKNOWN		{ return UNKNOWNSYM; }


in|IN			{ return (IN); }
set|SET			{ return (SET); }

and|AND			{ return (P_AND); }
or|OR			{ return (P_OR); }
"="		{ return (P_EQ); }
"<>"		{ return (P_NE); }
"<"		{ return (P_LT); }
">"		{ return (P_GT); }
"<="		{ return (P_LE); }
">="		{ return (P_GE); }
"*" 		{ return (STAR); }
"/" 		{ return (SLASH); }
"+" 		{ return (PLUS); }
"-" 		{ return (MINUS); }


\"{string_char}*\" {
	char *p1 = yytext;
	lexString = new char [(unsigned) strlen (p1)];
	strcpy (lexString, p1+1);
	lexString[strlen(lexString) - 1] = '\0';
	return (STRING_VAL);
}

{letter}{letter_or_digit}* {
	char *p1 = yytext;
	lexString = new char [(unsigned) strlen (p1) + 1];
	strcpy (lexString, p1);
	p1 = lexString;
	while (( *p1 = tolower (*p1)) != '\0') p1++;
	return (IDENT);
}

{digit}+ {
	sscanf(yytext, "%d", &lexInt);
	return INT_VAL;
	}

{digit}+\.{digit}+ {
		sscanf(yytext,"%f", &lexFloat);
		return FLOAT_VAL;
	}

{blank}+	{ }
\n		{ line_num ++ ;}

.		return yytext[0];

%{
/*	Watch this C comment folks */
%}

"/*""/"*([^*/]|[^*]"/"|"*"[^/])*"*"*"*/" {
			printf ("%s", yytext);
	}

%%

int yywrap() {
        /* return 0 to indicate further files to be processed;
           standard input will have to be reopened before returning
         */
    return 1;
}

/*************
      For future support of embedded SQL

        :{letter}{variable_char}*  {
		p1 = yytext;
		p1++;  // skip the colon 
		lexString = (char *) malloc ((unsigned) strlen (p1) + 1);
		strcpy (lexString, p1);
		return (COLON_FIELD);
	}
**************/

