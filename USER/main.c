#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <stdio.h>
#include "usart.h"
#include "delay.h"
#include "userinfo.h"
#include "nokia_5110.h"
#include "lcdlib.h"
#include "rc522.h"
#include "remote.h"
#include "outputio.h"
#include "userinfo.h"
#include "inputDeal.h"
#include "stmflash.h"

#define CREATE_TASK_STACK_SIZE	256
#define LCD_TASK_STACK_SIZE		512
#define REMOTE_TASK_STACK_SIZE	128
#define RC522_TASK_STACK_SIZE	128

#define CREATE_TASK_PRIORITY	10
#define LCD_TASK_PRIORITY		4
#define REMOTE_TASK_PRIORITY	4
#define RC522_TASK_PRIORITY		4

static TaskHandle_t createTaskHandle = NULL;
static TaskHandle_t LcdTaskHandle = NULL;
static TaskHandle_t remoteTaskHandle = NULL;
static TaskHandle_t rc522TaskHandle = NULL;

static void creatTask(void *para);
static void LcdTask(void *para);
static void remoteTask(void *para);
static void rc522Task(void *para);

static QueueHandle_t inputQueue;


void BSP_Init(void);


static void creatTask(void *para)
{
	BSP_Init();
	
	//����LCD�߳�
	xTaskCreate(LcdTask,
                "LCD",
                LCD_TASK_STACK_SIZE,
                NULL,
                LCD_TASK_PRIORITY,
                &LcdTaskHandle);
	
	//���������߳�
	xTaskCreate(remoteTask,
                "remote",
                REMOTE_TASK_STACK_SIZE,
                NULL,
                REMOTE_TASK_PRIORITY,
                &remoteTaskHandle);
	
	//����RC522�߳�
	xTaskCreate(rc522Task,
                "rc522",
                RC522_TASK_STACK_SIZE,
                NULL,
                CREATE_TASK_PRIORITY,
                &rc522TaskHandle);
				
	//������Ϣ����
	inputQueue = xQueueCreate(1, sizeof(struct messegeFrame));			
	
	vTaskDelete(createTaskHandle);
}

static void LcdTask(void *para)
{
	BaseType_t revStatus;
	struct messegeFrame messeg;
	uint8_t page = Standby;
	
	DealLcdShow(&messeg);
	while(1)
	{
		page = getNowPage();
		if (page == menuPage || page == systemInfoPage || page == makerInfo)
		{
			revStatus = xQueueReceive(inputQueue, &messeg, 10000);
		}
		else
		{
			revStatus = xQueueReceive(inputQueue, &messeg, portMAX_DELAY);
		}
		if (revStatus == pdPASS)		//������յ�������
		{
//			sprintf(debug, "re:%u", messeg.data);
//			LCD_write_english_string(0, 2, debug);
			if (messeg.type == REMOTE_TYPE)
			{
				DealRemoteState(messeg.data);
			}
			else if (messeg.type == RC522_TYPE)
			{
				DealRC522State(messeg.data);
			}
		}
		else
		{
			resetLcd();
		}
		DealLcdShow(&messeg);
	}	
}

static void remoteTask(void *para)
{
	struct messegeFrame messeg;
	uint8_t state;
	messeg.type = REMOTE_TYPE;
	while(1)
	{
		vTaskDelay(50);
		state = Remote_Scan();		//ɨ�����ң��
		if (state != 0)				//�������ң�صı�����
		{
			messeg.data = state;
			xQueueSend(inputQueue, &messeg, 0);
		}
	}
}

static void rc522Task(void *para)
{
	//���ڱ��Ž����Կ����ڲ���Կ����У�����Բ���Ҫ��Щ����
	//uint8_t RFID[16];			//���RFID
	//uint8_t CardKey[6] = {0xff,0xff,0xff,0xff,0xff,0xff};	//��ȡ������֤����
	uint32_t cardId = 0;		//����
	uint16_t cardType;
	uint8_t state;	
	struct messegeFrame messeg;
	messeg.type = RC522_TYPE;
	while(1)
	{
		state = MI_ERR;
		vTaskDelay(100);
		//�����е�nfc��
		if (PcdRequest(PICC_REQALL, (uint8_t*)&cardType) == MI_OK)
		{
			state = PcdAnticoll((uint8_t*)&cardId);		//����ײ	
		}
		//����ײ�ɹ�,�ҵ���Ӧ�Ŀ���
		if (state == MI_OK)
		{				
			messeg.data = cardId;
			xQueueSend(inputQueue, &messeg, 0);
//			sprintf(debug, "id:%u", messeg.data);
//			LCD_write_english_string(0, 1, debug);
		}
	}
}

int main()
{
	xTaskCreate(creatTask,
                "creat",
                CREATE_TASK_STACK_SIZE,
                NULL,
                CREATE_TASK_PRIORITY,
                &createTaskHandle);
	vTaskStartScheduler();
	BSP_Init();
	
	while (1)
	{
		
	}
    return 0;
}


void BSP_Init(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�
#if DEBUG 
	uart_init(115200);
#endif
	
	RC522_Init();
	Remote_Init();
	LCD_IO_Init();
	Door_IO_init();
	LED_Init();
	LCD_init();
	LCD_clear();
	
	LCD_write_english_string(0, 0, "loding...");
	
	InfoInit();
}

