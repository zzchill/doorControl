#include "sys.h"
#include "rc522.h"
#include "delay.h"
#include "usart.h"
#include "string.h"
#include "stmflash.h"
#include "nokia_5110.h"
#include "outputio.h"
#include "userinfo.h"
#include "remote.h"
#include "key.h"

#define SIZE sizeof(u16)
#define SIZE_NUM sizeof(NUM)	
extern struct USER student[USER_MAX_SIZE];
//////////////////////////////////////////////////////////
// M1卡分为16个扇区，每个扇区由四个块（块0、块1、块2、块3）组成
// 将16个扇区的64个块按绝对地址编号为：0~63
// 第0个扇区的块0（即绝对地址0块），用于存放厂商代码，已经固化不可更改
// 每个扇区的块0、块1、块2为数据块，可用于存放数据
// 每个扇区的块3为控制块（绝对地址为:块3、块7、块11.....）包括密码A，存取控制、密码B等

u8 rootkey=0;
u16 NUM=0;
u16 NUM_ROOT=0;
/*******************************
*连线说明：
*1--SDA  <----->PA4
*2--SCK  <----->PA5
*3--MOSI <----->PA7
*4--MISO <----->PA6
*5--悬空
*6--GND <----->GND
*7--RST <----->PB0
*8--VCC <----->VCC
************************************/

/*全局变量*/
unsigned char CT[2];//卡类型
unsigned char SN[4]; //卡号
unsigned char RFID[16];			//存放RFID
unsigned char lxl_bit=0;
unsigned char card1_bit=0;
unsigned char card2_bit=0;
unsigned char card3_bit=0;
unsigned char card4_bit=0;
unsigned char total=0;

/************root******************/
u8 root[NUM_ROOT_MAX][4]={0};

unsigned char lxl[4]= {196,58,104,217};
unsigned char card_1[4]= {0x7b,0x56,0x90,0xab};
unsigned char card_2[4]= {208,121,31,57};
unsigned char card_3[4]= {176,177,143,165};
unsigned char card_4[4]= {5,158,10,136};
/************************************/
u8 KEY[6]= {0xff,0xff,0xff,0xff,0xff,0xff};
u8 AUDIO_OPEN[6] = {0xAA, 0x07, 0x02, 0x00, 0x09, 0xBC};
unsigned char RFID1[16]= {0x00,0x00,0x00,0x00,0x00,0x00,0xff,0x07,0x80,0x29,0xff,0xff,0xff,0xff,0xff,0xff};
/*函数声明*/
unsigned char status;
unsigned char s=0x08;


#define   RC522_DELAY()  delay_us( 20 )


void RC522_Handel(void)
{

    status = PcdRequest(PICC_REQALL,CT);//寻卡

    //printf("\r\nstatus>>>>>>%d\r\n", status);

    if(status==MI_OK)// 寻卡成功
    {
        status=MI_ERR;
        status = PcdAnticoll(SN);// 防冲撞
    }

    if (status==MI_OK)// 防冲撞成功
    {
        status=MI_ERR;
        ShowID(SN); // 串口打印卡的ID号
				

        //total=card1_bit+card2_bit+card3_bit+card4_bit+lxl_bit;
        status =PcdSelect(SN);

    }
    else
    {

    }
    if(status==MI_OK)//选卡成功
    {

        status=MI_ERR;
        status =PcdAuthState(0x60,0x09,KEY,SN);
    }
    if(status==MI_OK)//验证成功
    {
        status=MI_ERR;
        status=PcdRead(s,RFID);
    }

    if(status==MI_OK)//读卡成功
    {
        status=MI_ERR;
        delay_ms(100);
    }

}

void RC522_Init ( void )
{
    SPI1_Init();

    RC522_Reset_Disable();

    RC522_CS_Disable();

    PcdReset ();

    M500PcdConfigISOType ( 'A' );//设置工作方式

}

void SPI1_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE );//PORTA、B时钟使能

    // CS
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
    GPIO_Init(GPIOA, &GPIO_InitStructure);					 //根据设定参数初始化PF0、PF1

    // SCK
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // MISO
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 		 //推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // MOSI
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // RST
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
    GPIO_Init(GPIOB, &GPIO_InitStructure);

}


/*
 * 函数名：SPI_RC522_SendByte
 * 描述  ：向RC522发送1 Byte 数据
 * 输入  ：byte，要发送的数据
 * 返回  : RC522返回的数据
 * 调用  ：内部调用
 */
void SPI_RC522_SendByte ( u8 byte )
{
    u8 counter;

    for(counter=0; counter<8; counter++)
    {
        if ( byte & 0x80 )
            RC522_MOSI_1 ();
        else
            RC522_MOSI_0 ();

        RC522_DELAY();

        RC522_SCK_0 ();

        RC522_DELAY();

        RC522_SCK_1();

        RC522_DELAY();

        byte <<= 1;

    }

}


/*
 * 函数名：SPI_RC522_ReadByte
 * 描述  ：从RC522发送1 Byte 数据
 * 输入  ：无
 * 返回  : RC522返回的数据
 * 调用  ：内部调用
 */
u8 SPI_RC522_ReadByte ( void )
{
    u8 counter;
    u8 SPI_Data;


    for(counter=0; counter<8; counter++)
    {
        SPI_Data <<= 1;

        RC522_SCK_0 ();

        RC522_DELAY();

        if ( RC522_MISO_GET() == 1)
            SPI_Data |= 0x01;

        RC522_DELAY();

        RC522_SCK_1 ();

        RC522_DELAY();

    }


//	printf("****%c****",SPI_Data);
    return SPI_Data;
}


/*
 * 函数名：ReadRawRC
 * 描述  ：读RC522寄存器
 * 输入  ：ucAddress，寄存器地址
 * 返回  : 寄存器的当前值
 * 调用  ：内部调用
 */
u8 ReadRawRC ( u8 ucAddress )
{
    u8 ucAddr, ucReturn;


    ucAddr = ( ( ucAddress << 1 ) & 0x7E ) | 0x80;

    RC522_CS_Enable();

    SPI_RC522_SendByte ( ucAddr );

    ucReturn = SPI_RC522_ReadByte ();

    RC522_CS_Disable();

    return ucReturn;
}


/*
 * 函数名：WriteRawRC
 * 描述  ：写RC522寄存器
 * 输入  ：ucAddress，寄存器地址
 *         ucValue，写入寄存器的值
 * 返回  : 无
 * 调用  ：内部调用
 */
void WriteRawRC ( u8 ucAddress, u8 ucValue )
{
    u8 ucAddr;

    ucAddr = ( ucAddress << 1 ) & 0x7E;

    RC522_CS_Enable();

    SPI_RC522_SendByte ( ucAddr );

    SPI_RC522_SendByte ( ucValue );

    RC522_CS_Disable();
}


/*
 * 函数名：SetBitMask
 * 描述  ：对RC522寄存器置位
 * 输入  ：ucReg，寄存器地址
 *         ucMask，置位值
 * 返回  : 无
 * 调用  ：内部调用
 */
void SetBitMask ( u8 ucReg, u8 ucMask )
{
    u8 ucTemp;

    ucTemp = ReadRawRC ( ucReg );

    WriteRawRC ( ucReg, ucTemp | ucMask );         // set bit mask

}


/*
 * 函数名：ClearBitMask
 * 描述  ：对RC522寄存器清位
 * 输入  ：ucReg，寄存器地址
 *         ucMask，清位值
 * 返回  : 无
 * 调用  ：内部调用
 */
void ClearBitMask ( u8 ucReg, u8 ucMask )
{
    u8 ucTemp;

    ucTemp = ReadRawRC ( ucReg );

    WriteRawRC ( ucReg, ucTemp & ( ~ ucMask) );  // clear bit mask

}


/*
 * 函数名：PcdAntennaOn
 * 描述  ：开启天线
 * 输入  ：无
 * 返回  : 无
 * 调用  ：内部调用
 */
void PcdAntennaOn ( void )
{
    u8 uc;

    uc = ReadRawRC ( TxControlReg );

    if ( ! ( uc & 0x03 ) )
        SetBitMask(TxControlReg, 0x03);

}


/*
 * 函数名：PcdAntennaOff
 * 描述  ：关闭天线
 * 输入  ：无
 * 返回  : 无
 * 调用  ：内部调用
 */
void PcdAntennaOff ( void )
{

    ClearBitMask ( TxControlReg, 0x03 );

}


/*
 * 函数名：PcdRese
 * 描述  ：复位RC522
 * 输入  ：无
 * 返回  : 无
 * 调用  ：外部调用
 */
void PcdReset ( void )
{
    RC522_Reset_Disable();

    delay_us ( 1 );

    RC522_Reset_Enable();

    delay_us ( 1 );

    RC522_Reset_Disable();

    delay_us ( 1 );

    WriteRawRC ( CommandReg, 0x0f );

    while ( ReadRawRC ( CommandReg ) & 0x10 );

    delay_us ( 1 );

    WriteRawRC ( ModeReg, 0x3D );            //定义发送和接收常用模式 和Mifare卡通讯，CRC初始值0x6363

    WriteRawRC ( TReloadRegL, 30 );          //16位定时器低位
    WriteRawRC ( TReloadRegH, 0 );			 //16位定时器高位

    WriteRawRC ( TModeReg, 0x8D );		      //定义内部定时器的设置

    WriteRawRC ( TPrescalerReg, 0x3E );			 //设置定时器分频系数

    WriteRawRC ( TxAutoReg, 0x40 );				   //调制发送信号为100%ASK


}


/*
 * 函数名：M500PcdConfigISOType
 * 描述  ：设置RC522的工作方式
 * 输入  ：ucType，工作方式
 * 返回  : 无
 * 调用  ：外部调用
 */
void M500PcdConfigISOType ( u8 ucType )
{
    if ( ucType == 'A')                     //ISO14443_A
    {
        ClearBitMask ( Status2Reg, 0x08 );

        WriteRawRC ( ModeReg, 0x3D );//3F

        WriteRawRC ( RxSelReg, 0x86 );//84

        WriteRawRC( RFCfgReg, 0x7F );   //4F

        WriteRawRC( TReloadRegL, 30 );//tmoLength);// TReloadVal = 'h6a =tmoLength(dec)

        WriteRawRC ( TReloadRegH, 0 );

        WriteRawRC ( TModeReg, 0x8D );

        WriteRawRC ( TPrescalerReg, 0x3E );

        delay_us ( 2 );

        PcdAntennaOn ();//开天线

    }

}


/*
 * 函数名：PcdComMF522
 * 描述  ：通过RC522和ISO14443卡通讯
 * 输入  ：ucCommand，RC522命令字
 *         pInData，通过RC522发送到卡片的数据
 *         ucInLenByte，发送数据的字节长度
 *         pOutData，接收到的卡片返回数据
 *         pOutLenBit，返回数据的位长度
 * 返回  : 状态值
 *         = MI_OK，成功
 * 调用  ：内部调用
 */
char PcdComMF522 ( u8 ucCommand, u8 * pInData, u8 ucInLenByte, u8 * pOutData, u32 * pOutLenBit )
{
    char cStatus = MI_ERR;
    u8 ucIrqEn   = 0x00;
    u8 ucWaitFor = 0x00;
    u8 ucLastBits;
    u8 ucN;
    u32 ul;

    switch ( ucCommand )
    {
    case PCD_AUTHENT:		//Mifare认证
        ucIrqEn   = 0x12;		//允许错误中断请求ErrIEn  允许空闲中断IdleIEn
        ucWaitFor = 0x10;		//认证寻卡等待时候 查询空闲中断标志位
        break;

    case PCD_TRANSCEIVE:		//接收发送 发送接收
        ucIrqEn   = 0x77;		//允许TxIEn RxIEn IdleIEn LoAlertIEn ErrIEn TimerIEn
        ucWaitFor = 0x30;		//寻卡等待时候 查询接收中断标志位与 空闲中断标志位
        break;

    default:
        break;

    }

    WriteRawRC ( ComIEnReg, ucIrqEn | 0x80 );		//IRqInv置位管脚IRQ与Status1Reg的IRq位的值相反
    ClearBitMask ( ComIrqReg, 0x80 );			//Set1该位清零时，CommIRqReg的屏蔽位清零
    WriteRawRC ( CommandReg, PCD_IDLE );		//写空闲命令
    SetBitMask ( FIFOLevelReg, 0x80 );			//置位FlushBuffer清除内部FIFO的读和写指针以及ErrReg的BufferOvfl标志位被清除

    for ( ul = 0; ul < ucInLenByte; ul ++ )
        WriteRawRC ( FIFODataReg, pInData [ ul ] );    		//写数据进FIFOdata

    WriteRawRC ( CommandReg, ucCommand );					//写命令


    if ( ucCommand == PCD_TRANSCEIVE )
        SetBitMask(BitFramingReg,0x80);  				//StartSend置位启动数据发送 该位与收发命令使用时才有效

    ul = 1000;//根据时钟频率调整，操作M1卡最大等待时间25ms

    do 														//认证 与寻卡等待时间
    {
        ucN = ReadRawRC ( ComIrqReg );							//查询事件中断
        ul --;
    } while ( ( ul != 0 ) && ( ! ( ucN & 0x01 ) ) && ( ! ( ucN & ucWaitFor ) ) );		//退出条件i=0,定时器中断，与写空闲命令

    ClearBitMask ( BitFramingReg, 0x80 );					//清理允许StartSend位

    if ( ul != 0 )
    {
        if ( ! (( ReadRawRC ( ErrorReg ) & 0x1B )) )			//读错误标志寄存器BufferOfI CollErr ParityErr ProtocolErr
        {
            cStatus = MI_OK;

            if ( ucN & ucIrqEn & 0x01 )					//是否发生定时器中断
                cStatus = MI_NOTAGERR;

            if ( ucCommand == PCD_TRANSCEIVE )
            {
                ucN = ReadRawRC ( FIFOLevelReg );			//读FIFO中保存的字节数

                ucLastBits = ReadRawRC ( ControlReg ) & 0x07;	//最后接收到得字节的有效位数

                if ( ucLastBits )
                    * pOutLenBit = ( ucN - 1 ) * 8 + ucLastBits;   	//N个字节数减去1（最后一个字节）+最后一位的位数 读取到的数据总位数
                else
                    * pOutLenBit = ucN * 8;   					//最后接收到的字节整个字节有效

                if ( ucN == 0 )
                    ucN = 1;

                if ( ucN > MAXRLEN )
                    ucN = MAXRLEN;

                for ( ul = 0; ul < ucN; ul ++ )
                    pOutData [ ul ] = ReadRawRC ( FIFODataReg );
            }
        }
        else
            cStatus = MI_ERR;
//			printf(ErrorReg);
    }

    SetBitMask ( ControlReg, 0x80 );           // stop timer now
    WriteRawRC ( CommandReg, PCD_IDLE );

    return cStatus;

}


/*
 * 函数名：PcdRequest
 * 描述  ：寻卡
 * 输入  ：ucReq_code，寻卡方式
 *                     = 0x52，寻感应区内所有符合14443A标准的卡
 *                     = 0x26，寻未进入休眠状态的卡
 *         pTagType，卡片类型代码
 *                   = 0x4400，Mifare_UltraLight
 *                   = 0x0400，Mifare_One(S50)
 *                   = 0x0200，Mifare_One(S70)
 *                   = 0x0800，Mifare_Pro(X))
 *                   = 0x4403，Mifare_DESFire
 * 返回  : 状态值
 *         = MI_OK，成功
 * 调用  ：外部调用
 */
char PcdRequest ( u8 ucReq_code, u8 * pTagType )
{
    char cStatus;
    u8 ucComMF522Buf [ MAXRLEN ];
    u32 ulLen;

    ClearBitMask ( Status2Reg, 0x08 );	//清理指示MIFARECyptol单元接通以及所有卡的数据通信被加密的情况
    WriteRawRC ( BitFramingReg, 0x07 );	//	发送的最后一个字节的 七位
    SetBitMask ( TxControlReg, 0x03 );	//TX1,TX2管脚的输出信号传递经发送调制的13.56的能量载波信号

    ucComMF522Buf [ 0 ] = ucReq_code;		//存入 卡片命令字

    cStatus = PcdComMF522 ( PCD_TRANSCEIVE,	ucComMF522Buf, 1, ucComMF522Buf, & ulLen );	//寻卡

    if ( ( cStatus == MI_OK ) && ( ulLen == 0x10 ) )	//寻卡成功返回卡类型
    {
        * pTagType = ucComMF522Buf [ 0 ];
        * ( pTagType + 1 ) = ucComMF522Buf [ 1 ];
    }

    else
        cStatus = MI_ERR;

    return cStatus;

}


/*
 * 函数名：PcdAnticoll
 * 描述  ：防冲撞
 * 输入  ：pSnr，卡片序列号，4字节
 * 返回  : 状态值
 *         = MI_OK，成功
 * 调用  ：外部调用
 */
char PcdAnticoll ( u8 * pSnr )
{
    char cStatus;
    u8 uc, ucSnr_check = 0;
    u8 ucComMF522Buf [ MAXRLEN ];
    u32 ulLen;

    ClearBitMask ( Status2Reg, 0x08 );		//清MFCryptol On位 只有成功执行MFAuthent命令后，该位才能置位
    WriteRawRC ( BitFramingReg, 0x00);		//清理寄存器 停止收发
    ClearBitMask ( CollReg, 0x80 );			//清ValuesAfterColl所有接收的位在冲突后被清除

    ucComMF522Buf [ 0 ] = 0x93;	//卡片防冲突命令
    ucComMF522Buf [ 1 ] = 0x20;

    cStatus = PcdComMF522 ( PCD_TRANSCEIVE, ucComMF522Buf, 2, ucComMF522Buf, & ulLen);//与卡片通信

    if ( cStatus == MI_OK)		//通信成功
    {
        for ( uc = 0; uc < 4; uc ++ )
        {
            * ( pSnr + uc )  = ucComMF522Buf [ uc ];			//读出UID
            ucSnr_check ^= ucComMF522Buf [ uc ];
        }

        if ( ucSnr_check != ucComMF522Buf [ uc ] )
            cStatus = MI_ERR;

    }

    SetBitMask ( CollReg, 0x80 );

    return cStatus;

}


/*
 * 函数名：CalulateCRC
 * 描述  ：用RC522计算CRC16
 * 输入  ：pIndata，计算CRC16的数组
 *         ucLen，计算CRC16的数组字节长度
 *         pOutData，存放计算结果存放的首地址
 * 返回  : 无
 * 调用  ：内部调用
 */
void CalulateCRC ( u8 * pIndata, u8 ucLen, u8 * pOutData )
{
    u8 uc, ucN;

    ClearBitMask(DivIrqReg,0x04);

    WriteRawRC(CommandReg,PCD_IDLE);

    SetBitMask(FIFOLevelReg,0x80);

    for ( uc = 0; uc < ucLen; uc ++)
        WriteRawRC ( FIFODataReg, * ( pIndata + uc ) );

    WriteRawRC ( CommandReg, PCD_CALCCRC );

    uc = 0xFF;

    do
    {
        ucN = ReadRawRC ( DivIrqReg );
        uc --;
    } while ( ( uc != 0 ) && ! ( ucN & 0x04 ) );

    pOutData [ 0 ] = ReadRawRC ( CRCResultRegL );
    pOutData [ 1 ] = ReadRawRC ( CRCResultRegM );

}


/*
 * 函数名：PcdSelect
 * 描述  ：选定卡片
 * 输入  ：pSnr，卡片序列号，4字节
 * 返回  : 状态值
 *         = MI_OK，成功
 * 调用  ：外部调用
 */
char PcdSelect ( u8 * pSnr )
{
    char ucN;
    u8 uc;
    u8 ucComMF522Buf [ MAXRLEN ];
    u32  ulLen;

    ucComMF522Buf [ 0 ] = PICC_ANTICOLL1;
    ucComMF522Buf [ 1 ] = 0x70;
    ucComMF522Buf [ 6 ] = 0;

    for ( uc = 0; uc < 4; uc ++ )
    {
        ucComMF522Buf [ uc + 2 ] = * ( pSnr + uc );
        ucComMF522Buf [ 6 ] ^= * ( pSnr + uc );
    }

    CalulateCRC ( ucComMF522Buf, 7, & ucComMF522Buf [ 7 ] );

    ClearBitMask ( Status2Reg, 0x08 );

    ucN = PcdComMF522 ( PCD_TRANSCEIVE, ucComMF522Buf, 9, ucComMF522Buf, & ulLen );

    if ( ( ucN == MI_OK ) && ( ulLen == 0x18 ) )
        ucN = MI_OK;
    else
        ucN = MI_ERR;

    return ucN;

}


/*
 * 函数名：PcdAuthState
 * 描述  ：验证卡片密码
 * 输入  ：ucAuth_mode，密码验证模式
 *                     = 0x60，验证A密钥
 *                     = 0x61，验证B密钥
 *         u8 ucAddr，块地址
 *         pKey，密码
 *         pSnr，卡片序列号，4字节
 * 返回  : 状态值
 *         = MI_OK，成功
 * 调用  ：外部调用
 */
char PcdAuthState ( u8 ucAuth_mode, u8 ucAddr, u8 * pKey, u8 * pSnr )
{
    char cStatus;
    u8 uc, ucComMF522Buf [ MAXRLEN ];
    u32 ulLen;

    ucComMF522Buf [ 0 ] = ucAuth_mode;
    ucComMF522Buf [ 1 ] = ucAddr;

    for ( uc = 0; uc < 6; uc ++ )
        ucComMF522Buf [ uc + 2 ] = * ( pKey + uc );

    for ( uc = 0; uc < 6; uc ++ )
        ucComMF522Buf [ uc + 8 ] = * ( pSnr + uc );

    cStatus = PcdComMF522 ( PCD_AUTHENT, ucComMF522Buf, 12, ucComMF522Buf, & ulLen );

    if ( ( cStatus != MI_OK ) || ( ! ( ReadRawRC ( Status2Reg ) & 0x08 ) ) )

    {
//			if(cStatus != MI_OK)
//					printf("666")	;
//			else
//				printf("888");
        cStatus = MI_ERR;
    }

    return cStatus;

}


/*
 * 函数名：PcdWrite
 * 描述  ：写数据到M1卡一块
 * 输入  ：u8 ucAddr，块地址
 *         pData，写入的数据，16字节
 * 返回  : 状态值
 *         = MI_OK，成功
 * 调用  ：外部调用
 */
char PcdWrite ( u8 ucAddr, u8 * pData )
{
    char cStatus;
    u8 uc, ucComMF522Buf [ MAXRLEN ];
    u32 ulLen;

    ucComMF522Buf [ 0 ] = PICC_WRITE;
    ucComMF522Buf [ 1 ] = ucAddr;

    CalulateCRC ( ucComMF522Buf, 2, & ucComMF522Buf [ 2 ] );

    cStatus = PcdComMF522 ( PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, & ulLen );

    if ( ( cStatus != MI_OK ) || ( ulLen != 4 ) || ( ( ucComMF522Buf [ 0 ] & 0x0F ) != 0x0A ) )
        cStatus = MI_ERR;

    if ( cStatus == MI_OK )
    {
        memcpy(ucComMF522Buf, pData, 16);
        for ( uc = 0; uc < 16; uc ++ )
            ucComMF522Buf [ uc ] = * ( pData + uc );

        CalulateCRC ( ucComMF522Buf, 16, & ucComMF522Buf [ 16 ] );

        cStatus = PcdComMF522 ( PCD_TRANSCEIVE, ucComMF522Buf, 18, ucComMF522Buf, & ulLen );

        if ( ( cStatus != MI_OK ) || ( ulLen != 4 ) || ( ( ucComMF522Buf [ 0 ] & 0x0F ) != 0x0A ) )
            cStatus = MI_ERR;

    }
    return cStatus;
}


/*
 * 函数名：PcdRead
 * 描述  ：读取M1卡一块数据
 * 输入  ：u8 ucAddr，块地址
 *         pData，读出的数据，16字节
 * 返回  : 状态值
 *         = MI_OK，成功
 * 调用  ：外部调用
 */
char PcdRead ( u8 ucAddr, u8 * pData )
{
    char cStatus;
    u8 uc, ucComMF522Buf [ MAXRLEN ];
    u32 ulLen;

    ucComMF522Buf [ 0 ] = PICC_READ;
    ucComMF522Buf [ 1 ] = ucAddr;

    CalulateCRC ( ucComMF522Buf, 2, & ucComMF522Buf [ 2 ] );

    cStatus = PcdComMF522 ( PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, & ulLen );

    if ( ( cStatus == MI_OK ) && ( ulLen == 0x90 ) )
    {
        for ( uc = 0; uc < 16; uc ++ )
            * ( pData + uc ) = ucComMF522Buf [ uc ];
    }

    else
        cStatus = MI_ERR;

    return cStatus;

}


/*
 * 函数名：PcdHalt
 * 描述  ：命令卡片进入休眠状态
 * 输入  ：无
 * 返回  : 状态值
 *         = MI_OK，成功
 * 调用  ：外部调用
 */
char PcdHalt( void )
{
    u8 ucComMF522Buf [ MAXRLEN ];
    u32  ulLen;

    ucComMF522Buf [ 0 ] = PICC_HALT;
    ucComMF522Buf [ 1 ] = 0;

    CalulateCRC ( ucComMF522Buf, 2, & ucComMF522Buf [ 2 ] );
    PcdComMF522 ( PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, & ulLen );

    return MI_OK;

}


void IC_CMT ( u8 * UID, u8 * KEY, u8 RW, u8 * Dat )
{
    u8 ucArray_ID [ 4 ] = { 0 };//先后存放IC卡的类型和UID(IC卡序列号)

    PcdRequest ( 0x52, ucArray_ID );//寻卡

    PcdAnticoll ( ucArray_ID );//防冲撞

    PcdSelect ( UID );//选定卡

    PcdAuthState ( 0x60, 0x10, KEY, UID );//校验

    if ( RW )//读写选择，1是读，0是写
        PcdRead ( 0x10, Dat );

    else
        PcdWrite ( 0x10, Dat );

    PcdHalt ();

}

void ShowID(u8 *p)	 //显示卡的卡号，以十六进制显示
{
    u8 num[9];
    u8 i;

    for(i=0; i<4; i++)
    {
        num[i*2]=p[i]/16;
        num[i*2]>9?(num[i*2]+='7'):(num[i*2]+='0');
        num[i*2+1]=p[i]%16;
        num[i*2+1]>9?(num[i*2+1]+='7'):(num[i*2+1]+='0');
    }
    num[8]=0;
    printf("ID>>>%s\r\n", num);
}

void reflash_information()
{
	u8 i,j;
	u16 temp;
	STMFLASH_Write(FLASH_SAVE_ADDR,(u16*)&NUM,1);
	for(i=0;i<NUM;i++)
	{
		for(j=0;j<USER_ID_SIZE/2;j++)
		{
			temp=student[i].id[j*2];
			temp=temp<<8;
			temp+=student[i].id[j*2+1];
			printf(" temp = %d arr=%d\r\n",temp,FLASH_USER_DATA+i*SAVE_BLOG+j*2);
			STMFLASH_Write(FLASH_USER_DATA+i*SAVE_BLOG+j*2,(u16*)&temp,1);
		}
		ShowID(student[i].id);
		for(j=0;j<USER_INFO_SIZE/2;j++)
		{
			temp=student[i].info[j*2];
			temp=temp<<8;
			temp+=student[i].info[j*2+1];
			STMFLASH_Write(FLASH_USER_DATA+i*SAVE_BLOG+4+j*2,(u16*)&temp,1);
		}
	}
}

void information_init()
{
	u16 temp;
	u16 i,j;
	STMFLASH_Read(FLASH_SAVE_ADDR,(u16*)&NUM,1);
	printf("NUM=%d\r\n",NUM);
  LCD_write_english_string(0,0,"Loading..");
	for(i=0;i<NUM;i++)
	{
		for(j=0;j<USER_ID_SIZE/2;j++)
		{
			STMFLASH_Read(FLASH_USER_DATA+i*SAVE_BLOG+j*2,(u16*)&temp,1);
			student[i].id[j*2] = (temp>>8);
			student[i].id[j*2+1] = (temp<<8)>>8;
			printf(" %d\r\n",FLASH_USER_DATA+i*SAVE_BLOG+j*2);
		}
		ShowID(student[i].id);
		//LED0=1;
		for(j=0;j<USER_INFO_SIZE/2;j++)
		{
			
			STMFLASH_Read(FLASH_USER_DATA+i*SAVE_BLOG+4+j*2,(u16*)&temp,1);
			student[i].info[j*2] = (temp>>8);
			student[i].info[j*2+1] = (temp<<8)>>8;
		}
		//LED1=1;
	}
	//LED0=0;
	STMFLASH_Read(FlASH_ROOT_NUM_DATA,(u16*)&NUM_ROOT,1);
	printf("NUM_ROOT=%d\r\n",NUM_ROOT);
	//LED0=1;
	for(i=0;i<NUM_ROOT;i++)
	{
		STMFLASH_Read(FlASH_ROOT_DATA+i*4,(u16*)&temp,1);
		root[i][0] = (temp>>8);
		root[i][1] = (temp<<8)>>8;
		STMFLASH_Read(FlASH_ROOT_DATA+i*4+2,(u16*)&temp,1);
		root[i][2] = (temp>>8);
		root[i][3] = (temp<<8)>>8;
		ShowID(root[i]);	
	}
	
}

void write_root_num(u8* USERID)
{
	u16 k=0;
	
	if(1)
	{	
		//write data to FLASH_USER_DATA+NUM*2+0 ~ FLASH_USER_DATA+NUM*2+2
		k=SN[0];
		k=k<<8;
		k+=SN[1];
		printf("k=%d\r\n",k);
		STMFLASH_Write(FlASH_ROOT_DATA+NUM_ROOT*4,(u16*)&k,1);
		k=SN[2];
		k=k<<8;
		k+=SN[3];
		STMFLASH_Write(FlASH_ROOT_DATA+NUM_ROOT*4+2,(u16*)&k,1);
		//write = RAM
		
		root[NUM_ROOT][0]=SN[0];
		root[NUM_ROOT][1]=SN[1];
		root[NUM_ROOT][2]=SN[2];
		root[NUM_ROOT][3]=SN[3];
		//update NUN
		NUM_ROOT=NUM_ROOT+1;
		STMFLASH_Write(FlASH_ROOT_NUM_DATA,(u16*)&NUM_ROOT,1);
		
	
		
	}

}




void read_data(u8 num)
{
	u16 k=0x21FE;
	u8 USERID[4];
	
	if(1)
	{
		STMFLASH_Read(FLASH_SAVE_ADDR+0*4,(u16*)&k,SIZE);
		printf("k=%d\r\n",k);
		STMFLASH_Read(FLASH_USER_DATA+0*4,(u16*)&k,1);
		printf("k=%d\r\n",k);
		USERID[0] = k>>8;
		USERID[1] = (k<<8) >>8;
		STMFLASH_Read(FLASH_USER_DATA+0*4+2,(u16*)&k,1);
		printf("k=%d\r\n",k);
		USERID[2] = k>>8;
		USERID[3] = (k<<8) >>8;

		ShowID(USERID);
	}
	

}

u8 check_same(u8 num[])
{
	static u8 temp[4]={0};
	int i=0,j;
	//printf("%d %d %d %d \r\n",temp[0],temp[1],temp[2],temp[3]);
	
	for(i=0;i<4;i++)
	{
		if(temp[i]==num[i])
			continue;
		for(j=0;j<4;j++)
		{
			temp[j]=num[j];
		}
		return 0;
	}

	return 1;
	
}
u8 check_read_same(u8 num[])
{
	static u8 temp[4]={0};
	int i=0,j;
	for(i=0;i<4;i++)
	{
		if(temp[i]==num[i])
			continue;
		for(j=0;j<4;j++)
		{
			temp[j]=num[j];
		}
		return 0;
	}
	return 1;
	
}
u8 check_clear_same(u8 num[])
{
	static u8 temp[4]={0};
	int i=0,j;
	for(i=0;i<4;i++)
	{
		if(temp[i]==num[i])
			continue;
		for(j=0;j<4;j++)
		{
			temp[j]=num[j];
		}
		return 0;
	}
	return 1;
	
}
u8 check_root_same(u8 num[])
{
	static u8 temp[4]={0};
	int i=0,j;
	for(i=0;i<4;i++)
	{
		if(temp[i]==num[i])
			continue;
		for(j=0;j<4;j++)
		{
			temp[j]=num[j];
		}
		return 0;
	}
	return 1;
	
}
u8 check_root_same2(u8 num[])
{
	static u8 temp[4]={0};
	int i=0,j;
	for(i=0;i<4;i++)
	{
		if(temp[i]==num[i])
			continue;
		for(j=0;j<4;j++)
		{
			temp[j]=num[j];
		}
		return 0;
	}
	return 1;
	
}

u8 check_wrote(u8 data[])
{
	u8 i = 8;
	for(i=0;i<NUM;i++)
	{
		//ShowID(user[i]);
		if(student[i].id[0] == SN[0] && student[i].id[1]==SN[1] && student[i].id[2]==SN[2] && student[i].id[3]==SN[3])
		{
			printf("EXIST");
			LCD_clear();
			LCD_write_english_string(0,1,"EXIST!");
			return 1;
		}
	}
	return 0 ;
}
u8 check_wrote_root(u8 data[])
{
	u8 i = 8;
	for(i=0;i<NUM_ROOT;i++)
	{
		ShowID(root[i]);
		if(root[i][0] == SN[0] && root[i][1]==SN[1] && root[i][2]==SN[2] && root[i][3]==SN[3])
		{
			printf("Exit");
			LCD_clear();
			LCD_write_english_string(0,1,"EXIST!");
			delay_ms(1000);
			return 1;
		}
	}
	return 0 ;
}













void write_num(u8 ml,u8* USERID)
{
	u16 k=0,i=0;
#if DEBUG
	NUM++;
	if(ml==WR_ID)
		for(i=0;i<USER_ID_SIZE;i++)//读取ID
			student[NUM].id[i]=USERID[i];
	
	else if(ml == WR_INFO)
		for(i=0;i<USER_INFO_SIZE;i++)//读取ID
			student[NUM].info[i]=USERID[i];
	
	
#else 
 
#endif

}

u8 show_id(u8 ml,char num,u8 number[],char * cur,char idnum[])
{
	u8 i=0,j=0,flag=0;
	
	if(ml==1)
	{
		
		if((*cur)==USER_ID_LONG-1)
		{
			(*cur)=USER_ID_LONG-1;
			
		}
		else
		{
			idnum[*cur]=num;
			(*cur)++;
			LCD_write_english_string(0,3,idnum);
		}
	}
	else if(ml == 0)
	{
		idnum[(*cur)-1]=' ';

		if((*cur)==0)
			(*cur)=1;
		(*cur)--;
		LCD_write_english_string(0,3,idnum);
	}
	else if(ml==2)
	{
		for(i=0;i<NUM;i++)
		{
			flag=0;
			for(j=0;j<USER_INFO_SIZE;j++)
			{	if(student[i].info[j]==(idnum[j*2]-48)+(idnum[j*2+1]-48)*10)
				{
					flag++;
				}
			}
			if(flag==5)
			{
				printf("Same ID\r\n");
				break;
			}
		}
		if(flag!=5)
		{
			for(i=0;i<USER_INFO_SIZE;i++)
				student[NUM].info[i]=(idnum[i*2]-48)+(idnum[i*2+1]-48)*10;
			for(i=0;i<USER_ID_SIZE;i++)
				student[NUM].id[i]=SN[i];
			printf("\r\n%d %d %d %d\r\n",SN[0],SN[1],SN[2],SN[3]);
			printf("id=%d %d %d %d\r\n",student[NUM].id[0],student[NUM].id[1],student[NUM].id[2],student[NUM].id[3]);
			NUM++;
			for(i=0;i<11;i++)
				idnum[i]=' ';
		}
		else
		{
			LCD_write_english_string(0,5,"ID exit");
			delay_ms(1000);
			LCD_write_english_string(0,5,"         ");
			return 0;
		}

	}
#if DEBUG
	printf("cur=%d",(*cur));
#endif
	return 1;
	
}



void write_mode()
{
	u8 num_flag=1,num,i,j;
	char idnum[11]={"          "};
	char current=0;
	if(rootkey == 1)
	{
#if 0
		NUM=6;
		for(i=0;i<6;i++)
			for(j=0;j<6;j++)
				student[i].info[j]=i*10+j;
#else	
		RC522_Handel();//读卡
		if(!check_root_same2(SN))//查看是否为同一张卡
		{
			if(!check_wrote(SN)&&NUM<=NUM_MAX&&SN[0]!=0&&SN[1]!=0&&SN[2]!=0&&SN[3]!=0)
			{
				LCD_clear();
				LCD_write_english_string(0,0,"New Card!");//找到
				LCD_write_english_string(0,1,"Enter ID:");//找到
				while(num_flag)
				{
					num=Remote_Scan();
					switch(num)
					{
						case 194:
						{
							if(show_id(2,48,student[NUM].info,&current,idnum))//>||	 
							{
								num_flag=0;
								reflash_information();
							}
							break;
						}  
						case 98:num_flag=0;break;//ch	
						case 144:show_id(0,48,0,&current,idnum);break;//EQ		    
						case 104:show_id(1,48,0,&current,idnum);break;//0					
						case 48:show_id(1,49,0,&current,idnum);break;//1		    
						case 24:show_id(1,50,0,&current,idnum);break;//2		    
						case 122:show_id(1,51,0,&current,idnum);break;//3		  
						case 16:show_id(1,52,0,&current,idnum);break;//4			   					
						case 56:show_id(1,53,0,&current,idnum);break;//5	 
						case 90:show_id(1,54,0,&current,idnum);break;//6
						case 66:show_id(1,55,0,&current,idnum);break;//7
						case 74:show_id(1,56,0,&current,idnum);break;//8
						case 82:show_id(1,57,0,&current,idnum);break;//9		
				
					}
				}
				
				//write_num(WR_ID,SN);
				if(num==194)
				{
					LCD_clear();
					printf("Write a new card");
					LCD_write_english_string(0,1,"New User!");
				}
				else if(num==98)
				{
					LCD_write_english_string(0,1,"Quck!");
				}
				delay_ms(1000);
			}
		}
#endif
	}
}















void write_root_mode()
{
	if(rootkey == 1)
	{
		RC522_Handel();
		
		if(!check_same(SN))
		{
			if(!check_wrote_root(SN)&&NUM_ROOT<=NUM_ROOT_MAX&&SN[0]!=0&&SN[1]!=0&&SN[2]!=0&&SN[3]!=0)
			{
				write_root_num(SN);
				LCD_clear();
				printf("Write a new root card");
				LCD_write_english_string(0,1,"NEW ROOT!");
				delay_ms(1000);
			}
			else if(NUM_ROOT>NUM_ROOT_MAX)
			{
				LCD_clear();
				LCD_write_english_string(0,1,"ROOT MAX!");
				delay_ms(1000);
			}
		}
	}
}

void read_mode()
{
	u8 i=0;
	RC522_Handel();
	if(check_read_same(SN))
	{
		for(i=0;i<NUM;i++)
		{
			//printf("%d",i);
			if(student[i].id[0]==SN[0]&&student[i].id[1]==SN[1]&&student[i].id[2]==SN[2]&&student[i].id[3]==SN[3])
			{
				printf("Open The Door");
				LCD_clear();

				Door1=1;
				LED1=1;
				showp2();
				
				delay_ms(1000);
				showp1();
				delay_ms(100);
				Door1=0;
				LED1=0;
				//while(PcdRequest(PICC_REQALL,CT)==MI_OK);
					SN[0]=0;
					SN[1]=0;
					SN[2]=0;
					SN[3]=0;

			}
		}
	}
}





u8 show_all_id(u8 ml,char num,u8 number[],char * cur)
{
	u8 i=0,j=0;
	static u8 base=0;
	static char idnum[12]={"           "};
	if(ml==1)
	{
		
		if((*cur)==PAGE_MAX-1)
		{
			(*cur)=PAGE_MAX-1;
			if(base<NUM-PAGE_MAX){ base++;}
		}
		else if((*cur)==NUM-1)
		{
			(*cur)=NUM-1;
		}
		else
		{
			idnum[*cur]=num;
			(*cur)++;
		}
	}
	
	else if(ml == 0)
	{
		if((*cur)==0)
		{
			(*cur)=1;
			 if(base >0 )base--;

		}
		(*cur)--;
	}
	
	
	//显示
	for(i=0;i<NUM&&i<PAGE_MAX-1;i++)
	{
		for(j=0;j<USER_INFO_SIZE;j++)
		{
			idnum[j*2]=student[base+i].info[j]%10+48;
			idnum[j*2+1]=student[base+i].info[j]/10+48;
		}
		idnum[10]=' ';
		idnum[11]=' ';
		LCD_write_english_string(0,i,idnum);
	}
	//显示标志'<'
	for(j=0;j<USER_INFO_SIZE;j++)
	{
		idnum[j*2]=student[base+(*cur)].info[j]%10+48;
		idnum[j*2+1]=student[base+(*cur)].info[j]/10+48;
	}
	idnum[10]=' ';
	idnum[11]='<';
	if(NUM!=0) LCD_write_english_string(0,(*cur),idnum);
	
	
#if DEBUG
	printf("cur=%d, base=%d,num=%d\r\n,",(*cur),base,NUM);
#endif
	return base+(*cur);
}


void del_student(u8 num)
{
	u8 i,j;
	for(i=num;i<NUM;i++)
	{
		for(j=0;j<USER_ID_SIZE;j++)
			student[i-1].id[j]=student[i].id[j];
		for(j=0;j<USER_INFO_SIZE;j++)
			student[i-1].info[j]=student[i].info[j];
	}
	NUM--;
}


void clear_mode()
{
	u8 key,i,j;
	u8 num_flag=1;
	u8 temp[USER_INFO_SIZE]={0};
	u8 sum_cur=0;
	char current=0;
	
#if 0
	NUM=6;
	for(i=0;i<6;i++)
		for(j=0;j<6;j++)
			student[i].info[j]=i*10+j;
#endif	
	
	if(rootkey==1)
	{
		if(NUM!=0){
			LCD_clear();
		sum_cur=show_all_id(3,0,0,&current);
		while(num_flag)
		{
			
			key=Remote_Scan();
			switch(key)
			{      
				case 194:del_student(sum_cur+1);reflash_information();num_flag=0;break;//>||	   
				case 98:num_flag=0;break;//ch	
				case 168:sum_cur=show_all_id(1,0,0,&current);break;//+	
				case 224:sum_cur=show_all_id(0,0,0,&current);break;//-	
			}
		}
		LCD_clear();
		if(key==194)
		{
			LCD_write_english_string(0,2,"   Del!");
			delay_ms(1000);
		}
		else if(key==98)
		{
			LCD_write_english_string(0,2,"   Quck!");
			delay_ms(1000);
		}
	}
	else
	{
		LCD_write_english_string(0,2," NULL!");
		delay_ms(1000);
	}
		//
	}
	//

}

void initSN()
{
	SN[0]=0;
	SN[1]=0;
	SN[2]=0;
	SN[3]=0;
}
void root_mode(u8 *blash)
{
	u8 i=0,j=0;
	u8 num_flag=1,key=0;
	u8 super_root[4];
	super_root[0]=0x80;
	super_root[1]=0x01;
	super_root[2]=0xBF;
	super_root[3]=0x90;
	RC522_Handel();
	if(rootkey==0)
	{
		if(check_root_same(SN))
		{
			if(super_root[0]==SN[0]&&super_root[1]==SN[1]&&super_root[2]==SN[2]&&super_root[3]==SN[3])
			{
				rootkey=1;
				LCD_clear();
				printf("Successful login\r\n");
				LCD_write_english_string(0,1,"Get ROOT!");
				*blash=2;
			}
			for(i=0;i<NUM_ROOT;i++)
			{
				//printf("%d",i);
				if(root[i][0]==SN[0]&&root[i][1]==SN[1]&&root[i][2]==SN[2]&&root[i][3]==SN[3])
				{
					rootkey=1;
					LCD_clear();
					printf("Successful login\r\n");
					LCD_write_english_string(0,1,"Get ROOT!");
					initSN();
					*blash=2;
				}
			}
		}
	}	
	else
	{
		while(num_flag)
		{
			key=Remote_Scan();
			switch(key)
			{

				case 162:num_flag=0;break;//ch-	    
				case 98:num_flag=0;break;//ch	    
				case 2:num_flag=0;break;//>>|
				case 226:num_flag=0;break;//ch+		  
				case 194:;break;//>||	   
				case 34:num_flag=0;break;//|<<		  
				case 224:num_flag=0;break;//-		  
				case 168:num_flag=0;break;//+		   
				case 144:num_flag=0;break;//EQ		 	    

				case 104:num_flag=0;break;//0					
				case 48:num_flag=0;break;//1		    
				case 24:num_flag=0;break;//2		    
				case 122:num_flag=0;break;//3		  
				case 16:num_flag=0;break;//4			   					
				case 56:num_flag=0;break;//5	 
				case 90:num_flag=0;break;//6
				case 66:num_flag=0;break;//7
				case 74:num_flag=0;break;//8
				case 82:num_flag=0;break;//9		
				
			}
			LCD_clear();
			LCD_write_english_string(0,0,"ADD ROOT");
			LCD_write_english_string(0,2,"Any  ");
			LCD_write_english_string(0,3,"Key");
			LCD_write_english_string(0,4,"Exit");
			write_root_mode();
		}
		*blash=2;
	}
	
}
u8 check_rootkey()
{
	if(rootkey==0)
		return 0;
	else
		return 1;
}
u8 logout()
{
	rootkey=0;
}