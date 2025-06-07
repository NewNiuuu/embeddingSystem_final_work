/**
  ******************************************************************************
  * File Name          : uCOSII_Demo.h
  * Description        : demo code for STM32F407 
  ******************************************************************************
  *
  * COPYRIGHT(c) 2019 jkuang@BUPT
  *
  ******************************************************************************
  */
#ifndef UCOSII_DEMO_H
#define UCOSII_DEMO_H
#ifdef __cplusplus
		 extern "C" {
#endif

#include  "ucos_ii.h"
#include	"farsight_f407.h"
#include 	"usart_it.h"
#include  "message.h"

//Common Stick Size of Task
#define STK_SIZE_DEF	128

//===================================================================
//app task record 
typedef struct _APP_RECORD
{
	OS_EVENT*	tQid;			// task Q ID
	INT16U    tPrio;		// task priority
	INT16U		tRes;			// reserved
} APP_RECORD;

#define APP_RECORD_NUM	OS_MAX_TASKS	//Max. number of tasks supported

extern APP_RECORD App_Record[APP_RECORD_NUM];

#define APP_TQID(x)		App_Record[x].tQid	
#define APP_TPRIO(x)	App_Record[x].tPrio	

//Task or Driver ID
//Driver ID
#define DRV_ID_usart1			-1
//System Task ID
#define OS_TID_Timer			0		// OS Timer task id use '0'

//User Task ID
//define task id of Task LEDx and SysManage----------------------------------------------------------------------------------------------------------
#define APP_TID_usart1		1
#define APP_TID_keyscan		2
#define APP_TID_LEDx			3
#define APP_TID_SysManage	4
#define APP_TID_tmrtest		5

#define KEYSTATE_TYPE INT8U

//Keycode definition(the mcode when you press KEYx)------------------------------------------------------------------------------------------------
#define MC_KEY1 1
#define MC_KEY2 2
#define MC_KEY3 3
#define MC_KEY4 4
//Tmrcode definition(the mcode when tmr send for change LEDx) 
#define MC_TMR0 6
#define MC_TMRSYN 7
#define MC_TMR1 8
#define MC_TMR2 9
//index of state array
#define KEY2_IDX 0
#define KEY3_IDX 1
#define KEY4_IDX 2
//state of three LED£ºLED2,LED3,LED4
OS_EXT KEYSTATE_TYPE keystate[3];
//semaphore for 1.Synchronous 2.task initially give up CPU
OS_EVENT *sem;


//===================================================================
//Key Scan
#define KEY_NUMBER		4
extern 	INT8U 	KeySwitch[KEY_NUMBER];	//Board key switch identification, value toggled while specified key pressed.

#define	KEYSWITCH(x)	KeySwitch[x-1]

INT8U Task_KeyScan_Creat(void);

//definition of task LEDx create-----------------------------------------------------------------------------------------------------------------------
INT8U Task_LEDx_Creat(void);


//===================================================================
// prototypes
int User_App_Initial(void);
void USER_Print_Decimal(INT32U);
void USER_Print_OSIdleCtr(void);


#ifdef __cplusplus
}
#endif
#endif /* UCOSII_DEMO_H */

/************************ (C) COPYRIGHT jkuang@BUPT *****END OF FILE****/
