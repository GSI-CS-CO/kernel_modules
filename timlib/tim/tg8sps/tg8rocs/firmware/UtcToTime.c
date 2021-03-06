/****************************************************************************/
/* Super efficient UTC to date conversion routine, it works until 20100.    */
/* YR & YY are set for 2003: (Wed Jan 1 00:00:00 2003 UTC = 1041375600)     */
/* Author: Julian Lewis: Brain Storming: Mike Clayton                       */
/****************************************************************************/

/* ================================================== */
/* Change these three constants in 2011               */

#define YR 1041379200 /* Wed Jan 1 00:00:00 2003 UTC  - 1 hour ???? */
#define YY 3          /* Year is 2003                 */

/* ================================================== */

#define YEARS 8       /* Number of years valid */
#define MONTHS 12     /* Months in a year */
#define SY 31536000   /* Seconds in a non leap year */
#define SD 86400      /* Seconds in a day */
#define SH 3600       /* Seconds in an hour */
#define SM 60         /* Seconds in a minute */

unsigned char DecToBcd(unsigned char dec) {
   return ((dec/10) << 4) | ((dec%10) & 0xF);
}

/* ==================================================================== */
/* This routine returns a pointer to a DateTime structurs corresponding */
/* to the supplied UTC time in seconds.                                 */
/* ==================================================================== */

void UtcToTime() {

unsigned long x; /* Contains seconds being used in calculations */
char lp;         /* lp leap */
int i;
Tg8DateTime ldt;

   utc -= YR;
   i=0;
   do {
      x = SY; lp = 0;
      if (!((i+YY) % 4)) { x += SD; lp = 1; }  /* Add leap day if needed */
      if (utc >= x) utc -= x;
      else break;
      i++;
   } while (1);
   ldt.aYear = DecToBcd(YY+i);

   i = 0;
   do {
      x = ((unsigned long) dm[i]) * SD;
      if (lp && (i==1)) x += SD; /* Deal with Febuary */
      if (utc >= x) utc -= x;
      else break;
      i++;
   } while (1);
   ldt.aMonth  = DecToBcd(i+1);

   ldt.aDay    = DecToBcd((utc/SD) +1); utc = utc%SD;
   ldt.aHour   = DecToBcd(utc/SH);      utc = utc%SH;
   ldt.aMinute = DecToBcd(utc/SM);      utc = utc%SM;
   ldt.aSecond = DecToBcd(utc);

   memcpy16((short *) &(dpm->At.aDt),(short *) &ldt,sizeof(Tg8DateTime));
   return;
}
