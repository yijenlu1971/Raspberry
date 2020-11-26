#ifndef __MODBUS_H
#define __MODBUS_H

// MODBUS definition
#define CODE_READ_DO		0x01
#define CODE_READ_DI		0x02
#define CODE_READ_AO		0x03
#define CODE_READ_AI		0x04
#define CODE_WRITE_DO		0x05
#define CODE_WRITE_AO		0x06
#define CODE_READ_STATUS	0x07
#define CODE_WRITE_DOS		0x0F
#define CODE_WRITE_AOS		0x10
#define CODE_SLAVE_ID		0x11
#define CODE_MASK_AO		0x16
#define CODE_RW_AOS			0x17
#define CODE_READ_FIFO		0x18

#define	MAX_TCP_BUFSIZE		260

typedef struct
{
	uint8_t		devID;
	uint8_t		sendBuf[MAX_TCP_BUFSIZE];
	uint8_t		recvBuf[MAX_TCP_BUFSIZE*2];
} MBUS_CLIENT;

extern uint16_t ModbusDataProcess(MBUS_CLIENT *pCtx);
extern uint8_t SendException(uint8_t* pSndBuf, uint8_t func, uint8_t code);
extern uint16_t crc_chk(uint8_t* data, uint16_t length);

#endif
