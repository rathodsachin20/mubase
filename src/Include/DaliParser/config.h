#ifndef _DALI_CONFIG_H
#define _DALI_CONFIG_H
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

/* config.h:
        Contains database and system configuration information.
        WARNING: This file should be able to compile with either C++
        or C, so use #defines, and move consts to system.h.
*/

#include <stdlib.h>

#define SPARC  1

#ifdef solaris2
#define DALI_SYSV
#endif

#ifdef __hpux
#define DALI_SYSV
#endif

#ifdef sunos
#define DALI_BSD
#undef DALI_SYSV
#endif

/***************************************************************************/
/*  Architecture Dependent Configuration Information */
/***************************************************************************/

/* 4-byte data type - should be changed depending on */
/* the platform.                                     */

typedef          int DaliInt32;      
typedef unsigned int DaliUInt32;

typedef int DaliCodeWord;

/***************************************************************************/
/*  Operating System configuration information */
/***************************************************************************/

#define MMAP_BEYOND_EOF

#define HAS_VOLATILE
#define MLOCK_DECLARED

#ifdef DALI_SYSV
/* #define OLDSTREAMS */
#define MEMCPY
#define RPCTLI
#else 
#ifdef DALI_BSD
/* #define OLDSTREAMS */
#endif
#endif

/*
 * OS dependent changes to above should be here...
 */

#ifdef __alpha
#undef MMAP_BEYOND_EOF
#undef SPARC
#define DALI_64BIT
/* #undef RPCTLI */
#endif

#ifdef __hppa
#undef MMAP_BEYOND_EOF
#undef SPARC
#undef HAS_VOLATILE
#undef RPCTLI
#endif

#ifdef __i386
#undef SPARC
#endif

#ifdef mips
#include <bstring.h>
#endif 

#ifdef DALI_SYSV
#include <string.h>
#endif

#ifdef MEMCPY
        /* Should be SYSV */
#define dali_bcopy(s,t,n) memcpy(t,s,n)
#define dali_bzero(s,n)  memset(s,'\0',n)
#define dali_bcmp(s,t,n)  memcmp(s,t,n)
#else
#define dali_bcopy(s,t,n) bcopy(s,t,n)
#define dali_bzero(s,n)  bzero(s,n)
#define dali_bcmp(s,t,n)  bcmp(s,t,n)
#endif

#ifdef __GNUC__
#define DALI_OLD_ASSIGN
#define DALI_NO_SIGPF
#else
#define DALI_CPLUS_RPC_HDR
#define DALI_SIGPF
#endif

#ifdef _CFRONT30
#undef HAS_VOLATILE
#undef MLOCK_DECLARED
#endif

#ifdef HAS_VOLATILE
#define VOLATILE volatile
#else
#define VOLATILE
#endif


/***************************************************************************/
/*  Database configuration information */
/***************************************************************************/
#define DALI_MAJOR_VERSION_NUMBER   (2)
#define DALI_MINOR_VERSION_NUMBER   (0)
        /* WARNING:  DaliMajorVersionNumber should be updated every time changes
                     are made that affect the core structure of database files,
                     that are incompatible with existing databases 
         */ 

#define DALI_FARM 0
        /* WARNING: Make sure above is consistent with c_convert.h */

#ifndef CODEWORD_PAGELATCH
#define CODEWORD_PAGELATCH 1
#endif

#if CODEWORD_PAGELATCH
#define PAGELATCH 1
#else
#define PAGELATCH 0
#endif

#define DALI_DEFAULT_SYSDB  "SysDB"
/* Look in the current DALI_DBPATH */

/************************************************************/
/*******     Environment variables consulted    *************/
/************************************************************/

#define DALI_ENV_DBPATH  "DALI_DBPATH"
#define DALI_ENV_SYSDB   "DALI_SYSDB"
#define DALI_ENV_PNUM    "DALI_ROOT_PNUM"

                
/************************************************************/

/** Maximum length of path name of database  **/
#define MAX_DALI_PATH_NAME 256
#define MAX_DALI_MC_NAME   128
/************************************************************
*
*   Database Parameters :  ARE DEFINED IN sysdb.c
*
************************************************************/
#if CODEWORD_PAGELATCH
#define MAX_CONCUR_SHARED_MUTS_PER_BLOCK   (300)
#else
#define MAX_CONCUR_SHARED_MUTS_PER_BLOCK   (30)
#endif

#define MAX_CONCUR_MUTS (30)
#define MAX_CONCUR_PROCS   (100)

#define DALI_MAX_OP_TYPE 150

#define MAX_MAPPED_DBS 1024

/************************************************************
*
*   Chunk/Segment Parameters :  Are defined in segments.c
*
************************************************************/

/*  Storage allocator parameters */
#define DALI_MAX_CHUNKS  32
#define DALI_MAX_BUCKETS 30
#define DALI_MIN_ALLOC 8

#define DALI_SEGS_PER_ARRAY 800
        /* Above set so that seg_array is just under a page */

#define DALI_DEFAULT_SEG_SIZE (4*4096)

/********* Transaction Table defines **********/

#define MAX_PRECOMMITTED_TRANS   (100)   
       /* precommitted, but not post committed. */

#define DALI_MAX_ACTIVE_TRANS (1024)

#define DALI_DBS_PER_TRANS  (50)

/*
 * The anti checkpoint db_sem is hashed into a hash table this big.
 * If two db's map to the same place, then actions on one will block
 * out checkpoints on the other.  If many DB's are created and 
 * destroyed, then we may need a tool which does:
 *
 *  Find out/compute sets of db's used together
 *  precompute the hash function and store it in the db
 *  compute it so that none of the sets that work together have the
 *  same offset.
 *
 * The other option, particularly if a common tx accesses many DBs
 * (more than 20), is to  increase this number.
 * Increasing this by one uses an additional 2K of memory or so,
 * more if DALI_MAX_ACTIVE_TRANS goes above 1024. Conversely, 
 * memory can be saved by decreasing, especially if there is only
 * a small number of dbs.
 */

   /* BEGIN Per process static variables used for CONFIGURATION */
   /* Not accessable by C code without a C++ wrapper. */
#if __cplusplus
struct DaliConf {
    static int use_rootserver;
    static int recover_safeguards;
    static int global_recovery_kill_procs;
    static int interactive_global_recovery;
    static long max_log_length;
    static long checkpoint_log_interval;
    static int flush_buffer_size;
    static char archive_cmd[MAX_DALI_PATH_NAME];
    static int file_increment;
    static int mem_increment;
    static int lock_release_on_precommit;
    static int max_sysdb_size; // FIX: for 64 bit architecture may need a long.
    static int max_db_size;
    static int pagesize;
        static int log2_pagesize; // Optimization - this stores log_2 of pagesize 
    static int lock_sleep_time;
    static int max_lock_tries;
    static char * const db_start;
    static int major_version_number;
    static int minor_version_number;
    static int use_shared_mem;
    static int use_codeword;
        static int log_alloc_node_size;

   /* END PER-PROCESS VARIABLES */
};
#endif 
        /* End C++ only */

/********* Debugging related defines not in debug.h **********/
#ifdef __cplusplus
// extern "C" {
#endif
extern char dali_command[];
extern void dali_debug_abort();

#ifdef __cplusplus
// }
#endif

#ifdef __STDC__
//# define dali_assert(ex)  {if (!(ex)){(void)fprintf(stderr, "Assertion " #ex " failed: file \"%s\", line %d - %s \n", __FILE__, __LINE__, dali_command); dali_debug_abort();}}
# define dali_assert(ex)  {if (!(ex)){(void)fprintf(stderr, "Assertion " #ex " failed: file \"%s\", line %d\n", __FILE__, __LINE__); abort();}}
#else
//# define dali_assert(ex)      {if (!(ex)){(void)fprintf(stderr, "Assertion ex failed: file \"%s\", line %d - %s \n", __FILE__, __LINE__, dali_command); dali_debug_abort();}}
# define dali_assert(ex)        {if (!(ex)){(void)fprintf(stderr, "Assertion ex failed: file \"%s\", line %d\n", __FILE__, __LINE__); abort();}}
#endif

#define DALI_R_ASSERT(x)  dali_assert(x)
#define DALI_ASSERT(x)  
#define DALI_D_ASSERT(x)   
/*
        #define DALI_ASSERT(x)  dali_assert(x)
        #define DALI_D_ASSERT(x)   dali_assert(x)
*/

#endif   /* _DALI_CONFIG_H */
