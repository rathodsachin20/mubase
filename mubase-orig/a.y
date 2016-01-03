%{
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include "iter.h"	
#include <string.h>

using namespace std;
	void yyerror(char *);
	int yylex(void);
	char* relation_name[25];
%}

 
%union { 
	char* str;
	class Predicate* pre;
	class Record* r;
	};
	
%token <str> EQ;
%token <str> GT;
%token <str> LT;
%token <str> GE;
%token <str> LE;
%token <str> NE;
%token <str> RELNAME;	
%token SELECT;
%token WHERE;

%type<pre> select_pred;
%type<r> E;
%%
	
	       

E:SELECT  RELNAME WHERE select_pred                          				                       {
	                     										                            printf("select\n");
	                     										                            string s;
	                     										                            s = $2;
	                     										                            int f = s.find_first_of("(");
	                     										                            string relation = s.substr(0,f);
	                     										                            const char *rel = &relation[0];
	                     										                            int length = s.length();
	                     										                            string dbname = s.substr(f+1,length-f-2);
	                     										                            const char *dbn = &dbname[0];
	                     										                            RIterator* pre;
	                     										                            cout<<"nanjh"<<endl;
	                     										                            pre = new BaseRelIterator(dbn,rel);
	                     										                            pre->open();
	                     										                            
	                     										                            Record* rec;
                                                                                            rec = pre->next();
                                                                                            Predicate* predicate;
                                                                                            predicate = $4;
                                                                                            while( rec != NULL ) {
                                                                                            if( predicate->evaluate(rec) )
                                                                                            rec->print();
                                                                                            $$=rec;
                                                                                            rec = pre->next();
                                                                                            }
                                                                                            pre->close();
	                                                                            			}



		;
	
select_pred :  select_pred'^'select_pred                                                   {
	                                                                                         printf("and in select\n");
	                                                                                         $$= new AndPredicate($1,$3);
	                                                                                        }
	                                                                                         	
	
		    | select_pred'|'select_pred                                                    {
	                                                                                         printf("or in select\n");
	                                                                                         $$= new OrPredicate($1,$3);
	                                                                                        }
	                                                                                         	 								
            | EQ				                                                            {
                                                                                            string s;
                     					                                                    s=$1;
							                                                                int f = s.find_first_of("=");
							                                                                string colName = s.substr(0, f);
							                                                                string s1 = &s[f+1];
							

							                                                                colName = colName.substr(1,colName.length()-1);
														                                    cout << "colname : " <<colName<<endl;               
							                                                                s1 = s1.substr(0,s1.length()-1);
							                                                                cout << "value : " << s1<<endl;
							                                                                Predicate* pr;
							                                                                pr = new EqualPred(colName,s1);
							                                                                
							                                                                $$ = pr;
							                                                                cout<<" out"<<endl;
							                                                                }
								
            | NE				                                                            {
                                                                                            string s;
                                                                                     		s=$1;
							                                                                int f = s.find_first_of("!");
							                                                                string colName = s.substr(0, f);
							                                                                string s1 = &s[f+2];
							

							                                                                colName = colName.substr(1,colName.length()-1);
											                                                cout << "colname : " << colName<<endl;
							                                                                s1 = s1.substr(0,s1.length()-1);
							                                                                cout << "value : " << s1<<endl;
							                                                                $$ = new NotEqualPred(colName,s1);
							                                                                }
							                                                                
			| GT				                                                            {string s;
                                                                                     		s=$1;
							                                                                int f = s.find_first_of(">");
							                                                                string colName = s.substr(0, f);
							                                                                string s1 = &s[f+1];							
							                                                                colName = colName.substr(1,colName.length()-1);
							                                                                cout << "colname : " << colName<<endl;
							                                                                s1 = s1.substr(0,s1.length()-1);
							                                                                cout << "value : " << s1<<endl;
							                                                                $$ = new GreaterPred(colName,s1);
							                                                                cout<<"hello";
							                                                                }
							                                                                
			| LT				                                                           {string s;
                                                                                     		s=$1;
							                                                                int f = s.find_first_of("<");
							                                                                string colName = s.substr(0, f);
							                                                                string s1 = &s[f+1];							
							                                                                colName = colName.substr(1,colName.length()-1);
							                                                                cout << "colname : " << colName<<endl;
							                                                                s1 = s1.substr(0,s1.length()-1);
							                                                                cout << "value : " << s1<<endl;
							                                                                $$ = new LesserPred(colName,s1);
							                                                                cout<<"hello";
							                                                                }
							                                                                
           | GE				                                                               {string s;
                                                                         					s=$1;
						                                                                    int f = s.find_first_of(">");
						                                                                    string colName = s.substr(0, f);
						                                                                    string s1 = &s[f+2];						
						                                                                    colName = colName.substr(1,colName.length()-1);
						                                                                    cout << "colname : " << colName<<endl;
						                                                                    s1 = s1.substr(0,s1.length()-1);
						                                                                    cout << "value : " << s1<<endl;
						                                                                    $$ = new GreaterEqPred(colName,s1);
						                                                                    cout<<"hello";
						                                                                    }
						                                                                    
           | LE				                                                               {string s;
                                                                         					s=$1;
						                                                                    int f = s.find_first_of("<");
						                                                                    string colName = s.substr(0, f);
						                                                                    string s1 = &s[f+2];						
						                                                                    colName = colName.substr(1,colName.length()-1);
						                                                                    cout << "colname : " << colName<<endl;
						                                                                    s1 = s1.substr(0,s1.length()-1);
						                                                                    cout << "value : " << s1<<endl;
						                                                                    $$ = new LesserEqPred(colName,s1);
						                                                                    cout<<"hello";
						                                                                    }
			;	   	
                     
%%
void yyerror(char* s)
{fprintf(stderr,"%s\n",s);}

