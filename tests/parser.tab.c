/* A Bison parser, made by GNU Bison 1.875.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Written by Richard Stallman by simplifying the original so called
   ``semantic'' parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     CREATE = 258,
     TABLE = 259,
     VIEW = 260,
     AS = 261,
     SELECT = 262,
     FROM = 263,
     WHERE = 264,
     P_AND = 265,
     P_OR = 266,
     INTO = 267,
     INSERT = 268,
     VALUES = 269,
     IS = 270,
     P_NOT = 271,
     UPDATE = 272,
     SET = 273,
     DELETE = 274,
     IN = 275,
     MAX = 276,
     DISTINCT = 277,
     AGGREGATE = 278,
     COUNT = 279,
     ORDER = 280,
     BY = 281,
     DESC = 282,
     ASC = 283,
     ALL = 284,
     ON = 285,
     GROUP = 286,
     DROP = 287,
     DATABASE = 288,
     INDEX = 289,
     TABLESPACE = 290,
     SCHEMA = 291,
     SIZE = 292,
     UNIQUE = 293,
     HASH = 294,
     TTREE = 295,
     P_EQ = 296,
     P_NE = 297,
     P_LE = 298,
     P_LT = 299,
     P_GE = 300,
     P_GT = 301,
     PLUS = 302,
     MINUS = 303,
     STAR = 304,
     SLASH = 305,
     MIN = 306,
     SUM = 307,
     AVG = 308,
     TRUESYM = 309,
     FALSESYM = 310,
     UNKNOWNSYM = 311,
     INT_SYM = 312,
     CHAR_SYM = 313,
     VARCHAR_SYM = 314,
     DOUBLE_SYM = 315,
     FLOAT_SYM = 316,
     STRING_VAL = 317,
     IDENT = 318,
     INT_VAL = 319,
     FLOAT_VAL = 320
   };
#endif
#define CREATE 258
#define TABLE 259
#define VIEW 260
#define AS 261
#define SELECT 262
#define FROM 263
#define WHERE 264
#define P_AND 265
#define P_OR 266
#define INTO 267
#define INSERT 268
#define VALUES 269
#define IS 270
#define P_NOT 271
#define UPDATE 272
#define SET 273
#define DELETE 274
#define IN 275
#define MAX 276
#define DISTINCT 277
#define AGGREGATE 278
#define COUNT 279
#define ORDER 280
#define BY 281
#define DESC 282
#define ASC 283
#define ALL 284
#define ON 285
#define GROUP 286
#define DROP 287
#define DATABASE 288
#define INDEX 289
#define TABLESPACE 290
#define SCHEMA 291
#define SIZE 292
#define UNIQUE 293
#define HASH 294
#define TTREE 295
#define P_EQ 296
#define P_NE 297
#define P_LE 298
#define P_LT 299
#define P_GE 300
#define P_GT 301
#define PLUS 302
#define MINUS 303
#define STAR 304
#define SLASH 305
#define MIN 306
#define SUM 307
#define AVG 308
#define TRUESYM 309
#define FALSESYM 310
#define UNKNOWNSYM 311
#define INT_SYM 312
#define CHAR_SYM 313
#define VARCHAR_SYM 314
#define DOUBLE_SYM 315
#define FLOAT_SYM 316
#define STRING_VAL 317
#define IDENT 318
#define INT_VAL 319
#define FLOAT_VAL 320




/* Copy the first part of user declarations.  */
#line 1 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"

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


/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 214 of yacc.c.  */
#line 262 "parser.tab.c"

#if ! defined (yyoverflow) || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# if YYSTACK_USE_ALLOCA
#  define YYSTACK_ALLOC alloca
# else
#  ifndef YYSTACK_USE_ALLOCA
#   if defined (alloca) || defined (_ALLOCA_H)
#    define YYSTACK_ALLOC alloca
#   else
#    ifdef __GNUC__
#     define YYSTACK_ALLOC __builtin_alloca
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning. */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
# else
#  if defined (__STDC__) || defined (__cplusplus)
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   define YYSIZE_T size_t
#  endif
#  define YYSTACK_ALLOC malloc
#  define YYSTACK_FREE free
# endif
#endif /* ! defined (yyoverflow) || YYERROR_VERBOSE */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
	 || (YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short) + sizeof (YYSTYPE))				\
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  register YYSIZE_T yyi;		\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (0)
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (0)

#endif

#if defined (__STDC__) || defined (__cplusplus)
   typedef signed char yysigned_char;
#else
   typedef short yysigned_char;
#endif

/* YYFINAL -- State number of the termination state. */
#define YYFINAL  10
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   172

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  72
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  33
/* YYNRULES -- Number of rules. */
#define YYNRULES  86
/* YYNRULES -- Number of states. */
#define YYNSTATES  140

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   320

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      44,    45,    71,     2,    42,     2,    43,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    41,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned char yyprhs[] =
{
       0,     0,     3,     7,     9,    12,    14,    16,    20,    21,
      25,    27,    36,    38,    40,    42,    43,    45,    47,    51,
      53,    56,    59,    62,    63,    66,    67,    71,    72,    77,
      78,    80,    82,    83,    87,    89,    93,    95,    99,   101,
     103,   108,   112,   119,   125,   129,   133,   138,   141,   145,
     147,   149,   150,   152,   154,   156,   160,   166,   168,   170,
     172,   174,   176,   178,   180,   182,   186,   190,   194,   198,
     200,   202,   206,   208,   210,   212,   214,   216,   218,   220,
     225,   231,   236,   241,   243,   245,   247
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const yysigned_char yyrhs[] =
{
      73,     0,    -1,    73,    41,    74,    -1,    74,    -1,    73,
      41,    -1,    77,    -1,     1,    -1,    44,    76,    45,    -1,
      -1,    76,    42,    99,    -1,    99,    -1,     7,    78,    79,
       8,    89,    83,    84,    85,    -1,    22,    -1,    29,    -1,
      23,    -1,    -1,    80,    -1,    54,    -1,    80,    42,    81,
      -1,    81,    -1,    97,    82,    -1,   103,    82,    -1,     6,
      99,    -1,    -1,     9,    91,    -1,    -1,    31,    26,    87,
      -1,    -1,    25,    26,    87,    86,    -1,    -1,    27,    -1,
      28,    -1,    -1,    87,    42,    88,    -1,    88,    -1,    99,
      43,    99,    -1,    99,    -1,    89,    42,    90,    -1,    90,
      -1,    99,    -1,    99,     6,    99,    75,    -1,    99,    99,
      75,    -1,    44,    77,    45,     6,    99,    75,    -1,    44,
      77,    45,    99,    75,    -1,    91,    11,    91,    -1,    91,
      10,    91,    -1,    91,    15,    92,    93,    -1,    16,    91,
      -1,    44,    91,    45,    -1,    94,    -1,    16,    -1,    -1,
      59,    -1,    60,    -1,    61,    -1,    96,    95,    96,    -1,
      96,    20,    44,    77,    45,    -1,    46,    -1,    47,    -1,
      49,    -1,    48,    -1,    51,    -1,    50,    -1,    97,    -1,
      98,    -1,    97,    52,    97,    -1,    97,    53,    97,    -1,
      97,    54,    97,    -1,    97,    55,    97,    -1,   100,    -1,
      88,    -1,    44,    97,    45,    -1,    68,    -1,    67,    -1,
     101,    -1,   102,    -1,    93,    -1,    69,    -1,    70,    -1,
      24,    44,    88,    45,    -1,    24,    44,    22,    88,    45,
      -1,    24,    44,    71,    45,    -1,   104,    44,    97,    45,
      -1,    21,    -1,    56,    -1,    57,    -1,    58,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   118,   118,   123,   128,   130,   131,   180,   185,   201,
     206,   222,   251,   255,   259,   264,   269,   270,   281,   286,
     293,   298,   306,   311,   316,   321,   325,   330,   335,   341,
     345,   346,   347,   398,   402,   408,   412,   418,   426,   432,
     434,   439,   444,   446,   454,   458,   462,   470,   476,   480,
     487,   492,   497,   498,   499,   503,   514,   525,   526,   527,
     528,   529,   530,   534,   553,   557,   561,   566,   570,   576,
     580,   584,   598,   607,   611,   614,   617,   634,   637,   641,
     645,   649,   653,   661,   662,   663,   664
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "CREATE", "TABLE", "VIEW", "AS", "SELECT", 
  "FROM", "WHERE", "P_AND", "P_OR", "INTO", "INSERT", "VALUES", "IS", 
  "P_NOT", "UPDATE", "SET", "DELETE", "IN", "MAX", "DISTINCT", 
  "AGGREGATE", "COUNT", "ORDER", "BY", "DESC", "ASC", "ALL", "ON", 
  "GROUP", "DROP", "DATABASE", "INDEX", "TABLESPACE", "SCHEMA", "SIZE", 
  "UNIQUE", "HASH", "TTREE", "';'", "','", "'.'", "'('", "')'", "P_EQ", 
  "P_NE", "P_LE", "P_LT", "P_GE", "P_GT", "PLUS", "MINUS", "STAR", 
  "SLASH", "MIN", "SUM", "AVG", "TRUESYM", "FALSESYM", "UNKNOWNSYM", 
  "INT_SYM", "CHAR_SYM", "VARCHAR_SYM", "DOUBLE_SYM", "FLOAT_SYM", 
  "STRING_VAL", "IDENT", "INT_VAL", "FLOAT_VAL", "'*'", "$accept", 
  "stmt_list", "sql_stmt", "opt_attr_name_list", "attr_name_list", 
  "select_exp", "select_modifier", "select_list", "select_elem_list", 
  "select_elem", "opt_name", "where_clause", "groupby_clause", 
  "order_clause", "sort_dir", "attr_spec_list", "attr_spec", 
  "table_ref_list", "table_ref", "cond_expr", "optnot", "truthval", 
  "simplecond", "compop", "rowcons", "expr", "expr_primary", "name", 
  "value", "int_val", "float_val", "aggexpr", "aggfunction", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const unsigned short yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,    59,    44,    46,    40,    41,   296,   297,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     310,   311,   312,   313,   314,   315,   316,   317,   318,   319,
     320,    42
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    72,    73,    73,    73,    74,    74,    75,    75,    76,
      76,    77,    78,    78,    78,    78,    79,    79,    80,    80,
      81,    81,    82,    82,    83,    83,    84,    84,    85,    85,
      86,    86,    86,    87,    87,    88,    88,    89,    89,    90,
      90,    90,    90,    90,    91,    91,    91,    91,    91,    91,
      92,    92,    93,    93,    93,    94,    94,    95,    95,    95,
      95,    95,    95,    96,    97,    97,    97,    97,    97,    98,
      98,    98,    99,   100,   100,   100,   100,   101,   102,   103,
     103,   103,   103,   104,   104,   104,   104
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     3,     1,     2,     1,     1,     3,     0,     3,
       1,     8,     1,     1,     1,     0,     1,     1,     3,     1,
       2,     2,     2,     0,     2,     0,     3,     0,     4,     0,
       1,     1,     0,     3,     1,     3,     1,     3,     1,     1,
       4,     3,     6,     5,     3,     3,     4,     2,     3,     1,
       1,     0,     1,     1,     1,     3,     5,     1,     1,     1,
       1,     1,     1,     1,     1,     3,     3,     3,     3,     1,
       1,     3,     1,     1,     1,     1,     1,     1,     1,     4,
       5,     4,     4,     1,     1,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       0,     6,    15,     0,     3,     5,    12,    14,    13,     0,
       1,     0,    83,     0,     0,    17,    84,    85,    86,    52,
      53,    54,    73,    72,    77,    78,     0,    16,    19,    70,
      76,    23,    64,    36,    69,    74,    75,    23,     0,     2,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    20,
       0,    21,     0,     0,     0,     0,    71,     0,    25,    38,
      39,    18,    22,    65,    66,    67,    68,    35,     0,     0,
      81,    79,     0,     0,     0,    27,     0,     8,    82,    80,
       0,     0,     0,    24,    49,     0,    63,    37,     0,    29,
       8,     0,    41,     0,     8,    47,     0,    63,     0,     0,
      51,     0,    57,    58,    60,    59,    62,    61,     0,     0,
       0,    11,    40,     0,    10,     8,    43,    48,    45,    44,
      50,     0,     0,    55,    26,    34,     0,     0,     7,    42,
      46,     0,     0,    32,     9,    56,    33,    30,    31,    28
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,     3,     4,    92,   113,     5,     9,    26,    27,    28,
      49,    75,    89,   111,   139,   124,    29,    58,    59,    83,
     121,    30,    84,   108,    85,    86,    32,    33,    34,    35,
      36,    37,    38
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -77
static const short yypact[] =
{
      29,   -77,     2,     9,   -77,   -77,   -77,   -77,   -77,    41,
     -77,    16,   -77,   -22,    85,   -77,   -77,   -77,   -77,   -77,
     -77,   -77,   -77,   -77,   -77,   -77,    30,   -38,   -77,   -77,
     -77,     6,   -77,   -17,   -77,   -77,   -77,    45,    12,   -77,
      23,    22,   -36,    63,     5,    85,    85,    85,    85,   -77,
       5,   -77,    85,     5,    37,    44,   -77,   121,    -2,   -77,
      -4,   -77,   -77,   -12,   -12,   -77,   -77,   -77,    51,    91,
     -77,   -77,   102,    11,   -36,   106,     5,   104,   -77,   -77,
       0,    11,    11,   146,   -77,    92,    61,   -77,   123,   125,
     104,     5,   -77,     5,   104,   -77,    18,    22,    11,    11,
     135,   114,   -77,   -77,   -77,   -77,   -77,   -77,    85,     5,
     133,   -77,   -77,    48,   -77,   104,   -77,   -77,   -77,   150,
     -77,    66,   121,   -77,   120,   -77,     5,     5,   -77,   -77,
     -77,   118,     5,    -8,   -77,   -77,   -77,   -77,   -77,   -77
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
     -77,   -77,   153,   -76,   -77,   -56,   -77,   -77,   -77,   122,
     129,   -77,   -77,   -77,   -77,    42,   -40,   -77,    93,    36,
     -77,    49,   -77,   -77,    64,     1,   -77,   -39,   -77,   -77,
     -77,   -77,   -77
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -5
static const short yytable[] =
{
      55,    72,    76,    60,    43,    62,    93,    73,    57,    10,
      31,    67,    44,    69,   112,    41,    -4,     1,   116,   137,
     138,    77,    40,     2,     6,     7,    50,    81,    98,    99,
       1,     8,    23,   100,   132,    60,     2,    90,    42,   129,
      74,    94,    47,    48,    31,    53,    63,    64,    65,    66,
      11,    44,   114,    68,   115,    82,    52,    -4,    45,    46,
      47,    48,    12,   117,    23,    13,   131,    56,    23,   125,
      19,    20,    21,    23,    45,    46,    47,    48,    22,    23,
      24,    25,    70,    97,    12,    14,   125,    13,   134,    71,
     127,    23,   136,   128,    54,    15,    78,    16,    17,    18,
      19,    20,    21,    45,    46,    47,    48,    14,    22,    23,
      24,    25,   101,    45,    46,    47,    48,    95,    96,    16,
      17,    18,    19,    20,    21,    19,    20,    21,     2,    14,
      22,    23,    24,    25,   118,   119,    79,    88,   102,   103,
     104,   105,   106,   107,    19,    20,    21,    80,    91,   109,
     110,   120,    22,    23,    24,    25,    98,    99,   122,   126,
      98,   100,   132,   135,    39,    61,    51,    87,   133,     0,
     130,     0,   123
};

static const short yycheck[] =
{
      40,    57,     6,    42,    42,    44,     6,     9,    44,     0,
       9,    50,     6,    53,    90,    14,     0,     1,    94,    27,
      28,    60,    44,     7,    22,    23,    43,    16,    10,    11,
       1,    29,    68,    15,    42,    74,     7,    76,     8,   115,
      42,    80,    54,    55,    43,    22,    45,    46,    47,    48,
      41,     6,    91,    52,    93,    44,    44,    41,    52,    53,
      54,    55,    21,    45,    68,    24,   122,    45,    68,   109,
      59,    60,    61,    68,    52,    53,    54,    55,    67,    68,
      69,    70,    45,    82,    21,    44,   126,    24,   127,    45,
      42,    68,   132,    45,    71,    54,    45,    56,    57,    58,
      59,    60,    61,    52,    53,    54,    55,    44,    67,    68,
      69,    70,    20,    52,    53,    54,    55,    81,    82,    56,
      57,    58,    59,    60,    61,    59,    60,    61,     7,    44,
      67,    68,    69,    70,    98,    99,    45,    31,    46,    47,
      48,    49,    50,    51,    59,    60,    61,    45,    44,    26,
      25,    16,    67,    68,    69,    70,    10,    11,    44,    26,
      10,    15,    42,    45,    11,    43,    37,    74,   126,    -1,
     121,    -1,   108
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,     1,     7,    73,    74,    77,    22,    23,    29,    78,
       0,    41,    21,    24,    44,    54,    56,    57,    58,    59,
      60,    61,    67,    68,    69,    70,    79,    80,    81,    88,
      93,    97,    98,    99,   100,   101,   102,   103,   104,    74,
      44,    97,     8,    42,     6,    52,    53,    54,    55,    82,
      43,    82,    44,    22,    71,    88,    45,    44,    89,    90,
      99,    81,    99,    97,    97,    97,    97,    99,    97,    88,
      45,    45,    77,     9,    42,    83,     6,    99,    45,    45,
      45,    16,    44,    91,    94,    96,    97,    90,    31,    84,
      99,    44,    75,     6,    99,    91,    91,    97,    10,    11,
      15,    20,    46,    47,    48,    49,    50,    51,    95,    26,
      25,    85,    75,    76,    99,    99,    75,    45,    91,    91,
      16,    92,    44,    96,    87,    88,    26,    42,    45,    75,
      93,    77,    42,    87,    99,    45,    88,    27,    28,    86
};

#if ! defined (YYSIZE_T) && defined (__SIZE_TYPE__)
# define YYSIZE_T __SIZE_TYPE__
#endif
#if ! defined (YYSIZE_T) && defined (size_t)
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T)
# if defined (__STDC__) || defined (__cplusplus)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# endif
#endif
#if ! defined (YYSIZE_T)
# define YYSIZE_T unsigned int
#endif

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrlab1


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { 								\
      yyerror ("syntax error: cannot back up");\
      YYERROR;							\
    }								\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

/* YYLLOC_DEFAULT -- Compute the default location (before the actions
   are run).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)         \
  Current.first_line   = Rhs[1].first_line;      \
  Current.first_column = Rhs[1].first_column;    \
  Current.last_line    = Rhs[N].last_line;       \
  Current.last_column  = Rhs[N].last_column;
#endif

/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (0)

# define YYDSYMPRINT(Args)			\
do {						\
  if (yydebug)					\
    yysymprint Args;				\
} while (0)

# define YYDSYMPRINTF(Title, Token, Value, Location)		\
do {								\
  if (yydebug)							\
    {								\
      YYFPRINTF (stderr, "%s ", Title);				\
      yysymprint (stderr, 					\
                  Token, Value);	\
      YYFPRINTF (stderr, "\n");					\
    }								\
} while (0)

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (cinluded).                                                   |
`------------------------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_stack_print (short *bottom, short *top)
#else
static void
yy_stack_print (bottom, top)
    short *bottom;
    short *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (/* Nothing. */; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_reduce_print (int yyrule)
#else
static void
yy_reduce_print (yyrule)
    int yyrule;
#endif
{
  int yyi;
  unsigned int yylineno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %u), ",
             yyrule - 1, yylineno);
  /* Print the symbols being reduced, and their result.  */
  for (yyi = yyprhs[yyrule]; 0 <= yyrhs[yyi]; yyi++)
    YYFPRINTF (stderr, "%s ", yytname [yyrhs[yyi]]);
  YYFPRINTF (stderr, "-> %s\n", yytname [yyr1[yyrule]]);
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (Rule);		\
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YYDSYMPRINT(Args)
# define YYDSYMPRINTF(Title, Token, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   SIZE_MAX < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#if YYMAXDEPTH == 0
# undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined (__GLIBC__) && defined (_STRING_H)
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
#   if defined (__STDC__) || defined (__cplusplus)
yystrlen (const char *yystr)
#   else
yystrlen (yystr)
     const char *yystr;
#   endif
{
  register const char *yys = yystr;

  while (*yys++ != '\0')
    continue;

  return yys - yystr - 1;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined (__GLIBC__) && defined (_STRING_H) && defined (_GNU_SOURCE)
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
#   if defined (__STDC__) || defined (__cplusplus)
yystpcpy (char *yydest, const char *yysrc)
#   else
yystpcpy (yydest, yysrc)
     char *yydest;
     const char *yysrc;
#   endif
{
  register char *yyd = yydest;
  register const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

#endif /* !YYERROR_VERBOSE */



#if YYDEBUG
/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yysymprint (FILE *yyoutput, int yytype, YYSTYPE *yyvaluep)
#else
static void
yysymprint (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  if (yytype < YYNTOKENS)
    {
      YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
# ifdef YYPRINT
      YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
    }
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  switch (yytype)
    {
      default:
        break;
    }
  YYFPRINTF (yyoutput, ")");
}

#endif /* ! YYDEBUG */
/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yydestruct (int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yytype, yyvaluep)
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  switch (yytype)
    {

      default:
        break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM);
# else
int yyparse ();
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */



/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM)
# else
int yyparse (YYPARSE_PARAM)
  void *YYPARSE_PARAM;
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  
  register int yystate;
  register int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  short	yyssa[YYINITDEPTH];
  short *yyss = yyssa;
  register short *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  register YYSTYPE *yyvsp;



#define YYPOPSTACK   (yyvsp--, yyssp--)

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* When reducing, the number of symbols on the RHS of the reduced
     rule.  */
  int yylen;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed. so pushing a state here evens the stacks.
     */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack. Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	short *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyoverflowlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyoverflowlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	short *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyoverflowlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;


      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YYDSYMPRINTF ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */
  YYDPRINTF ((stderr, "Shifting token %s, ", yytname[yytoken]));

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;


  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  yystate = yyn;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 119 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    {
				stmtList.put(yyvsp[0].relexpr);
				stmtCount++;
			;}
    break;

  case 3:
#line 124 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    {
				stmtList.put(yyvsp[0].relexpr);
				stmtCount++;
			;}
    break;

  case 6:
#line 131 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    { if (yychar) return (0); else exit (0); ;}
    break;

  case 7:
#line 181 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    {
		    yyval.stringList = yyvsp[-1].stringList;
		;}
    break;

  case 8:
#line 185 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    { 
		    yyval.stringList = NULL;
		;}
    break;

  case 9:
#line 202 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    {
		   yyvsp[-2].stringList->put(yyvsp[0].charval);
		   yyval.stringList = yyvsp[-2].stringList;
		;}
    break;

  case 10:
#line 207 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    {
		   yyval.stringList = new List<char *>;
		   yyval.stringList->put(yyvsp[0].charval);
		;}
    break;

  case 11:
#line 224 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    { 
			/* FIX:  add HAVING clause ! */
			RelExpr *t = SelectExpr::create(yyvsp[-2].exprval,yyvsp[-3].relexpr);
			// if ($7) 
			if (yyvsp[-6].intval == 2 || yyvsp[-1].tupleval) 
			{
				t = GroupAggExpr::create(yyvsp[-1].tupleval, yyvsp[-5].projectElemList, t);
				/* FIX:  handle aggregation attrs properly */
				/* ABOVE code repeats gb attrs in agg list */
			}
			else 
				t = ProjectExpr::create(yyvsp[-5].projectElemList,t);
			if (yyvsp[-6].intval)
				t = DupelimExpr::create(t);
			if (yyvsp[0].sortexpr) 
			{
				/* Orderby clause:
  				   sort expr. template already created. */
				 yyvsp[0].sortexpr->child = t;
				 t = yyvsp[0].sortexpr;
			}
			yyval.relexpr = t;

			if ( yyvsp[-2].exprval)
		    ;}
    break;

  case 12:
#line 252 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    {
		    yyval.intval = 1;
		;}
    break;

  case 13:
#line 256 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    {   
		    yyval.intval = 0;
		;}
    break;

  case 14:
#line 260 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    {   
		    yyval.intval = 2;
		;}
    break;

  case 15:
#line 264 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    {   
		    yyval.intval = 0;
		;}
    break;

  case 16:
#line 269 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    {yyval.projectElemList = yyvsp[0].projectElemList; ;}
    break;

  case 17:
#line 271 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    {
			SQLErrE("Yet to implement ``select *''", -1)
		    yyval.projectElemList = new ProjectElemList;
		    ProjectElem tmp;
		    tmp.newName = NULL;
		    tmp.projectionExpr = DaliStar::create();
		    yyval.projectElemList->put(tmp);
		;}
    break;

  case 18:
#line 282 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    {
		    yyvsp[-2].projectElemList->put(yyvsp[0].projectElemVal);
		    yyval.projectElemList = yyvsp[-2].projectElemList;
		 ;}
    break;

  case 19:
#line 287 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    {
		    yyval.projectElemList = new ProjectElemList;
		    yyval.projectElemList->put(yyvsp[0].projectElemVal);
		 ;}
    break;

  case 20:
#line 294 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    {
		    yyval.projectElemVal.newName = yyvsp[0].charval;
		    yyval.projectElemVal.projectionExpr = yyvsp[-1].exprval;
		;}
    break;

  case 21:
#line 299 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    {
		    yyval.projectElemVal.newName = yyvsp[0].charval;
		    yyval.projectElemVal.projectionExpr = yyvsp[-1].exprval;
		;}
    break;

  case 22:
#line 307 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    {
		 	yyval.charval = yyvsp[0].charval;
		;}
    break;

  case 23:
#line 311 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    {
			yyval.charval = NULL;
		;}
    break;

  case 24:
#line 317 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    {
		    yyval.exprval = yyvsp[0].exprval;
		;}
    break;

  case 25:
#line 321 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    {
		    yyval.exprval = NULL;
		;}
    break;

  case 26:
#line 326 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    {
		    yyval.tupleval = yyvsp[0].tupleval;
		;}
    break;

  case 27:
#line 330 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    {
		    yyval.tupleval = NULL;
		;}
    break;

  case 28:
#line 336 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    {
		    yyval.sortexpr = SortExpr::create(yyvsp[-1].tupleval, yyvsp[0].intval, NULL);
			/* Input of sort will be filled in later */
		;}
    break;

  case 29:
#line 341 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    {
		    yyval.sortexpr = NULL;
		;}
    break;

  case 30:
#line 345 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    { yyval.intval = -1; ;}
    break;

  case 31:
#line 346 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    { yyval.intval = 1;;}
    break;

  case 32:
#line 347 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    { yyval.intval = 1;;}
    break;

  case 33:
#line 399 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    {
		    yyval.tupleval = Tuple::appendExpr(yyvsp[-2].tupleval, yyvsp[0].exprval);
		;}
    break;

  case 34:
#line 403 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    {
		    yyval.tupleval = Tuple::create(yyvsp[0].exprval);
		;}
    break;

  case 35:
#line 409 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    {
		   yyval.exprval = RelArgRef::create(yyvsp[-2].charval, yyvsp[0].charval);
		;}
    break;

  case 36:
#line 413 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    {
		   yyval.exprval = RelArgRef::create("", yyvsp[0].charval);
		;}
    break;

  case 37:
#line 419 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    {
		    yyval.relexpr = JoinExpr::create(NULL, yyvsp[-2].relexpr, yyvsp[0].relexpr);
		    // kvr- FIX: Do we really want to do it this way?
		    /* SQL-92: 
			joins (OUTER, CROSS, INNER, ON, NATURAL, USING ..)  
		    */
                ;}
    break;

  case 38:
#line 427 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    {
		    yyval.relexpr = yyvsp[0].relexpr;
                ;}
    break;

  case 39:
#line 433 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    { yyval.relexpr = RelScanExpr::create(yyvsp[0].charval); ;}
    break;

  case 40:
#line 435 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    {
                    RelScanExpr *t = RelScanExpr::create(yyvsp[-3].charval);
		    yyval.relexpr =  RenameExpr::create(yyvsp[-1].charval, yyvsp[0].stringList, t);
		;}
    break;

  case 41:
#line 440 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    {
                    RelScanExpr *t = RelScanExpr::create(yyvsp[-2].charval);
		    yyval.relexpr =  RenameExpr::create(yyvsp[-1].charval, yyvsp[0].stringList, t);
		;}
    break;

  case 42:
#line 445 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    { yyval.relexpr = RenameExpr::create(yyvsp[-1].charval, yyvsp[0].stringList, yyvsp[-4].relexpr);;}
    break;

  case 43:
#line 447 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    { yyval.relexpr = RenameExpr::create(yyvsp[-1].charval, yyvsp[0].stringList, yyvsp[-3].relexpr);;}
    break;

  case 44:
#line 455 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    {
		    yyval.exprval = LogicalExpr::create(LogicalExpr::D_OR, yyvsp[-2].exprval, yyvsp[0].exprval);
		;}
    break;

  case 45:
#line 459 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    {
		    yyval.exprval = LogicalExpr::create(LogicalExpr::D_AND, yyvsp[-2].exprval, yyvsp[0].exprval);
		;}
    break;

  case 46:
#line 463 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    {
		    DaliTruthVal *t = DaliTruthVal::create(yyvsp[0].truthval);
		    if(yyvsp[-1].logicalop == LogicalExpr::D_NOT)
			yyval.exprval = LogicalExpr::create(LogicalExpr::D_IS_NOT, yyvsp[-3].exprval, t);
		    else yyval.exprval = LogicalExpr::create(LogicalExpr::D_IS, yyvsp[-3].exprval, t);
		;}
    break;

  case 47:
#line 473 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    {
			yyval.exprval = LogicalExpr::create(LogicalExpr::D_NOT, yyvsp[0].exprval);	
		;}
    break;

  case 48:
#line 477 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    {
		    yyval.exprval = yyvsp[-1].exprval;
		;}
    break;

  case 49:
#line 481 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    {
		    yyval.exprval = yyvsp[0].exprval;
		;}
    break;

  case 50:
#line 488 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    {
		    yyval.logicalop = LogicalExpr::D_NOT;
		;}
    break;

  case 51:
#line 492 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    {
		    yyval.logicalop = LogicalExpr::D_NO_LOP;
		;}
    break;

  case 52:
#line 497 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    { yyval.truthval = D_TRUE; ;}
    break;

  case 53:
#line 498 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    { yyval.truthval = D_FALSE; ;}
    break;

  case 54:
#line 499 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    { yyval.truthval = D_UNKNOWN; ;}
    break;

  case 55:
#line 504 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    {
		    yyval.exprval = LogicalExpr::create(yyvsp[-1].logicalop, yyvsp[-2].exprval, yyvsp[0].exprval);

			
		 /* 
		   Other things to add: between, like, in, match, all or any,
		   exists, unique, overlaps, test-for-null
		 */
		;}
    break;

  case 56:
#line 515 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    {

		// By default " attrib = all ( select_expression ) " is 
		// created  
		

		yyval.exprval=(LogicalExpr *) new InExpr(yyvsp[-4].exprval,((GeneralProjectExpr *)yyvsp[-1].relexpr));
	  ;}
    break;

  case 57:
#line 525 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    { yyval.logicalop = LogicalExpr::D_EQ; ;}
    break;

  case 58:
#line 526 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    { yyval.logicalop = LogicalExpr::D_NE; ;}
    break;

  case 59:
#line 527 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    { yyval.logicalop = LogicalExpr::D_LT; ;}
    break;

  case 60:
#line 528 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    { yyval.logicalop = LogicalExpr::D_LEQ;;}
    break;

  case 61:
#line 529 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    { yyval.logicalop = LogicalExpr::D_GT; ;}
    break;

  case 62:
#line 530 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    { yyval.logicalop = LogicalExpr::D_GEQ;;}
    break;

  case 63:
#line 535 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    {
		    yyval.exprval = yyvsp[0].exprval;
		/* FIX:  Later add row-wise comparison
		       | '(' expr_list ')'
                */
		;}
    break;

  case 64:
#line 554 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    {
		    yyval.exprval = yyvsp[0].exprval;
		;}
    break;

  case 65:
#line 558 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    {
		    yyval.exprval = ArithExpr::create(ArithExpr::D_PLUS, yyvsp[-2].exprval, yyvsp[0].exprval);
		;}
    break;

  case 66:
#line 562 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    {
			/* FIX:  Also add unary minus */
		    yyval.exprval = ArithExpr::create(ArithExpr::D_MINUS, yyvsp[-2].exprval, yyvsp[0].exprval);
		;}
    break;

  case 67:
#line 567 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    {
		    yyval.exprval = ArithExpr::create(ArithExpr::D_MULT, yyvsp[-2].exprval, yyvsp[0].exprval);
		;}
    break;

  case 68:
#line 571 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    {
		    yyval.exprval = ArithExpr::create(ArithExpr::D_DIV, yyvsp[-2].exprval, yyvsp[0].exprval);
		;}
    break;

  case 69:
#line 577 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    {
		    yyval.exprval = yyvsp[0].exprval;
		;}
    break;

  case 70:
#line 581 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    {
		    yyval.exprval = yyvsp[0].exprval;
		;}
    break;

  case 71:
#line 585 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    {
		    yyval.exprval = yyvsp[-1].exprval;
		;}
    break;

  case 72:
#line 599 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    {
		    yyval.charval = lexString;
		;}
    break;

  case 73:
#line 607 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    {
			yyval.exprval = DaliString::create(lexString, strlen(lexString)+1);
		        free(lexString);
		 ;}
    break;

  case 74:
#line 611 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    {
			yyval.exprval = DaliInt::create(yyvsp[0].intval);
		 ;}
    break;

  case 75:
#line 614 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    {
			yyval.exprval = DaliFloat::create(yyvsp[0].floatval);
		 ;}
    break;

  case 76:
#line 617 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    {
			yyval.exprval = DaliTruthVal::create(yyvsp[0].truthval);
		;}
    break;

  case 77:
#line 634 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    { yyval.intval = lexInt; ;}
    break;

  case 78:
#line 637 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    { yyval.floatval = lexFloat; ;}
    break;

  case 79:
#line 642 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    {
			yyval.exprval = AggExpr::create(AggExpr::D_COUNT, yyvsp[-1].exprval);
		;}
    break;

  case 80:
#line 646 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    {
			yyval.exprval = AggExpr::create(AggExpr::D_COUNT_DISTINCT, yyvsp[-1].exprval);
		;}
    break;

  case 81:
#line 650 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    {
			yyval.exprval = AggExpr::create(AggExpr::D_COUNT_STAR);
		;}
    break;

  case 82:
#line 654 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    {
			yyval.exprval = AggExpr::create(yyvsp[-3].aggfunc, yyvsp[-1].exprval);

	/* FIX:  allow DISTINCT here */
		;}
    break;

  case 83:
#line 661 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    { yyval.aggfunc = AggExpr::D_MAX; ;}
    break;

  case 84:
#line 662 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    { yyval.aggfunc = AggExpr::D_MIN; ;}
    break;

  case 85:
#line 663 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    { yyval.aggfunc = AggExpr::D_SUM; ;}
    break;

  case 86:
#line 664 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"
    { yyval.aggfunc = AggExpr::D_AVG; ;}
    break;


    }

/* Line 999 of yacc.c.  */
#line 1885 "parser.tab.c"

  yyvsp -= yylen;
  yyssp -= yylen;


  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (YYPACT_NINF < yyn && yyn < YYLAST)
	{
	  YYSIZE_T yysize = 0;
	  int yytype = YYTRANSLATE (yychar);
	  char *yymsg;
	  int yyx, yycount;

	  yycount = 0;
	  /* Start YYX at -YYN if negative to avoid negative indexes in
	     YYCHECK.  */
	  for (yyx = yyn < 0 ? -yyn : 0;
	       yyx < (int) (sizeof (yytname) / sizeof (char *)); yyx++)
	    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	      yysize += yystrlen (yytname[yyx]) + 15, yycount++;
	  yysize += yystrlen ("syntax error, unexpected ") + 1;
	  yysize += yystrlen (yytname[yytype]);
	  yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg != 0)
	    {
	      char *yyp = yystpcpy (yymsg, "syntax error, unexpected ");
	      yyp = yystpcpy (yyp, yytname[yytype]);

	      if (yycount < 5)
		{
		  yycount = 0;
		  for (yyx = yyn < 0 ? -yyn : 0;
		       yyx < (int) (sizeof (yytname) / sizeof (char *));
		       yyx++)
		    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
		      {
			const char *yyq = ! yycount ? ", expecting " : " or ";
			yyp = yystpcpy (yyp, yyq);
			yyp = yystpcpy (yyp, yytname[yyx]);
			yycount++;
		      }
		}
	      yyerror (yymsg);
	      YYSTACK_FREE (yymsg);
	    }
	  else
	    yyerror ("syntax error; also virtual memory exhausted");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror ("syntax error");
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      /* Return failure if at end of input.  */
      if (yychar == YYEOF)
        {
	  /* Pop the error token.  */
          YYPOPSTACK;
	  /* Pop the rest of the stack.  */
	  while (yyss < yyssp)
	    {
	      YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
	      yydestruct (yystos[*yyssp], yyvsp);
	      YYPOPSTACK;
	    }
	  YYABORT;
        }

      YYDSYMPRINTF ("Error: discarding", yytoken, &yylval, &yylloc);
      yydestruct (yytoken, &yylval);
      yychar = YYEMPTY;

    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*----------------------------------------------------.
| yyerrlab1 -- error raised explicitly by an action.  |
`----------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;

      YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
      yydestruct (yystos[yystate], yyvsp);
      yyvsp--;
      yystate = *--yyssp;

      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  YYDPRINTF ((stderr, "Shifting error token, "));

  *++yyvsp = yylval;


  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*----------------------------------------------.
| yyoverflowlab -- parser overflow comes here.  |
`----------------------------------------------*/
yyoverflowlab:
  yyerror ("parser stack overflow");
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  return yyresult;
}


#line 666 "/home/ravi/Optimizer/Merged/src/DaliParser/parser.y"


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

