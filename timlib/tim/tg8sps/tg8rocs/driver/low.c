/*========================================================================*/
/* Local procedures and variables used by the Tg8 driver program.	  */
/* Major revision J.Lewis Nov 24 1993                                     */
/* Vladimir Kovaltsov for the SL Version, February, 1997		  */
/*========================================================================*/

/*========================================================================*/
/* Reset the module							  */
/*========================================================================*/

void ResetModule(module)
Tg8DrvrModuleContext * module; {

Tg8Dpm * dpram; int ps,timet;
  dpram = module->ModuleAddress.VMEAddress;

  ReleaseVME(dpram); /* Clear the DSC interrupt */
  ResetMPC(dpram);   /* Issue the TG8 reset signal */

  /* Reset mail box protocol semaphores */

  disable(ps);
  sreset (&module->BusySemaphore);
  sreset (&module->TransferSemaphore);
  ssignal(&module->TransferSemaphore);
  restore(ps);

  /* Wait for the selftest be completed */
  sreset(&module->SleepSemaphore);
  timet = timeout(ssignal,(char *) &module->SleepSemaphore,400); /* wait 4 sec */
  swait(&module->SleepSemaphore,SEM_SIGIGNORE);
}

/*========================================================================*/
/* Disable the modules							  */
/*========================================================================*/

void DisableModule(module)
Tg8DrvrModuleContext *module; {

Tg8Dpm *dpram = module->ModuleAddress.VMEAddress;
   dpram->At.aModStatus = (module->Status &= ~Tg8DrvrMODULE_ENABLED);
   DisableMtgInt(dpram);
}

/*========================================================================*/
/* Disable the modules synchronously with the SSC frame			  */
/*========================================================================*/

void DisableSync(module)
Tg8DrvrModuleContext *module; {

Tg8Dpm *dpram = module->ModuleAddress.VMEAddress;
   dpram->At.aModStatus = (module->Status &= ~Tg8DrvrMODULE_ENABLED);
   DisableMtgIntSync(dpram);
}

/*========================================================================*/
/* Enable the modules							  */
/*========================================================================*/

void EnableModule(module)
Tg8DrvrModuleContext *module; {

Tg8Dpm *dpram = module->ModuleAddress.VMEAddress;
   dpram->At.aModStatus = (module->Status |= Tg8DrvrMODULE_ENABLED);
   EnableMtgInt(dpram);
}

/*========================================================================*/
/* Enable the modules synchronously with the SSC frame			  */
/*========================================================================*/

void EnableSync(module)
Tg8DrvrModuleContext *module; {

Tg8Dpm *dpram = module->ModuleAddress.VMEAddress;
   dpram->At.aModStatus = (module->Status |= Tg8DrvrMODULE_ENABLED);
   EnableMtgIntSync(dpram);
}

/*========================================================================*/
/* Trace the firmware in the modules					  */
/*========================================================================*/

void TraceFirmware(module,buf)
Tg8DrvrModuleContext *module;
int *buf; {

Tg8Dpm * dpram = module->ModuleAddress.VMEAddress;
   *buf = dpram->At.aTrace;
#if 0 /* debug */
   ReleaseVME(dpram);	       /* Clear the DPRAM interrupt flip-flop */
   ReleaseCntInt(dpram,0xff);  /* Clear all counter interrupts */
   dpram->At.aIntSrc = 0;
   dpram->At.aAlarms = 0;
#endif
}

/*========================================================================*/
/* Read the DPRAM content						  */
/*========================================================================*/

void GetDpram(module,buf)
Tg8DrvrModuleContext *module;
Tg8Dpram *buf; {

Tg8Dpm * dpram = module->ModuleAddress.VMEAddress;
Word sem = dpram->At.aSem;
  if (recoset())
    PrError(module,Tg8ERR_BUS_ERR);
  else
    memcpy16((short*)buf,(short*)dpram,0x7FC/*sizeof(Tg8Dpram)*/);
  noreco();
#if 0
  buf->At.aSem -= sem;
  buf->Hw = ReadStatusMPC(dpram);
#endif
}

/*========================================================================*/
/* Read the interrupt vector						  */
/*========================================================================*/

void ReadInterruptVector(module, vector)
Tg8DrvrModuleContext *module; int *vector; {

Tg8Dpm *dpram = module->ModuleAddress.VMEAddress;
   *vector = ReadVectorMPC(dpram) & 0xFF;
}

/*========================================================================*/
/* Read the firmware status of Tg8 module                                 */
/*========================================================================*/

void ReadRawStatus(module,s)
Tg8DrvrModuleContext *module;
Tg8StatusBlock       *s; {

Tg8Dpm * dpram = module->ModuleAddress.VMEAddress;
   s->Hw = ReadStatusMPC(dpram);   /* Hardware status */
   s->Fw = dpram->At.aFwStat;      /* Firmware status */
   s->Cc = dpram->At.aCoco;        /* Completion code */
   s->Am = dpram->At.aAlarms;      /* Alarms bitmask */
   s->Evo= dpram->ExceptionVector; /* Exception Vector */
   LW(s->Epc,dpram->ExceptionPC);  /* Exception PC */
   LW(s->ScTime,dpram->At.aScTime);  /* Time in the S-Cycle */
   STRUCPY(&s->Dt,&dpram->At.aDt);   /* RcvErr, date, time */
}

/*========================================================================*/
/* Read the most important information about of the Super Cycle.          */
/*========================================================================*/

void ReadScInfo(module,s)
Tg8DrvrModuleContext *module;
Tg8SuperCycleInfo    *s; {

unsigned long scnum, b1, b2;

Tg8Dpm * dpram = module->ModuleAddress.VMEAddress;

   LW(s->ScNumber ,dpram->At.aSc);        /* The Super Cycle number */

   /* Julian 26/07/2005 Super-cycle number mod */

   scnum = s->ScNumber;
   scnum = (0x00FFFF00 & scnum) >> 8; /* Throw away msb */
   b1 = scnum & 0xFF;
   b2 = (scnum >> 8) & 0xFF;
   scnum = (b1 << 8) | b2;            /* Swap bytes */
   s->ScNumber = scnum;

   LW(s->ScLength ,dpram->At.aScLen);     /* The last completed S-Cycle length */
   LW(s->ScTime   ,dpram->At.aScTime);    /* The current value of the S-Cycle time */
   LW(s->ScCounter,dpram->At.aNumOfSc);   /* Number of SC received since the last Reset */
}

/*========================================================================*/
/* Read the date and time from the Tg8 module   			  */
/*========================================================================*/

void ReadDateTime(module,buf)
Tg8DrvrModuleContext *module;
Tg8DateTime          *buf; {

Tg8Dpm * dpram = module->ModuleAddress.VMEAddress;
   STRUCPY(buf,&dpram->At.aDt);
}

/*========================================================================*/
/* Returns a 16-character string containing the date of the firmware      */
/* compilation                                                            */
/*========================================================================*/

void ReadCreationDate(module, str)
Tg8DrvrModuleContext *module;
char *str; {

Tg8Dpm *dpram = module->ModuleAddress.VMEAddress;
   memcpy16((short*)str,(short*)dpram->At.aFwVer,sizeof(dpram->At.aFwVer));
}

/*========================================================================*/
/* Read the telegram for given machine                   		  */
/*========================================================================*/

int ReadTelegram(module,machine,buf)
Tg8DrvrModuleContext *module;
int machine; Word *buf; {

Tg8Dpm * dpram = module->ModuleAddress.VMEAddress;
Word *tel;
  switch(machine) {
  case Tg8LEP: tel = dpram->At.aTelegLEP; break;
  case Tg8SPS: tel = dpram->At.aTelegSPS; break;
  case Tg8CPS: tel = dpram->At.aTelegCPS; break;
  case Tg8PSB: tel = dpram->At.aTelegPSB; break;
  case Tg8LPI: tel = dpram->At.aTelegLPI; break;
  default: return Tg8ERR_ILLEGAL_ARG;
  };
  memcpy16((short*)buf,(short*)tel,sizeof(dpram->At.aTelegLEP));
  return Tg8ERR_OK;
}

/*========================================================================*/
/* Ping the module					                  */
/*========================================================================*/

int PingModule(module)
Tg8DrvrModuleContext *module; {
int st;
   /* Wait for the completness of the previous DPM transaction */
   swait(&module->TransferSemaphore,SEM_SIGIGNORE);

   if ((st=WaitForMailbox(module,Tg8OP_PING_MODULE)) <0 ) return(st);
   ssignal(&module->TransferSemaphore);
   return(Tg8ERR_OK);
}

/*========================================================================*/
/* Read the selftest result 				                  */
/*========================================================================*/

int ReadSelfTestResult(module)
Tg8DrvrModuleContext *module; {

Tg8Dpm * dpram = module->ModuleAddress.VMEAddress; int st;
   /* Wait for the completness of the previous DPM transaction */
   swait(&module->TransferSemaphore,SEM_SIGIGNORE);

   if ((st=WaitForMailbox(module,Tg8OP_SELFTEST_RESULT)) <0 ) return(st);
   STRUCPY(&module->SelfTestRes,&dpram->BlockData.SelfTestResult);
   ssignal(&module->TransferSemaphore);
   return(Tg8ERR_OK);
}

/*========================================================================*/
/* Simulate a pulse					                  */
/*========================================================================*/

int SimulatePulse(module,msk)
Tg8DrvrModuleContext *module; int msk; {
Tg8Dpm * dpram = module->ModuleAddress.VMEAddress; int st;
   /* Wait for the completness of the previous DPM transaction */
   swait(&module->TransferSemaphore,SEM_SIGIGNORE);

   dpram->BlockData.SimPulse.Mask = msk;
   if ((st=WaitForMailbox(module,Tg8OP_SIMULATE_PULSE)) <0 ) return(st);
   ssignal(&module->TransferSemaphore);
   return(Tg8ERR_OK);
}

/*========================================================================*/
/* Read the current state of modules (use mbx protocol)			  */
/*========================================================================*/

int GetStatus(module,buf)
Tg8DrvrModuleContext *module;
Tg8StatusBlock *buf; {

Tg8Dpm * dpram = module->ModuleAddress.VMEAddress; int st;
   /* Wait for the completness of the previous DPM transaction */
   swait(&module->TransferSemaphore,SEM_SIGIGNORE);

   if ((st=WaitForMailbox(module,Tg8OP_GET_STATUS)) <0 ) return(st);
   STRUCPY(buf,&dpram->BlockData.StatusBlock);
   buf->Hw = ReadStatusMPC(dpram);
   module->Alarms |= buf->Am;
   ssignal(&module->TransferSemaphore);
   return(Tg8ERR_OK);
}

/*========================================================================*/
/* Set the module ID (e.g. machine number)                                */
/*========================================================================*/

int SetSscHeader(module,ssch)
Tg8DrvrModuleContext *module; int ssch; {

Tg8Dpm * dpram = module->ModuleAddress.VMEAddress; int st;
   /* Wait for the completness of the previous DPM transaction */
   swait(&module->TransferSemaphore,SEM_SIGIGNORE);

   dpram->At.aSscHeader = ssch;
   if ((st=WaitForMailbox(module,Tg8OP_SET_SSC_HEADER)) <0 ) return(st);
   ssignal(&module->TransferSemaphore);
   return(Tg8ERR_OK);
}

/*========================================================================*/
/* Clear action(s)             						  */
/*========================================================================*/

int ClearAction(module,row,cnt)
Tg8DrvrModuleContext *module;
int row,cnt; {

int st;
Tg8Dpm *dpram = module->ModuleAddress.VMEAddress;
Tg8MbxClearAction *c = &dpram->BlockData.ClearAction;
   /* Wait for the completness of the previous DPM transaction */
   swait(&module->TransferSemaphore,SEM_SIGIGNORE);

   c->Hdr.Row = row;
   c->Hdr.Number = cnt;
   if ((st=WaitForMailbox(module,Tg8OP_CLEAR_USER_TABLE)) <0 ) return(st);
   ssignal(&module->TransferSemaphore);
   return (st);
}

/*========================================================================*/
/* Enable/Disable action(s)						  */
/*========================================================================*/

int SetActionState(module,row,cnt,state)
Tg8DrvrModuleContext *module;
int row,cnt,state; {

int st;
Tg8Dpm *dpram = module->ModuleAddress.VMEAddress;
Tg8MbxActionState *s = &dpram->BlockData.ActionState;
   /* Wait for the completness of the previous DPM transaction */
   swait(&module->TransferSemaphore,SEM_SIGIGNORE);

   s->State = state;
   s->Hdr.Row = row;
   s->Hdr.Number = cnt;
   if ((st=WaitForMailbox(module,Tg8OP_SET_STATE)) <0 ) return(st);
   ssignal(&module->TransferSemaphore);
   return (st);
}

/*========================================================================*/
/* Set delay value for the set of action(s)				  */
/*========================================================================*/

int SetActionDelay(module,row,cnt,delay,clk)
Tg8DrvrModuleContext *module;
int row,cnt,delay; {

int st;
Tg8Dpm *dpram = module->ModuleAddress.VMEAddress;
Tg8MbxActionState *s = &dpram->BlockData.ActionState;
   /* Wait for the completness of the previous DPM transaction */
   swait(&module->TransferSemaphore,SEM_SIGIGNORE);

   s->State = delay;   /* Delay value */
   s->Aux = clk;       /* Clock type */
   s->Hdr.Row = row;
   s->Hdr.Number = cnt;
   if ((st=WaitForMailbox(module,Tg8OP_SET_DELAY)) <0 ) return(st);
   ssignal(&module->TransferSemaphore);
   return (st);
}

/*========================================================================*/
/* Set all action's parameters						  */
/*========================================================================*/

int SetAction(module,row,cnt,act)
Tg8DrvrModuleContext *module;
int row,cnt;
Tg8User *act; {

int n,rows,st;
Tg8Dpm *dpram = module->ModuleAddress.VMEAddress;
Tg8MbxRwAction *a = &dpram->BlockData.RwAction;
   /* Wait for the completness of the previous DPM transaction */
   swait(&module->TransferSemaphore,SEM_SIGIGNORE);

   rows = Tg8MBX_BUFF_SIZE/sizeof(Tg8User);
   a->Hdr.Row = row;
   while (cnt>0) {
     a->Hdr.Number = n = (cnt>rows? rows: cnt);
     memcpy16(a->Actions,act,n*sizeof(Tg8User));
     if ((st=WaitForMailbox(module,Tg8OP_SET_USER_TABLE)) <0 ) return(st);
     act += n;
     cnt -= n;
   };
   ssignal(&module->TransferSemaphore);
   return (st);
}

/*========================================================================*/
/* Set PPM action's parameters						  */
/*========================================================================*/

int SetPpmAction(module,row,cnt,act)
Tg8DrvrModuleContext *module;
int row,cnt;
Tg8PpmUser *act; {

int n,rows,st;
Tg8Dpm *dpram = module->ModuleAddress.VMEAddress;
Tg8MbxRwPpmAction *a = &dpram->BlockData.RwPpmAction;
   /* Wait for the completness of the previous DPM transaction */
   swait(&module->TransferSemaphore,SEM_SIGIGNORE);

   rows = Tg8MBX_BUFF_SIZE/sizeof(Tg8PpmUser);
   a->Hdr.Row = row;
   while (cnt>0) {
     a->Hdr.Number = n = (cnt>rows? rows: cnt);
     memcpy16(a->Actions,act,n*sizeof(Tg8PpmUser));
     if ((st=WaitForMailbox(module,Tg8OP_SET_PPM_TIMING)) <0 ) return(st);
     act += n;
     cnt -= n;
   };
   ssignal(&module->TransferSemaphore);
   return (st);
}

/*========================================================================*/
/* Set PPM line's gate (PLS condition)					  */
/*========================================================================*/

int SetGate(module,row,cnt,act)
Tg8DrvrModuleContext *module;
int row,cnt;
Tg8Gate *act; {

int n,rows,st;
Tg8Dpm *dpram = module->ModuleAddress.VMEAddress;
Tg8MbxRwGate *a = &dpram->BlockData.RwGate;
   /* Wait for the completness of the previous DPM transaction */
   swait(&module->TransferSemaphore,SEM_SIGIGNORE);

   rows = Tg8MBX_BUFF_SIZE/sizeof(Tg8Gate);
   a->Hdr.Row = row;
   while (cnt>0) {
     a->Hdr.Number = n = (cnt>rows? rows: cnt);
     memcpy16(a->Gates,act,n*sizeof(Tg8Gate));
     if ((st=WaitForMailbox(module,Tg8OP_SET_GATE)) <0 ) return(st);
     act += n;
     cnt -= n;
   };
   ssignal(&module->TransferSemaphore);
   return (st);
}

/*========================================================================*/
/* Set PPM timing's dimension					          */
/*========================================================================*/

int SetDim(module,row,cnt,act)
Tg8DrvrModuleContext *module;
int row,cnt;
Byte *act; {

int n,rows,st;
Tg8Dpm *dpram = module->ModuleAddress.VMEAddress;
Tg8MbxRwDim *a = &dpram->BlockData.RwDim;
   /* Wait for the completness of the previous DPM transaction */
   swait(&module->TransferSemaphore,SEM_SIGIGNORE);

   rows = Tg8MBX_BUFF_SIZE/sizeof(Byte);
   a->Hdr.Row = row;
   while (cnt>0) {
     a->Hdr.Number = n = (cnt>rows? rows: cnt);
     memcpy16(a->Dims,act,n*sizeof(Byte));
     if ((st=WaitForMailbox(module,Tg8OP_SET_DIM)) <0 ) return(st);
     act += n;
     cnt -= n;
   };
   ssignal(&module->TransferSemaphore);
   return (st);
}

/*========================================================================*/
/* Get PPM info from the user table       				  */
/*========================================================================*/

int GetPpmLine(module,act,row,cnt)
Tg8DrvrModuleContext *module;
Tg8PpmLine *act; int row,cnt; {

int n,rows,st;
Tg8Dpm *dpram = module->ModuleAddress.VMEAddress;
Tg8MbxRwPpmLine *a = &dpram->BlockData.RwPpmLine;
   /* Wait for the completness of the previous DPM transaction */
   swait(&module->TransferSemaphore,SEM_SIGIGNORE);

   rows = Tg8MBX_BUFF_SIZE/sizeof(Tg8PpmLine);
   a->Hdr.Row = row;
   while (cnt>0) {
     a->Hdr.Number = n = (cnt>rows? rows: cnt);
     if ((st=WaitForMailbox(module,Tg8OP_GET_PPM_LINE)) <0 ) return(st);
     memcpy16(act,a->Lines,n*sizeof(Tg8PpmLine));
     act += n;
     cnt -= n;
   };
   ssignal(&module->TransferSemaphore);
   return (st);
}

/*========================================================================*/
/* Get user table       						  */
/*========================================================================*/

int GetAction(module,act,row,cnt)
Tg8DrvrModuleContext *module;
Tg8User *act; int row,cnt; {

int n,rows,st;
Tg8Dpm *dpram = module->ModuleAddress.VMEAddress;
Tg8MbxRwAction *a = &dpram->BlockData.RwAction;
   /* Wait for the completness of the previous DPM transaction */
   swait(&module->TransferSemaphore,SEM_SIGIGNORE);

   rows = Tg8MBX_BUFF_SIZE/sizeof(Tg8User);
   a->Hdr.Row = row;
   while (cnt>0) {
     a->Hdr.Number = n = (cnt>rows? rows: cnt);
     if ((st=WaitForMailbox(module,Tg8OP_GET_USER_TABLE)) <0 ) return(st);
     memcpy16(act,a->Actions,n*sizeof(Tg8User));
     act += n;
     cnt -= n;
   };
   ssignal(&module->TransferSemaphore);
   return (st);
}

/*========================================================================*/
/* Read the recording table						  */
/*========================================================================*/

int GetRecording(module,rbuf,row,cnt)
Tg8DrvrModuleContext *module;
Tg8Recording *rbuf; int row,cnt; {

int n,rows,st;
Tg8Dpm *dpram = module->ModuleAddress.VMEAddress;
Tg8MbxRecording *r = &dpram->BlockData.Recording;
   /* Wait for the completness of the previous DPM transaction */
   swait(&module->TransferSemaphore,SEM_SIGIGNORE);

   rows = Tg8MBX_BUFF_SIZE/sizeof(Tg8Recording);
   r->Hdr.Row = row;
   while (cnt>0) {
     r->Hdr.Number = n = (cnt>rows? rows: cnt);
     if ((st=WaitForMailbox(module,Tg8OP_GET_RECORDING_TABLE)) <0 ) return(st);
     memcpy16(rbuf,r->Recs,n*sizeof(Tg8Recording));
     rbuf += n;
     cnt -= n;
   };
   ssignal(&module->TransferSemaphore);
   return (st);
}

/*========================================================================*/
/* Read interrupts table       						  */
/*========================================================================*/

void GetInterruptTable(module,buf)
Tg8DrvrModuleContext *module;
Tg8InterruptTable *buf; {

Tg8Dpm * dpram = module->ModuleAddress.VMEAddress;
   STRUCPY(buf,&dpram->It);
}

/*========================================================================*/
/* Read the events history table       					  */
/*========================================================================*/

int GetEventHistory(module,hbuf,cnt)
Tg8DrvrModuleContext *module;
Tg8History *hbuf; int cnt; {

int n,rows,st;
Tg8Dpm *dpram = module->ModuleAddress.VMEAddress;
Tg8MbxHistory *h = &dpram->BlockData.HistoryBlock;
   /* Wait for the completness of the previous DPM transaction */
   swait(&module->TransferSemaphore,SEM_SIGIGNORE);

   rows = Tg8MBX_BUFF_SIZE/sizeof(Tg8History); /* Entries per 1 transaction */
   h->Hdr.Row = 0; /* Start from the newest record */
   while (cnt>0) {
     h->Hdr.Number = n = (cnt>rows? rows: cnt);
     if ((st=WaitForMailbox(module,Tg8OP_GET_HISTORY_TABLE)) <0 ) return(st);
     memcpy16(hbuf,h->Hist,n*sizeof(Tg8History));
     hbuf += n;
     cnt -= n;
   };
   ssignal(&module->TransferSemaphore);
   return (st);
}

/*========================================================================*/
/* Read the clocks table       					          */
/*========================================================================*/

int GetClock(module,cbuf,cnt)
Tg8DrvrModuleContext *module;
Tg8Clock *cbuf; int cnt; {

int n,rows,st;
Tg8Dpm *dpram = module->ModuleAddress.VMEAddress;
Tg8MbxClock *c = &dpram->BlockData.ClockBlock;
   /* Wait for the completness of the previous DPM transaction */
   swait(&module->TransferSemaphore,SEM_SIGIGNORE);

   rows = Tg8MBX_BUFF_SIZE/sizeof(Tg8Clock); /* Entries per 1 transaction */
   c->Hdr.Row = 0; /* Start from the newest record */
   while (cnt>0) {
     c->Hdr.Number = n = (cnt>rows? rows: cnt);
     if ((st=WaitForMailbox(module,Tg8OP_GET_CLOCK_TABLE)) <0 ) return(st);
     memcpy16(cbuf,c->Clks,n*sizeof(Tg8Clock));
     cbuf += n;
     cnt -= n;
   };
   ssignal(&module->TransferSemaphore);
   return (st);
}

/*========================================================================*/
/* Read the auxiliary table       					  */
/*========================================================================*/

void GetAuxTable(module,buf)
Tg8DrvrModuleContext *module;
Tg8Aux *buf; {

Tg8Dpm * dpram = module->ModuleAddress.VMEAddress;
Word sem = dpram->At.aSem;
   STRUCPY(buf,&dpram->At);
   buf->aSem -= sem;
}

/*=========================================================================*/
/* Down load the firmware through the Dpram                                */
/*=========================================================================*/

int DownLoadFirmware(module,op)
Tg8DrvrModuleContext  *module;
Tg8FirmwareObject *op; {

  Tg8Dpm *dpram = module->ModuleAddress.VMEAddress;
  Tg8LoadDpm *dpl = (Tg8LoadDpm *) dpram;
  long p; int l,l1,i,j,timet; short st;

  /* Specify the "download request" and make the RESET. */
  /* Try 5 times. */

  for (i=0; i<5; i++) {
    dpl->Firmware_status = Tg8DOWNLOAD;
    ResetModule(module);
    if (dpl->Firmware_status == Tg8BOOT) goto dwld;
  };
  return PrError(module,Tg8ERR_TIMEOUT); /* Download failed */
  
dwld:
  /* Save the selftest result */
  STRUCPY(&module->SelfTestRes,&((StDpm*)dpram)->Info);

  l = op->Size;
  p = op->StartAddress;
  if (l == 0) return PrError(module,Tg8ERR_NOT_RUN);

  dpl->Firmware_status = Tg8DOWNLOAD_PENDING;
  i = 0;
  while (l>0) {
    dpl->Loader_done_flag = 0;
    l1 = Tg8LBUF_SIZE;
    if (l<l1) l1 = l;
    dpl->Address[0] = 0; /* Most significant word */
    dpl->Address[1] = p; /* Least --"-- */
    dpl->Length = l1*2;
    memcpy16(dpl->LdBuffer,op->Object+i,dpl->Length);
    dpl->Loader_done_flag = Tg8DWNLD_FLAG_READY;
    timet = timeout(ssignal,(char *) &module->SleepSemaphore,10); /* wait 0.1 sec */
    swait(&module->SleepSemaphore,SEM_SIGIGNORE);
    if (dpl->Loader_done_flag != Tg8DWNLD_FLAG_DONE) {
      return PrError(module,Tg8ERR_NO_ACK); /* Download failed */
    };
    i += l1;
    p += l1*2;
    l -= l1;
  };
  
  LW(dpl->Address[0],op->StartAddress);
  dpl->Length = 0;
  dpl->Loader_done_flag = Tg8DWNLD_FLAG_READY;
  for(i=0; i<5; i++) {
    timet = timeout(ssignal,(char *) &module->SleepSemaphore,100); /* wait for 1 sec */
    swait(&module->SleepSemaphore,SEM_SIGIGNORE);
    if (dpram->At.aFwStat & Tg8FS_RUNNING)
      return Tg8ERR_OK; /* Download succeed */
  };
  return PrError(module,Tg8ERR_NOT_RUN); /* Download failed */
}

/*================*/
/* Test the DPRAM */
/*================*/

void TestTheDpram(module,res)
Tg8DrvrModuleContext *module;
Tg8TestDpram *res; {

Tg8Dpm *dpram = module->ModuleAddress.VMEAddress;
union {
  unsigned char  *b;
  unsigned short *w;
  unsigned int   *l;
} m;
int i,ln,fr,to,dir; Word tem,wrd; unsigned char *dpmb;

  dpmb = (unsigned char*) dpram;
  bzero((void *) res,sizeof(*res));

  /* Read the whole DPRAM */

  dir = 2; /* Read */
  if (recoset())
    res->Status = 0x2000; /* BUS error occured during the read */
  else
    for (i=0; i<0x7FC; i+=2) {
      res->Addr = i;
      m.b = dpmb + i;
      wrd = *m.w;
      tem = wrd;
    };
  noreco();
  if (res->Status) goto faila;

  for (dir=0; dir<2; dir++) { /* Direction: 0=Forward; 1=Backward */
      ln = sizeof(Word);
      if (dir==0) {
	fr = sizeof(StDpmHead);
	to = 0x7FC;
      } else {
	fr = 0x7FC-ln;
	to = sizeof(StDpmHead)-ln;
	ln = -ln;
      };
      
      for (i=fr; i!=to; i+=ln) { /* DP Memory addresses */
	if (dir==0 && i>to || dir==1 && i<to) break;
	res->Addr = i;
	m.b = dpmb + i;
	tem = 0xAAAA;
testa:
	if (recoset())
	  res->Status = 0x4000; /* BUS error occured during the write */
	else {
	  *m.w = tem;
	  wrd = *m.w;
	};
	noreco();
	if (res->Status || wrd != tem) {
faila:    res->Status |= 0x8000|dir;
	  res->Val  = tem;
	  res->Back = wrd;
	  if (res->Status &0x6000) PrError(module,Tg8ERR_BUS_ERR);
	  return;
	};
	switch (tem) {
	case 0xAAAA:
	  tem = 0x5555;
	  break;
	case 0x5555:
	  tem = 0;
	  break;
	default:
	  continue; /* Test the next location */
	};
	goto testa;
      };
  };
}

/*================*/
/* AutoTests      */
/*================*/

void AutoTest(module,res)
Tg8DrvrModuleContext *module;
Tg8AutoTest *res; {

Tg8Dpm *dpram = module->ModuleAddress.VMEAddress;
Tg8LoadDpm *dpl = (Tg8LoadDpm *) dpram;
Tg8DrvrModuleAddress *moad = &module->ModuleAddress;
Uint swset;
  res->Res = 0;
  if (recoset()) {
    res->Res |= 0x8000; /* BUS Error occured */
    PrError(module,Tg8ERR_BUS_ERR);
  } else
    switch (res->Test) {
    case Tg8CARD_RESET:      /* Reset the card */
      dpl->Firmware_status = Tg8DOWNLOAD;
      ResetMPC(dpram);       /* Issue the TG8 reset signal */
      break;
    case Tg8CARD_CONF:       /* Check the card configuration */
      res->V1 = ReadVectorMPC(dpram) & 0xFF; /* Vector number */
      res->V2 = ReadStatusMPC(dpram);        /* Hardware status */
      if (res->V1 != moad->InterruptVector) res->Res |= 1;
      swset = (res->V2 & Tg8HS_EXTERNAL_CLOCK_JUMPER)? 1: 0;
      if (swset != moad->SwitchSettings) res->Res |= 2;
      break;
    case Tg8CARD_STAT:       /* Check the status register */
      res->V1 = ReadStatusMPC(dpram);        /* Hardware status */
      if (res->V1 != 0x3800 && res->V1 != 0x7800) res->Res |= 1;
      break;
    case Tg8CARD_ACK:        /* Check the card's boot program acknowledge */
      res->V1 = dpl->Firmware_status;
      if (res->V1 != Tg8BOOT) res->Res |= 1;
      break;
    case Tg8CARD_MBX:        /* Check the mailbox protocol support */
      res->V1 = PingModule(module);
      if (res->V1 != Tg8ERR_OK) res->Res |= 1;
      break;
    case Tg8CARD_XR:          /* Check the XILINX Receiver */
      res->V1 = *(Word*)&dpram->At.aDt.aRcvErr;  /* Get the err_hour pair */
      res->V1 >>= 8; /* Extract the Err part only */
      if (res->V1) res->Res |= 1;
      break;
    default:
      res->Test = 0;
    };
  noreco();
}

/*=========================================================================*/
/**************** Handle transaction timeouts on mail box **************** */
/*=========================================================================*/

int MbxTimeout(module)
Tg8DrvrModuleContext *module; {

   module->Timer = 0;
   sreset(&module->BusySemaphore);
}

/*=========================================================================*/
/* Wait for a mail box interrupt with timeout. A request is set in the     */
/* mail box, after which this routine initiates the transaction, and waits */
/* for the Tg8 to respond.                                                 */
/*=========================================================================*/

int WaitForMailbox(module,cmd)
Tg8DrvrModuleContext *module;
Tg8Operation cmd; {

Tg8Dpm *dpram; /* Tg8Dpm address */
int   ps;       /* Processor status word */
short st;       /* An error code */

   /* Get pointers to Tg8Dpm and the working area */
   dpram = module->ModuleAddress.VMEAddress;

   /* Set the timeout and initiate the request. */

   if (module->BusySemaphore) {
     /* Op. was completed after the request timed out probably ! */
     strcpy(module->ErrMsg,"[W]BUSY SEMAPHORE SET, RESET IT");
     sreset(&module->BusySemaphore); /* Clean the semaphore */
   };
#if 0
   if (dpram->At.aMbox != Tg8OP_NO_COMMAND) {
     dpram->At.aCoco = Tg8ERR_PENDING;
     goto fail; /* The firmware crashed */
   };
#endif
   module->Timer = timeout(MbxTimeout,(char *) &module->SleepSemaphore,wa->FirmwareTimeout);
   if (module->Timer <= 0) {
      module->Timer =  0;
fail: if (dpram->At.aCoco == Tg8ERR_OK) dpram->At.aCoco = Tg8ERR_REJECTED;
      st = dpram->At.aCoco;
      ssignal(&module->TransferSemaphore); /* Allow following transactions */
      return (PrError(module,st));
   };

   eieio();
   dpram->At.aMbox = cmd;   /* Start transaction */
   eieio();

   swait(&module->BusySemaphore,SEM_SIGIGNORE);   /* Wait blocking signals */

   /*****************************************/
   /* If the timeout is zero, we timed out. */
   /*****************************************/

   if (!module->Timer) {
     dpram->At.aCoco = Tg8ERR_TIMEOUT;
     goto fail;
   };

   /* All OK cancel timeout and return */
   CancelTimeout(&module->Timer);
   if (dpram->At.aCoco != Tg8ERR_OK) goto fail;

   return(Tg8ERR_OK);
}

/* eof low.c */



