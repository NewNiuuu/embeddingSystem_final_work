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

//declaration of Tmr0 callback function 
static void Tmr0CallbackFnct (void *p_arg);
//declaration of Tmr1 callback function 
static void Tmr1CallbackFnct (void *p_arg);
//declaration of Tmr2 callback function 
static void Tmr2CallbackFnct (void *p_arg);

static void LED_off(uint8_t led_num);
static void LED_on(uint8_t led_num);
static void LED_toggle(uint8_t led_num);

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
	INT8U err,i;
	//register APP_TID_SysManage and TASK_SysManage_PRIO--------------------------------------------------------------------------------------------------
	APP_TPRIO(APP_TID_LEDx) = TASK_LEDx_PRIO;
	//Create Q of task LEDx-------------------------------------------------------------------------------------------------------------------------------
	APP_TQID(APP_TID_LEDx) = 
							OSQCreate(&TaskLEDx_Qarray[0], TASK_LEDx_QSIZE);
	
	if(APP_TQID(APP_TID_LEDx) == NULL)
	{
		USER_USART1_print("\n Fail to create LEDx Msg Queue");
		APP_TPRIO(APP_TID_LEDx) = 0xFF;
		OSTaskDel(OS_PRIO_SELF);
		return;
	}
	/*create 3 timer: tmr0, tmr1 and tmr2 for LED TOGGLE*/
	//tmr0----------------------------------------------------------------------------------------------------------------------------------------
	OS_TMR *tmr0 = OSTmrCreate((INT32U )10,
												(INT32U )0,
												(INT8U  )OS_TMR_OPT_ONE_SHOT,
												(OS_TMR_CALLBACK)Tmr0CallbackFnct,
												(void * )0,
												(INT8U *)"Tmr0",
												(INT8U *)&err);
	if(err)
	{
		OSQDel(APP_TQID(APP_TID_LEDx),OS_DEL_ALWAYS,&err);
		APP_TQID(APP_TID_LEDx) = NULL;
		APP_TPRIO(APP_TID_LEDx) = 0xFF;
		OSTaskDel(OS_PRIO_SELF);
		return;
	}
	
	//tmr1-------------------------------------------------------------------------------------------------------------------------------------------
	OS_TMR *tmr1 = OSTmrCreate((INT32U )10,
												(INT32U )10,
												(INT8U  )OS_TMR_OPT_PERIODIC,
												(OS_TMR_CALLBACK)Tmr1CallbackFnct,
												(void * )0,
												(INT8U *)"Tmr1",
												(INT8U *)&err);
	if(err)
	{
		OSQDel(APP_TQID(APP_TID_LEDx),OS_DEL_ALWAYS,&err);
		APP_TQID(APP_TID_LEDx) = NULL;
		APP_TPRIO(APP_TID_LEDx) = 0xFF;
		OSTaskDel(OS_PRIO_SELF);
		return;
	}
	//tmr2---------------------------------------------------------------------------------------------------------------------------------------------
	OS_TMR *tmr2 = OSTmrCreate((INT32U )5,
												(INT32U )5,
												(INT8U  )OS_TMR_OPT_PERIODIC,
												(OS_TMR_CALLBACK)Tmr2CallbackFnct,
												(void * )0,
												(INT8U *)"Tmr2",
												(INT8U *)&err);
	if(err)
	{
		OSQDel(APP_TQID(APP_TID_LEDx),OS_DEL_ALWAYS,&err);
		APP_TQID(APP_TID_LEDx) = NULL;
		APP_TPRIO(APP_TID_LEDx) = 0xFF;
		OSTaskDel(OS_PRIO_SELF);
		return;
	}
	USER_USART1_print("\n====Task LEDx initialized====\n");
	Msg_SendShort(MC_TASK_LEDx, APP_TID_LEDx, APP_TID_SysManage, 0, 0);//send msg to LEDx task Q
	//suspend on the extern var sem after init--------------------------------------------------------------------------------------------------------
	OSSemPend (sem, 0, &err);
	OSSemPost(sem);
	
	//use a poniter point to cur_state
				static KEYSTATE_TYPE* cur_state=NULL;
	//
				static INT16U led_to_operate = 0;
	
	/*
	while(1)
	{	       
				
				MESSAGE_HEAD *msgP=(MESSAGE_HEAD *)OSQPend(APP_TQID(APP_TID_LEDx), 0, &err);
				if(err != OS_ERR_NONE){
					continue;
					}
				
				//set a local var to cut down mem assess times
				INT16U mcode=msgP->mCode;
				//key pressed flag, to avoid tmr msg changing the key state(go into the second switch)
				static INT8U if_key_press = 0;
				USER_USART1_print("\n mcode=");
				USER_Print_Decimal(mcode);
				
				if(mcode<5){
					//key press flag set 1
						if_key_press=1;
						LED_on(mcode);
						err=OSTmrStart(tmr0,&err);
						if(err==OS_FALSE){
							USER_USART1_print("\n ERR:tmr0 fail to start!");
							return;
						}
				}
				switch(mcode){
					//case key preprocess-----------------------------------------------------------------------------------------------------------------------
					case MC_KEY1:{
							for(i=0;i<NUM_KEY_CHG;i++){
							keystate[i]=0;
							}
							break;
					}
					case MC_KEY2:{
							cur_state = &keystate[KEY2_IDX];
							led_to_operate=2;
							break;
					}
					case MC_KEY3:{
							cur_state = &keystate[KEY3_IDX];
							led_to_operate=3;
							break;
					}
					case MC_KEY4:{
							cur_state = &keystate[KEY4_IDX];
							led_to_operate=4;
							break;
					}
					//case tmr---------------------------------------------------------------------------------------------------------------------------------
					case MC_TMR0:{
							LED_toggle(led_to_operate);
							break;
					}
					case MC_TMR1:{
							LED_toggle(led_to_operate);
							break;
					}
					case MC_TMR2:{
							LED_toggle(led_to_operate);
							break;
					}
					default:{
							USER_USART1_print("\n WARNING: msg handler into default.");
							break;
					}
				}
				if(if_key_press){
							USER_USART1_print("\n if_key_press=");
							USER_Print_Decimal(if_key_press);
						//change 1.keystate poniter 2.LEDx sparkle state
							switch(*cur_state){
							case 0:{
									*cur_state=1;
								//make sure none of tmr1 and tmr2 start
									OSTmrStop(tmr1, OS_TMR_OPT_NONE,(void*)0,&err);
									OSTmrStop(tmr2, OS_TMR_OPT_NONE,(void*)0,&err);
									LED_on(led_to_operate);
									break;
							}
							case 1:{
									*cur_state=2;
								//make sure none of tmr0 and tmr2 start
									OSTmrStop(tmr0, OS_TMR_OPT_NONE,(void*)0,&err);
									OSTmrStop(tmr2, OS_TMR_OPT_NONE,(void*)0,&err);
									err = OSTmrStart(tmr1, &err); 
									if(err==OS_FALSE){
											USER_USART1_print("\n ERR:tmr1 fail to start!");
											return;
										}
									break;
							}
							case 2:{
									*cur_state=0;
								//make sure none of tmr0 and tmr1 start
									OSTmrStop(tmr0, OS_TMR_OPT_NONE,(void*)0,&err);
									OSTmrStop(tmr1, OS_TMR_OPT_NONE,(void*)0,&err);
									err = OSTmrStart(tmr2, &err);
									if(err==OS_FALSE){
										USER_USART1_print("\n ERR:tmr2 fail to start!");
										return;
									}
									LED_off(led_to_operate);
									break;
							}
							default:{
								USER_USART1_print("\n WARNING: state handler into default.");
									break;
							}
							

						}
				}
					
				
				
				Msg_MemPut(msgP);

	}*/
		while(1)
	{	       
		MESSAGE_HEAD *msgP=(MESSAGE_HEAD *)OSQPend(APP_TQID(APP_TID_LEDx), 0, &err);
		if(err != OS_ERR_NONE){
			continue;
		}
		
		INT16U mcode=msgP->mCode;
		USER_USART1_print("\n LEDx received mcode=");
		USER_Print_Decimal(mcode);
		
		//case key preprocess-----------------------------------------------------------------------------------------------------------------------
		switch(mcode)
		{
			case MC_KEY1:
				// KEY1:reset all 3 keys' states
				for(i=0; i<NUM_KEY_CHG; i++){
					keystate[i] = 0;
				}
				//make sure none of tmr1 and tmr2 start
				OSTmrStop(tmr1, OS_TMR_OPT_NONE,(void*)0,&err);
				OSTmrStop(tmr2, OS_TMR_OPT_NONE,(void*)0,&err);
				// turn off all LED
				LED_off(2); LED_off(3); LED_off(4);
				USER_USART1_print(" -> ALL STATES RESET");
				break;

			case MC_KEY2:
				cur_state = &keystate[KEY2_IDX];
				led_to_operate = 2;

				// --- ???????????? ---
				switch(*cur_state) {
					case 0: // ???0?????1 (??)
						*cur_state = 1;
						OSTmrStop(tmr1, OS_TMR_OPT_NONE,(void*)0,&err);
						OSTmrStop(tmr2, OS_TMR_OPT_NONE,(void*)0,&err);
						LED_on(led_to_operate);
						USER_USART1_print(" -> LED2 State[ON]");
						break;
					case 1: // ???1?????2 (??)
						*cur_state = 2;
						OSTmrStart(tmr1, &err);
						USER_USART1_print(" -> LED2 State[SLOW BLINK]");
						break;
					case 2: // ???2?????0 (??)
						*cur_state = 0;
						OSTmrStop(tmr1, OS_TMR_OPT_NONE,(void*)0,&err);
						OSTmrStart(tmr2, &err);
						USER_USART1_print(" -> LED2 State[FAST BLINK]");
						break;
				}
				// --- ????? ---
				break; // MC_KEY2 ????

			case MC_KEY3:
				cur_state = &keystate[KEY3_IDX];
				led_to_operate = 3;
				// ... ?????KEY3???????? ...
				USER_USART1_print(" -> KEY3 pressed");
				break;

			case MC_KEY4:
				cur_state = &keystate[KEY4_IDX];
				led_to_operate = 4;
				// ... ?????KEY4???????? ...
				USER_USART1_print(" -> KEY4 pressed");
				break;

			// tmr1/tmr2:toggle led
			case MC_TMR1:
			case MC_TMR2:
				if(led_to_operate > 0) {
					LED_toggle(led_to_operate);
				}
				break;
			
				// tmr0: press any key just sparkle once
			case MC_TMR0:
				if(led_to_operate > 0) {
					LED_toggle(led_to_operate);
				}
				break;

			default:
				USER_USART1_print("\n WARNING: unhandled mcode.");
				break;
		}
		
		Msg_MemPut(msgP);
	}
		
  
}

//tmr0 callback function
static void Tmr0CallbackFnct (void *p_arg)
{
	Msg_SendShort(MC_TMR0, APP_TID_LEDx, APP_TID_LEDx, 0, 0);//send msg to LEDx task Q
}
//tmr1 callback function
static void Tmr1CallbackFnct (void *p_arg)
{
	Msg_SendShort(MC_TMR1, APP_TID_LEDx, APP_TID_LEDx, 0, 0);//send msg to LEDx task Q
}
//tmr2 callback function
static void Tmr2CallbackFnct (void *p_arg)
{
	Msg_SendShort(MC_TMR2, APP_TID_LEDx, APP_TID_LEDx, 0, 0);//send msg to LEDx task Q
}

// LED_XXX()
static void LED_off(uint8_t led_num) {
    switch(led_num) {
        case 1: LED1_OFF; break;
        case 2: LED2_OFF; break;
        case 3: LED3_OFF; break;
        case 4: LED4_OFF; break;
        default: break; 
    }
}

static void LED_on(uint8_t led_num) {
    switch(led_num) {
        case 1: LED1_ON; break;
        case 2: LED2_ON; break;
        case 3: LED3_ON; break;
        case 4: LED4_ON; break;
        default: break; 
    }
}

static void LED_toggle(uint8_t led_num) {
    switch(led_num) {
        case 1: LED1_TOGGLE; break;
        case 2: LED2_TOGGLE; break;
        case 3: LED3_TOGGLE; break;
        case 4: LED4_TOGGLE; break;
        default: break; 
    }
}


