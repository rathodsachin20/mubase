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
 *      Functions(s):
 *
 *      Description:
 *              your description goes here.
 *
 *********************************************************************
**/

#ifndef _ERROR_H
#define _ERROR_H

/* error.h */

extern FILE *dali_tracefile;
extern char dali_command[];


/***********************************************************************
err.h:  from Biliris and Thimios
***********************************************************************/



extern char dali_errm[];  /* a string to place an error message */
extern void dali_exit(int status);
 

#ifdef NERRM 
#    define Err(m)      {} 
#    define ErrA(m,a)   {}
#    define ErrAA(m,a,b)        {}
#    define ErrAAA(m,a,b,c)     {}
#    define ErrAAAAA(m,a,b,c,d) {}
#else
/*
#    define Err(m)     {fflush(stdout); fprintf(dali_tracefile, \
                       "Error: %s (File:%s, Line:%d):\n     %s\n", \
                        dali_command, __FILE__,\
                        __LINE__, m) ; fflush(dali_tracefile);}
#    define ErrA(m,a)      {sprintf(dali_errm, m, (a)); Err(dali_errm);}
#    define ErrAA(m,a,b)   {sprintf(dali_errm, m, (a), (b)); Err(dali_errm);}
#    define ErrAAA(m,a,b,c){sprintf(dali_errm, m, (a), (b), (c)); \
                               Err(dali_errm);}
#    define ErrAAAA(m,a,b,c,d)   {sprintf(dali_errm, m, (a), (b), (c), (d)); \
                                  Err(dali_errm);}
*/
#    define Err(m)      {} 
#    define ErrA(m,a)   {}
#    define ErrAA(m,a,b)        {}
#    define ErrAAA(m,a,b,c)     {}
#    define ErrAAAAA(m,a,b,c,d) {}
#endif

                        /* print an Error mesage and Return a value   */
#define ErrR(m, r)      {Err(m); return (r);}

#define ErrAR(m,a,r)            {ErrA(m,a); return (r);}

                        /* print an Error mesage and Exit             */
#define dali_exit(e)    {exit(e);}
#define ErrE(m, e)      {Err(m); dali_exit (e);}
#define ErrAE(m,a,e)    {ErrA(m,a); dali_exit (e);}
#define ErrAAE(m,a,b,e) {ErrAA(m,a,b); dali_exit (e);}

/**********************************************************************/

extern int CheckWithUser(char *s);

#define CheckWithUserA(msg,arg)   (sprintf(dali_errm, msg, (arg)), \
                                        CheckWithUser(dali_errm))
#define CheckWithUserAA(msg,arg1,arg2)  (sprintf(dali_errm, msg, (arg1), \
                                        (arg2)), CheckWithUser(dali_errm))


/************** ERROR CODES RETURNED ITEM MANAGER FUNCTIONS *************/


#define DALI_ERROR -1

/** WARNING: the following lock error codes must be consistent with **
 ** the codes returned by the lock manager as defined in lock.h     **/

#define DALI_LOCK_DELETED  -6     /* object to be locked has been deleted */
#define DALI_LOCK_NOT_GRANTED = -5  /* Maybe because of non-blocking request */
#define DALI_LOCK_NOT_OWNER -4
#define DALI_LOCK_TIMEOUT   -3
#define DALI_LOCK_DEADLOCK  -2
#define DALI_LOCK_ERROR     -10

#define DALI_ITEM_NOT_FOUND -7
#define DALI_FREE_ERROR -8
#define DALI_NOT_ENOUGH_MEMORY -9
#define DALI_ALREADY -11

#define DALI_MUTEX_TIMEOUT -12

#define DALI_UNIQUE_INDEX_VIOLATION    (-100)  /* unique index violation */

#define DALI_HF_RECORD_DELETED  -101

#endif /* _ERROR_H */

