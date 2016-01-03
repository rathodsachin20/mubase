// logical operators

#ifndef __LOGICALOP_H__
#define __LOGICALOP_H__

#include <string.h>

#include "List.h"
#include "PhyPropGroup.h"
#include "LogProp.h"
#include "Node.h"
#include "Expr.h"
#include "Equivalence.h"

using namespace std;

// Ravi:Symbolic constants for telling whether ordering among the operator's 
// Ravi:inputs is significant (relevant) or not. See LogicalOp_t::isOrdered.
#define  ORDERED_INPUT   1
#define  UNORDERED_INPUT 0

class Memo_t;           // memo structure for storing plans
class Volcano_t;        // volcano optimizer

class LogicalOp_t;      // base class for all logical operators

// derived classes
class Union_t;          // union operator
class Project_t;        // project operator
class Filter_t;         // filter operator: base of select and join operators
class Rel_t;            // relation access operator

//Ravi:Add:For the new Apply Operator
class Apply_t;          // apply operator
class ApplyOp_t;
class LeftSemiJoin_t;

/**
 * operator types
 */
enum LogicalOpType_t {
    UNION_T,            // n-ary union operator
    AGGREGATE_T,        // unary aggregation operator
    PROJECT_T,          // unary project operator
    SELECT_T,           // unary select operator
    JOIN_T,             // binary join operator
    REL_T,              // 0-ary base relation access operator
    //Ravi:Add:For the Apply operator.
    //TODO:Extend to make it n-ary
    APPLY_T             // binary apply operator
};


/**
 * Base class for all logical operators.
 */
class LogicalOp_t : public LogicalNode_t {
    static int Count;           // count of the number of logical ops
    static int CurID;           // id generator

    int id;                     // unique id of the logical op node
    LogicalOpType_t type;       // type of operator

    char isOrdered;             // is the input order important?
                                // Ravi:Com:0=>order not important, 
                                // 1=>order important
    char isDerived;             // is derived from some other logical
                                // expression
    char isExpanded;            // have all the transformations been
                                // applied to this logical expression?
   //Ravi:Doubt:What is a detached node? 
    char isDetached;            // is the op detached from the dag?

    // compute the logical property
    virtual LogProp_t *ComputeLogicalProp(void) const = 0;

    // create the list of degree of sharing given the lists of the children
    // equivalence nodes
    // the children lists are topologically sorted wrt the eq nodes
    AppendList_t<DegShar_t *> *CreateDegreeList(void);

protected:
    Equivalence_t *eqClass;     // associated equivalence class
    int numInputs;              // number of inputs
    Equivalence_t **inputs;     // input equivalence classes

public:
    LogicalOp_t(LogicalOpType_t type_a, int numInputs_a, char isOrdered_a);

    // get the node's id
    int ID(void) const { return id; }

    // check for equality
    int IsEqual(const LogicalOp_t *x) const
    {
        assert(x);
        return x->ID() == id;
    }

    // associated equivalence class
    Equivalence_t *EqClass(void) const
    { return eqClass; }

    // number of parents -- logical ops have only one parent eq class
    int NumParents(void) const
    { return 1; }

    // set the equivalence class
    void SetEqClass(Equivalence_t *eqClass_a)
    { eqClass = eqClass_a; }

    // sets the i-th input --- updates the prev input's parent list and
    // the new input's parent list
    void SetInput(int i, Equivalence_t *eq)
    {
        assert(i >= 0 && i < numInputs);
        if( inputs[i] )
            inputs[i]->DeleteParent(this);
        inputs[i] = eq;
        if( eq )
            eq->AddParent(this);
    }

    // return the i-th input
    Equivalence_t *Input(int i) const
    {
        assert(i >= 0 && i < numInputs);
        return inputs[i];
    }

    // return the number of inputs
    int NumInputs(void) const
    { return numInputs; }

    // return operator type
    LogicalOpType_t Type(void) const
    { return type; }

    // hash function -- naive
    int HashValue(void) const
    {
        int h = type;
        for( int i = 0 ; i < numInputs ; i++ )
            h += inputs[i]->ID();
        return h;
    }

    // logical property. Obtains from the equivalence class to which this 
    // op belongs to.
    LogProp_t *LogicalProp(void) const
    {
        if( eqClass )
            return eqClass->LogicalProp();
        return ComputeLogicalProp();
    }

    // expand the inputs and then apply all transformations possible
    // on this node
    void Expand(Memo_t& memo);

    // is the logexpr already expanded
    int IsExpanded(void) const
    { return isExpanded; }

    // mark as expanded
    void MarkExpanded(void)
    { isExpanded = 1; }

    // check for equivalence
    virtual int IsEquivalent(const LogicalOp_t *x) const;

    // push the predicate across or into the operator
    // returns the new operator if transformation possible; NULL if not
    virtual LogicalOp_t *PushInPred(Predicate_t *inPred, Memo_t& memo) = 0;

    // push the project across the operator
    // returns the new operator if transformation possible; NULL if not
    virtual LogicalOp_t *PushInProj(Project_t *inProj, Memo_t& memo) = 0;

    // generates algorithm operators for this operator
    // inputs are optimized by the optimizer provided
    // the plan must be better than the best plan
    // update the bestPlan if a better plan is found
    virtual void ApplyAlgorithms(PlanGroup_t *planGroup, Volcano_t *opt,
                                 Cost_t *totalCost, Plan_t *bestPlan, 
                                 CostVal_t *costLimit, Order_t* gpso=NULL, 
                                 int *plmap=NULL, int level=0, 
                                 Card_t callCount=1) = 0;

    // apply the transformations
    virtual void ApplyTransformations(Memo_t& memo) = 0;

    // lower bound on any algorithm corresponding to this logop
    virtual CostVal_t CostLB(void) const = 0;

    // is the logical operator admissible given the cost upper limit?
    int IsAdmissible(CostVal_t costUB) const;

    // mark the logical expression as a derived expression
    void MarkDerived(void)
    { isDerived = 1; }

    // is the logical expresion derived from some other logical
    // expression?
    int IsDerived(void) const
    { return isDerived; }

    // name -- for debug printing
    virtual void PrintName(void) const = 0;

    // for debugging
    virtual void PrintExpressionDAG(int indentLevel) const;
    virtual void PrintExpressionDAGXML(int indentLevel) const;

    // count of number of instances
    static int NodeCount(void)
    { return Count; }

    // count of the total number of instances generated
    static int TotalNodeCount(void)
    { return CurID; }

    // detach the node
    // Ravi:Doubt:What is detaching of a node?
    void Detach(void);

    // is the node detached?
    int IsDetached(void) const
    { return isDetached; }

    /**
     * Populates the list "correlatedVars" of the associated eq class.
     * See details of the instance variable correlatedAttrs.
     */
    virtual void SetOuterVarLogProp();

    /**
     * Compute the outer varaiable logical properties for this logical
     * from those of the input subexpressions.
     */
//    virtual LogProp_t *ComputeOuterVarLogProp();


    virtual void GetInterestingOrders(List_t<Order_t *> *intOrds, 
                                      int *plmap, int level);

    virtual void GetFavorableOrders(List_t<Order_t *> *favOrds);

    virtual ~LogicalOp_t(void);
};

// union operator
class Union_t : public LogicalOp_t {
    // compute the logical property
    LogProp_t *ComputeLogicalProp(void) const;

    // order of inp: l to r
    void ApplyFwdAlgorithms(PlanGroup_t *planGroup, Volcano_t *opt,
            Cost_t *totalCost, Plan_t *bestPlan, CostVal_t *costLimit, 
        Order_t* gpso=NULL, int *plmap=NULL, int level=0, Card_t callCount=1);

    // order of inp: r to l
    void ApplyRevAlgorithms(PlanGroup_t *planGroup, Volcano_t *opt,
            Cost_t *totalCost, Plan_t *bestPlan, CostVal_t *costLimit,
        Order_t* gpso=NULL, int *plmap=NULL, int level=0, Card_t callCount=1);

public:
    Union_t(int numInputs_a)
        : LogicalOp_t(UNION_T, numInputs_a, 0 /* not ordered */) 
    { }

    // not implemented yet
    LogicalOp_t *PushInPred(Predicate_t *, Memo_t&)
    { return NULL; }

    // not implemented yet
    LogicalOp_t *PushInProj(Project_t *, Memo_t&)
    { return NULL; }

    // generates merge operator
    // inputs are optimized by the optimizer provided
    // the plan must be better than the best plan
    // update the bestPlan if a better plan is found
    void ApplyAlgorithms(PlanGroup_t *planGroup, Volcano_t *opt,
            Cost_t *totalCost, Plan_t *bestPlan, CostVal_t *costLimit,
            Order_t* gpso=NULL, int *plmap=NULL, int level=0, 
            Card_t callCount=1); 

    // apply the transformations
    void ApplyTransformations(Memo_t& memo);

    // lower bound on any algorithm corresponding to this logop
    CostVal_t CostLB(void) const;

    // name -- for debug printing
    void PrintName(void) const
    { cout << "UNION"; }

    // is this the virtual root?
    int IsRoot(void) const
    { return eqClass ? eqClass->IsRoot() : 0; }

    ~Union_t(void)
    { }
};

// entries to map the output attributes to expressions
class AttrMapEntry_t : public RefC_t {
    Expr_t *expr;       // mapped attribute's index in input schema
    char *name;         // new name assigned to the mapped attribute

public:
    AttrMapEntry_t(Expr_t *expr_a, char *name_a) : expr(expr_a), name(NULL)
    { 
        if( name_a ) name = strdup(name_a);
        Refer(expr);
    }

    Expr_t *Expr(void) const
    { return expr; }

    char *Name(void) const
    { return name; }

    ~AttrMapEntry_t(void)
    { 
        if( name ) free(name);
        DeRefer(expr);
    }
};

// mapping of the output attributes to expressions
class AttrMap_t : public RefC_t {
    int numProj;        // number of entries in the projection list
    AttrMapEntry_t **proj;
                        // projection list --
                        // maps input schema to output schema

public:
    AttrMap_t(int numProj_a) : numProj(numProj_a), proj(NULL)
    {
        assert(numProj >= 0);

        if( numProj > 0 ) {
            proj = new AttrMapEntry_t *[numProj]; 
            for( int i = 0 ; i < numProj ; i++ )
                proj[i] = NULL;
        }
    }

    int NumProj(void) const
    { return numProj; }

    void SetProj(int i, AttrMapEntry_t *e)
    {
        assert(i >= 0 && i < numProj);
        assert(e);
        assert(!proj[i]);

        proj[i] = e;
        Refer(proj[i]);
    }

    AttrMapEntry_t *Entry(int i) const
    {
        assert(i >= 0 && i < numProj);
        assert(proj[i]);

        return proj[i];
    }

    // partition the project map entries into valid and invalid entries
    // with respect to the schema
    void Decompose(Schema_t *schema, AttrMap_t **valid, AttrMap_t **invalid) const;

    ~AttrMap_t(void)
    {
        if( numProj > 0 ) {
            for( int i = 0 ; i < numProj ; i++ ) {
                if( proj[i] != NULL )
                    DeRefer(proj[i]);
            }

            delete[] proj;
        }
    }
};

// group-by aggregate operator class
class Aggregate_t : public LogicalOp_t {
    AttrMap_t *groupByAttrMap;  // maps input schema to output schema
    AttrMap_t *aggAttrMap;      // maps aggregated attributes to output schema

    // compute the logical property
    LogProp_t *ComputeLogicalProp(void) const;

    void ApplyAggregateHash(PlanGroup_t *planGroup, Volcano_t *opt,
            Cost_t *totalCost, Plan_t *bestPlan, CostVal_t *costLimit,
        Order_t *gpso, int *plmap, int level, Card_t callCount);
    void ApplyAggregateScan(PlanGroup_t *planGroup, Volcano_t *opt,
            Cost_t *totalCost, Plan_t *bestPlan, CostVal_t *costLimit,
        Order_t *gpso, int *plmap, int level, Card_t callCount);

public:
    Aggregate_t(int numGroupByAttrs_a, int numAggAttrs_a)
        : LogicalOp_t(AGGREGATE_T, 1, 0 /* not ordered */),
                        groupByAttrMap(NULL), aggAttrMap(NULL)
    {
        assert(numGroupByAttrs_a >= 0);
        assert(numAggAttrs_a > 0);

        groupByAttrMap = new AttrMap_t(numGroupByAttrs_a); 
        Refer(groupByAttrMap);

        aggAttrMap = new AttrMap_t(numAggAttrs_a); 
        Refer(aggAttrMap);
    }

    Aggregate_t(AttrMap_t *groupByAttrMap_a, AttrMap_t *aggAttrMap_a)
        : LogicalOp_t(PROJECT_T, 1, 0 /* not ordered */),
            groupByAttrMap(groupByAttrMap_a), aggAttrMap(aggAttrMap_a)
    {
        Refer(groupByAttrMap);
        Refer(aggAttrMap);
    }

    int NumGroupByAttrs(void) const
    { return groupByAttrMap->NumProj(); }

    void SetGroupByExpr(int i, Expr_t *groupByExpr, char *newName)
    {
        assert(groupByExpr);
        assert(groupByExpr->ExprType() == REL_ARG_REF_T);
        AttrMapEntry_t *e = new AttrMapEntry_t(groupByExpr, newName);
        Refer(e);
        groupByAttrMap->SetProj(i, e);
        DeRefer(e);
    }

    Expr_t *GroupByExpr(int i) const
    {
        AttrMapEntry_t *e = groupByAttrMap->Entry(i);
        return e->Expr();
    }

    char *GroupByExprName(int i) const
    {
        AttrMapEntry_t *e = groupByAttrMap->Entry(i);
        return e->Name();
    }

    AttrMap_t *GroupByAttrMap(void) const
    { return groupByAttrMap; }

    int NumAggAttrs(void) const
    { return aggAttrMap->NumProj(); }

    void SetAggExpr(int i, Expr_t *aggExpr, char *newName)
    {
        assert(aggExpr);
        assert(aggExpr->ExprType() == AGG_EXPR_T);
        AttrMapEntry_t *e = new AttrMapEntry_t(aggExpr, newName);
        Refer(e);
        aggAttrMap->SetProj(i, e);
        DeRefer(e);
    }

    Expr_t *AggExpr(int i) const
    {
        AttrMapEntry_t *e = aggAttrMap->Entry(i);
        return e->Expr();
    }

    char *AggExprName(int i) const
    {
        AttrMapEntry_t *e = aggAttrMap->Entry(i);
        return e->Name();
    }

    AttrMap_t *AggAttrMap(void) const
    { return aggAttrMap; }

    // check for equivalence
    int IsEquivalent(const LogicalOp_t *x) const;

    // check for equivalence of aggregation lists
    int IsAggEquivalent(const LogicalOp_t *x) const;

    // check for equivalence of groupby lists
    int IsGroupByEquivalent(const LogicalOp_t *x) const;

    // not implemented yet
    LogicalOp_t *PushInPred(Predicate_t *, Memo_t&)
    { return NULL; }

    // not implemented yet
    LogicalOp_t *PushInProj(Project_t *, Memo_t&)
    { return NULL; }

    // generates project-scan operator
    // inputs are optimized by the optimizer provided
    // the plan must be better than the best plan
    // update the bestPlan if a better plan is found
    void ApplyAlgorithms(PlanGroup_t *planGroup, Volcano_t *opt,
            Cost_t *totalCost, Plan_t *bestPlan, CostVal_t *costLimit,
            Order_t* gpso=NULL, int *plmap=NULL, int level=0, 
            Card_t callCount=1);

    // apply the transformations
    void ApplyTransformations(Memo_t& memo);

    // does $this$ subsume the given aggregation wrt group-by list?
    int IsGroupBySubsumedBy(Aggregate_t *p) const;

    // does $this$ subsume the given aggregation wrt aggregation list?
    int IsAggSubsumedBy(Aggregate_t *p) const;

    // lower bound on any algorithm corresponding to this logop
    CostVal_t CostLB(void) const;

    // name -- for debug printing
    void PrintName(void) const
    {
        cout << "AGGREGATE ";
        int numAggAttrs = NumAggAttrs();

        int i = 0;
        for( i = 0 ; i < numAggAttrs ; i++ ) {
            Expr_t *e = AggExpr(i);
            assert(e);

            e->PrintExpr();
            cout << " ";
        }
        cout << "| ";
        int numGroupByAttrs = NumGroupByAttrs();
        for( i = 0 ; i < numGroupByAttrs ; i++ ) {
            Expr_t *e = GroupByExpr(i);
            assert(e);

            e->PrintExpr();
            cout << " ";
        }
        //cout << ">";
    }

    // for debugging
    void PrintExpressionDAGXML(int indentLevel) const;

    ~Aggregate_t(void)
    {
        DeRefer(groupByAttrMap);
        DeRefer(aggAttrMap);
    }
};
    
// project operator class
class Project_t : public LogicalOp_t {
    AttrMap_t *projMap; // projection list --
                        // maps input schema to output schema
    // compute the logical property
    LogProp_t *ComputeLogicalProp(void) const;

public:
    Project_t(int numProj_a)
        : LogicalOp_t(PROJECT_T, 1, 0 /* not ordered */), projMap(NULL)
    {
        assert(numProj_a > 0);
        projMap = new AttrMap_t(numProj_a); 
        Refer(projMap);
    }

    Project_t(AttrMap_t *projMap_a)
        : LogicalOp_t(PROJECT_T, 1, 0 /* not ordered */), projMap(projMap_a)
    { Refer(projMap); }

    int NumProj(void) const
    { return projMap->NumProj(); }

    void SetProj(int i, Expr_t *projExpr, char *newName)
    {
        AttrMapEntry_t *e = new AttrMapEntry_t(projExpr, newName);
        Refer(e);
        projMap->SetProj(i, e);
        DeRefer(e);
    }

    Expr_t *ProjExpr(int i) const
    {
        AttrMapEntry_t *e = projMap->Entry(i);
        return e->Expr();
    }

    char *ProjName(int i) const
    {
        AttrMapEntry_t *e = projMap->Entry(i);
        return e->Name();
    }

    AttrMap_t *AttrMap(void) const
    { return projMap; }

    // check for equivalence
    int IsEquivalent(const LogicalOp_t *x) const;

    // not implemented yet
    LogicalOp_t *PushInPred(Predicate_t *, Memo_t&)
    { return NULL; }

    // not implemented yet
    LogicalOp_t *PushInProj(Project_t *, Memo_t&)
    { return NULL; }

    // generates project-scan operator
    // inputs are optimized by the optimizer provided
    // the plan must be better than the best plan
    // update the bestPlan if a better plan is found
    void ApplyAlgorithms(PlanGroup_t *planGroup, Volcano_t *opt,
         Cost_t *totalCost, Plan_t *bestPlan, CostVal_t *costLimit,
         Order_t* gpso=NULL, int *plmap=NULL, int level=0, Card_t callCount=1);

    // apply the transformations
    void ApplyTransformations(Memo_t& memo);

    // lower bound on any algorithm corresponding to this logop
    CostVal_t CostLB(void) const;

    // name -- for debug printing
    void PrintName(void) const
    {
        cout << "PROJECT ";
        int numProj = NumProj();
        for( int i = 0 ; i < numProj ; i++ ) {
            Expr_t *e = ProjExpr(i);
            assert(e);

            e->PrintExpr();
            cout << " ";
        }
        //cout << ">";
    }

    // for debugging
    void PrintExpressionDAGXML(int indentLevel) const;

    ~Project_t(void)
    { DeRefer(projMap); }
};

// base class for select, join --- additionally contains a predicate
class Filter_t : public LogicalOp_t {
    Predicate_t *pred;

    // compute the logical property
    virtual LogProp_t *ComputeLogicalProp(void) const = 0;

public:
    Filter_t(LogicalOpType_t type_a, int numInputs_a, int isOrdered_a,
                                                    Predicate_t *pred_a)
        : LogicalOp_t(type_a, numInputs_a, isOrdered_a), pred(pred_a)
    { Refer(pred); }

    // check for equivalence
    virtual int IsEquivalent(const LogicalOp_t *x) const;

    // predicate
    Predicate_t *Predicate(void) const
    { return pred; }

    /*
    // NQO:Added for accessing the lvalue
    // Predicate_t **PredicateLValue(void) const
    // { return &pred; }
    */

    // merge the given predicate with the filter predicate
    // return the resulting filter operator
    virtual LogicalOp_t *PushInPred(Predicate_t *inPred, Memo_t& memo) = 0;

    // push the project across the operator
    // returns the new operator if transformation possible; NULL if not
    virtual LogicalOp_t *PushInProj(Project_t *inProj, Memo_t& memo) = 0;

    // generates algorithm operators
    // inputs are optimized by the optimizer provided
    // the plan must be better than the best plan
    // update the bestPlan if a better plan is found
    virtual void ApplyAlgorithms(PlanGroup_t *planGroup, Volcano_t *opt,
        Cost_t *totalCost, Plan_t *bestPlan, CostVal_t *costLimit,
        Order_t* gpso=NULL, int *plmap=NULL, int level=0, 
        Card_t callCount=1) = 0;

    // apply the transformations
    virtual void ApplyTransformations(Memo_t& memo) = 0;

    virtual ~Filter_t(void)
    { DeRefer(pred); }
};

// derived classes
class Select_t;         // select operator
class Join_t;           // join operator

// select operator class
class Select_t : public Filter_t {
    // compute the logical property
    LogProp_t *ComputeLogicalProp(void) const;

    // generates select scan operator
    void ApplySelectScan(PlanGroup_t *planGroup, Volcano_t *opt,
            Cost_t *totalCost, Plan_t *bestPlan, CostVal_t *costLimit,
        Order_t* gpso=NULL, int *plmap=NULL, int level=0, Card_t callCount=1);
    // generates indexed select operator
    void ApplyIndexedSelect(PlanGroup_t *planGroup,
                IndexOrder_t *indexOrder, Volcano_t *opt,
                Cost_t *totalCost, Plan_t *bestPlan, CostVal_t *costLimit,
        Order_t* gpso=NULL, int *plmap=NULL, int level=0, Card_t callCount=1);
    // apply the indexed select operator
    void ApplyIndexedSelect(PlanGroup_t *planGroup,
                                PhyPropGroup_t *indexInpReqProp,
                                Equivalence_t *indexInpEq,
                                Volcano_t *opt, Cost_t *totalCost,
                                Plan_t *bestPlan, CostVal_t *costLimit,
                Order_t* gpso=NULL, int *plmap=NULL, int level=0, 
                Card_t callCount=1);

public:
    Select_t(Predicate_t *pred_a) : Filter_t(SELECT_T, 1, 1, pred_a)
    {
        assert(pred_a);
        //Ravi:Following line commented out because when we transform
        //Ravi:the nested predicate to Apply Op, the select predicate
        //Ravi:can be just True if no other simple predicate is present
        //Ravi:in cunjunction with the nested predicate.
        //assert(!pred_a->IsEquivalent(ConstPreds.True()));
    }

    // merge the given predicate with the select predicate
    // return the resulting select operator
    LogicalOp_t *PushInPred(Predicate_t *inPred, Memo_t& memo);

    // push the project across the select
    // returns the new operator if transformation possible; NULL if not
    LogicalOp_t *PushInProj(Project_t *inProj, Memo_t& memo);

    // generates select scan operators
    // inputs are optimized by the optimizer provided
    // the plan must be better than the best plan
    // update the bestPlan if a better plan is found
    void ApplyAlgorithms(PlanGroup_t *planGroup, Volcano_t *opt,
         Cost_t *totalCost, Plan_t *bestPlan, CostVal_t *costLimit,
         Order_t* gpso=NULL, int *plmap=NULL, int level=0, Card_t callCount=1);

    // apply the transformations
    void ApplyTransformations(Memo_t& memo);

    // lower bound on any algorithm corresponding to this logop
    CostVal_t CostLB(void) const;

    // index order for indexed selection
    int IndexProperty(IndexOrder_t **indexOrder) const;

    // for debugging
    void PrintExpressionDAG(int indentLevel) const;
    void PrintExpressionDAGXML(int indentLevel) const;

    void PrintName(void) const
    {
        cout << "SELECT ";
        Predicate_t *p = Predicate();
        assert(p);
        p->PrintExpr();
        //cout << " >";
    }

    void SetOuterVarLogProp();

    void GetInterestingOrders(List_t<Order_t *> *intOrds, 

                             int *plmap, int level);
//  LogProp_t *ComputeOuterVarLogProp();

    ~Select_t(void) { }
};

// join operator class
class Join_t : public Filter_t {
    // compute the logical property
    LogProp_t *ComputeLogicalProp(void) const;

    void ApplyNestedLoopsJoin(PlanGroup_t *planGroup, Volcano_t *opt,
            Cost_t *totalCost, Plan_t *bestPlan, CostVal_t *costLimit,
        Order_t* gpso=NULL, int *plmap=NULL, int level=0, Card_t callCount=1);
    void ApplyMergeJoin(PlanGroup_t *planGroup,
            SortOrder_t *reqOutOrder, Volcano_t *opt,
            Cost_t *totalCost, Plan_t *bestPlan, CostVal_t *costLimit,
        Order_t* gpso=NULL, int *plmap=NULL, int level=0, Card_t callCount=1);
    // generates indexed nested loops join operators
    void ApplyIndexedNestedLoopsJoin(PlanGroup_t *planGroup,
                IndexOrder_t *leftInpIndexOrder,
                IndexOrder_t *rightInpIndexOrder, Volcano_t *opt,
                Cost_t *totalCost, Plan_t *bestPlan, CostVal_t *costLimit,
        Order_t* gpso=NULL, int *plmap=NULL, int level=0, Card_t callCount=1);
    // apply the indexed nested loops join operator
    void ApplyIndexedNestedLoopsJoin(PlanGroup_t *planGroup,
                                    IndexOrder_t *indexOrder,
                                    PhyPropGroup_t *indexInpReqProp,
                                    Equivalence_t *indexInpEq,
                                    PhyPropGroup_t *probeInpReqProp,
                                    Equivalence_t *probeInpEq,
                                    Volcano_t *opt, Cost_t *totalCost,
                                    Plan_t *bestPlan, CostVal_t *costLimit,
                    Order_t* gpso=NULL, int *plmap=NULL, int level=0, 
                    Card_t callCount=1);
    char isTransDisabled;

public:
    Join_t(Predicate_t *pred_a)
        : Filter_t(JOIN_T, 2, 0, pred_a), isTransDisabled(0) { }

    // merge the given predicate with the join predicate
    // return the resulting join operator
    LogicalOp_t *PushInPred(Predicate_t *inPred, Memo_t& memo);

    // push the project across the join
    // returns the new operator if transformation possible; NULL if not
    LogicalOp_t *PushInProj(Project_t *inProj, Memo_t& memo);

    // generates merge-join and nested loops operators
    // inputs are optimized by the optimizer provided
    // the plan must be better than the best plan
    // update the bestPlan if a better plan is found
    void ApplyAlgorithms(PlanGroup_t *planGroup, Volcano_t *opt,
         Cost_t *totalCost, Plan_t *bestPlan, CostVal_t *costLimit,
         Order_t* gpso=NULL, int *plmap=NULL, int level=0, Card_t callCount=1);

    // apply the transformations
    void ApplyTransformations(Memo_t& memo);

    // sort order for merge join
    int SortProperty(SortOrder_t **sortOrder,
                            SortOrder_t **leftInpSortOrder,
                            SortOrder_t **rightInpSortOrder) const;

    // index order for indexed join
    int IndexProperty(IndexOrder_t **leftInpIndexOrder,
                                IndexOrder_t **rightInpIndexOrder) const;

    // lower bound on any algorithm corresponding to this logop
    CostVal_t CostLB(void) const;

    void PrintName(void) const
    {
        cout << "JOIN ";
        Predicate_t *p = Predicate();
        assert(p);
        p->PrintExpr();
        //cout << " >";
    }

    // to avoid generating duplicate expressions
    void DisableTrans(void) { isTransDisabled = 1; }
    int IsTransDisabled(void) { return isTransDisabled; }

    // for debugging
    void PrintExpressionDAGXML(int indentLevel) const;

    // void GetFavorableOrders(List_t<Order_t *> *favOrds);

    ~Join_t(void) { }
};

// base relation access operator class
class Rel_t : public LogicalOp_t {
    char *name;

    // compute the logical property
    LogProp_t *ComputeLogicalProp(void) const;

public:
    Rel_t(const char *name_a) : LogicalOp_t(REL_T, 0, 1), name(NULL)
    { assert(name_a); name = strdup(name_a); }

    const char *Name(void) const { return name; }

    // check for equivalence
    int IsEquivalent(const LogicalOp_t *x) const;

    // not possible
    LogicalOp_t *PushInPred(Predicate_t *, Memo_t&)
    { return NULL; }

    // not possible
    LogicalOp_t *PushInProj(Project_t *, Memo_t&)
    { return NULL; }

    // generates relation scan operator
    // inputs are optimized by the optimizer provided
    // the plan must be better than the best plan
    // update the bestPlan if a better plan is found
    void ApplyAlgorithms(PlanGroup_t *planGroup, Volcano_t *opt,
         Cost_t *totalCost, Plan_t *bestPlan, CostVal_t *costLimit,
          Order_t* gpso=NULL, int *plmap=NULL, int level=0, Card_t callCount=1);

    // apply the transformations
    void ApplyTransformations(Memo_t& memo);

    // lower bound on any algorithm corresponding to this logop
    CostVal_t CostLB(void) const;

    void PrintName(void) const
    { cout << Name(); }

    void GetFavorableOrders(List_t<Order_t *> *favOrds);

    ~Rel_t(void)
    { free(name); }
};


/**
 * Apply operator class.
 */
class Apply_t : public LogicalOp_t {

    // The actual operation between LHS and RHS    
    ApplyOp_t *op;
    
    Card_t numDistinct; // Number of distincts in the LHS

    // compute the logical property
    LogProp_t *ComputeLogicalProp(void) const;

    // Apply the basic apply algorithm
    void ApplyBasicApplyOp(PlanGroup_t *planGroup, Volcano_t *opt,
         Cost_t *totalCost, Plan_t *bestPlan, CostVal_t *costLimit, 
         Order_t* gpso, int *plmap, int level, Card_t callCount);

    // Combines the bind i/p pso with the pso coming from top
    Order_t *CombinePSO(Order_t *rightGPSO, Order_t *localPSO);

public:
    // Construct the Filter_t with number of inputs as 2 and the 
    // supplied predicate.  First input is the LHS (bind expr) and 
    // the second input is the RHS (use expr)
    Apply_t(ApplyOp_t *op_a) : LogicalOp_t(APPLY_T, 2, ORDERED_INPUT), op(op_a)
    { }

    // Push the given predicate into the LHS and return this.
    LogicalOp_t *PushInPred(Predicate_t *inPred, Memo_t& memo);

    // push the project across the apply
    // returns the new operator if transformation possible; NULL if not
    LogicalOp_t *PushInProj(Project_t *inProj, Memo_t& memo);

    // Generates the physical operator for Apply
    // inputs are optimized by the optimizer provided
    // the plan must be better than the best plan
    // update the bestPlan if a better plan is found
    void ApplyAlgorithms(PlanGroup_t *planGroup, Volcano_t *opt,
                         Cost_t *totalCost, Plan_t *bestPlan, 
                         CostVal_t *costLimit,
                         Order_t* gpso=NULL, int *plmap=NULL, 
                         int level=0, Card_t callCount=1);

    // apply the transformations
    void ApplyTransformations(Memo_t& memo);

    // lower bound on any algorithm corresponding to this logop
    CostVal_t CostLB(void) const;

    void SetNumDistinct(Card_t numDist)
    { numDistinct = numDist; }

    Card_t NumDistinct()
    { return numDistinct; }

    // index order for indexed selection
    int IndexProperty(IndexOrder_t **indexOrder) const;

    // for debugging
    void PrintExpressionDAG(int indentLevel) const;
    void PrintExpressionDAGXML(int indentLevel) const;

    void PrintName(void) const;

    void SetOuterVarLogProp();

    void GetInterestingOrders(List_t<Order_t *> *intOrds, 
                             int *plmap, int level);

//  LogProp_t *ComputeOuterVarLogProp();

    ApplyOp_t *Op()
    { return op; }

    ~Apply_t(void) { }
};


/**
 * Base class of classes that encapsulate the operation between 
 * the outer tuple and the inner result set of the Apply.
 * QUESTION: Ideally this should be a LogicalOp_t (Join_t, 
 * AntiJoin_t etc. but the current design is not suitable for
 * this)
 */
class ApplyOp_t {
  
    protected: 
    Predicate_t *pred;    // This should not be here but is a hack to 
                          // make Apply_t::ComputeLogicalProp to work.
    LogProp_t *outerInpLP;
    LogProp_t *innerInpLP;
 
    public:
    ApplyOp_t(LogProp_t *a_outerInpLP, LogProp_t *a_innerInpLP, 
              Predicate_t *pred_a) : outerInpLP(a_outerInpLP), 
              innerInpLP(a_innerInpLP), pred(pred_a)
    { }

    Predicate_t *Predicate() 
    { return pred; }

    virtual void PrintName() = 0;
   
    // Ideally this should not come in logical ops.
    // There could be multiple ways of evaluating join, semijoin and 
    // antijoin etc.
    // virtual CostVal_t Cost() = 0;
    
    virtual ~ApplyOp_t(void) { }
};

class LeftSemiJoin_t : public ApplyOp_t {

    public:
    LeftSemiJoin_t(LogProp_t *a_outerInpLP, LogProp_t *a_innerInpLP,
                   Predicate_t *pred_a) 
                : ApplyOp_t(a_outerInpLP, a_innerInpLP, pred_a) 
    { }
    
    void PrintName()
    {
        cout << "LeftSemiJoin";
    }

    // Here we should cost for a single tuple from the left i/p
    // and pipelined right output. A CPU cost proportional to 
    // right input's blocks is incurred.
    static CostVal_t Cost(LogProp_t *leftLP, LogProp_t *rightLP)
    {   
        return Config_t::Cost(0, rightLP->RelSize_Blocks());
    }

    ~LeftSemiJoin_t(void) {} 

};

List_t<Order_t *> *DeduceIntOrds(const Set_t<int> *gbAttrSet, 
                                 const List_t<Order_t *> *inpFavOrds,
                                 const SchemaProp_t *sp); 

List_t<Order_t *> *DeduceIntOrdsAllPermutations(const Set_t<int> *attrSet);
List_t<Order_t *> *DeduceIntOrdsPGHeuristic(const Set_t<int> *attrSet);
List_t<Order_t *> *DeduceIntOrdsPlain(const Set_t<int> *attrSet);

void PrintOrds(const List_t<Order_t *> *intOrds, const Schema_t *schema);


// A utility class to generate sort orders corresponding to ALL 
// permutations of given attribute set. Needed for comparison with
// exhaustive enumeration of interesting orders for join and 
// group by.
class Permuter {
    int n;
    int level;
    int *value;
    AppendList_t<Order_t *> *allOrds;
    const int *elements;

    public:
    Permuter(AppendList_t<Order_t *> *resultList, const Set_t<int> *attrSet)
    {
        allOrds = resultList;
        n = attrSet->Size();
        elements = attrSet->Elements();
        value = new int[n];
        level = -1;
        for (int i = 0; i < n; i++)
            value[i] = 0;
    }

    void Generate()
    {
        visit(0);
    }

    void AddItem()
    {
        int *ordArray = new int[n];

        for (int i = 0; i < n; i++) {
            ordArray[i] = elements[value[i]-1];
        }

        Order_t *tmpOrd = new SortOrder_t(ordArray, n);
        allOrds->Insert(tmpOrd);
        delete[] ordArray;
    }


    void visit(int k)
    {
        level = level+1; 
        value[k] = level; 

        if (level == n)
            AddItem(); 
        else {
            for (int i = 0; i < n; i++) {
                if (value[i] == 0)
                    visit(i);
            }
        }
        level = level-1; value[k] = 0;
    }

    ~Permuter()
    {
        delete[] value;
    }
};

#endif // __LOGICALOP_H__
