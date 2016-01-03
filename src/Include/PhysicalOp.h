// Physical Operator class implementation

#ifndef __PHYSICALOP_H__
#define __PHYSICALOP_H__

#include "typedefs.h"
#include "Config.h"
#include "Node.h"
#include "Plan.h"
#include "Equivalence.h"
#include "LogicalOp.h"
#include "iter.h"

// type of the physical property
enum PhysicalOpType_t {
    ALGORITHM_T,
    ENFORCER_T
};

// base class for all physical operators
class PhysicalOp_t {
    static int CurID;           // id generator
    static int Count;           // count of the number of instances

    int id;                     // id of the node
    int numInputs;              // number of input
    PlanGroup_t **inputs;       // input plan groups
    PlanGroup_t **actualInputs; // actual input plan groups --- can be a
                                // plangroup covering the corresponding
                                // plangroup in input
    CostVal_t costVal;          // cost of the operator
    Plan_t *plan;               // plan with this operator as root
    // Ravi: Added
    Card_t callCount;
    
public:
    PhysicalOp_t(int numInputs_a)
          : id(CurID++), numInputs(numInputs_a),
                inputs(NULL), actualInputs(NULL), costVal(-1), plan(NULL)
    { assert(numInputs >= 0); Count++; callCount=-1;}

    // get the node's id
    int ID(void) const { return id; }

    // check for equality
    int IsEqual(const PhysicalOp_t *x) const
    {
        assert(x);
        return x->ID() == id;
    }

    void SetCallCount(Card_t cc)
    { callCount = cc; }

    Card_t CallCount(void) const
    { return callCount; }
 
    // operator type
    virtual PhysicalOpType_t Type(void) const = 0;

    // number of inputs
    int NumInputs(void) const
    { return numInputs; }

    // set the i-th input
    virtual void SetInput(int i, PlanGroup_t *pg)
    {
        assert(pg);
        assert(i >= 0 && i < numInputs);

        if( !inputs ) {
            inputs = new PlanGroup_t *[numInputs];
            for( int j = 0 ; j < numInputs ; j++ )
                inputs[j] = NULL;
        }

        // assert(inputs[i] == NULL);
        inputs[i] = pg;
        Refer(inputs[i]);
    }

    // i-th input
    PlanGroup_t *Input(int i) const
    {
        assert(i >= 0 && i < numInputs);
        assert(inputs);
        return inputs[i];
    }

    // set the i-th actualInput
    virtual void SetActualInput(int i, PlanGroup_t *pg)
    {
        assert(pg);
        assert(i >= 0 && i < numInputs);

        if( !actualInputs ) {
            actualInputs = new PlanGroup_t *[numInputs];
            for( int j = 0 ; j < numInputs ; j++ )
                actualInputs[j] = NULL;
        }

        PlanGroup_t *oldPG = actualInputs[i];

        actualInputs[i] = pg;
        Refer(actualInputs[i]);

        if( oldPG ) DeRefer(oldPG);
    }

    // i-th actualInput
    PlanGroup_t *ActualInput(int i) const
    {
        assert(i >= 0 && i < numInputs);
        if( !actualInputs )
            return NULL;
        return actualInputs[i];
    }

    // set the operator cost
    void SetCostVal(CostVal_t costVal_a)
    {
        assert(costVal_a >= 0);
        costVal = costVal_a;
    }

    // recompute cost val
    void RecomputeCostVal(void)
    { costVal = NQCostVal() + LocalCostVal(); }

    // operator cost
    CostVal_t CostVal(void) const
    {
        assert(costVal >= 0);
        return costVal;
    }

    // nested query cost val
    virtual CostVal_t NQCostVal(void)
    { return 0; }

    // local cost val
    virtual CostVal_t LocalCostVal(void) = 0;

    // associate with a plan
    void SetPlan(Plan_t *plan_a)
    {
        assert(plan_a);
        plan = plan_a;
    }

    // associated plan
    Plan_t *Plan(void) const
    {
        assert(plan);
        return plan;
    }

    // associated plangroup
    PlanGroup_t *PlanGroup(void) const
    {
        assert(plan);
        PlanGroup_t *pg = plan->PlanGroup();
        assert(pg);

        return pg;
    }

    // print plan
    virtual void PrintPlan(int indentLevel) const = 0;

    // print functions
    virtual void PrintName(void) const = 0;
    void PrintSQL(void) const;
    virtual void PrintSQLStmt(void) const = 0;

// added by sachin
    virtual void open() = 0;
    virtual Record* next() = 0;
    virtual void close() = 0;

//End - added by sachin

    // count of the total number of physical ops generated
    static int TotalNodeCount(void)
    { return CurID; }

    // count of the number of physical ops
    static int NodeCount(void)
    { return Count; }

    virtual ~PhysicalOp_t(void)
    {
        if( inputs ) {
            for( int i = 0 ; i < numInputs ; i++ )
                if( inputs[i] ) DeRefer(inputs[i]);

            delete[] inputs;
        }

        if( actualInputs ) {
            for( int i = 0 ; i < numInputs ; i++ )
                if( actualInputs[i] ) DeRefer(actualInputs[i]);

            delete[] actualInputs;
        }

        Count--;
    }
};

// derived classes
class Enforcer_t;       // enforcer -- enforces a physical property
class Algorithm_t;      // algorithm -- implementation of a logical operator

// type of enforcer
enum EnforcerType_t {
    SORT_T,
    INDEX_T,
    INDEXSORT_T
};

// enforcer -- enforces a physical property
class Enforcer_t : public PhysicalOp_t {
public:
    Enforcer_t(void) : PhysicalOp_t(1)
    { }

    // operator type
    PhysicalOpType_t Type(void) const
    { return ENFORCER_T; }

    // enforcer type
    virtual EnforcerType_t EnforcerType(void) const = 0;

    // set the i-th input
    void SetInput(int i, PlanGroup_t *pg)
    {
        assert(pg);
        PhysicalOp_t::SetInput(i, pg);
        pg->AddEnfParent(this);
    }

    // print plan
    void PrintPlan(int indentLevel = 0) const;

    // print name
    virtual void PrintName(void) const = 0;
    virtual void PrintSQLStmt(void) const = 0;
    
    virtual void open() = 0;
    virtual Record* next() = 0;
    virtual void close() = 0;
    
    virtual ~Enforcer_t(void)
    { }
};

// derived classes
class Sort_t;           // sort enforcer
class Index_t;          // index enforcer
class IndexSort_t;      // index-sort enforcer

// sort enforcer class
class Sort_t : public Enforcer_t {
    SortOrder_t *sortOrderProp; // sort order property enforced
    SortOrder_t *partialSortOrderProp; // partially satisfied order of the i/p
    static float GetCombinedSelectivity(Order_t *order, LogProp_t *lp);
    static Card_t GetDistinctValues(Order_t *order, LogProp_t *lp);

public:
    Sort_t(SortOrder_t *sortOrderProp_a) : sortOrderProp(sortOrderProp_a)
    { Refer(sortOrderProp); partialSortOrderProp = NULL; }

    Sort_t(SortOrder_t *sortOrderProp_a, SortOrder_t *partialOrder) : 
          sortOrderProp(sortOrderProp_a), partialSortOrderProp(partialOrder)
    { Refer(sortOrderProp); Refer(partialSortOrderProp);}

    SortOrder_t *SortOrder(void) const
    { return sortOrderProp; }

    // enforcer type
    EnforcerType_t EnforcerType(void) const
    { return SORT_T; }

    // cost model
    static CostVal_t Cost(Order_t *prop, LogProp_t *lp, Order_t *partialProp=NULL);

    // local cost val
    CostVal_t LocalCostVal(void)
    {
        PlanGroup_t *inpPG = Input(0);
        assert(inpPG);

        LogProp_t *lp = inpPG->LogicalProp();
        assert(lp);

        return Cost(sortOrderProp, lp);
    }

    // print name
    void PrintName(void) const;
    void PrintSQLStmt(void) const;
    
    void open(){};
    Record* next(){return NULL;}
    void close(){};
    
    ~Sort_t(void)
    { DeRefer(sortOrderProp); }
};

// index enforcer class
class Index_t : public Enforcer_t {
    IndexOrder_t *indexOrderProp;       // index order property enforced

public:
    Index_t(IndexOrder_t *indexOrderProp_a) : indexOrderProp(indexOrderProp_a)
    { Refer(indexOrderProp); }

    IndexOrder_t *IndexOrder(void) const
    { return indexOrderProp; }

    // enforcer type
    EnforcerType_t EnforcerType(void) const
    { return INDEX_T; }

    // cost model
    static CostVal_t Cost(LogProp_t *lp);

    // local cost val
    CostVal_t LocalCostVal(void)
    {
        PlanGroup_t *inpPG = Input(0);
        assert(inpPG);

        LogProp_t *lp = inpPG->LogicalProp();
        assert(lp);

        return Cost(lp);
    }

    // print name
    void PrintName(void) const;
    void PrintSQLStmt(void) const;
    
    void open(){};
    Record* next(){return NULL;}
    void close(){};
    

    ~Index_t(void)
    { DeRefer(indexOrderProp); }
};

// index enforcer class
class IndexSort_t : public Enforcer_t {
    IndexOrder_t *indexOrderProp;       // index order property enforced

public:
    IndexSort_t(IndexOrder_t *indexOrderProp_a)
                        : indexOrderProp(indexOrderProp_a)
    { Refer(indexOrderProp); }

    IndexOrder_t *IndexOrder(void) const
    { return indexOrderProp; }

    // enforcer type
    EnforcerType_t EnforcerType(void) const
    { return INDEXSORT_T; }

    // cost model
    static CostVal_t Cost(Order_t *prop, LogProp_t *lp);

    // local cost val
    CostVal_t LocalCostVal(void)
    {
        PlanGroup_t *inpPG = Input(0);
        assert(inpPG);

        LogProp_t *lp = inpPG->LogicalProp();
        assert(lp);

        return Cost(indexOrderProp, lp);
    }

    // print name
    void PrintName(void) const;
    void PrintSQLStmt(void) const;
    
    void open(){};
    Record* next(){return NULL;}
    void close(){};
    
    ~IndexSort_t(void)
    { DeRefer(indexOrderProp); }
};

// type of algorithm
enum AlgoType_t {
    RELSCAN_T,
    AGGREGATESCAN_T,
    AGGREGATEHASH_T,
    PROJECTSCAN_T,
    SELECTSCAN_T,
    INDEXEDSELECT_T,
    MERGEJOIN_T,
    NESTEDLOOPSJOIN_T,
    INDEXEDNESTEDLOOPSJOIN_T,
    MERGE_T,
    BASICAPPLY_T
};

// algorithm --- physical implementation of a logical operator
class Algorithm_t : public PhysicalOp_t {
public:
    Algorithm_t(int numInputs_a) : PhysicalOp_t(numInputs_a)
    { assert(numInputs_a >= 0); }

    // operator type
    PhysicalOpType_t Type(void) const
    { return ALGORITHM_T; }

    // return enforcer type
    virtual AlgoType_t AlgoType(void) const = 0;

    // set the i-th input
    void SetInput(int i, PlanGroup_t *pg)
    {
        assert(pg);
        PhysicalOp_t::SetInput(i, pg);
        pg->AddAlgParent(this);
    }

    // print plan
    void PrintPlan(int indentLevel = 0) const;

    // print name
    virtual void PrintName(void) const = 0;
    virtual void PrintSQLStmt(void) const = 0;
    
    virtual void open() = 0;
    virtual Record* next() = 0;
    virtual void close() = 0;
    

    virtual Predicate_t *Predicate(void) const
    { return NULL; }

    virtual ~Algorithm_t(void)
    { }
};

// relation scan operator
class RelScan_t : public Algorithm_t {
    char *relName;
    RIterator* iter;            //Iterator to iterate through records

public:
    RelScan_t(Rel_t *rel_a) : Algorithm_t(0), relName(NULL)
    {
        assert(rel_a);
        const char *name_a = rel_a->Name();
        if( name_a )
            relName = strdup(name_a);
    }

    //! Function added by Ramu to get the relation name for the PQDAG - pgPlan conversion 04/04/04
    char* RelName(void) {
            return(relName);
    }

    AlgoType_t AlgoType(void) const
    { return RELSCAN_T; }

    // cost for reading
    static CostVal_t Cost(LogProp_t *lp);

    // cost for materializing
    static CostVal_t MatCost(LogProp_t *lp);

    // local cost val
    CostVal_t LocalCostVal(void)
    {
        PlanGroup_t *pg = PlanGroup();
        assert(pg);

        LogProp_t *lp = pg->LogicalProp();
        assert(lp);

        return Cost(lp);
    }

    // print name
    void PrintName(void) const;
    void PrintSQLStmt(void) const;
    
    void open();
    Record* next();
    void close();
    RIterator* Iter()
    { return iter; }
    
    ~RelScan_t(void)
    {
        if( relName )
            free(relName);
    }
};

// projection operator
class ProjectScan_t : public Algorithm_t {
    AttrMap_t *projMap; // projection list --
                        // maps input schema to output schema
    PhysicalOp_t* phyOp; // input physical operator (in best plan)
    int* attrPos;        //maps attributes to index in input physicalOp
public:
    ProjectScan_t(Project_t *proj_a) : Algorithm_t(1), phyOp(NULL),
    attrPos(NULL)
    {
        assert(proj_a);
        projMap = proj_a->AttrMap();
        Refer(projMap);
        
        //PlanGroup_t *inpPG = Input(0);
        //if(!inpPG)
        //    cout<<"inpPG NULL\n";
        //assert(inpPG);
        /*
        Plan_t *plan = inpPG->BestPlan();
        if( !plan ){
            cout<<"BestPlan NULL.Assigning BestAlgPlan\n";
            plan = inpPG->BestAlgPlan();
        }
        else if( plan->IsFailed() ){
            cout<<"BestApgPlan Failed.Assiging BestAlgPlan\n";
            plan = inpPG->BestAlgPlan();
        }*/
        //assert(plan);
        //assert(!plan->IsFailed());
        
        //phyOp = plan->Root();
    }

    AlgoType_t AlgoType(void) const
    { return PROJECTSCAN_T; }

    AttrMap_t *AttrMap(void) const
    { return projMap; }
    
    PhysicalOp_t* PhyOp(){
        return phyOp;
    }
    
    // cost model
    static CostVal_t Cost(LogProp_t *inpLP, LogProp_t *outLP);

    // local cost val
    CostVal_t LocalCostVal(void)
    {
        PlanGroup_t *inpPG = Input(0);
        assert(inpPG);

        LogProp_t *inpLP = inpPG->LogicalProp();
        assert(inpLP);

        PlanGroup_t *outPG = PlanGroup();
        assert(outPG);

        LogProp_t *outLP = outPG->LogicalProp();
        assert(outLP);

        // Ravi:Todo:See the implementation of all these Cost functions
        return Cost(inpLP, outLP);
    }

    // print name
    void PrintName(void) const;
    void PrintSQLStmt(void) const;
    
    void open();
    Record* next();
    void close();

    ~ProjectScan_t(void)
    { DeRefer(projMap); }
};

// aggregation operator
class AggregateHash_t : public Algorithm_t {
    AttrMap_t *groupByAttrMap;  // maps input schema to output schema
    AttrMap_t *aggAttrMap;      // maps aggregated attributes to output schema

public:
    AggregateHash_t(Aggregate_t *agg_a)
        : Algorithm_t(1), groupByAttrMap(NULL), aggAttrMap(NULL)
    { 
        assert(agg_a);

        groupByAttrMap = agg_a->GroupByAttrMap();
        Refer(groupByAttrMap);

        aggAttrMap = agg_a->AggAttrMap();
        Refer(aggAttrMap);
    }

   // Changed by Ramu
    AttrMap_t* GroupByAttrMap() {
            return groupByAttrMap;
    }

    AttrMap_t* AggAttrMap() {
            return aggAttrMap;
    }
   // End of changed by Ramu

    AlgoType_t AlgoType(void) const
    { return AGGREGATEHASH_T; }

    // cost model
    static CostVal_t Cost(LogProp_t *inpLP, LogProp_t *outLP);

    // local cost val
    CostVal_t LocalCostVal(void)
    {
        PlanGroup_t *inpPG = Input(0);
        assert(inpPG);

        LogProp_t *inpLP = inpPG->LogicalProp();
        assert(inpLP);

        PlanGroup_t *outPG = PlanGroup();
        assert(outPG);

        LogProp_t *outLP = outPG->LogicalProp();
        assert(outLP);

        return Cost(inpLP, outLP);
    }

    // print name
    void PrintName(void) const;
    void PrintSQLStmt(void) const;
    
    void open(){};
    Record* next(){return NULL;}
    void close(){};

    ~AggregateHash_t(void)
    {
        DeRefer(groupByAttrMap);
        DeRefer(aggAttrMap);
    }
};

// aggregation operator
class AggregateScan_t : public Algorithm_t {
    AttrMap_t *groupByAttrMap;  // maps input schema to output schema
    AttrMap_t *aggAttrMap;      // maps aggregated attributes to output schema

public:
    AggregateScan_t(Aggregate_t *agg_a)
        : Algorithm_t(1), groupByAttrMap(NULL), aggAttrMap(NULL)
    { 
        assert(agg_a);

        groupByAttrMap = agg_a->GroupByAttrMap();
        Refer(groupByAttrMap);

        aggAttrMap = agg_a->AggAttrMap();

        Refer(aggAttrMap);
    }

    AlgoType_t AlgoType(void) const
    { return AGGREGATESCAN_T; }

    // Changes by Ramu
    AttrMap_t* GroupByAttrMap() {
            return groupByAttrMap;
    }

    AttrMap_t* AggAttrMap() {
            return aggAttrMap;
    }
    // End of changes by Ramu

    // cost model
    static CostVal_t Cost(LogProp_t *inpLP, LogProp_t *outLP);

    // local cost val
    CostVal_t LocalCostVal(void)
    {
        PlanGroup_t *inpPG = Input(0);
        assert(inpPG);

        LogProp_t *inpLP = inpPG->LogicalProp();
        assert(inpLP);

        PlanGroup_t *outPG = PlanGroup();
        assert(outPG);

        LogProp_t *outLP = outPG->LogicalProp();
        assert(outLP);

        return Cost(inpLP, outLP);
    }

    // print name
    void PrintName(void) const;
    void PrintSQLStmt(void) const;
    
    void open(){};
    Record* next(){return NULL;}
    void close(){};

    ~AggregateScan_t(void)
    {
        DeRefer(groupByAttrMap);
        DeRefer(aggAttrMap);
    }
};

// filter algorithm -- base class for selects and joins
class FilterAlgo_t : public Algorithm_t {
    Predicate_t *pred;

public:
    FilterAlgo_t(int numInp) : Algorithm_t(numInp), pred(NULL)
    { }

    void SetPredicate(Predicate_t *pred_a)
    {
        if( pred_a ) Refer(pred_a);
        if( pred ) DeRefer(pred);
        pred = pred_a;
    }

    Predicate_t *Predicate(void) const
    { return pred; }

    // nested query cost val
    virtual CostVal_t NQCostVal(void)
    {
        InExpr_t *inP = (InExpr_t *) pred->FindIn();    
        if( !inP )
            return 0;

        PlanGroup_t *pg = PlanGroup();
        assert(pg);

        Equivalence_t *eq = pg->EqClass();
        assert(eq);

        LogProp_t *lp = eq->LogicalProp();
        assert(lp);

        Card_t dist = inP->NumDistinct(lp);

        PlanGroup_t *inpPlanGroup = inP->InpPlanGroup();
        assert(inpPlanGroup);

        Plan_t *inpBestPlan = inpPlanGroup->BestPlan();
        assert(inpBestPlan);
        assert(!inpBestPlan->IsFailed()); // valid plan

        return dist * inpBestPlan->CostVal();
    }
    
    void open(){};
    Record* next(){return NULL;}
    void close(){};

    virtual ~FilterAlgo_t(void)
    { if( pred ) DeRefer(pred); }
};

// selection operator
class SelectScan_t : public FilterAlgo_t {
    char retainState;
    char isInProgress;

public:
    SelectScan_t(Select_t *sel_a) : FilterAlgo_t(1), isInProgress(0)
    {
        assert(sel_a);
        Predicate_t *pred_a = sel_a->Predicate();
        SetPredicate(pred_a);
    SetRetainState(0);
    }

    void SetRetainState(int s)
    { retainState = s; }

    AlgoType_t AlgoType(void) const
    { return SELECTSCAN_T; }

    // cost model
    static CostVal_t Cost(LogProp_t *inpLP, LogProp_t *outLP);

    // local cost val
    CostVal_t LocalCostVal(void)
    {
        PlanGroup_t *inpPG = Input(0);
        assert(inpPG);

        LogProp_t *inpLP = inpPG->LogicalProp();
        assert(inpLP);

        PlanGroup_t *outPG = PlanGroup();
        assert(outPG);

        LogProp_t *outLP = outPG->LogicalProp();
        assert(outLP);

        return Cost(inpLP, outLP);
    }

    // print name
    void PrintName(void) const;
    void PrintSQLStmt(void) const;
    
    void open(){};
    Record* next(){return NULL;}
    void close(){};

    int IsInProgress(void) const
    { return isInProgress; }

    void MarkInProgress(void)
    { isInProgress = 1; }

    ~SelectScan_t(void)
    { }
};

// indexed select operator
class IndexedSelect_t : public FilterAlgo_t {
public:
    IndexedSelect_t(Select_t *select_a) : FilterAlgo_t(1)
    {
        assert(select_a);
        Predicate_t *pred_a = select_a->Predicate();
        SetPredicate(pred_a);
    }

    AlgoType_t AlgoType(void) const
    { return INDEXEDSELECT_T; }

    // cost model
    static CostVal_t Cost(LogProp_t *inpLP, LogProp_t *outLP);

    // local cost val
    CostVal_t LocalCostVal(void)
    {
        PlanGroup_t *inpPG = Input(0);
        assert(inpPG);

        LogProp_t *inpLP = inpPG->LogicalProp();
        assert(inpLP);

        PlanGroup_t *outPG = PlanGroup();
        assert(outPG);

        LogProp_t *outLP = outPG->LogicalProp();
        assert(outLP);

        return Cost(inpLP, outLP);
    }

    // print name
    void PrintName(void) const;
    void PrintSQLStmt(void) const;
    
    void open(){};
    Record* next(){return NULL;}
    void close(){};

    ~IndexedSelect_t(void)
    { }
};

// merge-join operator
class MergeJoin_t : public FilterAlgo_t {
public:
    MergeJoin_t(Join_t *join_a) : FilterAlgo_t(2)
    {
        assert(join_a);
        Predicate_t *pred_a = join_a->Predicate();
        SetPredicate(pred_a);
    }

    AlgoType_t AlgoType(void) const
    { return MERGEJOIN_T; }

    // cost model
    static CostVal_t Cost(SortOrder_t *leftSortOrder, SortOrder_t *rightSorder,
                    LogProp_t *leftLP, LogProp_t *rightLP, LogProp_t *outLP);

    // local cost val
    CostVal_t LocalCostVal(void)
    {
        PlanGroup_t *leftInpPG = Input(0);
        assert(leftInpPG);
        PlanGroup_t *rightInpPG = Input(1);
        assert(rightInpPG);

        PhyPropGroup_t *leftPhyProp = leftInpPG->PropGroup();
        assert(leftPhyProp);
        PhyPropGroup_t *rightPhyProp = rightInpPG->PropGroup();
        assert(rightPhyProp);

        ListIter_t<PhyProp_t *> iter;

        List_t<PhyProp_t *>& leftPropList = leftPhyProp->PropList();
        SortOrder_t *leftSortOrder = NULL;
        iter.Attach(&leftPropList);
        while( !iter.IsEnd() ) {
            PhyProp_t *p = iter.Next();
            assert(p);

            if( p->Type() == SORTORDER_T ) {
                leftSortOrder = (SortOrder_t *) p;
                break;
            }
        }
        assert(leftSortOrder);

        List_t<PhyProp_t *>& rightPropList = rightPhyProp->PropList();
        SortOrder_t *rightSortOrder = NULL;
        iter.Attach(&rightPropList);
        while( !iter.IsEnd() ) {
            PhyProp_t *p = iter.Next();
            assert(p);

            if( p->Type() == SORTORDER_T ) {
                rightSortOrder = (SortOrder_t *) p;
                break;
            }
        }
        assert(rightSortOrder);

        LogProp_t *leftLP = leftInpPG->LogicalProp();
        assert(leftLP);
        LogProp_t *rightLP = rightInpPG->LogicalProp();
        assert(rightLP);

        PlanGroup_t *outPG = PlanGroup();
        assert(outPG);
        LogProp_t *outLP = outPG->LogicalProp();
        assert(outLP);

        return Cost(leftSortOrder, rightSortOrder, leftLP, rightLP, outLP);
    }

    // print name
    void PrintName(void) const;
    void PrintSQLStmt(void) const;
    
    void open(){};
    Record* next(){return NULL;}
    void close(){};

    ~MergeJoin_t(void)
    { }
};

// nested-loops-join operator
class NestedLoopsJoin_t : public FilterAlgo_t {
PhysicalOp_t *lPhyOp, *rPhyOp;  //Input Physical Ops
int newOuterRec;                //to maintain iterator state
Record* outerRec;                
    
public:
    NestedLoopsJoin_t(Join_t *join_a) : FilterAlgo_t(2), lPhyOp(NULL),
        rPhyOp(NULL), newOuterRec(1), outerRec(NULL)
    {
        assert(join_a);
        Predicate_t *pred_a = join_a->Predicate();
        SetPredicate(pred_a);
     /*
     //added by sachin   
        PlanGroup_t *leftInpPG = Input(0);
        assert(leftInpPG);

        PlanGroup_t *rightInpPG = Input(1);
        assert(rightInpPG);
        
        Plan_t *lPlan = leftInpPG->BestPlan();
            if( !lPlan )
                lPlan = leftInpPG->BestAlgPlan();
            else if( lPlan->IsFailed() )
                lPlan = leftInpPG->BestAlgPlan();
            assert(lPlan);
            assert(!lPlan->IsFailed());

        Plan_t *rPlan = rightInpPG->BestPlan();
            if( !rPlan )
                rPlan = rightInpPG->BestAlgPlan();
            else if( rPlan->IsFailed() )
                rPlan = rightInpPG->BestAlgPlan();
            assert(rPlan);
            assert(!rPlan->IsFailed());
            
        lPhyOp = lPlan->Root();
        rPhyOp = rPlan->Root();
    //end - added by sachin 
    */
    }

//added by sachin    
    PhysicalOp_t* LPhyOp()
    {return lPhyOp;}
    
    PhysicalOp_t* RPhyOp()
    {return rPhyOp;}
    
    void SetLPhyOp(PhysicalOp_t* inpOp){
        lPhyOp = inpOp;
    }
    
    void SetRPhyOp(PhysicalOp_t* inpOp){
        rPhyOp = inpOp;
    }
//end - added by sachin

    AlgoType_t AlgoType(void) const
    { return NESTEDLOOPSJOIN_T; }

    // cost model
    static CostVal_t Cost(LogProp_t *leftLp, LogProp_t *rightLP,
                                                LogProp_t *outLP);

    // local cost val
    CostVal_t LocalCostVal(void)
    {
        PlanGroup_t *leftInpPG = Input(0);
        assert(leftInpPG);

        LogProp_t *leftLP = leftInpPG->LogicalProp();
        assert(leftLP);

        PlanGroup_t *rightInpPG = Input(1);
        assert(rightInpPG);

        LogProp_t *rightLP = rightInpPG->LogicalProp();
        assert(rightLP);

        PlanGroup_t *outPG = PlanGroup();
        assert(outPG);

        LogProp_t *outLP = outPG->LogicalProp();
        assert(outLP);

        return Cost(leftLP, rightLP, outLP);
    }

    // print name
    void PrintName(void) const;
    void PrintSQLStmt(void) const;
    
    void open();
    Record* next();
    void close();

    ~NestedLoopsJoin_t(void)
    { }
};

// indexed nested-loops-join operator
class IndexedNestedLoopsJoin_t : public FilterAlgo_t {
public:
    IndexedNestedLoopsJoin_t(Join_t *join_a) : FilterAlgo_t(2)
    {
        assert(join_a);
        Predicate_t *pred_a = join_a->Predicate();
        SetPredicate(pred_a);
    }

    AlgoType_t AlgoType(void) const
    { return INDEXEDNESTEDLOOPSJOIN_T; }

    // cost model
    static CostVal_t Cost(IndexOrder_t *indexOrder,
                    LogProp_t *leftLP, LogProp_t *rightLP, LogProp_t *outLP);

    // local cost val
    CostVal_t LocalCostVal(void)
    {
        PlanGroup_t *leftInpPG = Input(0);
        assert(leftInpPG);

        PhyPropGroup_t *leftPhyProp = leftInpPG->PropGroup();
        assert(leftPhyProp);

        ListIter_t<PhyProp_t *> iter;

        List_t<PhyProp_t *>& leftPropList = leftPhyProp->PropList();
        IndexOrder_t *leftIndexOrder = NULL;
        iter.Attach(&leftPropList);
        while( !iter.IsEnd() ) {
            PhyProp_t *p = iter.Next();
            assert(p);

            if( p->Type() == INDEXORDER_T ) {
                leftIndexOrder = (IndexOrder_t *) p;
                break;
            }
        }
        assert(leftIndexOrder);

        LogProp_t *leftLP = leftInpPG->LogicalProp();
        assert(leftLP);

        PlanGroup_t *rightInpPG = Input(1);
        assert(rightInpPG);

        LogProp_t *rightLP = rightInpPG->LogicalProp();
        assert(rightLP);

        PlanGroup_t *outPG = PlanGroup();
        assert(outPG);

        LogProp_t *outLP = outPG->LogicalProp();
        assert(outLP);

        return Cost(leftIndexOrder, leftLP, rightLP, outLP);
    }

    // print name
    void PrintName(void) const;
    void PrintSQLStmt(void) const;
    
    void open(){};
    Record* next(){return NULL;}
    void close(){};

    ~IndexedNestedLoopsJoin_t(void)
    { }
};

// merge (union) operator
class Merge_t : public Algorithm_t {
public:
    Merge_t(const Union_t *) : Algorithm_t(2)
    { }

    AlgoType_t AlgoType(void) const
    { return MERGE_T; }

    // cost model
    static CostVal_t Cost(LogProp_t *leftLp, LogProp_t *rightLP,
                                                LogProp_t *outLP);

    // local cost val
    CostVal_t LocalCostVal(void)
    {
        PlanGroup_t *leftInpPG = Input(0);
        assert(leftInpPG);

        LogProp_t *leftLP = leftInpPG->LogicalProp();
        assert(leftLP);

        PlanGroup_t *rightInpPG = Input(1);
        assert(rightInpPG);

        LogProp_t *rightLP = rightInpPG->LogicalProp();
        assert(rightLP);

        PlanGroup_t *outPG = PlanGroup();
        assert(outPG);

        LogProp_t *outLP = outPG->LogicalProp();
        assert(outLP);

        return Cost(leftLP, rightLP, outLP);
    }

    // print name
    void PrintName(void) const;
    void PrintSQLStmt(void) const;

    void open(){};
    Record* next(){return NULL;}
    void close(){};

    ~Merge_t(void)
    { }
};


// basic apply operator (for now we assume it's a filter)
class BasicApplyOp_t : public Algorithm_t {

    Apply_t *logOp;

public:
    BasicApplyOp_t(Apply_t *apply_a) : Algorithm_t(2)
    {
        assert(apply_a);
        logOp = apply_a;
        // Predicate_t *pred_a = apply_a->Predicate();
        // SetPredicate(pred_a);
    }

    AlgoType_t AlgoType(void) const
    { return BASICAPPLY_T; }

    // cost model
    static CostVal_t Cost(LogProp_t *leftLp, LogProp_t *rightLP,
                                                LogProp_t *outLP);

    // local cost val
    CostVal_t LocalCostVal(void)
    {
        PlanGroup_t *leftInpPG = Input(0);
        assert(leftInpPG);

        LogProp_t *leftLP = leftInpPG->LogicalProp();
        assert(leftLP);

        PlanGroup_t *rightInpPG = Input(1);
        assert(rightInpPG);

        LogProp_t *rightLP = rightInpPG->LogicalProp();
        assert(rightLP);

        PlanGroup_t *outPG = PlanGroup();
        assert(outPG);

        LogProp_t *outLP = outPG->LogicalProp();
        assert(outLP);

        return Cost(leftLP, rightLP, outLP);
    }

    // print name
    void PrintName(void) const;
    void PrintSQLStmt(void) const;
    
    void open(){};
    Record* next(){return NULL;}
    void close(){};

    ~BasicApplyOp_t(void)
    { }
};


#endif  // __PHYSICALOP_H__
