/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

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
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     T_INT = 258,
     T_SET = 259,
     T_NOP = 260,
     T_STOP = 261,
     T_LOAD = 262,
     T_STORE = 263,
     T_BLOAD = 264,
     T_BSTORE = 265,
     T_PUSH = 266,
     T_POP = 267,
     T_DUP = 268,
     T_INVERT = 269,
     T_ADD = 270,
     T_SUB = 271,
     T_MULT = 272,
     T_DIV = 273,
     T_COMPARE = 274,
     T_JUMP = 275,
     T_JUMP_YES = 276,
     T_JUMP_NO = 277,
     T_INPUT = 278,
     T_PRINT = 279,
     T_COLON = 280
   };
#endif
/* Tokens.  */
#define T_INT 258
#define T_SET 259
#define T_NOP 260
#define T_STOP 261
#define T_LOAD 262
#define T_STORE 263
#define T_BLOAD 264
#define T_BSTORE 265
#define T_PUSH 266
#define T_POP 267
#define T_DUP 268
#define T_INVERT 269
#define T_ADD 270
#define T_SUB 271
#define T_MULT 272
#define T_DIV 273
#define T_COMPARE 274
#define T_JUMP 275
#define T_JUMP_YES 276
#define T_JUMP_NO 277
#define T_INPUT 278
#define T_PRINT 279
#define T_COLON 280




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

