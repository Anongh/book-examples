/*
	어휘분석 클래스　~scanner.h + scanner.cpp~
*/
#ifndef	__SCANNER_H
#define	__SCANNER_H

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
#include "tokendef.h"

class ScannerSub{
	int		nLine;				// 현재 처리하고 있는 행 번호
	Module	Mdl;					// 현재 처리하고 있는 파일
	FILE*	lpLogFP;				// 에러 메세지 출력용 파일 포인터
	char	linebuf[1024];				// 현재 처리하고 있는 행의 버퍼
	LPSTR	lpPos;					// 현재 처리하고 있는 행안의 위치
	Token	token;					// 토큰의 종류
	char	labelbuf[256];				// token이 TK_LABEL등일 때의 버퍼
	LONG	numbuf;					// token이 TK_NUM일 때의 버퍼
	bool	bPeeked;				// 벌써 PeekToken()로 토큰을 취득하고 있을까
	int		nErrorCount;			// 이 소스로 일어난 에러의 수

	void	Init(void);				// 버퍼의 초기화
	HRESULT	ReadLine(void);				// 1행 읽어들인다
	bool	IsToken(LPSTR &lp, LPSTR lp2);		// 일치하고 있으면 lp1를 토큰의 마지막까지 진행한다
	void	CopyLabel(LPSTR& lpPos);		// lpPos로부터 다음의 토큰까지 카피
	HRESULT	NumCheck(LPSTR& lpPos);			// 조사해서 수치라면 numbuf에. 그렇지 않으면 반환값：0이 아니다
	void	GetQuotedLabel(LPSTR &lpPos);		// 인용 라벨을 얻는다
	Token	PeekToken2(void);			// PeekToken()의 본체
	void	Error(LPSTR str);			// 에러 메세지를 표시

  public:
	ScannerSub(){ lpLogFP=stderr; nErrorCount=0; Init(); }
	~ScannerSub(){}

#ifdef WINVC
	HRESULT	ReadFile(string& filename);// 파일을 읽어들인다
#else
	HRESULT	ReadFile(string filename);// 파일을 읽어들인다
#endif
	
	Token	GetToken(void);			// 다음의 token을 얻는다(read 포인터 진행한다)
	Token	PeekToken(void){ token=PeekToken2(); return token; }
							// 다음의 token을 얻는다(read 포인터 진행하지 않는다)
	LPSTR	GetLabel(void){ return labelbuf; }	// token가 TK_LABEL등일 때, 그 라벨을 얻을 수 있다
	LONG	GetNum(void){ return numbuf; }		// token가 TK_NUM일 때, 그 수치를 얻을 수 있다

	int		GetScanline(void){ return nLine; }
	string	GetFileName(void){ return Mdl.GetFileName(); }
	void	SetLogFile(FILE* fp){ lpLogFP = fp; }
	int		GetErrorCount(void){ return nErrorCount; }
};

typedef stack<ScannerSub*> StacklpScannerSub;

class Scanner{
	StacklpScannerSub files;
	int		nIncludeNest;			// 인클루드의 네스트 수. 없을때는 0
	FILE*	lpLogFP;				// 에러 메세지 출력용 파일 포인터
	int		nErrorCount;			// 어휘분석부에서 일어난 에러의 수

  public:
	Scanner(){ lpLogFP=stderr; nIncludeNest=0; nErrorCount=0; }
	~Scanner(){ while(! files.empty()){ DELETE_SAFE(files.top()); files.pop(); } }

#ifdef WINVC
	HRESULT	ReadFile(string& filename);// 파일을 읽어들인다
#else
	HRESULT	ReadFile(string filename);// 파일을 읽어들인다
#endif

	Token	GetToken(void);		// 다음의 token을 얻는다(read 포인터 진행한다)
	Token	PeekToken(void);	// 다음의 token을 얻는다(read 포인터 진행하지 않는다)
	LPSTR	GetLabel(void){ return files.top()->GetLabel(); }
								// token가 TK_LABEL등일 때, 그 라벨을 얻을 수 있다
	LONG	GetNum(void){ return files.top()->GetNum(); }
								// token가 TK_NUM일 때, 그 수치를 얻을 수 있다
	int		GetScanline(void){ return files.top()->GetScanline(); }
	string	GetFileName(void){ return files.top()->GetFileName(); }
	void	SetLogFile(FILE* fp){ lpLogFP = fp; }
	int		GetErrorCount(void){ return nErrorCount; }
};

#endif
