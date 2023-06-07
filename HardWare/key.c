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
extern u16 NUM;
extern u16 NUM_ROOT;


//���ⰴ��ִ�д�����
//���ã�������
void do_key(u8 key,u8 *blash)			//ִ�а�����Ӧ����Ϊ
{
	static u8 i;
	static u8 sec[4]={0};
	//���ģʽ ��Ҫ��ROOTȨ��
	if(key==AddUser)	//KEY1����,д��STM32 FLASH
	{
		sec[0]=1;
		i=0;//������ã�ÿ����һ��ҳ����Ƚ�i���㣬֮�����û�а���������i��������i������=5ʱ���Զ�����������
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
	//����ģʽ
	if(key==WorkMode)	//KEY0����,��ȡ�ַ�������ʾ
	{

		i=0;//������ã�ÿ����һ��ҳ����Ƚ�i���㣬֮�����û�а���������i��������i������=5ʱ���Զ�����������
		printf("Work mode\r\n");
		LCD_clear();
		LCD_write_english_string(0,2," Work Mode ");
		LCD_write_english_string(0,0," GDUT  ELC");
		*blash=2;
		initSN();	
	}
	//ɾ��ģʽ  ��Ҫ��ROOTȨ��
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
			LCD_write_english_string(0,3," Limited�� ");
		}
		
		LCD_write_english_string(0,0," GDUT  ELC");
		delay_ms(200); 
		initSN();
		
	}
	//��¼ �� ���ROOT�û�
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
	//��ʼ��ϵͳ����������û�
	if(key==InitSystem)
	{
		i=0;
		if(check_rootkey())
		{
			LCD_clear();
			LCD_write_english_string(0,2," All Users ");
			LCD_write_english_string(0,23,"  Clean ");
			STMFLASH_Write(FLASH_SAVE_ADDR,(u16*)&i,1);

			NUM=0;					//���û�������

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
	//����Ա��ʼ����������й���Ա
	if(key==RootInit)
	{
		i=0;
		if(check_rootkey())
		{
			LCD_clear();
			LCD_write_english_string(0,2," Clean Root ");
			STMFLASH_Write(FlASH_ROOT_NUM_DATA,(u16*)&i,1);
			NUM_ROOT=0;					//������Ա������
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
	//����Ա�˳�
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
	
	if(i==5)				//��ʾ��ҳ��
	{
		LED0=!LED0;
		LCD_clear();
		LCD_Show_Pic(0, 0, 84, 48, pic_map);			//�������ͼͼ��
		if(check_rootkey())
			LCD_Show_Pic(0, 0, 12, 12, pic_unlock);		//���������Ͻǽ���ͼ��
		if(*blash==2) 
			LCD_Show_Pic(36, 0, 12, 12, w_mode);		//�������һ�о���ģʽͼ��
		else if(*blash==4) 
			LCD_Show_Pic(36, 0, 12, 12, r_mode);
		else if(*blash==1) 
			LCD_Show_Pic(36, 0, 12, 12, a_mode);
		i=0;
	}		   
}
//��ʾ�˵�����
//���ã���ʾ�˵�
u8 show_meanu(u8 ml,u8* cur,char show_char[][15],u8* base)
{
	//mlΪ1ʱ�б������ƶ�һ�У�Ϊ0ʱ�����ƶ�1��
	//cur �бꡢMEANU_MAXÿҳ�������4�У���0�п�ʼ����
	//baseÿҳ��һ��
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
	LCD_clear();				//ÿ�β�������ˢ��һ����ʾ
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
	
	
	
	return (*base)+(*cur);				//���ص�ǰҳ��һ���������б�����������
}
//�˵�ҳģʽ
//����˵�Ŀ¼��������ѡ�еĲ˵�
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
				    
				case 98:num_flag=0;sum_current=0;break;//ch	    ȷ����
				case 168:sum_current=show_meanu(1,&current,meanu,&base);break;//+	��һ��
				case 226:sum_current=show_meanu(1,&current,meanu,&base);break;//ch+	
				case 2:sum_current=show_meanu(1,&current,meanu,&base);break;//>>|			
				case 194:num_flag=0;break;//>||	  ȷ����
				case 34:sum_current=show_meanu(0,&current,meanu,&base);break;//|<<		��һ��	
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
void showp1()								//��ʾͼƬ1
{
	LCD_Show_Pic(0, 0, 84, 48, pic_heart);
}
void showp2()								//��ʾͼƬ2
{
	LCD_Show_Pic(0, 0, 84, 48, pic_dianxie);
}
//��Ϣҳģʽ
//�Ӳ˵�ҳѡ��ѡ�����ʾ��ҳ��
void show_mode(u8 ml,u8 *blash)				
{
	char temp[15]={"User Num:     "};		//�û�����
	char temp2[15]={"Root Num:     "};		//����Ա����
	u8 t=11,num_flag=1,key=0;
	if(ml==SystemInfo)						//ϵͳ��Ϣҳ
	{
		LCD_clear();
		while(num_flag)
		{
			key=Remote_Scan();
			if(key!=0) num_flag=0;			//��������˳��˽���
			LCD_write_english_string(0,0,"    Sys Info");

		//48��ASCII��Ϊ����0
			temp[t]=NUM/100+48;					//ȡ��λ
			if(temp[t]==48) temp[t]=' ';
			temp[t+1]=NUM/10%10+48;				//ȡʮλ
			if(temp[t+1]==48) temp[t+1]=' ';
			temp[t+2]=NUM%10+48;				//ȡ��λ
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
		*blash=2;						//���ع���ģʽ
	}
	else if(ml==MakerInfo)					//������Ϣҳ
	{
		LCD_clear();
		while(num_flag)
		{
			key=Remote_Scan();
			if(key!=0) num_flag=0;			//��������˳��˽���
			LCD_write_english_string(0,0,"  Maker Info  ");
			LCD_write_english_string(0,1,"Maker:Jakefish");
			LCD_write_english_string(0,2,"Ph:15521113661");

		}
		*blash=2;					   //���ع���ģʽ
	}
}