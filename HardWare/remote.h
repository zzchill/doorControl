#ifndef __RED_H
#define __RED_H 
#include "sys.h"   
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK战舰STM32开发板V3
//红外遥控解码驱动 代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2015/1/15
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved									  
//////////////////////////////////////////////////////////////////////////////////

#define RDATA 	PBin(9)	 	//红外数据输入脚

//红外遥控识别码(ID),每款遥控器的该值基本都不一样,但也有一样的.
//我们选用的遥控器识别码为0
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


extern u8 RmtCnt;			//按键按下的次数

void Remote_Init(void);    	//红外传感器接收头引脚初始化
u8 Remote_Scan(void);	    
#endif















