// logical operator classes implementation

#include <math.h>
#include <string.h>
#include "Config.h"
#include "Memo.h"
#include "Catalog.h"
#include "Expr.h"
#include "TransRule.h"
#include "PhysicalOp.h"
#include "Volcano.h"
#include "LogicalOp.h"

#define MAX_FAV_ORDS 50

void FixNumDistincts(SchemaProp_t *sp, Card_t numTuples);
CostVal_t SubQueryCost(InExpr_t *inCond, LogProp_t *outLP, Volcano_t *opt, 
                       CostVal_t *costLimit, PlanGroup_t **subQueryPlanGroup);
// instance count
int LogicalOp_t::Count = 0;

// id generator
int LogicalOp_t::CurID = 0;

extern Catalog_t *Catalog;
const int MAXGROUPBYATTRS = 10;

// constructor
LogicalOp_t::LogicalOp_t(LogicalOpType_t type_a,
                        int numInputs_a, char isOrdered_a)
            : id(CurID++), type(type_a),
            isOrdered(isOrdered_a), isDerived(0), isExpanded(0),
            isDetached(0), eqClass(NULL), numInputs(numInputs_a), inputs(NULL)
{
    assert(numInputs >= 0);

    if( numInputs > 0 ) {
        inputs = new Equivalence_t *[numInputs];
        for( int i = 0 ; i < numInputs ; i++ )
            inputs[i] = NULL;
    }

    Count++;
}

// destructor
LogicalOp_t::~LogicalOp_t(void)
{ Detach(); }

// detach the node
void LogicalOp_t::Detach(void)
{
    if( isDetached )
        return;
    isDetached = 1;

    if( eqClass ) {
        eqClass->DeleteLogExpr(this);
        eqClass = NULL;
    }

    if( numInputs > 0 ) {
        assert(inputs);

        for( int i = 0 ; i < numInputs ; i++ ) {
            Equivalence_t *eq = inputs[i];
            if( eq ) {
                eq->DeleteParent(this);
                if( eq->NumParents() == 0 )
                    delete eq;
                inputs[i] = NULL;
            }
        }

        delete[] inputs;
        inputs = NULL;

        numInputs = 0;
    }

    Count--;
}

// equivalence check
// compare the type and the inputs --- two cases due to $isOrdered$ flag
int LogicalOp_t::IsEquivalent(const LogicalOp_t *x) const
{
    assert(x);

    // check for equality
    if( IsEqual(x) )
        return 1;

    if( x->Type() != type )
        return 0;

    // types are equal --- now check the inputs

    assert( x->NumInputs() == numInputs );

    if( isOrdered ) {
        // inputs are ordered
        for( int i = 0 ; i < numInputs ; i++ ) {
            const Equivalence_t *xInp = x->Input(i);
            assert(xInp);
            if( !inputs[i]->IsEqual(xInp) )
                return 0;
        }

        return 1;
    }

    // not ordered

    int i = 0, j = 0;
    for( i = 0 ; i < numInputs ; i++ ) {
        // search for $x$'s $i$th input in $inputs$
        const Equivalence_t *xInp = x->Input(i);

        for( j = 0 ; j < numInputs ; j++ ) {
            if( inputs[j]->IsEqual(xInp) )
                break;
        }
        if( j == numInputs ) {
            // $x$'s $i$th input not found in $inputs$
            return 0;
        }
    }

    for( i = 0 ; i < numInputs ; i++ ) {
        // search for $i$th input in $x$'s inputs

        for( j = 0 ; j < numInputs ; j++ ) {
            const Equivalence_t *xInp = x->Input(j);
            if( inputs[i]->IsEqual(xInp) )
                break;
        }
        if( j == numInputs ) {
            // $i$th input not found in $x$'s inputs
            return 0;
        }
    }

    return 1;
}

/**
 * Populates the outerVarLogProp of the parent eq node.
 * See Equivalence_t::SetOuterVarLogProp()
 */
void LogicalOp_t::SetOuterVarLogProp()
{
    if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) )
        cout << "Setting outerVarLogProp for Op = " << ID() << endl;

    int n = NumInputs();

    for( int i = 0 ; i < n ; i++ ) {
        Equivalence_t *inp = Input(i);
            assert(inp);
        if (!inp->IsOuterVarLogPropSet()) {
            inp->SetOuterVarLogProp();
        }
    }

    // Form the outerVarLogProp of the parent as the union of the
    // outerVarLogProp of all child eq nodes. Mark the parent as
    // correlated if at least one child is correlated. The overriden methods
    // in concrete logical op classes should Mark the parent as correlated if
    // the op by itself uses any correlation variables (in its predicate).
    if (!eqClass->IsOuterVarLogPropSet()) {
        for( int i = 0 ; i < n ; i++ ) {
            Equivalence_t *inp = Input(i);
            assert(inp);
            eqClass->UnionOuterVarLogProp(inp->OuterVarLogProp());
            if (inp->IsCorrelated())
                eqClass->MarkCorrelated();
        }
    }

    if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) )
        cout << "Setting outerVarLogProp for Op = " << ID() 
             << " Done" << endl;
}


// equivalence check
int Filter_t::IsEquivalent(const LogicalOp_t *x) const
{
    assert(x);

    // check for equality
    if( IsEqual(x) )
        return 1;

    if( !LogicalOp_t::IsEquivalent(x) )
        return 0;

    const Predicate_t *xPred = ((const Filter_t *) x)->Predicate();
    assert(xPred);
    return pred->IsEquivalent(xPred);
}

// equivalence check
int Rel_t::IsEquivalent(const LogicalOp_t* x) const
{
    assert(x);

    // check for equality
    if( IsEqual(x) )
        return 1;

    if( !LogicalOp_t::IsEquivalent(x) )
        return 0;

    const char *xName = ((const Rel_t *) x)->Name();
    if( strcmp(name, xName) != 0 )
        return 0;

     return 1;
}

// equivalence check
int Aggregate_t::IsEquivalent(const LogicalOp_t* x) const
{
    assert(x);

    // check for equality
    if( IsEqual(x) )
        return 1;

    if( !LogicalOp_t::IsEquivalent(x) )
        return 0;

    return IsAggEquivalent(x) && IsGroupByEquivalent(x);
}

// equivalence check for the aggregate lists
int Aggregate_t::IsAggEquivalent(const LogicalOp_t* x) const
{
    assert(x);

    // check for equality
    if( IsEqual(x) )
        return 1;

    if( !LogicalOp_t::IsEquivalent(x) )
        return 0;

    const Aggregate_t *p = (const Aggregate_t *) x;

    int numAggAttrs = NumAggAttrs();
    if( p->NumAggAttrs() != numAggAttrs )
        return 0;

    int i = 0, j = 0;
    for( i = 0 ; i < numAggAttrs ; i++ ) {
        const Expr_t *pAggExpr = p->AggExpr(i);

        for( j = 0 ; j < numAggAttrs ; j++ ) {
            const Expr_t *aggExpr = AggExpr(j);
            if( aggExpr->IsEquivalent(pAggExpr) )
                break;
        }
        if( j == numAggAttrs ) {
            return 0;
        }
    }

    for( i = 0 ; i < numAggAttrs; i++ ) {
        const Expr_t *aggExpr = AggExpr(i);

        for( j = 0 ; j < numAggAttrs ; j++ ) {
            const Expr_t *pAggExpr = p->AggExpr(j);
            if( aggExpr->IsEquivalent(pAggExpr) )
                break;
        }
        if( j == numAggAttrs ) {
            return 0;
        }
    }

    return 1;
}

// equivalence check for the group-by lists
int Aggregate_t::IsGroupByEquivalent(const LogicalOp_t* x) const
{
    assert(x);

    // check for equality
    if( IsEqual(x) )
        return 1;

    if( !LogicalOp_t::IsEquivalent(x) )
        return 0;

    const Aggregate_t *p = (const Aggregate_t *) x;

    int numGroupByAttrs = NumGroupByAttrs();
    if( p->NumGroupByAttrs() != numGroupByAttrs )
        return 0;

    int i = 0, j = 0;
    for( i = 0 ; i < numGroupByAttrs ; i++ ) {
        const Expr_t *pGroupByExpr = p->GroupByExpr(i);

        for( j = 0 ; j < numGroupByAttrs ; j++ ) {
            const Expr_t *groupByExpr = GroupByExpr(j);
            if( groupByExpr->IsEquivalent(pGroupByExpr) )
                break;
        }
        if( j == numGroupByAttrs ) {
            return 0;
        }
    }

    for( i = 0 ; i < numGroupByAttrs; i++ ) {
        const Expr_t *groupByExpr = GroupByExpr(i);

        for( j = 0 ; j < numGroupByAttrs ; j++ ) {
            const Expr_t *pGroupByExpr = p->GroupByExpr(j);
            if( groupByExpr->IsEquivalent(pGroupByExpr) )
                break;
        }
        if( j == numGroupByAttrs ) {
            return 0;
        }
    }

    return 1;
}

// equivalence check
int Project_t::IsEquivalent(const LogicalOp_t* x) const
{
    assert(x);

    // check for equality
    if( IsEqual(x) )
        return 1;

    if( !LogicalOp_t::IsEquivalent(x) )
        return 0;

    int numProj = NumProj();
    const Project_t *p = (const Project_t *) x;
    
    if( p->NumProj() != numProj )
        return 0;

    int i = 0, j = 0;
    for( i = 0 ; i < numProj ; i++ ) {
        // search for $p$'s $i$th projection in $proj$
        const Expr_t *pProjExpr = p->ProjExpr(i);

        for( j = 0 ; j < numProj ; j++ ) {
            const Expr_t *projExpr = ProjExpr(j);
            if( projExpr->IsEquivalent(pProjExpr) )
                break;
        }
        if( j == numProj ) {
            // $p$'s $i$th projection not found in $proj$
            return 0;
        }
    }

    for( i = 0 ; i < numProj ; i++ ) {
        // search for $proj[i]$ in $p$'s projections
        const Expr_t *projExpr = ProjExpr(i);

        for( j = 0 ; j < numProj ; j++ ) {
            const Expr_t *pProjExpr = p->ProjExpr(j);
            if( projExpr->IsEquivalent(pProjExpr) )
                break;
        }
        if( j == numProj ) {
            // $proj[i]$ not found in $p$'s projections
            return 0;
        }
    }

    return 1;
}

// logical property computation for union
LogProp_t *Union_t::ComputeLogicalProp(void) const
{
    const Equivalence_t *leftInput = Input(0);
    assert(leftInput);
    LogProp_t *leftLP = leftInput->LogicalProp();
                                        // left input's logical prop
    assert(leftLP);

    const Equivalence_t *rightInput = Input(1);
    assert(rightInput);
    LogProp_t *rightLP = rightInput->LogicalProp();
                                        // right input's logical prop
    assert(rightLP);

    // the left and right schema must be identical --- return the
    // left schema (arbitrary choice)
    Schema_t *schema = leftLP->Schema();// left input's schema
    Refer(schema);

    SchemaProp_t *leftSchemaProp = leftLP->SchemaProp();
                                        // left input's schema prop
    assert(leftSchemaProp);

    SchemaProp_t *rightSchemaProp = rightLP->SchemaProp();
                                        // right left input's schema prop
    assert(rightSchemaProp);

    int leftNumAttr = leftSchemaProp->NumAttr();
    int rightNumAttr = rightSchemaProp->NumAttr();

    // --- just to make things work! does not matter since union is dummy ---
    if( leftNumAttr > rightNumAttr )
        leftNumAttr = rightNumAttr;
    else if( leftNumAttr < rightNumAttr )
        rightNumAttr = leftNumAttr;

    assert(leftNumAttr == rightNumAttr);
    // create the new schema prop
    SchemaProp_t *schemaProp = new SchemaProp_t(leftNumAttr);
    Refer(schemaProp);
    for( int i = 0 ; i < leftNumAttr ; i++ ) {
        AttrProp_t *leftProp = leftSchemaProp->AttributeProp(i);
        AttrProp_t *rightProp = rightSchemaProp->AttributeProp(i);
        AttrProp_t *attrProp = leftProp->UnionProp(rightProp);
        Refer(attrProp);
        schemaProp->SetAttributeProp(i, attrProp);
        DeRefer(attrProp);
        // Ravi: TODO: Update AttrPropRep (attr eq classes) 
    }

    Card_t leftRelSize_Tuples = leftLP->RelSize_Tuples();
    Card_t rightRelSize_Tuples = rightLP->RelSize_Tuples();
    Card_t relSize_Tuples = leftRelSize_Tuples + rightRelSize_Tuples;

    FixNumDistincts(schemaProp, relSize_Tuples);
    LogProp_t *lp =
        new IntRelLogProp_t(NULL, schema, schemaProp, relSize_Tuples);
    DeRefer(schema);
    DeRefer(schemaProp);

    return lp;
}

// logical property computation for project
LogProp_t *Project_t::ComputeLogicalProp(void) const
{
    const Equivalence_t *inp = Input(0);
    assert(inp);
    LogProp_t *inpLP = inp->LogicalProp();
    assert(inpLP);

    // input schema
    Schema_t *inpSchema = inpLP->Schema();
    assert(inpSchema);

    // input schema property
    SchemaProp_t *inpSchemaProp = inpLP->SchemaProp();
    assert(inpSchemaProp);

    int numProj = NumProj();

    Schema_t *schema = new Schema_t(numProj);
    Refer(schema);

    SchemaProp_t *schemaProp = new SchemaProp_t(numProj);
    Refer(schemaProp);

    for( int i = 0 ; i < numProj ; i++ ) {
        // bind the project expression with the input schema
        Expr_t *e = ProjExpr(i);

        // FIX: only simple projection supported for now
        assert(e->ExprType() == REL_ARG_REF_T);

        RelArgRef_t *r = (RelArgRef_t *) e;
        // attribute number in input schema bound to the $i$th projection
        int bnd = r->Binding(inpSchema);
        if (bnd == -1) {
            if(Config_t::IsDebug(DEBUG_ERROR)) {
                cout << "Fatal Error: Invalid attribute ";
                r->PrintExpr();
                cout << endl;
            }
            exit(1);
        }

        // set the schema attribute
        Attr_t *a = inpSchema->Attribute(bnd);
        schema->SetAttribute(i, a);

        // set the schema attribute property
        AttrProp_t *aProp = inpSchemaProp->AttributeProp(bnd);
        schemaProp->SetAttributeProp(i, aProp);

        // Copy the old eq class head. Will be overwritten in the next step.
        schemaProp->SetAttrPropRep(i, inpSchemaProp->AttrPropRep(bnd));
    }

    // Now update all the old eq class heads with a new head. The first 
    // attribute index in the eq class will be chosen as the new head.
    for( int i = numProj-1 ; i >= 0; i-- ) {
        int oldhead = schemaProp->AttrPropRep(i);
        int newhead = i;
        for (int j = i-1; j >= 0; j--) {
            if(schemaProp->AttrPropRep(j) == oldhead)
                newhead = j;
        }
        schemaProp->SetAttrPropRep(i, newhead);
    }

    // rel size remains same
    Card_t relSize_Tuples = inpLP->RelSize_Tuples();
    FixNumDistincts(schemaProp, relSize_Tuples);
    LogProp_t *lp =
        new IntRelLogProp_t(NULL, schema, schemaProp, relSize_Tuples);
    DeRefer(schema);
    DeRefer(schemaProp);

    return lp;
}

// logical property computation for aggregate
LogProp_t *Aggregate_t::ComputeLogicalProp(void) const
{
    const Equivalence_t *inp = Input(0);
    assert(inp);
    LogProp_t *inpLP = inp->LogicalProp();
    assert(inpLP);

    // input schema
    Schema_t *inpSchema = inpLP->Schema();
    assert(inpSchema);

    // input schema property
    SchemaProp_t *inpSchemaProp = inpLP->SchemaProp();
    assert(inpSchemaProp);

    int numGroupByAttrs = NumGroupByAttrs();
    int numAggAttrs = NumAggAttrs();
    int numAttrs = numGroupByAttrs + numAggAttrs;

    Schema_t *schema = new Schema_t(numAttrs);
    Refer(schema);

    SchemaProp_t *schemaProp = new SchemaProp_t(numAttrs);
    Refer(schemaProp);

    int i = 0, j = 0;

    // group-by attributes
    for( i = 0 ; i < numGroupByAttrs ; i++ ) {
        // bind the project expression with the input schema
        Expr_t *e = GroupByExpr(i);
        assert(e);

        // FIX: no complex expressions allowed
        assert(e->ExprType() == REL_ARG_REF_T);

        RelArgRef_t *r = (RelArgRef_t *) e;
        // attribute number in input schema bound to the $i$th projection
        int bnd = r->Binding(inpSchema);        
        assert(bnd != -1);

        // set the schema attribute
        Attr_t *a = inpSchema->Attribute(bnd);
        assert(a);
        schema->SetAttribute(j, a);

        // set the schema attribute property
        AttrProp_t *aProp = inpSchemaProp->AttributeProp(bnd);
        assert(aProp);
        schemaProp->SetAttributeProp(j, aProp);
        // Copy the old eq class head. Will be overwritten in the next step.
        schemaProp->SetAttrPropRep(j, inpSchemaProp->AttrPropRep(bnd));
        j++;
    }

    // Now update all the old eq class heads with a new head. The first 
    // attribute index in the eq class will be chosen as the new head.
    for( int k = numGroupByAttrs-1 ; k >= 0; k-- ) {
        int oldhead = schemaProp->AttrPropRep(k);
        int newhead = k;
        for (int m = k-1; m >= 0; m--) {
            if(schemaProp->AttrPropRep(m) == oldhead)
                newhead = m;
        }
        schemaProp->SetAttrPropRep(k, newhead);
    }
 
    for( i = 0 ; i < numAggAttrs ; i++ ) {
        // bind the project expression with the input schema
        Expr_t *e = AggExpr(i);
        assert(e);

        assert(e->ExprType() == AGG_EXPR_T);
        AggExpr_t *aggExpr = (AggExpr_t *) e;

        Expr_t *eInp = aggExpr->Input();
        assert(eInp->ExprType() == REL_ARG_REF_T);
        RelArgRef_t *rInp = (RelArgRef_t *) eInp;

        // attribute number in input schema bound to the $i$th projection
        int bnd = rInp->Binding(inpSchema);     
        assert(bnd != -1);

        // DIRTY FIX: we keep the name of the aggregate attribute as
        // the name of the aggregated attribute --- simplifies matters
        // for cascaded attributes. We assume only max/min type of
        // aggregations so that the maxval/minval/numDistinct properties
        // of the attribute remain the same after aggregation
        // Fixing this requires some thought --- will do it sometime

        // set the aggregated schema attribute
        Attr_t *a = inpSchema->Attribute(bnd);
        assert(a);
        schema->SetAttribute(j, a);

        // set the schema attribute property
        AttrProp_t *aProp = inpSchemaProp->AttributeProp(bnd);
        assert(aProp);
        schemaProp->SetAttributeProp(j, aProp);
        j++;
    }

    // max rel size is the product of the distinct values of the
    // groupBy attributes
    Card_t inpRelSize_Tuples = inpLP->RelSize_Tuples();
    Card_t relSize_Tuples = 1;
    for( j = 0 ; j < numGroupByAttrs ; j++ ) {
        AttrProp_t *aProp = schemaProp->AttributeProp(j);
        Card_t aDistinct = aProp->NumDistinct();
        assert(aDistinct >= 1);
        relSize_Tuples *= aDistinct;

        // output size should be at most input size
        if( relSize_Tuples > inpRelSize_Tuples ) {
            relSize_Tuples = inpRelSize_Tuples;
            break;
        }
    }

    FixNumDistincts(schemaProp, relSize_Tuples);

    LogProp_t *lp =
        new IntRelLogProp_t(NULL, schema, schemaProp, relSize_Tuples);
    DeRefer(schema);
    DeRefer(schemaProp);

    return lp;
}

// logical property computation for select
LogProp_t *Select_t::ComputeLogicalProp(void) const
{
    const Predicate_t *predicate = Predicate();
    assert(predicate);

    const Equivalence_t *inp = Input(0);
    assert(inp);
    LogProp_t *inpLP = inp->LogicalProp();
    assert(inpLP);

    // schema remains unchanged
    Schema_t *schema = inpLP->Schema();
    Refer(schema);

    SchemaProp_t *inpSchemaProp = inpLP->SchemaProp();
    assert(inpSchemaProp);
    int numAttr = inpSchemaProp->NumAttr();
                                        // number of attributes

    // initialize schema property with the input schema property
    SchemaProp_t *schemaProp = new SchemaProp_t(numAttr);
    Refer(schemaProp);
    for( int i = 0 ; i < numAttr ; i++ ) {
        AttrProp_t *inpAttrProp = inpSchemaProp->AttributeProp(i);
        schemaProp->SetAttributeProp(i, inpAttrProp);
        schemaProp->SetAttrPropRep(i, inpSchemaProp->AttrPropRep(i));
    }

    // recursively refines the schema property using the predicate and
    // returns the selectivity
    float selectivity = predicate->InferProp(schema, schemaProp);
    if( selectivity == 0 ) {
        cout << "--- ZERO SELECTIVITY ---" << endl;
        predicate->PrintExpr();
        selectivity = 1.0/Config_t::CardLimit();
    }
    assert(selectivity > 0);

    Card_t inpRelSize_Tuples = inpLP->RelSize_Tuples();
    // TODO: TEMPCODE : Changed for self-join
    // Card_t relSize_Tuples = ceil(selectivity * inpRelSize_Tuples);
    Card_t relSize_Tuples = inpRelSize_Tuples;
    FixNumDistincts(schemaProp, relSize_Tuples);

    LogProp_t *lp =
        new IntRelLogProp_t(NULL, schema, schemaProp, relSize_Tuples);
    DeRefer(schema);
    DeRefer(schemaProp);

    return lp;
}

// logical property computation for join
LogProp_t *Join_t::ComputeLogicalProp(void) const
{
    const Predicate_t *predicate = Predicate();
    assert(predicate);

    const Equivalence_t *leftinp = Input(0);

    assert(leftinp);
    LogProp_t *leftLP = leftinp->LogicalProp();
                                        // left input's logical prop
    assert(leftLP);

    Schema_t *leftSchema = leftLP->Schema();
                                        // left input's schema
    assert(leftSchema);

    const Equivalence_t *rightinp = Input(1);

    assert(rightinp);
    LogProp_t *rightLP = rightinp->LogicalProp();
                                        // right input's logical prop
    assert(rightLP);
    Schema_t *rightSchema = rightLP->Schema();
                                        // right input's schema
    assert(rightSchema);

    // create the new schema
    Schema_t *schema = MergeSchema(leftSchema, rightSchema);
    Refer(schema);

    SchemaProp_t *leftSchemaProp = leftLP->SchemaProp();
                                        // left input's schema prop
    assert(leftSchemaProp);

    SchemaProp_t *rightSchemaProp = rightLP->SchemaProp();
                                        // right left input's schema prop
    assert(rightSchemaProp);

    // create the new schema prop
    SchemaProp_t *schemaProp = MergeSchemaProp(leftSchemaProp, rightSchemaProp);
    Refer(schemaProp);

    // recursively refines the schema property using the predicate and
    // returns the selectivity

    float selectivity = predicate->InferProp(schema, schemaProp);

    if( selectivity == 0 ) {
        cout << "--- ZERO SELECTIVITY ---" << endl;
        predicate->PrintExpr();
        selectivity = 1.0/Config_t::CardLimit();
    }
    assert(selectivity > 0);

    Card_t leftRelSize_Tuples = leftLP->RelSize_Tuples();
    Card_t rightRelSize_Tuples = rightLP->RelSize_Tuples();
    // TODO: TEMPCODE: Changed just for a self join case (CHANGE LATER)
    // Card_t relSize_Tuples =
    //    ceil(selectivity * leftRelSize_Tuples * rightRelSize_Tuples);
    Card_t relSize_Tuples = leftRelSize_Tuples;
    if( !finite(relSize_Tuples) ) {
        cout << "--- CARDINALITY OVERFLOW ---" << endl;
        relSize_Tuples = Config_t::CardLimit();
    }

    FixNumDistincts(schemaProp, relSize_Tuples);

    LogProp_t *lp =
        new IntRelLogProp_t(NULL, schema, schemaProp, relSize_Tuples);
    DeRefer(schema);
    DeRefer(schemaProp);

    return lp;
}

// logical property computation
LogProp_t *Rel_t::ComputeLogicalProp(void) const
{
    return Catalog->GetRelLogProp(name);
}

// instantiate a new select with additional predicate pushed in
LogicalOp_t *Select_t::PushInPred(Predicate_t *inPred, Memo_t&)
{
    if( inPred->IsEquivalent(ConstPreds.True()) )
        return NULL;

    Predicate_t *selectPred = Predicate();
    Predicate_t *newSelectPred = NULL;

    if( selectPred->IsEquivalent(ConstPreds.True()) ) {
        newSelectPred = inPred;
    } else if( inPred->Implies(selectPred) ) {
        newSelectPred = inPred;
    } else if( selectPred->Implies(inPred) ) {
        newSelectPred = selectPred;
    } else {
        newSelectPred = new And_t(inPred, selectPred);
    } 

    Refer(newSelectPred);
    Select_t *newSelect = new Select_t(newSelectPred);
    DeRefer(newSelectPred);

    newSelect->MarkDerived();

    Equivalence_t *inp = Input(0);
    assert(inp);

    newSelect->SetInput(0, inp);

    return newSelect;
}

// instantiate a new join with additional predicate pushed in
LogicalOp_t *Join_t::PushInPred(Predicate_t *inPred, Memo_t&)
{
    if( inPred->IsEquivalent(ConstPreds.True()) )
        return NULL;

    Predicate_t *joinPred = Predicate();
    Predicate_t *newJoinPred = NULL;

    if( joinPred->IsEquivalent(ConstPreds.True()) ) {
        newJoinPred = inPred;
    } else if( inPred->Implies(joinPred) ) {
        newJoinPred = inPred;
    } else if( joinPred->Implies(inPred) ) {
        newJoinPred = joinPred;
    } else {
        newJoinPred = new And_t(inPred, joinPred);
    } 

    Refer(newJoinPred);
    Join_t *newJoin = new Join_t(newJoinPred);
    DeRefer(newJoinPred);

    newJoin->MarkDerived();

    Equivalence_t *leftInp = Input(0);
    assert(leftInp);

    Equivalence_t *rightInp = Input(1);
    assert(rightInp);

    newJoin->SetInput(0, leftInp);
    newJoin->SetInput(1, rightInp);

    return newJoin;
}

// push the project across the select
// returns the new operator if transformation possible; NULL if not
LogicalOp_t *Select_t::PushInProj(Project_t *inProj, Memo_t& memo)
{
    assert(inProj);

    LogProp_t *lp = LogicalProp();
    assert(lp);

    Schema_t *schema = lp->Schema();
    assert(schema);

    // get the schema for inProj
    Equivalence_t *eq = inProj->EqClass();
    assert(eq);

    LogProp_t *inProjLP = eq->LogicalProp();
    assert(inProjLP);

    Schema_t *inProjSchema = inProjLP->Schema();
    assert(inProjSchema);

    Predicate_t *selectPred = Predicate();
    assert(selectPred);

    if( !selectPred->IsValidSchema(schema, inProjSchema) )
        return NULL;

    // inProject can be pushed in
    // create new project operator

    AttrMap_t *inAttrMap = inProj->AttrMap();

    Project_t *newProj = new Project_t(inAttrMap);
    newProj->MarkDerived();

    Equivalence_t *inp = Input(0);
    assert(inp);

    newProj->SetInput(0, inp);

    // get the equivalence class corresponding to newProj
    Equivalence_t *newProjEq = NULL;
    LogicalOp_t *prev = memo.Search(newProj);
    if( prev ) {
        // already present
        delete newProj;
        newProj = NULL;
        newProjEq = prev->EqClass();
    } else {
        LogProp_t *newProjLP = newProj->LogicalProp();
        Refer(newProjLP);

        newProjEq = new Equivalence_t(newProjLP);
        DeRefer(newProjLP);

        newProj->SetEqClass(newProjEq);
        newProjEq->AddLogExpr(newProj);

        memo.Insert(newProj);
    }
    assert(newProjEq);

    Select_t *newSelect = new Select_t(selectPred);
    newSelect->MarkDerived();

    newSelect->SetInput(0, newProjEq);

    return newSelect;
}

// partition the project map entries into valid and invalid entries
// with respect to the schema
void AttrMap_t::Decompose(Schema_t *schema,
                    AttrMap_t **valid, AttrMap_t **invalid) const
{
    assert(schema);
    assert(valid);
    assert(invalid);

    PrependList_t<AttrMapEntry_t *> validList;
    PrependList_t<AttrMapEntry_t *> invalidList;

    int numValid = 0;
    int numInvalid = 0;

    for( int i = 0 ; i < numProj ; i++ ) {
        AttrMapEntry_t *e = Entry(i);
        assert(e);

        Expr_t *expr = e->Expr();
        assert(expr);
        assert(expr->ExprType() == REL_ARG_REF_T);

        RelArgRef_t *ref = (RelArgRef_t *) expr;
        int bnd = ref->Binding(schema);

        if( bnd == -1 ) {
            // does not bind -- invalid entry
            invalidList.Insert(e);
            numInvalid++;
        } else {
            // binds -- valid entry
            validList.Insert(e);
            numValid++;
        }
    }

    *valid = new AttrMap_t(numValid);

    ListIter_t<AttrMapEntry_t *> validIter;
    validIter.Attach(&validList);

    int validIndex = 0;
    while( !validIter.IsEnd() ) {
        AttrMapEntry_t *e = validIter.Next();

        (*valid)->SetProj(validIndex++, e);
    }

    *invalid = new AttrMap_t(numInvalid);

    ListIter_t<AttrMapEntry_t *> invalidIter;
    invalidIter.Attach(&invalidList);

    int invalidIndex = 0;
    while( !invalidIter.IsEnd() ) {
        AttrMapEntry_t *e = invalidIter.Next();

        (*invalid)->SetProj(invalidIndex++, e);
    }
}

// push the project across the join
// returns the new operator if transformation possible; NULL if not
LogicalOp_t *Join_t::PushInProj(Project_t *inProj, Memo_t& memo)
{
    assert(inProj);

    LogProp_t *lp = LogicalProp();
    assert(lp);

    Schema_t *schema = lp->Schema();
    assert(schema);

    // get the schema for inProj
    Equivalence_t *eq = inProj->EqClass();
    assert(eq);

    LogProp_t *inProjLP = eq->LogicalProp();
    assert(inProjLP);

    Schema_t *inProjSchema = inProjLP->Schema();
    assert(inProjSchema);

    Predicate_t *joinPred = Predicate();
    assert(joinPred);

    if( !joinPred->IsValidSchema(schema, inProjSchema) )
        return NULL;

    Equivalence_t *left = Input(0);
    assert(left);
    Equivalence_t *right = Input(1);
    assert(right);

    // get the schema for the left input
    LogProp_t *leftLP = left->LogicalProp();
    assert(leftLP);

    Schema_t *leftSchema = leftLP->Schema();

    AttrMap_t *inAttrMap = inProj->AttrMap();

    AttrMap_t *leftAttrMap = NULL;
    AttrMap_t *rightAttrMap = NULL;
    inAttrMap->Decompose(leftSchema, &leftAttrMap, &rightAttrMap);
    Refer(leftAttrMap);
    Refer(rightAttrMap);

    Project_t *leftProj = new Project_t(leftAttrMap);
    leftProj->MarkDerived();

    leftProj->SetInput(0, left);

    Equivalence_t *newLeft = NULL;
    LogicalOp_t *prevLeftProj = memo.Search(leftProj);
    if( prevLeftProj ) {
        // exists already
        delete leftProj;
        leftProj = NULL;
        newLeft = prevLeftProj->EqClass();
    } else {
        // does not exist
        LogProp_t *newLeftLP = leftProj->LogicalProp();
        Refer(newLeftLP);

        newLeft = new Equivalence_t(newLeftLP);
        DeRefer(newLeftLP);

        leftProj->SetEqClass(newLeft);
        newLeft->AddLogExpr(leftProj);

        memo.Insert(leftProj);
    }

    Project_t *rightProj = new Project_t(rightAttrMap);
    rightProj->MarkDerived();

    rightProj->SetInput(0, right);

    Equivalence_t *newRight = NULL;
    LogicalOp_t *prevRightProj = memo.Search(rightProj);
    if( prevRightProj ) {
        // exists already
        delete rightProj;
        rightProj = NULL;
        newRight = prevRightProj->EqClass();
    } else {
        // does not exist
        LogProp_t *newRightLP = rightProj->LogicalProp();
        Refer(newRightLP);

        newRight = new Equivalence_t(newRightLP);
        DeRefer(newRightLP);

        rightProj->SetEqClass(newRight);
        newRight->AddLogExpr(rightProj);

        memo.Insert(rightProj);
    }

    Join_t *newJoin = new Join_t(joinPred);
    newJoin->MarkDerived();

    assert(newLeft);
    assert(newRight);
    newJoin->SetInput(0, newLeft);
    newJoin->SetInput(1, newRight);

    DeRefer(leftAttrMap);
    DeRefer(rightAttrMap);

    return newJoin;
}

// --- currently, this is just a virtual root ---
// generates merge operator
// inputs are optimized by the optimizer provided
// update the bestPlan if a better plan is found
// NOTE: totalCost not modified
void Union_t::ApplyAlgorithms(PlanGroup_t *planGroup, Volcano_t *opt,
            Cost_t *totalCost, Plan_t *bestPlan, CostVal_t *costLimit,
        Order_t *gpso, int *plmap, int level, Card_t callCount)
{
    ApplyFwdAlgorithms(planGroup, opt, totalCost, bestPlan, costLimit,
                       gpso, plmap, level, callCount);
    if( IsRoot() ) {
        if( costLimit )
            cout << "fwd cost = " << *costLimit << endl;
        if( opt->IsRevRU() ) {
            ApplyRevAlgorithms(planGroup, opt, totalCost, bestPlan, costLimit,
                               gpso, plmap, level, callCount);
            if( costLimit )
                cout << "rev cost = " << *costLimit << endl;
        }
    }
}

// --- currently, this is just a virtual root ---
// generates merge operator
// inputs are optimized by the optimizer provided: ordering = l to r
// update the bestPlan if a better plan is found
// NOTE: totalCost not modified
void Union_t::ApplyFwdAlgorithms(PlanGroup_t *planGroup, Volcano_t *opt,
            Cost_t *totalCost, Plan_t *bestPlan, CostVal_t *costLimit,
            Order_t *gpso, int *plmap, int level, Card_t callCount)
{
    assert(planGroup);
    assert(opt);
    assert(totalCost);
    assert(bestPlan);

    // input equivalence classes
    Equivalence_t *firstInpEq = Input(0);
    assert(firstInpEq);
    Equivalence_t *secondInpEq = Input(1);
    assert(secondInpEq);

    // logical property of the inputs
    LogProp_t *firstInpLP = firstInpEq->LogicalProp();
    assert(firstInpLP);
    LogProp_t *secondInpLP = secondInpEq->LogicalProp();
    assert(secondInpLP);

    // logical property of the output
    LogProp_t *outLP = LogicalProp();
    Refer(outLP);

    // calculate the costLimit
    // Ravi: Costing change, multiplying by callCount
    CostVal_t mergeCost = callCount * Merge_t::Cost(firstInpLP, secondInpLP, outLP);
    DeRefer(outLP);
    CostVal_t inpCostLimit = -1;

    if( costLimit ) {
        // pruning enabled
        inpCostLimit = *costLimit - mergeCost;

        // if not feasible, return
        if( inpCostLimit < totalCost->Value() )
            return;
    }

    // required physical properties in the input -- no change
    PhyPropGroup_t *inpReqProp = planGroup->PropGroup();
    Refer(inpReqProp);

    // now optimize the inputs
    CostVal_t firstInpCostLimit = inpCostLimit;

    PlanGroup_t *firstInpPlanGroup = NULL;
    if( costLimit ) {
        // pruning enabled
        firstInpPlanGroup = opt->FindBestPlan(firstInpEq, inpReqProp,
                                                totalCost, &firstInpCostLimit,
                                              0, gpso, plmap, level, callCount);
    } else {
        firstInpPlanGroup = opt->FindBestPlan(firstInpEq, inpReqProp,
                                                        totalCost, NULL,
                                              0, gpso, plmap, level, callCount);
        assert(firstInpPlanGroup);
    }

    // if no feasible plan found, return
    if( !firstInpPlanGroup ) {
        assert(costLimit);
        DeRefer(inpReqProp);
        return;
    }

    Plan_t *firstInpBestPlan = firstInpPlanGroup->BestPlan();
    assert(firstInpBestPlan);
    Cost_t *firstInpCost = firstInpBestPlan->Cost();

    if( opt->IsRU() && firstInpPlanGroup->IsMarked() &&
            firstInpEq->UseCost() < firstInpBestPlan->CostVal() )
        firstInpCost = firstInpEq->UseCostADT(opt);

    assert(firstInpCost);

    // this is for the virtual root --- should go if union is to be a
    // real operator
    if( opt->IsRU() ) {
        opt->MarkBestPlan(firstInpPlanGroup);
        opt->NextQuery();
    }

    // update totalCost for the second input optimization
    totalCost->Add(firstInpCost);

    CostVal_t secondInpCostLimit = inpCostLimit;

    PlanGroup_t *secondInpPlanGroup = NULL;
    if( costLimit ) {
        // pruning enabled
        secondInpPlanGroup = opt->FindBestPlan(secondInpEq, inpReqProp,
                                                totalCost, &secondInpCostLimit,
                                              0, gpso, plmap, level, callCount);

    } else {
        secondInpPlanGroup = opt->FindBestPlan(secondInpEq, inpReqProp,
                                                totalCost, NULL,
                                              0, gpso, plmap, level, callCount);
        assert(secondInpPlanGroup);
    }

    // restore totalCost
    totalCost->Subtract(firstInpCost);

    // if no feasible plan found, return
    if( !secondInpPlanGroup ) {
        DeRefer(inpReqProp);
        return;
    }

    Plan_t *secondInpBestPlan = secondInpPlanGroup->BestPlan();
    assert(secondInpBestPlan);
    Cost_t *secondInpCost = secondInpBestPlan->Cost();
    if( opt->IsRU() && secondInpPlanGroup->IsMarked() &&
            secondInpEq->UseCost() < secondInpBestPlan->CostVal() )
        secondInpCost = secondInpEq->UseCostADT(opt);

    assert(secondInpCost);

    DeRefer(inpReqProp);

    // now instantiate the merge operator and set its input as the
    // plan obtained as above to get the required plan
    Merge_t *merge = new Merge_t(this);
    merge->SetInput(0, firstInpPlanGroup);
    merge->SetInput(1, secondInpPlanGroup);
    merge->SetCostVal(mergeCost);

    // calculate the cost
    Cost_t *cost = opt->CreateCost(merge, mergeCost);
    Refer(cost);
    cost->Add(firstInpCost);
    cost->Add(secondInpCost);

    // add the plan to the plan group
    planGroup->AddAlgPlan(merge, cost);

    // current total plan cost
    CostVal_t curCost = totalCost->SumCost(cost);

    // update the best plan if found plan better
    int isBetterPlan = 1;

    // if pruning enabled, we know that we have arrived at a better
    // plan, so a check is not necessary
    if( !costLimit ) {
        // pruning disabled
        Cost_t *prevCost = bestPlan->Cost();
        if( prevCost ) {
            CostVal_t prevBestCost = totalCost->SumCost(prevCost);
            if( curCost >= prevBestCost )
                isBetterPlan = 0;
        }
    }
    
    if( isBetterPlan ) {
        bestPlan->SetRoot(merge);
        bestPlan->SetCost(cost);

        if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) ) {
            bestPlan->PrintPlan();
            cout << endl;
            cout << endl;
        }
    }

    if( costLimit ) {
        // update the costLimit
        *costLimit = curCost;
    }

    DeRefer(cost);

    // this is for the virtual root --- should go if union is to be a
    // real operator
    if( opt->IsRU() ) {
        opt->UnMarkBestPlan(firstInpPlanGroup);
    }
}

// --- currently, this is just a virtual root ---
// generates merge operator
// inputs are optimized by the optimizer provided: ordering = r to l
// update the bestPlan if a better plan is found
// NOTE: totalCost not modified
void Union_t::ApplyRevAlgorithms(PlanGroup_t *planGroup, Volcano_t *opt,
            Cost_t *totalCost, Plan_t *bestPlan, CostVal_t *costLimit,
            Order_t *gpso, int *plmap, int level, Card_t callCount)
{
    assert(planGroup);
    assert(opt);
    assert(totalCost);
    assert(bestPlan);

    // input equivalence classes
    Equivalence_t *firstInpEq = Input(1);
    assert(firstInpEq);
    Equivalence_t *secondInpEq = Input(0);
    assert(secondInpEq);

    // logical property of the inputs
    LogProp_t *firstInpLP = firstInpEq->LogicalProp();
    assert(firstInpLP);
    LogProp_t *secondInpLP = secondInpEq->LogicalProp();
    assert(secondInpLP);

    // logical property of the output
    LogProp_t *outLP = LogicalProp();
    Refer(outLP);

    // calculate the costLimit
    // Ravi: Costing change, multiplying by callCount
    CostVal_t mergeCost = callCount * Merge_t::Cost(firstInpLP, secondInpLP, outLP);
    DeRefer(outLP);
    CostVal_t inpCostLimit = -1;

    if( costLimit ) {
        // pruning enabled
        inpCostLimit = *costLimit - mergeCost;

        // if not feasible, return
        if( inpCostLimit < totalCost->Value() )
            return;
    }

    // required physical properties in the input -- no change
    PhyPropGroup_t *inpReqProp = planGroup->PropGroup();
    Refer(inpReqProp);

    // now optimize the inputs
    CostVal_t firstInpCostLimit = inpCostLimit;

    PlanGroup_t *firstInpPlanGroup = NULL;
    if( costLimit ) {
        // pruning enabled
        firstInpPlanGroup = opt->FindBestPlan(firstInpEq, inpReqProp,
                                                totalCost, &firstInpCostLimit,
                                              0, gpso, plmap, level, callCount);
    } else {
        firstInpPlanGroup = opt->FindBestPlan(firstInpEq, inpReqProp,
                                                        totalCost, NULL,
                                              0, gpso, plmap, level, callCount);
        assert(firstInpPlanGroup);
    }

    // if no feasible plan found, return
    if( !firstInpPlanGroup ) {
        assert(costLimit);
        DeRefer(inpReqProp);
        return;
    }

    Plan_t *firstInpBestPlan = firstInpPlanGroup->BestPlan();
    assert(firstInpBestPlan);
    Cost_t *firstInpCost = firstInpBestPlan->Cost();

    if( opt->IsRU() && firstInpPlanGroup->IsMarked() &&
            firstInpEq->UseCost() < firstInpBestPlan->CostVal() )
        firstInpCost = firstInpEq->UseCostADT(opt);

    assert(firstInpCost);

    // this is for the virtual root --- should go if union is to be a
    // real operator
    if( opt->IsRU() ) {
        opt->MarkBestPlan(firstInpPlanGroup);
        opt->NextQuery();
    }

    // update totalCost for the second input optimization
    totalCost->Add(firstInpCost);

    CostVal_t secondInpCostLimit = inpCostLimit;

    PlanGroup_t *secondInpPlanGroup = NULL;
    if( costLimit ) {
        // pruning enabled
        secondInpPlanGroup = opt->FindBestPlan(secondInpEq, inpReqProp,
                                                totalCost, &secondInpCostLimit,
                                              0, gpso, plmap, level, callCount);

    } else {
        secondInpPlanGroup = opt->FindBestPlan(secondInpEq, inpReqProp,
                                                totalCost, NULL,
                                              0, gpso, plmap, level, callCount);
        assert(secondInpPlanGroup);
    }

    // restore totalCost
    totalCost->Subtract(firstInpCost);

    // if no feasible plan found, return
    if( !secondInpPlanGroup ) {
        DeRefer(inpReqProp);
        return;
    }

    Plan_t *secondInpBestPlan = secondInpPlanGroup->BestPlan();
    assert(secondInpBestPlan);
    Cost_t *secondInpCost = secondInpBestPlan->Cost();
    if( opt->IsRU() && secondInpPlanGroup->IsMarked() &&
            secondInpEq->UseCost() < secondInpBestPlan->CostVal() )
        secondInpCost = secondInpEq->UseCostADT(opt);

    assert(secondInpCost);

    DeRefer(inpReqProp);

    // now instantiate the merge operator and set its input as the
    // plan obtained as above to get the required plan
    Merge_t *merge = new Merge_t(this);
    merge->SetInput(0, firstInpPlanGroup);
    merge->SetInput(1, secondInpPlanGroup);
    merge->SetCostVal(mergeCost);

    // calculate the cost
    Cost_t *cost = opt->CreateCost(merge, mergeCost);
    Refer(cost);
    cost->Add(firstInpCost);
    cost->Add(secondInpCost);

    // add the plan to the plan group
    planGroup->AddAlgPlan(merge, cost);

    // current total plan cost
    CostVal_t curCost = totalCost->SumCost(cost);

    // update the best plan if found plan better
    int isBetterPlan = 1;

    // if pruning enabled, we know that we have arrived at a better
    // plan, so a check is not necessary
    if( !costLimit ) {
        // pruning disabled
        Cost_t *prevCost = bestPlan->Cost();
        if( prevCost ) {
            CostVal_t prevBestCost = totalCost->SumCost(prevCost);
            if( curCost >= prevBestCost )
                isBetterPlan = 0;
        }
    }
    
    if( isBetterPlan ) {
        bestPlan->SetRoot(merge);
        bestPlan->SetCost(cost);

        if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) ) {
            bestPlan->PrintPlan();
            cout << endl;
            cout << endl;
        }
    }

    if( costLimit ) {
        // update the costLimit
        *costLimit = curCost;
    }

    DeRefer(cost);

    // this is for the virtual root --- should go if union is to be a
    // real operator
    if( opt->IsRU() ) {
        opt->UnMarkBestPlan(firstInpPlanGroup);
    }
}

// generates project scan operator
// inputs are optimized by the optimizer provided
// update the bestPlan if a better plan is found
void Project_t::ApplyAlgorithms(PlanGroup_t *planGroup, Volcano_t *opt,
            Cost_t *totalCost, Plan_t *bestPlan, CostVal_t *costLimit,
        Order_t *gpso, int *plmap, int level, Card_t callCount)
{
    assert(planGroup);
    assert(opt);
    assert(totalCost);
    assert(bestPlan);

    // input equivalence class
    Equivalence_t *inpEq = Input(0);
    assert(inpEq);

    // the logical property of the input
    LogProp_t *inpLP = inpEq->LogicalProp();
    assert(inpLP);

    // the logical property of the output
    LogProp_t *outLP = LogicalProp();
    Refer(outLP);

    // Ravi: Costing change, multiplying by callCount
    CostVal_t projCost = callCount * ProjectScan_t::Cost(inpLP, outLP);
    DeRefer(outLP);

    CostVal_t inpCostLimit = -1;
    if( costLimit ) {
        // pruning enabled
        // calculate the costLimit
        inpCostLimit = *costLimit - projCost;

        // if not feasible, return
        if( inpCostLimit < totalCost->Value() ) 
            return;
    }

    // EIO: Fixed a bug (req prop needed translation to inpSchema)
    // required physical properties in the input - just translate to inp schema
    PhyPropGroup_t *reqProp = planGroup->PropGroup();
    PhyPropGroup_t *inpReqProp = new PhyPropGroup_t();
    Refer(inpReqProp);
    
    if( reqProp->NumProps() != 0 ) {
        const List_t<PhyProp_t *>& reqPropList = reqProp->PropList();
        ListIter_t<PhyProp_t *> reqPropIter;
        reqPropIter.Attach(&reqPropList);

        while( !reqPropIter.IsEnd() ) {
            PhyProp_t *prop = reqPropIter.Next();
            assert(prop);
            PhyProp_t *inpProp = prop->Translate(outLP, inpLP);
            inpReqProp->AddProperty(inpProp);
        }
    }
 
    // now optimize the input
    PlanGroup_t *inpPlanGroup = NULL;
    if( costLimit ) {
        // pruning enabled
        inpPlanGroup = opt->FindBestPlan(inpEq, inpReqProp,
                                    totalCost, &inpCostLimit, 
                                    0, gpso, plmap, level, callCount);
    } else {
        inpPlanGroup = opt->FindBestPlan(inpEq, inpReqProp, totalCost, NULL,
                                         0, gpso, plmap, level, callCount);
        assert(inpPlanGroup);
    }

    DeRefer(inpReqProp);

    // if no feasible plan found, return
    if( !inpPlanGroup )
        return;

    // a valid feasible best plan found for the input
    Plan_t *inpBestPlan = inpPlanGroup->BestPlan();
    assert(inpBestPlan);
    assert(!inpBestPlan->IsFailed()); // valid plan

    // input plan cost
    Cost_t *inpCost = inpBestPlan->Cost();

    if( opt->IsRU() && inpPlanGroup->IsMarked() &&
            inpEq->UseCost() < inpBestPlan->CostVal() )
        inpCost = inpEq->UseCostADT(opt);

    assert(inpCost);

    // now instantiate the project operator and set its input as the
    // plan obtained as above to get the required plan

    ProjectScan_t *proj = new ProjectScan_t(this);
    proj->SetCallCount(callCount);
    proj->SetInput(0, inpPlanGroup);
    proj->SetCostVal(projCost);

    // calculate the cost
    Cost_t *cost = opt->CreateCost(proj, projCost);
    Refer(cost);
    cost->Add(inpCost);

    // add the plan to the plan group
    planGroup->AddAlgPlan(proj, cost);

    // current total plan cost
    CostVal_t curCost = totalCost->SumCost(cost);

    // update the best plan if found plan better
    int isBetterPlan = 1;

    // if pruning enabled, we know that we have arrived at a better
    // plan, so a check is not necessary
    if( !costLimit ) {
        // pruning disabled
        Cost_t *prevCost = bestPlan->Cost();
        if( prevCost ) {
            CostVal_t prevBestCost = totalCost->SumCost(prevCost);
            if( curCost >= prevBestCost )
                isBetterPlan = 0;
        }
    }
    
    if( isBetterPlan ) {
        bestPlan->SetRoot(proj);
        bestPlan->SetCost(cost);

        if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) ) {
            bestPlan->PrintPlan();
            cout << endl;
            cout << endl;
        }
    }

    if( costLimit ) {
        // update the costLimit
        *costLimit = curCost;
    }

    DeRefer(cost);
}

// generates aggregate scan operators
void Aggregate_t::ApplyAlgorithms(PlanGroup_t *planGroup, Volcano_t *opt,
            Cost_t *totalCost, Plan_t *bestPlan, CostVal_t *costLimit,
        Order_t *gpso, int *plmap, int level, Card_t callCount)
{
    assert(planGroup);
    assert(opt);
    assert(totalCost);
    assert(bestPlan);

    ApplyAggregateHash(planGroup, opt, totalCost, bestPlan, costLimit,
                       gpso, plmap, level, callCount);
 // ApplyAggregateScan(planGroup, opt, totalCost, bestPlan, costLimit);  // SUD
    ApplyAggregateScan(planGroup, opt, totalCost, bestPlan, costLimit,
                       gpso, plmap, level, callCount);  
}

// generates aggregate scan operator
// inputs are optimized by the optimizer provided
// update the bestPlan if a better plan is found
void Aggregate_t::ApplyAggregateScan(PlanGroup_t *planGroup, Volcano_t *opt,
            Cost_t *totalCost, Plan_t *bestPlan, CostVal_t *costLimit,
            Order_t *gpso, int *plmap, int level, Card_t callCount)
{
    assert(planGroup);
    assert(opt);
    assert(totalCost);
    assert(bestPlan);

    // properties required in the current plan
    PhyPropGroup_t *reqProp = planGroup->PropGroup();
    assert(reqProp);


    // *** EIO: Commenting the following for EIO
    //if( reqProp->NumProps() != 0 )
    //    return;

    SortOrder_t *reqOutOrder = NULL;
    if( reqProp->NumProps() != 0 ) {
        if (NumGroupByAttrs() < 1)
            return;  // Can't guarantee any order
        List_t<PhyProp_t *>& propList = reqProp->PropList();
        ListIter_t<PhyProp_t *> propIter;
        propIter.Attach(&propList);
        PhyProp_t *p = propIter.Next();
            
        if( p->Type() == INDEXORDER_T ) {
            // need to materialize the index first
            return;
        }
        reqOutOrder = (SortOrder_t *) p;
    }
 
    // logicalProp of the output
    LogProp_t *outLP = LogicalProp();
    Refer(outLP);

    // input equivalence class
    Equivalence_t *inpEq = Input(0);
    assert(inpEq);

    // the logical property of the input
    LogProp_t *inpLP = inpEq->LogicalProp();
    assert(inpLP);

    // Ravi: NQO: multiplying the op cost by callCount
    CostVal_t aggCost = callCount * AggregateScan_t::Cost(inpLP, outLP);
    DeRefer(outLP);

    CostVal_t inpCostLimit = -1;
    if( costLimit ) {
        // pruning enabled
        // calculate the costLimit
        inpCostLimit = *costLimit - aggCost; 

        // if not feasible, return
        if( inpCostLimit < totalCost->Value() ) 
            return;
    }

    // required physical property in the input
    PhyPropGroup_t *inpReqProp = NULL;

    int numGroupByAttrs = NumGroupByAttrs();
    List_t<Order_t *> *intOrds = NULL;
    bool scalarGroupBy = false;
    // Translate the reqOutOrder to input schema
    SortOrder_t *reqOutOrderTrans = NULL;
    // input schema property
    SchemaProp_t *inpSchemaProp = inpLP->SchemaProp();
    assert(inpSchemaProp);

    if (reqOutOrder) {
        reqOutOrderTrans = (SortOrder_t *) reqOutOrder->Translate(outLP, inpLP);
    }

    if( numGroupByAttrs > 0 ) {
        // sort order for the input
        int inpOrder[MAXGROUPBYATTRS];
        assert(numGroupByAttrs <= MAXGROUPBYATTRS);

        // input schema
        Schema_t *inpSchema = inpLP->Schema();
        assert(inpSchema);


        // cardinality of the sort order
        Card_t card = 1;
        for( int i = 0 ; i < numGroupByAttrs ; i++ ) {
            // bind the group-by expression with the input schema
            Expr_t *e = GroupByExpr(i);
            assert(e);

            // FIX: no complex expressions allowed
            assert(e->ExprType() == REL_ARG_REF_T);

            RelArgRef_t *r = (RelArgRef_t *) e;

            // attribute number in input schema bound to the $i$th group-by attr
            int bnd = r->Binding(inpSchema);    
            assert(bnd != -1);

            inpOrder[i] = bnd;

            const AttrProp_t *p = inpSchemaProp->AttributeProp(bnd);
            assert(p);

            card *= p->NumDistinct();
        }

        // sort order enforced on input if card > 1
        // if( card > 1 ) {
        // TODO: Ravi - changed temporarily
        if( card > 0 ) {
            // EIO: Commenting out the following
            /*
            SortOrder_t *inpSortOrder =
                                new SortOrder_t(inpOrder, numGroupByAttrs);
            Refer(inpSortOrder);
            inpReqProp->AddProperty(inpSortOrder);
            DeRefer(inpSortOrder);
            */

            // EIO: deduce the set of interesting orders
            Set_t<int> gbAttrSet(inpOrder, numGroupByAttrs);
            List_t<Order_t *> *inpFavOrds = inpEq->GetFavorableOrders();
            if (reqOutOrderTrans != NULL) {
                inpFavOrds->Insert((Order_t *&) reqOutOrderTrans);
            }

            if (Config_t::IsDebug(DEBUG_INFO)) {
                cout << "Generating interesting orders "
                     << "for group by node: " << ID() << endl;
                cout << "GroupByAttrSetSize = " << gbAttrSet.Size();
            }
            if (Config_t::ExhaustiveEIO) {
                if (Config_t::IsDebug(DEBUG_INFO)) {
                    cout << "Algorithm: Exhaustive EIO"<< endl;
                }
                intOrds = DeduceIntOrdsAllPermutations(&gbAttrSet);
            }
            else if (Config_t::PostgresHeuristicEIO) {
                if (Config_t::IsDebug(DEBUG_INFO)) {
                    cout << "Algorithm: Postgres heuristic EIO"<< endl;
                }
                intOrds = DeduceIntOrdsPGHeuristic(&gbAttrSet);
            }
            else if (Config_t::PlainEIO) {
                if (Config_t::IsDebug(DEBUG_INFO)) {
                    cout << "Algorithm: Plain EIO"<< endl;
                }
                intOrds = DeduceIntOrdsPlain(&gbAttrSet);
            }
            else { 
                if (Config_t::IsDebug(DEBUG_INFO)) {
                    cout << "Algorithm: Favorable Order EIO"<< endl;
                }
                intOrds = DeduceIntOrds(&gbAttrSet, inpFavOrds, inpSchemaProp);
            }

            if(Config_t::IsDebug(DEBUG_INFO)) {
                cout << endl << " Number of interesting orders for LogOp " 
                      << ID() << " : " << intOrds->Size() << endl;
            }
            if(Config_t::IsDebug(DEBUG_VERBOSE_INFO)) {
                // PrintOrds(intOrds, inpSchema);
            }
        }
    }
    else {
        // FIX: Take care of scalar group by as well.
        scalarGroupBy = true;
        cout << "Currently a Group By clause is must";
        assert(false);
    }

    // EIO: Optimize the input for each of the interesting orders
    ListIter_t<Order_t *> intOrdIter;
    intOrdIter.Attach(intOrds);
    while(!intOrdIter.IsEnd()) {
        SortOrder_t *intOrder = (SortOrder_t *) intOrdIter.Next();
        Refer(intOrder);
        CostVal_t sortCost = NULL;
        SortOrder_t *lcpOrd = NULL;

        if (reqOutOrder != NULL) {
            SortOrder_t *intOrderTrans = (SortOrder_t *) 
                                          intOrder->Translate(inpLP, outLP);
            lcpOrd = reqOutOrder->LongestCommonPrefix(intOrderTrans);
            if (lcpOrd == NULL)
                continue; // When there is a rqd o/p order, consider 
                          // only those interesting orders that have 
                          // some common prefix with the o/p order. 
                          // Others will be considered anyway when
                          // we come back here again with no o/p order.

            if (!reqOutOrderTrans->IsCovered(inpSchemaProp, intOrder)) {
                sortCost = Sort_t::Cost(reqOutOrder, outLP, lcpOrd) * callCount;
                if( costLimit ) {
                    // pruning enabled
                    // calculate the costLimit
                    inpCostLimit = *costLimit - sortCost;

                    // if not feasible, continue
                    if( inpCostLimit < totalCost->Value() ) 
                        continue;
                }
            }
        }
        
        if(inpReqProp) {
            DeRefer(inpReqProp);
        }
        inpReqProp = new PhyPropGroup_t;
        Refer(inpReqProp);
        inpReqProp->AddProperty(intOrder);

        // now optimize the input
        PlanGroup_t *inpPlanGroup = NULL;
        if( costLimit ) {
            // pruning enabled
            inpPlanGroup = opt->FindBestPlan(inpEq, inpReqProp,
                                    totalCost, &inpCostLimit,
                                    0, gpso, plmap, level, callCount);
        } else {
            inpPlanGroup = opt->FindBestPlan(inpEq, inpReqProp, totalCost, NULL,
                                         0, gpso, plmap, level, callCount);
            assert(inpPlanGroup);
        }

        DeRefer(inpReqProp);

        // if no feasible plan found, return
        if( !inpPlanGroup )
            continue;

        // a valid feasible best plan found for the input
        Plan_t *inpBestPlan = inpPlanGroup->BestPlan();
        assert(inpBestPlan);
        assert(!inpBestPlan->IsFailed()); // valid plan

        // input plan cost
        Cost_t *inpCost = inpBestPlan->Cost();

        if( opt->IsRU() && inpPlanGroup->IsMarked() &&
            inpEq->UseCost() < inpBestPlan->CostVal() ) {
            inpCost = inpEq->UseCostADT(opt);
        }

        assert(inpCost);

        // now instantiate the aggregate operator and set its input as the
        // plan obtained as above to get the required plan
        PlanGroup_t *aggPG  = planGroup;
        Plan_t *aggBestPlan = bestPlan;
        bool needPartialSort = false;
        if (reqOutOrder) {
            if (! (reqOutOrderTrans->IsCovered(inpSchemaProp, intOrder))) {
                needPartialSort = true;
                PhyPropGroup_t *ppg = new PhyPropGroup_t();
                ppg->AddProperty(intOrder->Translate(inpLP, outLP));
                aggPG = EqClass()->PlanGroup(ppg);
                assert(aggPG != planGroup);

                aggBestPlan = aggPG->BestPlan();
                if (!aggBestPlan) {
                    aggBestPlan = new Plan_t(aggPG);
                }
            }
        }

        AggregateScan_t *agg = new AggregateScan_t(this);
        agg->SetInput(0, inpPlanGroup);
        agg->SetCostVal(aggCost);

        // calculate the cost
        Cost_t *cost = opt->CreateCost(agg, aggCost);
        Refer(cost);
        cost->Add(inpCost);

        // add the plan to the plan group
        // planGroup->AddAlgPlan(agg, cost);
        aggPG->AddAlgPlan(agg, cost);

        // current total plan cost
        CostVal_t curCost = totalCost->SumCost(cost);

        int isBetterPlan = 1;
        // if pruning enabled, we know that we have arrived at a better
        // plan, so a check is not necessary
        if( !costLimit ) {
            // pruning disabled
            Cost_t *prevCost = aggBestPlan->Cost();
            if( prevCost ) {
                CostVal_t prevBestCost = totalCost->SumCost(prevCost);
                if( curCost >= prevBestCost )
                    isBetterPlan = 0;
            }
        }
    
        if( isBetterPlan ) {
            aggBestPlan->SetRoot(agg);
            aggBestPlan->SetCost(cost);
            if (needPartialSort) {
                aggPG->SetBestPlan(aggBestPlan);
                aggPG->SetBestAlgPlan(aggBestPlan);
            }

            if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) ) {
                aggBestPlan->PrintPlan();
                cout << endl;
                cout << endl;
            }
        }

        if (!needPartialSort) {
            if( costLimit ) {
                // update the costLimit
                *costLimit = curCost;
            }
            continue;
        }

        Sort_t *sortEnf = new Sort_t(reqOutOrder, lcpOrd);
        sortEnf->SetCallCount(callCount);
        sortEnf->SetInput(0, aggPG);
        sortEnf->SetCostVal(sortCost);
        Cost_t *enfCost = opt->CreateCost(sortEnf, sortCost);
        Refer(enfCost);
        planGroup->AddEnfPlan(sortEnf, enfCost);

        // FIX: cost is already being referenced by child plan!
        Cost_t *tmp = cost->Clone();
        DeRefer(cost);
        cost = tmp;
        Refer(cost);
 
        cost->Add(enfCost);
 
        // current total plan cost
        curCost = totalCost->SumCost(cost);

        // update the best plan if found plan better
        isBetterPlan = 1;

        // if pruning enabled, we know that we have arrived at a better
        // plan, so a check is not necessary
        if( !costLimit ) {
            // pruning disabled
            Cost_t *prevCost = bestPlan->Cost();
            if( prevCost ) {
                CostVal_t prevBestCost = totalCost->SumCost(prevCost);
                if( curCost >= prevBestCost )
                    isBetterPlan = 0;
            }
        }
    
        if( isBetterPlan ) {
            bestPlan->SetRoot(sortEnf);
            bestPlan->SetCost(cost);

            if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) ) {
                bestPlan->PrintPlan();
                cout << endl;
                cout << endl;
            }
        }

        if( costLimit ) {
            // update the costLimit
            *costLimit = curCost;
        }

        DeRefer(cost);
    }
}

// generates aggregate hash operator
// inputs are optimized by the optimizer provided
// update the bestPlan if a better plan is found
void Aggregate_t::ApplyAggregateHash(PlanGroup_t *planGroup, Volcano_t *opt,
            Cost_t *totalCost, Plan_t *bestPlan, CostVal_t *costLimit,
            Order_t *gpso, int *plmap, int level, Card_t callCount)
{
    assert(planGroup);
    assert(opt);
    assert(totalCost);
    assert(bestPlan);

    // properties required in the current plan
    PhyPropGroup_t *reqProp = planGroup->PropGroup();
    assert(reqProp);

    // no property assumed in the output
    if( reqProp->NumProps() != 0 )
        return;

    // logicalProp of the output
    LogProp_t *outLP = LogicalProp();
    Refer(outLP);

    // input equivalence class
    Equivalence_t *inpEq = Input(0);
    assert(inpEq);

    // the logical property of the input
    LogProp_t *inpLP = inpEq->LogicalProp();
    assert(inpLP);

    // Ravi: Costing change, multiplying by callCount
    CostVal_t aggCost = callCount * AggregateHash_t::Cost(inpLP, outLP);
    DeRefer(outLP);

    CostVal_t inpCostLimit = -1;
    if( costLimit ) {
        // pruning enabled
        // calculate the costLimit
        inpCostLimit = *costLimit - aggCost;

        // if not feasible, return
        if( inpCostLimit < totalCost->Value() ) 
            return;
    }
    if(Config_t::IsDebug(DEBUG_VERBOSE_INFO)) {
         cout << "aggregate hash feasible, cost limit is " << inpCostLimit << endl;  // SUD
         cout << "                         total cost is " << totalCost->Value() << endl;  // SUD
    }

    // required physical property in the input
    PhyPropGroup_t *inpReqProp = new PhyPropGroup_t;
    Refer(inpReqProp);

    // now optimize the input
    PlanGroup_t *inpPlanGroup = NULL;
    if( costLimit ) {
        // pruning enabled
        inpPlanGroup = opt->FindBestPlan(inpEq, inpReqProp,
                                    totalCost, &inpCostLimit,
                                    0, gpso, plmap, level, callCount);
    } else {
        inpPlanGroup = opt->FindBestPlan(inpEq, inpReqProp, totalCost, NULL,
                                         0, gpso, plmap, level, callCount);
        assert(inpPlanGroup);
    }

    DeRefer(inpReqProp);

    // if no feasible plan found, return
    if( !inpPlanGroup ) {
        if(Config_t::IsDebug(DEBUG_WARN)) {
            cout << "--- aggregate hash plan NOT found " << endl;  // SUD
        }
        return;
    }

    if(Config_t::IsDebug(DEBUG_WARN)) {
        cout << "aggregate hash plan found " << endl;  // SUD
    }

    // a valid feasible best plan found for the input
    Plan_t *inpBestPlan = inpPlanGroup->BestPlan();
    assert(inpBestPlan);
    assert(!inpBestPlan->IsFailed()); // valid plan

    // input plan cost
    Cost_t *inpCost = inpBestPlan->Cost();

    if( opt->IsRU() && inpPlanGroup->IsMarked() &&
            inpEq->UseCost() < inpBestPlan->CostVal() )
        inpCost = inpEq->UseCostADT(opt);

    assert(inpCost);

    // now instantiate the project operator and set its input as the
    // plan obtained as above to get the required plan

    AggregateHash_t *agg = new AggregateHash_t(this);
    agg->SetInput(0, inpPlanGroup);
    agg->SetCostVal(aggCost);

    // calculate the cost
    Cost_t *cost = opt->CreateCost(agg, aggCost);
    Refer(cost);
    cost->Add(inpCost);

    // add the plan to the plan group
    planGroup->AddAlgPlan(agg, cost);

    // current total plan cost
    CostVal_t curCost = totalCost->SumCost(cost);

    // update the best plan if found plan better
    int isBetterPlan = 1;

    // if pruning enabled, we know that we have arrived at a better
    // plan, so a check is not necessary
    if( !costLimit ) {
        // pruning disabled
        Cost_t *prevCost = bestPlan->Cost();
        if( prevCost ) {
            CostVal_t prevBestCost = totalCost->SumCost(prevCost);
            if( curCost >= prevBestCost )
                isBetterPlan = 0;
        }
    }
    
    if( isBetterPlan ) {
        bestPlan->SetRoot(agg);
        bestPlan->SetCost(cost);

        if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) ) {
            cout << "aggregate hash found a new best plan" << endl;
            bestPlan->PrintPlan();
            cout << endl;
            cout << endl;
        }
    }

    if( costLimit ) {
        // update the costLimit
        *costLimit = curCost;
    }

    DeRefer(cost);
}

void Select_t::ApplyAlgorithms(PlanGroup_t *planGroup, Volcano_t *opt,
            Cost_t *totalCost, Plan_t *bestPlan, CostVal_t *costLimit,
        Order_t *gpso, int *plmap, int level, Card_t callCount)
{
    assert(planGroup);
    assert(opt);
    assert(totalCost);
    assert(bestPlan);

    ApplySelectScan(planGroup, opt, totalCost, bestPlan, costLimit,
                    gpso, plmap, level, callCount);

    if( !Config_t::IsIndexing() )
        return;

    // properties required in the current plan
    PhyPropGroup_t *reqProp = planGroup->PropGroup();
    assert(reqProp);

    // Ravi: Doubt: Could not understand the following code.
    if( reqProp->NumProps() == 0 ) {
        // the result does not have any physical property
        IndexOrder_t *indexOrder = NULL;

        int isIndexOrderPossible = IndexProperty(&indexOrder);

        if( isIndexOrderPossible ) {
            // index-order is possible
            Refer(indexOrder);

            ApplyIndexedSelect(planGroup, indexOrder, opt,
                               totalCost, bestPlan, costLimit,
                               gpso, plmap, level, callCount);
            DeRefer(indexOrder);
        }
    }

    // Ravi: TODO: Here call ApplyRestartableSelectScan()
}

// generates select scan operator
// inputs are optimized by the optimizer provided
// update the bestPlan if a better plan is found
void Select_t::ApplySelectScan(PlanGroup_t *planGroup, Volcano_t *opt,
            Cost_t *totalCost, Plan_t *bestPlan, CostVal_t *costLimit,
            Order_t *gpso, int *plmap, int level, Card_t callCount)
{
    assert(planGroup);
    assert(opt);
    assert(totalCost);
    assert(bestPlan);

    // input equivalence class
    Equivalence_t *inpEq = Input(0);
    assert(inpEq);

    // the logical property of the input
    LogProp_t *inpLP = inpEq->LogicalProp();
    assert(inpLP);

    // the logical property of the output
    LogProp_t *outLP = LogicalProp();
    Refer(outLP);

    CostVal_t selCost = callCount * SelectScan_t::Cost(inpLP, outLP);

    if( costLimit ) {
        // pruning enabled

        // if not feasible, return
        if( *costLimit - selCost < totalCost->Value() ) {
            DeRefer(outLP);
            return;
        }
    }

    // Ravi:Com:Code for handle the IN predicate. Optimize the 
    // Ravi:Com:subquery expression and find the cost of the best plan.
    Predicate_t *cond = Predicate(); 

    InExpr_t *inCond = (InExpr_t *) cond->FindIn();     

    PlanGroup_t *subQueryPlanGroup = NULL ;
    if ( inCond != NULL ) {
        CostVal_t subQueryCost = 0;
        if( costLimit ) {
            CostVal_t subQueryLimit = *costLimit - selCost;
            subQueryCost = SubQueryCost(inCond, outLP, opt,
                    &subQueryLimit, &subQueryPlanGroup);
        } else {
            subQueryCost = SubQueryCost(inCond, outLP, opt, NULL,
                    &subQueryPlanGroup);
        }

        if( subQueryCost == -1 ) {
            DeRefer(outLP);
            return;
        }
  
        selCost += subQueryCost;
    }

    DeRefer(outLP);

    CostVal_t inpCostLimit = -1;
    if( costLimit ) {
        // pruning enabled
        // calculate the costLimit
        inpCostLimit = *costLimit - selCost;

    // if not feasible, return
    // Ravi:Doubt:Why the following? Doesn't totalCost represent 
    // Ravi:the cost expended so far?
        if( inpCostLimit < totalCost->Value() ) 
            return;
    }
    // Ravi: BEGIN state-retention changes 
    Order_t *matchingOrd = NULL;
    if (gpso) {
        if(callCount > 1 && gpso->OrderSize() > 0) {
            int *matchingOrdArray = new int[gpso->OrderSize()];
            for (int i = 0; i < gpso->OrderSize(); i++) {
                matchingOrdArray[i] = -1;
            }
            matchingOrd = new SortOrder_t(matchingOrdArray, gpso->OrderSize());
            delete[] matchingOrdArray;
//            cond->GetMatchingOrder(gpso, EqClass()->OuterVarLogProp()->Schema(),
//                                   matchingOrd, inpLP->Schema());
            cond->GetMatchingOrder(gpso, EqClass()->OuterVarLogProp()->Schema(),
                  matchingOrd, EqClass()->LogicalProp()->Schema());
            // Compact the matchingOrder
            matchingOrdArray = (int *) matchingOrd->Order();
            int newSize = 0;
            for (int i = 0; i < matchingOrd->OrderSize(); i++) {
                if (matchingOrdArray[i] != -1) {
                    matchingOrdArray[newSize++] = matchingOrdArray[i];
                }
            }
            matchingOrd = new SortOrder_t(matchingOrdArray, newSize);
            // End compaction
             
            matchingOrd = (Order_t *) matchingOrd->Translate(
                          EqClass()->LogicalProp(), inpLP);
            if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) ) {
                cout << "Input Schema:" << endl;
                EqClass()->OuterVarLogProp()->Schema()->PrintSQL();
                    cout << endl;
                cout << "Parameter Order:" ;
                gpso->Print(EqClass()->OuterVarLogProp()->Schema());
                    cout << endl;
                cout << "Output Schema:" << endl;
                inpLP->Schema()->PrintSQL();
                    cout << endl;
                cout << "Matching Order :" ;
                matchingOrd->Print(inpLP->Schema());
                    cout << endl;
           }
        }
    }
    // Ravi: END state-retention changes 


    // if there is an index physical property in reqProp, return
    // Ravi:Doubt:Why return if an index phy prop exists?

    PhyPropGroup_t *reqProp = planGroup->PropGroup();

    List_t<PhyProp_t *>& propList = reqProp->PropList();
    ListIter_t<PhyProp_t *> iter;

    iter.Attach(&propList);
    while( !iter.IsEnd() ) {
        PhyProp_t *prop = iter.Next();
        assert(prop);

        if( prop->Type() == INDEXORDER_T ) {
            // need to materialize the index first
            return;
        }
        // Order to match gpso should subsume the required 
        // physical props or the rqd prop should subsume the 
        // matching ord.  Otherwise return (an explicit enforcer 
        // needed above.
        if (matchingOrd) {
            if (!matchingOrd->Covers(outLP->SchemaProp(), prop)) {
                // if required prop subsumes matching ord use rqd prop 
                if (prop->Covers(outLP->SchemaProp(), matchingOrd)) {
                    matchingOrd = (Order_t *) prop;
                }
                else {
                    // matchingOrd and prop not compatible, enforcer needed
                    return;  
                }
            }
        }
    }

    PhyPropGroup_t *inpReqProp = reqProp;
    Refer(inpReqProp);
  
    Card_t inpCallCount = callCount; 
    // Ravi: BEGIN state-retention changes
    if (matchingOrd) {
        // matchingOrd can of of 0 length when the selection predicate
        // does not use any attributes from the gpso (but an inner block
        // uses those attributes) In such a case state retention cannot
        // be used for the current block
        if (matchingOrd->OrderSize() != 0) {
            // Convert matchingOrd to an index ord 
            IndexOrder_t *idxOrd = new IndexOrder_t((int *) matchingOrd->Order(),
                                                    matchingOrd->OrderSize());
            DeRefer(inpReqProp);
            inpReqProp = new PhyPropGroup_t();
            Refer(inpReqProp);
            // inpReqProp->AddProperty(matchingOrd);
            inpReqProp->AddProperty(idxOrd);
            inpCallCount = 1;
        }
    }
    // Ravi: END state-retention changes

    // now optimize the input
    PlanGroup_t *inpPlanGroup = NULL;
    if( costLimit ) {
        inpPlanGroup = opt->FindBestPlan(inpEq, inpReqProp,
                                        totalCost, &inpCostLimit,
                                        0, gpso, plmap, level, inpCallCount);
    } else {
        inpPlanGroup = opt->FindBestPlan(inpEq, inpReqProp, totalCost, NULL,
                                         0, gpso, plmap, level, inpCallCount);
        assert(inpPlanGroup);
    }

    DeRefer(inpReqProp);

    // if no feasible plan found, return
    if( !inpPlanGroup )
        return;

    // a valid feasible best plan found for the input
    Plan_t *inpBestPlan = inpPlanGroup->BestPlan();
    assert(inpBestPlan);
    assert(!inpBestPlan->IsFailed()); // valid plan

    // input plan cost
    Cost_t *inpCost = inpBestPlan->Cost();

    if( opt->IsRU() && inpPlanGroup->IsMarked() &&
            inpEq->UseCost() < inpBestPlan->CostVal() )
        inpCost = inpEq->UseCostADT(opt);

    assert(inpCost);

    // now instantiate the select operator and set its input as the
    // plan obtained as above to get the required plan

    SelectScan_t *selScan = new SelectScan_t(this);
    selScan->SetCallCount(callCount);
    if(matchingOrd) {
        if(matchingOrd->OrderSize() != 0)
            selScan->SetRetainState(1);
    }
    selScan->SetInput(0, inpPlanGroup);
    selScan->SetCostVal(selCost);
    if( subQueryPlanGroup )
        subQueryPlanGroup->AddAlgParent(selScan);

    // calculate the cost
    Cost_t *cost = opt->CreateCost(selScan, selCost);
    Refer(cost);
    cost->Add(inpCost);

    // add the plan to the plan group
    planGroup->AddAlgPlan(selScan, cost);

    // current total plan cost
    CostVal_t curCost = totalCost->SumCost(cost);

    // update the best plan if found plan better
    int isBetterPlan = 1;

    // if pruning enabled, we know that we have arrived at a better
    // plan, so a check is not necessary
    if( !costLimit ) {
        // pruning disabled
        Cost_t *prevCost = bestPlan->Cost();
        if( prevCost ) {
            CostVal_t prevBestCost = totalCost->SumCost(prevCost);
            if( curCost >= prevBestCost )
                isBetterPlan = 0;
        }
    }
    
    if( isBetterPlan ) {
        bestPlan->SetRoot(selScan);
        bestPlan->SetCost(cost);

        if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) ) { // SUD
            cout << "Found best plan for select ..." << endl; // SUD
            bestPlan->PrintPlan();
            cout << endl;
            cout << endl;
        }
    }

    if( costLimit ) {
        // update the costLimit
        *costLimit = curCost;
    }

    DeRefer(cost);
}

// generates merge join and nested loops join operators operator
// inputs are optimized by the optimizer provided
// update the bestPlan if a better plan is found
void Join_t::ApplyAlgorithms(PlanGroup_t *planGroup, Volcano_t *opt,
            Cost_t *totalCost, Plan_t *bestPlan, CostVal_t *costLimit,
        Order_t *gpso, int *plmap, int level, Card_t callCount)
{
    assert(planGroup);
    assert(opt);
    assert(totalCost);
    assert(bestPlan);

    // properties required in the current plan
    PhyPropGroup_t *reqProp = planGroup->PropGroup();
    assert(reqProp);

    // nested loops join output does not have any physical property
    // so, it is applicable only if no physical property is required
    // Ravi: Why not push the req sort order onto the outer i/p?
    if( reqProp->NumProps() == 0 && !Config_t::DisableNLJ) {
        ApplyNestedLoopsJoin(planGroup, opt, totalCost, bestPlan, costLimit,
                             gpso, plmap, level, callCount);
    }

    // Try merge-join
    // Ravi: Fix made. Merge join must be considered irrespective of
    // the required output sort order.
    SortOrder_t *reqOutOrder = NULL;
    List_t<PhyProp_t *>& propList = reqProp->PropList();
    ListIter_t<PhyProp_t *> iter;
    iter.Attach(&propList);
    while( !iter.IsEnd() ) { // Ravi: Assuming the propList has exactly 1 prop
        PhyProp_t *prop = iter.Next();
        assert(prop);

        if( prop->Type() == INDEXORDER_T ) {
            // need to materialize the index first
            return;
        }
        else {
            assert(prop->Type() == SORTORDER_T);
            reqOutOrder = (SortOrder_t *) prop;
        }
    }

    /*
    // find out if sort-order is possible
    SortOrder_t *sortOrder = NULL;
    SortOrder_t *leftInpSortOrder = NULL;
    SortOrder_t *rightInpSortOrder = NULL;

    // Ravi: For EIO pass the reqd output order (prop) 
    // and the favorable orders of LHS and RHS to the 
    // below method. Get back a "set" of interesting orders to try.
    int isSortOrderPossible = SortProperty(&sortOrder, &leftInpSortOrder, 
                                           &rightInpSortOrder);
     if( isSortOrderPossible ) {
        // sort-order is possible
        Refer(sortOrder);
        Refer(leftInpSortOrder);
        Refer(rightInpSortOrder);
    */
    if(! Config_t::DisableMergeJoin) {
        ApplyMergeJoin(planGroup, reqOutOrder, opt, totalCost, bestPlan, 
                       costLimit, gpso, plmap, level, callCount);
    }
    else {
        if(Config_t::IsDebug(DEBUG_WARN)) {
            cout << "Merge-Join disabled" << endl;
        }
    }

    /*
    DeRefer(sortOrder);
    DeRefer(leftInpSortOrder);
    DeRefer(rightInpSortOrder);
    */

    // Now try Index nested loops join
    if( !Config_t::IsIndexing() )
        return;

    IndexOrder_t *leftInpIndexOrder = NULL;
    IndexOrder_t *rightInpIndexOrder = NULL;

    int isIndexOrderPossible = IndexProperty(&leftInpIndexOrder,
                                                &rightInpIndexOrder);

    if( isIndexOrderPossible && !Config_t::DisableIndexNLJ) {
        // index-order is possible
        if( leftInpIndexOrder ) Refer(leftInpIndexOrder);
        if( rightInpIndexOrder ) Refer(rightInpIndexOrder);

        ApplyIndexedNestedLoopsJoin(planGroup,
                                    leftInpIndexOrder, rightInpIndexOrder,
                                    opt, totalCost, bestPlan, costLimit,
                                    gpso, plmap, level, callCount);
        if( leftInpIndexOrder ) DeRefer(leftInpIndexOrder);
        if( rightInpIndexOrder ) DeRefer(rightInpIndexOrder);
    }
}

// generates scan operator
// inputs are optimized by the optimizer provided
// update the bestPlan if a better plan is found
void Rel_t::ApplyAlgorithms(PlanGroup_t *planGroup, Volcano_t *opt,
            Cost_t *totalCost, Plan_t *bestPlan, CostVal_t *costLimit,
        Order_t *gpso, int *plmap, int level, Card_t callCount)
{
    assert(planGroup);
    assert(opt);
    assert(totalCost);
    assert(bestPlan);

    // is the reqd prop built-in
    int isBuiltIn = 0;

    // does the plan group have all properties as indexed?
    int isIndexed = 0;

    // get the logical property
    LogProp_t *lp = LogicalProp();
    Refer(lp);

    // there must be no required physical properties in the output
    // except those built-in
// Ravi: what is a built-in property?  What is an IndexOrder property?
// Ravi: shouldn't you worry about sort order of the relation which may
// Ravi: be guaranteed by the presence of a primary index?  Then we should
// Ravi: specify if each index is primary or not and ensure there is only
// Ravi: one primary index

// Ravi:Ans: Confirmed (by printing out the built-in index list being prepared 
// Ravi:Ans: at the time of reading the catalog) that built-in indexes are the
// Ravi:Ans: user created indexes (those present in the catalog).
// Ravi:Ans: An IndexOrder property tells on which attributes an index exists.

    PhyPropGroup_t *reqProp = planGroup->PropGroup();
    SortOrder_t *reqOutOrder = NULL; // The reqd o/p sort order.
    bool needPartialSort = false;
    SortOrder_t *builtInOrder = NULL;

    if( reqProp->NumProps() != 0 ) {
        const List_t<PhyProp_t *>& reqPropList = reqProp->PropList();

        // populate inpReqProp
        // add all properties in reqProp except those covered by the one
        // being enforced
        ListIter_t<PhyProp_t *> reqPropIter;
        reqPropIter.Attach(&reqPropList);

        // check if all req props are built-in
        // Ravi: Assuming there will be only one req prop.
        isBuiltIn = 1;
        while( !reqPropIter.IsEnd() ) {
            PhyProp_t *prop = reqPropIter.Next();
            assert(prop);

            if( prop->Type() == INDEXORDER_T ) {
                isIndexed = 1;  //Ravi:Doubt:This is not used at all! 
                                //Ravi:Doubt:I think if even one property is not
                                //Ravi:Doubt:of type INDEXORDER_T we should return.

                if( !lp->IsBuiltIn(prop) ) {
                    isBuiltIn = 0;  
                    //Ravi:Fix:Added the break. 
                    break;
                }
            }
            else {      // sort order prop
                reqOutOrder = (SortOrder_t *) prop;
                builtInOrder = ((BaseRelLogProp_t *)lp)->BuiltInProp(reqOutOrder);
                if (!builtInOrder) {
                    isBuiltIn = 0;  
                    break;
                }
                if (!builtInOrder->Covers(lp->SchemaProp(), prop)) {
                    needPartialSort = true;
                }
            }
        }
        if( !isBuiltIn ) {
            DeRefer(lp);
            return;
        }
    }

    if( isBuiltIn )
        planGroup->MarkBuiltIn();

    // Ravi: Costing change, multiplying by callCount
    CostVal_t relScanCost = callCount * RelScan_t::Cost(lp);

    if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) ) {
        if( isBuiltIn ) {
            cout << "PlanGroup " << planGroup->ID() << " BuiltIn";
            if (needPartialSort)
                cout << " (NeedPartialSort)";
            if( planGroup->IsIndexed() )
                cout << " Index";
            else cout << " Sort";
            Schema_t *schema = lp->Schema();
            planGroup->PropGroup()->Print(schema);
            if( lp->Name() )
                cout << " on " << lp->Name();
            cout << " cost = " << relScanCost << endl;
        }
    }

    // DeRefer(lp);
    // lp = NULL;

    PlanGroup_t *scanPG  = planGroup;
    Plan_t *scanBestPlan = bestPlan;
    CostVal_t sortCost = 0;
    if ( needPartialSort) {
        // plan group for the expression with partial sort
        PhyPropGroup_t *ppg = new PhyPropGroup_t();
        ppg->AddProperty(builtInOrder);
        scanPG = EqClass()->PlanGroup(ppg);
        scanPG->MarkBuiltIn();
        assert(scanPG != planGroup);

        scanBestPlan = scanPG->BestPlan();
        if (!scanBestPlan) {
            scanBestPlan = new Plan_t(scanPG);
        }

        sortCost = Sort_t::Cost(reqOutOrder, lp, builtInOrder);
    }

    if( costLimit ) {
        // if not feasible, return
        if( *costLimit - relScanCost - sortCost < totalCost->Value()) 
            return;
    }

    // now instantiate the scan operator
    RelScan_t *relScan = new RelScan_t(this);
    relScan->SetCallCount(callCount);
    relScan->SetCostVal(relScanCost);

    if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) ) {
        if( isBuiltIn )
            cout << " rel op = " << relScan->ID() << endl;
    }

    // calculate the cost
    Cost_t *cost = opt->CreateCost(relScan, relScanCost);
    Refer(cost);

    // add the plan to the plan group
    scanPG->AddAlgPlan(relScan, cost);

    // current total plan cost
    CostVal_t curCost = totalCost->SumCost(cost);

    // update the best plan if found plan better
    int isBetterPlan = 1;

    // if pruning enabled, we know that we have arrived at a better
    // plan, so a check is not necessary
    if( !costLimit ) {
        // pruning disabled
        Cost_t *prevCost = scanBestPlan->Cost();
        if( prevCost ) {
            CostVal_t prevBestCost = totalCost->SumCost(prevCost);
            if( curCost >= prevBestCost )
                isBetterPlan = 0;
        }
    }
    
    if( isBetterPlan ) {
        scanBestPlan->SetRoot(relScan);
        scanBestPlan->SetCost(cost);

        if (needPartialSort) {
            scanPG->SetBestAlgPlan(scanBestPlan);
            scanPG->SetBestPlan(scanBestPlan);
        }

        if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) ) {
            scanBestPlan->PrintPlan();
            cout << endl;
            cout << endl;
        }
    }

    /* Ravi: TODO: Check this
    if( costLimit ) {
        // update the costLimit
        *costLimit = curCost; 
    }

    DeRefer(cost);
    */

    if (!needPartialSort)  {
        if( costLimit ) {
            // update the costLimit
            *costLimit = curCost; // Ravi: TODO: Check this
        }
        return;
    }

    // Instantiate the partial enforcer
    Sort_t *sortEnf = new Sort_t(reqOutOrder, builtInOrder);
    sortEnf->SetCallCount(callCount);
    sortEnf->SetInput(0, scanPG);
    sortEnf->SetCostVal(sortCost);
    Cost_t *enfCost = opt->CreateCost(sortEnf, sortCost);
    Refer(enfCost);
    enfCost->Add(scanBestPlan->Cost());
    planGroup->AddEnfPlan(sortEnf, enfCost);

    // current total plan cost
    curCost = totalCost->SumCost(enfCost);

    // update the best plan if found plan better
    isBetterPlan = 1;

    // if pruning enabled, we know that we have arrived at a better
    // plan, so a check is not necessary
    if( !costLimit ) {
        // pruning disabled
        Cost_t *prevCost = bestPlan->Cost();
        if( prevCost ) {
            CostVal_t prevBestCost = totalCost->SumCost(prevCost);
            if( curCost >= prevBestCost )
                isBetterPlan = 0;
        }
    }
    
    if( isBetterPlan ) {
        bestPlan->SetRoot(sortEnf);
        bestPlan->SetCost(enfCost);

        if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) ) {
            bestPlan->PrintPlan();
            cout << endl;
            cout << endl;
        }
    }

    if( costLimit ) {
        // update the costLimit
        *costLimit = curCost; // Ravi: TODO: Check this
    }

    DeRefer(enfCost);
}

// generates indexed selects
void Select_t::ApplyIndexedSelect(PlanGroup_t *planGroup,
            IndexOrder_t *indexOrder, Volcano_t *opt,
            Cost_t *totalCost, Plan_t *bestPlan, CostVal_t *costLimit,
        Order_t* gpso, int *plmap, int level, Card_t callCount)
{
    assert(planGroup);
    assert(indexOrder);
    assert(opt);
    assert(totalCost);
    assert(bestPlan);

    // input equivalence class
    Equivalence_t *inpEq = Input(0);
    assert(inpEq);

    // properties required in the current plan
    PhyPropGroup_t *reqPropGroup = planGroup->PropGroup();
    assert(reqPropGroup);

    // required physical properties in the probes
    PhyPropGroup_t * indexReqProp = new PhyPropGroup_t;
    Refer(indexReqProp);

    indexReqProp->AddProperty(indexOrder);

    ApplyIndexedSelect(planGroup, indexReqProp, inpEq,
                                opt, totalCost, bestPlan, costLimit,
                       gpso, plmap, level, callCount);
    DeRefer(indexReqProp);
}

// generates indexed select operator
void Select_t::ApplyIndexedSelect(PlanGroup_t *planGroup,
                PhyPropGroup_t *indexInpReqProp,
                Equivalence_t *indexInpEq,
                Volcano_t *opt, Cost_t *totalCost,
                Plan_t *bestPlan, CostVal_t *costLimit,
        Order_t *gpso, int *plmap, int level, Card_t callCount)
{
    assert(planGroup);
    assert(indexInpReqProp);
    assert(indexInpEq);
    assert(opt);
    assert(totalCost);
    assert(bestPlan);

    // the logical property of the inputs
    LogProp_t *indexInpLP = indexInpEq->LogicalProp();
    assert(indexInpLP);

    // the logical property of the output
    LogProp_t *outLP = LogicalProp();
    Refer(outLP);

    // Ravi: Costing change, multiplying by callCount
    CostVal_t indexedSelectCost = callCount * IndexedSelect_t::Cost(indexInpLP, outLP);

    if( costLimit ) {
        // pruning enabled

        // if not feasible, return
        if( *costLimit - indexedSelectCost < totalCost->Value() ) {
            DeRefer(outLP);
            return;
        }
    }

    Predicate_t *cond = Predicate(); 

    InExpr_t *inCond = (InExpr_t *) cond->FindIn();     

    PlanGroup_t *subQueryPlanGroup = NULL ;
    if ( inCond != NULL ) {
        CostVal_t subQueryCost = 0;
        if( costLimit ) {
            CostVal_t subQueryLimit = *costLimit -
                                        indexedSelectCost;
            subQueryCost = SubQueryCost(inCond, outLP, opt,
                    &subQueryLimit, &subQueryPlanGroup);
        } else {
            subQueryCost = SubQueryCost(inCond, outLP, opt, NULL,
                    &subQueryPlanGroup);
        }

        if( subQueryCost == -1 ) {
            DeRefer(outLP);
            return;
        }
  
        indexedSelectCost += subQueryCost;
    }

    DeRefer(outLP);

    CostVal_t inpCostLimit = -1;
    if( costLimit ) {
        // pruning enabled
        // calculate the costLimit
        inpCostLimit = *costLimit - indexedSelectCost;

        // if not feasible, return
        if( inpCostLimit < totalCost->Value() ) {
            return;
        }
    }

    // now optimize the inputs
    CostVal_t indexInpCostLimit = inpCostLimit;

    PlanGroup_t *indexInpPlanGroup = NULL;
    if( costLimit ) {
        // pruning enabled
        indexInpPlanGroup = opt->FindBestPlan(indexInpEq,
                                            indexInpReqProp, 
                                            totalCost, &indexInpCostLimit, 1,
                                            gpso, plmap, level, callCount);
    } else {
        indexInpPlanGroup = opt->FindBestPlan(indexInpEq,
                                                indexInpReqProp,
                                                totalCost, NULL, 1,
                                                gpso, plmap, level, callCount);
        assert(indexInpPlanGroup);
    }

    // if no feasible plan found, return
    if( !indexInpPlanGroup )
        return;

    Plan_t *indexInpBestPlan = indexInpPlanGroup->BestPlan();
    assert(indexInpBestPlan);
    Cost_t *indexInpCost = indexInpBestPlan->Cost();

    // FIX: indexInpPlanGroup is just zero cost --- need to simplify this
    if( opt->IsRU() && indexInpPlanGroup->IsMarked() &&
            indexInpPlanGroup->UseCost() < indexInpBestPlan->CostVal() )
        indexInpCost = indexInpPlanGroup->UseCostADT(opt);

    assert(indexInpCost);

    // now instantiate the indexedSelect operator and set its
    // input as the plan obtained as above to get the required plan
    IndexedSelect_t *indexedSelect = new IndexedSelect_t(this);
    indexedSelect->SetInput(0, indexInpPlanGroup);
    indexedSelect->SetCostVal(indexedSelectCost);
    if( subQueryPlanGroup )
        subQueryPlanGroup->AddAlgParent(indexedSelect);

    // calculate the cost
    Cost_t *cost = opt->CreateCost(indexedSelect,
                                        indexedSelectCost);
    Refer(cost);

    if( !indexInpPlanGroup->IsBuiltIn() ) // PRASAN
        cost->Add(indexInpCost);

    // add the plan to the plan group
    planGroup->AddAlgPlan(indexedSelect, cost);

    // current total plan cost
    CostVal_t curCost = totalCost->SumCost(cost);

    // update the best plan if found plan better
    int isBetterPlan = 1;

    // if pruning enabled, we know that we have arrived at a better
    // plan, so a check is not necessary
    if( !costLimit ) {
        // pruning disabled
        Cost_t *prevCost = bestPlan->Cost();
        if( prevCost ) {
            CostVal_t prevBestCost = totalCost->SumCost(prevCost);
            if( curCost >= prevBestCost )
                isBetterPlan = 0;
        }
    }

    if( isBetterPlan ) {
        bestPlan->SetRoot(indexedSelect);
        bestPlan->SetCost(cost);

        if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) ) {
            bestPlan->PrintPlan();
            cout << endl;
            cout << endl;
        }
    }

    if( costLimit ) {
        // update the costLimit
        *costLimit = curCost;
    }

    DeRefer(cost);
}

// generates merge join operator
// inputs are optimized by the optimizer provided
// update the bestPlan if a better plan is found
// NOTE: totalCost not modified
// the required properties are not consulted for obtaining a good sort
// order for the merge join consistent with the predicate
void Join_t::ApplyMergeJoin(PlanGroup_t *planGroup, SortOrder_t *reqOutOrder, 
             Volcano_t *opt, Cost_t *totalCost, Plan_t *bestPlan, 
             CostVal_t *costLimit, Order_t *gpso, int *plmap, int level, 
             Card_t callCount)
{
    assert(planGroup);
    assert(opt);
    assert(totalCost);
    assert(bestPlan);

    // input equivalence classes
    Equivalence_t *leftInpEq = Input(0);
    assert(leftInpEq);
    Equivalence_t *rightInpEq = Input(1);
    assert(rightInpEq);

    // the logical property of the inputs
    LogProp_t *leftInpLP = leftInpEq->LogicalProp();
    assert(leftInpLP);
    LogProp_t *rightInpLP = rightInpEq->LogicalProp();
    assert(rightInpLP);

    // the logical property of the output
    LogProp_t *outLP = LogicalProp();
    Refer(outLP);

    // EIO: Deduce the sort orders to try on inputs
    Set_t<int> joinAttrs;
    Predicate()->JoinAttrs(joinAttrs, outLP->Schema(), leftInpLP->Schema(),
                           rightInpLP->Schema());
    if (joinAttrs.Size() == 0)
        return; // No join attributes (e.g., cartasian product)
    Set_t<int> *joinAttrsCn = outLP->SchemaProp()->AttrPropRep(joinAttrs);
    List_t<Order_t *> *leftInpFavOrds = leftInpEq->GetFavorableOrders();
    List_t<Order_t *> *rightInpFavOrds = rightInpEq->GetFavorableOrders();
    AppendList_t<Order_t *> favOrdsCn; // In canonical form
    ListIter_t<Order_t *> ordIter;
    ordIter.Attach(leftInpFavOrds);
    while(!ordIter.IsEnd()) {
        SortOrder_t *fOrd = (SortOrder_t *) ordIter.Next();
        SortOrder_t *fOrdTrans = (SortOrder_t *) fOrd->Translate(leftInpLP, 
                                                                 outLP);
        SortOrder_t *fOrdCn = fOrdTrans->CanonicalOrder(outLP);
        favOrdsCn.Insert((Order_t *&)fOrdCn);
    }

    ordIter.Reset();
    ordIter.Attach(rightInpFavOrds);
    while(!ordIter.IsEnd()) {
        SortOrder_t *fOrd = (SortOrder_t *) ordIter.Next();
        SortOrder_t *fOrdTrans = (SortOrder_t *) fOrd->Translate(rightInpLP, 
                                                                 outLP);
        SortOrder_t *fOrdCn = fOrdTrans->CanonicalOrder(outLP);
        favOrdsCn.Insert((Order_t *&)fOrdCn);
    }

    SortOrder_t *reqOutOrderCn = NULL;
    if (reqOutOrder != NULL) {
        reqOutOrderCn = reqOutOrder->CanonicalOrder(outLP);
        favOrdsCn.Insert((Order_t *&) reqOutOrderCn);
    }

    // Deduce the set of orders to try on inputs.
    List_t<Order_t *> *intOrds;
    if (Config_t::IsDebug(DEBUG_INFO)) {
        cout << "Generating interesting orders for join node: " << ID() << endl;
        cout << "JoinAttrSetSize = " << joinAttrsCn->Size();
    }
    if (Config_t::ExhaustiveEIO) {
        if (Config_t::IsDebug(DEBUG_INFO)) {
            cout << "Algorithm: Exhaustive EIO" << endl;
        }
        intOrds = DeduceIntOrdsAllPermutations(joinAttrsCn);
    }
    else if (Config_t::PostgresHeuristicEIO) {
        if (Config_t::IsDebug(DEBUG_INFO)) {
            cout << "Algorithm: Postgres heuristic EIO " << endl;
        }
        intOrds = DeduceIntOrdsPGHeuristic(joinAttrsCn);
    }
    else if (Config_t::PlainEIO) {
        if (Config_t::IsDebug(DEBUG_INFO)) {
            cout << "Algorithm: Plain EIO"<< endl;
        }
        intOrds = DeduceIntOrdsPlain(joinAttrsCn);
    }
    else {
        if( Config_t::IsDebug(DEBUG_INFO) ) {
            cout << "Algorithm: Favorable Order EIO" << endl;
        }
        intOrds = DeduceIntOrds(joinAttrsCn, &favOrdsCn, outLP->SchemaProp());
    }

    if(Config_t::IsDebug(DEBUG_INFO)) {
        cout << endl << " Number of interesting orders for LogOp " 
             << ID() << " : " << intOrds->Size() << endl;
    }
    if(Config_t::IsDebug(DEBUG_VERBOSE_INFO)) {
        PrintOrds(intOrds, outLP->Schema());
    }

    ordIter.Reset();
    ordIter.Attach(intOrds);
    while(!ordIter.IsEnd()) {
        SortOrder_t *intOrd = (SortOrder_t *) ordIter.Next();
        bool needPartialSort = false;
        CostVal_t sortCost = 0;
        SortOrder_t *lcp = NULL;
        if (reqOutOrder) {
            // When there is a rqd o/p order, consider only those interesting 
            // orders that have some common prefix with the o/p order. 
            // Others will be considered anyway when we come back here again 
            // with no o/p order.
            lcp = reqOutOrderCn->LongestCommonPrefix(intOrd);
            if (!lcp) 
                continue; 
            if (!reqOutOrderCn->IsCovered(outLP->SchemaProp(), intOrd)) {
                needPartialSort = true;
                sortCost = Sort_t::Cost(reqOutOrderCn, outLP, lcp) * callCount;
            }
        }

        // Deduce the reqd props for inputs
        SortOrder_t *leftInpSortOrder;
        SortOrder_t *rightInpSortOrder;
        leftInpSortOrder  = intOrd->TranslateFromCanonical(outLP, leftInpLP);
        rightInpSortOrder = intOrd->TranslateFromCanonical(outLP, rightInpLP);
        if(Config_t::IsDebug(DEBUG_VERBOSE_INFO)) {
            cout << endl << "Order reqd from EqNode: " << leftInpEq->ID();
            leftInpSortOrder->Print(leftInpLP->Schema());
            cout << endl << "Order reqd from EqNode: " << rightInpEq->ID();
            rightInpSortOrder->Print(rightInpLP->Schema());
        }
    
        // Ravi: Costing change, multiplying by callCount
        CostVal_t mergeJoinCost = MergeJoin_t::Cost(leftInpSortOrder, 
                                  rightInpSortOrder, leftInpLP, 
                                  rightInpLP, outLP) * callCount;

        if( costLimit ) {
            // pruning enabled
            // if not feasible, return
            if( *costLimit - sortCost - mergeJoinCost < totalCost->Value() ) {
                DeRefer(outLP);
                return;
            }
        }

        Predicate_t *cond = Predicate(); 
        InExpr_t *inCond = (InExpr_t *) cond->FindIn();     

        PlanGroup_t *subQueryPlanGroup = NULL ;
        if ( inCond != NULL ) {
            CostVal_t subQueryCost = 0;
            if( costLimit ) {
                CostVal_t subQueryLimit = *costLimit - sortCost - mergeJoinCost;
                subQueryCost = SubQueryCost(inCond, outLP, opt,
                               &subQueryLimit, &subQueryPlanGroup);
            } else {
                subQueryCost = SubQueryCost(inCond, outLP, opt, NULL,
                               &subQueryPlanGroup);
            }

            if( subQueryCost == -1 ) {
                DeRefer(outLP);
                return;
            }
  
            mergeJoinCost += subQueryCost;
        }

        CostVal_t inpCostLimit = -1;
        if( costLimit ) {
            // pruning enabled
            // calculate the costLimit
            inpCostLimit = *costLimit - sortCost - mergeJoinCost;

            // if not feasible, return
            if( inpCostLimit < totalCost->Value() ) {
                DeRefer(outLP);
                return;
            }
        }

        // required physical properties in the inputs
        PhyPropGroup_t *leftInpReqProp = new PhyPropGroup_t;
        Refer(leftInpReqProp);
        PhyPropGroup_t *rightInpReqProp = new PhyPropGroup_t;
        Refer(rightInpReqProp);

        // now add the sort orders
        leftInpReqProp->AddProperty(leftInpSortOrder);
        rightInpReqProp->AddProperty(rightInpSortOrder);

        // now optimize the inputs
        CostVal_t leftInpCostLimit = inpCostLimit;

        PlanGroup_t *leftInpPlanGroup = NULL;
        if( costLimit ) {
            // pruning enabled
            leftInpPlanGroup = opt->FindBestPlan(leftInpEq, leftInpReqProp, 
                                    totalCost, &leftInpCostLimit,
                                    0, gpso, plmap, level, callCount);
        } else {
            leftInpPlanGroup = opt->FindBestPlan(leftInpEq, leftInpReqProp,
                                    totalCost, NULL, 
                                    0, gpso, plmap, level, callCount);
            assert(leftInpPlanGroup);
        }

        // if no feasible plan found, return
        if( !leftInpPlanGroup ) {
            DeRefer(leftInpReqProp);
            DeRefer(rightInpReqProp);
            return;
        }

        Plan_t *leftInpBestPlan = leftInpPlanGroup->BestPlan();
        assert(leftInpBestPlan);
        Cost_t *leftInpCost = leftInpBestPlan->Cost();

        if( opt->IsRU() && leftInpPlanGroup->IsMarked() &&
            leftInpEq->UseCost() < leftInpBestPlan->CostVal() ) {
            leftInpCost = leftInpEq->UseCostADT(opt);
        }

        assert(leftInpCost);

        // update totalCost for the right input optimization
        totalCost->Add(leftInpCost);

        CostVal_t rightInpCostLimit = inpCostLimit;

        PlanGroup_t *rightInpPlanGroup = NULL;
        if( costLimit ) {
            rightInpPlanGroup = opt->FindBestPlan(rightInpEq, rightInpReqProp,
                                     totalCost, &rightInpCostLimit,
                                     0, gpso, plmap, level, callCount);
        } else {
            rightInpPlanGroup = opt->FindBestPlan(rightInpEq, rightInpReqProp,
                                     totalCost, NULL,
                                     0, gpso, plmap, level, callCount);
            assert(rightInpPlanGroup);
        }

        // restore totalCost
        totalCost->Subtract(leftInpCost);

        // if no feasible plan found, return
        if( !rightInpPlanGroup ) {
            DeRefer(leftInpReqProp);
            DeRefer(rightInpReqProp);
            return;
        }

        Plan_t *rightInpBestPlan = rightInpPlanGroup->BestPlan();
        assert(rightInpBestPlan);
        Cost_t *rightInpCost = rightInpBestPlan->Cost();

        if( opt->IsRU() && rightInpPlanGroup->IsMarked() &&
            rightInpEq->UseCost() < rightInpBestPlan->CostVal() ) {
            rightInpCost = rightInpEq->UseCostADT(opt);
        }
        assert(rightInpCost);

        DeRefer(leftInpReqProp);
        DeRefer(rightInpReqProp);

        // now instantiate the mergeJoin operator and set its input as the
        // plan obtained as above to get the required plan
        MergeJoin_t *mergeJoin = new MergeJoin_t(this);
        mergeJoin->SetInput(0, leftInpPlanGroup);
        mergeJoin->SetInput(1, rightInpPlanGroup);
        mergeJoin->SetCostVal(mergeJoinCost);
        if( subQueryPlanGroup )
            subQueryPlanGroup->AddAlgParent(mergeJoin);

        // calculate the cost
        Cost_t *cost = opt->CreateCost(mergeJoin, mergeJoinCost);
        Refer(cost);
        cost->Add(leftInpCost);
        cost->Add(rightInpCost);

        // Refs to keep track of the original pg and original best plan
        // Needed when a partial sort is required above.
        PlanGroup_t *origPG = planGroup;
        Plan_t *origBestPlan = bestPlan;
        if (needPartialSort) {
            PhyPropGroup_t *ppg = new PhyPropGroup_t();
            ppg->AddProperty(intOrd);
            planGroup = EqClass()->PlanGroup(ppg);
            assert(planGroup != origPG);
            bestPlan = planGroup->BestPlan();
            if (!bestPlan) {
                bestPlan = new Plan_t(planGroup);
            }
        }

        // add the plan to the plan group
        planGroup->AddAlgPlan(mergeJoin, cost);

        // current total plan cost
        CostVal_t curCost = totalCost->SumCost(cost);

        // update the best plan if found plan better
        int isBetterPlan = 1;

        // if pruning enabled, we know that we have arrived at a better
        // plan, so a check is not necessary
        if( !costLimit ) {
            // pruning disabled
            Cost_t *prevCost = bestPlan->Cost();
            if( prevCost ) {
                CostVal_t prevBestCost = totalCost->SumCost(prevCost);
                if( curCost >= prevBestCost )
                    isBetterPlan = 0;
            }
        }

        if( isBetterPlan ) {
            bestPlan->SetRoot(mergeJoin);
            bestPlan->SetCost(cost);
            if (needPartialSort) {
                planGroup->SetBestPlan(bestPlan);
                planGroup->SetBestAlgPlan(bestPlan);
            }

            if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) ) {
                bestPlan->PrintPlan();
                cout << endl;
                cout << endl;
            }
        }

        if (!needPartialSort) {
            if( costLimit ) {
            // update the costLimit
                *costLimit = curCost;
            }
            continue;
        }

        Sort_t *sortEnf = new Sort_t(reqOutOrderCn, lcp);
        sortEnf->SetCallCount(callCount);
        sortEnf->SetInput(0, planGroup);
        sortEnf->SetCostVal(sortCost);
        Cost_t *enfCost = opt->CreateCost(sortEnf, sortCost);
        Refer(enfCost);
        origPG->AddEnfPlan(sortEnf, enfCost);

        // FIX: cost is already being referenced by child plan!
        Cost_t *tmp = cost->Clone();
        DeRefer(cost);
        cost = tmp;
        Refer(cost);
        cost->Add(enfCost);
 
        // current total plan cost
        curCost = totalCost->SumCost(cost);

        // update the best plan if found plan better
        isBetterPlan = 1;

        // if pruning enabled, we know that we have arrived at a better
        // plan, so a check is not necessary
        if( !costLimit ) {
            // pruning disabled
            Cost_t *prevCost = origBestPlan->Cost();
            if( prevCost ) {
                CostVal_t prevBestCost = totalCost->SumCost(prevCost);
                if( curCost >= prevBestCost )
                    isBetterPlan = 0;
            }
        }
    
        if( isBetterPlan ) {
            origBestPlan->SetRoot(sortEnf);
            origBestPlan->SetCost(cost);

            if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) ) {
                origBestPlan->PrintPlan();
                cout << endl;
                cout << endl;
            }
        }

        if( costLimit ) {
            // update the costLimit
            *costLimit = curCost;
        }
        DeRefer(cost);
    }
}

// generates nested-loops join operator
// inputs are optimized by the optimizer provided
// update the bestPlan if a better plan is found
// NOTE: totalCost not modified
void Join_t::ApplyNestedLoopsJoin(PlanGroup_t *planGroup, Volcano_t *opt,
            Cost_t *totalCost, Plan_t *bestPlan, CostVal_t *costLimit,
            Order_t *gpso, int *plmap, int level, Card_t callCount)
{
    assert(planGroup);
    assert(opt);
    assert(totalCost);
    assert(bestPlan);

    // input equivalence classes
    Equivalence_t *leftInpEq = Input(0);
    assert(leftInpEq);
    Equivalence_t *rightInpEq = Input(1);
    assert(rightInpEq);

    // get the logical property of the inputs
    LogProp_t *leftInpLP = leftInpEq->LogicalProp();
    assert(leftInpLP);
    LogProp_t *rightInpLP = rightInpEq->LogicalProp();
    assert(rightInpLP);

    // the logical property of the output
    LogProp_t *outLP = LogicalProp();
    Refer(outLP);

    // Ravi: Costing change, multiplying by callCount
    CostVal_t nestedLoopsJoinCost =
                    callCount * NestedLoopsJoin_t::Cost(leftInpLP, rightInpLP, outLP);

    if( costLimit ) {
        // pruning enabled

        // if not feasible, return
        if( *costLimit - nestedLoopsJoinCost < totalCost->Value() ) {
            DeRefer(outLP);
            return;
        }
    }

    Predicate_t *cond = Predicate(); 

    InExpr_t *inCond = (InExpr_t *) cond->FindIn();     

    PlanGroup_t *subQueryPlanGroup = NULL ;
    if ( inCond != NULL ) {
        CostVal_t subQueryCost = 0;
        if( costLimit ) {
            CostVal_t subQueryLimit = *costLimit - nestedLoopsJoinCost;
            subQueryCost = SubQueryCost(inCond, outLP, opt,
                    &subQueryLimit, &subQueryPlanGroup);
        } else {
            subQueryCost = SubQueryCost(inCond, outLP, opt, NULL,
                                                &subQueryPlanGroup);
        }

        if( subQueryCost == -1 ) {
            DeRefer(outLP);
            return;
        }
  
        nestedLoopsJoinCost += subQueryCost;
    }

    DeRefer(outLP);

    CostVal_t inpCostLimit = -1;
    
    if( costLimit ) {
        // pruning enabled
        // calculate the costLimit
        inpCostLimit = *costLimit - nestedLoopsJoinCost;

        // if not feasible, return
        if( inpCostLimit < totalCost->Value() )
            return;
    }

    // properties required in the current plan
    PhyPropGroup_t *reqProp = planGroup->PropGroup();

    // nested loops join output does not have any physical property
    // so, it is required that no physical property is required
    if( reqProp->NumProps() != 0 )
        return;

    // modify the required physical properties -- no change
    PhyPropGroup_t *inpReqProp = reqProp;
    Refer(inpReqProp);

    // now optimize the inputs
    CostVal_t leftInpCostLimit = inpCostLimit;

    PlanGroup_t *leftInpPlanGroup = NULL;
    
    if( costLimit ) {
        // pruning enabled
        leftInpPlanGroup = opt->FindBestPlan(leftInpEq, inpReqProp,
                                                totalCost, &leftInpCostLimit,
                                             0, gpso, plmap, level, callCount);
    } else {
        leftInpPlanGroup = opt->FindBestPlan(leftInpEq, inpReqProp,
                                                totalCost, NULL,
                                             0, gpso, plmap, level, callCount);
        assert(leftInpPlanGroup);
    }

    // if no feasible plan found, return
    if( !leftInpPlanGroup ) {
        DeRefer(inpReqProp);
        return;
    }

    Plan_t *leftInpBestPlan = leftInpPlanGroup->BestPlan();
    assert(leftInpBestPlan);
    Cost_t *leftInpCost = leftInpBestPlan->Cost();

    if( opt->IsRU() && leftInpPlanGroup->IsMarked() &&
            leftInpPlanGroup->UseCost() < leftInpBestPlan->CostVal() )
        leftInpCost = leftInpPlanGroup->UseCostADT(opt);

    assert(leftInpCost);

    // update totalCost for the right input optimization
    totalCost->Add(leftInpCost);

    CostVal_t rightInpCostLimit = inpCostLimit;

    PlanGroup_t *rightInpPlanGroup = NULL;
    if( costLimit ) {
        // pruning enabled
        rightInpPlanGroup = opt->FindBestPlan(rightInpEq, inpReqProp,
                                                totalCost, &rightInpCostLimit,
                                              0, gpso, plmap, level, callCount);
    } else {
        rightInpPlanGroup = opt->FindBestPlan(rightInpEq, inpReqProp,
                                                        totalCost, NULL,
                                              0, gpso, plmap, level, callCount);
        assert(rightInpPlanGroup);
    }

    // restore totalCost
    totalCost->Subtract(leftInpCost);

    // if no feasible plan found, return
    if( !rightInpPlanGroup ) {
        DeRefer(inpReqProp);
        return;
    }

    Plan_t *rightInpBestPlan = rightInpPlanGroup->BestPlan();
    assert(rightInpBestPlan);
    Cost_t *rightInpCost = rightInpBestPlan->Cost();

    if( opt->IsRU() && rightInpPlanGroup->IsMarked() &&
            rightInpPlanGroup->UseCost() < rightInpBestPlan->CostVal() )
        rightInpCost = rightInpPlanGroup->UseCostADT(opt);

    assert(rightInpCost);

    DeRefer(inpReqProp);

    // now instantiate the nestedLoopsJoin operator and set its input as the
    // plan obtained as above to get the required plan
    NestedLoopsJoin_t *nestedLoopsJoin = new NestedLoopsJoin_t(this);
    nestedLoopsJoin->SetCallCount(callCount);
    nestedLoopsJoin->SetInput(0, leftInpPlanGroup);
    nestedLoopsJoin->SetInput(1, rightInpPlanGroup);
    nestedLoopsJoin->SetCostVal(nestedLoopsJoinCost);
    if( subQueryPlanGroup )
        subQueryPlanGroup->AddAlgParent(nestedLoopsJoin);

    // calculate the cost
    Cost_t *cost = opt->CreateCost(nestedLoopsJoin, nestedLoopsJoinCost);
    Refer(cost);
    cost->Add(leftInpCost);
    cost->Add(rightInpCost);

    // add the plan to the plan group
    planGroup->AddAlgPlan(nestedLoopsJoin, cost);

    // current total plan cost
    CostVal_t curCost = totalCost->SumCost(cost);

    // update the best plan if found plan better
    int isBetterPlan = 1;

    // if pruning enabled, we know that we have arrived at a better
    // plan, so a check is not necessary
    if( !costLimit ) {
        // pruning disabled
        Cost_t *prevCost = bestPlan->Cost();
        if( prevCost ) {
            CostVal_t prevBestCost = totalCost->SumCost(prevCost);
            if( curCost >= prevBestCost )
                isBetterPlan = 0;
        }
    }
    
    if( isBetterPlan ) {
        bestPlan->SetRoot(nestedLoopsJoin);
        bestPlan->SetCost(cost);

        if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) ) {
            bestPlan->PrintPlan();
            cout << endl;
            cout << endl;
        }
    }

    if( costLimit ) {
        // update the costLimit
        *costLimit = curCost;
    }

    DeRefer(cost);
}

// generates indexed nested loop joins
void Join_t::ApplyIndexedNestedLoopsJoin(PlanGroup_t *planGroup,
            IndexOrder_t *leftInpIndexOrder,
            IndexOrder_t *rightInpIndexOrder, Volcano_t *opt,
            Cost_t *totalCost, Plan_t *bestPlan, CostVal_t *costLimit,
            Order_t *gpso, int *plmap, int level, Card_t callCount)
{
    assert(planGroup);
    assert(leftInpIndexOrder || rightInpIndexOrder);
    assert(opt);
    assert(totalCost);
    assert(bestPlan);

    // input equivalence classes
    Equivalence_t *leftInpEq = Input(0);
    assert(leftInpEq);
    Equivalence_t *rightInpEq = Input(1);
    assert(rightInpEq);

    // the logical property of the inputs
    LogProp_t *leftInpLP = leftInpEq->LogicalProp();
    assert(leftInpLP);
    LogProp_t *rightInpLP = rightInpEq->LogicalProp();
    assert(rightInpLP);

    // the logical property of the output
    LogProp_t *outLP = LogicalProp();
    Refer(outLP);

    // properties required in the current plan
    PhyPropGroup_t *reqPropGroup = planGroup->PropGroup();
    assert(reqPropGroup);

    // required physical properties in the probes
    PhyPropGroup_t *leftProbeReqProp = NULL;
    PhyPropGroup_t *rightProbeReqProp = NULL;

    PhyPropGroup_t *leftIndexReqProp = NULL;
    PhyPropGroup_t *rightIndexReqProp = NULL;

    if( leftInpIndexOrder ) {
        // right input is probe

        // returns null if the prop cannot be pushed down into the right input
        rightProbeReqProp = reqPropGroup->Translate(outLP, rightInpLP);
        if( rightProbeReqProp ) {
            Refer(rightProbeReqProp);

            leftIndexReqProp = new PhyPropGroup_t;
            Refer(leftIndexReqProp);

            leftIndexReqProp->AddProperty(leftInpIndexOrder);

            ApplyIndexedNestedLoopsJoin(planGroup, leftInpIndexOrder,
                                        leftIndexReqProp, leftInpEq,
                                        rightProbeReqProp, rightInpEq,
                                        opt, totalCost, bestPlan, costLimit,
                                        gpso, plmap, level, callCount);
            DeRefer(leftIndexReqProp);
            DeRefer(rightProbeReqProp);
        }
    }

    if( rightInpIndexOrder ) {
        // left input is probe

        // returns null if the prop cannot be pushed down into the left input
        leftProbeReqProp = reqPropGroup->Translate(outLP, leftInpLP);
        if( leftProbeReqProp ) {
            Refer(leftProbeReqProp);

            rightIndexReqProp = new PhyPropGroup_t;
            Refer(rightIndexReqProp);

            rightIndexReqProp->AddProperty(rightInpIndexOrder);

            ApplyIndexedNestedLoopsJoin(planGroup, rightInpIndexOrder,
                                        rightIndexReqProp, rightInpEq,
                                        leftProbeReqProp, leftInpEq,
                                        opt, totalCost, bestPlan, costLimit,
                                        gpso, plmap, level, callCount);
            DeRefer(rightIndexReqProp);
            DeRefer(leftProbeReqProp);
        }
    }

    DeRefer(outLP);
}

// generates indexed nested loops join operators
void Join_t::ApplyIndexedNestedLoopsJoin(PlanGroup_t *planGroup,
                                        IndexOrder_t *indexOrder,
                                PhyPropGroup_t *indexInpReqProp,
                                        Equivalence_t *indexInpEq,
                                        PhyPropGroup_t *probeInpReqProp,
                                        Equivalence_t *probeInpEq,
                                        Volcano_t *opt, Cost_t *totalCost,
                                        Plan_t *bestPlan, CostVal_t *costLimit,
                    Order_t* gpso, int *plmap, int level, Card_t callCount)
{
    assert(planGroup);
    assert(indexOrder);
    assert(indexInpReqProp);
    assert(indexInpEq);
    assert(probeInpReqProp);
    assert(probeInpEq);
    assert(opt);
    assert(totalCost);
    assert(bestPlan);

    if(Config_t::IsDebug(DEBUG_VERBOSE_INFO)) {
        cout << "trying for a indexNL join best plan for LogExpr: " << 
                        ID() << endl;  // SUD
        cout << "totalCost: " << totalCost->Value() << "Cost limit " << 
                        *costLimit << endl;  // SUD
        cout << "Probe Inp props: " << probeInpReqProp->NumProps() << endl;  
        // EqClass()->PrintExpressionDAG(0);  // SUD
    }

    // the logical property of the inputs
    LogProp_t *indexInpLP = indexInpEq->LogicalProp();
    assert(indexInpLP);
    LogProp_t *probeInpLP = probeInpEq->LogicalProp();
    assert(probeInpLP);

    // the logical property of the output
    LogProp_t *outLP = LogicalProp();
    Refer(outLP);

    // Ravi: Costing change, multiplying by callCount
    CostVal_t indexedNestedLoopsJoinCost =
                callCount * IndexedNestedLoopsJoin_t::Cost(indexOrder,
                                        indexInpLP, probeInpLP, outLP);

    if(Config_t::IsDebug(DEBUG_VERBOSE_INFO)) {
        cout << "IndexNestedLoopsJoinCost = " << 
                indexedNestedLoopsJoinCost << endl; //SUD
    }

    if( costLimit ) {
        // pruning enabled

        // if not feasible, return
        if( *costLimit - indexedNestedLoopsJoinCost < totalCost->Value() ) {
            DeRefer(outLP);
            return;
        }
    }

    Predicate_t *cond = Predicate(); 

    InExpr_t *inCond = (InExpr_t *) cond->FindIn();     

    PlanGroup_t *subQueryPlanGroup = NULL;
    if ( inCond != NULL ) {
        CostVal_t subQueryCost = 0;
        if( costLimit ) {
            CostVal_t subQueryLimit = *costLimit -
                                        indexedNestedLoopsJoinCost;
            subQueryCost = SubQueryCost(inCond, outLP, opt,
                            &subQueryLimit, &subQueryPlanGroup);
        } else {
            subQueryCost = SubQueryCost(inCond, outLP, opt, NULL,
                                                    &subQueryPlanGroup);
        }

        if( subQueryCost == -1 ) {
            DeRefer(outLP);
            return;
        }
  
        indexedNestedLoopsJoinCost += subQueryCost;
    }

    DeRefer(outLP);

    CostVal_t inpCostLimit = -1;
    if( costLimit ) {
        // pruning enabled
        // calculate the costLimit
        inpCostLimit = *costLimit - indexedNestedLoopsJoinCost;

        // if not feasible, return
        if( inpCostLimit < totalCost->Value() ) {
            return;
        }
    }

    // now optimize the inputs
    CostVal_t indexInpCostLimit = inpCostLimit;

    PlanGroup_t *indexInpPlanGroup = NULL;
    if( costLimit ) {
        // pruning enabled
        indexInpPlanGroup = opt->FindBestPlan(indexInpEq,
                                            indexInpReqProp, 
                                            totalCost, &indexInpCostLimit, 1,
                                            gpso, plmap, level, callCount);
    } else {
        indexInpPlanGroup = opt->FindBestPlan(indexInpEq,
                                            indexInpReqProp,
                                            totalCost, NULL, 1,
                                            gpso, plmap, level, callCount);
        assert(indexInpPlanGroup);
    }

    // if no feasible plan found, return
    if( !indexInpPlanGroup )
        return;

    Plan_t *indexInpBestPlan = indexInpPlanGroup->BestPlan();
    assert(indexInpBestPlan);
    Cost_t *indexInpCost = indexInpBestPlan->Cost();

    // FIX: need to simplify this
    if( opt->IsRU() && indexInpPlanGroup->IsMarked() &&
            indexInpPlanGroup->UseCost() < indexInpBestPlan->CostVal() )
        indexInpCost = indexInpPlanGroup->UseCostADT(opt);

    assert(indexInpCost);


    // update totalCost for the probe input optimization
    if( !indexInpPlanGroup->IsBuiltIn() ) // PRASAN
        totalCost->Add(indexInpCost);


    CostVal_t probeInpCostLimit = inpCostLimit;

    if(Config_t::IsDebug(DEBUG_VERBOSE_INFO)) {
        cout << "Found plan for index input, totalcost = " << 
                totalCost->Value() << endl; // SUD
        cout << "Optimizing probe input: cost limit = " << 
                probeInpCostLimit << endl; // SUD
    }

    PlanGroup_t *probeInpPlanGroup = NULL;
    if( costLimit ) {
        probeInpPlanGroup = opt->FindBestPlan(probeInpEq, probeInpReqProp,
                                                totalCost, &probeInpCostLimit,
                                              0, gpso, plmap, level, callCount);
    } else {
        probeInpPlanGroup = opt->FindBestPlan(probeInpEq, probeInpReqProp,
                                                            totalCost, NULL,
                                              0, gpso, plmap, level, callCount);
        assert(probeInpPlanGroup);
    }

    // restore totalCost
    if( !indexInpPlanGroup->IsBuiltIn() ) // PRASAN
        totalCost->Subtract(indexInpCost);

    // if no feasible plan found, return
    if( !probeInpPlanGroup ) {
        return;
    }

    Plan_t *probeInpBestPlan = probeInpPlanGroup->BestPlan();
    assert(probeInpBestPlan);
    Cost_t *probeInpCost = probeInpBestPlan->Cost();

    if( opt->IsRU() && probeInpPlanGroup->IsMarked() &&
            probeInpEq->UseCost() < probeInpBestPlan->CostVal() )
        probeInpCost = probeInpEq->UseCostADT(opt);

    assert(probeInpCost);

    if(Config_t::IsDebug(DEBUG_VERBOSE_INFO)) {
        cout << "Found plan for probe input, cost = " << 
                probeInpCost->Value() << endl; // SUDS
    }

    // now instantiate the indexedNestedLoopsJoin operator and set its
    // input as the plan obtained as above to get the required plan
    IndexedNestedLoopsJoin_t *indexedNestedLoopsJoin =
                                    new IndexedNestedLoopsJoin_t(this);
    indexedNestedLoopsJoin->SetCallCount(callCount);
    indexedNestedLoopsJoin->SetInput(0, indexInpPlanGroup);
    indexedNestedLoopsJoin->SetInput(1, probeInpPlanGroup);
    indexedNestedLoopsJoin->SetCostVal(indexedNestedLoopsJoinCost);
    if( subQueryPlanGroup )
        subQueryPlanGroup->AddAlgParent(indexedNestedLoopsJoin);

    // calculate the cost
    Cost_t *cost = opt->CreateCost(indexedNestedLoopsJoin,
                                        indexedNestedLoopsJoinCost);
    Refer(cost);
    if( !indexInpPlanGroup->IsBuiltIn() ) // PRASAN
        cost->Add(indexInpCost);
    cost->Add(probeInpCost);

    // add the plan to the plan group
    planGroup->AddAlgPlan(indexedNestedLoopsJoin, cost);

    // current total plan cost
    CostVal_t curCost = totalCost->SumCost(cost);

    // update the best plan if found plan better
    int isBetterPlan = 1;

    // if pruning enabled, we know that we have arrived at a better
    // plan, so a check is not necessary
    if( !costLimit ) {
        // pruning disabled
        Cost_t *prevCost = bestPlan->Cost();
        if( prevCost ) {
            CostVal_t prevBestCost = totalCost->SumCost(prevCost);
            if( curCost >= prevBestCost )
                isBetterPlan = 0;
        }
    }

    if( isBetterPlan ) {
        bestPlan->SetRoot(indexedNestedLoopsJoin);
        bestPlan->SetCost(cost);

        if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) ) {  // SUD
            cout << "Found a new indexNL join best plan" << endl;
            bestPlan->PrintPlan();
            cout << endl;
            cout << endl;
        }
    }

    if( costLimit ) {
        // update the costLimit
        *costLimit = curCost;
    }

    DeRefer(cost);
}

// transformations on union
void Union_t::ApplyTransformations(Memo_t&)
{ /* not implemented */ }

// transformations on project -- disabled
void Project_t::ApplyTransformations(Memo_t&)
{
    // PushProj_t::Apply(this, memo);
}

// transformations on project
void Aggregate_t::ApplyTransformations(Memo_t&)
{ /* not implemented */ }

// transformations on select
void Select_t::ApplyTransformations(Memo_t& memo)
{
    Predicate_t *selPred = Predicate();
    assert(selPred);

    assert(!selPred->IsEquivalent(ConstPreds.True()));
    SelectPushPred_t::Apply(this, memo);
    //Ravi:Add:Tranform into ApplyOp if predicate contains a nested subquery
    if(!Config_t::DisableApply) {
        NestedPredToApplyOp_t::Apply(this, memo);
    }
    else {
        if (Config_t::IsDebug(DEBUG_WARN)) {
            cout << "NQO Disabled. Transformation NestedPredToApplyOp "
                 << "not applied" << endl;
        }
    }
}

// transformations on join
void Join_t::ApplyTransformations(Memo_t& memo)
{
    Predicate_t *joinPred = Predicate();
    assert(joinPred);

    if( joinPred->IsEquivalent(ConstPreds.True()) )
        return;

    JoinPushPred_t::Apply(this, memo);

    if( !IsTransDisabled() ) {
        JoinAssoc_t::Apply(this, memo);
        JoinExchange_t::Apply(this, memo);
    }
}

// transformations on relation reference
void Rel_t::ApplyTransformations(Memo_t&)
{ /* not applicable */ }

// index order for indexed join
int Join_t::IndexProperty(IndexOrder_t **leftInpIndexOrder,
                            IndexOrder_t **rightInpIndexOrder) const
{
    assert(leftInpIndexOrder);
    assert(rightInpIndexOrder);

    if( !Config_t::IsIndexing() )
        return 0;

    Predicate_t *p = Predicate();
    assert(p);

    Equivalence_t *leftEq = Input(0);
    assert(leftEq);
    LogProp_t *leftLP = leftEq->LogicalProp();
    assert(leftLP);

    Schema_t *leftSchema = leftLP->Schema();
    assert(leftSchema);

    Equivalence_t *rightEq = Input(1);
    assert(rightEq);
    LogProp_t *rightLP = rightEq->LogicalProp();
    assert(rightLP);

    Schema_t *rightSchema = rightLP->Schema();
    assert(rightSchema);

    LogProp_t *lp = LogicalProp();
    Refer(lp);

    Schema_t *schema = lp->Schema();
    Refer(schema);

    DeRefer(lp);

    // get the attributes bindings that are part of the relevant join predicate

    int leftBnd = -1;
    int rightBnd = -1;

    int isValid = p->IndexAttrBnd(leftSchema, rightSchema, &leftBnd, &rightBnd);
    if( isValid ) {
        assert(leftBnd != -1 || rightBnd != -1);

        if( leftBnd != -1 )
            *leftInpIndexOrder = new IndexOrder_t(&leftBnd, 1);

        if( rightBnd != -1 )
            *rightInpIndexOrder = new IndexOrder_t(&rightBnd, 1);
    }

    DeRefer(schema);

    return isValid;
}

// index order for indexed selection
int Select_t::IndexProperty(IndexOrder_t **indexOrder) const
{
    assert(indexOrder);

    Predicate_t *p = Predicate();
    assert(p);

    Equivalence_t *inpEq = Input(0);
    assert(inpEq);
    LogProp_t *inpLP = inpEq->LogicalProp();
    assert(inpLP);

    Schema_t *inpSchema = inpLP->Schema();
    assert(inpSchema);

    // get the attributes bindings that are part of the relevant join predicate

    int inpBnd = -1;

    int isValid = p->IndexAttrBnd(inpSchema, &inpBnd);
    if( isValid ) {
        assert(inpBnd != -1);

        *indexOrder = new IndexOrder_t(&inpBnd, 1);
    }

    return isValid;
}

// sort order for merge join
int Join_t::SortProperty(SortOrder_t **sortOrder,
                            SortOrder_t **leftInpSortOrder,
                            SortOrder_t **rightInpSortOrder) const
{
    assert(sortOrder);
    assert(leftInpSortOrder);
    assert(rightInpSortOrder);

    Predicate_t *p = Predicate();
    assert(p);

    Equivalence_t *leftEq = Input(0);
    assert(leftEq);
    LogProp_t *leftLP = leftEq->LogicalProp();
    assert(leftLP);

    Schema_t *leftSchema = leftLP->Schema();
    assert(leftSchema);

    Equivalence_t *rightEq = Input(1);
    assert(rightEq);
    LogProp_t *rightLP = rightEq->LogicalProp();
    assert(rightLP);

    Schema_t *rightSchema = rightLP->Schema();
    assert(rightSchema);

    LogProp_t *lp = LogicalProp();
    Refer(lp);

    Schema_t *schema = lp->Schema();
    Refer(schema);

    DeRefer(lp);

    // get the attributes bindings that are part of the relevant join predicate

    int bnd = -1;
    int leftBnd = -1;
    int rightBnd = -1;

    int isValid = p->SortAttrBnd(schema, leftSchema, rightSchema,
                                                &bnd, &leftBnd, &rightBnd);
    if( isValid ) {
        assert(leftBnd != -1);
        // Ravi: SortOrder_t supports multiple attributes. However here
        // Ravi: assumption is being made that sorting on only one attribute
        // Ravi: will be required.
        *leftInpSortOrder = new SortOrder_t(&leftBnd, 1);

        assert(rightBnd != -1);
        *rightInpSortOrder = new SortOrder_t(&rightBnd, 1);

        assert(bnd != -1);
        *sortOrder = new SortOrder_t(&bnd, 1);
    }

    DeRefer(schema);

    return isValid;
}

// create the list of degree of sharing given the lists of the children
// equivalence nodes
// the children lists are topologically sorted wrt the eq nodes
AppendList_t<DegShar_t *> *LogicalOp_t::CreateDegreeList(void)
{

    // new list
    AppendList_t<DegShar_t *> *list = new AppendList_t<DegShar_t *>;

    if( numInputs == 0 )
        return list;

    // input lists
    List_t<DegShar_t *> **inpList =
                    new List_t<DegShar_t *> *[numInputs];
    // is this the last reference for the input lists
    int *isLast = new int[numInputs];

    // iterators on the input lists
    ListIter_t<DegShar_t *> *inpIter =
                    new ListIter_t<DegShar_t *>[numInputs];

    int i = -1;

    // attach the iterators
    for( i = 0 ; i < numInputs ; i++ ) {
        Equivalence_t *inp = Input(i);
        assert(inp);

        inpList[i] = inp->DegreeList(&isLast[i]);
        assert(inpList[i]);

        inpIter[i].Attach(inpList[i]);
    }

    DegShar_t **curEntry  = new DegShar_t *[numInputs];
    for( i = 0 ; i < numInputs ; i++ ) {
        if( !inpIter[i].IsEnd() ) {
            curEntry[i] = inpIter[i].Next();
            assert(curEntry[i]);
        } else {
            curEntry[i] = NULL;
        }
    }

    for( ; ; ) {
        int minTopNo = -1;

        // get the min topNo
        for( i = 0 ; i < numInputs ; i++ ) {
            if( !curEntry[i] )
                continue;

            const Equivalence_t *eq = curEntry[i]->EqNode();
            assert(eq);
            int topNo = eq->TopNo();

            if( minTopNo == -1 || topNo < minTopNo )
                minTopNo = topNo;
        }

        // if all input lists finished, break
        if( minTopNo == -1 )
            break;

        // sum the degrees of the children with min topNo
        int sumDegree = 0;
        DegShar_t *degShar = NULL;

        for( i = 0 ; i < numInputs ; i++ ) {
            if( !curEntry[i] )
                continue;

            const Equivalence_t *eq = curEntry[i]->EqNode();
            assert(eq);
            int topNo = eq->TopNo();

            if( topNo != minTopNo )
                continue;

            int degree = curEntry[i]->Degree();
            sumDegree += degree;
            degShar = curEntry[i];

            if( !inpIter[i].IsEnd() )
                curEntry[i] = inpIter[i].Next();
            else curEntry[i] = NULL;
        }
        assert(sumDegree > 0);

        DegShar_t *sumDegShar = NULL;
        if( degShar->Degree() == sumDegree ) {
            // the equivalence node was present only in one child list
            sumDegShar = degShar;
        } else {
            Equivalence_t *sumEq = degShar->EqNode();
            assert(sumEq);

            sumDegShar = new DegShar_t(sumEq, sumDegree);
        }

        Refer(sumDegShar);

        list->Insert(sumDegShar);
    }

    // delete the input lists if required
    for( i = 0 ; i < numInputs ; i++ ) {
        if( isLast[i] )
            DeleteDegreeList(inpList[i]);
    }

    delete[] curEntry;
    delete[] inpList;
    delete[] isLast;
    delete[] inpIter;

    return list;
}

// lower bound on the algorithm cost for the logical operator
CostVal_t Union_t::CostLB(void) const
{
    // input equivalence classes
    Equivalence_t *leftInpEq = Input(0);
    assert(leftInpEq);
    Equivalence_t *rightInpEq = Input(1);
    assert(rightInpEq);

    // get the logical property of the inputs
    LogProp_t *leftInpLP = leftInpEq->LogicalProp();
    assert(leftInpLP);
    LogProp_t *rightInpLP = rightInpEq->LogicalProp();
    assert(rightInpLP);

    // the logical property of the output
    LogProp_t *outLP = LogicalProp();
    Refer(outLP);

    CostVal_t mergeCost = Merge_t::Cost(leftInpLP, rightInpLP, outLP);
    DeRefer(outLP);

    return mergeCost;
}

// lower bound on the algorithm cost for the logical operator
CostVal_t Aggregate_t::CostLB(void) const
{
    // input equivalence class
    Equivalence_t *inpEq = Input(0);
    assert(inpEq);

    // get the logical property of the input
    LogProp_t *inpLP = inpEq->LogicalProp();
    assert(inpLP);

    // the logical property of the output
    LogProp_t *outLP = LogicalProp();
    Refer(outLP);

    CostVal_t aggCost = AggregateScan_t::Cost(inpLP, outLP);
    DeRefer(outLP);

    return aggCost;
}

// lower bound on the algorithm cost for the logical operator
CostVal_t Project_t::CostLB(void) const
{
    // input equivalence class
    Equivalence_t *inpEq = Input(0);
    assert(inpEq);

    // get the logical property of the input
    LogProp_t *inpLP = inpEq->LogicalProp();
    assert(inpLP);

    // the logical property of the output
    LogProp_t *outLP = LogicalProp();
    Refer(outLP);

    CostVal_t projCost = ProjectScan_t::Cost(inpLP, outLP);
    DeRefer(outLP);

    return projCost;
}

// lower bound on the algorithm cost for the logical operator
CostVal_t Select_t::CostLB(void) const
{
    // input equivalence class
    Equivalence_t *inpEq = Input(0);
    assert(inpEq);

    // get the logical property of the input
    LogProp_t *inpLP = inpEq->LogicalProp();
    assert(inpLP);

    // the logical property of the output
    LogProp_t *outLP = LogicalProp();
    Refer(outLP);

    CostVal_t selCost = SelectScan_t::Cost(inpLP, outLP);
    DeRefer(outLP);

    return selCost;
}

// lower bound on the algorithm cost for the logical operator
CostVal_t Join_t::CostLB(void) const
{
    /* --- TODO --- 
    // input equivalence classes
    Equivalence_t *leftInpEq = Input(0);
    assert(leftInpEq);
    Equivalence_t *rightInpEq = Input(1);
    assert(rightInpEq);

    // get the logical property of the inputs
    LogProp_t *leftInpLP = leftInpEq->LogicalProp();
    assert(leftInpLP);
    LogProp_t *rightInpLP = rightInpEq->LogicalProp();
    assert(rightInpLP);

    // the logical property of the output
    LogProp_t *outLP = LogicalProp();
    Refer(outLP);

    CostVal_t nestedLoopsJoinCost =
                    NestedLoopsJoin_t::Cost(leftInpLP, rightInpLP, outLP);
    CostVal_t mergeJoinCost = MergeJoin_t::Cost(leftInpLP, rightInpLP, outLP);
    CostVal_t nonIndexMinCost = nestedLoopsJoinCost < mergeJoinCost ?
                                    nestedLoopsJoinCost : mergeJoinCost;

    CostVal_t leftIndexCost =
                IndexedNestedLoopsJoin_t::Cost(leftInpLP, rightInpLP, outLP);
    CostVal_t rightIndexCost =
                IndexedNestedLoopsJoin_t::Cost(rightInpLP, leftInpLP, outLP);
    CostVal_t indexMinCost = leftIndexCost < rightIndexCost ?
                                    leftIndexCost : rightIndexCost;

    DeRefer(outLP);

    // return the minimum
    return nonIndexMinCost < indexMinCost ?  nonIndexMinCost : indexMinCost;
    --- TODO --- */
    return 0;
}

// lower bound on the algorithm cost for the logical operator
CostVal_t Rel_t::CostLB(void) const
{
    // get the logical property
    LogProp_t *lp = LogicalProp();
    Refer(lp);

    CostVal_t relScanCost = RelScan_t::Cost(lp);
    DeRefer(lp);

    return relScanCost;
}


void Rel_t::GetFavorableOrders(List_t<Order_t *> *favOrds)
{
    // TODO: Currently returning all index orders as favorable orders.
    // Should consider only primary index and indexes that cover the query.
    BaseRelLogProp_t *baseRelLogProp = (BaseRelLogProp_t *) LogicalProp();

    const List_t<IndexOrder_t *> *indices = baseRelLogProp->BuiltInIndexList();
    ListIter_t<IndexOrder_t *> iter;
    iter.Attach(indices);

    while(!iter.IsEnd())
    {
        IndexOrder_t *indexOrder = iter.Next();
        SortOrder_t *sortOrder = new SortOrder_t(indexOrder->Order(), 
                                                 indexOrder->OrderSize()); 
        // No need to check for duplicate fav ords here since
        // a Rel_t is a single log op node under its parent eq node.
        favOrds->Insert((Order_t *&)sortOrder);
    }
}

// is the logical operator admissible given the cost upper limit?
int LogicalOp_t::IsAdmissible(CostVal_t costUB) const
{
    // check if pruning disabled
    if( !Config_t::IsPruning() )
        return 1;

    // is the upper bound available?
    if( costUB < 0 )
        return 1;

    // operator cost lower bound
    CostVal_t opCostLB = CostLB();

    // total input cost lower bound -- sum of cost lower bounds of each input
    CostVal_t totInpCostLB = 0;
    int n = NumInputs();
    Equivalence_t *inpList[2];
    assert(n <= 2);

    for( int i = 0 ; i < n ; i++ ) {
        Equivalence_t *inp = Input(i);
        assert(inp);

        inpList[i] = inp;

        // check for repeated inputs
        int isRepeatedInp = 0;
        for( int j = 0 ; j < i ; j++ ) {
            Equivalence_t *prevInp = inpList[j];
            assert(prevInp);

            if( prevInp->ID() == inp->ID() ) {
                isRepeatedInp = 1;
                break;
            }
        }

        if( isRepeatedInp )
            continue;

        CostVal_t inpCostLB = inp->CostLB();
        assert(inpCostLB >= 0);

        totInpCostLB += inpCostLB;
    }

    return opCostLB + totInpCostLB <= costUB;
}

// apply all transformations possible on this node
void LogicalOp_t::Expand(Memo_t& memo)
{
    if( isExpanded  )
        return;

    isExpanded = 1;

    if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) ) {
        assert(eqClass);
        cout << endl;
        cout << "Expanding logexpr " << ID() <<
                        " eqclass = " << eqClass->ID() << endl;
        cout << endl;
    }

    // applies transformations and adds resulting logical expressions
    // to the exprList
    ApplyTransformations(memo);

    if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) ) {
        assert(eqClass);
        cout << endl;
        eqClass->PrintExpressionDAG(0);
        cout << "Finished Expanding logexpr " << ID() <<
                        " eqclass = " << eqClass->ID() << endl;
        cout << endl;
    }
}

// does $this$ subsume the given aggregation wrt group-by list?
// just check whether group-by attributes are a proper subset of the given
// aggregation's group-by attributes
int Aggregate_t::IsGroupBySubsumedBy(Aggregate_t *p) const
{
    assert(p);

    Equivalence_t *inpEq = Input(0);
    assert(inpEq);

    Equivalence_t *pInpEq = p->Input(0);
    assert(pInpEq);

    // for now ...
    assert(inpEq->ID() == pInpEq->ID());

    if( inpEq->ID() != pInpEq->ID() )
        return 0;

    int numGroupByAttrs = NumGroupByAttrs();
    int pNumGroupByAttrs = p->NumGroupByAttrs();

    if( numGroupByAttrs >= pNumGroupByAttrs )
        return 0;

    int i = 0, j = 0;
    for( i = 0 ; i < numGroupByAttrs; i++ ) {
        const Expr_t *groupByExpr = GroupByExpr(i);
        assert(groupByExpr);

        for( j = 0 ; j < pNumGroupByAttrs ; j++ ) {
            const Expr_t *pGroupByExpr = p->GroupByExpr(j);
            if( groupByExpr->IsEquivalent(pGroupByExpr) )
                break;
        }
        if( j == pNumGroupByAttrs ) {
            return 0;
        }
    }

    return 1;
}

// does $this$ subsume the given aggregation wrt aggregation list?
// just check whether aggregation attributes are a proper subset of the given
// aggregation's aggregation attributes
int Aggregate_t::IsAggSubsumedBy(Aggregate_t *p) const
{
    assert(p);

    Equivalence_t *inpEq = Input(0);
    assert(inpEq);

    Equivalence_t *pInpEq = p->Input(0);
    assert(pInpEq);

    // for now ...
    assert(inpEq->ID() == pInpEq->ID());

    if( inpEq->ID() != pInpEq->ID() )
        return 0;

    int numAggAttrs = NumAggAttrs();
    int pNumAggAttrs = p->NumAggAttrs();

    if( numAggAttrs >= pNumAggAttrs )
        return 0;

    int i = 0, j = 0;
    for( i = 0 ; i < numAggAttrs; i++ ) {
        const Expr_t *aggExpr = AggExpr(i);
        assert(aggExpr);

        for( j = 0 ; j < pNumAggAttrs ; j++ ) {
            const Expr_t *pAggExpr = p->AggExpr(j);
            if( aggExpr->IsEquivalent(pAggExpr) )
                break;
        }
        if( j == pNumAggAttrs ) {
            return 0;
        }
    }

    return 1;
}

// for debugging
void LogicalOp_t::PrintExpressionDAG(int indentLevel) const
{
    for(int j = 0; j < indentLevel; j++)
            cout << "| ";
 
    cout << "[LOGEXP" << ID() << "]";
    PrintName();
    cout << endl;
 
    for( int i = 0 ; i < numInputs ; i++ ) {
        Equivalence_t *inp = inputs[i];
        assert(inp);
        inp->PrintExpressionDAG(indentLevel+1);
    }
}

// Prints the expression DAG in XML format.
void LogicalOp_t::PrintExpressionDAGXML(int indentLevel) const
{
    for(int j = 0; j < indentLevel; j++)
            cout << " ";
 
    cout << "<LOGEXP id=\"" << ID() << "\" op=\"";
    //cout << "<![CDATA[";
    PrintName();
    //cout << endl;
    //cout << "]]>" << endl;
    cout << "\">" << endl;
 
    for( int i = 0 ; i < numInputs ; i++ ) {
        Equivalence_t *inp = inputs[i];
        assert(inp);
        inp->PrintExpressionDAGXML(indentLevel+1);
    }
    cout << "</LOGEXP>" << endl;
}


template <>  // Needed for G++ 4.1
int AppendDelList_t<LogicalOp_t *>::IsEqual(LogicalOp_t *const &x,
                                            LogicalOp_t *const &y) const
{
    if( x && y ) 
        return x->ID() == y->ID();

    return !x && !y;
}

template <>  // Needed for G++ 4.1
int AppendDelList_t<LogicalOp_t *>::ID(LogicalOp_t *const &x) const
{
    assert(x);
    return x->ID();
}

template <>  // Needed for G++ 4.1
int PrependDelList_t<LogicalOp_t *>::IsEqual(LogicalOp_t *const &x,
                                            LogicalOp_t *const &y) const
{
    if( x && y ) 
        return x->ID() == y->ID();

    return !x && !y;
}

template <>  // Needed for G++ 4.1
int PrependDelList_t<LogicalOp_t *>::ID(LogicalOp_t *const &x) const
{
    assert(x);
    return x->ID();
}

// for debugging
void Select_t::PrintExpressionDAG(int indentLevel) const
{
    for(int j = 0; j < indentLevel; j++)
        cout << "| ";
 
    cout << "[LOGEXP" << ID() << "]";

    cout << "SELECT< ";
    Predicate_t *p = Predicate();
    assert(p);
    p->PrintExpr(1);
    cout << " >";

    cout << endl;
 
    for( int i = 0 ; i < numInputs ; i++ ) {
        Equivalence_t *inp = inputs[i];
        assert(inp);
        inp->PrintExpressionDAG(indentLevel+1);
    }
}


/**
 * Ravi:Added method comment.
 * Ravi:Finds the cost of a nested subquery that is part of a InExpr.
 * Ravi:
 * Ravi:Logic: 
 * Ravi: - Let 'dist' be the number of distinct values in the LHS of the InExr.
 * Ravi:   Currently LHS is a simple RelArgRef and the number of distinct values
 * Ravi:   is directly derived from the catalog. 
 * Ravi:   See InExpr_t::CalculateNumDistinct()
 * Ravi:
 * Ravi: - If cost limit is not set optimize the RHS with no cost limit.
 * Ravi: - If cost limit is set to 'c', optimize the RHS with c/dist as 
 * Ravi:   the cost limit.
 * Ravi: - Let rc be the cost of the best plan for RHS.
 * Ravi: - The cost of the InExpr returned is dist * rc
 */
CostVal_t SubQueryCost(InExpr_t *inCond, LogProp_t *outLP,
                                Volcano_t *opt, CostVal_t *costLimit,
                                PlanGroup_t **subQueryPlanGroup)
{
    assert(inCond);
    assert(subQueryPlanGroup);

    Card_t dist = inCond->NumDistinct(outLP);

    *subQueryPlanGroup = inCond->InpPlanGroup();
    if( !*subQueryPlanGroup ) {
        Equivalence_t *subEq = inCond->InpEq();
        if( !costLimit ) {
            *subQueryPlanGroup = opt->FindBestPlan(subEq, NULL);
        } else {
            CostVal_t subQueryCostLimit = *costLimit/dist;
            *subQueryPlanGroup =
                        opt->FindBestPlan(subEq, &subQueryCostLimit);
            if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) )
                cout << "Nested Query: Eq = " << subEq->ID() <<
                " CostLimit = " << *costLimit/dist <<
                " Best Plan Cost = " << subQueryCostLimit << endl;
        }

        if( !*subQueryPlanGroup ) {
            return -1;
        }

        if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) )
            cout << "Nested Query Optimization Successful" << endl;
        inCond->SetInpPlanGroup(*subQueryPlanGroup);
    }

    Plan_t *subQueryBestPlan = (*subQueryPlanGroup)->BestPlan();
    assert(subQueryBestPlan);
    assert(!subQueryBestPlan->IsFailed()); // valid plan

    if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) ) {
        cout << "dist = " << dist <<
        " subQueryCost = " << subQueryBestPlan->CostVal() << endl;
    }
    return dist * subQueryBestPlan->CostVal();
}

void FixNumDistincts(SchemaProp_t *sp, Card_t numTuples)
{
    assert(sp);

    int n = sp->NumAttr();
    for( int i = 0 ; i < n ; i++ ) {
        AttrProp_t *a = sp->AttributeProp(i);
        assert(a);

        if( a->NumDistinct() > numTuples ) {
            if( IsShared(a) ) {
                a = a->Copy();
                sp->SetAttributeProp(i, a);
            }
            a->SetNumDistinct(numTuples);
        }
    }
}


// Ravi: *******************************************************
// Ravi: New methods added for the Apply operator 
// Ravi: *******************************************************

// logical property computation for the Apply operator
LogProp_t *Apply_t::ComputeLogicalProp(void) const
{
    const Predicate_t *predicate = op->Predicate();
    assert(predicate);

    const Equivalence_t *inp = Input(0);
    assert(inp);
    LogProp_t *inpLP = inp->LogicalProp();
    assert(inpLP);

    // schema remains unchanged
    Schema_t *schema = inpLP->Schema();
    Refer(schema);

    SchemaProp_t *inpSchemaProp = inpLP->SchemaProp();
    assert(inpSchemaProp);
    int numAttr = inpSchemaProp->NumAttr();
                                        // number of attributes

    // initialize schema property with the input schema property
    SchemaProp_t *schemaProp = new SchemaProp_t(numAttr);
    Refer(schemaProp);
    for( int i = 0 ; i < numAttr ; i++ ) {
        AttrProp_t *inpAttrProp = inpSchemaProp->AttributeProp(i);
        schemaProp->SetAttributeProp(i, inpAttrProp);
    }

    // recursively refines the schema property using the predicate and
    // returns the selectivity
    float selectivity = predicate->InferProp(schema, schemaProp);
    if( selectivity == 0 ) {
        cout << "--- ZERO SELECTIVITY ---" << endl;
        predicate->PrintExpr();
        selectivity = 1.0/Config_t::CardLimit();
    }
    assert(selectivity > 0);

    Card_t inpRelSize_Tuples = inpLP->RelSize_Tuples();
    Card_t relSize_Tuples = ceil(selectivity * inpRelSize_Tuples);
    FixNumDistincts(schemaProp, relSize_Tuples);

    LogProp_t *lp = new IntRelLogProp_t(NULL, schema, schemaProp, 
                                        relSize_Tuples);
    DeRefer(schema);
    DeRefer(schemaProp);

    return lp;
}


// Combines the bind i/p pso with the pso coming from top
Order_t *Apply_t::CombinePSO(Order_t *topPSO, Order_t *localPSO)
{
    int topPSOSize    = topPSO == NULL? 0 : topPSO->OrderSize();
    int localPSOSize  = localPSO == NULL? 0 : localPSO->OrderSize();
    int newOrdSize    = topPSOSize + localPSOSize;
    
    if(newOrdSize == 0) {
        return NULL;
    }

    int *newOrd       = new int[newOrdSize];

    for (int i = 0; i < topPSOSize; i++)
        newOrd[i] = topPSO->Order()[i];

    for (int i = 0; i < localPSOSize; i++)
        newOrd[topPSOSize+i] = localPSO->Order()[i];
    
    SortOrder_t *newPSO = new SortOrder_t(newOrd, newOrdSize);
    delete[] newOrd;
    
    return newPSO;
}

// Ravi:TODO: Yet to be implemented.
// Ravi:We need to push the predicate into the left input.
// Ravi:Returns NULL if no new operator is generated else the newly
// Ravi:created operator is returned.
LogicalOp_t *Apply_t::PushInPred(Predicate_t *inPred, Memo_t&)
{
    return NULL; 

    /* Following code is from the Select_t::PushInPred. Commented out.
    
    if( inPred->IsEquivalent(ConstPreds.True()) )
        return NULL;

    Predicate_t *selectPred = Predicate();
    Predicate_t *newSelectPred = NULL;

    if( selectPred->IsEquivalent(ConstPreds.True()) ) {
        newSelectPred = inPred;
    } else if( inPred->Implies(selectPred) ) {
        newSelectPred = inPred;
    } else if( selectPred->Implies(inPred) ) {
        newSelectPred = selectPred;
    } else {
        newSelectPred = new And_t(inPred, selectPred);
    } 

    Refer(newSelectPred);
    Select_t *newSelect = new Select_t(newSelectPred);
    DeRefer(newSelectPred);

    newSelect->MarkDerived();

    Equivalence_t *inp = Input(0);
    assert(inp);

    newSelect->SetInput(0, inp);

    return newSelect;
    */
}


// Ravi: Push the project across the apply
// Ravi: We need to push the project into the left input but retain all the
// Ravi: columns that are used in the right input.
// Ravi: Returns NULL when no new operator is generated. Else returns the 
// Ravi: newly constructed operator.
LogicalOp_t *Apply_t::PushInProj(Project_t *inProj, Memo_t& memo)
{
    return NULL;

    /* Following code is from the Select_t::PushInPred. Commented out.
    
    assert(inProj);

    LogProp_t *lp = LogicalProp();
    assert(lp);

    Schema_t *schema = lp->Schema();
    assert(schema);

    // get the schema for inProj
    Equivalence_t *eq = inProj->EqClass();
    assert(eq);

    LogProp_t *inProjLP = eq->LogicalProp();
    assert(inProjLP);

    Schema_t *inProjSchema = inProjLP->Schema();
    assert(inProjSchema);

    Predicate_t *selectPred = Predicate();
    assert(selectPred);

    if( !selectPred->IsValidSchema(schema, inProjSchema) )
        return NULL;

    // inProject can be pushed in
    // create new project operator

    AttrMap_t *inAttrMap = inProj->AttrMap();

    Project_t *newProj = new Project_t(inAttrMap);
    newProj->MarkDerived();

    Equivalence_t *inp = Input(0);
    assert(inp);

    newProj->SetInput(0, inp);

    // get the equivalence class corresponding to newProj
    Equivalence_t *newProjEq = NULL;
    LogicalOp_t *prev = memo.Search(newProj);
    if( prev ) {
        // already present
        delete newProj;
        newProj = NULL;
        newProjEq = prev->EqClass();
    } else {
        LogProp_t *newProjLP = newProj->LogicalProp();
        Refer(newProjLP);

        newProjEq = new Equivalence_t(newProjLP);
        DeRefer(newProjLP);

        newProj->SetEqClass(newProjEq);
        newProjEq->AddLogExpr(newProj);

        memo.Insert(newProj);
    }
    assert(newProjEq);

    Select_t *newSelect = new Select_t(selectPred);
    newSelect->MarkDerived();

    newSelect->SetInput(0, newProjEq);

    return newSelect;
    */
}

void Apply_t::ApplyAlgorithms(PlanGroup_t *planGroup, Volcano_t *opt,
                              Cost_t *totalCost, Plan_t *bestPlan, 
                              CostVal_t *costLimit, Order_t* gpso, int *plmap, 
                              int level, Card_t callCount)
{
    assert(planGroup);
    assert(opt);
    assert(totalCost);
    assert(bestPlan);

    ApplyBasicApplyOp(planGroup, opt, totalCost, bestPlan, costLimit,
                      gpso, plmap, level, callCount);
}


void Apply_t::ApplyBasicApplyOp(PlanGroup_t *planGroup, Volcano_t *opt,
              Cost_t *totalCost, Plan_t *bestPlan, CostVal_t *costLimit, 
              Order_t* gpso, int *plmap, int level, Card_t callCount)
{
    // input equivalence classes
    Equivalence_t *leftInpEq = Input(0);
    assert(leftInpEq);
    Equivalence_t *rightInpEq = Input(1);
    assert(rightInpEq);

    // get the logical property of the inputs
    LogProp_t *leftInpLP = leftInpEq->LogicalProp();
    assert(leftInpLP);
    LogProp_t *rightInpLP = rightInpEq->LogicalProp();
    assert(rightInpLP);
    
    // Card_t leftCallCount = leftInpLP->RelSize_Tuples();
     Card_t leftCallCount = NumDistinct();

    // the logical property of the output
    LogProp_t *outLP = LogicalProp();
    Refer(outLP);

    // Doubt: What should be the apply cost?
    CostVal_t basicApplyCost = callCount * BasicApplyOp_t::Cost(
                               leftInpLP, rightInpLP, outLP);
    if( costLimit ) {
        // pruning enabled
        // if not feasible, return
        if( *costLimit - basicApplyCost < totalCost->Value() ) {
            DeRefer(outLP);
            return;
            }
    }

    DeRefer(outLP);

    CostVal_t inpCostLimit = -1;
    
    if( costLimit ) {
        // pruning enabled
        // calculate the costLimit
        inpCostLimit = *costLimit - basicApplyCost;

        // if not feasible, return
        if( inpCostLimit < totalCost->Value() )
            return;
    }

    // properties required in the current plan
    PhyPropGroup_t *reqProp = planGroup->PropGroup();

    // We assume apply op does not guarantee any physical property
    // so, it is required that no physical property is required
    if( reqProp->NumProps() != 0 )
        return;

    // Get the interesting parameter sort orders from right i/p
    List_t<Order_t *> *iOrds;
    // TODO: Augment plmap with the variables bound in leftInpEq 
    int *rightPlmap = plmap;
    iOrds = rightInpEq->GetInterestingOrders(rightPlmap, level+1);
    assert(iOrds);

    // now optimize the inputs for each interesting pso
    CostVal_t leftInpCostLimit = inpCostLimit;
    PlanGroup_t *leftInpPlanGroup = NULL;

    ListIter_t<Order_t *> ordIter;
    ordIter.Attach(iOrds);
    while(!ordIter.IsEnd()) {
        Order_t *ord = (Order_t *) ordIter.Next();
        assert(ord);
        // TODO: currently an ugly hack (fix bug in GIO)
        if (ord->OrderSize() == 0)
            break;
        // Get local orders to be asked from left input
        Order_t *leftInpReqOrd = NULL;
        leftInpReqOrd = (Order_t *)ord->Translate(rightInpEq->OuterVarLogProp(),
                                                  leftInpEq->LogicalProp(), 1);
        assert(leftInpReqOrd);
        PhyPropGroup_t *leftInpReqProp = new PhyPropGroup_t();
        Refer(leftInpReqProp);
        leftInpReqProp->AddProperty(leftInpReqOrd);

        Order_t *leftGPSO = NULL;
        if (gpso) {
            leftGPSO = (Order_t *) gpso->Translate(EqClass()->OuterVarLogProp(),
                                         leftInpEq->OuterVarLogProp(), 1);
        }
        if( costLimit ) { 
                leftInpPlanGroup = opt->FindBestPlan(leftInpEq, leftInpReqProp, 
                               totalCost, &leftInpCostLimit, 0, leftGPSO, 
                               plmap, level, callCount);
        }
        else {
                leftInpPlanGroup = opt->FindBestPlan(leftInpEq, leftInpReqProp, 
                               totalCost, NULL, 0, leftGPSO, plmap, level, 
                               callCount);
            assert(leftInpPlanGroup);
        }

        Refer(leftInpReqProp);
        // if no feasible plan found, return
        if( !leftInpPlanGroup ) {
            DeRefer(leftInpReqProp);
            continue;
        }

        Plan_t *leftInpBestPlan = leftInpPlanGroup->BestPlan();
        assert(leftInpBestPlan);
        Cost_t *leftInpCost = leftInpBestPlan->Cost();

        if( opt->IsRU() && leftInpPlanGroup->IsMarked() &&
                leftInpPlanGroup->UseCost() < leftInpBestPlan->CostVal() )
            leftInpCost = leftInpPlanGroup->UseCostADT(opt);

        assert(leftInpCost);

        // update totalCost for the right input optimization
        totalCost->Add(leftInpCost);

        CostVal_t rightInpCostLimit = inpCostLimit;

        // Now optimize the use-expr with ord as the guaranteed pso
        Order_t *rightGPSO = NULL;
        if (gpso) {
            rightGPSO = (Order_t *)gpso->Translate(EqClass()->OuterVarLogProp(),
                                        rightInpEq->OuterVarLogProp(), 1);
        }

        // Translate back the req sort order to guaranteed sort order
        Order_t *localPSO = (Order_t *) leftInpReqOrd->Translate(
                                        leftInpEq->LogicalProp(),
                                        rightInpEq->OuterVarLogProp(), 1);
        // Concatenate the rightGPSO and localPSO
        rightGPSO = CombinePSO(rightGPSO, localPSO);

        // Create an empty required prop group for right input
        PhyPropGroup_t *rightInpReqProp = new PhyPropGroup_t();
        Refer(rightInpReqProp);

        // Ravi: WARNING: Changed for testing
        // Card_t rightCallCount = callCount*leftCallCount;
        Card_t rightCallCount = leftCallCount;
        PlanGroup_t *rightInpPlanGroup = NULL;

        if( costLimit ) {
            // pruning enabled
                rightInpPlanGroup = opt->FindBestPlan(rightInpEq, rightInpReqProp, 
                                totalCost, &rightInpCostLimit, 0, rightGPSO, 
                                rightPlmap, level+1, rightCallCount);
        }
        else {
                rightInpPlanGroup = opt->FindBestPlan(rightInpEq, rightInpReqProp, 
                                totalCost, NULL, 0, rightGPSO, 
                                rightPlmap, level+1, rightCallCount);
            assert(rightInpPlanGroup);
        }
        
        // restore totalCost
        totalCost->Subtract(leftInpCost);

        // if no feasible plan found, return
        if( !rightInpPlanGroup ) {
            // DeRefer(inpReqProp);
            continue;
        }

        Plan_t *rightInpBestPlan = rightInpPlanGroup->BestPlan();
        assert(rightInpBestPlan);
        Cost_t *rightInpCost = rightInpBestPlan->Cost();

        if( opt->IsRU() && rightInpPlanGroup->IsMarked() &&
                rightInpPlanGroup->UseCost() < rightInpBestPlan->CostVal() )
            rightInpCost = rightInpPlanGroup->UseCostADT(opt);

        assert(rightInpCost);
        // DeRefer(inpReqProp);


        // Now create the BasicApplyOp and add it to the plan group
 
        BasicApplyOp_t *basicApplyOp = new BasicApplyOp_t(this);
        basicApplyOp->SetCallCount(callCount);
        basicApplyOp->SetInput(0, leftInpPlanGroup);
        basicApplyOp->SetInput(1, rightInpPlanGroup);
        basicApplyOp->SetCostVal(basicApplyCost);

        // calculate the cost
        Cost_t *cost = opt->CreateCost(basicApplyOp, basicApplyCost);
        Refer(cost);
        cost->Add(leftInpCost);
        cost->Add(rightInpCost);

        // add the plan to the plan group
        planGroup->AddAlgPlan(basicApplyOp, cost);

        // current total plan cost
        CostVal_t curCost = totalCost->SumCost(cost);

        // update the best plan if found plan better
        int isBetterPlan = 1;

        // if pruning enabled, we know that we have arrived at a better
        // plan, so a check is not necessary
        if( !costLimit ) {
            // pruning disabled
            Cost_t *prevCost = bestPlan->Cost();
            if( prevCost ) {
                CostVal_t prevBestCost = totalCost->SumCost(prevCost);
                if( curCost >= prevBestCost )
                    isBetterPlan = 0;
            }
        }
    
        if( isBetterPlan ) {
            bestPlan->SetRoot(basicApplyOp);
            bestPlan->SetCost(cost);

            if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) ) {
                bestPlan->PrintPlan();
                cout << endl;
                cout << endl;
            }
        }

        if( costLimit ) {
            // update the costLimit
            *costLimit = curCost;
        }

        DeRefer(cost);
    }
}


// transformations on Apply op 
void Apply_t::ApplyTransformations(Memo_t& memo)
{
    /* Ravi: Currently no transformations on the Apply Op
    Predicate_t *selPred = Predicate();
    assert(selPred);

    assert(!selPred->IsEquivalent(ConstPreds.True()));
    SelectPushPred_t::Apply(this, memo);
    */
}


// index order for indexed selection
int Apply_t::IndexProperty(IndexOrder_t **indexOrder) const
{
    return 1;

    /*

    assert(indexOrder);

    Predicate_t *p = Predicate();
    assert(p);

    Equivalence_t *inpEq = Input(0);
    assert(inpEq);
    LogProp_t *inpLP = inpEq->LogicalProp();
    assert(inpLP);

    Schema_t *inpSchema = inpLP->Schema();
    assert(inpSchema);

    // get the attributes bindings that are part of the relevant join predicate

    int inpBnd = -1;

    int isValid = p->IndexAttrBnd(inpSchema, &inpBnd);
    if( isValid ) {
        assert(inpBnd != -1);

        *indexOrder = new IndexOrder_t(&inpBnd, 1);
    }

    return isValid;
    */
}

// lower bound on the algorithm cost for the logical operator
CostVal_t Apply_t::CostLB(void) const
{
    //Ravi:TODO:Yet to be implemented.
    return NULL;

    /*
    // input equivalence class
    Equivalence_t *inpEq = Input(0);
    assert(inpEq);

    // get the logical property of the input
    LogProp_t *inpLP = inpEq->LogicalProp();
    assert(inpLP);

    // the logical property of the output
    LogProp_t *outLP = LogicalProp();
    Refer(outLP);

    CostVal_t selCost = SelectScan_t::Cost(inpLP, outLP);
    DeRefer(outLP);

    return selCost;
    */
}

// for debugging
void Apply_t::PrintExpressionDAG(int indentLevel) const
{
    for(int j = 0; j < indentLevel; j++)
        cout << "| ";
 
    cout << "[LOGEXP" << ID() << "]";

    cout << "APPLY< ";
    Predicate_t *p = op->Predicate();
    assert(p);
    p->PrintExpr(1);
    cout << " >";

    cout << endl;
 
    for( int i = 0 ; i < numInputs ; i++ ) {
        Equivalence_t *inp = inputs[i];
        assert(inp);
        inp->PrintExpressionDAG(indentLevel+1);
    }
}


// DAG print - XML version
void Apply_t::PrintExpressionDAGXML(int indentLevel) const
{
    for(int j = 0; j < indentLevel; j++)
        cout << "  ";
 
    cout << "<LOGEXP id=\"" << ID() << "\" op=\"APPLY\"> " << endl;

    Predicate_t *p = op->Predicate();
    assert(p);
    p->PrintExprXML(1);

    /*
    if (p->FindIn()) {
        cout << "\">" << endl;
        cout << "<![CDATA[";
        p->PrintExpr(1);
        cout << "]]>" << endl;
    }
    else {
        p->PrintExpr(1);
        cout << "\">" << endl;
    }
    */
        
    for( int i = 0 ; i < numInputs ; i++ ) {
        Equivalence_t *inp = inputs[i];
        assert(inp);
        inp->PrintExpressionDAGXML(indentLevel+1);
    }
    cout << "</LOGEXP>" << endl;
}

void Select_t::PrintExpressionDAGXML(int indentLevel) const
{
    for(int j = 0; j < indentLevel; j++)
        cout << "  ";
 
    cout << "<LOGEXP id=\"" << ID() << "\" op=\"SELECT\">" << endl;

    Predicate_t *p = Predicate();
    assert(p);
    p->PrintExprXML(1);

    /*
    if (p->FindIn()) {
        cout << "\">" << endl;
        cout << "<![CDATA[";
        p->PrintExpr(1);
        cout << "]]>" << endl;
    }
    else {
        p->PrintExpr(1);
        cout << "\">" << endl;
    }
    */
        
    for( int i = 0 ; i < numInputs ; i++ ) {
        Equivalence_t *inp = inputs[i];
        assert(inp);
        inp->PrintExpressionDAGXML(indentLevel+1);
    }
    cout << "</LOGEXP>" << endl;
}


// DAG print - XML version
void Join_t::PrintExpressionDAGXML(int indentLevel) const
{
    for(int j = 0; j < indentLevel; j++)
        cout << "  ";
 
    cout << "<LOGEXP id=\"" << ID() << "\" op=\"JOIN\"> " << endl;

    Predicate_t *p = Predicate();
    assert(p);
    p->PrintExprXML(1);

    for( int i = 0 ; i < numInputs ; i++ ) {
        Equivalence_t *inp = inputs[i];
        assert(inp);
        inp->PrintExpressionDAGXML(indentLevel+1);
    }
    cout << "</LOGEXP>" << endl;
}

// DAG print - XML version
void Aggregate_t::PrintExpressionDAGXML(int indentLevel) const
{
    for(int j = 0; j < indentLevel; j++)
        cout << "  ";
 
    cout << "<LOGEXP id=\"" << ID() << "\" op=\"AGGREGATE\"> " << endl;
    cout << "<AGG_EXPRS>" << endl;
    int numAggAttrs = NumAggAttrs();
    int i = 0;
        for( i = 0 ; i < numAggAttrs ; i++ ) {
            Expr_t *e = AggExpr(i);
            assert(e);

            e->PrintExprXML();
        }
    cout << "</AGG_EXPRS>" << endl;

    cout << "<GROUPBY_ATTRS>" << endl;
        int numGroupByAttrs = NumGroupByAttrs();
        for( i = 0 ; i < numGroupByAttrs ; i++ ) {
            Expr_t *e = GroupByExpr(i);
            assert(e);

            e->PrintExprXML();
        }
    cout << "</GROUPBY_ATTRS>" << endl;
    
    for( int i = 0 ; i < numInputs ; i++ ) {
        Equivalence_t *inp = inputs[i];
        assert(inp);
        inp->PrintExpressionDAGXML(indentLevel+1);
    }
    cout << "</LOGEXP>" << endl;
}

// DAG print - XML version
void Project_t::PrintExpressionDAGXML(int indentLevel) const
{
    for(int j = 0; j < indentLevel; j++)
        cout << "  ";
 
    cout << "<LOGEXP id=\"" << ID() << "\" op=\"PROJECT\"> " << endl;
    cout << "<PROJ_EXPRS>" << endl;
        int numProj = NumProj();
        for( int i = 0 ; i < numProj ; i++ ) {
            Expr_t *e = ProjExpr(i);
            assert(e);

            e->PrintExprXML();
        }
    cout << "</PROJ_EXPRS>" << endl;

    for( int i = 0 ; i < numInputs ; i++ ) {
        Equivalence_t *inp = inputs[i];
        assert(inp);
        inp->PrintExpressionDAGXML(indentLevel+1);
    }
    cout << "</LOGEXP>" << endl;
}

/**
 * Populates the outerVarLogProp of the parent eq node.
 * See Equivalence_t::SetOuterVarLogProp()
 * Note that we have NOT moved this implementation to Filter_t because the
 * schema to be passed for the predicate for Join_t will be the schema
 * of the cross product of its inputs and for Apply also it is different.
 */
void Select_t::SetOuterVarLogProp()
{
    LogicalOp_t::SetOuterVarLogProp();

    Predicate_t *predicate = Predicate();
    assert(predicate);

    Equivalence_t *inp = Input(0);
    assert(inp);
    LogProp_t *inpLP = inp->LogicalProp();
    assert(inpLP);
    Schema_t *schema = inpLP->Schema();

    AppendList_t<Attr_t *> corrVarList;
    // Get the outer variables used in the predicate and also 
    // set the outerVarLogProp for any subqueries in the predicate.
    predicate->CorrelatedAttrList(schema, &corrVarList);

    if (!eqClass->IsOuterVarLogPropSet()) {
        eqClass->UnionOuterVarLogProp(MakeLogProp(MakeSchema(corrVarList)));
        // Ravi: TODO: Should we delete(s) here?
        if (corrVarList.Size() > 0)
            eqClass->MarkCorrelated();
    }
}


/**
 * Populates the outerVarLogProp of the parent eq node.
 * See Equivalence_t::SetOuterVarLogProp()
 */
void Apply_t::SetOuterVarLogProp()
{
    if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) )
        cout << "Setting outerVarLogProp for Op = " << ID() << endl;

    int n = NumInputs();

    for( int i = 0 ; i < n ; i++ ) {
        Equivalence_t *inp = Input(i);
            assert(inp);
        if (!inp->IsOuterVarLogPropSet()) {
            inp->SetOuterVarLogProp();
        }
    }

    if (!eqClass->IsOuterVarLogPropSet()) {
        Equivalence_t *leftInp = Input(0);
        assert(leftInp);
        eqClass->UnionOuterVarLogProp(leftInp->OuterVarLogProp());
        if (leftInp->IsCorrelated())
            eqClass->MarkCorrelated();

        // From the right input outer variables remove all those
        // which have a binding in the left input.

        Schema_t *leftSchema = leftInp->LogicalProp()->Schema();
        int numLeftAttr = leftSchema->NumAttr();
        Equivalence_t *rightInp = Input(1);
        assert(rightInp);
        int numRightAttr;
        LogProp_t *useExprOuterVarLogProp = rightInp->OuterVarLogProp();
        Schema_t *useExprOuterVarSchema;

        if (useExprOuterVarLogProp) {
            useExprOuterVarSchema = useExprOuterVarLogProp->Schema();
            numRightAttr = useExprOuterVarSchema->NumAttr();
        }
        else {
            numRightAttr = 0;
        }

        AppendList_t<Attr_t *> tempList;

        for (int j = 0; j < numRightAttr; j++) {
            Attr_t *rightCorrAttr = useExprOuterVarSchema->Attribute(j);
            assert(rightCorrAttr);
            int found = 0;
            for (int i = 0; i < numLeftAttr; i++) {
                if (rightCorrAttr->IsEquivalent(leftSchema->Attribute(i))) {
                    found = 1;
                    break;
                }
            }
            if (!found)
                tempList.Insert(rightCorrAttr);
        }

        if (tempList.Size() > 0) {
            eqClass->UnionOuterVarLogProp(MakeLogProp(MakeSchema(tempList)));
            eqClass->MarkCorrelated();
        }
    }

    if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) )
        cout << "Setting outerVarLogProp for Op = " << ID() 
             << " Done" << endl;
}


void LogicalOp_t::GetInterestingOrders(List_t<Order_t *> *intOrds, 
                                      int *plmap, int level) 
{
    if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) )
        cout << "Getting interesting props for Op = " << ID() << endl;

    int n = NumInputs();

    for( int i = 0 ; i < n ; i++ ) {
        Equivalence_t *inp = Input(i);
        assert(inp);

        List_t<Order_t *> *childOrds = inp->GetInterestingOrders(plmap, level);
        assert(childOrds);

        // Now translate the child orders to the schema asssociated
        // with the parent eq node.
        LogProp_t *srcLP  = inp->OuterVarLogProp();
        LogProp_t *destLP = EqClass()->OuterVarLogProp();

        if (srcLP && destLP) {
            // TODO (Memory): Should actually free the current childOrds.       
            childOrds = TranslateOrders(childOrds, srcLP, destLP);
            // Add the orders from the childOrds to the intOrds if not already 
            // present.
            UnionOrders(intOrds, childOrds, destLP);
        }
    }
}

void LogicalOp_t::GetFavorableOrders(List_t<Order_t *> *favOrds)
{
    int n = NumInputs();

    for( int i = 0 ; i < n ; i++ ) {
        Equivalence_t *inp = Input(i);
        assert(inp);

        List_t<Order_t *> *childOrds = inp->GetFavorableOrders();
        assert(childOrds);

        // Translate the child orders to the schema asssociated
        // with the parent eq node.
        LogProp_t *srcLP  = inp->LogicalProp();
        assert(srcLP);
        LogProp_t *destLP = EqClass()->LogicalProp();
        assert(destLP);
        childOrds = TranslateOrders(childOrds, srcLP, destLP, 
                                    TRANSLATE_PREFIX_ONLY);

        UnionOrders(favOrds, childOrds, destLP);
    }
}


// Here we should consider all the possible algorithms for 
// Select and should get the interesting orders for each.
// For now, just calling the predicate->GetInterestingOrders.
void Select_t::GetInterestingOrders(List_t<Order_t *> *intOrds, 
                                    int *plmap, int level) 
{
    // Call the super class method so that the child eq nodes are
    // processed.
    LogicalOp_t::GetInterestingOrders(intOrds, plmap, level);

    // Now add the orders of interest to this op
    Equivalence_t *inp = Input(0);
    assert(inp);
    LogProp_t *inpLP = inp->LogicalProp();
    assert(inpLP);

    AppendList_t<Order_t *> opOrds;
    Equivalence_t *eqClass = EqClass();
    // From the predicate find all the parameter orders of interest
    // considering the built-in index orders present in inp->LP. The
    // generated orders will be w.r.t. to the outervar schame of the 
    // eqClass
    Predicate()->GetInterestingOrders(&opOrds, inp, eqClass, plmap, level);

    UnionOrders(intOrds, &opOrds, eqClass->OuterVarLogProp());
}


void Apply_t::GetInterestingOrders(List_t<Order_t *> *intOrds, 
                                    int *plmap, int level) 
{

    if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) )
        cout << "Getting interesting props for Apply Op = " << ID() << endl;

    int n = NumInputs();
    Equivalence_t *eqClass = EqClass();

    // Get the interesting orders for the bind input
    Equivalence_t *bindInp = Input(0);
    assert(bindInp);
    List_t<Order_t *> *bindInpOrds = bindInp->GetInterestingOrders(plmap, level);
    assert(bindInpOrds);

    // Now translate the child orders to the outer var schema asssociated
    // with the parent eq node.
    LogProp_t *srcLP = bindInp->OuterVarLogProp();
    LogProp_t *destLP = EqClass()->OuterVarLogProp();
    
    if (srcLP && destLP) {
        // TODO (Memory): Should actually free the current bindInpOrds.
        bindInpOrds = TranslateOrders(bindInpOrds, srcLP, destLP);
        // Add the orders from the childOrds to the intOrds if not already 
        // present.
        UnionOrders(intOrds, bindInpOrds, destLP);
    }

    // Now process the use inputs
    for( int i = 1 ; i < n ; i++ ) {
        Equivalence_t *inp = Input(i);
        assert(inp);
       
        // TODO: Augment the parameter level map 
        // int *newPLM = GetAugmentedPLM(plmap, level, inp);
        int *newPLM = NULL; 

        List_t<Order_t *> *childOrds = inp->GetInterestingOrders(newPLM, level+1);
//      cout << "\n\n\n\n DEBUG: childOrds address : " << childOrds;
        assert(childOrds);
        // Translate the orders to get the ancestor orders
        srcLP = inp->OuterVarLogProp();

        if (srcLP && destLP) {
            // TODO (Memory): Should free old childOrds?
            childOrds = TranslateOrders(childOrds, srcLP, destLP);
            UnionOrders(intOrds, childOrds, destLP);
        }
    }
}

void Apply_t::PrintName() const
{
    cout << "APPLY ";
    op->PrintName();
    //cout << " >";
}

/*
List_t<Order_t *> *DeduceIntOrds(const Set_t<int>& gbAttrSet, 
                                 const List_t<Order_t *> *inpFavOrds)
{
    AppendList_t<Order_t *> *intOrds = new AppendList_t<Order_t *>();

    ListIter_t<Order_t *> favOrdIter;
    favOrdIter.Attach(inpFavOrds);

    while(!favOrdIter.IsEnd()) {
        SortOrder_t *favOrd = (SortOrder_t *) favOrdIter.Next();
        SortOrder_t *lcp    = favOrd->LongestCommonPrefix(&gbAttrSet);
        if (lcp != NULL) {
            Set_t<int> *lcpAttrs = lcp->AttrSet();
            Set_t<int> *freeAttrs = gbAttrSet.Minus(lcpAttrs);
            delete lcpAttrs;

            Order_t *ord = lcp;
            if (freeAttrs->Size() > 0) {
                // Choose an arbitrary order of the free attributes
                SortOrder_t *aOrder = SortOrder_t::MakeOrder(freeAttrs);
                // Add lcp + aOrder to the list of favorable orders
                ord = lcp->Concat(aOrder);
            }
            intOrds->Insert(ord);
        }
    }

    if (intOrds->IsEmpty()) {
        SortOrder_t *aOrder = SortOrder_t::MakeOrder(&gbAttrSet);
        intOrds->Insert((Order_t *&)aOrder);
    }
    return intOrds;
}
*/

// attrSet: Set of attrs in the group by or join predicate
// favOrds: Set of favorable orders from inputs and reqd o/p
List_t<Order_t *> *DeduceIntOrds(const Set_t<int> *attrSet, 
                                 const List_t<Order_t *> *favOrds,
                                 const SchemaProp_t *sp)
{
    AppendList_t<Order_t *> *intOrds = new AppendList_t<Order_t *>();

    SortOrder_t *tempList[MAX_FAV_ORDS];
    int tempListSize = 0;
    int deleteList[MAX_FAV_ORDS];
    int deleteCount = 0;

    ListIter_t<Order_t *> favOrdIter;
    favOrdIter.Attach(favOrds);
    while(!favOrdIter.IsEnd()) {
        SortOrder_t *fOrd = (SortOrder_t *) favOrdIter.Next();
        SortOrder_t *lcp  = fOrd->LongestCommonPrefix(attrSet);
        if (lcp != NULL) {
            // Insert only if no order >= lcp already present.
            // Also remove all orders < lcp
            bool toAdd = true;
            for (int i = 0; i < tempListSize; i++) {
                if (tempList[i]->IsCovered(sp, lcp)) {
                    deleteList[deleteCount++] = i;
                }
                else if (tempList[i]->Covers(sp, lcp)) {
                    toAdd = false;
                }
            }
            if (toAdd) {
                assert(tempListSize < MAX_FAV_ORDS);
                tempList[tempListSize++] = lcp;
            }
        }
    }

    for (int i = 0; i < tempListSize; i++) {
        bool skip = false;
        for (int j = 0; j < deleteCount; j++) {
            if (i == deleteList[j]) {
                skip = true;
                break;
            }
        }
        if (skip)
            continue;

        Set_t<int> *lcpAttrs = tempList[i]->AttrSet();
        Set_t<int> *freeAttrs = attrSet->Minus(lcpAttrs);
        delete lcpAttrs;

        Order_t *ord = tempList[i];
        if (freeAttrs->Size() > 0) {
            // Choose an arbitrary order of the free attributes
            SortOrder_t *aOrder = SortOrder_t::MakeOrder(freeAttrs);
            // Add lcp + aOrder to the list of favorable orders
            ord = tempList[i]->Concat(aOrder);
        }
        intOrds->Insert(ord);
    }

    // If there were no fav ords, take an arbitrary permutation
    if (intOrds->IsEmpty()) {
        SortOrder_t *aOrder = SortOrder_t::MakeOrder(attrSet);
        intOrds->Insert((Order_t *&)aOrder);
    }
    return intOrds;
}

// Exhaustive enumeration of interesting orders. Returns all permutations 
// of the given attr set.  This is for comparison purposes.
List_t<Order_t *> *DeduceIntOrdsAllPermutations(const Set_t<int> *attrSet)
{
    AppendList_t<Order_t *> *intOrds = new AppendList_t<Order_t *>();
    Permuter p(intOrds, attrSet);
    p.Generate();
    return intOrds;
}

// Generate interesting orders using the heuristc of PostgreSQL.
// Algo: Make n permutations where n is the number of attributes.
// Each permutation uses a different starting attribute, the
// remaining attributes are chosen arbitrarily.
List_t<Order_t *> *DeduceIntOrdsPGHeuristic(const Set_t<int> *attrSet)
{
    AppendList_t<Order_t *> *intOrds = new AppendList_t<Order_t *>();
    Set_t<int> *headEltSet;
    Set_t<int> *tailEltSet;
    int headEltArr[1];
    const int *elts = attrSet->Elements();
    int setSize = attrSet->Size();

    for (int i=0; i < setSize; i++) {
        headEltArr[0] = elts[i];
        headEltSet = new Set_t<int>(headEltArr, 1);
        tailEltSet = attrSet->Minus(headEltSet);
        SortOrder_t *headOrd = SortOrder_t::MakeOrder(headEltSet);
        SortOrder_t *tailOrd = SortOrder_t::MakeOrder(tailEltSet);
        SortOrder_t *intOrd  = headOrd->Concat(tailOrd);
        intOrds->Insert((Order_t *&)intOrd);
        delete headEltSet;
        delete tailEltSet;
        headEltSet = NULL;
        tailEltSet = NULL;
    }
    return intOrds;
}

// Returns a single interesting order formed by taking an
// arbitrary permutation of the attrSet
List_t<Order_t *> *DeduceIntOrdsPlain(const Set_t<int> *attrSet)
{
    AppendList_t<Order_t *> *intOrds = new AppendList_t<Order_t *>();
    SortOrder_t *ord = SortOrder_t::MakeOrder(attrSet);

    intOrds->Insert((Order_t *&)ord);
    return intOrds;
}

void PrintOrds(const List_t<Order_t *> *intOrds, const Schema_t *schema)
{
    ListIter_t<Order_t *> iter;
    iter.Attach(intOrds);
    while(!iter.IsEnd()) {
        Order_t * ord = iter.Next();
        ord->Print(schema);
    }
}
