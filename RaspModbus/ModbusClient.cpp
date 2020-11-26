#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <sys/socket.h> 
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h> 
#include <netinet/tcp.h>
#include "modbus.h"

MBUS_CLIENT		mbusCtx;

void* ModbusListener(void* threadid)
{
    long tid = (long)threadid;
    int server_fd, new_socket = -1;
    struct sockaddr_in addr;
    int opt = 1;

	mbusCtx.devID = 1;

    // Creating socket file descriptor 
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        printf("ModbusListener: socket failed\n");
        goto EXIT;
    }
    printf("ModbusListener Thread ID %d, Listening socket=%d\n", tid, server_fd);

    // Forcefully attaching socket to the port 502 
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        printf("ModbusListener: setsockopt error!\n");
        goto EXIT;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(5020);

    // Forcefully attaching socket to the port 502 
    if (bind(server_fd, (struct sockaddr*) &addr, sizeof(addr)) < 0)
    {
        printf("ModbusListener: bind(%d) failed!\n", errno);
        goto EXIT;
    } 

    while (1)
    {
        if (listen(server_fd, 1) < 0)
        {
            printf("ModbusListener: listen error(%d)!\n", errno);
        }
        else
        {
            int addrlen = sizeof(addr);

            if ((new_socket = accept(server_fd, (struct sockaddr*) &addr, (socklen_t*)&addrlen)) < 0)
            {
                printf("ModbusListener: accept error(%d)!\n", errno);
            }
            else
            {
                char ipStr[20];
                int nfds, rc, len;
                fd_set readfds;

                inet_ntop(AF_INET, &addr.sin_addr, ipStr, 20);
                printf("ModbusListener: IP:%s:%d connected. Socket=%d\n", ipStr, ntohs(addr.sin_port), new_socket);

                //////////////////////////////////////
                // Linux: No TCP KeepAlive in default.
                opt = 1;
                if (setsockopt(new_socket, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(opt)))
                    printf("ModbusListener: setsockopt SO_KEEPALIVE error(%d)!\n", errno);

                opt = 20;
                if (setsockopt(new_socket, SOL_TCP, TCP_KEEPIDLE, &opt, sizeof(opt)))
                    printf("ModbusListener: setsockopt TCP_KEEPIDLE error(%d)!\n", errno);

                opt = 1;
                if (setsockopt(new_socket, SOL_TCP, TCP_KEEPINTVL, &opt, sizeof(opt)))
                    printf("ModbusListener: setsockopt TCP_KEEPINTVL error(%d)!\n", errno);
                /////////////////////////////////////

                while (1)
                {
                    FD_ZERO(&readfds);
                    FD_SET(new_socket, &readfds);
                    nfds = new_socket + 1;

                    rc = select(nfds, &readfds, NULL, NULL, NULL);
                    if (rc == -1)
                    {
                        printf("ModbusListener: select error(%d).\n", errno);
                        break;
                    }
                    else if (rc == 0)
                    {
                        // time out
                        printf("ModbusListener: select timeout.\n");
                    }
                    else if (FD_ISSET(new_socket, &readfds))
                    {
                        len = recv(new_socket, mbusCtx.recvBuf, sizeof(mbusCtx.recvBuf), 0);
                        if (len < 0)
                        {
                            printf("ModbusListener: recv error(%d).\n", errno);
                            break;
                        }
                        else if (len == 0)
                        {
                            new_socket = -1;
                            printf("ModbusListener: socket disconnect.\n");
                            break;
                        }
                        else
                        {
                            printf("RecLen=%d\n", len);
                            if (len > 7)
                            {
								len = ModbusDataProcess( &mbusCtx );
								if (len > 0)
								{
									if (send(new_socket, mbusCtx.sendBuf, len, 0) < 0)
									{
										printf("ModbusListener: send error(%d).\n", errno);
										break;
									}
								}
                            }
                        }
                    }
                }

                if (new_socket >= 0) shutdown(new_socket, SHUT_RDWR);
                new_socket = -1;
            }
        }

        sleep(1);
    }

EXIT:
    if (server_fd > 0)
    {
        shutdown(server_fd, SHUT_RDWR);
    }

    pthread_exit(NULL);
}

void ModbusTCPClient()
{
	int	i = 0, rc;
	pthread_t threads;

	rc = pthread_create(&threads, NULL, ModbusListener, (void*)i);

	if (rc) {
		printf("Error:unable to create thread %d\n", rc);
	}
	//pthread_exit(NULL);
}

uint16_t ModbusDataProcess(MBUS_CLIENT* pCtx)
{
	uint8_t* pRecvBuf = pCtx->recvBuf;
	uint8_t * pSendBuf = pCtx->sendBuf;
	uint16_t Len = 0, StAddr, amount;
	uint8_t	Code = pRecvBuf[7];

	memcpy(pSendBuf, pRecvBuf, 7);

    if (pRecvBuf[6] == pCtx->devID && (Code > 0 && Code < 0x30))
    {
        StAddr = (((uint16_t)pRecvBuf[8]) << 8) | pRecvBuf[9];
		amount = (((uint16_t)pRecvBuf[10]) << 8) | pRecvBuf[11];

        /*if (Code == CODE_READ_DO)	Len = ReadCoil(&pSendBuf[6], StAddr, amount);
        else if (Code == CODE_READ_DI)	Len = ReadDiscIn(&pSendBuf[6], StAddr, amount);
        else if (Code == CODE_READ_AI)	Len = ReadInReg(&pSendBuf[6], StAddr, amount);
        else if (Code == CODE_READ_AO) Len = ReadHoldReg(&pSendBuf[6], StAddr, amount);
        else if (Code == CODE_WRITE_DO) Len = WritSingleCoil(&pSendBuf[6], StAddr, amount == 0 ? 0 : 1);
        else if (Code == CODE_WRITE_AO) Len = WritSingleReg(&pSendBuf[6], StAddr, amount);
        else if (Code == CODE_WRITE_DOS) Len = WritMultiCoils(&pSendBuf[6], StAddr, amount, &pRecvBuf[12]);
        else if (Code == CODE_WRITE_AOS) Len = WritMultipleReg(&pSendBuf[6], StAddr, amount, &pRecvBuf[12]);
        else*/
        {
            Len = SendException(&pSendBuf[6], Code, 0x01);
        }

		pSendBuf[4] = (uint8_t) (Len >> 8);
		pSendBuf[5] = (uint8_t) (Len & 0xFF);
		Len += 6;
    }

	return Len;
}

uint8_t SendException(uint8_t* pSndBuf, uint8_t func, uint8_t code)
{
	pSndBuf[1] = func + 0x80;
	pSndBuf[2] = code;
	return 3;
}

#if 0
// Read signle-bit (read-only)
uint8_t ReadDiscIn(u8* pSndBuf, u16 Addr, u16 Len)
{
	uint8_t	val, bit;
	uint16_t	i, idx;
	uint32_t	rc;

	if ((Len == 0) || (Len > 672))
		return SendException(pSndBuf, CODE_READ_DI, 0x03);
	else if ((Addr + Len) > 672)
		return SendException(pSndBuf, CODE_READ_DI, 0x02);

	pSndBuf[1] = CODE_READ_DI;
	pSndBuf[2] = (Len & 0x07) ? (Len >> 3) + 1 : (Len >> 3);

	idx = 3;
	val = bit = 0;
	for (i = Addr; i < Addr + Len; i++)
	{
		if (i < 512) QueryInputVal(i + VAR_DI_ID, &rc);
		else if (i < 544) QueryInputVal(i - 512 + VAR_WF_ID, &rc);
		else if (i < 560) QueryInputVal(i - 544 + VAR_NET_ID, &rc);
		else if (i < 640) QueryInputVal(i - 560 + VAR_DTF_ID, &rc);
		else QueryInputVal(i - 640 + VAR_DWF_ID, &rc);

		val |= (rc << bit);
		if (++bit == 8)
		{
			pSndBuf[idx++] = val;
			val = bit = 0;
		}
	}

	if (bit != 0) pSndBuf[idx++] = val;
	return idx;
}

// Read signle-bit (read-write)
uint8_t ReadCoil(u8* pSndBuf, u16 Addr, u16 Len)
{
	uint8_t	val, bit;
	uint16_t	i, idx;
	uint32_t	rc;

	if ((Len == 0) || (Len > 1088))
		return SendException(pSndBuf, CODE_READ_DO, 0x03);
	else if ((Addr + Len) > 1088)
		return SendException(pSndBuf, CODE_READ_DO, 0x02);

	pSndBuf[1] = CODE_READ_DO;
	pSndBuf[2] = (Len & 0x07) ? (Len >> 3) + 1 : (Len >> 3);

	idx = 3;
	val = bit = 0;
	for (i = Addr; i < Addr + Len; i++)
	{
		if (i < 512) QueryInputVal(i + VAR_DO_ID, &rc);
		else if (i < 1024) QueryInputVal(i - 512 + VAR_IDIO_ID, &rc);
#ifdef __MOTOR__
		else if (i < 1088) QueryInputVal(i - 1024 + VAR_AXSERVO_ID, &rc);
#endif

		val |= (rc << bit);
		if (++bit == 8)
		{
			pSndBuf[idx++] = val;
			val = bit = 0;
		}
	}

	if (bit != 0) pSndBuf[idx++] = val;
	return idx;
}

// Write signle-bit (read-write)
uint8_t WritSingleCoil(u8* pSndBuf, u16 Addr, u8 bOn)
{
	uint16_t	temp;

	if (Addr > 1088)
		return SendException(pSndBuf, CODE_WRITE_DO, 0x02);

	pSndBuf[1] = CODE_WRITE_DO;
	pSndBuf[2] = (Addr >> 8) & 0xFF;
	pSndBuf[3] = Addr & 0xFF;
	pSndBuf[4] = bOn ? 0xFF : 0;
	pSndBuf[5] = 0;

	if (Addr < 512) SetDoBit(Addr, bOn);
	else if (Addr < 1024)
	{
		Addr -= 512;
		temp = Addr >> 3;	Addr = Addr - (temp * 8);
		if (bOn)	hmiSvCtx.m[temp] |= (0x01 << Addr);
		else		hmiSvCtx.m[temp] &= ~(0x01 << Addr);
	}
#ifdef __MOTOR__
	else
	{
		Addr -= 1024;
		SetServo(Addr, bOn);

		temp = Addr >> 3;	Addr = Addr - (temp * 8);
		if (bOn)	hmiSvCtx.axServo[temp] |= (0x01 << Addr);
		else		hmiSvCtx.axServo[temp] &= ~(0x01 << Addr);
	}
#endif

	return 6;
}

uint8_t WritMultiCoils(u8* pSndBuf, u16 Addr, u16 Len, u8* pBuf)
{
	uint8_t		j, ByteCnt, bOn;
	uint16_t	i, temp, val, idx;

	ByteCnt = (Len & 0x07) ? (Len >> 3) + 1 : (Len >> 3);

	if ((Len == 0) || (Len > 1088) || (pBuf[0] != ByteCnt))
		return SendException(pSndBuf, CODE_WRITE_DOS, 0x03);
	else if ((Addr + Len) > 1088)
		return SendException(pSndBuf, CODE_WRITE_DOS, 0x02);

	pSndBuf[1] = CODE_WRITE_DOS;
	pSndBuf[2] = (Addr >> 8) & 0xFF;
	pSndBuf[3] = Addr & 0xFF;
	pSndBuf[4] = (Len >> 8) & 0xFF;
	pSndBuf[5] = Len & 0xFF;

	for (i = 1; i <= ByteCnt; i += 2)
	{
		val = (pBuf[i + 1] << 8) | pBuf[i];

		for (j = 0; j < 16; j++)
		{
			if (Len > 0)
			{
				bOn = (val >> j) & 0x01;
				if (Addr < 512) SetDoBit(Addr, bOn);
				else if (Addr < 1024)
				{
					idx = Addr - 512;
					temp = idx >> 3;	idx = idx - (temp * 8);
					if (bOn)	hmiSvCtx.m[temp] |= (0x01 << idx);
					else		hmiSvCtx.m[temp] &= ~(0x01 << idx);
				}
#ifdef __MOTOR__
				else
				{
					Addr -= 1024;
					SetServo(Addr, bOn);

					temp = idx >> 3;	idx = idx - (temp * 8);
					if (bOn)	hmiSvCtx.axServo[temp] |= (0x01 << idx);
					else		hmiSvCtx.axServo[temp] &= ~(0x01 << idx);
				}
#endif

				Addr++;
				Len--;
			}
		}

		if (Len == 0) break;
	}

	return 6;
}

// Read bytes (read-only)
uint8_t ReadInReg(u8* pSndBuf, u16 Addr, u16 Len)
{
	uint16_t	i;
	uint32_t	val;

	if ((Len == 0) || (Len > 125))
		return SendException(pSndBuf, CODE_READ_AI, 0x03);
	else if ((Addr + Len) > 1472)
		return SendException(pSndBuf, CODE_READ_AI, 0x02);

	pSndBuf[1] = CODE_READ_AI;
	pSndBuf[2] = Len * 2;

	for (i = Addr; i < Addr + Len; i++)
	{
#ifdef __OMRON__
		if (i < MAP_SIZE) val = transMap[i].value;
#else
		if (i < 256) QueryInputVal(i + VAR_AI_ID, &val);
		else if (i < 1024) QueryInputVal(i - 256 + VAR_X_ID, &val);
#ifdef __MOTOR__
		else if (i < 1344) QueryInputVal(i - 1024 + VAR_AXERR_ID, &val);
#endif
#endif
		else
			break;

		pSndBuf[3 + 2 * (i - Addr)] = (val >> 8) & 0xFF;
		pSndBuf[4 + 2 * (i - Addr)] = val & 0xFF;
	}

#ifdef __MOTOR__
	if (i >= 1344 && i < 1472)
	{
		if (i >= 1344 && i < 1408)
			QueryInputVal(i - 1344 + VAR_AXNOW_ID, &val);
		else
			QueryInputVal(i - 1408 + VAR_AXSTS_ID, &val);

		pSndBuf[3 + 2 * (i - Addr)] = (val >> 8) & 0xFF;
		pSndBuf[4 + 2 * (i - Addr)] = val & 0xFF;
		pSndBuf[5 + 2 * (i - Addr)] = (val >> 24) & 0xFF;
		pSndBuf[6 + 2 * (i - Addr)] = (val >> 16) & 0xFF;
	}
#endif

	return 3 + 2 * Len;
}

uint8_t ReadHoldReg(u8* pSndBuf, u16 Addr, u16 Len)
{
	uint16_t	i;
	uint32_t	val;

	if ((Len == 0) || (Len > 125))
		return SendException(pSndBuf, CODE_READ_AO, 0x03);
	else if ((Addr + Len) >= 1074)
		return SendException(pSndBuf, CODE_READ_AO, 0x02);

	pSndBuf[1] = CODE_READ_AO;
	pSndBuf[2] = Len * 2;

	for (i = Addr; i < Addr + Len; i++)
	{
#ifdef __OMRON__
		if (i < MAP_SIZE) val = transMap[i].reg;
		else if (Addr == 250 - 1) val = bReboot ? TRUE : FALSE;
		else if (i >= 400 && i < (400 + MAP_SIZE)) val = transMap[i - 400].addr;
#else
		if (i < 128) QueryInputVal(i - 1 + VAR_AO_ID, &val);
		else if (i < 144) QueryInputVal(i - 128 + VAR_CNT_ID, &val);
		else if (i < 152) QueryInputVal(i - 144 + VAR_TMR_ID, &val);
		else if (i < 184) QueryInputVal(i - 152 + VAR_IAIO_ID, &val);
		else if (i < 192) QueryInputVal(i - 184 + VAR_HTMR_ID, &val);
		else if (i < 256) QueryInputVal(i - 192 + VAR_BYTE_ID, &val);

#ifdef __MOTOR__
		else if (i < 320) QueryInputVal(i - 256 + VAR_AXIS_ID, &val);
		else if (i < 384) QueryInputVal(i - 320 + VAR_AXCMD_ID, &val);
		else if (i < 448) QueryInputVal(i - 384 + VAR_AXPOS_ID, &val);
		else if (i < 512) QueryInputVal(i - 448 + VAR_AXSPD_ID, &val);
		else if (i < 576) QueryInputVal(i - 512 + VAR_AXSET_ID, &val);
#endif
#endif
		else
			break;

		pSndBuf[3 + 2 * (i - Addr)] = (val >> 8) & 0xFF;
		pSndBuf[4 + 2 * (i - Addr)] = val & 0xFF;
	}

	if (i >= 704 && i < 1072)
	{
		if (i >= 704 && i < 784) QueryInputVal(i - 704 + VAR_DT_ID, &val);
		else if (i >= 784 && i < 816) QueryInputVal(i - 784 + VAR_DW_ID, &val);

#ifdef __MOTOR__
		else if (i >= 816 && i < 880) QueryInputVal(i - 816 + VAR_AXMOV_ID, &val);
		else if (i >= 880 && i < 944) QueryInputVal(i - 880 + VAR_AXPARA_ID, &val);
		else if (i >= 944 && i < 1008) QueryInputVal(i - 944 + VAR_AXPARB_ID, &val);
		else QueryInputVal(i - 1008 + VAR_AXSYS_ID, &val);
#endif

		pSndBuf[3 + 2 * (i - Addr)] = (val >> 8) & 0xFF;
		pSndBuf[4 + 2 * (i - Addr)] = val & 0xFF;
		pSndBuf[5 + 2 * (i - Addr)] = (val >> 24) & 0xFF;
		pSndBuf[6 + 2 * (i - Addr)] = (val >> 16) & 0xFF;
	}

	return 3 + 2 * Len;
}

uint8_t WritSingleReg(u8* pSndBuf, u16 Addr, u16 val)
{
	if (Addr > 704)
		return SendException(pSndBuf, CODE_WRITE_AO, 0x02);

	pSndBuf[1] = CODE_WRITE_AO;
	pSndBuf[2] = (Addr >> 8) & 0xFF;
	pSndBuf[3] = Addr & 0xFF;
	pSndBuf[4] = (val >> 8) & 0xFF;
	pSndBuf[5] = val & 0xFF;

#ifdef __OMRON__
	taskENTER_CRITICAL();
	if (Addr < MAP_SIZE) transMap[Addr].reg = val;
	else if (Addr == 250 - 1) bReboot = val ? TRUE : FALSE;
	else if (Addr >= 400 && Addr < (400 + MAP_SIZE)) transMap[Addr - 400].addr = val;
	taskEXIT_CRITICAL();
#else
	if (Addr < 128) SetAoVal(Addr - 1, val);
	else if (Addr < 144) hmiSvCtx.counter[Addr - 128] = val;
	else if (Addr < 152) hmiSvCtx.timer[Addr - 144] = val;
	else if (Addr < 184) hmiSvCtx.w[Addr - 152] = val;
	else if (Addr < 192) hmiSvCtx.htimer[Addr - 184] = val;
	else if (Addr < 256) hmiSvCtx.byte[Addr - 192] = (u8)val;

#ifdef __MOTOR__
	else if (Addr < 320) hmiSvCtx.axis[Addr - 256] = (u8)val;
	else if (Addr < 384) hmiSvCtx.axCmd[Addr - 320] = (u8)val;
	else if (Addr < 448) hmiSvCtx.axPos[Addr - 384] = (u8)val;
	else if (Addr < 512) hmiSvCtx.axSpd[Addr - 448] = (u8)val;
	else if (Addr < 576) hmiSvCtx.axSet[Addr - 512] = (u8)val;
#endif
#endif
	return 6;
}

uint8_t WritMultipleReg(u8* pSndBuf, u16 Addr, u16 Len, u8* pBuf)
{
	uint32_t	val;

	if (Addr <= 704)
	{
		if ((Len == 0) || (Len > 1) || (pBuf[0] != 2))
			return SendException(pSndBuf, CODE_WRITE_AOS, 0x03);

		val = (pBuf[1] << 8) | pBuf[2];

#ifdef __OMRON__
		taskENTER_CRITICAL();
		if (Addr < MAP_SIZE) transMap[Addr].reg = val;
		else if (Addr == 250 - 1) bReboot = val ? TRUE : FALSE;
		else if (Addr >= 400 && Addr < (400 + MAP_SIZE)) transMap[Addr - 400].addr = val;
		taskEXIT_CRITICAL();
#else
		if (Addr < 128) SetAoVal(Addr - 1, val);
		else if (Addr < 144) hmiSvCtx.counter[Addr - 128] = val;
		else if (Addr < 152) hmiSvCtx.timer[Addr - 144] = val;
		else if (Addr < 184) hmiSvCtx.w[Addr - 152] = val;
		else if (Addr < 192) hmiSvCtx.htimer[Addr - 184] = val;
		else if (Addr < 256) hmiSvCtx.byte[Addr - 192] = (u8)val;

#ifdef __MOTOR__
		else if (Addr < 320) hmiSvCtx.axis[Addr - 256] = (u8)val;
		else if (Addr < 384) hmiSvCtx.axCmd[Addr - 320] = (u8)val;
		else if (Addr < 448) hmiSvCtx.axPos[Addr - 384] = (u8)val;
		else if (Addr < 512) hmiSvCtx.axSpd[Addr - 448] = (u8)val;
		else if (Addr < 576) hmiSvCtx.axSet[Addr - 512] = (u8)val;
#endif
#endif
	}
	else
	{
		if ((Len == 0) || (Len > 2) || (pBuf[0] != 4))
			return SendException(pSndBuf, CODE_WRITE_AOS, 0x03);
		else if (Addr >= 1072)
			return SendException(pSndBuf, CODE_WRITE_AOS, 0x02);

		val = (pBuf[3] << 24) | (pBuf[4] << 16) | (pBuf[1] << 8) | pBuf[2];

		if (Addr < 784) hmiSvCtx.dt[Addr - 704] = val;
		else if (Addr < 816)
		{
			hmiSvCtx.dw[Addr - 784] = val;
			if ((Addr - 784) >= DW_RTC_BASE) SetDataToRTC(Addr - 784 - DW_RTC_BASE, val);
		}
#ifdef __MOTOR__
		else if (Addr < 880) hmiSvCtx.axMov[Addr - 816] = val;
		else if (Addr < 944) hmiSvCtx.axParA[Addr - 880] = val;
		else if (Addr < 1008) hmiSvCtx.axParB[Addr - 944] = val;
		else if (Addr < 1072) hmiSvCtx.axSys[Addr - 1008] = val;
#endif
	}

	pSndBuf[1] = CODE_WRITE_AOS;
	pSndBuf[2] = (Addr >> 8) & 0xFF;
	pSndBuf[3] = Addr & 0xFF;
	pSndBuf[4] = (Len >> 8) & 0xFF;
	pSndBuf[5] = Len & 0xFF;

	return 6;
}

#endif

uint16_t crc_chk(uint8_t* data, uint16_t length)
{
	uint8_t		i;
	uint16_t	reg_crc = 0xFFFF;

	while (length--)
	{
		reg_crc ^= *data++;
		for (i = 0; i < 8; i++)
		{
			if (reg_crc & 0x01) /* LSB(b0)=1 */
				reg_crc = (reg_crc >> 1) ^ 0xA001;
			else
				reg_crc = reg_crc >> 1;
		}
	}

	return reg_crc;
}
