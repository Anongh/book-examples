"src47"의 사용법의 설명                    2005/01/08 카와이 히데미
                                           2005/02/13 코야나기아명
1. 이것은 무엇인가?

  "src47"는, OSASK ver. 4.7의 「OSASK.EXE」를 생성하는데 필요한 모듈의 소스 프로그램 세트입니다.

2. make의 방법

  우선, tolset08를 준비합니다. 그렇다고 해도 이 문서를 쓰고 있는 현시점에서는 tolset08에는 아직 
들어 있지 않습니다. tolset08를 기다릴 수 없는 사람은 osa_dir2에 최신 툴을 넣고, golib00w.exe를 
golib00.exe로 rename하면 아마 tolset08에 상당하리라고 생각합니다. 민첩한 방법으로는,
http://www.afis.to/~mone/osask/osask_ml/200412/msg00018.html를 토대로 하는 것이 편할 것입니다.

 그런데, tolset08는 가능한 이 src47를 tolset디렉토리 안에 넣습니다. tolset의 밖에 두어서는 안됩니다. 
아니, 굳이 다른 곳에 두고 싶으면 그렇게 해도 괜찮습니다만, 그 경우에는 여기저기에 있는 Makefile내의 
TOOLPATH를 수정해야 합니다.

 그리고 나서, 사용하고 있는 OS에 대응하여 !cons_98.bat과 !cons_nt.bat를 실행해, 프롬프트가 나오면

prompt>make img_at

입력해 주세요. OSASK_AT.EXE, OSASK_AT.SYS, OSAIMGAT.BIN가 자동으로 생성됩니다.
「make img_tw」라면 TOWNS판, 「make img_ne」라면 NEC98판을 각각 할 수 있습니다.
「make img_qe」라면 qemu판, 「make img_vm」라면 VMware판, 「make img_bo」라면 Bochs판이 생깁니다.

EXE판만을 생성하는 경우는,
「make at」라면 PC/AT판, 「make tw」라면 TOWNS판, 「make ne」라면 NEC98판이,
「make qe」라면 qemu판, 「make vm」라면 VMware ver. 3판, 「make img_bo」라면 Bochs판이 생깁니다.

※Bochs는 1.3이후를 사용해 주세요. VMware는 version4.5이후에서는, PC/AT판의 바이너리를 그대로 
사용할 수 있습니다.

3. 테스트 실행의 방법

  prompt에서 「make run」라고 하면, OSASK의 qemu판을 자동으로 생성한 후에 실행합니다.

4. 커스터마이즈

  make_inc.txt를 편집하는 것으로, 윈도우 디자인이나 디폴트 반각 폰트 등을 변경할 수가 있습니다.

5. 저작권과 라이센스

  저작권에 대해서는, 해당하는 OSASK의 일반 공개판에 부속되는 COPYRIGHT.TXT를 참조해 주세요. 
  이 아카이브(archive)내의 모든 파일에 대해서 KL-01을 적용합니다.
