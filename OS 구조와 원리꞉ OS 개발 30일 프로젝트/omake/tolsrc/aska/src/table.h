/*
	데이터 테이블 클래스　~table.h + table.cpp~
*/
#ifndef	__TABLE_H
#define	__TABLE_H

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

enum RegisterType{
	R_GENERAL,		// 범용 레지스터
	R_SEGREG,		// 세그먼트(segment) 레지스터
	R_CTRL,			// 컨트롤 레지스터
	R_DEBUG,		// 디버그 레지스터
	R_TEST,			// 테스트 레지스터
	R_FLOAT,		// 부동 소수점 레지스터
	R_MMX,			// MMX 레지스터
};

class RegisterList{
  public:
	LPSTR			name;		// 레지스터명
	RegisterType	type;			// 레지스터 타입
	int				size;	// 레지스터 사이즈
	bool			bBase;	// 베이스 레지스터일까
	bool			bIndex;	// 인덱스 레지스터일까

	RegisterList(LPSTR n, RegisterType t, int s, bool b, bool i)
			{ name=n; type=t; size=s; bBase=b; bIndex=i; }
};

typedef map<string, RegisterList*> MaplpRegisterList;

class Register{
	MaplpRegisterList	mapregister;
	
  public:
	Register(void);
	~Register();
	
	void			Add(LPSTR key, RegisterList* r){ string k=key; mapregister[k]=r; }
	RegisterList*	Find(LPSTR key){
		string k = key;
		MaplpRegisterList::iterator it = mapregister.find(k);
		if(it == mapregister.end()) return NULL; else return it->second;
	}
};

class LabelList;

class StaticDataList{
  public:
	LPSTR			name;
	LabelList*		label;
	LPSTR			lpInit;
};

typedef list<StaticDataList> ListStaticDataList;

class SegmentList{
  public:
	ListStaticDataList liststatic;

	LPSTR			name;			// 세그먼트(segment) 네임
	Token			align;			// align 속성
	Token			combine;		// 콤바인 속성
	Token			use;			// USE 속성
	Token			access;			// 액세스 속성
	LPSTR			segmentclass;		// 세그먼트(segment) 클래스

	RegisterList*	assume;				// segment CODE == ES;의 ES를 보존하기 위해서 사용한다

	SegmentList(void)
		{ name=NULL; align=TK_BYTE; combine=TK_PRIVATE; use=TK_USE32;
			access=TK_ER; segmentclass=NULL; assume=NULL; }
	SegmentList(LPSTR n, Token al, Token co, Token us, Token ac, LPSTR cl)
		{ name=n; align=al; combine=co; use=us; access=ac; segmentclass=cl; assume=NULL; }

	void	AddStaticData(LPSTR varname, LabelList* label, LPSTR initialize);
};

typedef map<string, SegmentList*> MaplpSegmentList;

class Segment{
  public:
	MaplpSegmentList	mapsegment;
	
	Segment(void);
	~Segment();
	
	void			Add(LPSTR key, SegmentList* s){ string k=key; mapsegment[k]=s; }
	SegmentList*	Find(LPSTR key){
		string k = key;
		MaplpSegmentList::iterator it = mapsegment.find(k);
		if(it == mapsegment.end()) return NULL; else return it->second;
	}
};

class TagList;

class MemberList{
  public:
	bool			bSigned;	// 부호가 있는 형태인가
	TagList*		type;		// 변수의 형태
	Token			ptype;		// near:TK_NEAR, far:TK_FAR
	int				pdepth;	// 포인터 깊이. 이것이 0이라면 포인터가 아니다
	bool			bArray;		// 배열일까
	int				size;	// 변수 사이즈(배열, 포인터, 구조체도 가미)
	int				offset;	// 구조체에서의 멤버의 오프셋(offset)

	MemberList(void)
		{ type=NULL; bSigned=true; ptype=TK_FAR; pdepth=0; bArray=false; size=0; offset=0; }
	MemberList(bool bs, TagList* t, Token pt, int pd, bool ba, int s, int o)
		{bSigned=bs; type=t; ptype=pt; pdepth=pd; bArray=ba; size=s; offset=o; }
	MemberList(MemberList* m)
		{bSigned=m->bSigned; type=m->type; ptype=m->ptype; pdepth=m->pdepth;
			bArray=m->bArray; size=m->size; offset=m->offset; }
	MemberList&	operator=(MemberList& label);
};

typedef map<string, MemberList*> MaplpMemberList;

// typedef나 처음부터 있는 형태는 "type"라고 하는 멤버를 본다. typedef라면 member->type! =NULL
class TagList{
  public:
	int				size;		// 이 형태의 사이즈
	bool			bStruct;		// 구조체일까
	MaplpMemberList	mapmember;			// 구조체이면 멤버의 리스트
	
	TagList(bool bs){ bStruct=bs; size=0;}
	~TagList();

	void			AddMemberList(LPSTR n, bool bs, TagList* t, Token pt, int pd, bool ba, int s);
	MemberList*		FindMemberList(LPSTR key){
		string k = key;
		MaplpMemberList::iterator it = mapmember.find(k);
		if(it == mapmember.end()) return NULL; else return it->second;
	}
};

typedef map<string, TagList*> MaplpTagList;

class Tag{
	MaplpTagList	maptag;
	
  public:
	Tag(void);
	~Tag();
	
	void			Add(LPSTR key, TagList* t){ string k=key; maptag[k]=t; }
	TagList*		Find(LPSTR key){
		string k = key;
		MaplpTagList::iterator it = maptag.find(k);
		if(it == maptag.end()) return NULL; else return it->second;
	}
};


enum ParameterType{
	P_ERR,						// 미초기화 혹은 에러
	P_REG,						// 파라미터 타입이 레지스터. base에 들어간다
	P_MEM,						// 파라미터 타입이 메모리. base에 들어간다
	P_IMM,						// 파라미터 타입이 레지스터. base에 들어간다
};

class Parameter{
  public:
	ParameterType	paramtype;	// P_ERR, P_REG, P_MEM, P_IMM
	RegisterList*	seg;
	RegisterList*	base;
	RegisterList*	index;
	int				scale;		// 1, 2, 4, 8
	string			disp;
	int				ndisp;		// 여기는 멤버 검색시 오프셋(offset) 때문에 있다
	bool			bLabel;			// P_MEM에서도 Label인가
	bool			bSigned;		// 부호가 있는 형태인가
	TagList*		type;			// 변수의 형태
	Token			ptype;		// near:TK_NEAR, far:TK_FAR, offset:TK_OFFSET, segment:TK_SEGMENT
	int				pdepth;		// 포인터 깊이. 이것이 0이라면 포인터가 아니다
	int				size;		// 변수 사이즈(배열, 포인터, 구조체도 가미)
	bool			bArray;			// 배열일까

	void	Initialize()
		{ paramtype=P_ERR; seg=NULL; base=NULL; index=NULL; scale=1; disp=""; ndisp=0; size=0;
			bLabel=false; bSigned=true; type=NULL; ptype=TK_FAR; pdepth=0; bArray=false; }
	
	Parameter(void){ Initialize(); }
	Parameter&	operator=(Parameter& param);
};

//typedef list<TagList*> ListlpTagList;

//label는 type:dword, ptype:normal, bAddress:false, size:4로 한다
class LabelList{
  public:
	bool			bStatic;
	bool			bSigned;	// 부호가 있는 형태인가
	TagList*		type;		// 변수 형태
	Token			ptype;		// near:TK_NEAR, far:TK_FAR
	int				pdepth;		// 포인터 깊이. 이것이 0이라면 포인터가 아니다
	SegmentList*	segment;			// 데이터를 어디에 할당할까
	int				size;		// 변수 사이즈(배열, 포인터, 구조체도 가미)
	bool			bArray;		// 배열일까
	bool			bAlias;		// 앨리어스(alias)일까
	Parameter		alias;		// 앨리어스(alias)(정수는 여기에 숫자를 넣는다)
	int				nLocalAddress;	// 로컬 라벨이라면 로컬 주소를 넣는다
	bool			bFunction;	// 함수일까
//	ListlpTagList	listtag;		// 함수라면 함수의 인수의 형태를 넣는다

	LabelList(void)
		{bSigned=true; type=NULL; ptype=TK_FAR; pdepth=0; segment=NULL; size=0; bArray=false;
			bAlias=false; nLocalAddress=0; bFunction=false; bStatic=false; }
	LabelList(bool bs, TagList* t, Token pt, int pd, SegmentList* s
			, int sz, bool bary, bool balias, Parameter& a, int nla, bool bf)
		{ bSigned=bs; type=t; ptype=pt; pdepth=pd; bArray=bary; bFunction=bf; nLocalAddress=nla;
			size=sz; bAlias=balias; alias=a; segment=s; bStatic=false; }
	LabelList(LabelList* l)
		{ bSigned=l->bSigned; type=l->type; ptype=l->ptype; pdepth=l->pdepth; bArray=l->bArray;
			bFunction=l->bFunction; nLocalAddress=l->nLocalAddress; size=l->size; bAlias=l->bAlias;
			alias=l->alias; segment=l->segment; bStatic=l->bStatic; }
	LabelList&	operator=(LabelList& label);
};

typedef map<string, LabelList*> MaplpLabelList;

class Label{
	MaplpLabelList	maplabel;

  public:
	Label(void){}
	~Label(){ Clear(); }
	
	void			Clear(void);
	void			Add(LPSTR key, LabelList* l){ string k=key; maplabel[k]=l; }
	LabelList*		Find(LPSTR key){
		string k = key;
		MaplpLabelList::iterator it = maplabel.find(k);
		if(it == maplabel.end()) return NULL; else return it->second;
	}
};

enum CompareType{
	C_NOTHING,	// 없었다
	C_JA,		// (unsigned)>
	C_JAE,		// (unsigned)>=
	C_JB,		// (unsigned)<
	C_JBE,		// (unsigned)<=
	C_JG,		//   (signed)>
	C_JGE,		//   (signed)>=
	C_JL,		//   (signed)<
	C_JLE,		//   (signed)<=
	C_JE,		// ==
	C_JNE,		// ! =
	C_JC,		// CF == 1
	C_JNC,		// CF == 0
	C_JMP,		// 
};

#endif
