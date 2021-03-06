/***************************************************************************/
/* Include file to provide the PRIVATE Tg8 device driver tables.           */
/* Vladimir Kovaltsov for the SL Version, February, 1997		   */
/***************************************************************************/

/* Dont define the module more than once */

#ifndef TG8DRVR_P
#define TG8DRVR_P

#ifdef CETIA_PPC
#define disable(x)
#define restore(x)
#endif

#define CONVERTOR 0

#include <tg8drvr.h>    /* Tg8 Driver public definitions */
#include <tg8.h>	/* Tg8 Firmware definitions */

#define InterruptVME(base)  (*(short*)(Tg8HR_INT_VME+(char*)base))=1 /* MPC --> VME */
#define ReleaseVME(base)    {short i=(*(short*)(Tg8HR_INT_VME+(char*)base));}
#define ReleaseCntInt(base,msk) (*(short *)(Tg8HR_CLEAR_INT+(char*)base))=msk

#define ResetMPC(base)      (*(short *)(Tg8HR_STATUS+(char*)base))=1
#define ReadStatusMPC(base) (*(short*)(Tg8HR_STATUS+(char*)base))
#define ReadVectorMPC(base) (*(short *)(Tg8HR_INTERRUPT+(char*)base))

#define EnableMtgInt(base)  (*(short *)(Tg8HR_INTERRUPT+(char*)base))=Tg8ENABLE_INT
#define DisableMtgInt(base) (*(short *)(Tg8HR_INTERRUPT+(char*)base))=Tg8DISABLE_INT

#define EnableMtgIntSync(base)  (*(short *)(Tg8HR_INTERRUPT+(char*)base))=Tg8ENABLE_SYNC_INT
#define DisableMtgIntSync(base) (*(short *)(Tg8HR_INTERRUPT+(char*)base))=Tg8DISABLE_SYNC_INT

#define InterruptMPC(base)  (*(short*)(Tg8HR_INT_MPC+(char*)base))=1 /* VME --> MPC; Not used */
#define ReleaseMPC(base)    {short i=(*(short*)(Tg8HR_INT_MPC+(char*)base));}

#define STRUCPY(d,s)         memcpy16((short*)(d),(short*)(s),sizeof(*(s)))
#define LW32(d,s)            memcpy16((short*)&(d),(short*)&(s),sizeof(int))
#define LW(d,s) \
              {((short*)&(d))[0]=((short*)&(s))[0];((short*)&(d))[1]=((short*)&(s))[1];}

/***************************************************************************/
/* The "info" table passed to the install procedure at startup time.       */
/***************************************************************************/

typedef struct {
  Uint      Address;		 /* Base address (0xDEC00000 usually) */
  Uint      Increment;           /* Address increment (0x10000 usually) */
  Uint      Vector;	         /* The first interrupt vector number (0xB8 usually) */
  Uint      Level;	         /* Interrupt level (2 usually) */
  Uint      SscHeader;           /* The default SSC Header code (0x20:SL, 0x34:PS) */
} Tg8DrvrInfoTable;

/*****************************/
/* The module context.       */
/*****************************/

typedef struct {
  Tg8Dpm  * VMEAddress;		 /* Base address */
  Uint      InterruptVector;	 /* Interrupt vector number */
  Uint      InterruptLevel;	 /* Interrupt level (2 usually) */
  Uint      SwitchSettings;	 /* Switch settings */
  Uint      SscHeader;           /* SSC Header code (it sets up the acc. machine) */
} Tg8DrvrModuleAddress;

/***************************************************************************/
/* Device context. It describes any device related data.                   */
/***************************************************************************/

typedef struct {
  int  Mode;          /* Device access mode */
  Word ModuleIndex,   /* Module index (0-based) */
       DeviceIndex;   /* Device index */
  int Timer;          /* Timeout timer id number */
  int AppSemaphore;   /* Semaphore to wake up waiting process */
  int Tail;           /* Index in the incoming events queue (to read on it) */
  int Pid;            /* Owner's pid */
  int Signal;         /* Signal to be delivered to process pid (0=not used) */
  Tg8Event Filter;    /* Filter event code */
  Tg8DrvrOnClose CloseMode; /* The device close mode */
} Tg8DrvrDeviceContext;

/***************************************************************************/
/* Action context. It describes any action related data.                   */
/***************************************************************************/

typedef struct {
  int      Size;                /* The number of used actions (incl. holes) */
  int      Id     [Tg8ACTIONS]; /* Object's identifier */
  int      DevMask[Tg8ACTIONS]; /* Devices used by an action */
  Tg8User  Table  [Tg8ACTIONS]; /* The array of actions */
  Tg8Gate  Gate   [Tg8ACTIONS]; /* PLS condition per action */
  Byte     Dim    [Tg8ACTIONS]; /* Timing unit dimension */
} Tg8DrvrUserTable;

/***************************************************************************/
/* The event queue for a module contains unread events.                    */
/* Events are put on the Head of the queue and taken off the Tail (FiFo).  */
/***************************************************************************/

#define Tg8DrvrQUEUE_SIZE 128	/* Queue capacity */

typedef struct {int	     Head; /* The 1st free entry */
		Tg8DrvrEvent Queue [Tg8DrvrQUEUE_SIZE];
} Tg8DrvrQueue;

/***************************************************************************/
/* Module context.                                                         */
/***************************************************************************/

/* The module context is passed to the ISR as its parameter so that it */
/* knows which module is interrupting and its VME address etc. The ISR */
/* also needs the pointer to the drivers working area.                 */

typedef struct Tg8DrvrModuleContext {
  int      ModuleIndex;	      /* Zero based module number */
  int      Timer;             /* Timeout timer id number */
  int      TimeOut;	      /* Time out value for read/wait_events operation */
  int      BusySemaphore;     /* Transaction is in progress, wait for dpram interr. */
  int      TransferSemaphore; /* Protect the DPRAM access */
  int	   SleepSemaphore;    /* Sleep during the firmware booting */
  Uint     DevMask;           /* Bitmap of opened devices for the given module */
  Word     Status;	      /* The module's current status */
  Word     Alarms;            /* Driver's generated alarms (see Tg8Alarm enum) */
  int      ErrCnt;            /* Total number of errors erased */
  int      ErrCode;           /* The last error code */
  char     ErrMsg[128];       /* The last error message text */
  StDpmInfo SelfTestRes;      /* The SelfTest execution result */
  Tg8DrvrUserTable       UserTable;        /* User table */
  Tg8DrvrModuleAddress   ModuleAddress;	   /* Module's address */
  Tg8DrvrDeviceContext   Device[Tg8DrvrDEVICES]; /* All devices for the module */
  Tg8DrvrQueue           Queue;	           /* Received Events queue */
  struct Tg8DrvrWorkingArea * WorkingArea; /* Driver working area address */
} Tg8DrvrModuleContext;

/* We need one module context per module. */

typedef Tg8DrvrModuleContext * Tg8DrvrModuleContexts[Tg8DrvrMODULES];

/***************************************************************************/
/* Driver working area                                                     */
/***************************************************************************/

#define Tg8DrvrFIRMWARE_TIMEOUT 100 /* 1 second */

typedef struct Tg8DrvrWorkingArea {
   Uint                     FirmwareTimeout; /* Timeout for mailbox protocol */
   Uint                     DebugOn;	     /* Switch the debug mode */
   Tg8DrvrInfoTable         InfoTable;       /* Keep the driver's info table */
   Tg8DrvrModuleContexts    ModuleContexts;  /* All Tg8 modules data */
} Tg8DrvrWorkingArea;

#endif

/* eof Tg8DrvrP.h */
