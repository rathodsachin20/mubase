/* %W%  %G%     %R% */
/*********************************************************************/
/**        Copyright (c) 1993-94 AT&T.   All Rights Reserved.       **/
/**        Dali Storage Manager.                                    **/
/**        AT&T Bell Labs, Lab 1138.                                **/
/**                                                                 **/
/**        THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T.     **/
/**        The copyright notice above does not evidence any         **/
/**        actual or intended publication of such source code.      **/
/**                                                                 **/
/**   THIS SOFTWARE  COMES WITH NO  WARRANTIES.  ANY LIABILITY OF   **/
/**   ANY KIND OF DAMAGES RESULTING FROM THE USE OF THIS SOFTWARE   **/
/**   IS DISCLAIMED.                                                **/
/*********************************************************************/

/**
 *********************************************************************
 *
 *  Overview:
 *    This is the main user include file for Dali.  All system 
 *  headers should be included here or in config.h.
 *
 *********************************************************************
**/

#ifndef _DALI_H
#define _DALI_H

#include <unistd.h> 
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

#ifndef __hpux
// sysent.h does not exist on hpux
#include <sysent.h>
#endif


//#include "config.h"

enum ReturnCode {  
    /* Standard Dali return codes */
        DALI_SUCCESS=1, 
        DALI_FAILURE=-1,
        DALI_WAITING=-2,
        DALI_NO_SUCH_ITEM=-3,
        DALI_TIMEOUT=-4
} ;

//#include "debug.h"
//#include "error.h"


#ifdef MAKE_INLINE_LOG
#define INLINE_LOG inline
#else
#define INLINE_LOG
#endif

#ifdef MAKE_INLINE_TTREE
#define INLINE_TTREE inline
#else
#define INLINE_TTREE
#endif

#ifdef MAKE_INLINE_HASH
#define INLINE_HASH inline
#else
#define INLINE_HASH
#endif


//#include "dali_ptr.h"
//#include "dali_trans.h"
//#include "template.h"
//#include "dali_proc.h"
//#include "dali_item_mgr.h"
//#include "dali_func.h"
//#include "dali_sys.h"
//#include "safemutex.h"
//#include "dali_rec_mgr.h"
//#include "dali_cc_mgr.h"
//#include "mmdb.h"
//#include "procinfo.h"
//#include "sysdb.h"
//#include "hf.h"

#ifdef MAKE_INLINE_LOG
#include "loginline.h"
#endif

//#include "ttreeinline.h"
//#include "hashinline.h"


/*****************************************************************************/
#endif
         //_DALI_H 

