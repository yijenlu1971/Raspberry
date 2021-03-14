#include <wiringPi.h>
#include "TFT.h"

// LED Pin - wiringPi pin 0 �� BCM_GPIO 17�C
// �ڭ̥����b�H wiringPiSetupSys ��l�Ʈɨϥ� BCM �s���覡
// ��ܤ��P pin �s���ɡA�Шϥ� BCM �s���覡�A��
// ��s [�ݩʭ�] - [�ظm�ƥ�] - [Remote Post-Build Event] �R�O 
// ��� wiringPiSetupSys ���]�w�ϥ� gpio export
#define	LED	18

int main(void)
{
	const char str[] = "HAN YA\0";

	wiringPiSetupSys();

	pinMode(LED, OUTPUT);
	TFT* tftLCD = new TFT(17, 25, 24);

	tftLCD->begin();
	tftLCD->background(0, 0, 0);
	tftLCD->stroke(255, 255, 255);
	tftLCD->line(0, tftLCD->height()/2, tftLCD->width(), tftLCD->height() / 2);
	tftLCD->line(5, 0, 5, tftLCD->height());

	tftLCD->fill(0, 255, 0);
	tftLCD->circle(20, 20, 10);

	tftLCD->fill(255, 0, 0);
	tftLCD->rect(70, 100, 30, 20);

	tftLCD->setTextSize(2);
	tftLCD->text(str, 50, 20);

	while (true)
	{
		digitalWrite(LED, HIGH);  // �}��
		delay(500); // �@��
		digitalWrite(LED, LOW);	  // ����
		delay(500);
	}
	return 0;
}