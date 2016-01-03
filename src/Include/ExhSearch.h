// exhaustive search dag optimization algorithm

#ifndef __EXHSEARCH_H__
#define __EXHSEARCH_H__

#include "typedefs.h"
#include "Cost.h"
#include "List.h"
#include "Volcano.h"
#include "DAGOpt.h"

const int MaxInProgress = 10;

class Equivalence_t;            // equivalence class
class Plan_t;                   // evaluation plan

class ActiveStack_t;    // active plangroup stack
class SharStack_t;      // sharable plangroup stack

// entry for active stack
class ActiveStackEntry_t : public RefC_t {
    PlanGroup_t *pg;
    int numInProgress;

    PlanGroup_t *inProgress[MaxInProgress];

    // traversal for finding in-progress pgs
    void FindInProgressTrav(PlanGroup_t *planGroup,
                                    int travId, PlanGroup_t *a[], int *n);
    // find the weakest covering property node that is in progress
    void FindInProgress(PlanGroup_t *planGroup, PlanGroup_t *a[], int *n);

public:
    ActiveStackEntry_t(PlanGroup_t *pg_a);

    PlanGroup_t *PlanGroup(void) const
    { return pg; }

    PlanGroup_t *InProgress(int i) const
    {
        if( i >= 0 && i < numInProgress )
            return inProgress[i];
        return NULL;
    }

    int NumInProgress(void) const
    { return numInProgress; }

    int operator == (const ActiveStackEntry_t& e) const;

    int operator != (const ActiveStackEntry_t& e) const
    { return !(*this == e); }

    ~ActiveStackEntry_t(void);
};

// plan configuration
class PlanConfig_t : public RefC_t {
    Plan_t **planList;                  // sorted list of plan's node ids
    int size;                           // number of nodes in the plan
    CostVal_t costVal;                  // cost of the plan
    char isFailed;                      // is this a failed plan?

    // called by TranslateBestPlan
    // returns a nearest covering marked node 
    PlanGroup_t *MarkedSuper(PlanGroup_t *pg) const;

public:
    PlanConfig_t(CostVal_t costVal_a, int isFailed_a)
        : planList(NULL), size(0), costVal(costVal_a), isFailed(isFailed_a)
    { assert(costVal >= 0); }

    PlanConfig_t(const PlanConfig_t *config,
                        Plan_t *plan, CostVal_t costVal_a);

    Plan_t **PlanList(void) const
    { return planList; }

    int Size(void) const
    { return size; }

    CostVal_t CostVal(void) const
    { return costVal; }

    int IsFailed(void) const
    { return isFailed; }

    // translates the configuration of best plan to the actual plan
    void TranslateBestPlan(PlanGroup_t *rootGroup) const;

    // print the plan configuration
    void Print(void) const;

    ~PlanConfig_t(void)
    {
        if( planList )
            delete[] planList;
    }
};

// active stack configuration
// For efficiency purposes supports the option of lazy evaluation, i.e.
// when isLazy in the constructor is 1 idList is computed only if it is
// needed. Caution: in the case of lazy evaluation, user must take care that
// corresponding stack does not change in the meantime.
// Lazy evaluation allows lookup failures to be detected efficiently
// in tables indexed on ActiveStackConfig_t
class ActiveStackConfig_t : public RefC_t {
    static int CurID;                   // for unique ID assignment
    int id;                             // unique id for efficient
                                        // equality determination
    const ActiveStack_t *stack;         // stack -- NULL if not lazy
    ActiveStackEntry_t **entryList;     // sorted list of plangroup nodes
    int size;                           // number of nodes in the stack
    int sum;                            // sum of ids

    // create the idList from the stack
    void CreateEntryList(void);

public:
    ActiveStackConfig_t(const ActiveStack_t *stack_a, int isLazy);

    // unique id
    int ID(void) const
    { return id; }

    // evaluate idList if not evaluated before
    // stack == NULL if Evaluate has been invoked earlier
    void Evaluate(void);

    ActiveStackEntry_t **EntryList(void)
    {
        Evaluate();
        return entryList;
    }

    int Size(void) const
    { return size; }

    int Sum(void) const
    { return sum; }

    int operator == (ActiveStackConfig_t& config);

    int HashValue(void) const
    { return size + sum; }

    void Print(void) const;

    ~ActiveStackConfig_t(void);
};

// stack configuration
// For efficiency purposes supports the option of lazy evaluation, i.e.
// when isLazy in the constructor is 1 idList is computed only if it is
// needed. Caution: in the case of lazy evaluation, user must take care that
// corresponding stack does not change in the meantime.
// Lazy evaluation allows lookup failures to be detected efficiently
// in tables indexed on SharStackConfig_t
class SharStackConfig_t : public RefC_t {
    static int CurID;                   // for unique ID assignment
    int id;                             // unique id for efficient
                                        // equality determination
    const SharStack_t *stack;           // stack -- NULL if not lazy
    PlanGroup_t **planGroupList;        // sorted list of plangroup nodes
    int size;                           // number of nodes in the stack
    int sum;                            // sum of plangroup node ids

    // create the idList from the stack
    void CreatePlanGroupList(void);

public:
    SharStackConfig_t(const SharStack_t *stack_a, int isLazy);

    // unique id
    int ID(void) const
    { return id; }

    // evaluate idList if not evaluated before
    // stack == NULL if Evaluate has been invoked earlier
    void Evaluate(void);

    PlanGroup_t **PlanGroupList(void)
    {
        Evaluate();
        return planGroupList;
    }

    int Size(void) const
    { return size; }

    int Sum(void) const
    { return sum; }

    int operator == (SharStackConfig_t& config);

    int HashValue(void) const
    { return size + sum; }

    void Print(void) const;

    ~SharStackConfig_t(void);
};

// plan entry -- indexed by adornment
class PlanEntry_t {
    SharStackConfig_t *adornment;
    PlanConfig_t *planConfig;

public:
    // constructor
    PlanEntry_t(SharStack_t& sharStack, PlanConfig_t *planConfig_a = NULL);

    // copy constructor
    PlanEntry_t(const PlanEntry_t& e);

    // hash value --- used for indexing
    int HashValue(void) const
    { return adornment->HashValue(); }

    // equality comparison
    int operator == (PlanEntry_t& entry);

    // adornment --- shar stack configuration
    SharStackConfig_t* Adornment(void) const
    { return adornment; }

    // plan configuration
    PlanConfig_t *PlanConfig(void) const
    { return planConfig; }

    // set the plan configuration
    void SetPlanConfig(PlanConfig_t *planConfig_a);

    // print the plan configuration
    void Print(void) const;

    // destructor
    ~PlanEntry_t(void);
};

// size of hash table per stack configuration indexed by adornment
const int INNERTABLESIZE = 1;   // 1 implies a linked list

// plan cache entry -- indexed by stack configuration
class PlanCacheEntry_t {
    ActiveStackConfig_t *stackConfig;
    Lookup_t<INNERTABLESIZE, PlanEntry_t> *table;

public:
    PlanCacheEntry_t(ActiveStack_t& stack);

    PlanCacheEntry_t(const PlanCacheEntry_t& e);

    int HashValue(void) const
    { return stackConfig->HashValue(); }

    int operator == (PlanCacheEntry_t& entry);

    ActiveStackConfig_t* StackConfig(void) const
    { return stackConfig; }

    Lookup_t<INNERTABLESIZE, PlanEntry_t> *Table(void) const
    { return table; }

    PlanConfig_t *SearchPlan(SharStack_t& sharStack);

    void InsertPlan(SharStack_t& sharStack, PlanConfig_t *planConfig);

    ~PlanCacheEntry_t(void);
};

// size of hash table for cached plans indexed by stack configurations
const int OUTERTABLESIZE = 100;

// plan cache
class PlanCache_t {
    Lookup_t<OUTERTABLESIZE, PlanCacheEntry_t> table;

public:
    PlanCache_t(void) 
    { }

    PlanConfig_t *Search(ActiveStack_t& activeStack, SharStack_t& sharStack);

    void Insert(ActiveStack_t& activeStack, SharStack_t& sharStack,
                                                PlanConfig_t *planConfig);

    ~PlanCache_t(void)
    { }
};

// active stack for plan groups
class ActiveStack_t {
    PrependList_t<ActiveStackEntry_t *> list;
                                        // stack
    int size;                           // number of non-inprogress entries
    int sum;                            // sum of the non-onprogress entries

    ActiveStackConfig_t *config;        // cached configuration
                                        // invalidated by insert/delete

    // invalidate configuration -- called after insert and delete
    // if config is shared, then it must be completely evaluated now for
    // consistency sake
    void InvalidateConfig(void);

public:
    ActiveStack_t(void) : size(0), sum(0), config(NULL) { }

    // insert into plan-group stack --- sum maintained incrementally
    void Insert(PlanGroup_t *pg);

    // delete from plan-group stack --- sum maintained incrementally
    PlanGroup_t *Delete(void);

    const List_t<ActiveStackEntry_t *>& List(void) const { return list; }

    int Size(void) const { return size; }

    int Sum(void) const { return sum; }

    ActiveStackConfig_t *Config(void);

    int IsEmpty(void) const { return list.IsEmpty(); }

    void Print(void) const;

    ~ActiveStack_t(void)
    {
        ListIter_t<ActiveStackEntry_t *> pgIter;
        pgIter.Attach(&list);

        ActiveStackEntry_t *e = NULL;
        while( !pgIter.IsEnd() ) {
            e = pgIter.Next();
            DeRefer(e);
        }

        if( config )
            DeRefer(config);
    }
};

// stack for plan groups
class SharStack_t {
    PrependList_t<PlanGroup_t *> list;  // stack
    int sum;                            // sum of the entries

    SharStackConfig_t *config;          // cached configuration
                                        // invalidated by insert/delete

    // invalidate configuration -- called after insert and delete
    // if config is shared, then it must be completely evaluated now for
    // consistency sake
    void InvalidateConfig(void);

public:
    SharStack_t(void) : sum(0), config(NULL) { }

    // insert into plan-group stack --- sum maintained incrementally
    void Insert(PlanGroup_t *pg);

    // delete from plan-group stack --- sum maintained incrementally
    PlanGroup_t *Delete(void);

    const List_t<PlanGroup_t *>& List(void) const { return list; }

    int Sum(void) const { return sum; }

    SharStackConfig_t *Config(void);

    int IsEmpty(void) const { return list.IsEmpty(); }

    void Print(void) const;

    ~SharStack_t(void)
    {
        ListIter_t<PlanGroup_t *> pgIter;
        pgIter.Attach(&list);

        PlanGroup_t *pg = NULL;
        while( !pgIter.IsEnd() ) {
            pg = pgIter.Next();
            DeRefer(pg);
        }

        if( config )
            DeRefer(config);
    }
};

// exhaustive search algorithm class -- no caching
class ExhSearch_t : public DAGOpt_t {
    // filter for EnumeratePlans
    // copies any inprogress nodes at the top of the activeStack into the
    // system stack before calling Enumerate and restores the same while
    // unwinding the recursion
    PlanConfig_t *EnumerateFilter(CostVal_t *costLimit);

    // actual algorithm -- enumerates all plans
    // returns 1 if pg was available for free, 0 otherwise
    // this helps in pruning further unnecessary alternatives for pg's
    // parents in case pg is available for free
    int EnumeratePlans(int travID, PlanGroup_t *pg,
            CostVal_t *costLimit, PlanConfig_t **planConfig);

protected:
    ActiveStack_t activeStack;  // plangroups to be explored

    // preprocess the dag
    virtual PlanGroup_t *PreProcessDAG(Equivalence_t *rootEq, CostVal_t costUB);

    // interface function for actual search algorithm
    virtual PlanConfig_t *Enumerate(CostVal_t *costLimit);

    // put pg in current plan
    virtual void Mark(PlanGroup_t *pg);

    // remove pg from current plan
    virtual void UnMark(PlanGroup_t *pg);

    // is pg in current plan?
    int IsMarked(PlanGroup_t *pg);

    // is the pg in progress?
    int IsInProgress(PlanGroup_t *pg) const;

    // increment in-progress counts of pg and all the nodes that physically
    // subsume pg
    void SetInProgress(PlanGroup_t *pg);

    // decrement in-progress counts of pg and all the nodes that physically
    // subsume pg
    void ClearInProgress(PlanGroup_t *pg);

    // enumerates the plans with the plan node p fixed in the current plan
    void FixPlan(Plan_t *p, CostVal_t *costLimit, PlanConfig_t **planConfig);

public:
    ExhSearch_t(void) { }

    // search algorithm wrapper for preprocessing
    PlanGroup_t *FindBestPlan(Equivalence_t *eq, CostVal_t *costLimit,
                              SortOrder_t *reqOrder=NULL);

    virtual ~ExhSearch_t(void) { }
};

// exhaustive search with plan caching
class CachedExhSearch_t : public ExhSearch_t {
    PlanCache_t planCache;              // plan cache
    SharStack_t sharStack;              // plangroups that are in
                                        // current plan and are sharable        

    // preprocess the dag
    PlanGroup_t *PreProcessDAG(Equivalence_t *rootEq, CostVal_t costUB);

    // wrapper on the actual search algorithm to to enable caching
    PlanConfig_t *Enumerate(CostVal_t *costLimit);

    // put pg in current plan
    void Mark(PlanGroup_t *pg);

    // remove pg from current plan
    void UnMark(PlanGroup_t *pg);

public:
    CachedExhSearch_t(void) { }

    ~CachedExhSearch_t(void) { }
};

#endif  // __EXHSEARCH_H__
