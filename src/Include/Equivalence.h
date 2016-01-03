// equivalence node class

#ifndef __EQUIVALENCE_H__
#define __EQUIVALENCE_H__

#include "Node.h"
#include "List.h"
#include "LogProp.h"
#include "Map.h"

class Memo_t;
class LogicalOp_t;
class PhysicalOp_t;

class PhyPropGroup_t;

class PlanGroup_t;
class Volcano_t;

// equivalence class of logical and physical expressions
class Equivalence_t : public LogicalNode_t {
    static int CurID;                   // id generator
    static int Count;                   // number of eq nodes
    static int TopCount;                // topological ordering count

    int id;                             // node id
    int topNo;                          // topological ordering number
    LogProp_t *logProp;                 // logical property
  
    // Ravi: ---------------  Begin NQO Extension --------------------
    char isOuterVarLogPropSet;
    LogProp_t *outerVarLogProp; // Logical properties of outer variables.

    // Ravi: ---------------  End NQO Extension --------------------


    // Ravi: ------------  End EIO Extension ---------------
    AppendList_t<Order_t *> *favOrds;   // Favorable orders for this expression
    // Ravi: ---------------  End EIO Extension --------------------


    PrependDelList_t<LogicalOp_t *> parents;
                                        // list of parent logical operators
    AppendDelList_t<LogicalOp_t *> logExprs;
                                        // list of child logical expressions
                                        // append only

    PlanGroup_t *planGroupBase;         // points to the plan group with
                                        // no physical properties --
                                        // this forms the least element
                                        // of the partial order of plan
                                        // groups
    int numPlanGroups;                  // number of plan groups
                                        // Ravi: Now this is across all pgBases

    // Added by Ravi
    PlanGroupTable_t pgTable;           // Map of (pso, callCount)->pgBase
                                        // planGroupBase(see above) still
                                        // has the pgBase for pso={}

    int degree;                         // degree of sharing

    char progressCount;                 // in-progress count
    char isExpanded;                    // is this node expanded?
    char isPruned;                      // is this node pruned?
    char isSubsExpanded;                // is this node subsumption-expanded?

    char isAffected;                    // is the eq class affected by
                                        // the changes made -- used in
                                        // greedy incremental
    char isBaseRel;                     // is this a base relation?
    char isCorrelated;                  // is this node correlated?

    CostVal_t costLB;                   // cost lower bound
    Cost_t *useCost;                    // use cost adt

    char isSharable;                    // is this node sharable?
    char isRoot;                        // is this a root of the DAG?

    // traverses the partial order below base, visiting the plan groups
    // whose physical properties are covered by propGroup.
    // If a plan group for propGroup is found, it is returned as *pg
    // else a new plan group for propGroup is created, inserted at the
    // appropriate place in the partial order and returned as *pg
    // returns 1 if the plan group for prop was already present; 0 otherwise
    int Traverse(int travID, SchemaProp_t *schemaProp,
                    PlanGroup_t *base, PhyPropGroup_t *prop, PlanGroup_t **pg,
                 Order_t *gpso, Card_t callCount);

    // insert plan group corresponding to prop between base and next
    void InsertPlanGroup(PhyPropGroup_t *prop,
        PlanGroup_t *base, PlanGroup_t *nextPG, PlanGroup_t **pg,
        Order_t *gpso, Card_t callCount);

    // create the list of degree of sharing given the lists of the children
    // logical op nodes
    // the children lists are topologically sorted wrt the eq nodes
    AppendList_t<DegShar_t *> *CreateDegreeList(void);

    // merge plan group lists
    void MergePlanGroupList(int travID, PlanGroup_t *pg);

public:
    // constructor
    Equivalence_t(LogProp_t *logProp_a, int isBaseRel_a = 0);

    // get the node's id
    int ID(void) const { return id; }

    // check for equality
    int IsEqual(const Equivalence_t *x) const
    {
        assert(x);
        return x->ID() == id;
    }

    // topological number
    int TopNo(void) const
    { return topNo; }

    // next topological count
    void SetTopNo(void)
    { assert(topNo == -1); topNo = TopCount++; }

    // parent list
    List_t<LogicalOp_t *>& ParList(void)
    { return parents; }

    // child logical expression list
    AppendList_t<LogicalOp_t *>& LogExprList(void)
    { return logExprs; }

    // add a parent
    void AddParent(LogicalOp_t *lop);

    // delete a parent
    void DeleteParent(LogicalOp_t *lop);

    // number of parent logical expressions
    int NumParents(void) const
    { return parents.Size(); }

    // add a logexp
    void AddLogExpr(LogicalOp_t *lexp);

    // delete a logexp
    void DeleteLogExpr(LogicalOp_t *lexp);

    // number of logical expressions
    int NumLogExprs(void) const
    { return logExprs.Size(); }

    // get the plan group for a physical property --- add if not present
    PlanGroup_t *PlanGroup(PhyPropGroup_t *propgroup, Order_t *gpso=NULL,
                           Card_t callCount = 1);

    // get the logical property
    LogProp_t *LogicalProp(void) const 
    { return logProp; }

    // plan group base
    PlanGroup_t *PlanGroupBase(void) const
    { return planGroupBase; }

    // number of plan groups
    int NumPlanGroups(void) const
    { return numPlanGroups; }

    // set the degree of sharing
    void SetDegree(int degree_a)
    {
        degree = degree_a;
        assert(degree > 0);
    }

    // degree of sharing
    int Degree(void) const
    { return degree; }

    // is the node in progress?
    int IsInProgress(void) const
    { return progressCount; }

    // mark the node as in progress
    void MarkInProgress(void)
    { assert(progressCount >= 0); progressCount++; }

    // mark the node as in progress
    void UnMarkInProgress(void)
    { progressCount--; assert(progressCount >= 0); }

    // is the node expanded?
    int IsExpanded(void) const
    { return isExpanded; }

    // is the outerVarLogProp set?
    int IsOuterVarLogPropSet(void) const
    { return isOuterVarLogPropSet; }


    // mark the node as expanded
    void MarkExpanded(void)
    { isExpanded = 1; }

    // is the node pruned?
    int IsPruned(void) const
    { return isPruned; }

    // mark the node as pruned
    void MarkPruned(void)
    { isPruned = 1; }

    // is the node subsumption-expanded?
    int IsSubsExpanded(void) const
    { return isSubsExpanded; }

    // mark the node as subsumption-expanded
    void MarkSubsExpanded(void)
    { isSubsExpanded = 1; }

    // is this a base relation
    int IsBaseRel(void) const
    { return isBaseRel; }

    // unify two semantically identical equivalent nodes
    void Unify(Equivalence_t *eq, Memo_t& memo);

    // cost for using the materialized intermediate result
    CostVal_t UseCost(void) const;

    // cost adt for using the materialized intermediate result
    Cost_t *UseCostADT(Volcano_t *opt);

    // cost for materializing the materialized intermediate result
    CostVal_t MatCost(void) const
    {
        if (isBaseRel) return 0;
        Card_t relSize_Blocks = logProp->RelSize_Blocks();
        return Config_t::Cost(WriteTime*relSize_Blocks, relSize_Blocks);
    }

    // for greedy incremental
    // is the eq class affected by the changes?
    int IsAffected(void) const
    { return isAffected; }

    // mark the eq class as affected
    void MarkAffected(void)
    { assert(!isAffected); isAffected = 1; }

    // mark the eq class as not affected
    void UnMarkAffected(void)
    { assert(isAffected); isAffected = 0; }

    // is the eq class correlated?
    int IsCorrelated(void) const
    { return isCorrelated; }

    // mark the eq class as correlated
    void MarkCorrelated(void)
    { isCorrelated = 1; }

    // is the node sharable
    int IsSharable(void) const
    { return Config_t::IsUnifShar() ? isSharable
                                    : (parents.Size() > 1 && degree > 1); }

    // mark the eq class as shared
    void MarkSharable(void)
    { isSharable = 1; }

    // is this the root of the DAG
    int IsRoot(void) const
    { return isRoot; }

    // mark as the root of the DAG
    void SetRoot(void)
    { isRoot = 1; }

    // lower bound on the computation cost
    CostVal_t CostLB(void) const
    { assert(costLB >= 0); return costLB; }

    // print for debugging
    void PrintExpressionDAG(int indentLevel) const;
    
    // print DAG in XML format
    void PrintExpressionDAGXML(int indentLevel) const;

    // count of the number of instances
    static int NodeCount(void)
    { return Count; }

    // count of the total number of instances generated
    static int TotalNodeCount(void)
    { return CurID; }

    // list of nested nodes
    int NQEqList(List_t<Equivalence_t *> *list, int isNested = 0);

    // Ravi: ------------  Begin NQO Extension ---------------

    /**
     * Populates the outer variable logical properties - outerVarLogProp.
     */
    void SetOuterVarLogProp();

    LogProp_t *OuterVarLogProp() {
        return outerVarLogProp;
    }

    // this->outerVarLogProp = this->outerVarLogProp UNION aLogProp
    void UnionOuterVarLogProp(LogProp_t *aLogProp);

    List_t<Order_t *>* GetInterestingOrders(int *plmap, int level);
    List_t<Order_t *>* GetFavorableOrders();

    /*
    void PushOnScope(LogProp_t *logicalProp) {
        outerVariables.Insert(logicalProp);
    }

    LogProp_t * PopFromScope() {
        outerVariables.DeleteTop();
    }
    */
    // Ravi: ------------  End NQO Extension ---------------

    // clean up --- free the allocated memory
    ~Equivalence_t(void);
};

#endif // __EQUIVALENCE_H__
