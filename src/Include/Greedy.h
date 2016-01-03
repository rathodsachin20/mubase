// Greedy heursitic for DAG optimization

#ifndef __GREEDY_H__
#define __GREEDY_H__

#include "typedefs.h"
#include "Cost.h"
#include "List.h"
#include "Heap.h"
#include "Plan.h"
#include "Volcano.h"
#include "DAGOpt.h"
#include "HeapKey.h"

class Equivalence_t;

const int SHARHEAPSIZE = 5000;
const int PROPHEAPSIZE = 1000;
const int PGTABSIZE = 15;

// greedy heuristic
class Greedy_t : public DAGOpt_t {
    char isSharPrune;                   // prune using sharable nodes?

    Heap_t<SharKey_t> sharHeap;         // sharable node heap -- ordered
                                        // by benefit

    Heap_t<PropKey_t> propHeap;         // propagation heap -- ordered
                                        // by topological number

    // used for local computation at each equivalence class
    // zeroth node is the dummy node representing the external relations
    // and materialized views; 1 ... numPG are the nodes for this eq class
    PlanGroup_t *pgList[PGTABSIZE];
    int bestExtPlan[PGTABSIZE];
    Plan_t *table[PGTABSIZE][PGTABSIZE];

    // takes care of supernode formation -- supernodes are maintained as
    // partitions of index as in union-find
    int superIndex[PGTABSIZE];
    int bestPlan[PGTABSIZE]; // contains the solution

    // register that the configuration for the eq class needs to be
    // reevaluated
    void RegisterChange(Equivalence_t *eq);

    // propagate changes -- does duplicate merging and calls Propagate
    // does not propagate along infeasible eq nodes
    void PropagateChanges(CostVal_t bestCost);

    // update the cost of a plangroup and propagate the update to plans that
    // can use it
    void Propagate(Equivalence_t *eq);

    // populate the sharHeap with the sharable plan groups
    void PopulateSharHeap(Equivalence_t *rootEq);

    // assign topological numbers, mark sharable nodes and populate sharheap
    void PopulateSharHeapUnifShar(Equivalence_t *eq, int markSharable = 0);

    // populate the sharHeap with all the plan groups
    void PopulateSharHeapNoPrune(Equivalence_t *eq);

    // recursively inserts all plan groups covering with given pg into the
    // heap h
    void HeapInsert(int travID,
                        Heap_t<SharKey_t> *h, PlanGroup_t *pg, int isNested);

    // recursively inserts all plan groups covering with given pg into the
    // shar heap with infinity benefit
    void HeapInsertUniformBenefit(int travID,
            Heap_t<SharKey_t> *h, PlanGroup_t *pg);

    // materialize the given plan group and update the best plan
    void Materialize(PlanGroup_t *pg);

    // unmaterialize the given plan group and update the best plan
    void UnMaterialize(PlanGroup_t *pg);

    // update the best alg plan for the plan group
    Plan_t *UpdateBestAlgPlans(int travId, PlanGroup_t *pg);

    // create the table for min cost arb computation
    void CreateTable(int travId, PlanGroup_t *pg);

    // take closure of the plans
    void IntTableClosure(PlanGroup_t *pg);

    // find min cost arb
    void FindMinCostArb(void);

    // assign costs to the plans
    // CostVal_t AssignPlanCost(PlanGroup_t *pg);

    // set the best plans for the computed configuration
    void SetBestPlans(void);
    Plan_t *SetBestPlan(int pgIndex);
    Plan_t *SetBestExtPlan(int pgIndex);
    Plan_t *SetBestUnMarkedPlan(int pgIndex);

    // update the parent plans and the containing equivalence classes,
    // if necessary
    void UpdateParents(void);

    // supernode representative
    int SuperPG(int i);

    // merge supernodes
    // superPG for a superNode is the pg with the smallest cost ext plan
    void MergeSuperPG(int i, int j);

    // cost for an entry in the plan matrix
    CostVal_t ComputationCost(int rowIndex, int colIndex);

    // index for a plangroup in pgList
    int Index(PlanGroup_t *pg);

    // recompute the cost of an alg plan
    void RecomputeAlgCost(Plan_t *plan);

    // recompute the cost of an enf plan
    void RecomputeEnfCost(Plan_t *plan);

    // returns the total cost of the plan rooted at plan group
    CostVal_t TotalCost(PlanGroup_t *planGroup);

    // used by TotalCost
    CostVal_t TotalCostTrav(int travID, PhysicalOp_t *parOp, int inpNo,
                                        PlanGroup_t *pg, CostVal_t *matCost);

    // returns the min cost covering node for pg
    void MinCostCoveringPG(Algorithm_t *op, PlanGroup_t *pg, int inpNo,
                            PlanGroup_t **covPG, CostVal_t *covCost);

    void TravCoveringPG(int travID, Algorithm_t *op, PlanGroup_t *pg,
                int inpNo, PlanGroup_t **retBestPG, CostVal_t *retBestCost);

    void CoveredMarkedPGList(PlanGroup_t *pg,
                                PlanGroup_t *coveredMarkedPG[],
                                            int *numCoveredMarkedPG);
    void TravCoveredPGList(int travID, PlanGroup_t *pg,
                                PlanGroup_t *coveredMarkedPG[],
                                            int *numCoveredMarkedPG);
    // add pg and recursively all covering pgs
    void AddPG(int travID, PlanGroup_t *pg,
                        PlanGroup_t **allPGList, int *count);

    void PopulateAllPGList(int travID, Equivalence_t *eq,
                                    PlanGroup_t *allPGList[], int *count);

public:
    Greedy_t(int isUnify_a, int isSharPrune_a = 1)
                : DAGOpt_t(isUnify_a), isSharPrune(isSharPrune_a),
                    sharHeap(SHARHEAPSIZE), propHeap(PROPHEAPSIZE) { }

    // search algorithm wrapper for preprocessing
    PlanGroup_t *FindBestPlan(Equivalence_t *eq, CostVal_t *costLimit,
                              SortOrder_t *reqOrder=NULL);

    // greedy heuristic without monotonicity heuristic
    PlanGroup_t *FindBestNoMonoPlan(Equivalence_t *rootEq, CostVal_t *costLimit);

    ~Greedy_t(void) { }
};


#endif  // __GREEDY_H__
