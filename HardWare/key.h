#ifndef __KEY_H
#define __KEY_H	 
#include "sys.h"

//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEKս��STM32������
//������������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2012/9/3
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved									  
//////////////////////////////////////////////////////////////////////////////////   	 


#define	WorkMode  101
#define AddUser  102
#define DelUser  103
#define RootLogin  104
#define InitSystem 105
#define RootOut  106
#define MakerInfo 107
#define SystemInfo 108
#define RootInit 109



void showp1();
void showp2();
void meanu_mode(u8* pblash);
void do_key(u8 key,u8 *blash);
void show_mode(u8 ml,u8 *blash);			    
#endif
