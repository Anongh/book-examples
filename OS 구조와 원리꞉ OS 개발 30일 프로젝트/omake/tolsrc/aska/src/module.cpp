#include <string.h>
#include "module.h"
#ifdef LINUX
#include <sys/stat.h>
unsigned int filelength(int fd){
  struct stat st;
  if (fstat(fd, &st))
    return 0;
  return st.st_size;
}
#endif

//현재는 디렉토리의 검색 등은 하고 있지 않기 때문에, 단순하게""나 <>을 뗄 뿐
string	Module::MakeFullPath(string& p){
	string	str = p.substr(0, 0);
	if(str == "\"" || str == "\'" || str == "<"){
		str = p.substr(1, str.size()-1);
	}else{
		str = p.substr();
	}
	return str;
}


HRESULT	Module::ReadFile(string& filename){
	FILE* lpFP;

	Release();
	FileName = MakeFullPath(filename);
	lpFP = fopen(FileName.c_str(), "rb");
	if(lpFP == NULL){
		return 1;	// fileopen 실패
	}
	
	dwMdlSize = filelength(fileno(lpFP));

	if (dwMdlSize == 0){
	  fclose(lpFP);
	  return 2;
	}

	lpMdlAdr = (LPVOID) new unsigned char[dwMdlSize];
	
	if(lpMdlAdr == NULL){
		dwMdlSize = 0;
		fclose(lpFP);
		return 3;	// 메모리 확보 실패
	}
	if(fread(lpMdlAdr, 1, dwMdlSize, lpFP) ! = dwMdlSize){
		dwMdlSize = 0;
		fclose(lpFP);
		return 4;	// read 실패
	}
	if(fclose(lpFP)){
		lpMdlAdr = NULL;
		dwMdlSize = 0;
		return 5;	// fileclose 실패
	}
	lpMdlPos = (LPSTR) lpMdlAdr;
	return 0;		// 정상 종료
}

HRESULT Module::ReadLine(LPSTR buf){
	int i, j;
	
	if(lpMdlPos == NULL) return 3;
	j = ((LPSTR) lpMdlAdr + dwMdlSize) - lpMdlPos;
	if (j <= 0) return 1;	// 이제 파일이 없다
	if (j > 1023) i = 1023;	// 버퍼 사이즈로 한계점 도달
	for(i = j; i > 0; i--){
		if(*lpMdlPos == 0x0D && *(lpMdlPos+1) == 0x0A){
			*buf = '\0';
			lpMdlPos += 2;
			return 0;	// 1행 종료
		}

		if (*lpMdlPos == 0x0a || *lpMdlPos == 0x0d){
		  *buf = '\0';
		  lpMdlPos++;
		  return 0;
		}

		*(buf++) = *(lpMdlPos++);
	}
	*buf = '\0';
	if (j < 1023)
	  return 0;		// 파일 종단
	else
	  return 2;	// 버퍼 오버플로우
}
