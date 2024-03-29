/*
********************************
*  108-2 OSP Group 5 Project  *
********************************
*/
#include "stdio.h"
#include "includes.h"

/*
*********************************************************************************************************
*                                              CONSTANTS
*********************************************************************************************************
*/

#define          TASK_STK_SIZE     512                /* Size of each task's stacks (# of WORDs)       */
#define          TASK_START_ID       0                /* Application tasks IDs                         */
#define          TASK_CLK_ID         1
#define          TASK_1_ID           2
#define          TASK_2_ID           3
#define          TASK_3_ID           4
#define          TASK_4_ID           5
#define          TASK_5_ID           6

#define          TASK_START_PRIO    10                /* Application tasks priorities                  */
#define          TASK_CLK_PRIO      11
#define          TASK_1_PRIO        12
#define          TASK_2_PRIO        13
#define          TASK_3_PRIO        14
#define          TASK_4_PRIO        15
#define          TASK_5_PRIO        16


	#define		PERIODIC_TASK_START_ID  20
	#define		PERIODIC_TASK_START_PRIO  20
	typedef struct{
		INT32U RemainTime;
		INT32U ExecutionTime;
		INT32U Period;
		INT32U Deadline;
	} TASK_EXTRA_DATA;



/*
*********************************************************************************************************
*                                              VARIABLES
*********************************************************************************************************
*/

OS_STK        TaskStartStk[TASK_STK_SIZE];            /* Startup    task stack                         */
OS_STK        TaskClkStk[TASK_STK_SIZE];              /* Clock      task stack                         */

OS_EVENT     *AckMbox;                                /* Message mailboxes for Tasks #4 and #5         */
OS_EVENT     *TxMbox;




	OS_STK TaskStk[8][TASK_STK_SIZE];
	TASK_EXTRA_DATA TaskExtraData[8];
	TASK_EXTRA_DATA tempp;
	INT8U myT;
	INT8U NumberOfTasks;
	INT8U ExecutionTime[8];
	INT8U PeriodTime[8];
	INT8U TaskNum[8];
	INT32U MyStartTime;

/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

        void  TaskStart(void *data);                  /* Function prototypes of tasks                  */
static  void  TaskStartCreateTasks(void);
static  void  TaskStartDispInit(void);
static  void  TaskStartDisp(void);
        void  TaskClk(void *data);
        void  Task1(void *data);
        void  Task2(void *data);
        void  Task3(void *data);
        void  Task4(void *data);
        void  Task5(void *data);

	void PeriodicTask(void *data);

/*$PAGE*/
/*
*********************************************************************************************************
*                                                  MAIN
*********************************************************************************************************
*/

void main (void)
{
    OS_STK *ptos;
    OS_STK *pbos;
    INT32U  size;

	FILE *InputFile;
	INT8U i;
	InputFile = fopen("Input.txt","r");
	fscanf(InputFile,"%d",&NumberOfTasks);

	for(i=0;i<NumberOfTasks;i++)
	{
		fscanf(InputFile,"%d%d",&ExecutionTime[i],&PeriodTime[i]);
		TaskExtraData[i].ExecutionTime=ExecutionTime[i]*OS_TICKS_PER_SEC;
		TaskExtraData[i].Period=PeriodTime[i]*OS_TICKS_PER_SEC;
		TaskExtraData[i].Deadline=PeriodTime[i]*OS_TICKS_PER_SEC;
		TaskExtraData[i].RemainTime=ExecutionTime[i]*OS_TICKS_PER_SEC;
		TaskNum[i] = i+1 ;
	}
	fclose(InputFile);

    PC_DispClrScr(DISP_FGND_WHITE);                        /* Clear the screen                         */

    OSInit();                                              /* Initialize uC/OS-II                      */

    PC_DOSSaveReturn();                                    /* Save environment to return to DOS        */
    PC_VectSet(uCOS, OSCtxSw);                             /* Install uC/OS-II's context switch vector */

    PC_ElapsedInit();                                      /* Initialized elapsed time measurement     */

    ptos        = &TaskStartStk[TASK_STK_SIZE - 1];        /* TaskStart() will use Floating-Point      */
    pbos        = &TaskStartStk[0];
    size        = TASK_STK_SIZE;
    OSTaskStkInit_FPE_x86(&ptos, &pbos, &size);            
    OSTaskCreateExt(TaskStart,
                   (void *)0,
                   ptos,
                   TASK_START_PRIO,
                   TASK_START_ID,
                   pbos,
                   size,
                   (void *)0,
                   OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

    OSStart();                                             /* Start multitasking                       */
}
/*$PAGE*/
/*
*********************************************************************************************************
*                                               STARTUP TASK
*********************************************************************************************************
*/

void  TaskStart (void *pdata)
{
#if OS_CRITICAL_METHOD == 3                                /* Allocate storage for CPU status register */
    OS_CPU_SR  cpu_sr;
#endif
    INT16S     key;


    pdata = pdata;                                         /* Prevent compiler warning                 */

    TaskStartDispInit();                                   /* Setup the display                        */

    OS_ENTER_CRITICAL();                                   /* Install uC/OS-II's clock tick ISR        */
    PC_VectSet(0x08, OSTickISR);
    PC_SetTickRate(OS_TICKS_PER_SEC);                      /* Reprogram tick rate                      */
    OS_EXIT_CRITICAL();

    OSStatInit();                                          /* Initialize uC/OS-II's statistics         */

    AckMbox = OSMboxCreate((void *)0);                     /* Create 2 message mailboxes               */
    TxMbox  = OSMboxCreate((void *)0);

    TaskStartCreateTasks();                                /* Create all other tasks                   */

    for (;;) {
        TaskStartDisp();                                   /* Update the display                       */

        if (PC_GetKey(&key)) {                             /* See if key has been pressed              */
            if (key == 0x1B) {                             /* Yes, see if it's the ESCAPE key          */
                PC_DOSReturn();                            /* Yes, return to DOS                       */
            }
        }

        OSCtxSwCtr = 0;                                    /* Clear context switch counter             */
        OSTimeDly(OS_TICKS_PER_SEC);                       /* Wait one second                          */
    }
}
/*$PAGE*/
/*
*********************************************************************************************************
*                                        INITIALIZE THE DISPLAY
*********************************************************************************************************
*/

static  void  TaskStartDispInit (void)
{
	char s[80];
	INT8U i;
	INT8U j;
	INT8U k;
	INT8U m;
	INT8U n;
	INT8U temp;

	PC_DispStr( 0,  0, "                                107-2 OSP Project                               ", DISP_FGND_WHITE + DISP_BGND_RED + DISP_BLINK);
	PC_DispStr( 0,  1, "                                    Group 5                                     ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	PC_DispStr( 0,  2, "                                    B0429047                                    ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	PC_DispStr( 0,  3, "                                    B0529003                                    ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	PC_DispStr( 0,  4, "                                    B0529048                                    ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	PC_DispStr( 0,  5, "                                    B0529031                                    ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	PC_DispStr( 0,  6, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	PC_DispStr( 0,  7, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	PC_DispStr( 0,  8, "Task          Start Time  End Time  Deadline   Period  ExcuctionTime    Run     ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	PC_DispStr( 0,  9, "----          ----------  --------  --------   ------  -------------    ---     ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	for (k = NumberOfTasks - 1; k > 0; k--)
	{
		for (m = 0; m <= k - 1; m++)
		{
			if (PeriodTime[m] > PeriodTime[m + 1])
			{
				temp = PeriodTime[m];
				PeriodTime[m] = PeriodTime[m + 1];
				PeriodTime[m + 1] = temp;

				temp = ExecutionTime[m];
				ExecutionTime[m] = ExecutionTime[m + 1];
				ExecutionTime[m + 1] = temp;
				
				temp = TaskNum[m];
				TaskNum[m] = TaskNum[m+1];
				TaskNum[m+1] = temp ;
				
				tempp = TaskExtraData[m];
				TaskExtraData[m] = TaskExtraData[m+1];
				TaskExtraData[m+1] = tempp;
			}
		}
	}
	
	/*print time*/
	for(i=0;i<NumberOfTasks;i++)
	{
		
		sprintf(s,"Task%3d :                                    %6d     %6d                  ",
            TaskNum[i],
            PeriodTime[i],
            ExecutionTime[i]
		);
		PC_DispStr(0,10+i,s,DISP_FGND_BLACK+DISP_BGND_LIGHT_GRAY);
	}
	for(j=(NumberOfTasks+10);j<22;j++)
	{
		PC_DispStr(0, j, "                                                                                ", DISP_FGND_BLACK+DISP_BGND_LIGHT_GRAY);
	}

    PC_DispStr(0, 22, "#Tasks    :   CPU Usage  %                                                      ",DISP_FGND_BLACK+DISP_BGND_LIGHT_GRAY);
    PC_DispStr(0, 23, "#Task switch/sec                                                                ",DISP_FGND_BLACK+DISP_BGND_LIGHT_GRAY);
    PC_DispStr(0, 24, "              <-PRESS ESC TO QUIT->                                             ",DISP_FGND_BLACK+DISP_BGND_LIGHT_GRAY+DISP_BLINK);
}

/*$PAGE*/
/*
*********************************************************************************************************
*                                           UPDATE THE DISPLAY
*********************************************************************************************************
*/

static  void  TaskStartDisp (void)
{
    char   s[80];


    sprintf(s, "%5d", OSTaskCtr);                                  /* Display #tasks running               */
    PC_DispStr(18, 22, s, DISP_FGND_YELLOW + DISP_BGND_BLUE);

    sprintf(s, "%3d", OSCPUUsage);                                 /* Display CPU usage in %               */
    PC_DispStr(36, 22, s, DISP_FGND_YELLOW + DISP_BGND_BLUE);

    sprintf(s, "%5d", OSCtxSwCtr);                                 /* Display #context switches per second */
    PC_DispStr(18, 23, s, DISP_FGND_YELLOW + DISP_BGND_BLUE);

    sprintf(s, "V%4.2f", (float)OSVersion() * 0.01);               /* Display uC/OS-II's version number    */
    PC_DispStr(75, 24, s, DISP_FGND_YELLOW + DISP_BGND_BLUE);

    switch (_8087) {                                               /* Display whether FPU present          */
        case 0:
             PC_DispStr(71, 22, " NO  FPU ", DISP_FGND_YELLOW + DISP_BGND_BLUE);
             break;

        case 1:
             PC_DispStr(71, 22, " 8087 FPU", DISP_FGND_YELLOW + DISP_BGND_BLUE);
             break;

        case 2:
             PC_DispStr(71, 22, "80287 FPU", DISP_FGND_YELLOW + DISP_BGND_BLUE);
             break;

        case 3:
             PC_DispStr(71, 22, "80387 FPU", DISP_FGND_YELLOW + DISP_BGND_BLUE);
             break;
    }
}

/*$PAGE*/
/*
*********************************************************************************************************
*                                             CREATE TASKS
*********************************************************************************************************
*/
static void TaskStartCreateTasks(void)
{
	INT8U i;
	char s[80];
	MyStartTime=OSTimeGet();

	OSTaskCreateExt(TaskClk,
		(void *)0,
		&TaskClkStk[TASK_STK_SIZE-1],
		TASK_CLK_PRIO,
		TASK_CLK_ID,
		&TaskClkStk[0],
		TASK_STK_SIZE,
		(void *)0,
		OS_TASK_OPT_STK_CHK|OS_TASK_OPT_STK_CLR
	);

	for(i=0;i<NumberOfTasks;i++)
	{
		OSTaskCreateExt(PeriodicTask,
			(void *)0,
			&TaskStk[i][TASK_STK_SIZE-1],
			(PERIODIC_TASK_START_PRIO+i),
			(PERIODIC_TASK_START_ID+i),
			&TaskStk[i][0],
			TASK_STK_SIZE,
			&TaskExtraData[i],
			OS_TASK_OPT_STK_CHK|OS_TASK_OPT_STK_CLR
		);
	}
}
void  TaskClk (void *data)
{
    char s[40];


    data = data;
    for (;;) {
        PC_GetDateTime(s);
        PC_DispStr(60, 23, s, DISP_FGND_YELLOW + DISP_BGND_BLUE);
        OSTimeDly(OS_TICKS_PER_SEC);
    }
}
void PeriodicTask(void *pdata)
{
	
	INT8U x;
	TASK_EXTRA_DATA *MyPtr;
	char s[34];
	char p[34];
	INT8U i;
	INT32U TaskTime;
	
	pdata=pdata;
	MyPtr=OSTCBCur->OSTCBExtPtr;
	x=0;
	
	MyPtr->Deadline=MyStartTime+MyPtr->Period;
	MyPtr->RemainTime=MyPtr->ExecutionTime;
	for(;;)
	{
		x++;myT++;
		sprintf(s,"%4d",x);
		PC_DispStr(71,10+OSPrioCur-PERIODIC_TASK_START_PRIO,s,DISP_FGND_RED+DISP_BGND_LIGHT_GRAY);//run
		sprintf(s,"%10d",OSTimeGet());
		PC_DispStr(14,10+OSPrioCur-PERIODIC_TASK_START_PRIO,s,DISP_FGND_YELLOW+DISP_BGND_LIGHT_GRAY);//start time
		sprintf(s,"%10d",MyPtr->Deadline);
		PC_DispStr(34,10+OSPrioCur-PERIODIC_TASK_START_PRIO,s,DISP_FGND_BLUE+DISP_BGND_LIGHT_GRAY);//dead line
		
		sprintf(s,"%3d",TaskNum[OSPrioCur-PERIODIC_TASK_START_PRIO]);
		PC_DispStr(0+3*myT,19,s,DISP_FGND_BLUE+DISP_BGND_LIGHT_GRAY);
		sprintf(s,"%3d",OSTimeGet()/OS_TICKS_PER_SEC);
		PC_DispStr(0+3*myT,20,s,DISP_FGND_BLUE+DISP_BGND_LIGHT_GRAY);
		
		while(1)
		{
			
			if(MyPtr->RemainTime<=0)
			{
				break;
			}
		}
		
		MyPtr->Deadline=MyPtr->Deadline+MyPtr->Period;
		MyPtr->RemainTime=MyPtr->ExecutionTime;

		sprintf(s,"%10d",OSTimeGet());
		PC_DispStr(24,10+OSPrioCur-PERIODIC_TASK_START_PRIO,s,DISP_FGND_GREEN+DISP_BGND_LIGHT_GRAY);//end time

		if(MyPtr->Deadline-MyPtr->Period >OSTimeGet() )
		{
			OSTimeDly(MyPtr->Deadline - MyPtr->Period - OSTimeGet());
		}
	}
}
