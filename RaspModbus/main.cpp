#include <wiringPi.h>

// LED Pin - wiringPi pin 0 �� BCM_GPIO 17�C
// �ڭ̥����b�H wiringPiSetupSys ��l�Ʈɨϥ� BCM �s���覡
// ��ܤ��P pin �s���ɡA�Шϥ� BCM �s���覡�A��
// ��s [�ݩʭ�] - [�ظm�ƥ�] - [Remote Post-Build Event] �R�O 
// ��� wiringPiSetupSys ���]�w�ϥ� gpio export
#define	LED	17

extern void ModbusTCPClient();

int main(void)
{
	wiringPiSetupSys();

	pinMode(LED, OUTPUT);
	ModbusTCPClient();

	while (true)
	{
		digitalWrite(LED, HIGH);  // �}��
		delay(500); // �@��
		digitalWrite(LED, LOW);	  // ����
		delay(500);
	}
	return 0;
}