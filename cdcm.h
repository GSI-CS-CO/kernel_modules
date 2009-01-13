/**
 * @file cdcm.h
 *
 * @brief General CDCM definitions.
 *
 * @author Georgievskiy Yury, Alain Gagnaire. CERN AB/CO.
 *
 * @date Created on 02/06/2006
 *
 * Should be included by anyone, who wants to use CDCM.
 * Many thanks to Julian Lewis and Nicolas de Metz-Noblat.
 *
 * @version $Id: cdcm.h,v 1.4 2009/01/09 10:26:03 ygeorgie Exp $
 */
#ifndef _CDCM_H_INCLUDE_
#define _CDCM_H_INCLUDE_

#ifdef __linux__

#include "cdcmDrvr.h"
#include "cdcmLynxAPI.h"
#include "cdcmLynxDefs.h"

#else  /* __Lynx__ */

#include "cdcmBoth.h"
#include "cdcmIo.h"

#include <dldd.h>
#include <errno.h>
#include <sys/types.h>
#include <conf.h>
#include <kernel.h>
#include <sys/file.h>
#include <io.h>
#include <sys/ioctl.h>
#include <time.h>
#include <param.h>
#include <string.h>
#include <ces/absolute.h>
#include <ces/vmelib.h>

extern unsigned long find_controller   _AP((unsigned long vmeaddr, unsigned long len, unsigned long am, unsigned long offset, unsigned long size, struct pdparam_master  *param));
extern unsigned long return_controller _AP((unsigned long physaddr, unsigned long len));
extern int vme_intset                  _AP((int vct, int (*handler)(),char *arg, long *sav));
extern int vme_intclr                  _AP((int vct, long *sav));
extern int nanotime(unsigned long *);

#endif	/* __linux__ */

#endif /* _CDCM_H_INCLUDE_ */
