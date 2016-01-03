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

#ifndef _TEMPLATE_H
#define _TEMPLATE_H

  /* Above a hack for old version of Ode compiler which doesn't understands
        templates
  */

/*
    Template class  DaliPtr_of<Type> is used to create database pointers to
        objects of class <Type>.  Note that Type must be a class and not
        a primitive type.
*/

extern const DaliDB * DaliDB_NULL;

template <class Type> class DaliPtr_of : public DaliPtr {
  public:
    inline DaliPtr_of() {}                                                   
    inline DaliPtr_of(DaliPtr t) { *((DaliPtr*)this)=t; }                         
    inline DaliPtr_of(int t) : DaliPtr(t) {}
    inline DaliPtr_of(const DaliDB *db, Type *v) : DaliPtr(db, (void*) v) {}       

    inline Type *mem_ptr(const DaliDB *db) const {
        return (Type*) ((DaliPtr*)this)->mem_ptr(db);                
    }
    inline Type *mem_ptr() const {
        return (Type*) ((DaliPtr*)this)->mem_ptr();                
    }
    inline Type *mem_ptr(DbStartData *db) const {                           
        return (Type*) ((DaliPtr*)this)->mem_ptr(db);                
    }
/** Another inline, leaving so it will compile.
 **/
    inline DaliPtr_of<Type> operator+ (int i) const;                        
    inline Type& operator* () const { return *mem_ptr();}                   
    /* inline Type* operator& () const  { return mem_ptr();}  */
    /*
    typedef Type * TypePtr;
    inline TypePtr() const { return mem_ptr();}                    
    */
    inline Type* operator-> () const { return mem_ptr();}                   
    inline operator Type* () const { return mem_ptr();}                    
#ifdef DALI_OLD_ASSIGN
    inline DaliPtr_of<Type>& operator=(const DaliPtr& pr2) { 
        dbid = pr2.dbid; mptr = pr2.mptr; return *this; 
    }
#endif

    inline friend DaliPtr_of<Type> operator+ (int i, 
                                const DaliPtr_of<Type> & pr1) 
      { return pr1 + i;}

    inline int operator! () const {
        return ! *((DaliPtr *) this);
    }
};

template <class Type> class SysDBPtr_of : public SysDBPtr {
  public:
    inline SysDBPtr_of() {}                                                   
    inline SysDBPtr_of(SysDBPtr t) { *((SysDBPtr*)this)=t;}                         
    inline SysDBPtr_of(int t) : SysDBPtr(t) {}
    inline SysDBPtr_of(Type *v) : SysDBPtr((void*) v) {}       
    inline SysDBPtr_of(const DaliDB *, Type *v) : SysDBPtr((void *)v) {}

    inline Type *mem_ptr() const {
        return (Type*) ((SysDBPtr*)this)->mem_ptr();                
    }
    SysDBPtr_of<Type> operator+ (int i) const;                      //was inline
    inline Type& operator* () const { return *mem_ptr();}                   
#ifdef DALI_OLD_ASSIGN
    inline SysDBPtr_of<Type>& operator =(const SysDBPtr& pr2){
        mptr = pr2.mptr;
        return *this;
    }
#endif
  
    inline Type* operator-> () const { return mem_ptr();}                   
    inline operator Type* () const { return mem_ptr();}                    
    inline friend SysDBPtr_of<Type> operator+ (int i, const SysDBPtr_of<Type>& pr1) 
                 { return pr1 + i; }                                        
    inline int operator! () const {
        return ! *((SysDBPtr *) this);
    }
};

/*************************************/

template <class Type> class DbOffset_of : public DbOffset {
  public:
    inline DbOffset_of() {}                                                   
    inline DbOffset_of(DbOffset t) { *((DbOffset*)this)=t;}                         
    inline DbOffset_of(int t) : DbOffset(t) {}
    inline DbOffset_of(const DaliDB *db, Type *v) :
                 DbOffset(db, (void*) v) {}       
    inline DbOffset_of(const DatabaseID &db, Type *v) :
                 DbOffset(db, (void*) v) {}       

    inline Type *mem_ptr(const DaliDB *db) const {
        return (Type*) ((DbOffset*)this)->mem_ptr(db);                
    }
    inline Type *mem_ptr(DbStartData *dbs) const {
        return (Type*) ((DbOffset*)this)->mem_ptr(dbs);                
    }
    inline Type *mem_ptr(const DatabaseID& dbid) const {
        return (Type*) ((DbOffset*)this)->mem_ptr(dbid);                
    }
#ifdef DALI_OLD_ASSIGN
    inline DbOffset_of<Type>& operator =(const DbOffset& pr2){
        mptr = pr2.mptr;
        return *this;
    }
#endif
    DbOffset_of<Type> operator+ (int i) const;                      
    inline friend DbOffset_of<Type> operator+ (int i, const DbOffset_of<Type>& pr1) 
                 { return pr1 + i; }                                        
};

// FIX:  the definitions for the member functions of this class
// need to be placed somewhere after the DaliSys declaration, but
// the solaris compiler has rules about where template code can go
// for it to be found.  falloc.c, where it was, isn't an option.
// We probably will need a template.c, though it may work to put
// it in dalisys.  Commenting it out for now, since it isn't used.


/**************** A Generic Stack Implementation **************/

class DaliLinkedListItem{
public:
  DaliLinkedListItem *nxt; 
  /* val */
  static DaliLinkedListItem *allocate(int size)
    {
      DaliLinkedListItem *lliptr = (DaliLinkedListItem *)malloc(
                                              sizeof(DaliLinkedListItem)+size);
      return lliptr;
    }
  inline void *valptr()
    {
      return (void *)((char *)this + sizeof(DaliLinkedListItem));
    }
};

template <class Type>
class DaliStack {
public:
  inline DaliStack() { head = NULL;}
  inline void init() { head = NULL;}
  inline void insert(Type);
  inline Type remove();
  inline int is_empty() { return head == NULL;}
private:
  DaliLinkedListItem *head;
};

#if MEM_MGR_FIXED
/************ SysDB Mem Manager Functions **********************/

/** the following class allocates a chunk of memory in sysdb and **
 ** manages it - thus avoiding the overhead of fallocing and     **
 ** obtaining latches every time memory needs to be malloc'd     **/

#define DEFAULT_CHUNK_SIZE 1000

template <class TypeName, class PtrType> class Mem_Mgr {
private:
   PtrType free_list_head;
   DatabaseID dbid;
   int seg_no;
   int chunk_size;
   int new_chunk();
 public:
   Mem_Mgr(){}
   void init(DatabaseID _dbid, int _seg_no, int _size = DEFAULT_CHUNK_SIZE);
   PtrType alloc();
   int free(PtrType);
// Should be private:  !!
};

template <class TypeName, class PtrType> inline void Mem_Mgr<TypeName, PtrType>::
init(DatabaseID _dbid, int _seg_no, int _size)
{
   dbid = _dbid;
   seg_no = _seg_no;
   chunk_size = _size;
   free_list_head = NULL;
};    


/******* Mem_Mgr Functions *****************************************/

/** the following functions allocate and free memory in a chunk  ***/   

template <class TypeName, class PtrType> int 
   Mem_Mgr<TypeName, PtrType>::new_chunk(){
      DaliDB *db = DaliSys::mapinfo.db_params(dbid);
      free_list_head = f_alloc(db, TransID_NULL, chunk_num,
                sizeof(TypeName)*chunk_size, _DontLogAction);
      PtrType i, *k;
      int j;
      for(i = free_list_head, j=0; j< chunk_size; j++){
         k= (PtrType *)i.mem_ptr();
         i = i+1;
         *k = i;
      }         
      *k = NULL;
      return 0;
}

template <class TypeName, class PtrType> PtrType 
        Mem_Mgr<TypeName, PtrType>::alloc()
{
      if (!free_list_head){
         new_chunk();
      }
      PtrType temp = free_list_head;
      free_list_head = *((PtrType *)temp.mem_ptr());
      return temp;
}    

template <class TypeName, class PtrType> int 
        Mem_Mgr<TypeName, PtrType>::free(PtrType block)
{
      PtrType *i = (PtrType *) block.mem_ptr();
      *i = free_list_head;
      free_list_head = block;
      return 0;
}    

#endif
/*************************************************************************/

/**** Following definitions were brought here from template.c   ******/

template <class Type> DbOffset_of<Type>
inline DbOffset_of<Type>::operator+ (int i) const {
    DbOffset temp = *this;
    temp = temp + (i * sizeof(Type));
    DbOffset_of<Type> tmp1(temp);
    return tmp1;
}


template <class Type> SysDBPtr_of<Type>
inline SysDBPtr_of<Type>::operator+ (int i) const {
    SysDBPtr temp = *this;
    temp = temp + (i * sizeof(Type));
    SysDBPtr_of<Type> tmp1(temp);
    return tmp1;
}


template <class Type> DaliPtr_of<Type>
inline DaliPtr_of<Type>::operator+ (int i) const {
    DaliPtr temp = *this;
    temp = temp + (int) (i * sizeof(Type));
    DaliPtr_of<Type> tmp1(temp);
    return tmp1;
}


inline DaliPtr::DaliPtr(const DatabaseID &id, int _mptr) 
{
     if(!_mptr) dbid = DatabaseID_NULL; else dbid = id;
     mptr = _mptr; 
}

inline DaliPtr::DaliPtr(const DatabaseID &_dbid, const DbOffset& offset) 
{

        if (!offset.offset()) dbid = DatabaseID_NULL; else dbid = _dbid;
        mptr = offset.offset();
}

inline int operator==(const DaliPtr& pr1,const DaliPtr& pr2) {
 return (pr2.mptr == 0) ? (pr1.mptr == 0 || pr1.dbid.local_id() == 0)
           : ((pr1.mptr==pr2.mptr) && (pr1.dbid == pr2.dbid)); 
}

inline DaliPtr operator+ (const DaliPtr& pr1, int i) {
        return  DaliPtr(pr1.dbid, pr1.mptr +i);
}
inline DaliPtr operator+ (int i, const DaliPtr& pr1) {
        return  DaliPtr(pr1.dbid, pr1.mptr +i);
}
 


/**************Stack Functions ******************************/

template <class Type>
inline void DaliStack<Type>:: insert(Type val)
{
  DaliLinkedListItem *llptr = DaliLinkedListItem::allocate(sizeof(Type));
  llptr->nxt = head;
  head = llptr;
  *(Type *)llptr->valptr() = val;
}

template <class Type>
inline Type DaliStack<Type>::remove()
{
  if(head == NULL){
    Type null_type;
    return null_type;
    //FIX: will only work correctly with classes that have a null constructor!!
  }
  else{
    DaliLinkedListItem *llptr = head;
    head = head->nxt;
    Type retval = *(Type *)llptr->valptr();
    free((void *)llptr);
    return retval;
  }
}

/******************** Hash Index Template Class *********************/

#define NUM_HASH_BUCKETS 1000
enum ComparisonOp {EQ, NEQ, LT, GT, GEQ, LEQ, UNDEF};

template <class Type> class DaliHashIndex;
template <class Type> class DaliHashIter;

template <class Type>
class DaliHashNode {
  Type val;
  DbOffset next;
  friend DaliHashIndex<Type>;
  friend DaliHashIter<Type>;
};

class LockInfo;

/****************************************************************
 * Lock levels for indexing and heap file.
 ****************************************************************/

enum DaliLockLevel { 
    DALILL_NONE,                 // Locks have been obtained above
    DALILL_SHORT,                // Only valid for open scan: release
                                 // previous lock on getNext()
    DALILL_SHORT_MANUAL,         // User may release lock manually
    DALILL_LONG,                 // Commit duration lock will be obtained
    DALILL_LONG_HANDLE_PHANTOM   // Same as long, plus some efforts 
                                 // will be taken to handle phantoms --
                                 // such as next key or bucket locking.
};

/**************************************************************/

#define DaliHashIterNodeSysDBPtr SysDBPtr_of<DaliHashIterNode<Type> > 

template <class Type> 
class HashParams{ /** parameters logged when performing inserts/deletes on 
                    * hash table */
  int hash_type;  /* WARNING: this should be the first element of this class */
                  /* hash_insert/delete_undo depend on it */
  DaliPtr hash_ptr;
  DaliLockLevel lock_level;
  Type val;
  friend class DaliHashIndex<Type>;
  friend int hash_insert_undo(const TransID& tid, DaliTransTimeStamp _TS, 
                     DaliLogOpCode opcode, void *params, int params_len);
  friend int hash_delete_undo(const TransID& tid, DaliTransTimeStamp _TS, 
                     DaliLogOpCode opcode, void *params, int params_len);
};

template <class Type>
class DaliHashIndex {
  int num_buckets;
  DbOffset hash_bucket_offset;
  int size;
  int chunk_num;
  int must_obtain_locks;
  DbOffset lock_table_offset;
  int num_lock_buckets;
  DbOffset current_scans_ptr;
  DbOffset current_scans_mutex;
  int is_unique_index;
  int hash_type;

public:
//  typedef SysDBPtr_of<DaliHashIterNode<Type> > DaliHashIterNodeSysDBPtr;
  DatabaseID db_id;

  DaliHashIndex<Type>(){num_buckets=NUM_HASH_BUCKETS; 
                        db_id = DatabaseID_NULL; size = 0;
                        hash_bucket_offset = DbOffset_NULL;}

  INLINE_HASH static DaliPtr create(DaliDB *db, TransID tid, void *ptr, int size, 
        int hash_type = 0, int chunk_num = NULL, int must_lock = 1, 
        int is_unique_index = 0, int _num_buckets = NUM_HASH_BUCKETS);

  INLINE_HASH static int destroy(TransID tid, DaliPtr h_daliptr);

  INLINE_HASH int insertKey(TransID tid, Type el,
                               DaliLockLevel lock_level = DALILL_LONG_HANDLE_PHANTOM,
                               Type **typePtr = NULL);

  INLINE_HASH int deleteKey(TransID tid, Type el, DaliLockLevel 
                    lock_level = DALILL_LONG_HANDLE_PHANTOM);

  INLINE_HASH int deleteAll(TransID tid);
  INLINE_HASH int hash_insert_undo(const TransID &tid, HashParams<Type> *hash_params);
  INLINE_HASH int hash_delete_undo(const TransID &tid, HashParams<Type> *hash_params);
  INLINE_HASH Type *HashInsert(TransID tid, Type el);    
  INLINE_HASH int HashDelete(TransID tid, DaliOpID op_id, Type el);

  INLINE_HASH DaliHashIter<Type> *initScan(TransID tid, ComparisonOp cmp_op, 
         void *key_val, DaliLockLevel lock_level=DALILL_LONG_HANDLE_PHANTOM);
  INLINE_HASH void closeScan(TransID tid, DaliHashIter<Type> *);
  INLINE_HASH Type *find(void *key_val);
  INLINE_HASH Type *find(Type key_info);
  INLINE_HASH void *void_data(){
     return (void *)((char *)this + sizeof(DaliHashIndex<Type>));
       }
  INLINE_HASH DaliHashIter<Type> *alloc_iter(TransID tid, DaliHashIter<Type> *iter);
  INLINE_HASH void adjust_iters(DbOffset _offset);
  INLINE_HASH void de_alloc_iter(TransID tid, DaliHashIter<Type> *iter);
  friend class DaliHashIter<Type>;
};

template <class Type>
class DaliHashIter {
  void *key_val;
  DaliHashIndex<Type> *hash_ptr;
  DbStartData * db_start;
  DbOffset hash_node_offset;
  DaliLockLevel lock_level;
  TransID tid;
  int first_invocation, bucket;
  Type *ptr_to_val;
 public:
  INLINE_HASH DaliHashIter<Type> *init(TransID tid, DaliHashIndex<Type> *_hash_ptr, 
                   void *_key_val, DaliLockLevel lock_level);
  INLINE_HASH int next(Type ** _ptr_to_val);
  INLINE_HASH DaliHashIter<Type> * copy();
  // for cfront
  friend class DaliHashIndex<Type>;

};


template <class Type>
class DaliHashIterNode {
  typedef SysDBPtr_of<DaliHashIterNode<Type> > DaliHashIterNodeSysDBPtr1;
  DaliHashIter<Type> iter;
  DaliHashIterNodeSysDBPtr1 nxt;
  friend class DaliHashIndex<Type>;
};


#endif 
      // _TEMPLATE_H

