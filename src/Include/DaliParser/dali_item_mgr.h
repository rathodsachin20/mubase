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

#ifndef _DALI_ITEM_MGR_H
#define _DALI_ITEM_MGR_H

/*********************************************************************/

   /** 
    *  WARNING:  Should make sure C version of these definitions (in
    *   c_convert.h) are consistent with the definitions below.
   **/

enum MemType  { _NULL_MEM = 0, _PERSISTENT_MEM=1, _ZEROED_MEM=2,
                _TRANSIENT_MEM=3, _TRANSDURATION_MEM=4
              };

inline int is_dali_transient_space(MemType m) { return (m == _TRANSIENT_MEM || 
                                                m == _TRANSDURATION_MEM);
                               }
inline int is_dali_transient_data(MemType m) { return (m == _TRANSIENT_MEM || 
                                               m == _TRANSDURATION_MEM ||
                                               m == _ZEROED_MEM);
                                        
                               }

/* ZEROED_MEM is persistent in that it lasts longer than a single program
   run, but it is zeroed on system restart--no recovery or logging
   is done for data in such memory.  Its is useful for things like 
   semaphores.
*/

enum DaliChunkNumbers {
        DaliNullChunk = 0,
        DaliSegInfoChunk,
        DaliControlInfoChunk,
        DaliPersistentItemHeaderChunk,
        DaliPersistentFreeListChunk,
        DaliPersistentMemChunk,
        DaliZeroedMemChunk,
        DaliTransientItemHeaderChunk,
        DaliTransientFreeListChunk,
        DaliTransientMemChunk,
        DaliRelMgrHashNodeChunk,
        DaliRelMgrTtreeNodeChunk,
        DaliInitialNumChunks
};


int is_dali_transient_space(DaliDB *db, int chunk);
int is_dali_transient_data(DaliDB *db, int chunk);

enum ProtectMode { _DB_NOACCESS = 0,
                   _DB_RDONLY = 0x1,
                   _DB_WRONLY = 0x2,
                   _DB_RDWR =   0x3,
                   _DB_APPEND_ONLY = 0x4,
                   _DB_CREAT =  0x8,
                   _DB_TRUNC =  0x10,
                   _DB_REMOTEACC = 0x20,
                   _DB_TRANSIENT = 0x40,
                   _DB_RECOVER = 0x80,
                   _DB_MLOCK = 0x100,
                   _DB_EXCLUSIVE = 0x200,
                   _DB_PROTECT = 0x400
                 };

/*** Formerly was: 
enum LogActionMode { _LogAction, _DontLogAction, _LogDefault};  
        // Used by f_alloc and f_free
***/


enum LogActionMode { _LogOff = 0, _LogAtomic = 1, _LogStandard = 2 }; 
/***
        ****
            _LogStandard == All logging is on;  
                            Guarantees Atomicity + Durability
            _LogAtomic   == No logging to disk;  redo logging is turned off,
                            undo logging is turned on, pending_list logging is
                            turned on.
                            Guarantees Atomicity, but not durability.
            _LogOff      == No logging at all;
           
This next is not currently needed.  May be reintroduced.
            _LogDefault  == Used as argument to functions, where logging mode   
                                is left unspecified and deduced from the DB.
        ****
        * Note that the numbers are currently assigned so that 
        * a greater number means more logging.  If we ever 
        * have logging for recovery but not atomicity, then this
        * would no longer be possible, and DaliDB::do_*_logging()
        * would have to change.
        ****

***/
        


/*********************************************************************/


class ItemHeader;
class LockInfo;

#ifndef _lock_ptr_
#define _lock_ptr_

class Lock;
typedef SysDBPtr_of<Lock> LockSysDBPtr;
typedef DaliPtr_of<LockSysDBPtr> LockSysDBPtrDbPtr;

#endif 
        // _lock_ptr_  /* _lock_ptr_ is also defined in lock.h */

class ItemID { 
  DaliPtr header;
  public:                                                             
    ItemID (): header()
      {   }

/* This conversion is evil...   PB 9/19/94
   ItemID (const DaliPtr& t) : header(t)
      {   }
*/
/*
 * Several of these are nasty,since the header dbptr is an implementation
 * secret of an itemid.  FIX:?
 */

    inline void fix_uniqueid();

    ItemID (DaliDB *dbp, const ItemHeader *v) 
      : header(dbp, (void *) v)  // FIX: don't cast away constness?
      { fix_uniqueid();   }

    ItemID (DaliDB *db_params, int offset) 
      : header(db_params, offset) 
      { fix_uniqueid();   }

    ItemID (const DatabaseID &dbid, int offset) 
      : header(dbid, offset) 
      { fix_uniqueid();   }

        // FIX:  For the following functions, we may want to make a 
        //      struct AllocArgs containing arguments such as typetag,
        //      which most people probably do not want to see!

    ItemID (DaliDB *db, const TransID& tid, int chunk_num,
                          int _size, ItemID &_typetag);

        /* Creation/Deletion */
    static ItemID createItem (DaliDB *db, const TransID& tid, int size);
    static ItemID createItem (DaliDB *db, const TransID& tid,
                int chunk_num, int _size, ItemID _typetag);
    int deleteItem(DaliDB *db, const TransID& tid);

    static ItemID createItem (DaliDB *db, int size);
    static ItemID createItem (DaliDB *db,
                int chunk_num, int _size, ItemID _typetag);
    int deleteItem(DaliDB *db);

    static ItemID createItem (const TransID& tid, int size);
    static ItemID createItem (const TransID& tid,
                int chunk_num, int _size, ItemID _typetag);
    int deleteItem(const TransID& tid);

    static ItemID createItem (int size);
    static ItemID createItem (int chunk_num, int _size, ItemID _typetag);
    int deleteItem();


    int create (DaliDB *db, const TransID& tid, int size);
    int create (DaliDB *db, int size);
    int create (const TransID& tid, int size);
    int create (int size);

/**** Can't use this, it's a reserved word for C++
    void delete (DaliDB *db, const TransID &trans);
    void delete (const TransID &trans);
    void delete (DaliDB *db);
    void delete ();
*/

        /* Named Item Interface */

    int createNamed(DaliDB *db, const TransID &tid, 
                                const char *nm, int size);
    int createNamed(const TransID &tid, const char *nm, int size);
    int createNamed(DaliDB *db, const char *nm, int size);
    int createNamed(const char *nm, int size);

    /**** Static Versions ****/
    static ItemID createNamedItem(DaliDB *db, const TransID &tid, 
                                const char *nm, int size);
    static ItemID createNamedItem(const TransID &tid, const char *nm, int size);
    static ItemID createNamedItem(DaliDB *db, const char *nm, int size);
    static ItemID createNamedItem(const char *nm, int size);


    int deleteNamed(DaliDB *db, const TransID &tid, 
                                const char *nm);
    int deleteNamed(const TransID &tid,const char *nm);
    int deleteNamed(DaliDB *db, const char *nm);
    int deleteNamed(const char *nm);

    static int deleteNamedItem(DaliDB *db, const TransID &tid, 
                                const char *nm);
    static int deleteNamedItem(const TransID &tid,const char *nm);
    static int deleteNamedItem(const char *nm);
    static int deleteNamedItem(DaliDB *db, const char *nm);

    int findNamed(DaliDB *db, const TransID &tid, const char *nm);
    int findNamed(const TransID &tid, const char *nm);
    int findNamed(DaliDB *db,const char *nm);
    int findNamed(const char *nm);

    static ItemID findNamedItem(DaliDB *db, const TransID &tid, const char *nm);
    static ItemID findNamedItem(const TransID &tid, const char *nm);
    static ItemID findNamedItem(DaliDB *db,const char *nm);
    static ItemID findNamedItem(const char *nm);


        /* Get/Put/Access interface */

    int getItem(DaliDB *db, const TransID &tid, char *buf, int size);
    int getItem(const TransID &tid, char *buf, int size);
    int getItem(DaliDB *db, char *buf, int size);
    int getItem(char *buf, int size);

    int putItem(DaliDB *db, const TransID &tid, const char *buf, int size);
    int putItem(const TransID &tid, const char *buf, int size);
    int putItem(DaliDB *db, const char *buf, int size);
    int putItem(const char *buf, int size);

/*
 * Direct Access interface functions 
 */

    int beginAccess(DaliDB* db,  const TransID &trans, LockInfo lock_req) ;
    int beginAccess(const TransID &trans, LockInfo lock_req) ;
    int beginAccess(DaliDB *db, LockInfo lock_req) ;
    int beginAccess(LockInfo lock_req) ;

    int endAccess(DaliDB* db,  const TransID &trans); 
    int endAccess(const TransID &trans) ;
    int endAccess(DaliDB* db);
    int endAccess() ;

    int noteUpdate(const TransID &tid);
    int noteUpdate();
    int noteUpdate(int offset, int length);
    int noteUpdate(const TransID &tid, int offset, int length);

    inline DatabaseID DbId() const { return header.DbId();}
    inline DaliDB *db_params() const ;
    inline DaliDB *db() const ;

 /*
  * We store the unique id in the space normally used by a chunk, since 
  * headers know what chunk they are in.  
  */
    inline short unique_id() const { return header.chunk(); }
    inline void set_unique_id(short id) { header.set_chunk(id); }

    DaliPtr data_DbPtr(const DaliDB *db) const;
    inline void * data_ptr(const DaliDB *db) const;
    inline const void * data_ptrC(const DaliDB *db) const;

    inline ItemHeader *head_ptr(const DaliDB *db) const {
        DALI_D_ASSERT(db != NULL);
        return (ItemHeader*) header.mem_ptr(db); 
    }
    inline ItemHeader *head_ptr() const {
        return (ItemHeader*) header.mem_ptr(); 
    }
    inline int head_offset() const {
        return  header.offset(); 
    }
    inline DbOffset head_db_offset() const {
        return  header.db_offset(); 
    }

    inline DaliPtr data_DbPtr() const;

    inline void * data_ptr() const;
    inline const void * data_ptrC() const;

    
       /* functions to return lock hdrs */
    inline LockSysDBPtr lock_DbPtr(const DaliDB *db=NULL) const;
    LockSysDBPtrDbPtr lock_DbPtrDbPtr(const DaliDB *db=NULL) const;

    inline int data_length(const DaliDB *db=NULL) const;
    inline int set_data_length(int size) const;
    inline int valid() const;
    inline int pre_lock_valid() const;

// FIX: introduce item uniqueness and check for it here!
    inline int operator==(const ItemID & o) const
     { return header == o.header && header.chunk() == o.header.chunk(); }

    inline int operator!=(const ItemID & o) const
     { return header != o.header || header.chunk() != o.header.chunk(); }

    inline int operator!() const
     { return !(header); }

    inline ItemID& operator=(const ItemID& pr2) {
        header=pr2.header;      // copies unique id too
        return *this; 
    };                          
/************* No way to get the DB
    inline ItemID& operator=(ItemHeader *pr2) {
        oid=(void *)pr2;
        return *this; 
    };
***********/


/* I don't understand this, so I'm going to comment it out and
 * see what breaks.  PB 9/7/94
 *
    //inline ItemID operator+ (int i) const;    
    inline friend DaliPtr operator+(int i, const ItemID& pr1) {
         return pr1.data_DbPtr() + i; 
    };                                  
    inline friend DaliPtr operator+(const ItemID& pr1, int i) {
         return pr1.data_DbPtr() + i; 
    };                                  
*/
        /* Item info stored before item */
    
    static int ItemInfoSize;  // Extra space for info about items;
                      // Warning: Must not be changed after items are created!!

    static void set_item_info_size(int i) { ItemInfoSize = i;}
        // Warning:  Above should only be called at system initialization
        // time!  Should NOT be called after any items have been created.
    inline void *item_info() const;     // defined in falloc.h


   inline ItemID typetag() const; 
   inline DbOffset typetag_offset() const; 

   inline int set_typetag(const ItemID &id);
   inline int set_typetag(DbOffset offset); 
   inline void set_header(DaliPtr ptr) { header = ptr; };
   inline const DaliPtr& headerDaliPtr() const { return header; };

    friend class ItemHeader;


};

extern const ItemID ItemID_NULL;


/********************************************************************/

class TransID;
class DaliDB;

class ItemHeader {
protected:
/*  
 * DatabaseID dbid;
 *
 * We can't think of a time when we don't already know this, or could easily
 * know it.  And it saves 4 bytes.
 */
  DbOffset data_offset;    
                // pointer to the data itself;  data is preceded by a backptr
                // to the item header
        // WARNING: We assume that data is in the same database as itemheader.
        //          Eventually need to consider support for non-local data.

  int data_size;  // size of actual data -- does not include backptr    
        // WARNING: Assume no item is of size 0!

  short data_chunk;  // these next two have to be together to save space.
  short uniqueid;    

  DbOffset _typetag_offset;
  //For Ode: offset of item header of the type descriptor of this item 
  //For everyone else:  an extra dboffset associated with an item.

  LockSysDBPtr lock_hdr;   // SysDBPtr of a LockHeader in system database.
                           // The LockHeader must be dynamically allocated.

public:

  inline int data_length() { return data_size; }
  inline int set_data_length(int size) {data_size = size; return 0;}
/* Next function taken outline. */
  DaliPtr data_DbPtr(const DatabaseID &dbid);
  inline void * data_ptr(const DaliDB * db ){
        DALI_D_ASSERT(db != NULL);
        return data_offset.mem_ptr(db);
  }
  inline const void * data_ptrC(const DaliDB * db) const {
        DALI_D_ASSERT(db != NULL);
        return data_offset.mem_ptr(db);
  }

/*  inline DatabaseID mydb() const { return dbid; }  // See earlier assumption  
 */
  inline ItemID myself(DaliDB *db) const;

  inline LockSysDBPtr lock_DbPtr() const {
        return lock_hdr;
  }

  inline LockSysDBPtrDbPtr lock_DbPtrDbPtr(const DaliDB *db) const;

  inline short unique_id() const { return uniqueid; }
  inline void set_unique_id(short id) { uniqueid = id; }


  inline int valid() const { return data_size != 0; }  
                        // Assume size = 0 is invalid ItemID!

  inline int pre_lock_valid(short id) const { return unique_id() == id; }  
                        // Assume size = 0 is invalid ItemID!

  inline void lock_init() {
         lock_hdr = SysDBPtr_NULL;
  } 

  static int createItem(DaliDB *db, const TransID& tid, int chunk_num,
                    int _size, ItemID _typetag, ItemID &iid);
      // Returns a ItemID which references an ItemHeader.  
      // If Typetag is DaliPtr_NULL, set typetag to point to the newly allocated 
      // ItemHeader--used for metatype descriptor since metatype is of type 
      // metatype

   int deleteItem(DaliDB *db, const TransID& tid);

   inline ItemID typetag(DaliDB *db) const;
   inline DbOffset typetag_offset() const;

   inline int set_typetag(const ItemID &id);
   inline int set_typetag(DbOffset offset);

   friend class _pref;
   friend class _Dali_HashNode;
   friend class _Dali_HashIndex;

   void AuditData(const DaliDB *db) const;
   void DebugDisplay(FILE *fp) const;
   static void DebugHeaderList(FILE *fp, DaliDB *db);
};



/********************************************************************/

inline LockSysDBPtr ItemID::lock_DbPtr(const DaliDB *) const { 
        ItemHeader* hdr = head_ptr();
        if(hdr) return hdr->lock_DbPtr();
        else return 0;
}

inline int ItemID::valid() const { 
        ItemHeader* hdr = head_ptr();
        if(hdr) return hdr->valid();
        else return 0;
}

inline int ItemID::pre_lock_valid() const { 
        ItemHeader* hdr = head_ptr();
        if(hdr) return hdr->pre_lock_valid(unique_id());
        else return 0;
}

inline int ItemID::data_length(const DaliDB *) const { 
        ItemHeader* hdr = head_ptr();
        if(hdr) return hdr->data_length();
        else return 0;
}

inline int ItemID::set_data_length(int size) const {

        ItemHeader* hdr = head_ptr();
        if(hdr) return hdr->set_data_length(size);
        else return 0;
}

/**** This guy was demoted out of inline for not compiling.  It is
 **** now at the end of obj_alloc.c.  Dan points out that a better fix
 **** might be to remove it, or to at least take out the parameter.
 ****
inline DaliPtr ItemID::data_DbPtr(const DaliDB *) const { 
****/

inline void * ItemID::data_ptr() const
{
         return data_ptr(header.db_params());
     /******* FIX to improve efficiency ********/
}

inline const void * ItemID::data_ptrC() const 
{
         return data_ptrC(header.db_params());
}

inline void * ItemID::data_ptr(const DaliDB *db) const {
        if (*this==ItemID_NULL) return NULL;
        else { 
            ItemHeader* hdr = head_ptr();
            DALI_D_ASSERT(db != NULL);

            DALI_D_ASSERT(hdr->unique_id() == unique_id());

            if(hdr) return hdr->data_ptr(db);
            else return NULL;
        }
}

inline const void * ItemID::data_ptrC(const DaliDB *db) const {
        if (*this==ItemID_NULL) return NULL;
        else {
            ItemHeader* hdr = head_ptr();

            DALI_D_ASSERT(db != NULL);
            DALI_D_ASSERT(hdr->unique_id() == unique_id());
    
            if(hdr) return hdr->data_ptrC(db);
            else return NULL;
        }
}

inline void ItemID::fix_uniqueid() {
        if (header != DaliPtr_NULL) 
            set_unique_id(head_ptr()->unique_id());
}

DbOffset ItemHeader::typetag_offset() const { 
     return _typetag_offset;
}

int ItemHeader::set_typetag(const ItemID &id) {
     _typetag_offset = id.header.offset(); 
     return 0;
}

int ItemHeader::set_typetag(DbOffset offset)    {
     _typetag_offset = offset; 
     return 0;
}

ItemID ItemID::typetag() const {
     return head_ptr()->typetag(db_params());
}

DbOffset ItemID::typetag_offset() const { 
     return head_ptr()->typetag_offset();
}

int ItemID::set_typetag(const ItemID &id)   {
     head_ptr()->set_typetag(id.header.offset()); 
     return 0;
}

int ItemID::set_typetag(DbOffset offset)    {
     head_ptr()->set_typetag(offset); 
     return 0;
}

inline DaliPtr ItemID::data_DbPtr() const {
     return data_DbPtr(header.db_params());
};
/********************************************************************/
/********************************************************************/
#endif 
        // _DALI_ITEM_MGR_H

