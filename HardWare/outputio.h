#ifndef __OUTPUTIO_H
#define __OUTPUTIO_H	 
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEKս��STM32������
//LED��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2012/9/2
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 
#define LED0 PAout(8)// PA8
#define LED1 PAout(9)// PA9
#define Door1 PBout(8)// PB8	
void Door_IO_init();
void LED_Init(void);//��ʼ��

		 				    
#endif
