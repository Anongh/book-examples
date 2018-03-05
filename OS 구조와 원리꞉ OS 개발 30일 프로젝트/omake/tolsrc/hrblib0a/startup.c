void HariMain(void);

void HariStartup(void)
{
	/* 장래 HariMain의 실행에 앞서 무엇인가 하고 싶어지면, 여기에 추가한다 */

	HariMain();

	/* 장래 HariMain의 종료후에 무엇인가 처리를 시키고 싶어지면, 여기에 추가한다 */

	return;
}
