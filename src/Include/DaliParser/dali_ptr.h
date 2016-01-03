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

/*                                                              */
/*    Copyright (c) 1994                                AT&T            */
/*              All Rights Reserved                             */
/*                                                              */
/*      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T.            */
/*        The copyright notice above does not evidence any              */
/*       actual or intended publication of such source code.            */
/*                                                              */

#ifndef _DBPTR_H
#define _DBPTR_H

class DaliDB;
extern const DaliDB *DaliDB_NULL;
class DbStartData;
class ostream;

/*
The inet_addr field of DatabaseID (below) is irrelevant if all dbptr 
refs are within a single sysdb.  Change DALI_FARM (config.h) to 1 to enable 
cross system references.
*/

class DatabaseID {
  public:
#if DALI_FARM
unsigned long inet_addr;  // Internet address of machine on which db
                              // resides;  Should be changed to new 64 bit
                              // address format when it is introduced.
#endif
    short loc_id;             // Specifies which local name of dabatase
    short _extra;              // two bytes that would always be aligned into this
                              // structure.  Used to store chunk number in DbPtr.
   /* WARNING:  If this class is edited, make sure its shadow in 
    *           interface.x is kept up to date
   */
    DatabaseID() {
        loc_id = 0;
        _extra = 0;
#if DALI_FARM
        inet_addr = 0;
#endif
    }

#if DALI_FARM
    DatabaseID(unsigned long inaddr, short _locid, int ex = 0) {
        loc_id = _locid;
        inet_addr = inaddr;
        _extra = ex;
    }
#else 
    DatabaseID(unsigned long , short _locid, int ex = 0) {
        loc_id = _locid;
        _extra = ex;
    }
#endif

    unsigned long internet_addr() { 
#if DALI_FARM
        return inet_addr; 
#else
        return 0;
#endif
    }
    int local_id() const { return loc_id; }
    short extra() const {return _extra;}
    DaliDB * db_ptr() const;
    void set_extra(short ex) { _extra = ex; }
    int operator == (const DatabaseID &i) const { 
#if DALI_FARM
        return ( inet_addr == i.inet_addr && loc_id == i.loc_id);
#else
        return (loc_id == i.loc_id);
#endif
    }
    int operator != (const DatabaseID &i) const { 
#if DALI_FARM
        return ! (inet_addr == i.inet_addr && loc_id == i.loc_id);
#else
        return ! (loc_id == i.loc_id);
#endif
        // return !(*this == i);        YSW
    }
    friend ostream & operator << (ostream& s, const DatabaseID &ptr);

};

extern const DatabaseID DatabaseID_NULL;
class DbOffset;
class SegHeader;

typedef long DaliDB_unique_id;   /* Used as unique id of a database */

class DaliPtr {
public:
    DatabaseID dbid;
    unsigned int mptr;
   /* WARNING:  If this class is edited, make sure its shadow in 
    *           interface.x is kept up to date
   */
    // Constructors 
    inline  DaliPtr(void) { dbid = DatabaseID_NULL; mptr = 0; };
    inline DaliPtr(const DaliPtr &pr2) { dbid = pr2.dbid; mptr = pr2.mptr; };
/* OUTLINED */
           DaliPtr(const DatabaseID &id, int _mptr) ;
    inline DaliPtr(const DatabaseID &id, void *_mptr);
    inline DaliPtr(const DaliDB * db, void *v);
    inline DaliPtr(const DaliDB * db, int offset); 
    inline DaliPtr(const DaliDB * db, int chunk_num, void *v);
    inline DaliPtr(const DaliDB * db, int chunk_num, int offset); 
           DaliPtr(const DatabaseID &db, const DbOffset& offset); 
    inline DaliPtr(const DaliDB *db, const DbOffset& offset); 

    inline DaliPtr(DbStartData *db, void *v);
    inline DaliPtr(int t);  

    short chunk() const {return dbid.extra();}
    void set_chunk(short chnk) { dbid.set_extra(chnk);}

  /*** FIX:  rename f_alloc and f_free to:  
    static DaliPtr malloc(const DaliDB *db, const TransID &tid, int size);
    int free(const DaliDB *db, const TransID &tid, int size);
  ***/

    // Operators

    // normal assignment of one DaliPtr to another
/** assignment copies the extra info **/
/** Comment:  a compiler generated bitwise copy might be faster.  And this is
  * one of the most inner things in Dali.
  **/

    inline DaliPtr& operator=(const DaliPtr& pr2) { 
        dbid = pr2.dbid; mptr = pr2.mptr; return *this; 
    };

    friend ostream & operator << (ostream& s, const DaliPtr &ptr);
/** equality ignores the extra info **/
    friend inline int operator==(const DaliPtr& pr1,const DaliPtr& pr2);
    inline int operator==(const int i) const { 
        return (mptr == 0 && i == 0) ;
    };
    friend inline int operator!=(const DaliPtr& pr1,const DaliPtr& pr2) { 
        return !(pr1 == pr2);
    }
    inline int operator! () const { return (mptr == 0); }

    // Functions 
    // Should return pointer into process memory.  DaliPtr will store offset 
    //  into database, and from DaliDB we can find the start address of the
    //  DB in user process space.

      //  The DaliDB argument to mem_ptr is an efficiency hack.
      //  If it is not passed, there is enough information via the 
      //  dbid field.

    inline DatabaseID DbId() const { return dbid; }
    inline int offset() const { return mptr; }  

    inline void*  mem_ptr(const DaliDB *db) const;
    inline void*  mem_ptr() const;

    inline void*  mem_ptr(const DbStartData *db) const {
            return (char *)db+mptr; 
    };

    inline const void* mem_ptrC(const DaliDB *db) const ;
    inline const void* mem_ptrC() const ;

    inline DaliDB * db_params() const;

    inline operator void *() { return mem_ptr();}
    inline DbOffset db_offset() const;

    inline SegHeader * get_seg_header() const;

    inline int valid() const;

    void printOn(FILE *fp) const { fprintf(fp," %d.%d",DbId().local_id(),offset());}

    friend DaliPtr operator+ (const DaliPtr& pr1, int i);
    friend DaliPtr operator+ (int i, const DaliPtr& pr1);
    friend class DbMapInfo;
    friend class SysDBPtr;
};

extern const DaliPtr DaliPtr_NULL;    // null DaliPtr


 /************** Class SysDBPtr *********************************/

class SysDBPtr{
public:
  unsigned int mptr;
  // Constructors
  inline SysDBPtr(void) { mptr = 0;}
  inline SysDBPtr(const SysDBPtr &pr2) {mptr = pr2.mptr;}
  inline SysDBPtr(const DaliPtr &pr2);
  inline SysDBPtr(int _mptr) {mptr = _mptr;}
  inline SysDBPtr(void *_mptr);

  // Operators
  inline SysDBPtr& operator =(const SysDBPtr& pr2){
     mptr = pr2.mptr;
     return *this;
  };
  
  friend inline int operator == (const SysDBPtr& pr1, const SysDBPtr& pr2){
     return (pr1.mptr == pr2.mptr);
  };
   
  inline int operator == (const int i) const {
     return (mptr == i);
  };

  friend inline int operator !=(const SysDBPtr& pr1, const SysDBPtr& pr2) {
     return(pr1.mptr != pr2.mptr);
  };
  inline int operator !() const { return (mptr == 0); }

  friend inline SysDBPtr operator + (const SysDBPtr& pr1, int i);
  friend inline SysDBPtr operator + (int i, const SysDBPtr& pr1);

  // Functions

  inline void * mem_ptr();
  // Outline...
  DaliPtr dali_ptr() const;

  inline operator void *() {return mem_ptr();};
  inline unsigned int offset() { return mptr; };
  void printOn(FILE *fp) const { fprintf(fp," SysDB.%d",mptr);}
};

extern const SysDBPtr SysDBPtr_NULL;
 
inline SysDBPtr operator + (const SysDBPtr& pr1, int i) {
  return( SysDBPtr(pr1.mptr+i));
}

inline SysDBPtr operator + (int i, const SysDBPtr& pr1) {
  return( SysDBPtr(pr1.mptr+i));
}


 /************** Class DbOffset *********************************/

class DbOffset{
public:
  unsigned int mptr;
  // Constructors
  inline DbOffset(void) { mptr = 0;}
  inline DbOffset(const DbOffset &pr2) {mptr = pr2.mptr;}
  inline DbOffset(int _mptr) {mptr = _mptr;}
  inline DbOffset(const DaliDB *db, void *_mptr);
  inline DbOffset(const DatabaseID &dbid, void *_mptr);
  inline DbOffset(const DaliPtr &dbptr) { mptr = dbptr.offset();}

  // Operators
  inline DbOffset& operator =(const DbOffset& pr2){
     mptr = pr2.mptr;
     return *this;
  };
  
  friend inline int operator == (const DbOffset& pr1, const DbOffset& pr2){
     return (pr1.mptr == pr2.mptr);
  };
   
  inline int operator == (const int i) const {
     return (mptr == i);
  };

  friend inline int operator !=(const DbOffset& pr1, const DbOffset& pr2) {
     return(pr1.mptr != pr2.mptr);
  };
  inline int operator !() const { return (mptr == 0); }

  friend inline DbOffset operator + (const DbOffset& pr1, int i);
  friend inline DbOffset operator + (const DbOffset& pr1, unsigned int i);
  friend inline DbOffset operator + (int i, const DbOffset& pr1);

  // Functions
  inline SegHeader * get_seg_header(DaliDB *db) const;


  inline void * mem_ptr(const DaliDB *db) const;
  inline void * mem_ptr(const DatabaseID &dbid) const;
  inline void * mem_ptr(const DbStartData *dbs) const;
  inline int offset() const { return mptr; }
  inline operator int () { return mptr;} // FIX: remove this operator, should just
                                        // use offset to avoid bugs in which a 
                                        // DbOffset is passed to a function which should
                                        // get an integer.
  void printOn(FILE *fp) { fprintf(fp," DB.%d",mptr);}
};

extern const DbOffset DbOffset_NULL;
 
inline DbOffset operator + (const DbOffset& pr1, int i)
{
    return( DbOffset(pr1.mptr+i));
}

inline DbOffset operator + (const DbOffset& pr1, unsigned int i)
{
    return pr1 + (int) i;
}

inline DbOffset operator + (int i, const DbOffset& pr1)
{
    return( DbOffset(pr1.mptr+i));
}


inline DaliPtr::DaliPtr(int t)
{ 
    if(t) {
        Err("Non-zero integer assignment to DaliPtr");          
    }
    else {
        dbid = DatabaseID_NULL;
        mptr = 0;
    }
}                                                                           

inline DbOffset DaliPtr::db_offset() const
{
    return DbOffset(mptr);
}  

inline int dali_div_by_pagesize(int someaddr)
{
        return someaddr >> DaliConf::log2_pagesize;
}

inline int page_align_down(int offset) {
    return offset & (DaliConf::pagesize - 1);
}

/*
inline int page_align(int offset) {
    return align(offset, DaliConf::pagesize);
}
*/


inline int align_by_4(int x)
{
        return ((x-1) | 3) + 1;
}

inline int align_by_8(int x)
{
        return ((x-1) | 7) + 1;
}

inline int align_by_power2(int x, int y)
{
        return ((x-1) | (y-1)) + 1;
}

inline int align(int x, int y)
{
    return (x/y)*y + (x%y? y : 0);
}

inline int page_align(int offset) {
    return align_by_power2(offset, DaliConf::pagesize);
}



/***************************************************************************/
#endif 
        // _DBPTR_H
