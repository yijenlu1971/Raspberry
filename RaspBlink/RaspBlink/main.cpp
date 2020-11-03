#include <stdio.h>
#include <string.h>
#include <wiringPi.h>

extern "C"
{
#include "LCM1602.h"
}

// LED Pin - wiringPi pin 0 為 BCM_GPIO 17。
// 我們必須在以 wiringPiSetupSys 初始化時使用 BCM 編號方式
// 選擇不同 pin 編號時，請使用 BCM 編號方式，並
// 更新 [屬性頁] - [建置事件] - [Remote Post-Build Event] 命令 
// 其對 wiringPiSetupSys 的設定使用 gpio export
#define	LED	17

int main(void)
{
	char msg[17];
	int	i = 0;

	wiringPiSetupSys();

	pinMode(LED, OUTPUT);
	int fd = lcm1602Setup(0x27, CURSOR_LEFT, SHIFT_OFF, DISPLAY_ON, CURSOR_OFF,
				CURSOR_BLINK_OFF, TWO_LINE, SMALL_FONT, LCD_BACKLIGHT_OFF);

	while (true)
	{
		digitalWrite(LED, HIGH);  // 開啟

		memset(msg, 0, sizeof(msg));
		sprintf(msg, "Loop  %10d", i++);
		lcm1602WriteLine(fd, msg, 0, LCD_BACKLIGHT_ON);

		delay(500); // 毫秒
		digitalWrite(LED, LOW);	  // 關閉

		memset(msg, 0, sizeof(msg));
		sprintf(msg, "Loop  %10d", i++);
		lcm1602WriteLine(fd, msg, 1, LCD_BACKLIGHT_ON);

		delay(500);

//		if ((i % 10) == 0) printf("Hello Raspberry! count=%d\n", i);
	}
	return 0;
}