#include "stm32f10x.h"
#include "inputDeal.h"
#include "nokia_5110.h"
#include "lcdlib.h"
#include "outputio.h"
#include "userinfo.h"
#include "remote.h"
#include "freeRTOS.h"
#include "task.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MEANU_ITEM_NUM			10		 //菜单页的项目数
#define SCREEN_SHOW_MAX_LINE	6		//屏幕最多能显示行数


static uint8_t workMode = userMode;		//工作状态，如果卡丢了就修改这个成rootMode重新烧录后进行添加管理员数据
static uint8_t nowPage = Standby;		//当前工作页

static uint16_t cursors = 0;			//当前光标位置

static uint32_t cardToOps;				//一个传输变量不必在意
static struct UserData input;			//输入页面缓冲区

									

char meanu[MEANU_ITEM_NUM][15]={		"Add  User     ",
										"Del  User     ",
										"Root Login    ",
										"Root Add      ",
										"Root  Out     ",
										"Mod UserData  ",
										"System Info   ",	
										"Maker  Info   ",
										"init Root     ",
										"init User     ",};
									
//读卡得到数据的状态机
void DealRC522State(uint32_t data)
{
	switch (nowPage)
	{
		//当处于闲置界面时读到了卡数据就改变状态
		case Standby: 
			if ((findUserCard(data) != -1) || (findRootCard(data) != -1))
			{
				nowPage = doorOpen;
			}
			break;
			
		case rootLoginPage:
			//找到管理员的卡
			if (findRootCard(data) != -1)
			{
				workMode = rootMode;
				//设置提醒
				LCD_write_english_string(0, 2, " login success  ");
				vTaskDelay(1000);
				nowPage = menuPage;
				cursors = 0;
			}
			else
			{
				//设置提醒
				LCD_write_english_string(0, 2, "  login faild   ");
				vTaskDelay(1000);
				nowPage = menuPage;
				cursors = 0;
			}
			break;
			
		case addUserPage:
			nowPage = inputDataPage;
			cardToOps = data;
			break;
		
		case addRootPage:
			nowPage = inputDataPage;
			cardToOps = data;
			break;
		
		case modifyPage:
			if (findUserCard(data) != -1)
			{
				nowPage = inputDataPage;
				cardToOps = data;
			}
			else
			{
				nowPage = errPage;
			}
			break;
		
		default:
			break;
	}
}

//红外按键的状态机，写的很屎
void DealRemoteState(uint32_t data)
{
	static uint16_t input_index = 0;
	static uint16_t ops_page_type = 0;
	if (nowPage == Standby)			//如果处于闲置界面有按键按下进入菜单
	{
		nowPage = menuPage;
		return;
	}
	switch (data)
	{
		//ch按键是用来返回待机状态
		case REMOTE_CH_KEY_ID:
			if (nowPage == menuPage || nowPage == systemInfoPage 
				|| nowPage == makerInfo)
			{
				resetLcd();
			}
			break;
		
		//主要是用来控制光标的移动			
		case REMOTE_PREV_ID:		
			//当不同界面的显示界面切换回来时需要先限幅
			if (nowPage == menuPage)
			{
				cursors = (cursors < MEANU_ITEM_NUM - 1) ? (cursors) : (MEANU_ITEM_NUM - 1);	
			}
			else if (nowPage == modifyPage || nowPage == dataDeletePage)
			{
				cursors = (cursors < getUserNum() - 1) ? (cursors) : (getUserNum() - 1);
			}
			cursors = (cursors > 0) ? (cursors - 1) : (cursors);
			break;
		
		//主要是用来控制光标的移动
		case REMOTE_NEXT_ID:
			if (nowPage == menuPage)
			{
				cursors = (cursors < MEANU_ITEM_NUM - 1) ? (cursors + 1) : (MEANU_ITEM_NUM - 1);	//限制光标的行数
			}
			else if (nowPage == modifyPage || nowPage == dataDeletePage)
			{
				cursors = (cursors < getUserNum() - 1) ? (cursors + 1) : (getUserNum() - 1);
			}
			break;
		
		//最复杂的一个按键，就是确认按键
		case REMOTE_PAUSE_ID:
			if (nowPage == menuPage)
			{
				//状态gei
				if (cursors == 0 && workMode == rootMode) 		{ nowPage = addUserPage;}
				else if (cursors == 1 && workMode == rootMode) 	{ nowPage = dataDeletePage;cursors = 0;}
				else if (cursors == 2) 							{ nowPage = rootLoginPage;}
				else if (cursors == 3 && workMode == rootMode) 	{ nowPage = addRootPage;}
				else if (cursors == 4 && workMode == rootMode) 	{ nowPage = rootOutPage; workMode = userMode;}
				else if (cursors == 5 && workMode == rootMode) 	{ nowPage = modifyPage;}
				else if (cursors == 6) 							{ nowPage = systemInfoPage;}
				else if (cursors == 7) 							{ nowPage = makerInfo;}
				else if (cursors == 8 && workMode == rootMode) 	{ nowPage = noticePage;}
				else if (cursors == 9 && workMode == rootMode) 	{ nowPage = noticePage;}
				else {nowPage = hasNoRight;}
				
				ops_page_type = nowPage;
			}
			else if (nowPage == systemInfoPage || nowPage == makerInfo)
			{
				backMenu();
			}
			else if (nowPage == noticePage)		//当是警告删除界面时再次确认就是删除全部数据
			{
				//删除数据
				if (cursors == 8)
				{
					LCD_write_english_string(0, 2, "please wait");
					cleanAllRoot();
				}
				else if (cursors == 9)
				{
					LCD_write_english_string(0, 2, "please wait");
					cleanAllUser();
					
				}
				backMenu();
			}

			//当数据接收完毕后
			else if (nowPage == inputDataPage)
			{
				uint8_t state = 0xff;				//判断是否成功的标记位
				if (ops_page_type == addUserPage)
				{
					state = addUserInfo(cardToOps, &input);
				}
				else if (ops_page_type == addRootPage)
				{
					state = addRootInfo(cardToOps, &input);
				}
				else if (ops_page_type == modifyPage)
				{
					state = changUserDataByID(cardToOps, &input);
				}
				memset(&input, 0, sizeof(struct UserData));
				input_index = 0;
				nowPage = (state == 0) ? (opsSuccesePage) : (errPage);
			}
			//删除数据
			else if (nowPage == dataDeletePage)
			{
				nowPage = (deleteUser(cursors) == 0) ? (opsSuccesePage):(errPage);
			}
			
			
			break;
			
		//返回到memu的按键
		case REMOTE_EQ_ID:
			if (nowPage == addUserPage || nowPage == makerInfo
				|| nowPage == modifyPage || nowPage == dataDeletePage
				|| nowPage == rootLoginPage || nowPage == noticePage 
				|| nowPage == systemInfoPage)
			{
				backMenu();
			}
			else if (nowPage == inputDataPage)
			{
				memset(&input, 0, sizeof(struct UserData));
				input_index = 0;
				backMenu();
			}
			break;
			
		//用于撤回上次的输入操作
		case REMOTE_VOL_MINUS_ID:
			if (nowPage == inputDataPage)
			{
				input_index = (input_index == 0) ? (input_index) : (input_index - 1);
				input.StudentID[input_index] = '\0';
			}
			break;
		
		//下面的所有按键都是用来输入数据使用的
		case REMOTE_NUM0_ID:
			if ((nowPage == inputDataPage) && (input_index < USER_DATA_SIZE - 1))
			{
				input.StudentID[input_index] = '0';
				input.StudentID[input_index + 1] = '\0';
				input_index = (input_index == USER_DATA_SIZE - 2) ? (input_index) : (input_index + 1);
			}
			break;
		case REMOTE_NUM1_ID:
			if ((nowPage == inputDataPage) && (input_index < USER_DATA_SIZE - 1))
			{
				input.StudentID[input_index] = '1';
				input.StudentID[input_index + 1] = '\0';
				input_index = (input_index == USER_DATA_SIZE - 2) ? (input_index) : (input_index + 1);
			}
			break;
		case REMOTE_NUM2_ID:
			if ((nowPage == inputDataPage) && (input_index < USER_DATA_SIZE - 1))
			{
				input.StudentID[input_index] = '2';
				input.StudentID[input_index + 1] = '\0';
				input_index = (input_index == USER_DATA_SIZE - 2) ? (input_index) : (input_index + 1);
			}
			break;
		case REMOTE_NUM3_ID:
			if ((nowPage == inputDataPage) && (input_index < USER_DATA_SIZE - 1))
			{
				input.StudentID[input_index] = '3';
				input.StudentID[input_index + 1] = '\0';
				input_index = (input_index == USER_DATA_SIZE - 2) ? (input_index) : (input_index + 1);
			}
			break;
		case REMOTE_NUM4_ID:
			if ((nowPage == inputDataPage) && (input_index < USER_DATA_SIZE - 1))
			{
				input.StudentID[input_index] = '4';
				input.StudentID[input_index + 1] = '\0';
				input_index = (input_index == USER_DATA_SIZE - 2) ? (input_index) : (input_index + 1);
			}
			break;
		
		case REMOTE_NUM5_ID:
			if ((nowPage == inputDataPage) && (input_index < USER_DATA_SIZE - 1))
			{
				input.StudentID[input_index] = '5';
				input.StudentID[input_index + 1] = '\0';
				input_index = (input_index == USER_DATA_SIZE - 2) ? (input_index) : (input_index + 1);
			}
			break;
		
		case REMOTE_NUM6_ID:
			if ((nowPage == inputDataPage) && (input_index < USER_DATA_SIZE - 1))
			{
				input.StudentID[input_index] = '6';
				input.StudentID[input_index + 1] = '\0';
				input_index = (input_index == USER_DATA_SIZE - 2) ? (input_index) : (input_index + 1);
			}
			break;
		
		case REMOTE_NUM7_ID:
			if ((nowPage == inputDataPage) && (input_index < USER_DATA_SIZE - 1))
			{
				input.StudentID[input_index] = '7';
				input.StudentID[input_index + 1] = '\0';
				input_index = (input_index == USER_DATA_SIZE - 2) ? (input_index) : (input_index + 1);
			}
			break;
		
		case REMOTE_NUM8_ID:
			if ((nowPage == inputDataPage) && (input_index < USER_DATA_SIZE - 1))
			{
				input.StudentID[input_index] = '8';
				input.StudentID[input_index + 1] = '\0';
				input_index = (input_index == USER_DATA_SIZE - 2) ? (input_index) : (input_index + 1);
			}
			break;
		
		case REMOTE_NUM9_ID:
			if ((nowPage == inputDataPage) && (input_index < USER_DATA_SIZE - 1))
			{
				input.StudentID[input_index] = '9';
				input.StudentID[input_index + 1] = '\0';
				input_index = (input_index == USER_DATA_SIZE - 2) ? (input_index) : (input_index + 1);
			}
			break;
		
		default:break;
	};
}

//根据状态进行显示和处理
void DealLcdShow(struct messegeFrame* messege)
{
	LCD_clear();
	
	if (messege->type == NONE_TYPE)
	{
		resetLcd();
		LCD_Show_Pic(0, 0, 84, 48, pic_map);
	}
	switch (nowPage)
	{
		case Standby:
			LCD_Show_Pic(0, 0, 84, 48, pic_map);
			break;
		
		case doorOpen:
			DOOR1 = 0;	//打开门
			resetLcd();
			LCD_Show_Pic(0, 0, 84, 48, pic_door_close);
			vTaskDelay(500);
			LCD_clear();
			LCD_Show_Pic(0, 0, 84, 48, pic_door_open);
			vTaskDelay(500);
			LCD_clear();
			LCD_Show_Pic(0, 0, 84, 48, pic_map);
			DOOR1 = 1;	//关闭门
			break;
		
		case menuPage:
			showMenu(MEANU_ITEM_NUM);
			break;
		
		case addUserPage:
			LCD_write_english_string(0, 0, "put card on");
			break;
		
		case inputDataPage:
			LCD_write_english_string(0, 0, "enter data:");
			LCD_write_english_string(0, 1, (char *)(input.StudentID));
			break;
		
		//比较麻烦要显示出数据
		case dataDeletePage:
			showAllUserData();
			break;
		
		case rootLoginPage:
			//如果已经登录了
			if (workMode == rootMode)
			{
				LCD_write_english_string(0, 0, " logining now ");
				vTaskDelay(1000);
				LCD_clear();
				backMenu();
				showMenu(MEANU_ITEM_NUM);
			}
			else
			{
				LCD_write_english_string(0, 0, "put card on");
			}
			break;
		
		case rootOutPage:
			LCD_write_english_string(0, 0, "root log out");
			vTaskDelay(1000);
			LCD_clear();
			backMenu();
			showMenu(MEANU_ITEM_NUM);
			break;
		
		case addRootPage:
			LCD_write_english_string(0, 0, "put root card");
			break;
		
		case modifyPage:
			LCD_write_english_string(0, 0, "put old card");
			break;
		
		case systemInfoPage:
			showSysInfo();	
			break;
		
		case makerInfo:
			LCD_write_english_string(0, 0,  "  Maker Info  ");
			LCD_write_english_string(0, 1, "Maker:chill");
			LCD_write_english_string(0, 2, "wx:chill_zhzh");
			break;
		
		case noticePage:
			LCD_write_english_string(0, 0, "do you sure to");
			LCD_write_english_string(0, 1, "clean all data");
			break;
		
		case opsSuccesePage:
			LCD_write_english_string(0, 1, "operate succes");
			vTaskDelay(1000);
			LCD_clear();
			backMenu();
			showMenu(MEANU_ITEM_NUM);
			break;
		
		case hasNoRight:
			LCD_write_english_string(0, 1, " has no right ");
			vTaskDelay(1000);
			LCD_clear();
			backMenu();
			showMenu(MEANU_ITEM_NUM);
			break;
		
		case errPage:
			LCD_write_english_string(0, 1, "  err happen  ");
			vTaskDelay(1000);
			LCD_clear();
			backMenu();
			showMenu(MEANU_ITEM_NUM);
			break;
		
		default:
			break;
	}
}


void resetLcd(void)
{
	nowPage = Standby;
	cursors = 0;
}

void backMenu(void)
{
	nowPage = menuPage;
	cursors = 0;
}


uint8_t getNowPage(void)
{
	return nowPage;
}

void showMenu(uint8_t showLineNum)
{
	uint8_t tmp = 0;
	if (cursors / SCREEN_SHOW_MAX_LINE > 0)
	{
		showLineNum %= SCREEN_SHOW_MAX_LINE;
	}
	showLineNum = (showLineNum > SCREEN_SHOW_MAX_LINE) ? (SCREEN_SHOW_MAX_LINE) : (showLineNum);
	tmp = (cursors / SCREEN_SHOW_MAX_LINE) * SCREEN_SHOW_MAX_LINE;
	for (int i = 0; i < showLineNum; i++)
	{
		LCD_write_english_string(0, i, meanu[tmp + i]);
	}
	LCD_write_english_string(83 - 12, cursors % SCREEN_SHOW_MAX_LINE, "<");
}

void showSysInfo(void)
{
	char debug[16];
	sprintf(debug, "UserNum:%d", getUserNum());
	LCD_write_english_string(0, 1, debug);
	sprintf(debug, "RootNum:%d", getRootNum());
	LCD_write_english_string(0, 2, debug);
	LCD_write_english_string(0, 3, "Data:2023-6-17");
	LCD_write_english_string(0, 4, "Verson:V 1.1");
}

void showAllUserData(void)
{
	static uint16_t lastGroup = 0;
	static struct UserData dataToShow[SCREEN_SHOW_MAX_LINE];
	struct UserData buf;
	uint16_t nowGroup = cursors / SCREEN_SHOW_MAX_LINE * SCREEN_SHOW_MAX_LINE;		//获取显示的数据起始下标
	int tmp = 0;
	
	if (lastGroup != nowGroup || nowGroup == 0)
	{
		for (int i = 0; i < nowGroup; i++)
		{
			tmp = findNextUserData(tmp, &buf) + 1;
		}
		
		for (int i = 0; i < SCREEN_SHOW_MAX_LINE && i < getUserNum() - nowGroup; i++)
		{
			tmp = findNextUserData(tmp, &dataToShow[i]) + 1;
			LCD_write_english_string(0, i, (char *)dataToShow[i].StudentID); 
		}
	}
	
	lastGroup = nowGroup;
	LCD_write_english_string(83 - 12, cursors % SCREEN_SHOW_MAX_LINE, "<"); 
}


