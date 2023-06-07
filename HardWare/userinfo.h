#ifndef __USERINFO_H
#define __USERINFO_H 

typedef unsigned char u8;

#define WR_ID 1
#define WR_INFO 2

#define DEBUG 1
#define USER_ID_SIZE 4
#define USER_INFO_SIZE 6
#define USER_MAX_SIZE 100
#define USER_ID_LONG 12
#define SAVE_BLOG 10
#define PAGE_MAX 5
#define MEANU_MAX 4
#define MEANU_LONG 9


struct USER
{
	u8 id[USER_ID_SIZE];//4
	u8 info[USER_INFO_SIZE];//6
};
/***/




#endif