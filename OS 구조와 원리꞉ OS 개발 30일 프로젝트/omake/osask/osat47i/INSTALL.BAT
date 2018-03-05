@ECHO OFF
ECHO OSASK를 디스크에 인스톨 합니다
ECHO A드라이브에 디스크를 넣어 Enter를 눌러 주세요
ECHO 인스톨을 중지할 경우는, Ctrl+C를 눌러 주세요
PAUSE
IMGTOL w -512 A: OSAIMGAT.BIN
ECHO 인스톨을 완료했습니다