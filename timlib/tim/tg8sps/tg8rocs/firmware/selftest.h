/*********************************************************************/
/* EXPORT: Tg8 hardware check program DPRAM layout.                  */
/* J.Lewis 7th Oct 94                                                */
/* V.Kovaltsov Oct 96                                                */
/*********************************************************************/

#ifndef Tg8SELF_TEST
#define Tg8SELF_TEST

/************************************************************/
/* Status values concerning DSC - 68332 transactions        */
/* NB Use crazey values to avoid starting tests by accident */
/************************************************************/

typedef enum {
   MbxST_GO   = 1234,      /* Go do the operation, set by DSC */
   MbxST_BUSY = 5678,      /* Operation is in progress, not yet complete */
   MbxST_DONE = 9012,      /* Operation completed OK, no error */
   MbxST_BUS_ERROR = 3456  /* Bus error while attempting operation */
 } MbxStatus;
 
/*****************************/
/* Driver mail box protocols */
/*****************************/

typedef enum {
   MbxOP_READ  = -1,
   MbxOP_WRITE = 1,
   MbxOP_NONE  = 0
 } MbxOperation;

/***************************************/
/* Mail box operation completion codes */
/***************************************/

typedef enum {
   MbxCC_BUSY             = -1,
   MbxCC_DONE             =  1,
   MbxCC_BOOT             = 0x3842,
   MbxCC_332Bug           = 0x3844,
   MbxCC_DOWNLOAD         = 0x3856,
   MbxCC_DOWNLOAD_PENDING = 0x4678,
   MbxCC_FIRMWARE_RUNNING = 0x7777 /* Signal to down loader */
 } MbxComplCode;

/********************************************************/
/* Layout the DPRAM and extensions using 16 bit shorts  */
/********************************************************/

#define Tg8DPRAM_SIZE 2048 /* 0x800 , in bytes, 2K */
#define StCOUNTERS    8    /* Number of counters */

/* Describe the memory range */

typedef struct {
   Ushort Bot;          /* Bottom (lowest) address */
   Ushort Top;          /* Top (highest) address */
 } StMemory;

/* Describe the fault properties */

typedef struct {
   int   Code;         /* Error bitmask */
   int   Templ;        /* Template data (written) */
   int   Actual;       /* Data was read */
   Ushort At;           /* The current (failed at) address */
   Ushort Dir;          /* Direction (LSB) & Access type (HSB) */
   Ushort BusInt;       /* The BUS interrupt conditions */
   Ushort N_of_bus;     /* The number of the BUS interrupts */
} StFault;

/**************************************************************************/
/* Date and time (NOTE: this structure has 'int' alignment type)          */
/**************************************************************************/

typedef struct { unsigned short Year,
				Month,
				Day,
				Hour,
				Minute,
				Second;
		 unsigned int   Ms;
} StDateTime;

/* Test execution Information section format */

typedef struct {
   int   N_of_bus;     /* Number of the BUS interrupts */
   int   N_of_frames;  /* Number of incoming frames */
   int   EventFrame;   /* The last received Event Frame */

   int   CycleNum;     /* Number of CPS cycles processed */
   short CycleDur;     /* The last CPS cycle duration */
   short CTrain;       /* C-Train clock value */
   StDateTime Date;    /* The current date & time */

   Ushort TestProg;     /* The test is in progress */
   Ushort TestPassed;   /* The bitmask of successfully passed tests */
   Ushort FaultType;    /* The bitmap of failed tests */
   Ushort FaultCnt;     /* Number of faults */
   struct { /* RAM test */
     StFault  Err;     /* Fault condition */
     StMemory Mem;     /* Addresses */
   } Ram;
   struct { /* DPRAM test */
     StFault  Err;     /* Fault condition */
     StMemory Mem;     /* Addresses */
   } Dpram;
   struct { /* CAM test */
     StFault  Err;     /* Fault condition */
     StMemory Mem;     /* Addresses */
     struct {
       Ushort At;        /* CAM match address */
       Ushort Cnt;       /* Number of matches */
     } Match;
   } Cam;
   struct { /* XILINX Receiver test */
     StFault  Err;         /* Fault condition */
     Ushort    Rcv;         /* The last XILINX Receiver error */
     Ushort    Ssc;         /* SSC code */
   } Xilinx;
   struct { /* COUNTERs test */
     StFault  Err[StCOUNTERS]; /* Fault conditions per counter */
     Ushort    Bad;         /* Bad counters mask */
     Ushort    Delay;       /* The last used counter delay */
   } Counter;
   int DscInt[1+StCOUNTERS]; /* Number of DSC interrupts */
   int MbxInt;               /* Number of MBX operations */

 } StDpmInfo;

/* Data used by the firmware downloading process */

typedef struct {
   short IoDirection;    /* R/W mode - not used at booting */
   short DoneFlag;       /* Booting completed */
   short FirmwareStatus; /* Booting protocol handshake */
   short VectorOffset;   /* Offset part of the exception */
   int   ExceptionPC;    /* PC value on exception */
   /* Note: long word alignment should be for compatibility with the Power PC!!! */

 } StDpmHead;

/* DPRAM layout used by the selftest program */

typedef struct StDpm {

   /* The first locations in DPRAM must stay the same, for compatability */
   /* with the boot firmware, and its standard interrupt handler. */
   StDpmHead Head;

   /* Test execution Information section */
   StDpmInfo Info;

   /* The rest of the DP memory is a buffer simply */
   short Buffer [Tg8DPRAM_SIZE/2 -2 -1 -(sizeof(StDpmHead)+sizeof(StDpmInfo))/2];

   /* XILINX XR Reception status (XrHardwareStatus) */
   short XilinxStatus;

   /* Any locations >=0x7FC have special assignment */

   short InterruptDsc; /* 7FC: DSC will read this word to clear an interrupt */
   short InterruptMcp; /* 7FE: DSC will write here to interrupt 68332 -- unused */

   /* These are hardware registers which extend the DPRAM. */

   short HardwareStatus;        /* 800: Hardware status word (Tg8HardwareStatus) */
   short Vector;                /* 802: Read  -> interrupt vector   */
				/* Write -> Enable/Disable/(Sync?)  */
   short CounterInterruptMask;  /* 804: Counter interrupts bits 0-7 */
                                /* DSC will write this word to clear interrupts */
 } StDpm;

/********************************/
/* BUS Interrupt sources        */
/********************************/

typedef enum {
   BI_INIT = 1,      /* BUS interrupt occurs during the MC hardware initializing */
   BI_RAM,           /* At writing onto the RAM location 'bus_int'*/
   BI_RAM_TEST,      /* During the RAM test */
   BI_DPRAM,         /* At DPRAM access */
   BI_DPRAM_TEST,    /* During the DPRAM test */
   BI_CAM,           /* At CAM access */
   BI_CAM_TEST,      /* During the CAM test */
   BI_XILINX,        /* At XILINX access (read err & set SSC code) */
   BI_XILINX_TEST,   /* During the XILINX test */
   XI_READF1,        /* Read Frame 1 */
   XI_READF2,        /* Read Frame 2 */
   XI_READERR,       /* Read the Receiver Error */
   XI_RESETERR,      /* Reset the Receiver Error */
   XI_SSC,           /* Set the SSC code */
   BI_COUNTER_TEST,  /* During the COUNTER test */
   CI_DELAY,         /* Read/Write the Delay register */
   CI_CONFIG,        /* Write the Configuration register */
   N_OF_BI
} StBusInt;
 
/********************************/
/* Memory scan direction        */
/********************************/

typedef enum {
   D_FORWARD = 0, /* Forward direction */
   D_BACKWARD,    /* Backward direction */
   N_OF_DIR
} StDirection;

/********************************/
/* Memory access type           */
/********************************/

typedef enum {
   A_BYTE = 0, /* Memory Byte access */
   A_WORD,     /* Memory Word access */
   A_INT32,    /* Memory Int  access */
   N_OF_ACC
} StAccess;

/********************************/
/* Memory test patterns         */
/********************************/

typedef enum {
   TM_AA,      /* Write AA value  */
   TM_55,      /* Write 55 ...  */
   TM_00,      /* Write 00 value  */
   N_OF_TEMPL
} StTemplate;

/********************************/
/* CAM Test patterns            */
/********************************/

typedef enum {
   TC_A = 1,    /* Write: tem,~tem,tem^inv, where inv==0x5A5A */
   TC_B,        /* Write: ~tem,tem^inv,tem */
   TC_C,        /* Write: ~tem,tem,tem^inv */
   TC_D,        /* Write: tem^inv,tem,~tem */
   TC_E         /* Matching:  tem,tem^inv,~tem */
} StCamTemplate;

/********************************/
/* Counter counting faulty      */
/********************************/

typedef enum {
   CO_COUNT_FAST=1,  /* The counting is too fast */
   CO_BLOCKED,       /* The counting is blocked -- no interrupts from it */
   CO_NO_DSC_INT,    /* No interrupts on the DSC site */
   N_OF_CONT_ER
} StCounting;

/********************************/
/* The kind of tests            */
/********************************/

typedef enum {
   T_RAM   = 0x0001, /* RAM test in progress/passed */
   T_DPRAM = 0x0002, /* DPRAM test */
   T_CAM   = 0x0004, /* CAM test */
   T_XILINX= 0x0008, /* XILINX Receiver test */
   T_MS    = 0x0010,  /* TPU #0 -- ms clock */
   T_COUNTERS= 0x0020 /* TPU #1-8 -- counters */
} StTest;

/********************************/
/* Error codes                  */
/********************************/

typedef enum {
   E_CAM_RW = 1,
   E_CAM_MATCH
} StErrCode;

#endif

/* eof selftest.h */
