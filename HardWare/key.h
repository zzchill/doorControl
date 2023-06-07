#ifndef __KEY_H
#define __KEY_H	 
#include "sys.h"

//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK战舰STM32开发板
//按键驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2012/9/3
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
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
