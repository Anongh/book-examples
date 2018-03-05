/*
	텍스트 파일 모듈화 클래스　~textmodule.h + textmodule.cpp~
															Ver. [2000/02/17]
*/
#ifndef	__TEXTMODULE_H
#define	__TEXTMODULE_H

#pragma warning(disable:4786)

#include <stdio.h>
#include <stdlib.h>

#include <string>
#include <list>
#include <map>
#include <stack>

using namespace std;


class	TextModule{
  public:
// 이하 2개에 의해, String의 실체가 string인가 wstring인가에는 의존하지 않게 한다
	typedef string					String;
	typedef	unsigned char			Letter;
	typedef	long					SizeType;
	typedef	list<String>			ListString;
	typedef	ListString::iterator	LineData;

  protected:
	ListString	TextData;			// 텍스트 데이터를 보존하는 버퍼
	
	String		FileName;			// 현재 관련 지을 수 있었던 파일명
	LineData	LineItr;			// 현재의 행의 이테레이타
	int			LinePos;		// 현재의 행 번호
	bool		BufferEOF;			// EOF라면 true가 된다

  public:
	TextModule(){ LineItr=TextData.begin(); LinePos=1; BufferEOF=false; }
	~TextModule(){}
	
	void		New();					// 신규 작성
	void		Open(String& filename);	// open
	void		Close(){ New(); }		// close
	void		Save();					// 덮어쓰기 보존
	void		Save(String& filename){ FileName=filename; Save(); }	// 이름을 붙여 보존
	
	String		GetFileName(){ return FileName; }	// 파일명 취득
	SizeType	GetLinePos(){ return LinePos; }		// 처리행 취득
//	SizeType	GetFileSize();					// 파일 사이즈 취득
	SizeType	GetMaxLinePos(){ return TextData.size(); }	// 최대행=맨 마지막 줄
	bool		IsEOF(){ return BufferEOF; }		// EOF 조사
	
	void		NextLine(SizeType line=1);		// 다음 행으로 진행
	void		PrevLine(SizeType line=1);		// 앞의 행으로 돌아온다
	void		SeekLine(SizeType linepos);		// 행 번호로 진행
	void		SeekLine(LineData);				// 행에 진행
	SizeType	Seek(LineData);					// 행 정보로부터 행 번호 취득

	String		PeekLine();						// 행 read. 다음에 진행되지 않는다
	String		GetLine();						// 행 read. 다음에 진행된다

#ifdef WINVC
	void		PutLine(String& str){ InsertLine(LineItr, str); }
#else
	void		PutLine(String str){ InsertLine(LineItr, str); }	// 행 write. 다음에 진행된다
#endif
	LineData	ReserveLine(){ return InsertLine(LineItr, String()); }	// 공백행을 삽입. 북마크로 사용한다. 사용하면 EraseLine()로 소거
#ifdef WINVC
	LineData	InsertLine(LineData, String&);
#else
	LineData	InsertLine(LineData, String);	// 행 삽입
#endif
	void		EraseLine(LineData);		// 행 소거
};

#endif
