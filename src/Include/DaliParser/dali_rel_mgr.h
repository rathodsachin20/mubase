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
#ifndef REL_MGR_H
#define REL_MGR_H
#include "dali.h"
//#include "ttree.h"
#include "reltypes.h"
#include "tuple_ptr.h"
//#include "hf.h"
#include "catalog.h"
#include "nonpershash.h"
#include "list.h"

#define MAX_NAME_LENGTH 64
#define MAX_ATTR_NAME_LENGTH 64
#define CATALOG_DB_NAME "CatalogDB"
#define NUM_BITS_IN_CHAR 8
#define MAX_ATTR_PRINT_LEN 500


enum DaliIndexType{ 
    DALI_HASH, DALI_TTREE, DALI_HEAPFILE, DALI_UNIQUE_TTREE, DALI_UNIQUE_HASH, DALI_OTM
};

class RELCAT;
class ATTRCAT;
class INDEXCAT;
class INDEXATTRCAT;
class VIEWCAT;
class VIEWATTRCAT;
class OTMCAT;
class OTMATTRCAT;
class REFCAT;
class REFATTRCAT;

typedef CatalogDBPtr_of<ATTRCAT> ATTRCATCatalogDBPtr;
typedef CatalogDBPtr_of<RELCAT> RELCATCatalogDBPtr;
typedef CatalogDBPtr_of<INDEXCAT> INDEXCATCatalogDBPtr;
typedef CatalogDBPtr_of<INDEXATTRCAT> INDEXATTRCATCatalogDBPtr;
typedef CatalogDBPtr_of<VIEWCAT> VIEWCATCatalogDBPtr;
typedef CatalogDBPtr_of<VIEWATTRCAT> VIEWATTRCATCatalogDBPtr;
typedef CatalogDBPtr_of<OTMCAT> OTMCATCatalogDBPtr;
typedef CatalogDBPtr_of<OTMATTRCAT> OTMATTRCATCatalogDBPtr;
typedef CatalogDBPtr_of<REFCAT> REFCATCatalogDBPtr;
typedef CatalogDBPtr_of<REFATTRCAT> REFATTRCATCatalogDBPtr;

class RELCAT{
    public:
/* Warning: if you add or remove a field, update NUM_ATTRS below */
    char relName[MAX_NAME_LENGTH];
    int internalLength;
    int externalLength;
    int numAttrs;
    HFDbPtr HF_File;
    DaliPtr lock_ptr;
    int numIndxs;
    int numFromOTM;
    int numToOTM;
    int numFromRef;
    int numToRef;
    int maxAlignFactor;

    enum {NUM_ATTRS = 12}; 
    friend ostream & operator << (ostream &, const RELCAT &);
};


class ATTRCAT{
    public:
/* Warning: if you add or remove a field, update NUM_ATTRS below */
    char relName[MAX_NAME_LENGTH];
    char attrName[MAX_NAME_LENGTH];
    int offset;
    int length;
    DaliAttrType type;
    int position_in_rel;

    enum {NUM_ATTRS = 6}; 
    friend ostream & operator << (ostream &, const ATTRCAT &);
};


class INDEXCAT{
    public:
/* Warning: if you add or remove a field, update NUM_ATTRS below */
    char relName[MAX_NAME_LENGTH];
    char indexName[MAX_NAME_LENGTH];
    DaliIndexType type;
    DaliPtr Index_File;
    int numAttrs;

    enum {NUM_ATTRS = 5}; 
    friend ostream & operator << (ostream &, const INDEXCAT &);
};

class INDEXATTRCAT{
    public:
/* Warning: if you add or remove a field, update NUM_ATTRS below */
    INDEXCATCatalogDBPtr indexId;
    char relName[MAX_NAME_LENGTH];
    char attrName[MAX_NAME_LENGTH];
    char indexName[MAX_NAME_LENGTH];
    int position_in_index;

    enum {NUM_ATTRS = 5}; 
    friend ostream & operator << (ostream &, const INDEXATTRCAT &);
};

class VIEWCAT {
public:
/* Warning: if you add or remove a field, update NUM_ATTRS below */
    char viewName[MAX_NAME_LENGTH];
    char relName[MAX_NAME_LENGTH];
    int length;
    int numAttrs;

    enum {NUM_ATTRS = 4}; 
    friend ostream & operator << (ostream &, const VIEWCAT &);
};

class VIEWATTRCAT {
public:
/* Warning: if you add or remove a field, update NUM_ATTRS below */
    char viewName[MAX_NAME_LENGTH];
    char attrName[MAX_NAME_LENGTH];
    int position_in_view;

    enum {NUM_ATTRS = 3}; 
    friend ostream & operator << (ostream &, const VIEWATTRCAT &);
};

class OTMCAT{
 public:
  char OTMName[MAX_NAME_LENGTH];
  char fromRelName[MAX_NAME_LENGTH];
  char toRelName[MAX_NAME_LENGTH];
  int numAttrs;
  int offset;
  int oneToManyFlag;

  enum {NUM_ATTRS = 6};
  friend ostream & operator << (ostream &, const OTMCAT &);
};

class OTMATTRCAT{
 public:
  OTMCATCatalogDBPtr OTMId;
  char fromRelName[MAX_NAME_LENGTH];
  char fromAttrName[MAX_NAME_LENGTH];
  char toRelName[MAX_NAME_LENGTH];
  char toAttrName[MAX_NAME_LENGTH];

  enum {NUM_ATTRS = 5};
  friend ostream & operator << (ostream &, const OTMATTRCAT &);
};

class REFCAT{
 public:
  char fromRelName[MAX_NAME_LENGTH];
  char toRelName[MAX_NAME_LENGTH];
  int numAttrs;

  enum {NUM_ATTRS = 3};
  friend ostream & operator << (ostream &, const REFCAT &);
};

class REFATTRCAT{
 public:
  REFCATCatalogDBPtr refId;
  char fromRelName[MAX_NAME_LENGTH];
  char fromAttrName[MAX_NAME_LENGTH];
  char toRelName[MAX_NAME_LENGTH];
  char toAttrName[MAX_NAME_LENGTH];

  enum {NUM_ATTRS = 5};
  friend ostream & operator << (ostream &, const REFATTRCAT &);
};

typedef DaliHashIndex<tuplePtr> hash_index_on_tuples;
typedef DaliPtr_of<hash_index_on_tuples> hash_index_on_tuplesDbPtr;

typedef DaliTTreeIndex<tuplePtr> ttree_index_on_tuples;
typedef DaliPtr_of<ttree_index_on_tuples> ttree_index_on_tuplesDbPtr;

class CatalogMgr {
    public:
    static DaliDB *catalog_db;
    static DatabaseID catalog_id;
    
    static int Init(TransID tid);
    static int ReInit(TransID tid);
    static int dumpCatalogForRelation(TransID &tid, char *name);
};

inline CatalogDBPtr::CatalogDBPtr(void *_mptr) {
    mptr = (char *)_mptr - CatalogMgr::catalog_db->start_addr();
}

inline DaliPtr CatalogDBPtr::dali_ptr(){
    return DaliPtr(CatalogMgr::catalog_db, mptr);
}

inline void *CatalogDBPtr::mem_ptr() { 
    return (void *) (CatalogMgr::catalog_db->start_addr()+ mptr);
    
}


enum DaliRefMethod {
    DREL_UNKNOWN_METHOD = -1,
    DREL_COPY_BUF, DREL_COPY_FIELDS,
    DREL_POINT_TUPLE, DREL_POINT_FIELDS,
    DREL_TYPED_POINT_FIELDS // Typed pointers..
};

class DaliTableHandle;
class DaliAttrList;
class DaliAttrValList;
class DaliSearchCriteria;
class DaliSearchPlan;
class DaliRelInternal;


struct DaliInternalAttr {

        /*kvr - This structure is used to represent one attribute in 
          DaliRelInternal or DaliViewInternal, which themselves are used 
          as caches for information about open tables and open views
          respectively. */

    ATTRCAT *attrcat; /* kvr- pointer to tuple in ATTRCAT. In the case
          of views, attrcat will point to the base table's ATTRCAT entry. */
    int offsetInTarget; /* kvr- In case of views, offset inside view's 
          tuple. attrcat->offset will then give offset inside base table's tuple. 
          In case of base tables, this will be the same as attrcat->offset. */
    int view_position;
    int alpha_position; /* kvr- these two indices seem to be 
          represent the same thing, i.e. position in tuple. Then why have 
          both? */
    int position_in_relInternal;

    /*
     * the participating_indexes is a list of index numbers
     * which this attribute participates in.
     * It is intended to allow fast creation of a bit vector
     * representing the indexes affected by an update.
     */
    int participating_index_count;
    unsigned char *participating_indexes;

    int participating_from_OTM_count;
    int participating_to_OTM_count;
    unsigned char *participating_from_OTMs;
    unsigned char *participating_to_OTMs;

    int participating_from_ref_count;
    int participating_to_ref_count;
    unsigned char *participating_from_refs;
    unsigned char *participating_to_refs;

    DaliInternalAttr(){ participating_indexes = NULL; 
                        participating_from_OTMs = NULL;
                        participating_to_OTMs = NULL;
                        participating_from_refs = NULL;
                        participating_to_refs = NULL;
                      }
    ~DaliInternalAttr(){ delete [] participating_indexes;
                        delete [] participating_from_OTMs;
                        delete [] participating_to_OTMs;
                        delete [] participating_from_refs;
                        delete [] participating_to_refs;
                      }
    
    int formatValue(const void *tuple, char *buf, int maxLen);

    friend ostream & operator << (ostream &, const DaliInternalAttr &);

};


struct DaliInternalIndex {
    INDEXCAT *indexcat;
    DaliIndexType type;
    void *index;
    int position_in_relInternal;
    int attrCount;
    INDEXATTRCAT **attrList; // Sorted by position_in_index
    int *attrPosition;

    DaliInternalIndex(){attrList = NULL; attrPosition = NULL;}
    ~DaliInternalIndex(){ delete []attrList; delete [] attrPosition;}
    friend ostream & operator << (ostream &, const DaliInternalIndex &);
};


class DaliAttrInfo {
/*
 * Warning: This class caches a pointer to the internal information for
 * the attribute.  Anywhere this class is embeded, care should be taken
 * to provide a mechanism to determine when these pointers (ainfo) are
 * invalid.  Typically, if the AttrInfo is used with a different table,
 * the ainfo pointer is invalidated.
 */
public:
    DaliAttrInfo(const char *in_name) {
        strcpy(name, in_name); ainfo = 0;
    }
    DaliAttrInfo(const char *in_name, DaliRelInternal *rel_info);
    DaliAttrInfo() : ainfo(0) { name[0] = 0;}
    
    char name[MAX_NAME_LENGTH];
    DaliInternalAttr *ainfo;
};

class DaliAttrVal : public DaliAttrInfo{
public:
    DaliVal val;
    DaliAttrVal() : DaliAttrInfo(0) { };
    DaliAttrVal(char *_name, int _val) : DaliAttrInfo(_name), val(_val){ };
    DaliAttrVal(char *_name, float _val) : DaliAttrInfo(_name), val(_val){ };
    DaliAttrVal(char *_name, char *_val) : DaliAttrInfo(_name), val(_val){ };
    DaliAttrVal(char *_name, double _val) : DaliAttrInfo(_name), val(_val){ };
    DaliAttrVal(char *_name, DaliDate _val) : DaliAttrInfo(_name), val(_val){ };
    DaliAttrVal(char *_name, DaliTime _val) : DaliAttrInfo(_name), val(_val){ };
    DaliAttrVal(char *_name, const daliNULL &) : DaliAttrInfo(_name), val() { };
    DaliAttrVal(char *_name, void *_val, int sz) : DaliAttrInfo(_name), val(_val, sz){ };

    DaliAttrVal & operator = (const DaliAttrVal &);

    int destroy(){return val.destroy();}
    ~DaliAttrVal(){destroy();}
};

class DaliAttrList{
public:
    DaliAttrList() { 
      currTable = NULL;
      currTableTimestamp = -1;
      affected_idxs = NULL;
      affected_from_OTMs = NULL;
      affected_to_OTMs = NULL;
      affected_from_refs = NULL;
      affected_to_refs = NULL;

    }
    void addAttr(char *);
    friend DaliAttrList& operator << (DaliAttrList &, char *);
    char *affected_idxs;
    char *affected_from_OTMs;
    char *affected_to_OTMs;
    char *affected_from_refs;
    char *affected_to_refs;
    // need a reset/reinit type function here...
    void reset();
    void destroy() {attr_list.destroy();};
    ~DaliAttrList(){
    delete affected_idxs;
    delete affected_from_OTMs;
    delete affected_to_OTMs;
    delete affected_from_refs;
    delete affected_to_refs;
    destroy();}
    int contains(char *name);
protected:
    DaliRelInternal *currTable; // to detect when tables have chagned.
        int currTableTimestamp;
    List<DaliAttrInfo> attr_list;
    // int determineIdxs(DaliRelInternal *info); in DaliRelInternal.
    friend class DaliAttrListIter;
};


typedef Listiter<DaliAttrInfo> DaliAttrInfoIter;

class DaliAttrListIter : public DaliAttrInfoIter {
public:
  DaliAttrListIter(DaliAttrList &alist, DaliRelInternal *cTable = 0);
  int next(DaliAttrInfo *(&aptr));
    /* If a non-null hTable has been passed on creation, the DaliAttrInfo */
    /* pointed to by aptr on a >0 return will have the ainfo filled in to */
    /* be the corresponding DaliInternalAttr record from DaliRelInternal */
    /* for hTable.  If not found, DALI_NO_ATTR will be returned. */

  protected:
    int sameTable;
    int tableTimestamp;
    DaliRelInternal *currTable; // to detect when tables have chagned.
};


/* Consider making AttrValList a subtype of AttrList.  This would */
/* require parallel lists, but would allow a lot more code to be */
/* factored out. */

class DaliAttrValList{
 public:
  DaliAttrValList(){termcount =0; termmax =0; attr_val_list = NULL;
                     currTable = NULL;};
  void addTerm(char *name, int);
  void addTerm(char *name, float);
  void addTerm(char *name, double);
  void addTerm(char *name, DaliDate);
  void addTerm(char *name, DaliTime);
  void addTerm(char *name, char *);
  void addTerm(char *name, void *, int sz);
  void addTerm(char *name, const daliNULL &);
  int addTerm(DaliAttrVal &attr_val);
  int reset();
  int destroy();
  ~DaliAttrValList(){destroy();}
  friend DaliAttrValList& operator << (DaliAttrValList &, char *);
  friend DaliAttrValList& operator << (DaliAttrValList &, int);
  friend DaliAttrValList& operator << (DaliAttrValList &, float);
  friend class DaliAttrValListIter;
 protected:
    enum ListState {NEED_ATTR, NEED_VAL, LIST_ERROR} flag;
                   /* stores state info */
    DaliAttrVal *attr_val_list;
    DaliRelInternal *currTable; // to detect when tables have chagned.

    int termcount;
    int termmax;
};


class DaliAttrValListIter {
public:
  DaliAttrValListIter(DaliAttrValList &alist, DaliRelInternal *cTable = 0);
  int at_end();
  int next(DaliAttrVal *(&aptr));
    /* If a non-null hTable has been passed on creation, the DaliAttrInfo */
    /* pointed to by aptr on a >0 return will have the ainfo filled in to */
    /* be the corresponding DaliInternalAttr record from DaliRelInternal */
    /* for hTable.  If not found, DALI_NO_ATTR will be returned. */
  void reset();

  protected:
    int sameTable;
    DaliAttrValList *avlist;
    int current;
    
};



class DaliTableIter;
class DaliTableSchema;
class DaliTupleHandle;
class DaliInternalOTMRef;

class DaliRelInternal {
  public:
    int opentimestamp;
    int opencount;
    RELCAT *relInfo; 
    // LockSysDBPtrDbPtr *lockHeader;
    // lockTypes currentLock; 
    DaliDB *db;
    int numAttrs;    // cached from *rel
    int length;      // cached from *rel
//    HF *hfPtr;
    DaliInternalAttr *attrList;  // sorted in lexicographic order
    int numIndxs;
    DaliInternalIndex *affectedIndxs;  
      // what indexes are affected by an update here?
      // on an insert, if allowed, use baserel->affected_idxs if defined.


    SavePoint savePoint;
    tuplePtr tuple_being_inserted;
    char *affected_indxs;

    /*** OTM info cached in Rel Internal ***/
    DaliInternalOTMRef *fromOTMList;
    DaliInternalOTMRef *toOTMList;
    int numFromOTM;
    int numToOTM;
    char *affected_from_OTMs;
    char *affected_to_OTMs;

    /*** Ref info cached in Rel Internal ***/
    DaliInternalOTMRef *fromRefList;
    DaliInternalOTMRef *toRefList;
    int numFromRef;
    int numToRef;
    char *affected_from_refs;
    char *affected_to_refs;

    /* Small utility functions */
    DaliInternalAttr *lookupAttr(const char *name);


//    HF *hfptr() {return relInfo->HF_File;}

    virtual char *name() { return relInfo->relName; }
    char *affectedIndexes() { return affected_indxs; }
    char *affectedFromOTMs() { return affected_from_OTMs; }
    char *affectedToOTMs() { return affected_to_OTMs; }
    char *affectedFromRefs() { return affected_from_refs; }
    char *affectedToRefs() { return affected_to_refs; }

    int init_affected_indexes_and_OTMs_refs(DaliInternalAttr &);

    virtual int baseIndxCount() { return numIndxs;}    

    virtual int baseFromOTMCount() { return numFromOTM;}    
    virtual int baseToOTMCount() { return numToOTM;}    
    virtual int baseFromRefCount() { return numFromRef;}    
    virtual int baseToRefCount() { return numToRef;}    

    virtual DaliRelInternal *baseRelInfo() { return this;}    

    /* Large utility functions */
    int insert(const TransID &tid, DaliRefMethod ref, tuplePtr &new_tup,
               void *buf, DaliAttrList *alist = NULL);
    int beginInsert(const TransID &tid, tuplePtr &new_tup);
    int endInsert(const TransID &tid, tuplePtr new_tup, 
                  DaliAttrList *alist = NULL);
    int update(const TransID &tid, DaliRefMethod ref, tuplePtr tup, 
               void *buf, DaliAttrList *alist = 0);
    int update(const TransID &tid, DaliRefMethod ref, tuplePtr tup, 
               DaliAttrValList &update_list);
    int beginUpdate(const TransID &tid, tuplePtr tup, 
                    DaliAttrList *alist = 0);
    int endUpdate(const TransID &tid, tuplePtr tup,
                  DaliAttrList *alist = 0);

    /* Versions of the above functions with AttrValLists as parameters */

    int insert(const TransID &tid, 
               DaliRefMethod ref,
               tuplePtr &new_tup, 
               void *buf, DaliAttrValList &list, 
               DaliAttrList *alist = NULL);
    int beginInsert(const TransID &tid,tuplePtr &new_tup, 
                    DaliAttrValList &list);
                    
    int endInsert(const TransID &tid, tuplePtr new_tup, 
                  DaliAttrValList &vlist,
                  DaliAttrList *alist = NULL); 

    int beginUpdate(const TransID &tid, tuplePtr tup, DaliAttrValList &, 
                    DaliAttrList *touchdAttrs = NULL);
    /* beginupdate actually changes the touchdAttrs  to include those 
        from the attrvallist */
    /* therefore endUpdate not needed */
    int update(const TransID &tid, 
               DaliRefMethod refMethod,
               tuplePtr tup, 
               void *buf, DaliAttrValList &, 
               DaliAttrList *touchdAttrs = NULL);


    virtual int deleteTuple(const TransID &tid, tuplePtr tup);

    int formatTuple(const void *tup, char *buf, int maxLen);
    int formatTuple(const void *tup, char *buf, int maxLen, 
                    DaliAttrList &alist);

    int formatHeaders(char *buf, int maxLen);
    int formatHeaders(char *buf, int maxLen, 
                    DaliAttrList &alist);

    virtual int cacheRelation(const TransID &tid);
    /*
     * causes all information for relName to be looked up from the 
     * catalog and cached in this structure.
     */

    // subsetAttrs check to make sure that the attr list of index
    // is a subset of the attr list of fromOTM

    virtual int subsetAttrs(DaliInternalIndex* index,
                            DaliInternalOTMRef& fromOTM);

    virtual int canInsert() { return 1;};
    virtual int isBaseRelation() { return 1;};

    int getTableLock(const TransID &tid,LockInfo  lm);
    int getTupleLock(const TransID &tid, tuplePtr tptr, LockInfo  lm);

    /* Handling Indexes */
    virtual int insertIndexes(const TransID &tid, tuplePtr tptr,
                              const char *affected_bit_list =0);
    virtual int deleteIndexes(const TransID &tid, tuplePtr tptr,
                              const char *affected_bit_list = 0);

    /* Handling OTM relationships */
    virtual int insertFromOTMs(const TransID &tid, tuplePtr tptr,
                              const char *affected_bit_list);
    virtual int insertToOTMs(const TransID &tid, tuplePtr tptr,
                              const char *affected_bit_list);
    virtual int deleteFromOTMs(const TransID &tid, tuplePtr tptr,
                              const char *affected_bit_list);
    virtual int deleteToOTMs(const TransID &tid, tuplePtr tptr,
                              const char *affected_bit_list);

    int OpToOTMs(const TransID &tid, tuplePtr tptr,
                      const char *affected_bit_list, int flag);

    /* Checking for Referential Integrtiy constraints */
    virtual int checkReferentialIntegrityForInsert(const TransID &tid,
                              tuplePtr tptr, const char *affected_ref_list);
    virtual int checkReferentialIntegrityForDelete(const TransID &tid,
                              tuplePtr tptr, const char *affected_ref_list);
    int findSatisfyingTuple(const TransID &tid, DaliTableHandle &dbHandle,
                              DaliSearchCriteria &crit, void **tptr);
    int doesTableContainOtherTuples(const TransID &tid, 
                                    DaliTableHandle &dbHandle, 
                                  DaliSearchCriteria &crit, tuplePtr *tptr);

    int determine_index_position(INDEXCAT *index_ptr);
    int determine_attr_position(char *attrName);

    int determineAffectedIndexAndOTMAndRef(DaliAttrList &list);

    /* Moving Data */
    int fillUserBuffer(void *source_ptr, 
                              DaliRefMethod refMethod, void * buf);
    inline int fillUserBuffer(tuplePtr tptr, 
                              DaliRefMethod refMethod, void * buf) {
        return fillUserBuffer(tptr.mem_ptr(db),refMethod,buf);
    }
    int fillUserBuffer_for_insert(const TransID &tid, void *buf, 
               DaliAttrVal & attr_val, DaliRefMethod ref);
    int fillUserBuffer_for_update(const TransID &tid, void *buf, 
               DaliAttrVal & attr_val, DaliRefMethod ref);

    int copyIntoTuple(tuplePtr tptr, DaliRefMethod refMethod,
                      void *buf);
    int copyIntoTuple(tuplePtr tptr, DaliRefMethod refMethod,
                      void *buf, DaliAttrList &alist);
    int copyIntoTuple(tuplePtr tptr, DaliAttrValList *avlist = 0);
    int copyIntoTuple(tuplePtr tptr, DaliRefMethod refMethod,
                      void *tuple_ptr, DaliAttrValList &avlist);

    /* Logging */
    int copyIntoTupleLogged(tuplePtr tptr, DaliRefMethod refMethod,
                      void *buf, DaliAttrList *alist = 0);


    /*
     * The above function is for updating a small percentage of a
     * tuples fields/bytes.  Otherwise we should log the entire
     * tuple in a single go for speed.
     * 
     * To log the entire tuple, call functions on tptr or the HF
     * directly.
     */

    /* Querying: */
    virtual void fillSearchTermInfo(DaliSearchCriteria &crit);
    DaliInternalIndex *chooseIndex(DaliSearchCriteria &crit);

    
    // function to free all allocated structures
    void cleanUp();

    // default constructor and destructor
    DaliRelInternal();
    virtual ~DaliRelInternal();

    virtual int choosePlan(DaliSearchCriteria &crit, DaliSearchPlan &plan);

    /*** OTM related functions ***/

    int cacheOTMInfo(const TransID &, hash_index_on_tuples *, DaliInternalOTMRef *);
    int cacheOTMInfoforAttr(const TransID &, char **, hash_index_on_tuples *, 
                            unsigned char *, DaliInternalOTMRef *, int);
    int determine_OTM_position(char *otmName, DaliInternalOTMRef *otmList, int num_entries);
    DaliInternalOTMRef *lookupFromOTM(char *otmName);
    DaliInternalOTMRef *lookupToOTM(char *otmName);

    int constructSearchCrit(DaliInternalOTMRef *targetOTM, 
                            DaliInternalOTMRef *sourceOTM, void *tpr);


    /*** Ref related functions ***/

    int cacheRefInfo(const TransID &, hash_index_on_tuples *hashIndex,
                            DaliInternalOTMRef *refList);
    int cacheRefInfoforAttr(const TransID & tid, char ** keyInfo,
                            hash_index_on_tuples *hashIndex, 
                            unsigned char *refAttrList, DaliInternalOTMRef *refList, int num);
    int determine_ref_position(REFCAT *, DaliInternalOTMRef *, int num_entries);
    DaliInternalOTMRef *lookupFromRef(REFCAT * refcatPtr);
    DaliInternalOTMRef *lookupToRef(REFCAT * refcatPtr);


    friend ostream & operator << (ostream &, const DaliRelInternal &);
    friend class DaliTableIter;
  private:
    /** functions to get and set null attr values **/
    int getAttrNullBit(tuplePtr tptr, int attr_pos);
    int getAttrNullBits(tuplePtr tptr);
    int setAttrNullBit(const TransID &tid, tuplePtr tptr, int attr_pos, 
                       int bitval);
    int setAttrNullBits(const TransID &tid, tuplePtr tptr, int bitval);
    int setAllAttrNullBits(const TransID &tid, tuplePtr tptr, int bitval);
    int setAllAttrNullBits(const TransID &tid, tuplePtr tptr, 
                           DaliAttrList &alist, int bitval);
    unsigned char *getPtrToAttrNullBits(tuplePtr tptr);
};

class DaliViewInternal : public DaliRelInternal {
  public:
    VIEWCAT *viewInfo; // null for base relation - 
    DaliRelInternal *baseRel;

    virtual int cacheRelation(const TransID &tid);
    int canInsert(){ return 0;}
    virtual int isBaseRelation() { return 0;};
    virtual int baseIndxCount() { return baseRel->baseIndxCount();}    
    virtual int baseFromOTMCount() { return baseRel->baseFromOTMCount();}    
    virtual int baseToOTMCount() { return baseRel->baseToOTMCount();}    
    virtual int baseFromRefCount() { return baseRel->baseFromRefCount();}    
    virtual int baseToRefCount() { return baseRel->baseToRefCount();}    
    virtual DaliRelInternal *baseRelInfo() { return baseRel;}    
    virtual int deleteTuple(const TransID &tid, tuplePtr tup);
    virtual char *name() { return viewInfo->viewName; }
        
    virtual int insertIndexes(const TransID &tid, tuplePtr tptr,
                              const char *affected_bit_list);
    virtual int deleteIndexes(const TransID &tid, tuplePtr tptr,
                              const char *affected_bit_list);

    virtual int insertFromOTMs(const TransID &tid, tuplePtr tptr,
                              const char *affected_bit_list);
    virtual int deleteFromOTMs(const TransID &tid, tuplePtr tptr,
                              const char *affected_bit_list);
    virtual int insertToOTMs(const TransID &tid, tuplePtr tptr,
                              const char *affected_bit_list);
    virtual int deleteToOTMs(const TransID &tid, tuplePtr tptr,
                              const char *affected_bit_list);

    virtual int checkReferentialIntegrityForInsert(const TransID &tid,
                              tuplePtr tptr, const char *affected_ref_list);
    virtual int checkReferentialIntegrityForDelete(const TransID &tid,
                              tuplePtr tptr, const char *affected_ref_list);

    virtual ~DaliViewInternal();
    virtual int choosePlan(DaliSearchCriteria &crit, DaliSearchPlan &plan){
      return baseRel->choosePlan(crit, plan);
    }
    virtual void fillSearchTermInfo(DaliSearchCriteria &crit){
      baseRel->fillSearchTermInfo(crit);
    }
    friend ostream & operator << (ostream &, const DaliViewInternal &);
};

class DaliTableHandle {
  public:
    int open(const TransID &tid, const char *name, 
             DaliRefMethod in_def_method = DREL_UNKNOWN_METHOD);

    int close(const TransID &tid);

    int initScan(const TransID &tid,
                  DaliTableIter &iter,
                  DaliSearchCriteria &criterea,
                  DaliRefMethod ref = DREL_UNKNOWN_METHOD,  
                  DaliSearchPlan *plan = 0);

    int initScan(const TransID &tid,
                  DaliTableIter &iter,
                  DaliRefMethod ref = DREL_UNKNOWN_METHOD);

    int closeScan(DaliTableIter &iter);

    int pointLookup(const TransID &tid, DaliTupleHandle &ref, 
                    DaliSearchCriteria &criterea);

    int pointLookup(const TransID &tid, void *buf,
                    DaliSearchCriteria &criterea);

    int insert(const TransID &tid, void *buf,
               DaliRefMethod ref = DREL_UNKNOWN_METHOD,
               DaliAttrList *alist = NULL);
    int insert(const TransID &tid, void *buf, DaliAttrValList &list, 
               DaliRefMethod ref = DREL_UNKNOWN_METHOD,
               DaliAttrList *alist = NULL);

    /* NOTE: beginInsert/endInsert pairs cannot overlap since the redo
     * log record for a beginUpdate invoked by a beginInsert is filled
     * in only by endInsert - invoking another beginInsert before the 
     * endInsert results in an f_alloc call which would cause the redo
     * log record to be appended to the global log before it is filled 
     * in */

    int beginInsert(const TransID &tid, void *, DaliRefMethod
                    ref = DREL_UNKNOWN_METHOD); 
    int beginInsert(const TransID &tid, void *, DaliAttrValList &list,
                    DaliRefMethod ref = DREL_UNKNOWN_METHOD); 
    int endInsert(const TransID &tid, DaliAttrList *alist = NULL); 
    int endInsert(const TransID &tid, DaliAttrValList &vlist,
                  DaliAttrList *alist = NULL); 

    int choosePlan(DaliSearchCriteria &crit, DaliSearchPlan &plan){
      return schemaInfo->choosePlan(crit, plan);
    }

    int deleteWhere(const TransID &tid, DaliSearchCriteria
                                     &criteria);

    int printQuery(const TransID &tid, DaliSearchCriteria &criteria,
                   FILE *fp = stdout);
    int printQuery(const TransID &tid, DaliSearchCriteria &criteria,
        DaliAttrList &alist, FILE *fp = stdout);


    int returnSatisfyingTuple(const TransID &tid, DaliTableHandle &handle,
                                     void **ptr);

    int scheme(DaliTableSchema &sch);
    /* FIX: need some way to find out about indexes */

    /* Smaller utility functions */
    char *name();

//  protected:

    DaliRelInternal *schemaInfo;
    DaliRefMethod defaultRefMethod;
};

class DaliRelAttr {
public:
    DaliRelAttr(const char *name);
    DaliRelAttr(const char *name, DaliTableHandle &rel);
    DaliRelAttr(const char *name, char *relname);
    DaliRelAttr(const char *name, DaliAttrType typ, int size,
                 void *default_value = 0);
    DaliRelAttr(const DaliRelAttr &other);

    void init(const char *name);
    void init(const char *name, DaliTableHandle &rel);
    void init(const char *name, char *relname);
    void init(const char *name, DaliAttrType typ, int size, 
              void *default_value = 0);

    int resolve(const TransID &tid, const char *in_relname = 0); 
                         // look up in schema
    int resolve(const DaliTableHandle &htab); // look up from open table

    void setRel(DaliTableHandle &rel);
    void setRel(char *relname);

    char *attrName() {return ainfo.attrName;}
    DaliAttrType attrType() {return ainfo.type;}
    int attrLen() {return ainfo.length;}
    ATTRCAT &aInfo() {return ainfo;}

protected:
    ATTRCAT *attrib; // in schema
    RELCAT *rel; // in schema
    ATTRCAT ainfo;
    friend class DaliTableSchema;
};


class DaliTableSchema {
public:    
    DaliTableSchema() 
        : fldlist(0), fldcount(0), fldmax(0) { };
    int addAttr(class DaliRelAttr &);
    int addAttr(const char *name, DaliAttrType typ = DALI_UNKNOWN_TYPE, int length = 0);
    int countAttrs() { return fldcount;} 
    DaliRelAttr & NthAttr(int n);
    void reset() { fldcount = 0; };
    ~DaliTableSchema() { destroy(); }
    int print(FILE *fp);
    int printAsStruct(FILE *fp, char *structtype);
    friend DaliTableSchema& operator << (DaliTableSchema &,
                                          const DaliRelAttr &);

    int resolveAttrs(const TransID& tid, const char *tableName);
    int resolveAttrs(DaliTableHandle &htab);
    int computeOffsets(int& total_length, int &max_align_factor);

    int destroy() { free(fldlist); fldcount = 0; fldmax = 0; return 0;};

protected:

    DaliTableHandle handle;
    DaliRelAttr *fldlist;
    int fldcount;
    int fldmax;
};





class DaliSearchTerm {
    public:
    DaliSearchTerm(const char *, char *);
    DaliSearchTerm(const char *, char *, int);
    DaliSearchTerm(const char *, char *, float);
    DaliSearchTerm(const char *, char *, double);
    DaliSearchTerm(const char *, char *, DaliDate);
    DaliSearchTerm(const char *, char *, DaliTime);
    DaliSearchTerm(const char *, char *, const daliNULL &);
    DaliSearchTerm(const char *, char *, char *);
    DaliSearchTerm(const char *, char *, void *, int sz);
    DaliSearchTerm(const char *, char *, const DaliRelAttr &);
    DaliSearchTerm() : val(0) { };

/*    
    setup(char * name, char * cmp_op, int _val) { 
      new (this) DaliSearchTerm(name, cmp_op, _val);
    }
    setup(char * name, char * cmp_op, float _val) { 
      new (this) DaliSearchTerm(name, cmp_op, _val);
    }
    setup(char * name, char * cmp_op, char *_val) { 
      new (this) DaliSearchTerm(name, cmp_op, _val);
    }
    */
    int destroy() { return val.destroy(); }
    ~DaliSearchTerm() {
          destroy();
    }
    DaliSearchTerm & operator = (const DaliSearchTerm &);
    DaliAttrInfo fld;
    ComparisonOp op;
    DaliVal val;
};

class DaliSearchCriteria;

class DaliSearchPlan {

public:

    DaliRelInternal *relInfo;
    DaliInternalIndex *whichIndex;
    enum Direction {FORWARD,BACKWARD} whichDirection;  // Default FORWARD
                 // not currently implemented
    ComparisonOp start_cmp_op;
    void **start_key_val;  
    ComparisonOp end_cmp_op;
    void **end_key_val;

    int setIndex(const DaliTableHandle& tableHandle, const char *indexName,
                Direction dir = FORWARD);

    /* User can pass in ">=" and we use op */
    int setStartVal(const char * op, DaliAttrValList &list);
    int setStopVal(const char * op, DaliAttrValList &list);
    DaliSearchPlan(const char *indexName, Direction dir = FORWARD);
    /* Initialize other fields */
    
    DaliSearchPlan() : whichDirection(FORWARD), whichIndex(NULL),
    start_key_val(NULL), end_key_val(NULL), relInfo(NULL) { };
    DaliSearchPlan(DaliInternalIndex *indx) : whichDirection(FORWARD),
    whichIndex(indx), start_key_val(NULL), end_key_val(NULL), relInfo(NULL){};
    int destroy();
    ~DaliSearchPlan();
    int extractKeys(DaliSearchCriteria *search);

    /*
     * Figure out start and end key val, and comparison ops, based
     * on the index type,  fields and the comparisons present in
     * search.
     */
};

struct DaliSearchTermInfo{
  DaliSearchTerm *first;
  DaliSearchTerm *second;
};

class DaliSearchCriteria {
public:
    DaliSearchCriteria()
        { init(); }
    DaliSearchCriteria(DaliTableHandle &rel)
        {init(); setRelation(rel); }
    DaliSearchCriteria(const char *, char * comp_op, int);
    DaliSearchCriteria(const char *, char * comp_op, float);
    DaliSearchCriteria(const char *, char * comp_op, char *);

    void init()
    { termcount = 0; info =0; termmax = 0; termInfo = 0; termlist = 0;} 
    void addTerm(const char *, char * comp_op);
    void addTerm(const char *, char * comp_op, int);
    void addTerm(const char *, char * comp_op, float);
    void addTerm(const char *, char * comp_op, double);
    void addTerm(const char *, char * comp_op, DaliDate);
    void addTerm(const char *, char * comp_op, DaliTime);
    void addTerm(const char *, char * comp_op, char *);
    void addTerm(const char *, char * comp_op, void *, int);
    void addTerm(const char *, char * comp_op, const daliNULL &);
    int addTerm(DaliSearchTerm &trm);
    int append(DaliSearchCriteria &crit);
    void copy(const DaliSearchCriteria &crit);
    int setRelation(DaliTableHandle &table) {
        info = table.schemaInfo;
        return 1;
    }

    int zeroVal();
    int reset(); 
    int destroy(); 
    int choosePlan(DaliTableHandle &table, DaliSearchPlan &plan){
      return table.schemaInfo->choosePlan(*this, plan);
      };
    ~DaliSearchCriteria() {destroy();};
    int has_terms(){return termcount;}
    friend class DaliRelInternal;
    friend class DaliTableIter;
protected:
    DaliRelInternal *info;
    DaliSearchTermInfo *termInfo;
                /* kvr- Look at the comments inside
                   DaliRelInternal::fillSearchTermInfo(DaliSearchCriteria &crit)
                   for the meaning of the $termInfo$ field. */

    //List<DaliSearchTerm> termlist;
    DaliSearchTerm *termlist;
    int termcount;
    int termmax;
};


struct DaliInternalOTMRef {
  OTMCAT *otmcat;
  REFCAT *refcat;    // only one of these two will be used in a instance
  int numAttrs;
  int isUnique;
  DaliInternalIndex *index;
  DaliSearchCriteria crit;
  DaliRelInternal *relInfo;
  DaliTableHandle handle;
  DaliInternalOTMRef *otmRefInfo;
  DaliInternalAttr **attrList;

  DaliInternalOTMRef(){attrList = 0;}
  ~DaliInternalOTMRef(){delete []attrList; crit.zeroVal(); crit.destroy();}
  friend ostream & operator << (ostream &, const DaliInternalOTMRef &);

};

class DaliTableIter;

class DaliTupleHandle {
 public:
    int init(DaliTableHandle &which_rel);

    int recSize();
    void *dataPtr() {return buf_ptr;}

    int deleteTuple(const TransID &tid);

    /* should insert be a member function of this class */

    virtual int copyFrom(const DaliTupleHandle &source) = 0;
    /*
     * or:  copyNamed() and copyPositional() ?
     */

    int referTo(const DaliTupleHandle &source);
    int referTo(const DaliTableIter &srciter);
// protected:
    int referTo(tuplePtr source);

 public:
/*    virtual int update(const TransID &tid, DaliAttrList *alist = 0);
    virtual int beginUpdate(const TransID &tid, DaliAttrList *alist = 0);
    virtual int endUpdate(const TransID &tid, DaliAttrList *alist = 0);
    */
    int destroy();
    DaliTupleHandle() { tuple_ptr = buf_ptr = NULL; info = (DaliRelInternal *) NULL; }
    ~DaliTupleHandle() { destroy(); }
protected:
    DaliRefMethod refMethod;
    DaliRelInternal *info;
    tuplePtr tuple;
    void *tuple_ptr;
    void *buf_ptr;
    int buf_size;
    int own_buffer; //  did we allocate it?

friend class DaliTableIter;

};

class DaliTupleBuf : public DaliTupleHandle {
 public:
    DaliTupleBuf() {own_buffer = 0;};
    int setupBuf(DaliTableHandle &which_rel, void  *buf);
    int setupFieldList(DaliTableHandle &which_rel, void **list);
    int setupFieldList(DaliTableHandle &which_rel, void * ...);
    virtual int copyFrom(const DaliTupleHandle &source);

    /* should insert be a member function of this class */

    int insert(const TransID &tid, DaliAttrList *alist);
    int insert(const TransID &tid, DaliAttrValList &list, 
               DaliAttrList *alist);

    int update(const TransID &tid, DaliAttrList *alist = 0);
    int update(const TransID &tid, DaliAttrValList &list, 
               DaliAttrList *alist = 0);

protected:
};

class DaliTupleRef : public DaliTupleHandle {
 public:
    int setupPointer(DaliTableHandle &which_rel);
    int setupPointerList(DaliTableHandle &which_rel, void **list);
    int setupPointerList(DaliTableHandle &which_rel);
    virtual int copyFrom(const DaliTupleHandle &source);

    int beginInsert(const TransID &tid);
    int endInsert(const TransID &tid, DaliAttrList *alist = NULL);

    int beginInsert(const TransID &tid, DaliAttrValList &list);
    int endInsert(const TransID &tid, 
                  DaliAttrValList &list,
                  DaliAttrList *alist = NULL);

    int beginUpdate(const TransID &tid, DaliAttrList *alist);
    int endUpdate(const TransID &tid, DaliAttrList *alist);

    int beginUpdate(const TransID &tid, 
                    DaliAttrValList &list,
                    DaliAttrList *alist);

    int endUpdate(const TransID &tid, 
                  DaliAttrValList &list,
                  DaliAttrList *alist);

};

class DaliRelMgr {
    /*
     * Do we want something here to ecapsulate DaliSys::open()?
     */

public:
    static NonPersHash *open_tables;
    static int open_table_timestamp;

    static DaliTableHandle hRELCAT;
    static DaliTableHandle hATTRCAT;
    static DaliTableHandle hINDEXCAT;
    static DaliTableHandle hINDEXATTRCAT;
    static DaliTableHandle hVIEWCAT;
    static DaliTableHandle hVIEWATTRCAT;
    static DaliTableHandle hOTMCAT;
    static DaliTableHandle hOTMATTRCAT;
    static DaliTableHandle hREFCAT;
    static DaliTableHandle hREFATTRCAT;

    static hash_index_on_tuples *relcat_index_on_relname;
    static hash_index_on_tuples *attrcat_index_on_relname;
    static hash_index_on_tuples *attrcat_index_on_relname_and_attrname;
    static hash_index_on_tuples *indexcat_index_on_relname;
    static hash_index_on_tuples *indexattrcat_index_on_relname;
    static hash_index_on_tuples *indexattrcat_index_on_relname_and_attrname;
    static hash_index_on_tuples *indexattrcat_index_on_indexid;
    static hash_index_on_tuples *viewcat_index_on_viewname;
    static hash_index_on_tuples *viewattrcat_index_on_viewname;
    static hash_index_on_tuples *OTMcat_index_on_fromRelName;
    static hash_index_on_tuples *OTMcat_index_on_toRelName;
    static hash_index_on_tuples *OTMattrcat_index_on_OTMId;
    static hash_index_on_tuples *OTMattrcat_index_on_fromRelName_and_fromAttrName;
    static hash_index_on_tuples *OTMattrcat_index_on_toRelName_and_toAttrName;
    static hash_index_on_tuples *refcat_index_on_fromRelName;
    static hash_index_on_tuples *refcat_index_on_toRelName;
    static hash_index_on_tuples *refattrcat_index_on_refId;
    static hash_index_on_tuples *refattrcat_index_on_fromRelName_and_fromAttrName;
    static hash_index_on_tuples *refattrcat_index_on_toRelName_and_toAttrName;

    static DaliDB *rel_db;

    static int init(char *db_name = NULL);
    static int open(char *db_name = NULL);
    static int initCatalogInfo();

    static RELCATCatalogDBPtr getRelid(const TransID &tid, const char *relname);
    static ATTRCATCatalogDBPtr getAttrid(const TransID &tid, const char *relName, 
                                         const char *attrName);
    static VIEWCATCatalogDBPtr getViewid(const TransID &tid, const char *viewname);

    static int createTable(const TransID &tid, char *name, 
                           DaliTableSchema &scheme, DaliDB *db=0);

    static int createTable(const TransID &tid, char *name, 
                           DaliTableSchema &scheme, 
                           char *db_file_name);

    static int createIndex(const TransID &tid, char *name, 
                           char *table_name,
                           DaliAttrList &attr_list, DaliIndexType type);

    static int deleteIndex(const TransID &tid, char *name, 
                           char *table_name);

    static int auditIndex(const TransID &tid, char *name, 
                           char *table_name);

    static int printIndex(const TransID &tid, char *name, 
                           char *table_name);

    static int createOneToMany( const TransID &tid, char* oneToManyName,
                         char *fromTableName, DaliAttrList& fromTableList,
                         char *toTableName, DaliAttrList& toTableList,
                         int addPointers = 0, int oneToManyFlag = 1);

    static int createReferentialIntegrity(const TransID &tid, 
                         char *fromTableName, DaliAttrList& fromTableList,
                         char *toTableName, DaliAttrList& toTableList);

    static int createManyToMany( const TransID &tid, char* manyToManyName,
                         char *fromTableName, DaliAttrList& fromTableList,
                         char *toTableName, DaliAttrList& toTableList,
                         int addPointers = 0);

    static int createView(const TransID &tid, 
                          char *name, char *sourceTable,
                          DaliTableSchema &scheme);

    static int createView(const TransID &tid, 
                          char *name, char *sourceTable,
                          DaliAttrList &list);

    static int deleteTable(const TransID &tid, char *name);

    static int deleteView(const TransID &tid, char *name);

    static int copyTable(const TransID &tid, 
                         char *sourceName, char *targetName);

    static int renameTable(const TransID &tid, 
                           char *sourceName, char *targetName);
    
   static int renameView(const TransID &tid, char *name, char *new_name);
    
    static int openTable(const TransID &tid, char *name, DaliTableHandle &table_handle,
                         DaliRefMethod in_def_meth =DREL_UNKNOWN_METHOD);

    static int closeTable(const TransID &tid, DaliTableHandle &table_handle);

    static int openTableView(const TransID &tid, char *sourceTable, 
                      DaliTableSchema &schema); /* NOT IMPLEMENTED YET */
    /* utility functions */
    static ComparisonOp comparison_op(char *);
    static DaliDB   *DetermineDBFile();
    static DaliDB   *DetermineIndexDBFile();
    static int validTableName(const TransID & tid, char *name);

    /** functions to get and set null attr values **/
    static int getAttrNullBit(DaliDB *db, tuplePtr tptr, int attr_pos);
    static int setAttrNullBit(const TransID &tid, DaliDB *db, 
                              tuplePtr tptr, int attr_pos, int bitval);
    static int setAllAttrNullBits(const TransID &tid, DaliDB *db, 
                                  tuplePtr tptr, int numAttrs, int bitval);
    static int setAllAttrNullBits(const TransID &tid, tuplePtr tptr, 
                           DaliRelInternal &relInfo, DaliAttrList &alist, 
                                  int bitval);
    static unsigned char *getPtrToAttrNullBits(DaliDB *db, tuplePtr tptr);
};


extern DaliRelAttr DaliRelAttr_NULL;
extern DaliSearchCriteria DaliSearchCriteria_NULL;

class DaliTableIter {
 public:

  DaliTableIter();
  DaliTableIter(DaliTableHandle hTable, DaliRefMethod = DREL_UNKNOWN_METHOD);
  int setTable(DaliTableHandle hTab, DaliRefMethod = DREL_UNKNOWN_METHOD);
  int open(const TransID &tid);
  int close();
  int reset();

  int next(void *);
  int next(DaliTupleHandle &);

  int next();
  int current(void *);
  int current(DaliTupleHandle &);

  int beginUpdate( DaliAttrList *touchdAttrs = NULL);
  int beginUpdate(DaliAttrValList &, DaliAttrList *touchdAttrs = NULL);
  int endUpdate( DaliAttrList *touchdAttrs = NULL);
  int update(void *, DaliAttrList *touchdAttrs = NULL);
  int update(void *, DaliAttrValList &, DaliAttrList *touchdAttrs = NULL);
  int deleteTuple();

  int initScan(char *OTMname, DaliTableIter &iter, DaliRefMethod = DREL_UNKNOWN_METHOD);
  int initScan(char *OTMname, DaliTableIter &iter, 
               DaliSearchCriteria &crit, DaliRefMethod = DREL_UNKNOWN_METHOD);
  int closeScan(DaliTableIter &iter);
  char * selectedIndex() { return indexcatptr->indexName; } 


  int addTerm(const char *, char * comp_op, int);
  int addTerm(const char *, char * comp_op, float);
  int addTerm(const char *, char * comp_op, char *);
  int addTerm(const char *, char * comp_op, double);
  int addTerm(const char *, char * comp_op, DaliDate);
  int addTerm(const char *, char * comp_op, DaliTime);
  int addTerm(const char *, char * comp_op, void *, int);
  int addTerm(const char *, char * comp_op, const daliNULL &);

  DaliTableIter & operator = (const DaliTableIter &);

 
  int isOpen() { return search_in_progress; }
  void setDaliRefMethod(DaliRefMethod _refMethod){refMethod = _refMethod;}

 protected:

  void init(TransID _tid, DaliRelInternal *_relInfo, 
           DaliRefMethod _refmethod, 
           DaliSearchCriteria *crit, DaliIndexType _indextype, 
           void *_indexIter, INDEXCAT *_indexcatptr);

  int endOfScan();

  int get_pointer_to_next_tuple(void **);
  /* kvr- It seems that this function returns a pointer to the next
     tuple in the base table, in case $this$ iterator pertains to a 
         view. */

  int search_in_progress;
  DaliRelInternal *relInfo;
  DaliRefMethod refMethod;
    /* The old iterater doesn't have a notion of a current tuple.
     * Can we add it?  What if it gets deleted through another iterator
     * of the same transaction?  Can we make ttree, hf and hash iterators
     * expose a current tuple and use that?  
     *
     * Current code uses the "tuple" above, but that may have to change.
     */


 public:

  TransID tid;
  DaliIndexType indextype;
  void * indexIter;
  INDEXCAT *indexcatptr;
  tuplePtr tuple;
  void *tuple_ptr;
  DaliDB *db;
  DaliSearchCriteria search_crit;

  int does_tuple_satisfy_pred(void *ptr);
  
  friend class DaliTableHandle;

};

/**** OTM list declarations ****/

#define DALI_NUM_OTM_PTRS  (3)

class DaliDbOffsetListNode;

typedef DbOffset_of<DaliDbOffsetListNode>  DaliOListOffset;
 
class DaliDbOffsetListNode {
    DaliOListOffset next;
    
    DbOffset ptrBlock[DALI_NUM_OTM_PTRS];

    friend class DaliDbOffsetListIter;    
    friend class DaliDbOffsetList;    
};
class DaliDbOffsetListIter;

class DaliDbOffsetList {
    DaliOListOffset first;

    public:
    void init() {first = DbOffset_NULL;}
    int insert(const TransID &tid,  DaliDB *db, DbOffset dbo);
    int remove(const TransID &tid,  DaliDB *db, DbOffset dbo);
    int initScan(const TransID &tid, DaliDB *db, 
                 DaliDbOffsetListIter &iter);
    int destroy(const TransID &tid, DaliDB *db);         
    friend class DaliDbOffsetListIter;
};

class DaliDbOffsetListIter {
    DaliOListOffset *ptrToCurrNode;
    DaliDbOffsetListNode *currNode;
    int currOffset; // offset in node
    DaliDB *db;

    int findNextEmpty();
    int advance();

    public:
    int next(DbOffset &data);
    int removeCurrent(const TransID &tid);
    DaliDbOffsetListIter *copy();
    friend class DaliDbOffsetList;
};

/********************* Error Codes Returned by Rel Mgr ***************/
/* Internal Errors */
#define DALI_CATALOG_INCONSISTENT (-2500)

/* User Errors  */
#define DALI_NO_ATTR                   (-2000)
#define DALI_NO_TABLE                  (-2001)
#define DALI_CANNOT_OPEN_TABLE         (-2002)
#define DALI_CANNOT_OPEN_DBFILE        (-2003)
#define DALI_NEED_INDEX                (-2004)
#define DALI_NEED_UNIQUE_INDEX         (-2005)  /* to create otm */
#define DALI_INVALID_REFMETHOD         (-2006)
#define DALI_REF_INTEGRITY             (-2008)
#define DALI_NO_INSERTS                (-2009)  /* attempted insert on view */
#define DALI_NO_NESTING                (-2010)  /* begin insert inside begini insert */
#define DALI_NO_TUPLE                (-2011)  /* no current tuple */

#endif
