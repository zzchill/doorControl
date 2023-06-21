#ifndef __RC522_H
#define __RC522_H
#include "sys.h"
#include "stm32f10x.h"
#include <stdint.h>

/////////////////////////////////////////////////////////////////////
//MF522������
/////////////////////////////////////////////////////////////////////
#define PCD_IDLE              0x00               //ȡ����ǰ����
#define PCD_AUTHENT           0x0E               //��֤��Կ
#define PCD_RECEIVE           0x08               //��������
#define PCD_TRANSMIT          0x04               //��������
#define PCD_TRANSCEIVE        0x0C               //���Ͳ���������
#define PCD_RESETPHASE        0x0F               //��λ
#define PCD_CALCCRC           0x03               //CRC����

/////////////////////////////////////////////////////////////////////
//Mifare_One��Ƭ������
/////////////////////////////////////////////////////////////////////
#define PCD_IDLE              0x00               //ȡ����ǰ����
#define PCD_AUTHENT           0x0E               //��֤��Կ
#define PCD_RECEIVE           0x08               //��������
#define PCD_TRANSMIT          0x04               //��������
#define PCD_TRANSCEIVE        0x0C               //���Ͳ���������
#define PCD_RESETPHASE        0x0F               //��λ
#define PCD_CALCCRC           0x03               //CRC����
#define PICC_REQIDL           0x26               //Ѱ��������δ��������״̬
#define PICC_REQALL           0x52               //Ѱ��������ȫ����
#define PICC_ANTICOLL1        0x93               //����ײ
#define PICC_ANTICOLL2        0x95               //����ײ
#define PICC_AUTHENT1A        0x60               //��֤A��Կ
#define PICC_AUTHENT1B        0x61               //��֤B��Կ
#define PICC_READ             0x30               //����
#define PICC_WRITE            0xA0               //д��
#define PICC_DECREMENT        0xC0               //�ۿ�
#define PICC_INCREMENT        0xC1               //��ֵ
#define PICC_RESTORE          0xC2               //�������ݵ�������
#define PICC_TRANSFER         0xB0               //���滺����������
#define PICC_HALT             0x50               //����

/////////////////////////////////////////////////////////////////////
//MF522 FIFO���ȶ���
/////////////////////////////////////////////////////////////////////
#define DEF_FIFO_LENGTH       64                 //FIFO size=64byte
#define MAXRLEN  18

/////////////////////////////////////////////////////////////////////
//MF522�Ĵ�������
/////////////////////////////////////////////////////////////////////
// PAGE 0
#define     RFU00                 0x00
#define     CommandReg            0x01
#define     ComIEnReg             0x02
#define     DivlEnReg             0x03
#define     ComIrqReg             0x04
#define     DivIrqReg             0x05
#define     ErrorReg              0x06
#define     Status1Reg            0x07
#define     Status2Reg            0x08
#define     FIFODataReg           0x09
#define     FIFOLevelReg          0x0A
#define     WaterLevelReg         0x0B
#define     ControlReg            0x0C
#define     BitFramingReg         0x0D
#define     CollReg               0x0E
#define     RFU0F                 0x0F
// PAGE 1
#define     RFU10                 0x10
#define     ModeReg               0x11
#define     TxModeReg             0x12
#define     RxModeReg             0x13
#define     TxControlReg          0x14
#define     TxAutoReg             0x15
#define     TxSelReg              0x16
#define     RxSelReg              0x17
#define     RxThresholdReg        0x18
#define     DemodReg              0x19
#define     RFU1A                 0x1A
#define     RFU1B                 0x1B
#define     MifareReg             0x1C
#define     RFU1D                 0x1D
#define     RFU1E                 0x1E
#define     SerialSpeedReg        0x1F
// PAGE 2
#define     RFU20                 0x20
#define     CRCResultRegM         0x21
#define     CRCResultRegL         0x22
#define     RFU23                 0x23
#define     ModWidthReg           0x24
#define     RFU25                 0x25
#define     RFCfgReg              0x26
#define     GsNReg                0x27
#define     CWGsCfgReg            0x28
#define     ModGsCfgReg           0x29
#define     TModeReg              0x2A
#define     TPrescalerReg         0x2B
#define     TReloadRegH           0x2C
#define     TReloadRegL           0x2D
#define     TCounterValueRegH     0x2E
#define     TCounterValueRegL     0x2F
// PAGE 3
#define     RFU30                 0x30
#define     TestSel1Reg           0x31
#define     TestSel2Reg           0x32
#define     TestPinEnReg          0x33
#define     TestPinValueReg       0x34
#define     TestBusReg            0x35
#define     AutoTestReg           0x36
#define     VersionReg            0x37
#define     AnalogTestReg         0x38
#define     TestDAC1Reg           0x39
#define     TestDAC2Reg           0x3A
#define     TestADCReg            0x3B
#define     RFU3C                 0x3C
#define     RFU3D                 0x3D
#define     RFU3E                 0x3E
#define     RFU3F		  		        0x3F

/////////////////////////////////////////////////////////////////////
//��MF522ͨѶʱ���صĴ������
/////////////////////////////////////////////////////////////////////
#define 	MI_OK                 0
#define 	MI_NOTAGERR           (1)
#define 	MI_ERR                (2)

#define	SHAQU1	0X01
#define	KUAI4	0X04
#define	KUAI7	0X07
#define	REGCARD	0xa1
#define	CONSUME	0xa2
#define READCARD	0xa3
#define ADDMONEY	0xa4



/***********************RC522 �����궨��**********************/
#define RC522_SPI_PIN_PORT	GPIOA
#define RC522_SPI			SPI1
#define RC522_SPI_PIN_RCC	RCC_APB2Periph_GPIOA
#define RC522_SPI_RCC		RCC_APB2Periph_SPI1

#define RC522_RESET_PORT	GPIOB
#define RC522_RESET_PIN_RCC	RCC_APB2Periph_GPIOB
#define RC522_RESET_PIN		GPIO_Pin_0

#define RC522_NSS_PIN	GPIO_Pin_4
#define RC522_SCK_PIN	GPIO_Pin_5
#define RC522_MISO_PIN	GPIO_Pin_6
#define RC522_MOSI_PIN	GPIO_Pin_7

#define          RC522_CS_Enable()         GPIO_ResetBits ( RC522_SPI_PIN_PORT, RC522_NSS_PIN )
#define          RC522_CS_Disable()        GPIO_SetBits ( RC522_SPI_PIN_PORT, RC522_NSS_PIN )

#define          RC522_Reset_Enable()      GPIO_ResetBits( RC522_RESET_PORT, RC522_RESET_PIN )
#define          RC522_Reset_Disable()     GPIO_SetBits ( RC522_RESET_PORT, RC522_RESET_PIN )

#define          RC522_SCK_0()             GPIO_ResetBits( RC522_SPI_PIN_PORT, RC522_SCK_PIN )
#define          RC522_SCK_1()             GPIO_SetBits ( RC522_SPI_PIN_PORT, RC522_SCK_PIN )

#define          RC522_MOSI_0()            GPIO_ResetBits( RC522_SPI_PIN_PORT, RC522_MOSI_PIN )
#define          RC522_MOSI_1()            GPIO_SetBits ( RC522_SPI_PIN_PORT, RC522_MOSI_PIN )

#define          RC522_MISO_GET()          GPIO_ReadInputDataBit ( RC522_SPI_PIN_PORT, RC522_MISO_PIN )

void SPIInit(SPI_TypeDef* SPIx);			 //��ʼ��SPI��
void SPI_SetSpeed(SPI_TypeDef* SPIx, uint8_t SPI_BaudRatePrescaler);//����SPI�ٶ�   
int SPI_WriteNBytes(SPI_TypeDef* SPIx, uint8_t *TxData, uint32_t SendNum);
int SPI_ReadNBytes(SPI_TypeDef* SPIx, uint8_t *TxData, uint8_t *RxData, uint32_t ReciveNum);

void RC522_Init(void);
char PcdRequest(unsigned char req_code,unsigned char *pTagType);
char PcdAnticoll(unsigned char *pSnr);
char PcdSelect(unsigned char *pSnr);
char PcdAuthState(unsigned char auth_mode,unsigned char addr,unsigned char *pKey,unsigned char *pSnr);
char PcdRead(unsigned char addr,unsigned char *pData);
char PcdWrite(unsigned char addr,unsigned char *pData);
char PcdValue(unsigned char dd_mode,unsigned char addr,unsigned char *pValue);
char PcdBakValue(unsigned char sourceaddr, unsigned char goaladdr);
char PcdHalt(void);
void CalulateCRC(unsigned char *pIndata,unsigned char len,unsigned char *pOutData);
char PcdReset(void);
unsigned char ReadRawRC(unsigned char Address);
void WriteRawRC(unsigned char Address, unsigned char value);
void SetBitMask(unsigned char reg,unsigned char mask);
void ClearBitMask(unsigned char reg,unsigned char mask);
char PcdComMF522(unsigned char Command,
                 unsigned char *pInData,
                 unsigned char InLenByte,
                 unsigned char *pOutData,
                 unsigned int  *pOutLenBit);
void PcdAntennaOn(void);
void PcdAntennaOff(void);
void RC522_Config(unsigned char Card_Type);

void ShowID(uint8_t *p);	 //��ʾ���Ŀ��ţ���ʮ��������ʾ
void clear_mode(void);
void read_mode();
void write_mode();
void information_init();
void root_mode(uint8_t *blash);
void initSN();



#endif

