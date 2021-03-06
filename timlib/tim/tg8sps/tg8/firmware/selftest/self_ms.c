/*****************************************************************/
/* TPU Channel 0 - MS clock                                      */
/*****************************************************************/

MsTest() {
int n; unsigned int i; short f1;

  dpm->Info.TestProg = T_MS;

  /* Enable Bus Monitor (BME) & Set BMT=64 Clocks */
  sim->SimProtect = Tg8ENABLE_BUSMONITOR;
  bus_int = 0;

  /* Wait for the next 1000 Ms interrupts */
  /* Watchdog might wake up */

  for (f1=0,n=tpu_int[0]; f1<2; f1++)
    for (i=0; i<500000; i++)
      if (tpu_int[0] != n) break;
  if (tpu_int[0] == n) dpm->Info.FaultType |= T_MS;  /* Sorry, no MS interrupts */
  else                 dpm->Info.TestPassed |= T_MS; /* OK */
}

/* eof */
