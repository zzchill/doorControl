#include "userinfo.h"
#include "stmflash.h"
#include "stm32f10x.h"
#include "nokia_5110.h"
#include <stdio.h>

static struct CardIdToOffsetMap FlashDataMap[USER_MAX_NUM];
static struct CardIdToOffsetMap RootDataMap[ROOT_MAX_NUM];

static struct SupperIndex index;

uint32_t getRootNum(void)
{
	return index.RootNum;
}

uint32_t getUserNum(void)
{
	return index.UserNum;
}

//根据偏移获取flash中用户数据
void readUserData(uint32_t offset, struct UserData *data)
{
	assert_param(offset % 2 != 1);
	
	STMFLASH_Read(USER_DATA_ADDR + offset, (uint16_t *)(data), sizeof(struct UserData) / sizeof(uint16_t));
}

//更改flash中用户数据
void writeUserData(uint32_t offset, struct UserData *newData)
{
	assert_param(offset % 2 != 1);
	
	STMFLASH_Write(USER_DATA_ADDR + offset, (uint16_t *)newData, sizeof(struct UserData) / sizeof(uint16_t));
}

//根据偏移获取flash中用户数据
void readRootData(uint32_t offset, struct UserData *data)
{
	assert_param(offset % 2 != 1);
	
	STMFLASH_Read(ROOT_DATA_ADDR + offset, (uint16_t *)(data), sizeof(struct UserData) / sizeof(uint16_t));
}

//更改flash中用户数据
void writeRootData(uint32_t offset, struct UserData *newData)
{
	assert_param(offset % 2 != 1);
	
	STMFLASH_Write(ROOT_DATA_ADDR + offset, (uint16_t *)newData, sizeof(struct UserData) / sizeof(uint16_t));
}

void InfoInit(void)
{
	userDataMapGet();
	rootDataMapGet();
}

//获取已经存在flash中的索引信息并返回对应的指针
void userDataMapGet(void)
{	
	
	index.UserMap = FlashDataMap;
	STMFLASH_Read(INDEX_BASE_ADDR + ((char *)&index.UserNum - (char *)&index),
				(uint16_t *)(&index.UserNum), sizeof(uint32_t) / sizeof(uint16_t));
	struct CardIdToOffsetMap *ptr = FlashDataMap;
	for (int i = 0; i < USER_MAX_NUM; i++)
	{
		STMFLASH_Read(USER_MAP_BASE_ADDR + i * sizeof(struct CardIdToOffsetMap),
					(uint16_t *)(ptr), sizeof(struct CardIdToOffsetMap) / sizeof(uint16_t));
//		printf("userptr:%p, valid:%d, offset:%d, id:%d\n", ptr, ptr->valid, ptr->offset, ptr->cardID);
		ptr++;
	}
	
}

//返回值为0则添加成功，否则失败
uint8_t addUserInfo(uint32_t cardID, struct UserData *newData)
{
	uint8_t res = 1;
	if (index.UserNum >= USER_MAX_NUM)
	{
		return res;
	}
	
	for (int i = 0; i < USER_MAX_NUM; i++)
	{
		if (index.UserMap[i].valid == 0)
		{
			index.UserMap[i].cardID = cardID;
			index.UserMap[i].offset = i * sizeof(struct UserData);
			index.UserMap[i].valid = 1;
			
			index.UserNum++;
			
			//更新索引中人数在flash中的数据
			STMFLASH_Write(INDEX_BASE_ADDR + ((char *)&index.UserNum - (char *)&index),
							(uint16_t *)(&(index.UserNum)), sizeof(index.UserNum) / sizeof(uint16_t));
			
			//更新索引map在flash中的数据
			STMFLASH_Write(USER_MAP_BASE_ADDR + i * sizeof(struct CardIdToOffsetMap),
							(uint16_t *)(&index.UserMap[i]), sizeof(struct CardIdToOffsetMap) / sizeof(uint16_t));
			
			//更新用户数据
			STMFLASH_Write(USER_DATA_ADDR + i * sizeof(struct UserData),
							(uint16_t *)newData, sizeof(struct UserData) / sizeof(uint16_t));
			res = 0;
			break;
		}
	}
	return res;
}

//返回值为0则删除成功，否则失败
uint8_t deleteUserInfo(uint32_t cardID)
{
	uint32_t j = 0;
	for (int i = 0; i < USER_MAX_NUM && j < index.UserNum; i++)
	{
		//如果映射中没有数据
		if (index.UserMap[i].valid == 0)
		{
			continue;
		}
		
		j++;
		
		if (index.UserMap[i].cardID == cardID)
		{
			index.UserMap[i].valid = 0;
			index.UserNum--;
			
			//更新索引中人数在flash中的数据
			STMFLASH_Write(USER_DATA_ADDR, (uint16_t *)(&index.UserNum), sizeof(index.UserNum) / sizeof(uint16_t));
			
			//更新索引map在flash中的数据,只修改valid位
			STMFLASH_Write(USER_MAP_BASE_ADDR + i * sizeof(struct CardIdToOffsetMap),
							(uint16_t *)(&index.UserMap[i]), sizeof(index.UserMap[i].valid) / sizeof(uint16_t));
			
			return 0;
		}
	}
	return 1;
}

//传入的参数是用户列表中的第n位，从0开始
//返回值为0则删除成功，否则失败
uint8_t deleteUser(uint32_t numIndex)
{
	if (numIndex > index.UserNum)return 1;
	uint32_t j = 0;
	for (int i = 0; i < USER_MAX_NUM && j < index.UserNum; i++)
	{
		//如果映射中没有数据
		if (index.UserMap[i].valid == 0)
		{
			continue;
		}
		
		if (j++ == numIndex)
		{
			index.UserMap[i].valid = 0;
			index.UserNum--;
			
			//更新索引中人数在flash中的数据
			STMFLASH_Write(USER_DATA_ADDR, (uint16_t *)(&index.UserNum), sizeof(index.UserNum) / sizeof(uint16_t));
			
			//更新索引map在flash中的数据,只修改valid位
			STMFLASH_Write(USER_MAP_BASE_ADDR + i * sizeof(struct CardIdToOffsetMap),
							(uint16_t *)(&index.UserMap[i]), sizeof(index.UserMap[i].valid) / sizeof(uint16_t));
			
			return 0;
		}
		
	}
	return 1;
}

//当返回 -1 代表没找到该卡
//找到了就返回map中对应的下标
int findUserCard(uint32_t cardID)
{
	uint32_t j = 0;
	for (int i = 0; i < USER_MAX_NUM && j < index.UserNum; i++)
	{
		//如果映射中没有数据
		if (index.UserMap[i].valid == 0)
		{
			continue;
		}
		
		j++;
		
		if (index.UserMap[i].cardID == cardID)
		{
			return i;
		}
	}
	return -1;
}

//读取下个有效索引的数据并返回对应下标,data是传入的缓冲区
int findNextUserData(int start, struct UserData *data)
{
	for (int i = start; i < USER_MAX_NUM; i++)
	{
		//如果映射中有数据
		if (index.UserMap[i].valid == 1)
		{
			readUserData(index.UserMap[i].offset, data);
			return i;
		}
		
	}
	return -1;
}

//通过卡号改变用户数据,
//返回0代表修改成功
uint8_t changUserDataByID(uint32_t cardID, struct UserData *data)
{
	int mark = findUserCard(cardID);
	if (mark != -1)
	{
		writeUserData(index.UserMap[mark].offset, data);
		return 0;
	}
	return 0xff;
}


//传入的参数是用户列表中的第n位，从0开始
//返回0代表修改成功
uint8_t changUserData(uint32_t numIndex, struct UserData *data)
{
	if (numIndex > index.UserNum)return 1;
	uint32_t j = 0;
	for (int i = 0; i < USER_MAX_NUM && j < index.UserNum; i++)
	{
		//如果映射中没有数据
		if (index.UserMap[i].valid == 0)
		{
			continue;
		}
		
		if (j++ == numIndex)
		{
			writeUserData(index.UserMap[i].offset, data);
			return 0;
		}
	}
	return 0xff;
}

void cleanAllUser(void)
{
	
	for (int i = 0; i < USER_MAX_NUM; i++)
	{
		index.UserMap[i].valid = 0;
			
		//更新索引map在flash中的数据,只修改valid位
		STMFLASH_Write(USER_MAP_BASE_ADDR + i * sizeof(struct CardIdToOffsetMap),
						(uint16_t *)(&index.UserMap[i]), sizeof(index.UserMap[i].valid) / sizeof(uint16_t));
	}
	index.UserNum = 0;
	//更新索引中人数在flash中的数据
	STMFLASH_Write(INDEX_BASE_ADDR + ((char *)&index.UserNum - (char *)&index),
					(uint16_t *)(&index.UserNum), sizeof(index.UserNum) / sizeof(uint16_t));
}

//获取已经存在root用户flash中的索引信息并返回对应的指针
void rootDataMapGet(void)
{
	index.RootMap = RootDataMap;
	STMFLASH_Read(INDEX_BASE_ADDR + ((char *)&index.RootNum - (char *)&index),
					(uint16_t *)(&index.RootNum), sizeof(uint32_t) / sizeof(uint16_t));
	struct CardIdToOffsetMap *ptr = RootDataMap;
	for (int i = 0; i < ROOT_MAX_NUM; i++)
	{
		STMFLASH_Read(ROOT_MAP_BASE_ADDR + i * sizeof(struct CardIdToOffsetMap),
					(uint16_t *)(ptr), sizeof(struct CardIdToOffsetMap) / sizeof(uint16_t));
//		printf("rootptr:%p, valid:%d, offset:%d, id:%d\n", ptr, ptr->valid, ptr->offset, ptr->cardID);
		ptr++;
	}
}



//返回值为0则添加成功，否则失败
uint8_t addRootInfo(uint32_t cardID, struct UserData *newData)
{
	uint8_t res = 1;
	if (index.RootNum >= ROOT_MAX_NUM)
	{
		return res;
	}
	
	for (int i = 0; i < ROOT_MAX_NUM; i++)
	{
		if (index.RootMap[i].valid == 0)
		{
			index.RootMap[i].cardID = cardID;
			index.RootMap[i].offset = i * sizeof(struct UserData);
			index.RootMap[i].valid = 1;
			
			index.RootNum++;
			
			//更新索引中人数在flash中的数据
			STMFLASH_Write(INDEX_BASE_ADDR + ((char *)&index.RootNum - (char *)&index),
							(uint16_t *)(&index.RootNum), sizeof(index.RootNum) / sizeof(uint16_t));
			
			//更新索引map在flash中的数据
			STMFLASH_Write(ROOT_MAP_BASE_ADDR + i * sizeof(struct CardIdToOffsetMap),
							(uint16_t *)(&index.RootMap[i]), sizeof(struct CardIdToOffsetMap) / sizeof(uint16_t));
			
			//更新用户数据
			STMFLASH_Write(ROOT_DATA_ADDR + i * sizeof(struct UserData),
							(uint16_t *)newData, sizeof(struct UserData) / sizeof(uint16_t));
			res = 0;
			break;
		}
	}
	return res;
}

//返回值为0则删除成功，否则失败
uint8_t deleteRootInfo(uint32_t cardID)
{
	uint32_t j = 0;
	for (int i = 0; i < ROOT_MAX_NUM && j < index.RootNum; i++)
	{
		//如果映射中没有数据
		if (index.RootMap[i].valid == 0)
		{
			continue;
		}
		
		j++;
		
		if (index.RootMap[i].cardID == cardID)
		{
			index.RootMap[i].valid = 0;
			index.RootNum--;
			
			//更新索引中人数在flash中的数据
			STMFLASH_Write(ROOT_DATA_ADDR, (uint16_t *)(&index.UserNum), sizeof(index.RootNum) / sizeof(uint16_t));
			
			//更新索引map在flash中的数据,只修改valid位
			STMFLASH_Write(ROOT_MAP_BASE_ADDR + i * sizeof(struct CardIdToOffsetMap),
							(uint16_t *)(&index.RootMap[i]), sizeof(index.RootMap[i].valid) / sizeof(uint16_t));
			
			return 0;
		}
	}
	return 1;
}

//当返回 -1 代表没找到该卡
//找到了就返回map中对应的下标
int findRootCard(uint32_t cardID)
{
	uint32_t j = 0;
	for (int i = 0; i < ROOT_MAX_NUM && j < index.RootNum; i++)
	{
		//如果映射中没有数据
		if (index.RootMap[i].valid == 0)
		{
			continue;
		}
		
		j++;
		
		if (index.RootMap[i].cardID == cardID)
		{
			return i;
		}
	}
	return -1;
}

//读取下个有效索引的数据并返回对应下标
int findNextRootData(int start, struct UserData *data)
{
	for (int i = start; i < ROOT_MAX_NUM; i++)
	{
		//如果映射中有数据
		if (index.RootMap[i].valid == 1)
		{
			readRootData(index.RootMap[i].offset, data);
			return i;
		}
		
	}
	return -1;
}

//通过卡号改变用户数据,
//返回0代表修改成功
uint8_t changRootDataByID(uint32_t cardID, struct UserData *data)
{
	int mark = findRootCard(cardID);
	if (mark != -1)
	{
		writeRootData(index.RootMap[mark].offset, data);
		return 0;
	}
	return 0xff;
}

void cleanAllRoot(void)
{
	
	for (int i = 0; i < ROOT_MAX_NUM; i++)
	{
		index.RootMap[i].valid = 0;
			
		//更新索引map在flash中的数据,只修改valid位
		STMFLASH_Write(ROOT_MAP_BASE_ADDR + i * sizeof(struct CardIdToOffsetMap),
						(uint16_t *)(&index.RootMap[i]), sizeof(index.RootMap[i].valid) / sizeof(uint16_t));
	}
	index.RootNum = 0;
	//更新索引中人数在flash中的数据
	STMFLASH_Write(INDEX_BASE_ADDR + ((char *)&index.RootNum - (char *)&index), 
					(uint16_t *)(&index.RootNum), sizeof(index.RootNum) / sizeof(uint16_t));
}

