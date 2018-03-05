/*
	문자열 조작 클래스　~stringx.h~		Ver. [2000/02/11]
*/
#ifndef	__STRINGX_H
#define	__STRINGX_H

#pragma warning(disable:4786)

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string>
#include <list>
#include <map>
#include <stack>

using namespace std;



class stringx{
  public:
	typedef	string::size_type	size_type;

  protected:
	string		str;

// 문자열 조작 스태틱 루틴
  public:
	static size_type	strlen(char* s);

// string 조작 루틴
  public:
	void		set(string s){ str=s; }
	string		get(){ return str; }
	stringx&	operator=(string& s){ set(s); return *this; }
	stringx&	operator=(char* s){ set(s); return *this; }
	string		operator()(){ return get(); }
				operator string(){ return get(); }
	
	string		substr(size_type pos, size_type n);
	string		copy(size_type pos, size_type n){ return substr(pos, n); }
	string		cut(size_type pos, size_type n);
	string		left(size_type pos){ return substr(0, pos); }
	string		right(size_type pos){ return substr(pos+1, str.size() -pos-1); }
//	void		fromdigit();
	string		tolower();
	string		toupper();
	size_type	lastdelimiter(char* s);
//	size_type	find(string s, size_type pos=0);
//	size_type	search(string s, size_type pos=0){ return find(s, pos); }
//	void		replace();
	long		tolong(){ return atol(str.c_str()); }
	double		todouble(){ return atof(str.c_str()); }
};

#endif
