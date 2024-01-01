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
	InputFile = fopen("Input2.txt","r");
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
        //TaskStartDisp();                                   /* Update the display                       */

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

	PC_DispStr( 0,  0, "                                108-2 OSP Project                               ", DISP_FGND_WHITE + DISP_BGND_RED + DISP_BLINK);
	PC_DispStr( 0,  1, "                                    Group 05                                    ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	PC_DispStr( 0,  2, "                                    B0529003                                   " , DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	PC_DispStr( 0,  3, "                                    B0529031                                    ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	PC_DispStr( 0,  4, "                                    B0529048                                    ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	PC_DispStr( 0,  5, "                                    B0429047                                        ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
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

    //PC_DispStr(0, 22, "#Tasks    :   CPU Usage  %                                                      ",DISP_FGND_BLACK+DISP_BGND_LIGHT_GRAY);
    //PC_DispStr(0, 23, "#Task switch/sec                                                                ",DISP_FGND_BLACK+DISP_BGND_LIGHT_GRAY);
    //PC_DispStr(0, 24, "              <-PRESS ESC TO QUIT->                                             ",DISP_FGND_BLACK+DISP_BGND_LIGHT_GRAY+DISP_BLINK);
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
		sprintf(s,"%10d",OSTimeGet()/OS_TICKS_PER_SEC); //output start
		PC_DispStr(14,10+OSPrioCur-PERIODIC_TASK_START_PRIO,s,DISP_FGND_YELLOW+DISP_BGND_LIGHT_GRAY);//start time
		sprintf(s,"%10d",MyPtr->Deadline/OS_TICKS_PER_SEC);//output deadline
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

		sprintf(s,"%10d",OSTimeGet()/OS_TICKS_PER_SEC);//output end time
		PC_DispStr(24,10+OSPrioCur-PERIODIC_TASK_START_PRIO,s,DISP_FGND_GREEN+DISP_BGND_LIGHT_GRAY);//end time

		if(MyPtr->Deadline-MyPtr->Period >OSTimeGet() )
		{
			OSTimeDly(MyPtr->Deadline - MyPtr->Period - OSTimeGet());
		}
	}
}
/*
. .     . . ........... ......... ......... . . . ... . . . ..................:...:::..:rr:.  ...::.::i:......::rr7i:..     ........:::.:::.............................................................
                                                                           . ..........iv7:..:ir...r:..:ii::::::::.   .   .:...:::::.............. ................................................... .
. .   .   . . . . . . ..... ... . . . . . . . .   . . . . . . . ... ..... .  ...... .::.iuPdKuYriii:::ir7vPMDu7. .......:rJsLrriiii:ii:.................................................................
.      .   . . . ... ....... . . . . . .         . .     . ...  ...  .......:7r:. .:i::7YSI7LDDj:rYJvr..:777vKD1:::..:7qDMKuYKZP5ULLLsYYv7i:.....:.................................................... .
.   .   . ... . ........... . .   .       .               . .    ...... .ivvvr7ri.:sKY7r:7d1vubXJYI5KX1:.rSJriUPPuuuSEBBU.::.     ..::r7sLYvvi:.:::.... ..:.............................................
.  . .   . ................. . . . .     .               . .   ...   ..:..iLjXKbdPUU11j7.vMKr7JK5XXdZPUPdv..JgZQDqbBQPY17i::.:::...........i7L77i..:::................................................ .
. . . . . ............... . . . . . .                       .   .   ..:::...YQQYLu17irDQZ5Xv7vL1DgEIXgBBd1JIBBgZgEMBBd5UXPMqv:::i:.         ...:r77:..:.......:....................................... .
.  . . . ................. . . . . .   .                     ...   .....:7I5vJv7UP2bP2XgZZdPgBQPqDggDMEX1ZQRbqKZQBDqKgMbU5PQQBgSYYYIJi...........:::::................................................ .
. . . . ................... . . . . .                             ..  .:1X27v2PPDgggBBEJPQBdqZBQMbDQBPJ5BBgPgMDPZQQPqEgbgMZKDMRgQQggQqIJv:...    ..:i:::.............................................  .
.  . ............. ... ... . . . . . .               ... .    ..   r5gdqdRXSdQgZPgQgPDDggDPgMgDDgMZZEQQQZDZEDBQP5dgMggbPdQQMbqSPdbUXKISgMgEZP1r:.   .::::............................................. .
. ......................... . .   .   .             ...   ..:iri7IQBBMggDPZgMRMPZDgMBQgbdZQQQEZDQRgEZdZPddgZZdggQRgZgMQMdSPEggDPqqZZZqqPggQDggBQDJi..................................................  .
. .......................... . . . . . .           .      .7JZBBBQZMDZPPbMRQgQDZEgbDZgDgZggMgggQMgZDEEPdEgZgEEdgggdZEQRQDdPDgRDDEgDMZZqqIPdRgb2jJISKLii:  .i. ........................................ .
. ....................... . . . . .           .  ..     ..sMMDQQRZggMDggQgMEZZgZggDEZdgMQDZdgRQgggQgMDDDgMQMRDDZEPEPZDgggDgZgDMgggMDgZP1uuKERggbdbqviij7:::  .:......................................  .
. .......................... . . . . . . .   . ...      :QBQgREPPDMREPPgRQQRgRRQQQRQgDZQRMEDgQQRZMMQgMDMgRMRMRggdZEgZgEZEggDEZDgggZZZgMgXusIKDgQQBBQXYrrir.  ........................................  .
. ............................. . . . . . . .  ..   .. .IBBSqDRRQggZDDMRRgMMQgggDEDZggMgMgMgRgRgMgMDgDgDgZgZgDMDgZggRZgZDggggEDZgZDZgMQgEKbEgddKPqbPDQBMuvr...  .....................................  .
. .................................... . . . .   ....:rdBgqggDEDDRMRgRMRggZgMgDRZZgRMQMMgRgRgggggQMRgMMQMgZggMZDDgggDgZMDRggggDgDgZMDRDMRQRZXqERD5YIbZSdBBP7rr.......................................  .
. ................................. . . .   . .   .iYIQQBRgZMggDRgMDMgMEDgggQDMQQgQRMgRgQgMDMgQgggRgggQMMDgZggggRMRZDZggRDgZggMDgDgggEggQggPqKgRQbSIPdbPqKdP2r:.................................... .  .
. .................................... . .....    .rSRQgbZgMgMgQggEDZMggDgZgDgZMgggMgMgRMRgRgQgRgMgRgRgRgRgggMgRgRDgZgDgZgDMggZgZgggDgdbPgDQQgqXSdZZDgKuvSbbPIr:.................................. ..  .
. ...................................  .:...  ..iv2PQBBRDPgQRDgZgDgZggMgRZgDggMgMgRgRgRgRgRgRgRgMMQMRMMMRgRgMgQgMZgDggggDZgggZDZDDgZDEMgMgMDEqEZMggPPSXXbPKUUuYvr:..  ....... .......................  .
. ................................... .... .  :UZBBBQRDRMMDgEgZgDgDRgRgRgggMgRgRgRgMgRgRgRMRMRgQgRgMgRgMgRgMgRRRDDEggQgMZZEgDgZgEZdDEDZgEDggZZZBQgbDuvqRPqqEIL7L7:.. ..... ........................... .
. .......................................  ...:JRBBQgbEDMggZggMDgggDggMgMgMMRgMgRgMgMgRgRgRgQMQgMgMgRMRgRgMgMMQggZDDggMggEgZggRZgEDdDZZKPPgRBRDXKPPUUPQMbKu7XgKs7::.......  .........................  .
. .................................... ....   .vgBQQgggRggDMgQggZgDgDgMRMRMQgRgMgRgMgRgRgRgQgRgRgMgRgMgRgRgRgMgMMQgMgDDgDgDggMMRZgZDDgdZgMqXPgEq5ZQBEK5UJqZD51UX2Yri.  . ............................  .
. ....................................  ...  :7IDgDZggQRRZgMMDgDgZggRgQMQgMgRgMgMgMgRgMgRgMMRgMMRgggRgRgRgRgRgMgRRQggZgDMMgDggggMDgDgDEbRgPUSbQMgZDPDggq5YIqU7j5K1uvi..   ...........................  .
. .................................... ..  vXdgQEgMMDDgQggZgDgDRgRMMgMZgDMgMgMgMgRgMgMgMMRMRMRgQgRgRgRgMMRgMgRgRgRgMDgDgggDgZgZggRMMDDZZPbDMZPqgRgbbqbDgK5v7Y2YYvvYsri:... ........................... .
. .................................  ..:. :PBQQgMMBRRDMgMggZggMMRRQgMggEggMgMgMgMgRgMgMgggRgMgMgRDMgMgMgMMRgRMQMMgggRMQggZgEDZMDgZgEggBRDEgQRKSSERBQZPZbPKL:L2vr777rri:.... .........................  .
. ......................................  .7KMQQMMgMgMgMDgDggRgMgMgMMMgRMMgMgRMRgRgRgggggggRgMMRgRgMgMgMgMgMgRMRggDMMQMMZDZDEZbZEgMgbEEgDgPPbggZ2UXgBQPKKII2r:7vr7rrii::.............................  .
. .....................................   .7PdgZggMgMgRgMDgDMgRgMMRgQgMgMgMgMgRgMgMgMgMgRgMgMgRMQgMgRMRgRgRgRgQMMDggRMQgMEqKDEEEgggEggDPdEDPdRQgELvSDEMZqKU77rrr:.iJ7rri:............................  .
. ...................................:....iqBBQQggDggRgRgMgMgRgRgRgMgQgQgMgMgMgMgRgMgQgRgRgRgRgMMRgRMRMRgRgRgRMRggDRgQMRgggDKEqPgBZPdQQgdZDgEZbZRQ5vYPbZgDji.:717::JIv:Yv.  .......................... .
. ...................................  .1DQBQbZgMDgDMgMgMgMgRgRMRMRMRgRMMgMgMMRgMgRgMgQMQMRMRMMgRMRMQMQgQgRgggRgMgggRgQMgMBQbdZ5KDM51KMggS5qEqPDRZS7r7vvIgQqIsY1EZX7ri7uL:...........................  .
. ....................................  rbDMEbZQggDMgMgMgMgMgMgMgRgMMRgMgMgRgRgRgRgMgRMMgRgMgMgMgMgRgRgQgRMMgMgMggDggRgMggZEDQQDJUMEuXPDMgSqqqbQQQqsir1UrvXQBBJv2DPjvL77rr:........................... .
. ....................................  :vPDQRRgMgMgMgRgRgMgMgggRgRgRgRgMgRgMgMMRgRgMggDMgMgMMRgRgMgRgRMRgRgMggggZgZgDMgggQgEqEEPSZRDSuUQBgEd2sugBBEr.Lq57rvXbd17i71qIv:71i .........................  .
. .................................... .rZQBDDbZgMgRgMgRggDggMgRgMgMgMgMgRgRgRgRgRgRgMZgDggMgRgRgRDMgMgRMQgRgMDDEDZgDgDggQQQPdDDDK7sbEUs1EMgbXr:rPEKri7riKZuivddviiLs1Yr7Jr:.........................  .
. ................................. ..  :5QgEdgggDgDggMDgZgDMgRgRgMgMgRgRgMgRgRgQMQRQgMgMggggZMgMDMZgDggMgRgRDEdgZMMRDDZggRgDMQEEqIrr2gIiLQbPMKr7UdKJ7Lj1sjJLvI5uvuv:7bYrr7:. ..:....................  .
. .................................  :. iPQDZBRgDgDMgMgMgMZggMgMgRgMgMgMgMgMDMgRgMgQgMgRgMgMggDgDDEggMgMZZZggMDDEgDMggZgDgEgQgEDdP21sjIS7sPPPRP1UbgRQbr7SuLIL7vvrJriYUsIIJr:.  ......................  .
. .................................:.  :KBQPbRRdgggDMgMgMDggggMgggRgMgMgRgMDggMgMgMgRgMggggDgDgZgZDgQMQggEgDRMgDDEZZgggggDEEQQDKgZUSDsuPXIbdggPUSPMDZqqjugErYjr.:rri5si7Xq5r.  ........ .............  .
. ...................................  iQBKDBQbdZRDgDgDgDgDgDRgMgRgMgMgMgRMRgMgMgMgMgMgMggZgZgZDEggMDMgRgMgRgMggDZEDZgggZgDgEgDZDDSdZKSEbSUbgQDdPPqEbddP5gEIu1vi:iv517::Lbbu:. ... ... ..............  .
. .................................  ...KQMbgQgEgggggDgDggggggggMgMgMggDMgMgMgRgMgRgMgMggDgZgEDdDDgEEdDDMMRDgZgggZDdZDgDgDMZEDQZgDZPZDd5KKqZREbKbdDEb5KPd5ZBdvvvvruKXvrivjSjr........................  .
. .................................  . :dBgEqEMBMMggDgDgDgDMgMgMDMgggMgMgRgRgggMgggRgMgMggZgEZEZEDbEPdbDZMDgZDDMDgZDEDZgEDggEQMbDBEdZQgEPK5PEDggPKPQPIZMXKZRq7:752YuSqusr7sur. ... ........... . ....  .
. ..................................  .dBgPDQgZdgggDgDgZggggMgggMDMDMgMgMgMggDgDgDggMgRgMDDZDEZEZdEbDdDDgDMDMDDDQMgEEZDDDEDDQgZdMRQEbZRDdqPPEZQMZPZgPUdDPsXQBPUjSSYr5dK7ir2Ui.  .   ..... . .........  .
. .................................  :SBgddDRBDEDRgRgMgMgMDMgMgRgMgMgRgMgRggZgDgDgZDEMggDgZDZDddPDZdPDggPEEgZDEgMQgZdDZgDgEZZDdgMQEPKEDZEEPbPDDDbZDQK5gM5XPggEqP5uuX12Us77Yv:.   ... ......... . ....  .
. ................................. :EBMgRQZZEEEggggMgMgMDgDMggDgDMDgDgDMggZgDgDgZgZgggDDZggMggDgddPZEDPqSPdMgMEZEMggEbPDggDQMDPbdgZDEgZgEPKPbDZggRPSSMDbZMdDbSUIIZgKJJL1U57:.. ....  ... ... ... . .  .
. ........................ .......  :QQgRBZdEgdEDgDMgMgMgMDgggDgDgDMDgggDgDgDgZgEDEgZgEgZDEZdDdDZZdDPPPdbEPPPDDQQQZPqEDMDDZDddEMQQZbqEdEEgEDEgZDbZgMEPPPXPqPgZsjXEEPsri7UKv:..   ... . ... . . .....   .
. ................................. :1DMQDPKRQMbgDgDMggggggZMDMDMggggggZMggDgggEZbEEDDgDgZDdbPPPdZMDP5XdQRZSSqDEddQQBMEPZggdEbZDQRgddPqXPPdbZZMggQMurrLUEQQPKSqdgdPuvrv1gZ7.... ......... . . . .....  .
. ...................................7ZBZEggEDgMDgZgggggggDgDgDgDgDggMgggRDgZgZDddPEdgggggZDddPddZEgPKIqbZPbPPIXqZgRRQRMdZgQQQgEPEgBgZqPdDdZEgZZPgbj:.:7jPdEqqKqIIuuvLJXbqi ..   ......... . . . . ..  .
. .................................  .qQPPBQdqgQMZgDgZgDgZDZgggggDgDgDgDMggEZdDZDdZdEEgZgDgZgZDZgPPPDdPSS5PPgDdqXISSqXPbMRQgZEQQQggZgZdbggQQBBBRDPMgr  .IbdPDP2jI2KSUJ22I7. ... . ....... . . . . . .  .
. ..................................  vQMgggZgDDZgDgZgZgDgZgDgDgggDgggggggZdPbdgDgEZdgZZZDEMDgZZdbPdZgDdXqqbXPEb2U1I22JUSdPPXdDMddZQQRgQRDX1JusJvvLSZX: :UDEq2J5PPPqvvuXIr. ........ . . . . . . . ..  .
. ..................................  7RQgDdDgZbgDgZgDMggZgZgDDEDEgDgZgZMgdKPZgEbPZEDEgbPbgDMggdZDgbZgQDggRDXJIUqED522K5svjSEXuLvuDQQPSUU2PbRMbL7Yuiiii7qggSXSPbP1K57iLU1i. ... . . . . . . . . . . .  .
. ..................................  rERRZDEgEDDgZgDgggDDZgDRDgDgZgggDgDgPPdQZKdMqKXKPDZMgRgQgZbggggBMZXbDRdS15S55PKdPX112urirsZgYr7I2uuSPQgdqIvju7. :Ub1sbQZJISuuSY777i:..   . . . ... . . . . . ..  .
. ................................... :JQQgdDZDZMZDZgDMDgZDDgDggMDgDgZgggDEXqRQPbqSSP5KPgEPPdPEdDDZSqPZZDDQgbqb1sjX5uvvsYr7ii7dgK7rrYLUISSPqUr7ri.:ir::r1vrsZds:iLXSUvLY7:.       .     . . .     . .  .
. ...................................  idBZEbEEgggEgZMDgDDDDEgDgDgEDEDZgZDZdKEMg11dMZMP5IIIgRMggPbK512UuYJIPdDqssqJr7J1XJr:rXBdr:r7Jjv7s7vL7r7:.     . .7Zs.:XPr.UZPL7v2U7.      . .     .             .
. ...................................  .ugMZdPDDgZZEDDgDgZgDDZgggZDdZEDDDbDgg55PgDgP17vuPKKPZdRRMdbXXU2sY777v1Zgd1sPBBQEBBBDMS7iLJ7ir7Y7777iirr:...     .7viYKPvrjEZPPg5vi.     . .           . . .    .
. ....................................  iPQQbdZRDgdZZgggDDDgDgDgDgZgEgZDMQqPZdXZBQJvUSKqSJ7JXgMbUPggPSJYvLvsYriSQRRQZqSqUj5QQ2iYusrirJ2KXPbMXL5gIsi:.:.  ibQsiJbqqPESEQZjr.  . . . . . . .   . . .     .
. ................................. ... .uBBbXgRgEZZDDgDDDgZgggEZEgDgZgdgDZPddRRPYLuK5IjL7L7Y1SqEu7uPPXu1sY7vLLisQRLr7u2ur:sRji72J7JZZBBQMBBQ7uMQZqUv:irYvss7rv::rgBBQME2:    .   .     . . . . . . .  .
. .................................. .   7gBEPEMbZbEZMgRDEPgMMEDZDdZDgEZqPgREdggvrKdU2jvi7vr:::YPbSS2UYJJ1ujv2u:.dSv5buriYsqP7rd175QQgPZDEUjvvr:.7KbIsiii:  .:.  .2Bg5vvi. . . .   . . . . . .   . ..  .
. ................................... ... igBZEZgggPZggMgdDRBZEDQEZEggZZgEgdddQgJrSIvri:ii7sXPgQBBBgbKPKKI5U2Sb2jUqPP1r..i7KQbI2uIgZ5sJ7r:::i:. ..i7jvi.    :..   :7:   .  .. . . . ... .   .   . .    .
. .. ....... ... . ... ..............  .   PBDPZZMZEEDPEDgZDDEPggZEDbZdDgBgEPEQBSv7vrriir1dBQBBQRQggEZDQMEIII5sXq5PgqSuv:. .ug5isZ2ss1jYri:ii7rrii:.   .   :ii.   :7:   .    . . . . . . .   . .   .   .
. ........... ... . ......... . . .  ..    rqDMbdPEERgZDQQDbggZEDbPKPbEdZqKI5YUPKvriiiJdQMb1uJ1sL7juUj2KZK21qKIvJXSjXqIYvr. .sj:i7rLIJJ1uv7r7vsL7ii:.     .ir.  ..:7Jrirr.    . . . . .   . .       .  .
. .............................. .   ..    .JQQZPddEggqdQBMggQgEdMbqZgdDbP5S17r2qIvvYj5gZSr::rrr:::777rY1SUIXESv1B5iYq52u2Ji .iriiJP27rJUqXIv7irirri::..  .:.....  iJIvvi.   .   .   . . . .   .   .   .
. ......... . . . . . . ..... ... ... ...  iMBQPZEgERgZEMgDEZDRdgBRERggDDKu7Yr7s2s7LPdIrii7v7:ii7irvjYsJI2IU5U1YUPXj2jYYuju7: .:..77rvvr77vii:iii::.:..  .:.. ... .:2P1i.   . . . . . .   . .     . .   
. ........ . . . . . . . . . . .....   .  :ZBBEPEQQBQBBBRMgQZZDMgMDEKZMRd5v7vLLj11vJ1J7v77r7r7r77LvvvJ1I1JLJuuvuJLUbSLrvuu77i:.....:rus7ri...:i7r:.:..  .::.    ...:vPIi     . . . . . .   . .         .
. ..... . ....... ..... . .   .....  ..  7BQRMDEQMbIXPdEgPgRQgRMRdESYIRgXYv7v7vvUUJ77r77JJjLv7vL12UvvLI5IssJSUJUXuuJsvJ15uuvriiivJ77J7rr777iiiiirii::...::.    .....:JI:              .           . .   
.  . ... . . . . . ... . ... ...     .  rQBQEZDBQU:::rr7uEEbZQQDDBM2rL5Issvv77vJj1sL7YYuu222u1sJLYvssJLLvYvsvJuUJsvLLuu1JJ7i:7L7:::::::rr77L7riiiiirrri:.    ........7Ji       . . .   .   . .   .     .
. . .     .     . . . ...   . . . .     QQXQBbQD1r7ISL7:iPBRgZgEgQgjrrL77777vvLLsYJsJLLvYLsJU1U1UJsJuJJvLvLLJ15Uusu121us2J:.:r7i.    .7U1v77vvJYL777ri:.    ........:rL:    .   .     . .     .     .   
.  .             .       ...   ...     vQBbEMBBg1vJZMPu7vgBRgMgMggqJrrr777r77vvYvsYjjU1uvL777vLJsL7vvYvYLsj2U5U1Lss21UssjUv:  ....   :rYvLvJJ1Jvriiii:.. ............rr.     . .         . .           .
. .   .   .         . ... ..:..     .rEQQgMMgdQQgYLPMKYrJXRRQgggMZPs7i77L7v7v7v7LvLvLLJJ11UjjYsLYJ1JJsuJ1u1u1sLvssjsjJuJuuIr  ..:.  .r1LsvL77iiir77i:::.............:ii.      . . . .             . .  .
.  .     .         .  ...      .    iBBggQbMDdDBQu7PZ5Yri7JKSPdgDgb57rr7vYvv777v7v77r7rLsU1Uu112U5II1U12u1jY777u1IuJL1U511uv:.   .:::vLv7vvYvY7vr7riii::............i7i.   .   . . .     .       .     .
. .                     .:.        .UQBgZqEQRqgBBqISbbPJriiiv5DdZgMS7r7r77vr7r77v77r7vsvv777LLuuUsYYjJuJusjYJLJUK55JJsUuuLv77i:. .riirYLvr7rvvYv7rrrrii::.....:.....ir:.  ...     . .   .         . .  .
                       . .       :qBgqdBggMMZgZMQBdu5gSY7v7vLbRMdRMK77vvrrr7rrrrr7r77vv7rvvJJuYLr77YJ1jJLYsu15UI5522ss7vvLLL7v:. ... .i1s7rrr7rrr7riiiii::..........ir:    . .   . . .   .             .
.             .     .        .YQBBBgRQgPMQQdgQRZMRgPq5uYJssJXgQqIPBDYrsYLr777r7r777r7vv77r7r77LsuLLLuJ1juJuJ1U5UJvLL7irrrr7r7ir:. .   .rsJurrrv7rrvri::::.:.........:..         . . . .     .       .  .
             .   ...     .rZBBBBQRDZDRZdZQgEDQggDRMMEP2uJjjIXMgIJdQDvrrYv777rrr7r7rrr77v7v77r77777r77LLjJ1uI5u7vLvr:..:ri::iii:i:::ri..7J2Jvr77r:irrii::::.....:::::..             .   .   .   .       .
. .         ..   ..     rgBBBgDRBQgZgDgZggRgRgMgggQQQZPI22SXEgb77qQgXv77v77rrrrrrrrirr7r77v7v77rrr7r77vvJjU2Xsri7irr7vJs7:irrirJq5v77.:vIUs7vvv77rrrri:::::..........     ...     .   .       .     .  .
                    .  :dBggRZbgDgZRggMQggMBRgMQDRMQRMdbKqXPdEUriJbQESv7rv77rrirrrirrrrrirr7r777r777777Ls1uI217i.:7dQQDPjL7vrrvXdPusJuJ1S5LYLY77vvrrii::::::............             . .         .     .
          .     .     :IBQdKgDMggDRRgdMQRDMQQDRRRgMMQggDZPPKPXIsY75dPIur7LL77rrrrrrirrrrrrrr7r7r7r7r77vYuuIIX1YrrruXIvrr77777vYriivrrsUYU1JY1jsvvrriri:::::.....:.:..         .               . . . .   
                     iZBBQPKPZdEPDQQZDgQgggQMggRRQgMgRgQgDPPqPXEPXSdPXJYLJvvrrr7r7rririrr7r7r7rrrrirr7vss1U55XXS1uJusYY1sYvj15v:  .. .:iir7LJuj1svrrii::::..........     .     . .           .         .
.           .       7gBREPPbggREPbQMggRgMgRgQgMMQQQgRgQMMDZEZqXKEZggQKsvJYY7rrrr7r7rririrrriiirr7r7rv7LvsLJJjuXKqSqXSu1UUvLvvrr:::...::. .::irYYL7777i:.:...........    .         . . . .         . .  .
   ..    .         7QBDEEgggdZZgggZggMgRgMgggMgMMRMMgQMRgMZggD5ISdZZEquJvLLsv7rrr7rririrrrirrrrrr77vvYsjYsvssjYYYju2uuLssYii7Y77ii::::.... ...:iiiiiii::.........::::..  .               . .   . . .   .
      ..      .   LRBREbggRggEZZMgDZMDRgMDgDgZgDRgggRgMMRMMgQDZPddZEdPXJvvJsj7ririrrriririrr7r7r7rrrvvsLsLYYjsuJJsJsuj1YY7:i2I2Yi.i::::::.. ..:::..::::::.............                            . .  .
.     .          7QBDEZZEgggZZEgDgEgDgDgDggMgRgRMQMMgMgMgQgRgMMQgEqPZgq1vsYsLvrriii77rrrrrrrrrrrrrrvvL7v7LLJYYvLLYvLvjJsvr:r5qXKr....::....:.::::i::::::.........  .                     .         .   .
  :.    .       1BBMDEgEgDgZDEgDgDDDgDgggDMgMgRgQMRgMgMgRMRgggQRgqS5bEE52JJvLvvrriir7rrrrrrr7rrrrr7vJvv77vJLLvvYju2u1YuuSu:.r7Yjvii...::i:::.....::::.:.:........                                 . .   
:   ..         2BBDXZgbDZgZgZgDgggDgDMDggggMgMgMgRgRgQMQMQgMgRZgggddPZEEq5Jv7vLLrrirrrrrrrrrrrr7rrr77vrrrvLY7v77r7vYvv7vJI1s7r:i7uJYrrrri:...:irii::...:.........      .                 .             .
.        :7sQBBQEbgMMggggZgZDZgDgZgZMggDgDggMgMggZMgRMQRQgMgRZZDQBBRRggggPXJLvJvvrririrrrrrrrr7r7rrr777rvLYvLLJv7r77YvLvLvjsL7r:r77rvv7rrii:::iii::::::..........       .             . . . .   . . .  .
  .    .YBBBRQQQqXqEKEDMZDEDZgDMZDDggggggRggDggMDgDgDggQMRgQMRZgMQQMgRgMRRdKuYvv77rrrrrrrrrrrrrrr7r7r77v7v77r7r7Ys77rvYs7rrvvvrririir7rrv2SX1Y7rr7riii::........     .   .           .   . .     .     .
      igBBMgMdKXgBBdKEBRDEDZgDgDMZgDRggDRggDggMgRgggggRgMgQgRggDgEDEggRgQQQPKus7777rr77iir777rrrrr7r7r777rriiiiiLYvrrrvvvr77v7vvJ2KS5JsJ2IPPPKXjvir7r::::..... .:.    .         . .    ..     .         
    .KBBKdgQEdZMMgbbPDDgDgDgZgZgDgDggggggMDMgRgMgMgMgMgMgggMggggDgZggMgRMQMRDEX17rr7rrr777rii777rrrrr7r7rrr77YUXSqqKSSIXSKKKKgDdKPI2Yr..  .:i7Lsvv7vr::::....  rggs.   .   . ..    .   .         . .   .
  :uBBZqRRgPMQQbqqgQQdPEgDgZgZgDgDgDgDgDgDgggggDRgMgRgMgRgRgMgMgMgMDMgRggZgRQQRq1rrr7rrr7rr:ir7rrrrr777r77juIPRgZPX5SXPPEbEqSuv:.....       ...:rLYrrri:..::.  uBBB2:     .   .     .               .   
.iDBBdKDgDdDZZEDggZZEgDDZDZgZgZgDgDDDgZgZgDgDgDgggDggMgggMgMgRMRMMgRgMgRgRMRMQQQEKLriv7i:i7L77rrr7r7r77vvJ1ISKI2JJLJJ1UUjuJJ77riirii.  :........:777rr::::....:PBQDBBqi.       .....                   .
BRQEbZBgPSbZDEDEDZDDgDgDDZDDDDDDgDgDgZgDgDgggggDgDgDggMgggggMgRgRgMgMgMgMgRggEgQBQEJr:ir7vvrrirr7rrrrr7vJjUujYYvJsU1IU2JsvssjsuYvr7rri:::.:.....:irr7ri:i:.  .UPZgggQBM7    ..  . .                    .
BBRq5EgdSggDZgDZbZDMggEZZDZgEgDgZDZgZgDgDgggDgDgDgggDggMDgDMgMgMgMgMgMgMEEgQgZdMQBBR57iii777rrrrr7rri77sJ2jJvvvLr7rrrrir77rrrrrriii7rri:::.........:i7:::::: .gBQBQdSgBBg7          ..                 .
b5dMEKgggbEZDdEDggDEZDgEDZgEDZDZgZgZgDgggDgggggggggDgDgDggMDgZggMgRgMgRDgZggMDgDMgQQBQKr::7Lvrir7r7rrr7vYYsvL77rrrr77ri:::r77ir77rv7ri:::.......:.:.:i:.:....LBBgDQQQZgQBBgr    .     .             .   
dKPMgDZMgdPDDgZgZDZgZgZDEgZgZDZgZgZgDgDgDgDgDgggZgDgDgDMDgggDgZggggMggDMMQMDZgggZZZMQBQd1Liii77vrrr777r7rvvv7LsJvv7v77r7r7r7r77Yrrr7rri:::::::.....:::..::  uBBQMgbgQBZdgBBB5:   ..    .               .
DPZdDEDEgEZEgEgDgZDEDdZZgZgDDZgZgZgZgZgZgZgDgDggggggggMDggMgMgggMgMgMgggMgMDgDMgRgMggZQBBg57rr7rrrrr7r77v7v77777v77rrr7r77777rrr7rrrrrrii::::.......:...:  rBQQMQREPQQRdgqiiKs.      ..                .
EddDdZEZdgEZdZEDdZEgdDdDEDdDZDZDDgDgZDZgDgDgDgDggggMDgggDMDMDgZgDggMgMgRgMDgDggMgMggDRDEDBBBPsiii77r:irvr7r7vsLLvLv777777vvvvv7v7vvvrri:::.:............. iBQggRgQQRggZggd7iri...    ..                .
DdDdDZZdZEDdDEZEDEDEDEDEZdDEDZDZgDgZgZgDgDgZgDgDgDgDgDMggDgDgEgZgDMgMgRgMDgDMgggRggDgZMMQQQQQMdj7irr7r77YsusY7v7v7Lvv7vvLvYvY7v7vvvri::::..........:...  7QQBQgdDDQQQMgPPKgbj77i.    ..                 
ZZZDEDdDEDEgEDZDZgEgZDEDEgZgEgZDZDEgDgZgZgZgZggMDgZggMDgDgDgDgDgDgZggMggDgDMDgDgDgDggQQRZZdgQBQRd27riirvLYv7rvvLvsYJsuJuJjvv7v77777r:..:..............  iQBBPEBBQBBBQQBBgS55JISji.   .   .             .
gdgEDEDZDdDDgDgDgZgDgZgDgZDEgEgEZdDZgDgZgZgDgZgZgZgDgDgZgDgZgDgDgDgZgDgDgZgDggMDgDggQQMbbdRQQZgMBQgIJ7777r7vjsLvLvLvsLYvYv7777v7v7v7r::.............   JBBERBQqUSPRgqU5Ij7s1j227vi.                     
ZEEgZgEgZDDgDgZDZDZgZgZgDgDgDgZgDDZgZgDgZgZgDgZgDgZgZgZgZgZgDgDgDgZgggDgZggMgRDgZgggZDZRMRDgDMgRgMMBgqvi:r7YvYYuYsvLvLvL7vrvvL777vvL7r::::..........  SBBQRMPIUqS1LJj5XqI5dQEIr:i7:    .               .
DbgEgEDDgZgDDZDdZEDEgDgZgEgZgDgDgDgggDgZgDgZgDgZDZgDgDgZgDgZgEDZgZgDMgMggDggRggZDDggMZQQBgZPdDQMgZgQBBgIY777vvsLYvv7Lvvvv7v7Yv7rrr777ii::.:........ .sBBQBBg25RgdbggQQRgggBBgL::JqJ.   .                
ZZEDZgZgZgZgEDDgZgDgDgZMgMDMZgZDZgZgZgDgZgZgEDEgZgZgDgDgZDZDZgZgggZDZDZZZgDgggDgDgZgZgEgZDEgEgZgDRgdqgQBMK7i:r7v77rvLY77r7r77777rrr77vrri:::::.. .. :PMBRgdEqPZBQDI5EQDPdBBZsriYZRJi.                   
gEDDgDgDgggggDRgRgRgMggMRgggMDgDgDggggMggDgDgDgZgZgDgDgDgDgZgZgZDZgZZdEdZZgDgDMDgggDgZDEgZgDgZgDgPEMBQgdgRQqJr7rrrrir7sYY77r77v77rriiirrv7r::::.... LQQ2usjuKX51qggPEZggQdv7Kv7PBK:.i.                  
EEEDEDEgZgDggMgRgMggDgDgDgDgDMggDgDgEgEDDgZgZDZgEgZDZDZgZDZgZgZDEDEDZgDgEZEDZDEgZgEDZgDgZgZgDgDgDgDRggZgMBQQMK7riirsJviir7riirirriirii:iirrrii:.    UBR7:7XRMgdqdQDdEgdZbPqq2r7QEririi.                .
ZPEdZdDdDEgZDEgZZEDZZdZZDZgZDDDZgDgEDEDZDZgZDEDZDZDEgZgZgZgZDEDEZZgZggRggZgZDZDEDEZdDZDDgZDDgDgDgMQEPPgQQgDgBBRIvriirr7rrr777rririrr777rririi...:rIZ2isPPJ2gREgqdgQdZgBQRgDXJ7Jur:irr:                  
EddZEZEDZDdZEZEZEZdDEZEgZgZgggDMDRgMggDggMgRgRgMgMgRgMgRgRgRDgDgDgEDEgEgZgEDZgZgDggMggZDZDDDDgDgZgDgZDEZZggMgRQBBdvi:ir7rrirrriiiririrrri::::irs5DgEJsPgI7YZZZZZqdgRPgQPIPZZq5Yr:irri7:                 
DbgZgZDZgZDEDZgDgDgZgZggQgRMRgRgRRQDgDgZggMgMMRMRMQRQMQMRMQMMgMgRgMDDdZdEEZEZEDEDZgggEgZgZgZgZgDgZZbEEDDgEZdDZZZQBBDSL7ii:::::::iiiiiiiir7uIPDBBBPSXDEKJsLKRBgquSDgdMqIDRS2J7ri:7Y7i7si                 
EdEDZDEDEDZDDgDggRMRMMgRMRMRMQgQgRggdDZDZgDgDMgMgggMDMgggMgMgMgQRQQQRRgggRggDgZDdZEDEZZDZgZDEDDgDgZgggDgZDZZZZdDZDgQBBRgPPSqKPKq5S5PdgDDEMQBMdSKPZX1vsv1551PZQMSLPBBBEKPP5r::777vsirri..                
DPZdDEDDDZgZgDgggZgEgDMggggDgDMgRgRggDMgRMRMRgRgMgMgMgggMgggRgQMQMQRQQQRQQQRQRRgMDgDgEZZgZgggDgEDZgDMgDdZZDZZdZbZDgEZZQQQQBBBRBBBQBBBQBBBQQRREZZRMRMBQXvvr1gMbDQDJ77v1bd5r7vYu5vr:rrri::                
EdEZEDZDDgDgDgggDgEZZgggZgEDEgZgZgDgDgDggMgMggDgggggDgDgZgDggMggZDEgDggMMMMRgRgRgRgRggDMZgEgZgZgDZdDZDEDdddZEDEEdEbZdZdEbEPDgQggEDZdUvrjqMMgddKXUIUuIgQQPXSdgRdggQRU.vBgYvvYvYri:irv77i.               .
gdgZgZgDgDggMDgDgDggMgMDgDgDgZgZgDgggDMggDMDgDgDgDgDgDgDgDggMgMDgZgDgZgZgDMDggMgMgMgRRQgRDMZgZggQDDDgDgZDdDZRggZgDgDgZgEgZRDdqdDgq2YsJS5srv2gMgq55Xsr:i:7bQPgQBRQRQMMPY:isjv7rrivjIuJ7i.                



*/