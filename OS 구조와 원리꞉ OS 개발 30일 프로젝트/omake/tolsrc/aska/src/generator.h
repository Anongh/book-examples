/*
	코드 생성 클래스　~generator.h + generator.cpp~
*/
#ifndef	__GENERATOR_H
#define	__GENERATOR_H

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
#include "table.h"
#include "textmodule.h"

class Generator{
  public:
	typedef	TextModule::LineData	LineData;

  private:
	Register		RegisterData;
	Segment			SegmentData;
	Tag				TagData;
	Label			LocalData;
	Label			GlobalData;
	bool			bExistMain;			// 원시 파일내에 main()이 나오면 true

	string			OutFileName;			// 코드를 출력하는 파일명
	TextModule		OutFile;			// 코드 출력용 파일 모듈
	Scanner*		scanner;			// Scanner의 포인터(Error()로 사용한다)
	FILE*			lpLogFP;			// 에러 메세지 출력용 FP
	int				nErrorCount;		// 이 소스로 일어난 에러의 수

	int				CheckPower2(int x);
	void			FlushStaticData(void);
	void			RegistBecome(Parameter& param1, Parameter& param2);
	HRESULT			RegisterDown(Parameter& param1, Parameter& param2);
	void			RegistAdd(Parameter& param1, Parameter& param2);
	void			RegistSub(Parameter& param1, Parameter& param2);
	void			RegistMul(Parameter& param1, Parameter& param2);
	void			RegistDiv(Parameter& param1, Parameter& param2);
	void			RegistAnd(Parameter& param1, Parameter& param2);
	void			RegistOr(Parameter& param1, Parameter& param2);
	void			RegistXor(Parameter& param1, Parameter& param2);
	void			RegistCmp(Parameter& param1, Parameter& param2);
	void			RegistShl(Parameter& param1, Parameter& param2);
	void			RegistShr(Parameter& param1, Parameter& param2);
	void			OpenSegment(SegmentList* segment);
	void			CloseSegment(SegmentList* segment);
	void			Error(LPSTR str);	// 에러 메세지를 표시

  public:
	SegmentList*	prevseg;		// 전에 처리하고 있던 세그먼트(segment)를 넣는다
	SegmentList*	seg;			// 현재 처리 중의 세그먼트(segment)를 넣는다

	void	debug(void);
	Generator();
	~Generator(){}
	
	void			BeginGenerate(string& outfilename, Scanner* s);
	void			EndGenerate(void);

	RegisterList*	FindRegisterList(LPSTR key){ return RegisterData.Find(key); }
	void			AddSegmentList(LPSTR key, SegmentList* s){ SegmentData.Add(key, s); }
	SegmentList*	FindSegmentList(LPSTR key){ return SegmentData.Find(key); }
	void			AddTagList(LPSTR key, TagList* t){ TagData.Add(key, t); }
	TagList*		FindTagList(LPSTR key){ return TagData.Find(key); }
	void			AddLocalLabelList(LPSTR key, LabelList* l){ LocalData.Add(key, l); }
	LabelList*		FindLocalLabelList(LPSTR key){ return LocalData.Find(key); }
	void			ClearLocalLabelList(void){ LocalData.Clear(); }
	void			AddGlobalLabelList(LPSTR key, LabelList* l){ GlobalData.Add(key, l); }
	LabelList*		FindGlobalLabelList(LPSTR key){ return GlobalData.Find(key); }
	LabelList*		FindLabelList(LPSTR key);

	void			Seek(LineData linedata){ OutFile.SeekLine(linedata); }
	LineData		Reserve(){ return OutFile.ReserveLine(); }
	void			Erase(LineData linedata){ OutFile.EraseLine(linedata); }

	void			Param2LPSTR(LPSTR buf, Parameter& param);
	void			OutputMASM(LPSTR label, LPSTR command, LPSTR param, LPSTR comment);
	void			Call(Token ptype, LPSTR func);
	void			GlobalLabel(LPSTR label);
	void			LocalLabel(LPSTR label);
	void			Op1(Token command, Parameter& param);
	void			Op2(Token command, Parameter& param1, Parameter& param2);
	void			StartFunction(LPSTR funcname, Token ptype, SegmentList* segment);
	void			EndFunction(LPSTR funcname, int sysvarlocalvalue);
	void			RegistVariable(LPSTR varname, LabelList* label, LPSTR initialize);
	void			StartSegment(SegmentList* segment);
	void			EndSegment(void);
	void			AssumeSegment(SegmentList* seg, RegisterList* reg);
	void			Jump(CompareType cmptype, int labelcount);
	void			GlobalJump(CompareType cmptype, LPSTR globallabel);
	void			LocalJump(CompareType cmptype, LPSTR locallabel);
	void			Jump(CompareType cmptype, LPSTR locallabel);
	
	LPSTR			ConstString(LPSTR str);
	
	// 잠정 커맨드
	void			Asmout(LPSTR buf);

	void			SetLogFile(FILE* fp){ lpLogFP = fp; }
	int				GetErrorCount(void){ return nErrorCount; }
};

#endif
