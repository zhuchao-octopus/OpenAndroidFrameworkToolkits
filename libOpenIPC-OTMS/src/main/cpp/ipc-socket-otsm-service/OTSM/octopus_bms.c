/********************************** (C) COPYRIGHT *******************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************
 */

/******************************************************************************/
/* Header file contains */

#include "octopus_platform.h"  			// Include platform-specific header for hardware platform details
#include "octopus_bms.h"
#include "octopus_system.h"

/*******************************************************************************
 * DEBUG SWITCH MACROS
*/
#ifdef TASK_MANAGER_STATE_MACHINE_BMS

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTANTS
// 
//1ms interrupter
//sync ______________________________________________________________|--| 62ms+2ms
//0 ____|-- 4ms+2ms
//1 --|____ 2ms+4ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////
#define BMS_LOW                    			0      	//低电平
#define BMS_HIGH                    		1      	//高电平

#define BMS_REV_SYNC_L_TIME_NUM         60     //同步信号低电平时间：62ms = 30000us / 1ms = 62

#define BMS_REV_SYNC_H_TIME_NUM_MIN     1      //同步信号高电平最小时间
#define BMS_REV_SYNC_H_TIME_NUM_MAX     2      //同步信号高电平最大时间

//#define BMS_REV_SHORT_TIME_NUM_MIN      18      //一个逻辑周期中短的时间最小值：1000-100us = 900us / 50us = 18
//#define BMS_REV_SHORT_TIME_NUM_MAX      22      //一个逻辑周期中短的时间最大值：1000+100us = 1000us / 50us = 22

//#define BMS_REV_LONG_TIME_NUM_MIN       36      //一个逻辑周期中长的时间最小值：2ms-200us = 1800us / 50us = 36
//#define BMS_REV_LONG_TIME_NUM_MAX       44      //一个逻辑周期中长的时间最大值：2ms+200us = 2200us / 50us = 44

//#define BMS_REV_LOGIC_CYCLE_NUM_MIN     54      //一个逻辑周期最小时间：3ms-300us = 2700us / 50us = 54
#define BMS_REV_LOGIC_CYCLE_NUM_MAX     12      //一个逻辑周期最大时间：3ms+300us = 3300us / 50us = 66

#define BMS_REV_HALF_LOGIC_CYCLE_MIN    5      //一个逻辑周期的1/2最小时间：1.5ms-150us = 1350us / 50us = 27
//#define BMS_REV_HALF_LOGIC_CYCLE_MAX    33      //一个逻辑周期的1/2最大时间：1.5ms+150us = 1650us / 50us = 33

#define BMS_REV_END_SIGNAL_TIME_NUM     200      //结束信号电平时间：5ms低电平 + Nms高电平，实际检测5ms低电平就行，一帧数据发送完成后检测5ms低电平就代表完成了，不发数据的时候上拉电阻拉高了

#define BMS_REV_BIT_NUM                 8       //接收的bit位个数，1字节=8bit

#define BMS_REV_DATA_NUM                6      //接收的数据个数

#define BMS_SEND_SYNC_L_TIME_NUM        600     //同步信号低电平时间：30ms = 30000us / 50us = 600
#define BMS_SEND_SYNC_H_TIME_NUM        10      //同步信号高电平时间：500us = 500us / 50us = 10
#define BMS_SEND_SHORT_TIME_NUM         10      //一个逻辑周期中短的时间：500us = 500us / 50us = 10
#define BMS_SEND_LONG_TIME_NUM          20      //一个逻辑周期中长的时间：1ms = 1000us / 50us = 20
#define BMS_SEND_BIT_NUM                8       //发送的bit位个数，1字节=8bit
#define BMS_SEND_DATA_MAX_NUM           12      //发送的数据最大个数

/***************************************************************************************************
 *                                           GLOBAL VARIABLES
 ***************************************************************************************************/

/* 类型定义 -------------------------------------------------------------------*/
typedef enum
{
	BMS_REV_STATE_INITIAL = 0,    //初始状态，等待接收同步信号
	BMS_REV_STATE_SYNC_L = 1,    //接收同步低电平信号状态
	BMS_REV_STATE_SYNC_H = 2,    //接收同步高电平信号状态
	BMS_REV_STATE_REV_BIT = 3,    //读取数据码电平状态
	BMS_REV_STATE_BUILD_DATA = 4,    //构建字节数据
	BMS_REV_STATE_END_SIGNAL = 5,    //接收结束电平信号状态
	BMS_REV_STATE_IDLE_SIGNAL = 6,
	BMS_REV_STATE_RESTART = 7,     //接收过程出错重新接收状态
	BMS_REV_STATE_STOP = 8
} BMS_REV_STATE_T;   //接收数据状态枚举

typedef enum
{
	BMS_SEND_STATE_INITIAL = 0,    //初始状态，等待发送数据标志
	BMS_SEND_STATE_SYNC_L = 1,    //发送同步低电平信号状态
	BMS_SEND_STATE_SYNC_H = 2,    //发送同步高电平信号状态
	BMS_SEND_STATE_GET_BIT = 3,    //获取数据位
	BMS_SEND_STATE_BIT_L = 4,    //发送数据码低电平信号状态
	BMS_SEND_STATE_BIT_H = 5,    //发送数据码高电平信号状态
	BMS_SEND_STATE_CHECK_BIT = 6,    //检查数据位状态
	BMS_SEND_STATE_END_SIGNAL = 7,    //发送结束电平信号状态

} BMS_SEND_STATE_T;  //发送数据状态枚举

/* 接收变量定义 -------------------------------------------------------------------*/
uint32_t BMS_receive_H_L_Level_time_cnt = 0; //高低电平时间计数
uint8_t BMS_receive_start_H_L_Level_timming_flag = 0; //开始高低电平计时标记

uint8_t BMS_receive_state = BMS_REV_STATE_IDLE_SIGNAL;      //接收数据状态
uint8_t BMS_receive_bit_num = 0;    //接收的bit位个数
uint8_t BMS_receive_data_num = 0;   //接收的数据个数

//接收数据缓存数组-用一个数组来缓存数据，51个数据字节
uint8_t BMS_receive_data_buf[BMS_REV_DATA_NUM] = { 0 };
uint8_t BMS_receive_buf[BMS_REV_DATA_NUM] = { 0 };

uint8_t BMS_receive_has_read_bit = 0;               //1-已经读取一个bit位
uint8_t BMS_receive_check_OK = 0;                   //1-校验和正确，0-校验和失败
uint8_t BMS_receive_read_success = 0;                 //一帧数据是否读取成功，0-不成功，1-成功

#ifdef BMS_SEND_HANDLER
/* 发送变量定义 -------------------------------------------------------------------*/
uint32_t BMS_send_H_L_Level_time_cnt = 0; //高低电平时间计数
uint8_t BMS_send_start_H_L_Level_timming_flag = 0; //开始高低电平计时标记

uint8_t BMS_send_state = 0;      //接收数据状态
uint8_t BMS_send_bit_num = 0;    //接收的bit位个数
uint8_t BMS_send_data_num = 0;   //接收的数据个数

uint8_t BMS_send_bit = 0;               //1-已经读取一个bit位

uint8_t BMS_send_data_buf[BMS_SEND_DATA_MAX_NUM] = { 0 };
uint8_t BMS_send_data_num_target = 0;
uint8_t BMS_send_req_flag = 0;               //存在发送请求
#endif

static bool BMS_has_inited = false;
/*************************************************************************************************
 * GLOBAL FUNCTIONS DECLEAR
 */
/*************************************************************************************************/

static void BMS_GPIO_Init(void);                //GPIO初始化函数
static uint8_t BMS_Get_Input_Pin_Data(void);    //读取BMS接收端口电平，带消抖处理
static void BMS_Receive_Data_Handle(void);      //接收数据处理—带校准位，即波特率自适应
#ifdef BMS_SEND_HANDLER
static void BMS_Send_Data_Handle(void);         //接收数据处理—带校准位，即波特率自适应
#endif
static void BMS_Checksum_Handle(void);         //校验和处理

bool BMS_Is_Idle(void);
/***************************************************************************************************
 *                                            LOCAL FUNCTION
 ***************************************************************************************************/
void otsm_bms_init(void)
{
  if (!BMS_has_inited)
	{
		BMS_GPIO_Init();
		///BMS_TIM2_Int_Init(50 - 1, 96 - 1);
		BMS_has_inited = true;
	}
}
 
bool BMS_IsInit(void)
{
  return BMS_has_inited;
}

void BMS_GPIO_Init(void)
{
	BMS_SEND_DATA_BIT_LOW();
}

void BMS_DeInit(void)
{
if (BMS_has_inited)
	{
		///TIM_ITConfig(TIM2, TIM_IT_Update, DISABLE);
		///TIM_Cmd(TIM2, DISABLE);
		BMS_has_inited = false;
	}
}

void BMS_ReStart()
{
	BMS_receive_state = BMS_REV_STATE_INITIAL;
}

bool BMS_Is_Idle(void)
{
	if (BMS_receive_state == BMS_REV_STATE_STOP)
		return 1;
	else
		return 0;
}

void BMS_Delay_us(uint32_t us) {
  DELAY_US(us);  
}

uint8_t BMS_ReadData(uint8_t* data, uint8_t maxlen)
{
	MY_ASSERT(data);
	if (!BMS_has_inited)
	{
		return 0;
	}

	if (BMS_receive_read_success && BMS_receive_check_OK)
	{
		uint8_t len = maxlen < BMS_REV_DATA_NUM ? maxlen : BMS_REV_DATA_NUM;
		for (int i = 0; i < len; i++)
		{
			data[i] = BMS_receive_buf[i];
		}
		BMS_receive_read_success = false;
		return len;
	}
	return 0;
}
/*********************************************************************
 * @fn      TIM2_IRQHandler
 *
 * @brief   This function handles TIM2 global interrupt request.
 *
 * @return  none
 */

void BMS_IO_IRQHandler(void)
{
		//BMS 接收处理
		if (BMS_receive_start_H_L_Level_timming_flag == 1) //开始高低电平计时标记
		{
			BMS_receive_H_L_Level_time_cnt++;     //高低电平维持时间计数变量
		}
		/////////////////////////////////////////////////////////////////////////////////////

		BMS_Receive_Data_Handle();      //接收数据处理

		/////////////////////////////////////////////////////////////////////////////////////
		//BMS 发送处理
		#ifdef BMS_SEND_HANDLER
		if (BMS_send_start_H_L_Level_timming_flag == 1)
		{
			BMS_send_H_L_Level_time_cnt++;     //高低电平维持时间计数变量
		}
		BMS_Send_Data_Handle();     //发送数据处理
		#endif
    /////////////////////////////////////////////////////////////////////////////////////
}

///__attribute__((section(".highcode")))
void BMS_Receive_Data_Handle(void)
{

	switch (BMS_receive_state)													//检测当前接收数据状态
	{
	case BMS_REV_STATE_INITIAL:                         //初始状态，未接收到同步信息，进行同步判断
		if (BMS_Get_Input_Pin_Data() == BMS_LOW)          //判断接收引脚的电平状态，当读到低电平时，开始计时
		{
			BMS_receive_bit_num = 0;                				//重置bit位计数器
			BMS_receive_data_num = 0;               				//重置接收数据个数
			BMS_receive_H_L_Level_time_cnt = 0;             //高低电平计时变量清0
			BMS_receive_start_H_L_Level_timming_flag = 1;   //开始高低电平计时
			BMS_receive_state = BMS_REV_STATE_SYNC_L;       //进入读取同步低电平信号状态

			memset(BMS_receive_data_buf, 0, BMS_REV_DATA_NUM);
		}
		break;

	case BMS_REV_STATE_SYNC_L:                          //在读取同步低电平信号期间
		if (BMS_Get_Input_Pin_Data() == BMS_HIGH)         //同步信号低电平检测期间读到高电平
		{
			if (BMS_receive_H_L_Level_time_cnt >= BMS_REV_SYNC_L_TIME_NUM) //如果同步信号低电平时间>=SYNC_L_TIME_NUM
			{                                       				//同步信号低电平时间要>=10ms
				BMS_receive_H_L_Level_time_cnt = 0;         	//高低电平计时变量清0
				BMS_receive_state = BMS_REV_STATE_SYNC_H;   	//进入读取同步信号高电平状态
			}
			else
			{
				BMS_receive_state = BMS_REV_STATE_IDLE_SIGNAL; //进入重新接收状态
			}
		}
		break;

	case BMS_REV_STATE_SYNC_H:                          //在读取同步信号高电平期间
		if (BMS_Get_Input_Pin_Data() == BMS_LOW)          //同步信号高电平检测期间读到低电平
		{
			//判断同步信号高电平时间是否在1ms±100us之间
			if (BMS_receive_H_L_Level_time_cnt >= BMS_REV_SYNC_H_TIME_NUM_MIN
					&& BMS_receive_H_L_Level_time_cnt <= BMS_REV_SYNC_H_TIME_NUM_MAX)
			{
				BMS_receive_H_L_Level_time_cnt = 0;          //高低电平计时变量清0
				BMS_receive_has_read_bit = 0;
				BMS_receive_state = BMS_REV_STATE_REV_BIT;   //进入读取数据状态
			}
			else
			{
				BMS_receive_state = BMS_REV_STATE_IDLE_SIGNAL;//进入重新接收状态
			}
		}
		else            																	//如果在同步信号高电平检测期间，时间超过2ms±200us，认为超时
		{
			//判断时间是否超时 2ms±200us
			if (BMS_receive_H_L_Level_time_cnt >= BMS_REV_LOGIC_CYCLE_NUM_MAX)
			{
				BMS_receive_state = BMS_REV_STATE_IDLE_SIGNAL;      //进入重新接收状态
			}
		}
		break;

	case BMS_REV_STATE_REV_BIT:          //在读取数据码电平期间
		if (BMS_Get_Input_Pin_Data() == BMS_HIGH)               //同步信号低电平检测期间读到高电平
		{
			//判断时间是否超时 2ms±200us
			if (BMS_receive_H_L_Level_time_cnt >= BMS_REV_LOGIC_CYCLE_NUM_MAX)
			{
				BMS_receive_state = BMS_REV_STATE_IDLE_SIGNAL;      //进入重新接收状态
			}
			else
			{
				if (BMS_receive_has_read_bit == 0)
				{
					if ((BMS_receive_H_L_Level_time_cnt < (BMS_REV_HALF_LOGIC_CYCLE_MIN)))
					{
						BMS_receive_data_buf[BMS_receive_data_num] |= 0x01;
					}
					BMS_receive_has_read_bit = 1;
					BMS_receive_state = BMS_REV_STATE_BUILD_DATA;
				}
			}
		}
		else
		{
			if (BMS_receive_H_L_Level_time_cnt >= BMS_REV_LOGIC_CYCLE_NUM_MAX)
			{
				BMS_receive_state = BMS_REV_STATE_IDLE_SIGNAL;      //进入重新接收状态
			}
		}
		break;

	case BMS_REV_STATE_BUILD_DATA:
		if (BMS_Get_Input_Pin_Data() == BMS_LOW)                //同步信号高电平检测期间读到低电平
		{
			//判断同步信号高电平时间是否在1ms±100us之间
			if (BMS_receive_has_read_bit == 1)
			{
				BMS_receive_H_L_Level_time_cnt = 0;         //高低电平计时变量清0

				BMS_receive_has_read_bit = 0;                   //清0，读取下一个bit位
				BMS_receive_bit_num++;                  //接收的bit数++

				if (BMS_receive_bit_num == BMS_REV_BIT_NUM)   //如果一个字节8个bit位接收完成
				{
					BMS_receive_data_num++;             //接收的数据个数++
					BMS_receive_bit_num = 0;            //接收bit位个数清0重新接收
					if (BMS_receive_data_num == BMS_REV_DATA_NUM)   //如果数据采集完毕
					{
						BMS_receive_state = BMS_REV_STATE_END_SIGNAL; //进入接收结束低电平信号状态
					}
					else
					{
						BMS_receive_state = BMS_REV_STATE_REV_BIT; //进入读取数据状态
					}
				}
				else                                //如果一个字节8个bit位还没有接收完成
				{
					//将接收数据缓存左移一位，数据从低bit位开始接收
					BMS_receive_data_buf[BMS_receive_data_num] =
							BMS_receive_data_buf[BMS_receive_data_num] << 1;
					BMS_receive_state = BMS_REV_STATE_REV_BIT; //进入读取数据状态
				}
			}
		}
		else            //如果在同步信号高电平检测期间，时间超过2ms±200us，认为超时
		{
			//判断时间是否超时 2ms±200us
			if (BMS_receive_H_L_Level_time_cnt >= BMS_REV_LOGIC_CYCLE_NUM_MAX)
			{
				BMS_receive_state = BMS_REV_STATE_IDLE_SIGNAL;      //进入重新接收状态
			}
		}
		break;

	case BMS_REV_STATE_END_SIGNAL:                              //在接收结束信号低电平期间
		if (BMS_Get_Input_Pin_Data() == BMS_LOW)
		{
			if (BMS_receive_H_L_Level_time_cnt >= BMS_REV_END_SIGNAL_TIME_NUM) //如果读到低电平时间>=5ms
			{

				memcpy(BMS_receive_buf, BMS_receive_data_buf, BMS_REV_DATA_NUM);
				BMS_Checksum_Handle();

				BMS_receive_read_success = 1;                   //一帧数据读取成功
				BMS_receive_start_H_L_Level_timming_flag = 0;   //停止高低电平计时
				BMS_receive_H_L_Level_time_cnt = 0;             //定时器计数值清0
				BMS_receive_state = BMS_REV_STATE_INITIAL;      //接收状态清0
			}
		}
		else    //结束信号低电平检测期间一直为低
		{
			if (BMS_receive_H_L_Level_time_cnt >= BMS_REV_LOGIC_CYCLE_NUM_MAX) //如果读到低电平时间>=10ms，认为超时
			{                               //一帧数据发送完成后需要间隔50ms才发送第二帧数据，期间肯定会被拉高
				BMS_receive_state = BMS_REV_STATE_IDLE_SIGNAL;      //进入重新接收状态
			}
		}
		break;
	#if 1
	case BMS_REV_STATE_IDLE_SIGNAL:
		//BMS_receive_state = BMS_REV_STATE_RESTART;
		//break;
	case BMS_REV_STATE_RESTART:                      		//重新接收数据状态
		BMS_receive_start_H_L_Level_timming_flag = 0;     //停止高低电平计时
		BMS_receive_H_L_Level_time_cnt = 0;          			//定时器计数值清0
		BMS_receive_has_read_bit = 0;                			//清0，读取下一个bit位
		BMS_receive_bit_num = 0;
		BMS_receive_data_num = 0;
		BMS_receive_state = BMS_REV_STATE_INITIAL;        //接收状态清0
		break;
	#endif
	}
}

#ifdef BMS_SEND_HANDLER
uint8_t BMS_SendData_Sync(uint8_t* data, uint8_t len)
{
	MY_ASSERT(data);
	MY_ASSERT(len);

	if (!BMS_has_inited)
	{
		return 0;
	}

	uint8_t byte = 0;
	uint8_t flag = 0;
	//同步信号
	BMS_SEND_DATA_BIT_LOW();
	BMS_Delay_us(20000);
	BMS_SEND_DATA_BIT_HIGH();
	BMS_Delay_us(500);
	for (int i = 0; i < len; i++)
	{
		byte = data[i];
		for (int j = 0; j < 8; j++)
		{
			flag = byte & (0x80 >> j);
			if (flag)
			{
				BMS_SEND_DATA_BIT_LOW();
				BMS_Delay_us(500);
				BMS_SEND_DATA_BIT_HIGH();
				BMS_Delay_us(1000);
			}
			else
			{
				BMS_SEND_DATA_BIT_LOW();
				BMS_Delay_us(1000);
				BMS_SEND_DATA_BIT_HIGH();
				BMS_Delay_us(500);
			}
		}
	}
	BMS_SEND_DATA_BIT_LOW();
	BMS_Delay_us(5000);
	return len;
}

uint8_t BMS_SendData(uint8_t* data, uint8_t len)
{
	#if 1
	LOG_LEVEL("BMS_SendData:");
	for (int i = 0; i < len; i++)
	{
		LOG_LEVEL("%02x ", data[i]);
	}
	LOG_LEVEL("\r\n");
	#endif
	MY_ASSERT(data);
	MY_ASSERT(len);
	MY_ASSERT(len <= BMS_SEND_DATA_MAX_NUM);
	if (!BMS_has_inited)
	{
		return 0;
	}

	if (BMS_send_req_flag == false)
	{
		for (int i = 0; i < len; i++)
		{
			BMS_send_data_buf[i] = data[i];
		}
		BMS_send_data_num_target = len;
		BMS_send_req_flag = true;
		return len;
	}
	return 0;
}
///__attribute__((section(".highcode")))
void BMS_Send_Data_Handle(void)
{
	switch (BMS_send_state)															 //检测当前接收数据状态
	{
	case BMS_SEND_STATE_INITIAL:                         //初始状态，未接收到同步信息，进行同步判断
		if (BMS_send_req_flag == true)             			   //判断接收引脚的电平状态，当读到低电平时，开始计时
		{
			BMS_send_bit_num = 0;                						 //重置bit位计数器
			BMS_send_data_num = 0;               						 //重置接收数据个数
			BMS_send_H_L_Level_time_cnt = 0;                 //高低电平计时变量清0
			BMS_send_start_H_L_Level_timming_flag = 1;       //开始高低电平计时
			BMS_send_state = BMS_SEND_STATE_SYNC_L;          //进入读取同步低电平信号状态
			BMS_SEND_DATA_BIT_LOW();
		}
		break;

	case BMS_SEND_STATE_SYNC_L:                          //在读取同步低电平信号期间
		if (BMS_send_H_L_Level_time_cnt >= BMS_SEND_SYNC_L_TIME_NUM) //如果同步信号低电平时间>=SYNC_L_TIME_NUM
		{                                                  //同步信号低电平时间要>=10ms
			BMS_send_H_L_Level_time_cnt = 0;                 //高低电平计时变量清0
			BMS_SEND_DATA_BIT_HIGH();
			BMS_send_state = BMS_SEND_STATE_SYNC_H;          //进入读取同步信号高电平状态
		}
		break;

	case BMS_SEND_STATE_SYNC_H:                          //在读取同步信号高电平期间
		//判断同步信号高电平时间是否在1ms±100us之间
		if (BMS_send_H_L_Level_time_cnt >= BMS_SEND_SYNC_H_TIME_NUM)
		{
			BMS_send_H_L_Level_time_cnt = 0;                 //高低电平计时变量清0
			BMS_SEND_DATA_BIT_LOW();
			BMS_send_state = BMS_SEND_STATE_GET_BIT;         //进入读取数据状态
		}
		break;

	case BMS_SEND_STATE_GET_BIT:                         //在读取数据码电平期间
		BMS_send_H_L_Level_time_cnt = 0;
		BMS_send_bit = BMS_send_data_buf[BMS_send_data_num] & (0x80 >> BMS_send_bit_num);
		BMS_send_state = BMS_SEND_STATE_BIT_L; 						 //进入读取数据状态
		break;

	case BMS_SEND_STATE_BIT_L:
		if (BMS_send_bit)
		{
			if (BMS_send_H_L_Level_time_cnt >= BMS_SEND_SHORT_TIME_NUM)
			{
				BMS_send_H_L_Level_time_cnt = 0;
				BMS_SEND_DATA_BIT_HIGH();
				BMS_send_state = BMS_SEND_STATE_BIT_H;      	 //进入重新接收状态
			}
		}
		else
		{
			if (BMS_send_H_L_Level_time_cnt >= BMS_SEND_LONG_TIME_NUM)
			{
				BMS_send_H_L_Level_time_cnt = 0;
				BMS_SEND_DATA_BIT_HIGH();
				BMS_send_state = BMS_SEND_STATE_BIT_H;      		//进入重新接收状态
			}
		}
		break;

	case BMS_SEND_STATE_BIT_H:
		if (BMS_send_bit)
		{
			if (BMS_send_H_L_Level_time_cnt >= BMS_SEND_LONG_TIME_NUM)
			{
				BMS_send_H_L_Level_time_cnt = 0;
				BMS_SEND_DATA_BIT_LOW();
				BMS_send_state = BMS_SEND_STATE_CHECK_BIT;      //进入重新接收状态
			}
		}
		else
		{
			if (BMS_send_H_L_Level_time_cnt >= BMS_SEND_SHORT_TIME_NUM)
			{
				BMS_send_H_L_Level_time_cnt = 0;
				BMS_SEND_DATA_BIT_LOW();
				BMS_send_state = BMS_SEND_STATE_CHECK_BIT;      //进入重新接收状态
			}
		}
		break;

	case BMS_SEND_STATE_CHECK_BIT:
		BMS_send_bit_num++;
		if (BMS_send_bit_num == BMS_SEND_BIT_NUM)   				//如果一个字节8个bit位接收完成
		{
			BMS_send_data_num++;             									//接收的数据个数++
			BMS_send_bit_num = 0;            									//接收bit位个数清0重新接收
			if (BMS_send_data_num == BMS_send_data_num_target)//如果数据采集完毕
			{
				BMS_send_state = BMS_SEND_STATE_END_SIGNAL;   	//进入接收结束低电平信号状态
			}
			else
			{
				BMS_SEND_DATA_BIT_LOW();
				BMS_send_state = BMS_SEND_STATE_GET_BIT; 				//进入读取数据状态
			}
		}
		else                                								//如果一个字节8个bit位还没有接收完成
		{
			BMS_SEND_DATA_BIT_LOW();
			BMS_send_state = BMS_SEND_STATE_GET_BIT; 					//进入读取数据状态
		}
		break;

	case BMS_SEND_STATE_END_SIGNAL:                       //在接收结束信号低电平期间
		BMS_send_req_flag = false;
		BMS_send_start_H_L_Level_timming_flag = 0;       		//停止高低电平计时
		BMS_send_H_L_Level_time_cnt = 0;                 		//定时器计数值清0
		BMS_send_bit_num = 0;
		BMS_send_data_num = 0;
		BMS_send_state = BMS_SEND_STATE_INITIAL;          	//接收状态清0
		break;
	}
}
#endif
/*******************************************************************************
 *函数名称 : Check_Sum_Handle
 *函数功能 : 校验和处理
 *输入参数 : void
 *输出返回 : void
 *******************************************************************************/
///__attribute__((section(".highcode")))
void BMS_Checksum_Handle(void)
{
	uint8_t checkByte = 0;
	uint64_t checkXor = 0;
	for (int i = 0; i < (BMS_REV_DATA_NUM - 1); i++)
	{
		checkXor = checkXor ^ BMS_receive_buf[i];
	}

	checkByte = (unsigned char) checkXor;
	if (checkByte == BMS_receive_buf[BMS_REV_DATA_NUM - 1])//校验和正确
	{
		BMS_receive_check_OK = 1;           								 //标记校验成功
	}
	else
	{
		BMS_receive_check_OK = 0;           								 //标记校验失败
	}
}

///__attribute__((section(".highcode")))
uint8_t BMS_Get_Input_Pin_Data(void)
{
	static uint8_t flag = 0;
	static uint8_t value = 0;
	static uint8_t ret = 0;
	uint8_t cur = BMS_RECEIVE_DATA_BIT();
	if (value != cur)
	{
		flag = 3;
		value = cur;
	}
	if (flag)
	{
		flag--;
		if (flag == 0)
		{
			ret = value;
		}
	}
	return ret;
}

#endif
