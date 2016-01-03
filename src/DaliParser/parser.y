%{
#include "DaliParser/sql_parse.h"
extern int yylex();
extern int yyerror(char *s);
extern char *yytext, *lexString;
extern int line_num, lexInt;
extern float lexFloat;
extern List<SQLStmt *> stmtList;

//Table_descriptor *curr_table = 0;
//MultiTableSchema *curr_tgroup = 0;
//DaliTableSchema *curr_schema = 0;
//Index_descriptor *curr_index = 0;
//DaliAttrList *curr_attr_list = 0;
char *curr_field = 0;
int in_table_group =  0;
int constraint_count = 0;

/* int setup_new_table(char *name_a); */
void setup_singleton_table_group();
//int setup_new_index(char *name, int is_unique, DaliIndexType type);
/* void setup_new_table_group(char *name); */

typedef union {
     int intval;
     float floatval;
     Expr *exprval;
     LogicalExpr::LogicalOp logicalop;
     AggExpr::AggFunc aggfunc;
     char * charval;
     D_TruthVal truthval;
     Tuple * tupleval;
	 SQLStmt *sqlStmt;
     RelExpr *relexpr;
//     NonRelExpr *nonRelexpr;
     SortExpr *sortexpr;
//     UpdateExpr updateExprVal;
//     UpdateExprList *updateExprList;
     ProjectElem projectElemVal;
     ProjectElemList *projectElemList;
     List<char *> *stringList;
//   DaliIndexType indexType;
} YYSTYPED;
#define YYSTYPE YYSTYPED
%}

/* SQL keywords. What follows are the declarations alone. Precedence
   and associativity information follow later. */

%token CREATE TABLE VIEW AS 
%token SELECT FROM WHERE P_AND P_OR INTO INSERT VALUES IS
%token P_NOT UPDATE SET DELETE IN MAX DISTINCT AGGREGATE COUNT
%token ORDER BY DESC ASC ALL ON
%token GROUP DROP DATABASE INDEX TABLESPACE
%token SCHEMA SIZE UNIQUE HASH TTREE

%token 	';'	','	'.'	'('  ')'
%token P_EQ  P_NE  P_LE  P_LT  P_GE  P_GT 
%token  PLUS MINUS 
%token STAR SLASH
%token  MIN SUM AVG 
%token  TRUESYM FALSESYM UNKNOWNSYM

%token  INT_SYM CHAR_SYM  VARCHAR_SYM DOUBLE_SYM FLOAT_SYM

/* Other tokens */

%token STRING_VAL IDENT INT_VAL FLOAT_VAL 

%type <exprval> attr_spec 
%type <projectElemVal> select_elem
/* %type <updateExprVal> attr_val */
%type <exprval> expr expr_primary value aggexpr rowcons
%type <exprval> simplecond cond_expr where_clause
%type <logicalop> compop optnot
%type <aggfunc> aggfunction
%type <charval> name opt_name opt_ident
%type <truthval> truthval
%type <tupleval> expr_list groupby_clause 
%type <projectElemList> select_list select_elem_list
/* %type <updateExprList> attrValList */
/* %type <tupleval> attr_spec_list value_list opt_attr_spec_list */
%type <tupleval> attr_spec_list value_list
%type <stringList> attr_name_list opt_attr_name_list 
%type <intval>  sort_dir opt_unique opt_size
%type <sortexpr> order_clause
%type <intval> int_val select_modifier
%type <floatval> float_val
/* %type <indexType> opt_index_type */

/* %type <sqlStmt> sql_stmt dml_stmt */
/* %type <sqlStmt> sql_stmt */
%type <relexpr> sql_stmt
/* %type <nonRelexpr> ddl_stmt create_stmt drop_stmt */
/* %type <nonRelexpr> insert_exp update_exp delete_exp */
/* %type <nonRelexpr> create_database create_table create_index create_schema */
/* %type <nonRelexpr> create_tablespace  */
/* %type <relexpr> select_exp tableExpr table_ref table_ref_list */
%type <relexpr> select_exp table_ref table_ref_list


/* Precedence and Associativity of operators */

%left IS  
%left P_OR 
%left P_AND
%right P_NOT
%nonassoc P_EQ  P_NE  P_LE  P_LT  P_GE  P_GT 
%left PLUS MINUS
%left STAR SLASH

%%

/*-------------------------------------------------------*/

/* Grammar */

stmt_list	: stmt_list ';' sql_stmt 
			{
				stmtList.put($3);
				stmtCount++;
			}
			| sql_stmt
			{
				stmtList.put($1);
				stmtCount++;
			}
			| stmt_list ';'
			;
sql_stmt	: select_exp 
			| error 	{ if (yychar) return (0); else exit (0); }
			;
opt_size 	: SIZE int_val {$$ = $2;}
			| {$$ = 0;}
			;

opt_ident	: name {$$ = $1; }
			| /* null */  {$$ = NULL; }
			;
/*
ct_clause_list  : ct_clause
		| ct_clause_list ',' ct_clause
		;

ct_clause	: ct_field
		| table_constraint  ARU, commented
		;

ct_field	: name { curr_field = $1; } attrtype  
		  opt_fld_default opt_fld_constraint  ARU, commented
		;

ct_phys_attribute_list
		: IN name { }
		|  		   { }
		;

attrtype	: INT_SYM 	   
			{
			}
			| DOUBLE_SYM
			{
				curr_schema->addAttr(curr_field, sQLToDaliType(D_Double));
			}
			| FLOAT_SYM
			{
				curr_schema->addAttr(curr_field, sQLToDaliType(D_Float));
			}
			| CHAR_SYM '(' int_val ')'
			{
				curr_schema->addAttr(curr_field, sQLToDaliType(D_String), $3);
			}
			;

col_list	: name  { curr_attr_list->addAttr($1); }
			| col_list ',' name { curr_attr_list->addAttr($3); }
			;
*/
opt_attr_name_list  
		: '(' attr_name_list ')'
		{
		    $$ = $2;
		}
		| 
		{ 
		    $$ = NULL;
		}
		;
/*
opt_attr_spec_list  
		: '(' attr_spec_list ')'
		{
		    $$ = $2;
		}
		| 
		{ 
		    $$ = NULL;
		}
		;
*/
attr_name_list	: attr_name_list ',' name
		{
		   $1->put($3);
		   $$ = $1;
		}
 		| name
		{
		   $$ = new List<char *>;
		   $$->put($1);
		}
		;

opt_unique	: UNIQUE {$$ = 1;}
		|  {$$ = 0;}
		;
/*
opt_index_type	: HASH  { $$ = DALI_HASH; }
		| TTREE { $$ = DALI_TTREE; }
		|	{ $$ = DALI_TTREE; }
		;
*/
select_exp: SELECT  select_modifier select_list 
		FROM table_ref_list where_clause groupby_clause order_clause 
		    { 
			/* FIX:  add HAVING clause ! */
			RelExpr *t = SelectExpr::create($6,$5);
			// if ($7) 
			if ($2 == 2 || $7) 
			{
				t = GroupAggExpr::create($7, $3, t);
				/* FIX:  handle aggregation attrs properly */
				/* ABOVE code repeats gb attrs in agg list */
			}
			else 
				t = ProjectExpr::create($3,t);
			if ($2)
				t = DupelimExpr::create(t);
			if ($8) 
			{
				/* Orderby clause:
  				   sort expr. template already created. */
				 $8->child = t;
				 t = $8;
			}
			$$ = t;

			if ( $6)
		    }
		    ;

select_modifier : DISTINCT 					
		{
		    $$ = 1;
		}
		| ALL 					
		{   
		    $$ = 0;
		}
		| AGGREGATE
		{   
		    $$ = 2;
		}
		|
		{   
		    $$ = 0;
		}
;

select_list     : select_elem_list {$$ = $1; }
		| STAR
		{
			SQLErrE("Yet to implement ``select *''", -1)
		    $$ = new ProjectElemList;
		    ProjectElem tmp;
		    tmp.newName = NULL;
		    tmp.projectionExpr = DaliStar::create();
		    $$->put(tmp);
		}
		;

select_elem_list : select_elem_list ',' select_elem
		 {
		    $1->put($3);
		    $$ = $1;
		 }
		 | select_elem
		 {
		    $$ = new ProjectElemList;
		    $$->put($1);
		 }
		 ;

select_elem     :  expr opt_name
		{
		    $$.newName = $2;
		    $$.projectionExpr = $1;
		}
		|  aggexpr  opt_name
		{
		    $$.newName = $2;
		    $$.projectionExpr = $1;
		}
		/* FIX:  Allow qualifier.* as in SQL-92 */ 
		;

opt_name	:  AS  name
		{
		 	$$ = $2;
		}
		|
		{
			$$ = NULL;
		}
		;

where_clause    : WHERE cond_expr 		
		{
		    $$ = $2;
		}
		| /* null */
		{
		    $$ = NULL;
		}

groupby_clause  : GROUP BY attr_spec_list
		{
		    $$ = $3;
		}
		|
		{
		    $$ = NULL;
		}
;

order_clause    : ORDER BY attr_spec_list sort_dir 
		{
		    $$ = SortExpr::create($3, $4, NULL);
			/* Input of sort will be filled in later */
		}
		| /* null */	
		{
		    $$ = NULL;
		}

sort_dir 	: DESC 			{ $$ = -1; }
		| ASC			{ $$ = 1;}
		| 			{ $$ = 1;} /* null */
;
/*
delete_exp	: DELETE FROM name
		{
		    $$ = DeleteTable::create($3);
		}
		| DELETE FROM name  WHERE cond_expr  
		{
		    $$ = DeleteTable::create($3, $5);
		}
;

update_exp	: UPDATE  name  SET attrValList  WHERE cond_expr
		{
		    $$ = UpdateTable::create($2, $4, $6);
		}
		| UPDATE  name  SET attrValList 
		{
		    $$ = UpdateTable::create($2, $4, NULL);
		}
;
*/
value_list      : value_list ',' value	
		{
		    $$ = Tuple::appendExpr($1, $3);
		}
		| value 				
		{
		    $$ = Tuple::create($1);
		}
;
/*
attrValList   : attrValList ',' attr_val
		{
		    $1->put($3);
		    $$ = $1;
		}
		| attr_val
		{
		    $$ = new UpdateExprList;
		    $$->put($1);
		}
;
attr_val	: attr_spec P_EQ expr 
		{
		    $$.attribute = $1;
		    $$.rhs = $3;
		}
;
*/
attr_spec_list  : attr_spec_list ',' attr_spec
		{
		    $$ = Tuple::appendExpr($1, $3);
		}
		| attr_spec
		{
		    $$ = Tuple::create($1);
		}
;
	
attr_spec	: name '.' name 			
		{
		   $$ = RelArgRef::create($1, $3);
		}
		| name 			
		{
		   $$ = RelArgRef::create("", $1);
		}
;

table_ref_list  : table_ref_list ',' table_ref 	
		{
		    $$ = JoinExpr::create(NULL, $1, $3);
		    // kvr- FIX: Do we really want to do it this way?
		    /* SQL-92: 
			joins (OUTER, CROSS, INNER, ON, NATURAL, USING ..)  
		    */
                }
		| table_ref 		
		{
		    $$ = $1;
                }
		;

table_ref	: name
		  { $$ = RelScanExpr::create($1); }
		| name AS name opt_attr_name_list
		{
                    RelScanExpr *t = RelScanExpr::create($1);
		    $$ =  RenameExpr::create($3, $4, t);
		}
		| name name opt_attr_name_list
		{
                    RelScanExpr *t = RelScanExpr::create($1);
		    $$ =  RenameExpr::create($2, $3, t);
		}
		| '(' select_exp ')' AS name  opt_attr_name_list
		  { $$ = RenameExpr::create($5, $6, $2);}
		| '(' select_exp ')' name opt_attr_name_list
		  { $$ = RenameExpr::create($4, $5, $2);}
;

/*******************  Expressions *************************/

    /*** Conditional Expressions ***/

cond_expr	: cond_expr P_OR cond_expr /* Rule 71 */
		{
		    $$ = LogicalExpr::create(LogicalExpr::D_OR, $1, $3);
		}
		| cond_expr P_AND cond_expr
		{
		    $$ = LogicalExpr::create(LogicalExpr::D_AND, $1, $3);
		}
		| cond_expr IS optnot truthval
		{
		    DaliTruthVal *t = DaliTruthVal::create($4);
		    if($3 == LogicalExpr::D_NOT)
			$$ = LogicalExpr::create(LogicalExpr::D_IS_NOT, $1, t);
		    else $$ = LogicalExpr::create(LogicalExpr::D_IS, $1, t);
		}
   /* FIX: Add IN clause, exists clause, and > ALL and friends also */
		| P_NOT cond_expr  /* Rule 75 */
			/* If a P_OR, or P_AND or IS follows, reduce will
			   take precedence over shift. */
		{
			$$ = LogicalExpr::create(LogicalExpr::D_NOT, $2);	
		}
		| '(' cond_expr ')'
		{
		    $$ = $2;
		}
		| simplecond 
		{
		    $$ = $1;
		}
;


optnot          : P_NOT 
		{
		    $$ = LogicalExpr::D_NOT;
		}
		|  
		{
		    $$ = LogicalExpr::D_NO_LOP;
		}
;

truthval	: TRUESYM        { $$ = D_TRUE; }
		| FALSESYM	 { $$ = D_FALSE; }
		| UNKNOWNSYM     { $$ = D_UNKNOWN; }
;


simplecond	: rowcons  compop  rowcons 
		{
		    $$ = LogicalExpr::create($2, $1, $3);

			
		 /* 
		   Other things to add: between, like, in, match, all or any,
		   exists, unique, overlaps, test-for-null
		 */
		}
		
		|	rowcons  IN  '('  select_exp ')' 
	  {

		// By default " attrib = all ( select_expression ) " is 
		// created  
		

		$$=(LogicalExpr *) new InExpr($1,((GeneralProjectExpr *)$4));
	  } 
;

compop		: P_EQ	{ $$ = LogicalExpr::D_EQ; }
		| P_NE    { $$ = LogicalExpr::D_NE; }
		| P_LT    { $$ = LogicalExpr::D_LT; }
		| P_LE    { $$ = LogicalExpr::D_LEQ;}
		| P_GT    { $$ = LogicalExpr::D_GT; }
		| P_GE    { $$ = LogicalExpr::D_GEQ;}
;

    /*** Other (string, arith, ..) Expressions ***/
rowcons:   	  expr 
		{
		    $$ = $1;
		/* FIX:  Later add row-wise comparison
		       | '(' expr_list ')'
                */
		}
;

expr_list	: expr
		{
		    $$ = Tuple::create($1);
		}
		| expr_list ',' expr
		{
		    $$ = Tuple::appendExpr($1,$3);
		}
;

expr		: expr_primary
		{
		    $$ = $1;
		}
		| expr PLUS expr /* rule 94 */
		{
		    $$ = ArithExpr::create(ArithExpr::D_PLUS, $1, $3);
		}
		| expr MINUS expr
		{
			/* FIX:  Also add unary minus */
		    $$ = ArithExpr::create(ArithExpr::D_MINUS, $1, $3);
		}
		| expr STAR expr
		{
		    $$ = ArithExpr::create(ArithExpr::D_MULT, $1, $3);
		}
		| expr SLASH expr
		{
		    $$ = ArithExpr::create(ArithExpr::D_DIV, $1, $3);
		}
;

expr_primary	: value 
		{
		    $$ = $1;
		}
		| attr_spec
		{
		    $$ = $1;
		}
		| '(' expr ')'
		{
		    $$ = $2;
		}
	/* fill in general expressions :
		Scalar subquery, 
		set function spec (ie. expressions involving 
			aggregate functions which are not currently supported)
		case expr, 
		cast
			
	*/
;

name		: IDENT
		{
		    $$ = lexString;
		}
;

set 		: '(' value_list ')'	
;

value : STRING_VAL {
			$$ = DaliString::create(lexString, strlen(lexString)+1);
		        free(lexString);
		 }
		| int_val 	{
			$$ = DaliInt::create($1);
		 }
        | float_val {
			$$ = DaliFloat::create($1);
		 }
		| truthval {
			$$ = DaliTruthVal::create($1);
		}

		   /** FIX:  add these rules or delete if not required.
			| COLON_FIELD
			| SYMB_CONST
                    **/ 
		/* FIX:  Add general case:
		        Time type
			parameter spec (:parameter, :embedded-variable-name)
			dynamic parameter spec (?)
			  INDICATOR variables for parameters (NULL values)
			USER, VALUE . 
	        */
;

int_val		: INT_VAL     { $$ = lexInt; }
;

float_val	: FLOAT_VAL  { $$ = lexFloat; }
;

aggexpr
		: COUNT '(' attr_spec ')'          
		{
			$$ = AggExpr::create(AggExpr::D_COUNT, $3);
		}
		| COUNT '(' DISTINCT attr_spec ')' 
		{
			$$ = AggExpr::create(AggExpr::D_COUNT_DISTINCT, $4);
		}
		| COUNT '(' '*' ')'	           
		{
			$$ = AggExpr::create(AggExpr::D_COUNT_STAR);
		}
		| aggfunction '(' expr ')'	           
		{
			$$ = AggExpr::create($1, $3);

	/* FIX:  allow DISTINCT here */
		}
;

aggfunction     : MAX    { $$ = AggExpr::D_MAX; }
		| MIN    { $$ = AggExpr::D_MIN; }
		| SUM    { $$ = AggExpr::D_SUM; }
		| AVG    { $$ = AggExpr::D_AVG; }
		;
%%

int yyerror(char *s) {
	return printf( "** syntax error at line number %d\n%s",
						line_num, s?s:"");
}
/*
int setup_new_table(char *name)
{
	if(!in_table_group)
		setup_singleton_table_group();

	curr_table = new Table_descriptor(name); 
	curr_schema = &curr_table->schema;
	constraint_count = 0;
	return 1;
}

void
setup_singleton_table_group()
{
	curr_tgroup = new MultiTableSchema;
}

int setup_new_index(char *name, int is_unique, DaliIndexType type)
{
	if(!in_table_group)
		setup_singleton_table_group();

	curr_index = new Index_descriptor; 
	curr_index->name = name;
	if(is_unique)
	{
		switch(type)
		{
		  case DALI_HASH:	curr_index->type = DALI_UNIQUE_HASH; break;
		  case DALI_TTREE:	curr_index->type = DALI_UNIQUE_TTREE; break;
		  default:
			SQLErrAR("Cannot create unique index of type %d\n", type, -1)
		}
	}
	else
	{
		switch(type)
		{
		  case DALI_HASH: case DALI_TTREE:
			curr_index->type = type; break;
		  default:
			SQLErrAR("Cannot create index of type %d\n", type, -1)
		}
	}
	curr_attr_list = &curr_index->attrList;
	return 1;
}

void
setup_new_table_group(char *name)
{
	if(name)
	     SQLErr("Warning: names not currently supported for schemas")
	in_table_group = 1;
	curr_tgroup = new MultiTableSchema;
}
*/
