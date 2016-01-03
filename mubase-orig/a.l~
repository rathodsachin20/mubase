%{
#include <stdlib.h>
#include<string.h>
#include "y.tab.h"
void yyerror(char*);
%}

%%

"select * from" 				            { return SELECT;}

"where"                                 {return WHERE;}

[a-zA-Z0-9 ._]+"("[a-zA-Z0-9 ._]+")"    { return RELNAME;}

"{"[a-zA-Z0-9 ._]+"="[ a-z.A-Z0-9_]+"}"	{printf("returns equal predicate\n");	
				                         yylval.str=yytext;
					                     printf("%s\n",yylval.str);
					                     return EQ;			
				                        }
				                        
"{"[a-zA-Z0-9 ._]+"<"[ a-z.A-Z0-9_]+"}"	 {	
					                     yylval.str=strdup(yytext);
				                         return LT;			
				                         }
"{"[a-zA-Z0-9 ._]+">"[ a-z.A-Z0-9_]+"}"	 {	
					                     yylval.str=yytext;
					                     return GT;
					                     }			
"{"[a-zA-Z0-9 ._]+"<="[ a-z.A-Z0-9_]+"}" {	
					                      yylval.str=yytext;
					                      return LE;			
				                          }
"{"[a-zA-Z0-9 ._]+">="[ a-z.A-Z0-9_]+"}" {	
					                      yylval.str=yytext;
					                      return GE;			
					                     } 
"{"[a-zA-Z0-9 ._]+"!="[ a-z.A-Z0-9_]+"}" {	
					                      yylval.str=yytext;
					                      return NE;	
					                     }
						
[~|^]                                    {return *yytext;}
                              

[ \t]							 	
		 	                             
.	                                      {
                                          printf("invalid input hi \"%s\"\n",yytext);
	                                      yyerror("yerror");
	                                      }
%%

int yywrap(void) {
return 0;
} 

