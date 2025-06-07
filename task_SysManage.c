#include	"uCOSII_Demo.h"
//task SysManage base parameters------------------------------------------------------------------------------------------------------------------------
#define TASK_SysManage_PRIO    	25
static OS_STK task_SysManage_stk[STK_SIZE_DEF];
static void task_SysManage(void *p_arg);

//MsgQ of task SysManage---------------------------------------------------------------------------------------------------------------------------------
#define TASK_SysManage_QSIZE		10
static void *TaskSysManage_Qarray[TASK_SysManage_QSIZE];

//--------------SysManage Task--------------------
#define PRINT_EN_KEYSCAN
// Key Value
#define KEY_UP				1
#define KEY_DOWN			0

//declaration of function 
static void TmrSynCallbackFnct (void *p_arg);

//============================================================
//task SysManage Create
INT8U Task_SysManage_Creat(void)
{
	INT8U err, i;

	//Key Switch Record initial
	for(i=0; i<KEY_NUMBER; i++)
	{
		KEYSWITCH(i+1) = 1;
	}
	
	//Creat SysManage task
	err = OSTaskCreate(	task_SysManage, (void*)0,
											&task_SysManage_stk[STK_SIZE_DEF-1], TASK_SysManage_PRIO);
	return(err);
}

//SysManageµÄÈÎÎñº¯Êý
static void task_SysManage(void *p_arg)
{
	
	//register APP_TID_SysManage and TASK_SysManage_PRIO--------------------------------------------------------------------------------------------------
	APP_TPRIO(APP_TID_SysManage) = TASK_SysManage_PRIO;
	//Create Q of task SysManage--------------------------------------------------------------------------------------------------------------------------
	APP_TQID(APP_TID_SysManage) = 
							OSQCreate(&TaskSysManage_Qarray[0], TASK_SysManage_QSIZE);
	if(APP_TQID(APP_TID_SysManage) == NULL)
	{
		APP_TPRIO(APP_TID_SysManage) = 0xFF;
		OSTaskDel(OS_PRIO_SELF);
		return;
	}
	//create a task count to determine whether synchornized-----------------------------------------------------------------------------------------------
	INT8U task_cnt=0;
	//create a synTimer for synchronize
	static OS_TMR *tmrSyn; 
	INT8U err;
	tmrSyn = OSTmrCreate((INT32U )5,
												(INT32U )0,
												(INT8U  )OS_TMR_OPT_ONE_SHOT,
												(OS_TMR_CALLBACK)TmrSynCallbackFnct,
												(void * )0,
												(INT8U *)"TmrSyn",
												(INT8U *)&err);
	if(err)
	{
		OSQDel(APP_TQID(APP_TID_tmrtest),OS_DEL_ALWAYS,&err);
		APP_TQID(APP_TID_tmrtest) = NULL;
		APP_TPRIO(APP_TID_tmrtest) = 0xFF;
		OSTaskDel(OS_PRIO_SELF);
		return;
	}
	//task KeyScan Create------------------------------------------------------------------------------------------------------------------------------
	err = Task_KeyScan_Creat();
	if(err)
		return;
	
	//task LEDx Create--------------------------------------------------------------------------------------------------------------------------------
	err = Task_LEDx_Creat();
	if(err)
		return;
	
	 //the circulation for Synchronous--------------------------------------------------------------------------------------------------------------------
    while(1){
        //utilize timer to check MsgQ
        //start timer
				OSTmrStart(tmrSyn, &err);  //start timer for synchronize
        //pend MsgQ
        MESSAGE_HEAD *msgP=(MESSAGE_HEAD *)OSQPend(APP_TQID(APP_TID_SysManage), 0, &err);
        if(msgP->mCode==MC_TMRSYN)
				{
						USER_USART1_print("TIME OUT ERROR");
						return;
				}
        if(msgP->mSendTsk==APP_TID_keyscan||msgP->mSendTsk==APP_TID_LEDx)
        {
            task_cnt++;
        }
        if(task_cnt>=2)
            break;
    }
	
	USER_USART1_print("\n====task LEDx and keyscan Created====\n");

	while(1)
	{	       
		
				USER_USART1_print("Key 4\n");

	}
		
}

//============================================================
static void TmrSynCallbackFnct (void *p_arg)
{
	//send msg to SysManage task Q
	Msg_SendShort(MC_TMRSYN, APP_TID_SysManage, APP_TID_SysManage, 0, 0);
}
