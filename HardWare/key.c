#include "stm32f10x.h"
#include "key.h"
#include "sys.h" 
#include "delay.h"
#include "rc522.h"
#include "usart.h"
#include "outputio.h"
#include "stmflash.h"
#include "nokia_5110.h"
#include "lcdlib.h"
#include "remote.h"
#include "userinfo.h"
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
extern u16 NUM;
extern u16 NUM_ROOT;


//红外按键执行处理函数
//作用：处理按键
void do_key(u8 key,u8 *blash)			//执行按键对应的行为
{
	static u8 i;
	static u8 sec[4]={0};
	//添加模式 需要有ROOT权限
	if(key==AddUser)	//KEY1按下,写入STM32 FLASH
	{
		sec[0]=1;
		i=0;//做标记用，每进入一个页面后先将i归零，之后如果没有按键按下则i递增，当i递增到=5时，自动返回主界面
		if(check_rootkey())
		{
			*blash=1;
			printf("ADD mode\r\n");
			LCD_clear();
			LCD_write_english_string(0,2," Add Mode ");
			LCD_write_english_string(0,3," ADD USERS ");
		}
		else
		{	
			printf("You are not root!\r\n");
			LCD_clear();
			LCD_write_english_string(0,2," Add Mode ");
			LCD_write_english_string(0,3," Limited!");
		}
		LCD_write_english_string(0,0," GDUT  ELC");
	}
	//工作模式
	if(key==WorkMode)	//KEY0按下,读取字符串并显示
	{

		i=0;//做标记用，每进入一个页面后先将i归零，之后如果没有按键按下则i递增，当i递增到=5时，自动返回主界面
		printf("Work mode\r\n");
		LCD_clear();
		LCD_write_english_string(0,2," Work Mode ");
		LCD_write_english_string(0,0," GDUT  ELC");
		*blash=2;
		initSN();	
	}
	//删除模式  需要有ROOT权限
	if(key==DelUser)
	{
		if(sec[2]==3)
			sec[1]=2;
		i=0;
		if(check_rootkey())
		{
			printf("Del Mode\r\n");
			LCD_clear();
			LCD_write_english_string(0,2," DEL Mode ");
			*blash=3;
		}
		else
		{
			LCD_clear();		
			LCD_write_english_string(0,2," DEL Mode ");
			printf("Not root\r\n");
			LCD_write_english_string(0,3," Limited！ ");
		}
		
		LCD_write_english_string(0,0," GDUT  ELC");
		delay_ms(200); 
		initSN();
		
	}
	//登录 或 添加ROOT用户
	if(key==RootLogin)
	{
		if(sec[0]==1)
			sec[3]=4;
		i=0;

		LCD_clear();
		LCD_write_english_string(0,0," GDUT  ELC");
		LCD_write_english_string(0,2," Root Login ");
		*blash=4;
		initSN();
	}
	//初始化系统，清除所有用户
	if(key==InitSystem)
	{
		i=0;
		if(check_rootkey())
		{
			LCD_clear();
			LCD_write_english_string(0,2," All Users ");
			LCD_write_english_string(0,23,"  Clean ");
			STMFLASH_Write(FLASH_SAVE_ADDR,(u16*)&i,1);

			NUM=0;					//将用户数清零

		}
		else
		{
			LCD_clear();
			LCD_write_english_string(0,1," You're not");

			LCD_write_english_string(0,2,"   Root ");
			LCD_write_english_string(0,3,"  Limited ");
		}
		delay_ms(2000);
	}
	//管理员初始化，清除所有管理员
	if(key==RootInit)
	{
		i=0;
		if(check_rootkey())
		{
			LCD_clear();
			LCD_write_english_string(0,2," Clean Root ");
			STMFLASH_Write(FlASH_ROOT_NUM_DATA,(u16*)&i,1);
			NUM_ROOT=0;					//将管理员数清零
		}
		else
		{
			LCD_clear();
			LCD_write_english_string(0,1," You're not");

			LCD_write_english_string(0,2,"   Root ");
			LCD_write_english_string(0,3,"  Limited ");
		}
		delay_ms(2000);
	}
	//管理员退出
	if(key==RootOut)
	{
		i=0;
		LCD_clear();
		if(check_rootkey())
			LCD_write_english_string(0,2,"  Logout! ");
		else
			LCD_write_english_string(0,2,"  No Root!");
		logout();
	}
	if(key==MakerInfo)
	{
		*blash=MakerInfo;
	}
	if(key==SystemInfo)
	{
		*blash=SystemInfo;
	}
	
	i++;
	delay_ms(150);  
	
	if(i==5)				//显示主页面
	{
		LED0=!LED0;
		LCD_clear();
		LCD_Show_Pic(0, 0, 84, 48, pic_map);			//主界面地图图像
		if(check_rootkey())
			LCD_Show_Pic(0, 0, 12, 12, pic_unlock);		//主界面左上角解锁图标
		if(*blash==2) 
			LCD_Show_Pic(36, 0, 12, 12, w_mode);		//主界面第一行居中模式图标
		else if(*blash==4) 
			LCD_Show_Pic(36, 0, 12, 12, r_mode);
		else if(*blash==1) 
			LCD_Show_Pic(36, 0, 12, 12, a_mode);
		i=0;
	}		   
}
//显示菜单函数
//作用：显示菜单
u8 show_meanu(u8 ml,u8* cur,char show_char[][15],u8* base)
{
	//ml为1时行标向下移动一行，为0时向上移动1行
	//cur 行标、MEANU_MAX每页最大行数4行，从0行开始算起
	//base每页第一行
	u8 i,j;
	char temp[15];
	if(ml==1)
	{
		if(*cur == MEANU_MAX-1)
		{	if ((*base)<MEANU_LONG-MEANU_MAX)(*base)++;}
		else
			(*cur)++;
	}
	else if(ml==0)
	{
		if(*cur == 0)
		{	if((*base)>0) (*base)--;}
		else
			(*cur)--;
	}
	LCD_clear();				//每次操作都会刷新一次显示
	for(i=0;i<MEANU_MAX;i++)
	{

		LCD_write_english_string(0,i,show_char[(*base)+i]);
	}
	for(j=0;j<12;j++)
		temp[j]=show_char[(*cur)+(*base)][j];
	temp[11]='<';
	temp[12]=' ';
	temp[13]=' ';
	LCD_write_english_string(0,*cur,temp);
	LCD_write_english_string(0,4,"\\\\\\\\\\\\\\\\\\\\\\\\\\\\");
	LCD_write_english_string(0,5,"    Meanu");
	
	
	
	return (*base)+(*cur);				//返回当前页第一行行数和行标所在行行数
}
//菜单页模式
//进入菜单目录，并处理选中的菜单
void meanu_mode(u8* pblash)
{
	char meanu[MEANU_LONG][15]={"Work Mode  ",
									 "Add  User  ",
									 "Del  User  ",
									 "Root Login ",
									 "Root  Out  ",
									 "Root init   ",
									 "Init System  ",		 
									 "Maker  Info  ",
									 "System Info  ",
									 
									};
	u8 num_flag=1,key=0,current=0,sum_current=0,base=0;
	
	show_meanu(2,&current,meanu,&base);
	while(num_flag)
	{
		key=Remote_Scan();
		switch(key)
		{
				    
				case 98:num_flag=0;sum_current=0;break;//ch	    确定键
				case 168:sum_current=show_meanu(1,&current,meanu,&base);break;//+	下一行
				case 226:sum_current=show_meanu(1,&current,meanu,&base);break;//ch+	
				case 2:sum_current=show_meanu(1,&current,meanu,&base);break;//>>|			
				case 194:num_flag=0;break;//>||	  确定键
				case 34:sum_current=show_meanu(0,&current,meanu,&base);break;//|<<		上一行	
				case 224:sum_current=show_meanu(0,&current,meanu,&base);break;//-		 
				case 162:sum_current=show_meanu(0,&current,meanu,&base);break;//ch-							
		}

	}
		printf("%d\r\n",sum_current);
		switch(sum_current)
		{
			case 0:*pblash=WorkMode;break;//Work Mode  
			case 1:*pblash=AddUser;break;//Add  User  
			case 2:*pblash=DelUser;break;//Del  User  
			case 3:*pblash=RootLogin;break;//Root Login 
			case 4:*pblash=RootOut;break;
			case 5:*pblash=RootInit;break;//RootInit 
			case 6:*pblash=InitSystem;break;  //Init System
			case 7:*pblash=MakerInfo;break;//Maker  Info 
			case 8:*pblash=SystemInfo;break;//System  Info 
		}
		
									
	
}
void showp1()								//显示图片1
{
	LCD_Show_Pic(0, 0, 84, 48, pic_heart);
}
void showp2()								//显示图片2
{
	LCD_Show_Pic(0, 0, 84, 48, pic_dianxie);
}
//信息页模式
//从菜单页选择选项后显示的页面
void show_mode(u8 ml,u8 *blash)				
{
	char temp[15]={"User Num:     "};		//用户个数
	char temp2[15]={"Root Num:     "};		//管理员个数
	u8 t=11,num_flag=1,key=0;
	if(ml==SystemInfo)						//系统信息页
	{
		LCD_clear();
		while(num_flag)
		{
			key=Remote_Scan();
			if(key!=0) num_flag=0;			//按任意键退出此界面
			LCD_write_english_string(0,0,"    Sys Info");

		//48的ASCII码为数字0
			temp[t]=NUM/100+48;					//取百位
			if(temp[t]==48) temp[t]=' ';
			temp[t+1]=NUM/10%10+48;				//取十位
			if(temp[t+1]==48) temp[t+1]=' ';
			temp[t+2]=NUM%10+48;				//取个位
			LCD_write_english_string(0,1,temp);
			//NUM_ROOT
			temp2[t]=NUM_ROOT/100+48;
			if(temp2[t]==48) temp2[t]=' ';
			temp2[t+1]=NUM_ROOT/10%10+48;
			if(temp2[t+1]==48) temp2[t+1]=' ';
			temp2[t+2]=NUM_ROOT%10+48;
			LCD_write_english_string(0,2,temp2);
			LCD_write_english_string(0,3,"Data:2020-10-19");
			LCD_write_english_string(0,4,"Verson:V 1.0");
		}
		*blash=2;						//返回工作模式
	}
	else if(ml==MakerInfo)					//作者信息页
	{
		LCD_clear();
		while(num_flag)
		{
			key=Remote_Scan();
			if(key!=0) num_flag=0;			//按任意键退出此界面
			LCD_write_english_string(0,0,"  Maker Info  ");
			LCD_write_english_string(0,1,"Maker:Jakefish");
			LCD_write_english_string(0,2,"Ph:15521113661");

		}
		*blash=2;					   //返回工作模式
	}
}