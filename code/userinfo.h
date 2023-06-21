#ifndef __USERINFO_H
#define __USERINFO_H 

#include <stdint.h>


#define DEBUG 			0

#define USER_DATA_SIZE 		16
#define STUDENT_ID_SIZE 	16	

#define USER_MAX_NUM 		256
#define ROOT_MAX_NUM		16

//flash�ڵ�������������|-index data-|- map -|-empty-|-data-|
#define INDEX_BASE_ADDR		0x08008000 		//���Ҫע�ⲻ�ܸ���map�ļ��е���ʹ�õ�rom
#define ROOT_MAP_BASE_ADDR	0x08008010
#define USER_MAP_BASE_ADDR	0x08008090
#define ROOT_DATA_ADDR  	0x0800A000
#define USER_DATA_ADDR		0x0800B000 		




struct UserData
{	
	uint8_t StudentID[USER_DATA_SIZE];	
};
/***/

struct CardIdToOffsetMap
{
	uint16_t valid;			//��ӳ���Ƿ�����Ч��
	uint16_t offset;		//�������ڹ涨flash�ռ��ƫ��λ��
	uint32_t cardID;		//����
};

struct SupperIndex
{
	uint32_t UserNum;		//�û����ݵĸ���
	uint32_t RootNum;		//����Ա����
	struct CardIdToOffsetMap *UserMap;		//�����û����ݵ�ƫ������
	struct CardIdToOffsetMap *RootMap;		//����Ա�û����ݵ�ƫ������
};

void InfoInit(void);

void userDataMapGet(void);
uint8_t addUserInfo(uint32_t cardID, struct UserData *newData);
uint8_t deleteUserInfo(uint32_t cardID);
uint8_t deleteUser(uint32_t numIndex);
int findUserCard(uint32_t cardID);
int findNextUserData(int start, struct UserData *data);
uint8_t changUserDataByID(uint32_t cardID, struct UserData *data);
uint8_t changUserData(uint32_t numIndex, struct UserData *data);
void cleanAllUser(void);

void rootDataMapGet(void);
uint8_t addRootInfo(uint32_t cardID, struct UserData *newData);
uint8_t deleteRootInfo(uint32_t cardID);
int findRootCard(uint32_t cardID);
int findNextRootData(int start, struct UserData *data);
uint8_t changRootDataByID(uint32_t cardID, struct UserData *data);
void cleanAllRoot(void);


uint32_t getRootNum(void);
uint32_t getUserNum(void);

#endif
