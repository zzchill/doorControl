#ifndef __INPUTDEAL_H
#define __INPUTDEAL_H	 
#include "sys.h"
#include <stdint.h>

enum inputType
{
	NONE_TYPE = 0,
	REMOTE_TYPE,
	RC522_TYPE
};

enum uiType
{
	Standby = 0,
	doorOpen,
	menuPage,
	
	addUserPage,
	dataDeletePage,
	rootLoginPage,
	rootOutPage,
	addRootPage,
	modifyPage,
	systemInfoPage,
	makerInfo,
	
	inputDataPage,
	noticePage,			//清除所有用户数据后的提示页面
	opsSuccesePage,	
	hasNoRight,			
	errPage,			

};

enum workMode
{
	userMode,
	rootMode,
};

struct messegeFrame
{
	uint8_t type;
	uint32_t data;
};

uint8_t getNowPage(void);
void backMenu(void);
void resetLcd(void);
void DealRC522State(uint32_t data);
void DealRemoteState(uint32_t data);
void DealLcdShow(struct messegeFrame* messege);
void showSysInfo(void);
void showMenu(uint8_t showLineNum);
void showAllUserData(void);


//#define	WorkMode  101
//#define AddUser  102
//#define DelUser  103
//#define RootLogin  104
//#define InitSystem 105
//#define RootOut  106
//#define MakerInfo 107
//#define SystemInfo 108
//#define RootInit 109



//void showp1();
//void showp2();
//void meanu_mode(u8* pblash);
//void do_key(u8 key,u8 *blash);
//void show_mode(u8 ml,u8 *blash);			    
#endif
