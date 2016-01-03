#include "error.h"
/*
#define SQLErr(m)     {fflush(stdout); fprintf(dali_tracefile, \
                       "SQL error in stmt no %d- %s\n", stmtCount, m) ; \
                           fflush(dali_tracefile);}
#define SQLErrA(m,a)   {sprintf(dali_errm, m, (a)); SQLErr(dali_errm);}
#define SQLErrAA(m,a,b)   {sprintf(dali_errm, m, (a), (b)); SQLErr(dali_errm);}
#define SQLErrAAA(m,a,b,c){sprintf(dali_errm, m, (a), (b), (c)); \
                               SQLErr(dali_errm);}
#define SQLErrAAAA(m,a,b,c,d)   {sprintf(dali_errm, m, (a), (b), (c), (d)); \
                                  SQLErr(dali_errm);}

                         print an Error mesage and Return a value   
#define SQLErrR(m, r)   {SQLErr(m); return (r);}
#define SQLErrAR(m,a,r)         {SQLErrA(m,a); return (r);}
#define SQLErrAAR(m,a,b,r)              {SQLErrAA(m,a,b); return (r);}
#define SQLErrAAAR(m,a,b,c,r)           {SQLErrAAA(m,a,b,c); return (r);}

                         print an Error mesage and Exit          
#define dali_exit(e)    {exit(e);}
#define SQLErrE(m, e)   {SQLErr(m); dali_exit (e);}
#define SQLErrAE(m,a,e) {SQLErrA(m,a); dali_exit (e);}
#define SQLErrAAE(m,a,b,e)      {SQLErrAA(m,a,b); dali_exit (e);}
#define SQLErrAAAE(m,a,b,c,e)           {SQLErrAAA(m,a,b,c); dali_exit(e);}
*/
#define SQLErr(m)     {}
#define SQLErrA(m,a)   {}
#define SQLErrAA(m,a,b)   {}
#define SQLErrAAA(m,a,b,c){}
#define SQLErrAAAA(m,a,b,c,d)   {}
#define SQLErrR(m, r)   {}
#define SQLErrAR(m,a,r)         {}
#define SQLErrAAR(m,a,b,r)              {}
#define SQLErrAAAR(m,a,b,c,r)           {}

#ifndef dali_exit
#define dali_exit(e)    {}
#endif

#define SQLErrE(m, e)   {}
#define SQLErrAE(m,a,e) {}
#define SQLErrAAE(m,a,b,e)      {}
#define SQLErrAAAE(m,a,b,c,e)           {}
