/**
 * @file cdcmCpu.h
 *
 * @brief Supported processor types 
 *
 * @author Yury GEORGIEVSKIY
 *
 * @date Created on 26/10/2008
 *
 * <long description>
 *
 * @version $Id: cdcmCpu.h,v 1.1 2009/01/09 10:05:27 ygeorgie Exp $
 */
#ifndef _CDCM_CPU_H_INCLUDE_
#define _CDCM_CPU_H_INCLUDE_

/*! @name Supported processor types
 *
 * See /acc/src/dsc/co/Make.common for more details on supported CPU types.
 */
//@{
#define ppc4 (('p'<<24)|('p'<<16)|('c'<<8)|'4') //!< LynxOS 4.0.0  for CES RIO806x OWS 6.2.0
#define Lces (('L'<<24)|('c'<<16)|('e'<<8)|'s') //!< Linux gcc-4.0 for RIO8064
#define L864 (('L'<<24)|('8'<<16)|('6'<<8)|'4') //!< diskless SLC4 Linux
#define L865 (('L'<<24)|('8'<<16)|('6'<<8)|'5') //!< diskless SLC5 Linux
//@}

#if 0
/* IT'S NOT FUCKING WORKING!!!!!!!!!! */
typedef enum {
  ppc4 = 1977,	//!< LynxOS 4.0.0  for CES RIO806x OWS 6.2.0
  Lces,		//!< Linux gcc-4.0 for RIO8064
  L864,		//!< diskless SLC4 Linux
  L865		//!< diskless SLC5 Linux
} cdcm_cpu;
#endif

#endif /* _CDCM_CPU_TYPES_H_INCLUDE_ */
