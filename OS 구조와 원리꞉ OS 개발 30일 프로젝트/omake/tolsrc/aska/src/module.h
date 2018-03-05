/*
	파일 모듈화 클래스　~module.h + module.cpp~
*/
#ifndef	__MODULE_H
#define	__MODULE_H

#pragma warning(disable:4786)

#include <stdio.h>
#include <stdlib.h>
#ifndef LINUX
#include <io.h>
#endif
#include <iostream>

#include <string>
#include <list>
#include <map>
#include <stack>

using namespace std;

#include "macro.h"

class Module{
	LPVOID	lpMdlAdr;		// 모듈 주소
	DWORD	dwMdlSize;		// 모듈 사이즈
	LPSTR	lpMdlPos;		// ReadLine의 처리 위치
	string	FileName;		// 모듈화하고 있는 파일명

	string	MakeFullPath(string& filename);		// 디렉토리를 보충한다

  public:
	Module(void){ lpMdlAdr=NULL; dwMdlSize=0; lpMdlPos=NULL; }
	~Module(){ Release(); }
#ifdef WINVC
	void	Release(void){ DELETEPTR_SAFE(lpMdlAdr); dwMdlSize=0; lpMdlPos=NULL; }
#else
	void	Release(void){ DELETEPTR_SAFE((unsigned char*) lpMdlAdr); dwMdlSize=0; lpMdlPos=NULL; }
#endif

	string	GetFileName(void){ return FileName; }
	LPVOID	GetModuleAddress(void){ return lpMdlAdr; }
	DWORD	GetModuleSize(void){ return dwMdlSize; }
	HRESULT ReadFile(string& filename);
		// 반환값 0:정상 종료  1:fileopen 실패  3:메모리 확보 실패  4:read 실패  5:fileclose 실패
	HRESULT ReadLine(LPSTR buf);
		// 반환값 0:정상 종료  1:EOF  2:버퍼 overflow  3:파일 non read
};

//현단계에서는 디렉토리의 추가 등은 하고 있지 않다. 그 중
//include 디렉토리를 추가해서 순서를 검색해 나가는 방식으로 하고 싶다.

#endif
