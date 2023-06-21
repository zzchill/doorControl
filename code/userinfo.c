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

//����ƫ�ƻ�ȡflash���û�����
void readUserData(uint32_t offset, struct UserData *data)
{
	assert_param(offset % 2 != 1);
	
	STMFLASH_Read(USER_DATA_ADDR + offset, (uint16_t *)(data), sizeof(struct UserData) / sizeof(uint16_t));
}

//����flash���û�����
void writeUserData(uint32_t offset, struct UserData *newData)
{
	assert_param(offset % 2 != 1);
	
	STMFLASH_Write(USER_DATA_ADDR + offset, (uint16_t *)newData, sizeof(struct UserData) / sizeof(uint16_t));
}

//����ƫ�ƻ�ȡflash���û�����
void readRootData(uint32_t offset, struct UserData *data)
{
	assert_param(offset % 2 != 1);
	
	STMFLASH_Read(ROOT_DATA_ADDR + offset, (uint16_t *)(data), sizeof(struct UserData) / sizeof(uint16_t));
}

//����flash���û�����
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

//��ȡ�Ѿ�����flash�е�������Ϣ�����ض�Ӧ��ָ��
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

//����ֵΪ0����ӳɹ�������ʧ��
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
			
			//����������������flash�е�����
			STMFLASH_Write(INDEX_BASE_ADDR + ((char *)&index.UserNum - (char *)&index),
							(uint16_t *)(&(index.UserNum)), sizeof(index.UserNum) / sizeof(uint16_t));
			
			//��������map��flash�е�����
			STMFLASH_Write(USER_MAP_BASE_ADDR + i * sizeof(struct CardIdToOffsetMap),
							(uint16_t *)(&index.UserMap[i]), sizeof(struct CardIdToOffsetMap) / sizeof(uint16_t));
			
			//�����û�����
			STMFLASH_Write(USER_DATA_ADDR + i * sizeof(struct UserData),
							(uint16_t *)newData, sizeof(struct UserData) / sizeof(uint16_t));
			res = 0;
			break;
		}
	}
	return res;
}

//����ֵΪ0��ɾ���ɹ�������ʧ��
uint8_t deleteUserInfo(uint32_t cardID)
{
	uint32_t j = 0;
	for (int i = 0; i < USER_MAX_NUM && j < index.UserNum; i++)
	{
		//���ӳ����û������
		if (index.UserMap[i].valid == 0)
		{
			continue;
		}
		
		j++;
		
		if (index.UserMap[i].cardID == cardID)
		{
			index.UserMap[i].valid = 0;
			index.UserNum--;
			
			//����������������flash�е�����
			STMFLASH_Write(USER_DATA_ADDR, (uint16_t *)(&index.UserNum), sizeof(index.UserNum) / sizeof(uint16_t));
			
			//��������map��flash�е�����,ֻ�޸�validλ
			STMFLASH_Write(USER_MAP_BASE_ADDR + i * sizeof(struct CardIdToOffsetMap),
							(uint16_t *)(&index.UserMap[i]), sizeof(index.UserMap[i].valid) / sizeof(uint16_t));
			
			return 0;
		}
	}
	return 1;
}

//����Ĳ������û��б��еĵ�nλ����0��ʼ
//����ֵΪ0��ɾ���ɹ�������ʧ��
uint8_t deleteUser(uint32_t numIndex)
{
	if (numIndex > index.UserNum)return 1;
	uint32_t j = 0;
	for (int i = 0; i < USER_MAX_NUM && j < index.UserNum; i++)
	{
		//���ӳ����û������
		if (index.UserMap[i].valid == 0)
		{
			continue;
		}
		
		if (j++ == numIndex)
		{
			index.UserMap[i].valid = 0;
			index.UserNum--;
			
			//����������������flash�е�����
			STMFLASH_Write(USER_DATA_ADDR, (uint16_t *)(&index.UserNum), sizeof(index.UserNum) / sizeof(uint16_t));
			
			//��������map��flash�е�����,ֻ�޸�validλ
			STMFLASH_Write(USER_MAP_BASE_ADDR + i * sizeof(struct CardIdToOffsetMap),
							(uint16_t *)(&index.UserMap[i]), sizeof(index.UserMap[i].valid) / sizeof(uint16_t));
			
			return 0;
		}
		
	}
	return 1;
}

//������ -1 ����û�ҵ��ÿ�
//�ҵ��˾ͷ���map�ж�Ӧ���±�
int findUserCard(uint32_t cardID)
{
	uint32_t j = 0;
	for (int i = 0; i < USER_MAX_NUM && j < index.UserNum; i++)
	{
		//���ӳ����û������
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

//��ȡ�¸���Ч���������ݲ����ض�Ӧ�±�,data�Ǵ���Ļ�����
int findNextUserData(int start, struct UserData *data)
{
	for (int i = start; i < USER_MAX_NUM; i++)
	{
		//���ӳ����������
		if (index.UserMap[i].valid == 1)
		{
			readUserData(index.UserMap[i].offset, data);
			return i;
		}
		
	}
	return -1;
}

//ͨ�����Ÿı��û�����,
//����0�����޸ĳɹ�
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


//����Ĳ������û��б��еĵ�nλ����0��ʼ
//����0�����޸ĳɹ�
uint8_t changUserData(uint32_t numIndex, struct UserData *data)
{
	if (numIndex > index.UserNum)return 1;
	uint32_t j = 0;
	for (int i = 0; i < USER_MAX_NUM && j < index.UserNum; i++)
	{
		//���ӳ����û������
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
			
		//��������map��flash�е�����,ֻ�޸�validλ
		STMFLASH_Write(USER_MAP_BASE_ADDR + i * sizeof(struct CardIdToOffsetMap),
						(uint16_t *)(&index.UserMap[i]), sizeof(index.UserMap[i].valid) / sizeof(uint16_t));
	}
	index.UserNum = 0;
	//����������������flash�е�����
	STMFLASH_Write(INDEX_BASE_ADDR + ((char *)&index.UserNum - (char *)&index),
					(uint16_t *)(&index.UserNum), sizeof(index.UserNum) / sizeof(uint16_t));
}

//��ȡ�Ѿ�����root�û�flash�е�������Ϣ�����ض�Ӧ��ָ��
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



//����ֵΪ0����ӳɹ�������ʧ��
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
			
			//����������������flash�е�����
			STMFLASH_Write(INDEX_BASE_ADDR + ((char *)&index.RootNum - (char *)&index),
							(uint16_t *)(&index.RootNum), sizeof(index.RootNum) / sizeof(uint16_t));
			
			//��������map��flash�е�����
			STMFLASH_Write(ROOT_MAP_BASE_ADDR + i * sizeof(struct CardIdToOffsetMap),
							(uint16_t *)(&index.RootMap[i]), sizeof(struct CardIdToOffsetMap) / sizeof(uint16_t));
			
			//�����û�����
			STMFLASH_Write(ROOT_DATA_ADDR + i * sizeof(struct UserData),
							(uint16_t *)newData, sizeof(struct UserData) / sizeof(uint16_t));
			res = 0;
			break;
		}
	}
	return res;
}

//����ֵΪ0��ɾ���ɹ�������ʧ��
uint8_t deleteRootInfo(uint32_t cardID)
{
	uint32_t j = 0;
	for (int i = 0; i < ROOT_MAX_NUM && j < index.RootNum; i++)
	{
		//���ӳ����û������
		if (index.RootMap[i].valid == 0)
		{
			continue;
		}
		
		j++;
		
		if (index.RootMap[i].cardID == cardID)
		{
			index.RootMap[i].valid = 0;
			index.RootNum--;
			
			//����������������flash�е�����
			STMFLASH_Write(ROOT_DATA_ADDR, (uint16_t *)(&index.UserNum), sizeof(index.RootNum) / sizeof(uint16_t));
			
			//��������map��flash�е�����,ֻ�޸�validλ
			STMFLASH_Write(ROOT_MAP_BASE_ADDR + i * sizeof(struct CardIdToOffsetMap),
							(uint16_t *)(&index.RootMap[i]), sizeof(index.RootMap[i].valid) / sizeof(uint16_t));
			
			return 0;
		}
	}
	return 1;
}

//������ -1 ����û�ҵ��ÿ�
//�ҵ��˾ͷ���map�ж�Ӧ���±�
int findRootCard(uint32_t cardID)
{
	uint32_t j = 0;
	for (int i = 0; i < ROOT_MAX_NUM && j < index.RootNum; i++)
	{
		//���ӳ����û������
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

//��ȡ�¸���Ч���������ݲ����ض�Ӧ�±�
int findNextRootData(int start, struct UserData *data)
{
	for (int i = start; i < ROOT_MAX_NUM; i++)
	{
		//���ӳ����������
		if (index.RootMap[i].valid == 1)
		{
			readRootData(index.RootMap[i].offset, data);
			return i;
		}
		
	}
	return -1;
}

//ͨ�����Ÿı��û�����,
//����0�����޸ĳɹ�
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
			
		//��������map��flash�е�����,ֻ�޸�validλ
		STMFLASH_Write(ROOT_MAP_BASE_ADDR + i * sizeof(struct CardIdToOffsetMap),
						(uint16_t *)(&index.RootMap[i]), sizeof(index.RootMap[i].valid) / sizeof(uint16_t));
	}
	index.RootNum = 0;
	//����������������flash�е�����
	STMFLASH_Write(INDEX_BASE_ADDR + ((char *)&index.RootNum - (char *)&index), 
					(uint16_t *)(&index.RootNum), sizeof(index.RootNum) / sizeof(uint16_t));
}

