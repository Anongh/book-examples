/*
	구문 분석 클래스　~parser.h + parser.cpp~
*/
#ifndef	__PARSER_H
#define	__PARSER_H

#pragma warning(disable:4786)

#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include <string>
#include <list>
#include <map>
#include <stack>

using namespace std;

#include "macro.h"
#include "module.h"
#include "scanner.h"
#include "generator.h"
#include "filepath.h"

class Parser{
  public:
	typedef	Generator::LineData	LineData;

  private:
	Scanner		scanner;			// 스캔 클래스
	Generator	generator;			// 코드 제네레이터
	
	int			StructAlignCount;	// align를 위한 유사 라벨명
	int			LoopLabel[32];		// 네스트 한 loop문으로 break하기 위해
	int			LoopLabelPoint;		// 네스트수
	int			LocalLabelCounter;	// 조건 분기나 루프로 내부적으로 사용하는 라벨
	int			StatementLevel;		// statement의 레벨
	LPSTR		lpFunctionName;		// 현재 처리하고 있는 함수명
	Parameter	defaultlocal;		// 로컬 변수용 local == SS:EBP;
	SegmentList*defaultsegment;		// 함수를 할당하는 default segment
	SegmentList*defaultdatasegment;		// 변수를 할당하는 default segment
	int			SysVarLocalValue;	// 함수내의 로컬 변수 영역의 바이트수
	
	LPSTR		lpLogFileName;		// 에러 메세지 출력 파일명			
	FILE*		lpLogFP;			// 에러 메세지 출력용 파일 포인터
	int			nErrorCount;		// 구문 분석부에서 일어난 에러의 수

	//int			stackpoint;		// 함수가 call되었을 때 stackpoint로부터 추가하고 있는 양

	// 이하 HRESULT가 있는 것은 정상 종료시는 0, 하위의 루틴으로 에러가 났을 때는 1,
	// 그 이외의 숫자는 함수 내부에서의 에러라고 하는 의미를 가진다. (즉! =0이라면 에러)
	HRESULT	Sizeof(Parameter& param);
	HRESULT	Address(Parameter& param);
	HRESULT	LocalAddress(Parameter& param);
	HRESULT	Selector(Parameter& param, LabelList* label);
	HRESULT	Selector2(Parameter& param);
	void	Member2Param(Parameter& param, MemberList* member);
	HRESULT	Param2Param(Parameter& to, Parameter& from);
	HRESULT	Array(Parameter& param);
	void	PointerCheck(RegisterList* reg);
	HRESULT	ArrayReg(Parameter& param, RegisterList* reg);
	HRESULT	Pointer(Parameter& param);
	HRESULT	GetParameter(Parameter& param);
	HRESULT	Cast(Parameter& param);
	HRESULT	Immedeate(Parameter& param);
	void	Expression(void);
	void	Statement(void);
	void	DefineFunction(void);
	void	DefineVariable(void);
	void	InitialArray(LabelList* label);
	void	DefineAlias(LabelList* label);
	void	DefineInitial(LabelList* label, LPSTR buf);
	void	DefineStruct(void);
	void	DefineMember(TagList* tag);
	void	DefineSegment(void);
	void	DefineDefault(void);
	void	StatementSequence(void);
	void	IfStatement(void);
	CompareType Compare(void);
	CompareType IsCmpOperator(bool bSigned);
	CompareType TransCompare(CompareType cmptype);
	CompareType FlagCompare(void);
	void	LoopStatement(void);
	void	BreakStatement(CompareType cmptype = C_JMP);
	void	AltStatement(void);
	void	AsmoutStatement(void);
	void	ForStatement(void);
	void	WhileStatement(void);
	void	DoStatement(void);
	void	ContinueStatement(CompareType cmptype = C_JMP);
	void	AssumeSegment(void);


//	void	alt_statement(void);
//	bool	case_block(DWORD dw); // 라벨 ID
	void	StartParse(void);

	void	Error(LPSTR str);

  public:
	Parser(void);
	~Parser(){}
	
#ifdef WINVC
	HRESULT	Compile(string& filename);
#else
	HRESULT	Compile(string filename);
#endif

#ifdef WINVC
	HRESULT	Compile(string& filename, string& outfilename);
#else
	HRESULT	Compile(string filename, string outfilename);
#endif

	void	SetLogFile(LPSTR filename);
	int		GetErrorCount(void){ return nErrorCount; }
};

#endif
