// Volcano implementation

#ifndef __VOLCANO_H__
#define __VOLCANO_H__

#include "typedefs.h"
#include "Memo.h"
#include "Cost.h"
#include "Optimizer.h"

class Equivalence_t;
class PhyPropGroup_t;
class LogicalOp_t;
class PhysicalOp_t;
class SelectScan_t;
class Plan_t;
class PlanGroup_t;

// volcano base class
class Volcano_t : public Optimizer_t {
    Memo_t memo;                // hash table for logical expressions
    int isFullExpand;           // fully expand the dag?
    int isRU;                   // is sharing enabled across queries?
    int curQueryID;             // id of the current query being optimized

    // find best plan for the plangroup
    Plan_t *FindBestPlan(PlanGroup_t *planGroup, Cost_t *totalCost,
                         CostVal_t *costLimit, int isInpIndex, 
                         Order_t *pso = NULL, int *plmap = NULL, 
                         int level = 1, Card_t callCount = 1);

    // find best algorithm plan for the plangroup
    Plan_t *FindBestAlgPlan(PlanGroup_t *planGroup, Cost_t *totalCost,
                            CostVal_t *costLimit, int isInpIndex,
                            Order_t *pso = NULL, int *plmap = NULL, 
                            int level = 1, Card_t callCount = 1);

    // find best enforcer plan for the plangroup
    Plan_t *FindBestEnfPlan(PlanGroup_t *planGroup, Cost_t *totalCost,
                            CostVal_t *costLimit, int isInpIndex,
                            Order_t *pso = NULL, int *plmap = NULL, 
                            int level = 1, Card_t callCount = 1);

    // expand the dag rooted at eq fully by applying all possible
    // transformations
    void ExpandDAG(Equivalence_t *eq);

    // prune the dag rooted at eq
    void PruneDAG(Equivalence_t *eq, CostVal_t costUB);

    // prune off the logical expressions in the given list
    void PruneLogExprs(List_t<LogicalOp_t *>& list);

    // apply the enforcers
    void ApplyEnforcers(PlanGroup_t *planGroup,
        Cost_t *totalCost, Plan_t *bestPlan, CostVal_t *costLimit,
    Order_t *pso, int *plmap, int level, Card_t callCount);

    // add a select operator as a subsumption path
    void AddSelect(Equivalence_t *parEq, Equivalence_t *childEq,
                                                    Predicate_t *pred);

    // add an aggregate operator as a subsumption path
    void AddAggregate(Equivalence_t *parEq, Equivalence_t *childEq,
                                                    Aggregate_t *agg);

    // add a project as subsumption path
    void AddProjection(Equivalence_t *parEq, Equivalence_t *childEq,
                                                    Aggregate_t *agg);

    // add a new subsuming aggregate eq class
    void AddSubsAggregate(Equivalence_t *leftEq, AttrMap_t *leftAttrMap,
                            Equivalence_t *rightEq, AttrMap_t *rightAttrMap,
                            AttrMap_t *groupByAttrMap, Equivalence_t *eq);

    // apply logical subsumption
    void ApplyLogicalSubs(Equivalence_t *eq);

    // expand the dag rooted at eq fully by applying subsumption
    // transformations
    void ExpandSubsumption(Equivalence_t *eq);

public:
    Volcano_t(int isUnify_a, int isFullExpand_a, int isRU_a) :
                        memo(isUnify_a), isFullExpand(isFullExpand_a),
                                        isRU(isRU_a), curQueryID(0) { }

    // logical expression hash table
    Memo_t& Memo(void) { return memo; }

    // instantiate the costs
    virtual Cost_t *CreateCost(void) const = 0;
    virtual Cost_t *CreateCost(PhysicalOp_t *op, CostVal_t opCost) const = 0;

    // expand the dag rooted at rootEq without b&b physical pruning
    PlanGroup_t *ExpressionDAG(Equivalence_t *rootEq, CostVal_t costUB);

    // find best plan for the expression dag rooted at rootEq
    PlanGroup_t *FindBestPlan(Equivalence_t *rootEq,
                              CostVal_t *costLimit, SortOrder_t *reqOrder=NULL);

    // find best plan for the <eq, reqProp> plan group
    PlanGroup_t *FindBestPlan(Equivalence_t *eq, PhyPropGroup_t *reqProp, 
                              Cost_t *totalCost, CostVal_t *costLimit, 
                              int isInputIndexed = 0, Order_t *pso = NULL, 
                              int *plmap = NULL, int level = 1,
                              Card_t callCount = 1);

    // find best algorithm plan for the <eq, reqProp> plan group
    PlanGroup_t *FindBestAlgPlan(Equivalence_t *eq,
                                PhyPropGroup_t *reqProp, Cost_t *totalCost,
                                CostVal_t *costLimit, int isInputIndexed = 0, 
                                Order_t *pso = NULL, 
                                int *plmap = NULL, int level = 1,
                                Card_t callCount = 1);

    // find best enforcer plan for the <eq, reqProp> plan group
    PlanGroup_t *FindBestEnfPlan(Equivalence_t *eq,
                                PhyPropGroup_t *reqProp, Cost_t *totalCost,
                                CostVal_t *costLimit, int isInputIndexed = 0, 
                                Order_t *pso = NULL, int *plmap = NULL, 
                                int level = 1, Card_t callCount = 1);

    // is full expansion enabled?
    int IsFullExpand(void) const
    { return isFullExpand; }

    // is MQO enabled across queries?
    int IsRU(void) const
    { return isRU; }

    // if isRU > 1 consider rev perm
    int IsRevRU(void) const
    { return isRU > 1; }

    // id of the current query being optimized
    int CurQueryID(void) const
    { return curQueryID; }

    // increment the query ID
    void NextQuery(void)
    { curQueryID++; }

    // mark the best plan rooted at pg
    void MarkBestPlan(PlanGroup_t *pg);

    // unmark the best plan rooted at pg
    void UnMarkBestPlan(PlanGroup_t *pg);

    virtual ~Volcano_t(void) { }
};

// ordinary volcano
class VolcanoOrd_t : public Volcano_t {
    // used by SharCost
    CostVal_t SharCostTrav(int travID, PhysicalOp_t *op, int inpNo,
                                PlanGroup_t *pg, CostVal_t *matCost);

    // compute the usecount -- number of times a node is use in the plan
    // given that some nodes are materialized (marked as shared)
    void ComputeUseCount(int travID, PlanGroup_t *pg, int mult);

    // set the isshared flag of the plan nodes based on prev computed usecounts
    void MarkShared(int travID, PlanGroup_t *pg);

    // reset the isshared flag of the plan nodes 
    void UnMarkShared(int travID, PlanGroup_t *pg);

protected:
    // returns the cost of the plan rooted at plan group eliminating the
    // sharing
    CostVal_t SharCost(PlanGroup_t *planGroup);

public:
    VolcanoOrd_t(int isUnify_a, int isFullExpand_a, int isRU_a = 0)
                    : Volcano_t(isUnify_a, isFullExpand_a, isRU_a) { }

    Cost_t *CreateCost(void) const
    { return new CostOrd_t; }

    Cost_t *CreateCost(PhysicalOp_t *, CostVal_t opCost) const
    { return new CostOrd_t(opCost); }

    virtual ~VolcanoOrd_t(void) { }
};

// volcano with common subexpressions in the plan eliminated
class VolcanoShar_t : public VolcanoOrd_t {
    int AddSelectScan(PlanGroup_t *prevInpPG,
                        PlanGroup_t *newInpPG, SelectScan_t *op);

    // apply logical subsumption postpass on the optimal plan
    void ApplyLocalLogicalSubs(int travID, PlanGroup_t *pg);

    void ApplyLogicalSubs(int travID, PlanGroup_t *pg);

    // mark the best plan for pg
    int MarkPlan(int travID, PlanGroup_t *pg);

public:
    VolcanoShar_t(int isUnify_a, int isFullExpand_a, int isRU_a = 0)
                    : VolcanoOrd_t(isUnify_a, isFullExpand_a, isRU_a) { }

    // find best plan for the expression dag rooted at rootEq
    PlanGroup_t *FindBestPlan(Equivalence_t *rootEq, CostVal_t *costLimit,
                              SortOrder_t *reqOrder=NULL);

    virtual ~VolcanoShar_t(void) { }
};

// volcano with common subexpressions in the plan eliminated
// and reuse of results in the best plans of the prev queries
class VolcanoRU_t : public VolcanoShar_t {
public:
    VolcanoRU_t(int isUnify_a, int isFullExpand_a)
                    : VolcanoShar_t(isUnify_a, isFullExpand_a, 1) { }

    virtual ~VolcanoRU_t(void) { }
};

// volcano with common subexpressions in the plan eliminated
// and reuse of results in the best plans of the prev queries
// rev order also considered
class VolcanoRevRU_t : public VolcanoShar_t {
public:
    VolcanoRevRU_t(int isUnify_a, int isFullExpand_a)
                    : VolcanoShar_t(isUnify_a, isFullExpand_a, 2) { }

    virtual ~VolcanoRevRU_t(void) { }
};

#endif // __VOLCANO_H__
