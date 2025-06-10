/**
  ******************************************************************************
  * File Name          : keyscan_demo.c
  * Description        : demo code for key scan for STM32F407 of farsight 
  ******************************************************************************
  *
  * COPYRIGHT(c) 2020 jkuang@BUPT
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/

/* USER CODE BEGIN Includes */
#include	"uCOSII_Demo.h"
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
INT8U 	KeySwitch[KEY_NUMBER];	//Board key switch identification, value toggled while specified key pressed.

INT8U 	keyscan; //key scan state, bit0-3, '0': key down already, scan halt; '1': key scan enable
#define KEYSCANGET(x)		(keyscan & (0x1<<(x-1)))		//get key(x) scan state, x = 1-4 
#define KEYSCANEN(x) 		(keyscan |= 0x1<<(x-1))		//set key(x) scan enable, x = 1-4 
#define KEYSCANDISEN(x) (keyscan &= ~(0x1<<(x-1)))		//set key(x) scan disable, x = 1-4 

#define TASK_KETSCAN_PRIO    	20
static OS_STK task_keyscan_stk[STK_SIZE_DEF];
static void task_keyscan(void *p_arg);
//MsgQ of task Keyscan---------------------------------------------------------------------------------------------------------------------------------
#define TASK_keyscan_QSIZE		10
static void *Taskkeyscan_Qarray[TASK_keyscan_QSIZE];
/* Private variables ---------------------------------------------------------*/

//--------------Key Scan Task--------------------
#define PRINT_EN_KEYSCAN
// Key Value
#define KEY_UP				1
#define KEY_DOWN			0

//============================================================
//task KeyScan Create
INT8U Task_KeyScan_Creat(void)
{
	INT8U err, i;

	//Key Switch Record initial
	for(i=0; i<KEY_NUMBER; i++)
	{
		KEYSWITCH(i+1) = 1;
	}
	
	//Creat Key scan task
	err = OSTaskCreate(	task_keyscan, (void*)0,
											&task_keyscan_stk[STK_SIZE_DEF-1], TASK_KETSCAN_PRIO);
	return(err);
}
	

/*
//============================================================
static INT8U KEYx_Down_Process(INT8U key)
{
	KEYSCANDISEN(key);
	KEYSWITCH(key) ^= 1;
	//send msg to tmetst task Q Instance 3
	return(Msg_SendShort(MC_KEYx_SWITCH, APP_TID_keyscan, APP_TID_tmrtest, key, key));
}
*/


//============================================================
static void task_keyscan(void *p_arg)
{
	INT8U 				err ;

	keyscan = 0;
	
	APP_TPRIO(APP_TID_keyscan) = TASK_KETSCAN_PRIO;
	//Create Q of task keyscan--------------------------------------------------------------------------------------------------------------------------
	APP_TQID(APP_TID_keyscan) = OSQCreate(&Taskkeyscan_Qarray[0], TASK_keyscan_QSIZE);
	if(APP_TQID(APP_TID_keyscan) == NULL)
	{
		USER_USART1_print("Fail to create Keyscan Msg Queue\n");
		APP_TPRIO(APP_TID_keyscan) = 0xFF;
		OSTaskDel(OS_PRIO_SELF);
		return;
	}
	
	//suspend on the extern var sem after init-------------------------------------------------------------------------------------------------------------
	USER_USART1_print("\n====Task KeyScan Initilized====\n");
	Msg_SendShort(MC_TASK_keyscan, APP_TID_keyscan, APP_TID_SysManage, 0, 0);//send msg to SysManage task Q
	USER_USART1_print("before keyscan sem->OSEventGrp:");
				USER_Print_Decimal(sem->OSEventGrp);
	OSSemPend (sem, 0, &err);
	USER_USART1_print("after keyscan sem->OSEventGrp:");
				USER_Print_Decimal(sem->OSEventGrp);
	OSSemPost(sem);
	
	while(1)
	{	       
		USER_USART1_print("\n====Task KeyScan body====\n");
		OSTimeDly(OS_TICKS_PER_SEC/10);	// task delay
		USER_USART1_print("\n====Task KeyScan after delay====\n");
		// enable scan only when key up
		if(KEY1_READ == KEY_UP)
			KEYSCANEN(1);
		if(KEY2_READ == KEY_UP)
			KEYSCANEN(2);
		if(KEY3_READ == KEY_UP)
			KEYSCANEN(3);
		if(KEY4_READ == KEY_UP)
			KEYSCANEN(4);

		// any key down process while scan enable
		if(	(KEYSCANGET(1) && (KEY1_READ == KEY_DOWN)) ||
			 	(KEYSCANGET(2) && (KEY2_READ == KEY_DOWN)) ||
				(KEYSCANGET(3) && (KEY3_READ == KEY_DOWN)) ||
				(KEYSCANGET(4) && (KEY4_READ == KEY_DOWN)))
		{
			USER_USART1_print("\n====Task KeyScan body before quiver====\n");
			OSTimeDly(OS_TICKS_PER_SEC/10); // È¥¶¶¶¯
			USER_USART1_print("\n====Task KeyScan body after quiver====\n");
			// key 1 down process
			if(KEYSCANGET(1) && (KEY1_READ == KEY_DOWN)) 
			{
				//KEYx_Down_Process(1);
				//when key down send msg to task LEDx-----------------------------------------------------------------------------------------------------
				Msg_SendShort(MC_KEY1, APP_TID_keyscan, APP_TID_LEDx, 0, 0);
				
#ifdef PRINT_EN_KEYSCAN
				USER_USART1_print("Key 1\n");
#endif
			}

			// key 2 down process
			if(KEYSCANGET(2) && (KEY2_READ == KEY_DOWN)) 
			{
				//KEYx_Down_Process(2);
				//send a invalid mCode msg for test
				//Msg_SendShort(65535, APP_TID_keyscan, APP_TID_tmrtest, 0, 0);
				//when key down send msg to task LEDx-----------------------------------------------------------------------------------------------------
				Msg_SendShort(MC_KEY2, APP_TID_keyscan, APP_TID_LEDx, 0, 0);
#ifdef PRINT_EN_KEYSCAN
				USER_USART1_print("Key 2\n");
#endif
			}

			// key 3 down process
			if(KEYSCANGET(3) && (KEY3_READ == KEY_DOWN))
			{
				//KEYx_Down_Process(3);
				//when key down send msg to task LEDx-----------------------------------------------------------------------------------------------------
				Msg_SendShort(MC_KEY3, APP_TID_keyscan, APP_TID_LEDx, 0, 0);
#ifdef PRINT_EN_KEYSCAN
				USER_USART1_print("Key 3\n");
#endif
			}

			// key 4 down process
			if(KEYSCANGET(4) && (KEY4_READ == KEY_DOWN))
			{
				//KEYx_Down_Process(4);
				//when key down send msg to task LEDx-----------------------------------------------------------------------------------------------------
				Msg_SendShort(MC_KEY4, APP_TID_keyscan, APP_TID_LEDx, 0, 0);
#ifdef PRINT_EN_KEYSCAN
				USER_USART1_print("Key 4\n");
#endif
			}
		}
		else
		{
			USER_USART1_print("\n====Keyscan else====\n");
			OSTimeDly(OS_TICKS_PER_SEC/10);	// task delay
		}
  }
}


/************************ (C) COPYRIGHT jkuang@BUPT *****END OF FILE****/
