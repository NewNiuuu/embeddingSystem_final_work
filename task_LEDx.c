#include	"uCOSII_Demo.h"
//task LEDx base parameters-----------------------------------------------------------------------------------------------------------------------------
#define TASK_LEDx_PRIO    	23
static OS_STK task_LEDx_stk[STK_SIZE_DEF];
static void task_LEDx(void *p_arg);

//MsgQ of task LEDx ------------------------------------------------------------------------------------------------------------------------------------
#define TASK_LEDx_QSIZE		10
static void *TaskLEDx_Qarray[TASK_LEDx_QSIZE];


//--------------LEDx Task--------------------
#define PRINT_EN_KEYSCAN
// Key Value
#define KEY_UP				1
#define KEY_DOWN			0

//============================================================
//task LEDx Create
INT8U Task_LEDx_Creat(void)
{
	INT8U err, i;

	//Key Switch Record initial
	for(i=0; i<KEY_NUMBER; i++)
	{
		KEYSWITCH(i+1) = 1;
	}
	
	//Creat Key scan task
	err = OSTaskCreate(	task_LEDx, (void*)0,
											&task_LEDx_stk[STK_SIZE_DEF-1], TASK_LEDx_PRIO);
	return(err);
}

//LEDxµÄÈÎÎñº¯Êý
static void task_LEDx(void *p_arg)
{
	
	//register APP_TID_SysManage and TASK_SysManage_PRIO--------------------------------------------------------------------------------------------------
	APP_TPRIO(APP_TID_LEDx) = TASK_LEDx_PRIO;
	//Create Q of task LEDx-------------------------------------------------------------------------------------------------------------------------------
	APP_TQID(APP_TID_LEDx) = 
							OSQCreate(&TaskLEDx_Qarray[0], TASK_LEDx_QSIZE);
	if(APP_TQID(APP_TID_LEDx) == NULL)
	{
		APP_TPRIO(APP_TID_LEDx) = 0xFF;
		OSTaskDel(OS_PRIO_SELF);
		return;
	}
	

	USER_USART1_print("\n====Task LEDx Created====\n");

	while(1)
	{	       
		
				USER_USART1_print("Key 4\n");

	}
		
		
  
}
