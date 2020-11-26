#include <wiringPi.h>

// LED Pin - wiringPi pin 0 為 BCM_GPIO 17。
// 我們必須在以 wiringPiSetupSys 初始化時使用 BCM 編號方式
// 選擇不同 pin 編號時，請使用 BCM 編號方式，並
// 更新 [屬性頁] - [建置事件] - [Remote Post-Build Event] 命令 
// 其對 wiringPiSetupSys 的設定使用 gpio export
#define	LED	17

extern void ModbusTCPClient();

int main(void)
{
	wiringPiSetupSys();

	pinMode(LED, OUTPUT);
	ModbusTCPClient();

	while (true)
	{
		digitalWrite(LED, HIGH);  // 開啟
		delay(500); // 毫秒
		digitalWrite(LED, LOW);	  // 關閉
		delay(500);
	}
	return 0;
}