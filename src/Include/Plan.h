// evaluation plan class

#ifndef __PLAN_H__
#define __PLAN_H__

#include "RefCount.h"
#include "List.h"
// #include "POEdgeList.h"
#include "Cost.h"
#include "PhyProp.h"
#include "PhyPropGroup.h"
#include "Equivalence.h"

#include "Lookup.h"

class PlanGroup_t;

// entry for an ordering edge
class POEdgeEntry_t {
    PlanGroup_t *planGroup;

public:
    POEdgeEntry_t(PlanGroup_t *planGroup_a = NULL) : planGroup(planGroup_a)
    { }

    POEdgeEntry_t(const POEdgeEntry_t& x)
    { planGroup = x.PlanGroup(); }

    PlanGroup_t *PlanGroup(void) const
    { return planGroup; }

    inline int operator == (const POEdgeEntry_t& x) const;

    inline int HashValue(void);

    ~POEdgeEntry_t(void)
    { }
};

// edge list
class POEdgeList_t {
    Lookup_t<1, POEdgeEntry_t> table;

public:
    POEdgeList_t(void) { }

    inline void Insert(PlanGroup_t *pg);

    ~POEdgeList_t(void) { }

    friend class POEdgeIter_t;
};

// edge iterator
class POEdgeIter_t {
    POEdgeList_t *list;         // attached list
    void *state;                // state of the iterator

public:
    POEdgeIter_t(void) : list(NULL), state(NULL) { }

    inline void Attach(POEdgeList_t *list_a);

    inline void Detach(void);

    inline PlanGroup_t *Next(void);

    void Delete(void)
    { assert(list); list->table.Remove(&state); }

    ~POEdgeIter_t(void)
    { if( list ) Detach(); }
};


// evaluation plan
// if root is NULL and cost not NULL, represents a failed plan
// if root is null and cost is NULL, represents the scan for a
// materialized view corresponding to plangroup
class Plan_t {
    PlanGroup_t *planGroup;     // plan group of which this plan is a member
    PhysicalOp_t *root;         // root of the plan
                                // root == NULL implies a failed plan

    Cost_t *cost;               // cost of the plan

public:
    Plan_t(PlanGroup_t *pg, PhysicalOp_t *root_a = NULL, Cost_t *cost_a = NULL)
        : planGroup(pg), root(root_a), cost(cost_a)
    {
        assert(planGroup);
        if( cost ) Refer(cost);
    }

    // id of the plan is taken to be the same as the id of
    // the root of the plan
    int ID(void) const;

    // return the plan group
    PlanGroup_t *PlanGroup(void) const
    { return planGroup; }

    // set the cost
    void SetCost(Cost_t *cost_a)
    {
        Cost_t *oldCost = cost;
        cost = cost_a;
        if( cost ) Refer(cost);
        if( oldCost ) DeRefer(oldCost);
    }

    // set the root
    void SetRoot(PhysicalOp_t *op)
    { root = op; }

    // return the cost
    Cost_t *Cost(void) const
    { return cost; }

    // cost value
    CostVal_t CostVal(void) const
    {
        if( !cost )
            return 0;
        return cost->Value();
    }

    // return the root
    PhysicalOp_t *Root(void) const
    { return root; }

    // is the plan failed?
    int IsFailed(void) const
    { return !cost || !root; }

    // print
    void PrintPlan(int indentLevel = 0) const;

    // destructor
    ~Plan_t(void)
    { if( cost ) DeRefer(cost); }
};

// group of equivalent plans corresponding to a physical property
class PlanGroup_t : public RefC_t {
    static int Count;                   // instance count
    static int CurID;                   // used to determine plan group's id
    static int CurTravID;               // used to determine traversal ids

    int id;                             // plan group's id

    Equivalence_t *eqClass;             // associated equivalence class
    PhyPropGroup_t *prop;               // group of physical properties
    PrependList_t<Plan_t *> algPlans;   // alg plans for the physical props
    PrependList_t<Plan_t *> enfPlans;   // enf plans for the physical props

    Plan_t *bestPlan;                   // best plan out of the above
    Plan_t *bestAlgPlan;                // best alg plan out of the above
    Plan_t *bestEnfPlan;                // best enf plan out of the above

    char bestPlanQueryID;               // id of the query that
                                        // optimized for bestPlan last
    char bestAlgPlanQueryID;            // id of the query that
                                        // optimized of bestAlgPlan last
    char bestEnfPlanQueryID;            // id of the query that
                                        // optimized of bestEnfPlan last

    PrependList_t<PhysicalOp_t *> algParents;
                                        // algorithm ops that have this
                                        // plan group as an input
    PrependList_t<PhysicalOp_t *> enfParents;
                                        // enforcer ops that have this
                                        // plan group as an input

    // the following two lists define the partial order over the plan groups
    POEdgeList_t sub;                   // plan groups whose physical
                                        // properties are covered by prop
    POEdgeList_t super;                 // plan groups whose physical
                                        // properties cover prop
    int travID;                         // id of the last traversal that
                                        // visited this plan group

    int progressCount;                  // optimization for this
                                        // plangroup is in progress
                                        // maintained as a reference
                                        // count for efficiency purposes

    int activeStackCount;               // number of times $this$ is present
                                        // in the active stack (exhaustive)
    int sharStackCount;                 // number of times $this$ is present
                                        // in the shar stack (exhaustive)

    char isMarked;                      // is the plan group marked?

    char isIndexed;                     // is this plan group indexed?

    char isShared;                      // is this plan group shared?
                                        // use in SH/RU to take care of
                                        // materialization costs
    char isBuiltIn;                     // built in phy props (index/sort/...)

    Cost_t *useCost;                    // reuse cost

    int useCount;                       // count of the number of uses
    
    Order_t *pso;                       // Parameter sort order
    Card_t   callCount;                 // call count

public:
    PlanGroup_t(Equivalence_t *eqClass_a, PhyPropGroup_t *prop_a)
        : id(CurID++), eqClass(eqClass_a), prop(prop_a),
            bestPlan(NULL), bestAlgPlan(NULL), bestEnfPlan(NULL),
            bestPlanQueryID(-1), bestAlgPlanQueryID(-1), bestEnfPlanQueryID(-1),
            travID(-1), progressCount(0),
            activeStackCount(0), sharStackCount(0), isMarked(0), isIndexed(-1),
            isShared(0), isBuiltIn(0), useCost(NULL), useCount(0)
    {
        assert(eqClass);
        Refer(prop);
        pso = NULL;
        callCount = -1;
        Count++;
    }

    int ID(void) const
    { return id; }

    void SetParameterSortOrder(Order_t *a_pso)
    { pso = a_pso; }

    void SetCallCount(Card_t a_callCount)
    { callCount = a_callCount; }
    
    Order_t *ParameterSortOrder(void)
    { return pso; }

    Card_t CallCount(void)
    { return callCount; }

    Equivalence_t *EqClass(void) const
    { return eqClass; }

    PhyPropGroup_t *PropGroup(void) const
    { return prop; }

    // add algorithm plan to algPlans
    Plan_t *AddAlgPlan(PhysicalOp_t *root, Cost_t *cost);

    // add enforcer plan to enfPlans
    Plan_t *AddEnfPlan(PhysicalOp_t *root, Cost_t *cost);

    List_t<Plan_t *>& AlgPlans(void)
    { return algPlans; }

    List_t<Plan_t *>& EnfPlans(void)
    { return enfPlans; }

    Plan_t *BestPlan(void) const
    { return bestPlan; }

    Plan_t *BestAlgPlan(void) const
    { return bestAlgPlan; }

    Plan_t *BestEnfPlan(void) const
    { return bestEnfPlan; }

    int BestPlanQueryID(void) const
    { assert(bestPlanQueryID != -1); return bestPlanQueryID; }

    int BestAlgPlanQueryID(void) const
    { assert(bestAlgPlanQueryID != -1); return bestAlgPlanQueryID; }

    int BestEnfPlanQueryID(void) const
    { assert(bestEnfPlanQueryID != -1); return bestEnfPlanQueryID; }

    void SetBestPlan(Plan_t *p)
    { bestPlan = p; }

    void SetBestAlgPlan(Plan_t *p)
    { bestAlgPlan = p; }

    void SetBestEnfPlan(Plan_t *p)
    { bestEnfPlan = p; }

    void SetBestPlanQueryID(int bestPlanQueryID_a)
    {
        assert(bestPlanQueryID_a != -1);
        bestPlanQueryID = bestPlanQueryID_a;
    }

    void SetBestAlgPlanQueryID(int bestAlgPlanQueryID_a)
    {
        assert(bestAlgPlanQueryID_a != -1);
        bestAlgPlanQueryID = bestAlgPlanQueryID_a;
    }

    void SetBestEnfPlanQueryID(int bestEnfPlanQueryID_a)
    {
        assert(bestEnfPlanQueryID_a != -1);
        bestEnfPlanQueryID = bestEnfPlanQueryID_a;
    }

    List_t<PhysicalOp_t *>& AlgParents(void)
    { return algParents; }

    List_t<PhysicalOp_t *>& EnfParents(void)
    { return enfParents; }

    void AddAlgParent(PhysicalOp_t *op)
    { assert(op); algParents.Insert(op); }

    void AddEnfParent(PhysicalOp_t *op)
    { assert(op); enfParents.Insert(op); }

    POEdgeList_t& SubList(void)
    { return sub; }

    POEdgeList_t& SuperList(void)
    { return super; }

    // start a new scan
    static int StartScan(void)
    { return ++CurTravID; }

    // mark the plan group as visited
    void Visit(int myTravID)
    { travID = myTravID; }

    // has the plan group been visited earlier?
    int IsVisited(int myTravID) const
    { return travID == myTravID; }

    int IsMarked(void) const
    { return isMarked; }

    void Mark(void)
    { isMarked = 1; }

    void UnMark(void)
    { isMarked = 0; }

    // is sharable if corresponding eq class is sharable
    int IsSharable(void) const
    { return eqClass->IsSharable(); }

    // degree of sharing
    int Degree(void) const
    { return eqClass->Degree(); }

    // optimization is in progress
    int IsInProgress(void) const
    { assert(progressCount >= 0); return progressCount; }

    // increment the progressCount
    void IncrInProgress(void)
    { assert(progressCount >= 0); progressCount++; }

    // decrement the progressCount
    void DecrInProgress(void)
    { assert(progressCount > 0); progressCount--; }

    // used in exhaustive

    // multiplicity count in the active stack
    int ActiveStackCount(void) const
    { assert(activeStackCount >= 0); return activeStackCount; }

    // increment multiplicity count in the active stack
    void IncrActiveStackCount(void)
    { assert(activeStackCount >= 0); activeStackCount++; }

    // decrement multiplicity count in the active stack
    void DecrActiveStackCount(void)
    { assert(activeStackCount > 0); activeStackCount--; }

    // multiplicity count in the shar stack
    int SharStackCount(void) const
    { assert(sharStackCount >= 0); return sharStackCount; }

    // increment multiplicity count in the shar stack
    void IncrSharStackCount(void)
    { assert(sharStackCount >= 0); sharStackCount++; }

    // decrement multiplicity count in the shar stack
    void DecrSharStackCount(void)
    { assert(sharStackCount > 0); sharStackCount--; }

    // detach the plangroup from the equivalence class
    void Detach(void);

    // cost for materializing the intermediate result
    CostVal_t MatCost(void) const
    { return eqClass->MatCost(); }

    // does the pg contain an indexorder req prop?
    int IsIndexed(void);

    // cost for using the materialized intermediate result
    CostVal_t UseCost(void)
    { return IsIndexed() ? 0 : eqClass->UseCost(); }

    // cost adt for using the materialized intermediate result
    Cost_t *UseCostADT(Volcano_t *opt);

    void Merge(PlanGroup_t *pg);

    // number of nodes
    static int NodeCount(void)
    { return Count; }

    // number of nodes
    static int TotalNodeCount(void)
    { return CurID; }

    // is this plangroup shared?
    int IsShared(void) const
    { return isShared; }

    // mark this plangroupa as shared
    void MarkShared(void)
    { isShared = 1; }

    // unmark this plangroupa as shared
    void UnMarkShared(void)
    { isShared = 0; }

    // is this plangroup built-in?
    int IsBuiltIn(void) const
    { return isBuiltIn; }

    // mark this plangroupa as built-in
    void MarkBuiltIn(void)
    { isBuiltIn = 1; }

    // count of number of uses
    int UseCount(void) const
    { return useCount; }

    // increment the number of uses
    void IncrUseCount(int mult)
    { useCount += mult; }

    void ResetUseCount(void)
    { useCount = 0; }

    // output logical property
    LogProp_t *LogicalProp(void) const
    {
        assert(eqClass);

        LogProp_t *lp = eqClass->LogicalProp();
        assert(lp);
        return lp;
    }

    ~PlanGroup_t(void);
};


// edge entry equality operator
inline int POEdgeEntry_t::operator == (const POEdgeEntry_t& x) const
{
    PlanGroup_t *xPlanGroup = x.PlanGroup();
    if( !planGroup && !xPlanGroup )
        return 1;
    if( !planGroup || !xPlanGroup )
        return 0;
    return planGroup->ID() == xPlanGroup->ID();
}

// edge entry hash value
inline int POEdgeEntry_t::HashValue(void)
{ return planGroup ? planGroup->ID() : 0; }


// insertion in the list
inline void POEdgeList_t::Insert(PlanGroup_t *pg)
{
    assert(pg);
    POEdgeEntry_t entry(pg);

    Error_t errVal = table.Insert(entry);
    assert(errVal == E_OK);
}

// attach iterator to a list
inline void POEdgeIter_t::Attach(POEdgeList_t *list_a)
{
    assert(list_a);
    assert(!list);

    list = list_a;
    list->table.IterStateInit(&state);
}

// detach iterator from attached list
inline void POEdgeIter_t::Detach(void)
{
    assert(list);
    list->table.IterStateDelete(&state);

    list = NULL;
    state = NULL;
}

// next plan group in the list --- NULL at end of list
inline PlanGroup_t *POEdgeIter_t::Next(void)
{
    POEdgeEntry_t x;

    list->table.IterNext(&x, &state);
    return x.PlanGroup();
}

#endif  // __PLAN_H__
