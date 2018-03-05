/*
	토큰의 정의　~tokendef.h~						Ver. [2000/02/16]
*/
#ifndef	__TOKENDEF_H
#define	__TOKENDEF_H

enum Token{
	TK_EOF,		// EOF
	TK_ERROR,	// 어휘분석중에 에러
	TK_INCLUDE,	// include
	TK_DEFINE,	// define
	
	TK_LABEL,	// 변수명, 함수명, 주소 라벨
	TK_NUM,		// 수치
	TK_NOTHING,	// 그 이외의 데이터(사실은 Token에 혼합하는 것은 좋지 않다)

	TK_DLM,		// ;
	TK_COLON,	// :

	TK_PLUS,	// +
	TK_MINUS,	// -
	TK_MUL,		// *
	TK_DIV,		// /
	TK_REM,		// %
	TK_AND,		// &
	TK_OR,		// |
	TK_XOR,		// ^
	TK_CPL,		// ~
	TK_NOT,		// !
	TK_DOT,		// .
	TK_COMMA,	// ,
	
//	TK_SHARP,	// #
//	TK_AT,		// @
	
	TK_LBR,		// {
	TK_RBR,		// }
	TK_LPR,		// (
	TK_RPR,		// )
	TK_LSQ,		// [
	TK_RSQ,		// ]
	TK_QUOTE,	// '...'
	TK_WQUOTE,	// "..."

	TK_CMP,		// Op2()의 command용. 사실은 여기에 쓰는 것은 좋지 않다
	TK_BE,		// <
	TK_LE,		// <=
	TK_AB,		// >
	TK_GE,		// >=
	TK_EQ,		// ==
	TK_NE,		// ! =

	TK_BECOME,	// =
	TK_ADD,		// +=
	TK_SUB,		// -=
	TK_MULA,	// *=
	TK_DIVA,	// /=
	TK_ANDA,	// &=
	TK_ORA,		// |=
	TK_XORA,	// ^=
	TK_SHRA,	// >>=
	TK_SHLA,	// <<=
	TK_NEG,		// =-
	TK_CPLA,	// =~

	TK_INC,		// ++
	TK_DEC,		// --
	TK_SHR,		// >>
	TK_SHL,		// <<
	TK_MEMBER,	// ->
	TK_DCOLON,	// ::
	TK_DAND,	// &&

	TK_IF,		// if
	TK_ELSE,	// else
	TK_LOOP,	// loop		// loop{}와 break
	TK_BREAK,	// break
	TK_ALT,		// alt		// alt { case a : st , case b: st , default:xxx }
	TK_CASE,	// case
	TK_DEFAULT,	// default
	TK_GOTO,	// goto
	TK_RETURN,	// return
	TK_STRUCT,	// struct
	TK_ASMOUT,	// asmout
	TK_FOR,		// for
	TK_WHILE,	// while
	TK_DO,		// do
	TK_CONTINUE,// continue

	TK_CF,		// CF
	TK_ZF,		// ZF

	TK_DWORD,	// dword
	TK_WORD,	// word
	TK_BYTE,	// byte
	TK_INT,		// int
	TK_LONG,	// long
	TK_SHORT,	// short
	TK_CHAR,	// char
	TK_VOID,	// void
	TK_NEAR,	// near
	TK_FAR,		// far
	TK_STATIC,	// static
	TK_SIGNED,	// signed
	TK_UNSIGNED,// unsigned
	TK_OFFSET,	// offset
	TK_SEGMENT,	// segment
	TK_REP,		// rep
	TK_TRUE,	// true
	TK_FALSE,	// false
//	TK_LOCAL,	// local
	TK_SIZEOF,	// sizeof
//	TK_CODE,	// default(code == CODE);

	TK_PARA, TK_PAGE, TK_PAGE4K,		// align 속성. BYTE, WORD, DWORD도 있다
	TK_PRIVATE, TK_PUBLIC, TK_STACK, TK_COMMON,	// 콤바인 속성
	TK_USE16, TK_USE32,							// USE 속성
	TK_RO, TK_EO, TK_ER, TK_RW,					// 액세스 속성
};

#endif
