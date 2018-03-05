"go_0020s"에 대해 카와이 히데미  kawai@imasy.org

·이것은 무엇인가?

  이것은 go_0020 w/go_0020 o의 소스입니다. go_0020 w라고 하는 것은 gcc-3.2를 개조해서
만든 컴파일러의 win32판 바이너리입니다. go_0020 o는 그 OSASK판입니다.

  세세한 것을 말하자면, 이 go_0020 s는 gcc-3.2의 소스에 go_0020 p의 패치를 넣은
후의 소스입니다. go_0020 s만으로 모든 소스가 포함되어 있기 때문에 go_0020 s를 갖고
있으면 go_0020 p는 불필요합니다.

  (자매품의 통계)
    go_0020w : win32판 바이너리               (GPL)
    go_0020o : OSASK판 바이너리               (GPL)
    go_0020s : go_0020w, o의 소스           (GPL)
    go_0020p : go_0020 s와 gcc-3. 2 소스의 차이분 (KL-01)

·어떻게 실행 바이너리를 만들지?

  make에 앞서, include/rule.mak를 설정합니다. 우선 디렉토리 go_0020 s 안에서
setmode go로 하면, win32상의 GO로 make 할 수 있는 상태가 됩니다. 또 setmode gcc로
하면, win32의 API를 직접 호출하지는 않고 ANSI-C의 표준 라이브러리만을 사용하게 
됩니다. 이 상태라면 MinGW나 GO+w32clibc로 make 할 수 있는 상태가 됩니다. 어느것인가로 
해도 이 rule.mak는 양식에 지나지 않기 때문에, 각자의 환경에 맞추어 패스를 기입해 주세요
(w32clibc를 사용한다면, include/rule.mak를 조금 더 고쳐 쓸 필요가 있겠지요)

  go판의 경우, go_0009 w나 그 이후에 make 합니다. 기본적으로는 디렉토리 gcc안에서
"make"하면, cpp0.exe, cc1.exe, cc1plus.exe 3개가 생깁니다. 자세한 것은
Makefile를 해석해 주세요. 간단한 구조이기 때문에 읽을 수 있다고 생각합니다. 3개가 생긴 후
에 upx -9 *.exe로 하면 각각 적격인 사이즈가 됩니다.

  gcc판의 경우는, 우선 아래와 같이 에, golib00, gas2nask, naskcnv0, sjisconv의 stdc
판을 만듭니다(이러한 툴은 cpp0, cc1, cc1plus의 생성에 필요). 그리고 디렉토리
gcc안에서 "make"하면, cpp0.exe, cc1.exe, cc1plus.exe의 3개가 생깁니다.

  또, 디렉토리 gcc안에서"make -r osaskgo.bin"로 하면, OSASK판의 바이너리도
할 수 있습니다.

  golib00w, gas2nask, nask, sjisconv, naskcnv0에 대해서는, toolw32와 toolstdc의 양방에
들어 있습니다. setmode go의 경우는, toolw32안에서 make하면, 각각 생성됩니다.
setmode gcc의 경우는, toolstdc 안에서 make합니다. 각각 upx하면, 배포
판 바이너리가 됩니다.

·라이센스에 대해

  이 소스 세트는 GPL로 허가합니다. 만약, GPL는 아니고 KL-01으로 라이센스된
소스를 원하는 사람은 go_0020 p 쪽을 사용해 주세요.

  또한 이하의 디렉토리에 관해서는, 예외적으로 KL-01(川合堂라이센스-01)입니다.
    drv_osa, drv_stdc, drv_w32, funcs, go_lib, nasklib, omake, toolstdc,
    toolw32, w32clibc
 
  GPL의 라이센스문은 Copying에, KL-01의 라이센스문은 이하의 URL에 있습니다.
        http://www.imasy.org/~mone/kawaido/license01-1. 0. html

  GPL 부분의 저작권에 대해서는 각각의 전 저작자에게 돌아갑니다. KL-01 부분의 저작권에
대해서, go_lib내의 string계 함수군의 저작권은 Gaku씨에게 그 이외는 川合에게 돌아갑니다.

·libmingw.lib에 대해

  libmingw는, MinGW의 라이브러리로부터 실행 바이너리 생성에 아무래도 필요한 것만을
모은 간이 라이브러리입니다. 이 라이센스는 당연히 GPL입니다.

·사례

  gcc의 개발자의 여러분에게 진심으로의 감사의 말씀을 드립니다. 만약 이 go의 일부가 gcc의 개발
자 분들에게 인정을 받을 수 있다면 더 없이 기쁘겠습니다.

  Gaku씨의 string 라이브러리를 사용했습니다. Gaku씨, 감사합니다.

  또 개발을 직접 도와 준 구민씨, henoheno씨, 그리고 OSASK 커뮤니티 여러분,
감사합니다.

  덧붙여 카와이의 2002.10. 03~2003.02. 28의 기간을 둘 수 있는 개발 성과는, 특별 인가 법인 정보
처리 진흥 사업 협회 (IPA)의 「未踏 소프트웨어 창조 사업 未踏 유스」에서의 위탁업무에 의한
것입니다. 이 장소를 빌려 지원을 받을 수 있었던 것에 대해 감사의 말씀드립니다.

·어디가 바뀌었는지?

  기능적인 차이에 대해서는, 바이너리판을 참조해 주세요. 여기에서는 소스 레벨
에서의 차이만 언급하고 있습니다.

< go_0000s → go_0001s >

  include 디렉토리내의 2개의 파일을 삭제했습니다. 이것은 make에 불필요한 파일
이였습니다. 소스 내의 개행 코드가 CRLF와 LF만의 것이 혼재하고 있었습니다. LF만
으로 통일했습니다. delaln32의 바이너리와 소스도 일단 넣어 두기로 했습니다.

< go_0001s → go_0002s >

  드라이버(drv_stdc) 내의 파일 이외의 모두는 외부의 인클루드 파일을 필요로 
하지 않습니다. 자신이 가지고 있는 라이브러리에서만 움직이고 있습니다. 필요한 것은 이하의 5함수
main0, GOL_open, GOL_close, GOL_stepdir, GOL_sysabort
뿐입니다. 이것들은 환경에 의존하여 작성되는 것입니다. 이것들은 물론 표준 라이브러리만으로
기술할 수가 있고, 그것은 drv_stdc에 들어가 있습니다. 이것은,
malloc, fopen, fseek, ftell, fread, fclose, fwrite, fputs
만으로 쓰여져 있습니다.
  드라이버와 callmain.c를 제외한 모든 파일은 go_0002 w로 컴파일 할 수 있습니다.
환경에 의존하는 드라이버와 callmain.c는 MinGW로 컴파일 할 필요가 있습니다.
  자세한 사항은 Makefile를 참조해 주세요.

< go_0002s → go_0004s >

  stdc드라이버 외에 w32드라이버를 준비했습니다. 이것은 MinGW를 가지고 있지 않아도
컴파일 할 수 있습니다. 또 stdc판보다 컴팩트합니다.
  golib00w를 만들어서 소스를 넣었습니다.
  Makefile를 go_0004 w 밖에 사용하지 않도록 수정하였습니다.
  libmingw가 필요하게 되어 추가하였습니다.

< go_0004s → go_0005s >

  일반 공개를 향해 문서를 조금 고쳤습니다. 그것 뿐입니다.

< go_0005s → go_0006s >

  gas2nask와 nask를 추가해 Makefile를 고쳐 써 cc1와 cc1plus를 조금 개조하였습니다.
(필요한 align 명령을 명시적으로 출력합니다).

< go_0006s → go_0007s >

  sjisconv과 naskcnv0를 추가하고, drv_osa를 추가해 nask를 OSASK판과 소스를 공용할 수 있는 
형식으로 새롭게 cc1와 cc1plus의 섹션 주위의 버그를 수정하고, golib00w로 긴 파일명에서의
버그를 고쳤습니다.

< go_0007s → go_0008p >

  [OSASK 5584]로 go_0007의 NASK에 바이너리(binary) 모드시의 버그가 발견되었으므로 수정하였습니다.

< go_0008p → go_0009s >

  osaskgo.bin를 만드는 경우에 한해서 ASKA가 필요하게 되었습니다. 또 drv_osa안의 osamain.c안의 
함수 refresh_static()는 osaskgo.map의 기술에 맞추어 수정하지 않으면 안되는 부분이 있으므로 
주의가 필요합니다.

< go_0009s → go_0011s >

  go_0010 p에서의 naskcnv0의 수정을 수중에 넣어, sjisconv를 osaskgo에도 넣은 정도입니다.

< go_0011s → go_0012s >

  golib00의 stdc판(golib00s)을 추가했습니다. 그리고 drv_stdc를 온전히 하고, cc1 등을
Linux등에서도 make할 수 있도록 했습니다.

< go_0012s → go_0013s >

  naskcnv0의 stdc판(nskcn0bs)을 추가했습니다. go_0012의 버그도 수정했습니다.

< go_0013s → go_0014s >

  gas2nask의 stdc판(gas2nsks)을 추가했습니다.

< go_0014s → go_0015s >

  nask의 stdc판을 추가했습니다.

< go_0015s → go_0017s >

  gas2nask의 fsubrs 버그를 수정했습니다.

< go_0017s → go_0018s >

  gas2nask의 버그를 수정했습니다(fistps, fists, filds, stosl, fabs, fucomp,
fistps, fdivs). win32-console용의 간이 라이브러리를 추가했습니다.

< go_0018s → go_0019s >

  번들 라이브러리의 헤더 파일을 잘못했을 뿐이므로, go_0018 s와
go_0019 s의 차이는 이 문서 뿐입니다.

< go_0019 → go_0020 >

  소스 레벨에서의 개정을 하고 go판과 gcc(stdc)판의 차이를 include/rule.mak인 만큼으로
했습니다.
  stdc판에서는 이용하고 있는 함수를 이전의 12개에서 8개로 한층 더 감소시켰습니다.
    fopen, fclose, fread, fwrite, fseek, ftell, malloc, exit
혹은, 이하의 6함수를 만드는 것만으로도 좋습니다.
  GOLD_getsize(파일 사이즈 취득),
  GOLD_read(파일의 바이너리 모드 리드 -- 일괄로 파일 전체를 읽을 뿐),
  GOLD_write_b(파일의 바이너리 모드 라이트 -- 일괄로 파일 전체를 쓸 뿐),
  GOLD_write_t(파일의 텍스트 모드 라이트 -- 파일을 쓸 때는 일괄,
                  콘솔에의 메세지 출력의 경우는 몇번정도 call될 수 있다),
  GOLD_exit(exit와 동일),
  malloc(malloc와 같다)
malloc에 대해서는 각각의 main() 안에서 최초로 1번 call하고 있을 뿐이므로, malloc으로
만들지 않아도 대체 가능합니다.
  OS의존 드라이버를, 완전 공통화했습니다. nask나 gas2nask나 sjisconv등의 디렉토리를
정리하여 개정하기 쉽게 했습니다. 이 2점에 의해 기능이 늘어 난 것에 관계없이
소스는 25KB이상 작아져 파일수는 3개 줄고, 디렉토리도 7개 감소하고 있습니다.


·문의는?

  OSASK-ML인가, OSASK 게시판에 오세요.

               2003.11. 12  카와이 히데미  kawai@imasy.org
