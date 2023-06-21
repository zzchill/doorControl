#include "nokia_5110.h"
#include "lcdlib.h"
#include "bmp_pixel.h"
#include "write_chinese_string_pixel.h"
#include "english_6x8_pixel.h"

/*-----------------------------------------------------------------------
LCD_init          : 3310LCD��ʼ��

��д����          ��2004-8-10 
����޸�����      ��2004-8-10 
-----------------------------------------------------------------------*/

void LCD_IO_Init(void)
{
 
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);	 //ʹ��PB,PE�˿�ʱ��
	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13| GPIO_Pin_12| GPIO_Pin_14|GPIO_Pin_10| GPIO_Pin_11;				 //LED0-->PB.5 �˿�����
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
 GPIO_Init(GPIOB, &GPIO_InitStructure);					 //�����趨������ʼ��GPIOB.5
	
	//GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10| GPIO_Pin_11;				 //LED0-->PB.5 �˿�����
	//GPIO_Init(GPIOB, &GPIO_InitStructure);	
	
	
	
	GPIO_SetBits(GPIOB,GPIO_Pin_14);						 //PB.5 �����
	GPIO_SetBits(GPIOB,GPIO_Pin_12);
	GPIO_SetBits(GPIOB,GPIO_Pin_13);
	GPIO_SetBits(GPIOB,GPIO_Pin_10);
	GPIO_SetBits(GPIOB,GPIO_Pin_11);

}
 

void delay_1us(void)                 //1us��ʱ����
{
	volatile unsigned int t;
	t = 11;
    while (t != 0)
    {
      t--;
    }
}

void delay_1ms(void)                 //1ms��ʱ����
{
	delay_nus(1000);
}
  
void delay_nus(unsigned int n)       //N us��ʱ����
{
  volatile unsigned int num;
  volatile unsigned int t;
 
  for (num = 0; num < n; num++)
  {
    t = 11;
    while (t != 0)
    {
      t--;
    }
  }
}

void delay_nms(unsigned int n)       //N ms��ʱ����
{
	volatile unsigned int num;
	for (num = 0; num < n; num++)
	{
		delay_nus(1000);
	}
}


void LCD_init(void)
  {
            // ����һ����LCD��λ�ĵ͵�ƽ����
   LCD_RST = 0;
    delay_1us();

   LCD_RST = 1;
    
		// �ر�LCD
   LCD_CE = 0;
    delay_1us();
		// ʹ��LCD
   LCD_CE = 1;
    delay_1us();

    LCD_write_byte(0x21, 0);	// ʹ����չ��������LCDģʽ
    LCD_write_byte(0xc8, 0);	// ����ƫ�õ�ѹ
    LCD_write_byte(0x06, 0);	// �¶�У��
    LCD_write_byte(0x13, 0);	// 1:48
    LCD_write_byte(0x20, 0);	// ʹ�û�������
    LCD_clear();	        // ����
    LCD_write_byte(0x0c, 0);	// �趨��ʾģʽ��������ʾ
        
           // �ر�LCD
   LCD_CE = 0;
  }

/*-----------------------------------------------------------------------
LCD_clear         : LCD��������

��д����          ��2004-8-10 
����޸�����      ��2004-8-10 
-----------------------------------------------------------------------*/
void LCD_clear(void)
  {
    unsigned int i;

    LCD_write_byte(0x0c, 0);			
    LCD_write_byte(0x80, 0);			

    for (i=0; i<504; i++)
      LCD_write_byte(0, 1);			
  }

/*-----------------------------------------------------------------------
LCD_set_XY        : ����LCD���꺯��

���������X       ��0��83
          Y       ��0��5

��д����          ��2004-8-10 
����޸�����      ��2004-8-10 
-----------------------------------------------------------------------*/
void LCD_set_XY(unsigned char X, unsigned char Y)
  {
    LCD_write_byte(0x40 | Y, 0);		// column
    LCD_write_byte(0x80 | X, 0);          	// row
  }

/*-----------------------------------------------------------------------
LCD_write_char    : ��ʾӢ���ַ�

���������c       ����ʾ���ַ���

��д����          ��2004-8-10 
����޸�����      ��2004-8-10 
-----------------------------------------------------------------------*/
void LCD_write_char(unsigned char c)
  {
    unsigned char line;

    c -= 32;

    for (line=0; line<6; line++)
      LCD_write_byte(font6x8[c][line], 1);
  }

/*-----------------------------------------------------------------------
LCD_write_english_String  : Ӣ���ַ�����ʾ����

���������*s      ��Ӣ���ַ���ָ�룻
          X��Y    : ��ʾ�ַ�����λ��,x 0-83 ,y 0-5

��д����          ��2004-8-10 
����޸�����      ��2004-8-10 		
-----------------------------------------------------------------------*/
void LCD_write_english_string(unsigned char X,unsigned char Y,char *s)
  {
    LCD_set_XY(X,Y);
    while (*s) 
      {
	 LCD_write_char(*s);
	 s++;
      }
  }
/*-----------------------------------------------------------------------
LCD_write_chinese_string: ��LCD����ʾ����

���������X��Y    ����ʾ���ֵ���ʼX��Y���ꣻ
          ch_with �����ֵ���Ŀ��
          num     ����ʾ���ֵĸ�����  
          line    �����ֵ��������е���ʼ����
          row     ��������ʾ���м��
��д����          ��2004-8-11 
����޸�����      ��2004-8-12 
���ԣ�
	LCD_write_chi(0,0,12,7,0,0);
	LCD_write_chi(0,2,12,7,0,0);
	LCD_write_chi(0,4,12,7,0,0);	
-----------------------------------------------------------------------*/                        
void LCD_write_chinese_string(unsigned char X, unsigned char Y, 
                   unsigned char ch_with,unsigned char num,
                   unsigned char line,unsigned char row)
  {
    unsigned char i,n;
    
    LCD_set_XY(X,Y);                             //���ó�ʼλ��
    
    for (i=0;i<num;)
      {
      	for (n=0; n<ch_with*2; n++)              //дһ������
      	  { 
      	    if (n==ch_with)                      //д���ֵ��°벿��
      	      {
      	        if (i==0) LCD_set_XY(X,Y+1);
      	        else
      	           LCD_set_XY((X+(ch_with+row)*i),Y+1);
              }
      	    LCD_write_byte(write_chinese[line+i][n],1);
      	  }
      	i++;
      	LCD_set_XY((X+(ch_with+row)*i),Y);
      }
  }
  


/*-----------------------------------------------------------------------
LCD_draw_map      : λͼ���ƺ���

���������X��Y    ��λͼ���Ƶ���ʼX��Y���ꣻ
          *map    ��λͼ�������ݣ�
          Pix_x   ��λͼ���أ�����
          Pix_y   ��λͼ���أ���

��д����          ��2004-8-13
����޸�����      ��2004-8-13 
-----------------------------------------------------------------------*/
void LCD_draw_bmp_pixel(unsigned char X,unsigned char Y,unsigned char *map,
                  unsigned char Pix_x,unsigned char Pix_y)
  {
    unsigned int i,n;
    unsigned char row;
    
    if (Pix_y%8==0) row=Pix_y/8;      //����λͼ��ռ����
      else
        row=Pix_y/8+1;
    
    for (n=0;n<row;n++)
      {
      	LCD_set_XY(X,Y);
        for(i=0; i<Pix_x; i++)
          {
            LCD_write_byte(map[i+n*Pix_x], 1);
          }
        Y++;                         //����
      }      
  }

/*-----------------------------------------------------------------------
LCD_write_byte    : ʹ��SPI�ӿ�д���ݵ�LCD

���������data    ��д������ݣ�
          command ��д����/����ѡ��

��д����          ��2004-8-10 
����޸�����      ��2004-8-13 
-----------------------------------------------------------------------*/
void LCD_write_byte(unsigned char dat, unsigned char command)
  {
    unsigned char i;
    //PORTB &= ~LCD_CE ;		        // ʹ��LCD
    LCD_CE = 0;
    
    if (command == 0)
     // PORTB &= ~LCD_DC ;	        // ��������
     LCD_DC = 0;
    else
     // PORTB |= LCD_DC ;		        // ��������
     LCD_DC = 1;
		for(i=0;i<8;i++)
		{
			if(dat&0x80)
				SDIN = 1;
			else
				SDIN = 0;
			SCLK = 0;
			dat = dat << 1;
			SCLK = 1;
		}
   // SPDR = data;			// �������ݵ�SPI�Ĵ���

    //while ((SPSR & 0x80) == 0);         // �ȴ����ݴ������
	
    //PORTB |= LCD_CE ;			// �ر�LCD
     LCD_CE = 1;
  }

void LCD_Send_Data(unsigned char byte)
{
    unsigned char i;
	
    LCD_CE = 0;				//????
    LCD_DC = 1;	      		//?????
	
    for(i=0;i<8;i++)		//??1byte??
    { 
        if(byte&0x80)
     	   SDIN = 1;
		else
       		SDIN = 0;

        SCLK = 0;
        byte = byte<<1;
        SCLK = 1;
    }
	
    LCD_CE = 1;				//????
}
void LCD_Send_Commond(unsigned char byte)
{
    unsigned char i;

    LCD_CE = 0;				//????
    LCD_DC = 0;	      		//?????
	
    for(i=0;i<8;i++)		//??1byte??
    { 
        if(byte&0x80)
        	SDIN = 1;
		else
			SDIN = 0;
	
		SCLK = 0;
		byte = byte << 1;
		SCLK = 1;
    }
	
    LCD_CE = 1;				//????
}
void LCD_Set_XY(unsigned char x, unsigned char y)
{
    LCD_Send_Commond(y | 0x40);
    LCD_Send_Commond(x | 0x80);
}

void LCD_Show_Pic(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t *pic)
{
	uint8_t i, j, row;
	if(height>48)
		height=48;
	if(height%8)
		row = height/8+1;
	else
		row = height/8;
	for(j=0; j<row; j++)
	{
		LCD_Set_XY(x,y+j);
		for(i=0; i<width; i++)
		{
			LCD_Send_Data(*pic);
			pic++;
		}
    }
}



