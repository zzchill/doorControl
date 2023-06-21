#ifndef __RED_H
#define __RED_H 
#include "sys.h"   
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEKս��STM32������V3
//����ң�ؽ������� ����	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2015/1/15
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved									  
//////////////////////////////////////////////////////////////////////////////////

#define RDATA 	PBin(9)	 	//�������������

//����ң��ʶ����(ID),ÿ��ң�����ĸ�ֵ��������һ��,��Ҳ��һ����.
//����ѡ�õ�ң����ʶ����Ϊ0
#define REMOTE_ID 0   

#define REMOTE_CH_MINUS_ID	162
#define REMOTE_CH_KEY_ID	98
#define REMOTE_CH_PLUS_ID	226

#define REMOTE_PREV_ID		34
#define REMOTE_NEXT_ID		2
#define REMOTE_PAUSE_ID		194

#define REMOTE_VOL_MINUS_ID	224
#define REMOTE_VOL_PLUS_ID	168
#define REMOTE_EQ_ID		144

#define REMOTE_NUM0_ID		104
#define REMOTE_NUM100_ID	152
#define REMOTE_NUM200_ID	176

#define REMOTE_NUM1_ID		48
#define REMOTE_NUM2_ID		24
#define REMOTE_NUM3_ID		122

#define REMOTE_NUM4_ID		16
#define REMOTE_NUM5_ID		56
#define REMOTE_NUM6_ID		90

#define REMOTE_NUM7_ID		66
#define REMOTE_NUM8_ID		74
#define REMOTE_NUM9_ID		82

//-----------------------------------------

#define KEY_CH_MINUS		1
#define KEY_CH				2
#define KEY_CH_PLUS			3

#define KEY_PREV			4
#define KEY_NEXT			5
#define KEY_PAUSE			6

#define KEY_VOL_MINUS		7
#define KEY_VOL_PLUS		8
#define KEY_EQ				9

#define KEY_NUM100			10
#define KEY_NUM200			11
#define KEY_NUM0			12

#define KEY_NUM1			13
#define KEY_NUM2			14
#define KEY_NUM3			15

#define KEY_NUM4			16
#define KEY_NUM5			17
#define KEY_NUM6			18

#define KEY_NUM7			19
#define KEY_NUM8			20
#define KEY_NUM9			21


extern u8 RmtCnt;			//�������µĴ���

void Remote_Init(void);    	//���⴫��������ͷ���ų�ʼ��
u8 Remote_Scan(void);	    
#endif















