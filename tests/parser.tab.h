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




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;



