// physical operators

#include <assert.h>
extern "C" {
#include <math.h>
}

#include "typedefs.h"
#include "Config.h"

#include "LogProp.h"
#include "PhysicalOp.h"

#define my_max(a,b) ( ((a)>(b)) ? (a) : (b) )
#define my_min(a,b) ( ((a)<(b)) ? (a) : (b) )
// 1 added so that the log result does not become negative or -NaN
#define log2(x)     ceil( log(x+1)/log(2.0) )
#define logn(x,n)   ceil( log(x+1)/log(n) )

// id generator
int PhysicalOp_t::CurID = 0;

// instance count
int PhysicalOp_t::Count = 0;

float Sort_t::GetCombinedSelectivity(Order_t *order, LogProp_t *lp)
{
    SchemaProp_t *sp = lp->SchemaProp();
    assert(sp);

    int orderSize = order->OrderSize();
    const int *ordAttrs = order->Order();
    assert(ordAttrs);

    float sel = 1.0;
    for( int i = 0 ; i < orderSize ; i++ ) {
        const AttrProp_t *p = sp->AttributeProp(ordAttrs[i]);
        assert(p);

        sel /= p->NumDistinct();
    }
    
    return sel;
}

Card_t Sort_t::GetDistinctValues(Order_t *order, LogProp_t *lp)
{
    SchemaProp_t *sp = lp->SchemaProp();
    assert(sp);

    int orderSize = order->OrderSize();
    const int *ordAttrs = order->Order();
    assert(ordAttrs);

    Card_t dist = 1;
    for( int i = 0 ; i < orderSize ; i++ ) {
        const AttrProp_t *p = sp->AttributeProp(ordAttrs[i]);
        assert(p);

        dist *= p->NumDistinct();
    }

    Card_t relSizeTuples = lp->RelSize_Tuples();

    if (dist < relSizeTuples)
        return dist;
    else
        return relSizeTuples;
}

// Precondition: sortOrderProp subsumes partialProp 
CostVal_t Sort_t::Cost(Order_t *sortOrderProp, LogProp_t *inpLP, 
                       Order_t *partialProp)
{
    assert(sortOrderProp);
    assert(inpLP);

    // If Plain EIO, partial sorting is given the same cost
    // as a full sort. 
    if(Config_t::DisablePartialSort) {
        partialProp = NULL;
    }

    // For testing. Just assigning a small cost for partial sort.
    /*
    if (partialProp != NULL)
        return 0.0001;
    */

    // get the logical property of the input/output
    // schema property of the input/output
    SchemaProp_t *sp = inpLP->SchemaProp();
    assert(sp);

    // Start EIO changes
    float sel  = 1;
    Card_t dist = 1;
    if(partialProp != NULL) {
        assert(sortOrderProp->Covers(sp, partialProp));
        sel  = GetCombinedSelectivity(partialProp, inpLP);
        dist = GetDistinctValues(partialProp, inpLP); 
    }
    // End EIO changes

    int orderSize = sortOrderProp->OrderSize();
    const int *order = sortOrderProp->Order();
    assert(order);

    Card_t card = 1;
    for( int i = 0 ; i < orderSize ; i++ ) {
        const AttrProp_t *p = sp->AttributeProp(order[i]);
        assert(p);

        card *= p->NumDistinct();
    }

    if( card <= 1 )
        return 0;

    // EIO: Multiplying with sel
    Card_t relSize_Tuples = inpLP->RelSize_Tuples() * sel;
    assert(relSize_Tuples > 0);

    // EIO: Multiplying with sel
    Card_t relSize_Blocks = inpLP->RelSize_Blocks() * sel;
    assert(relSize_Blocks > 0);

    Card_t io_cost = 0;
    Card_t cpu_Blocks = 0;
    if( relSize_Blocks < NumBuffers ) {
        io_cost = 0;
    } else {
        // assuming each merge input has enough buffers so that seek
        // time is dwarfed by write+read time
	    // Ravi: Made a correction. Added ReadTime*relSizeBlocks
        io_cost = (ReadTime+WriteTime)* relSize_Blocks *
                ceil(logn(ceil(relSize_Blocks/NumBuffers), NumBuffers-1)) + 
                ReadTime * relSize_Blocks;
    }

    // EIO: Removed the +1
    // cpu_Blocks = log2(ceil(relSize_Tuples))*relSize_Blocks+1;
    cpu_Blocks = log2(ceil(relSize_Tuples))*relSize_Blocks;

    // cout << "CPU Blocks = " << cpu_Blocks << endl;

    // EIO: Multiplying with dist
    // cout << "Segment Sort Cost=" << Config_t::Cost(io_cost, cpu_Blocks);
    // cout << "Distincts = " << dist;

    // EIO: Removed the ceil
    // return Config_t::Cost(io_cost, ceil(cpu_Blocks)) * dist;
    return Config_t::Cost(io_cost, cpu_Blocks) * dist;
}

CostVal_t Index_t::Cost(LogProp_t *inpLP)
{
    assert(inpLP);

    Card_t relSize_Blocks = inpLP->RelSize_Blocks();
    assert(relSize_Blocks > 0);

    Card_t indexFanout = Config_t::IndexFanout();
    assert(indexFanout > 1);

    // Size of (clustered) index   =  1 + 1/fanout + 1/fanout^2 ...
    // Assumes input is pipelined
    // Output is sequential 

    Card_t io_cost =
                WriteTime * relSize_Blocks * (indexFanout/(indexFanout - 1));

    return Config_t::Cost(io_cost, 0);
}

CostVal_t IndexSort_t::Cost(Order_t *indexOrder, LogProp_t *inpLP)
{
    assert(inpLP);

    CostVal_t sortcost = Sort_t::Cost(indexOrder, inpLP);
    CostVal_t indexcost = Index_t::Cost(inpLP);

    return sortcost+indexcost;
}

CostVal_t RelScan_t::Cost(LogProp_t *lp)
{
    assert(lp);

    Card_t relSize_Blocks = lp->RelSize_Blocks();
    assert(relSize_Blocks > 0);

    return Config_t::Cost(ReadTime*relSize_Blocks, relSize_Blocks);
}

CostVal_t RelScan_t::MatCost(LogProp_t *lp)
{
    assert(lp);

    Card_t relSize_Blocks = lp->RelSize_Blocks();
    assert(relSize_Blocks > 0);

    return Config_t::Cost(WriteTime*relSize_Blocks, relSize_Blocks);
}

CostVal_t ProjectScan_t::Cost(LogProp_t *, LogProp_t *)
{
    // implicit
    return 0;
}

CostVal_t AggregateHash_t::Cost(LogProp_t *inLP, LogProp_t *outLP)
{
    assert(outLP);

    Card_t outRelSize_Blocks = outLP->RelSize_Blocks();
    assert(outRelSize_Blocks > 0);
    Card_t inRelSize_Blocks = inLP->RelSize_Blocks();
    assert(inRelSize_Blocks > 0);
    Card_t inRelSize_Tuples = inLP->RelSize_Tuples();
    assert(inRelSize_Tuples > 0);

    // cpu cost weightage is 0.2ms and 1 probe = 200 instr at 100MIPS 
    // in-memory hash
    if( outRelSize_Blocks < NumBuffers * 0.5 )
        return Config_t::Cost(0, ceil(inRelSize_Tuples*0.01+outRelSize_Blocks));

    // hybrid hash -- assuming half of mem is in hybrid portion
    return
        Config_t::Cost((ReadTime+WriteTime)*(inRelSize_Blocks-0.5*NumBuffers),
                            ceil(inRelSize_Tuples*0.01 + outRelSize_Blocks));
}

CostVal_t AggregateScan_t::Cost(LogProp_t *, LogProp_t *outLP)
{
    assert(outLP);

    Card_t outRelSize_Blocks = outLP->RelSize_Blocks();
    assert(outRelSize_Blocks > 0);

    return Config_t::Cost(0, ceil(outRelSize_Blocks));
}

CostVal_t SelectScan_t::Cost(LogProp_t *inpLP, LogProp_t *outLP)
{
    assert(inpLP);
    assert(outLP);

    Card_t inpRelSize_Blocks = inpLP->RelSize_Blocks();
    assert(inpRelSize_Blocks > 0);

    Card_t outRelSize_Blocks = outLP->RelSize_Blocks();
    assert(outRelSize_Blocks > 0);

    return Config_t::Cost(0, ceil(inpRelSize_Blocks+outRelSize_Blocks));
}

CostVal_t IndexedSelect_t::Cost(LogProp_t *inpLP, LogProp_t *outLP)
{
    assert(outLP);

    Card_t inpRelSize_Blocks = inpLP->RelSize_Blocks();
    assert(inpRelSize_Blocks > 0);

    Card_t indexFanout = Config_t::IndexFanout();
    assert(indexFanout > 1);

    Card_t outRelSize_Blocks = outLP->RelSize_Blocks();
    assert(outRelSize_Blocks > 0);

    float sel = 1.0;
    if( inpLP->RelSize_Tuples() > 0 )
        sel = outLP->RelSize_Tuples()/inpLP->RelSize_Tuples();
    
    // Note: We are assuming input is pipelined -- a relscan below
    //       ensures this if select is on a db relation, and accounts
    //       for the read cost

    // assume at least first level of index is in memory

    Card_t inBlocks = my_max(0,ceil(logn(inpRelSize_Blocks,indexFanout)));

    // If input rel << memory, assume lower levels are also  partially cached
    // Not very satisfactory with nested queries, but we don't  know how
    // many time select is called in that case, so we live with this.
    // To hack caching effects for nested queries, we assume cost is 0
    // if relation is small enough.

    if (inpRelSize_Blocks < NumBuffers * 0.75)
        inBlocks = inBlocks * (inpRelSize_Blocks/NumBuffers);
    if (inpRelSize_Blocks < NumBuffers * 0.25)
        inBlocks = 0; // Assume its entirely memory resident

    // Card_t iocost = (SeekTime+ReadTime)*inBlocks + 
    //                          ReadTime * outRelSize_Blocks;

    Card_t iocost = (SeekTime+ReadTime)*inBlocks + 
                                ReadTime * sel * inpRelSize_Blocks;

    return Config_t::Cost(iocost, ceil(outRelSize_Blocks));
}

CostVal_t MergeJoin_t::Cost(SortOrder_t *leftInpSortOrder,
                                SortOrder_t *rightInpSortOrder,
                                LogProp_t *leftLP, LogProp_t *rightLP,
                                                        LogProp_t *outLP)
{
    assert(leftInpSortOrder);
    assert(leftInpSortOrder->OrderSize() > 0);
    assert(rightInpSortOrder);
    assert(rightInpSortOrder->OrderSize() > 0);
    assert(leftLP);
    assert(rightLP);
    assert(outLP);

    const int *leftOrder = leftInpSortOrder->Order();
    assert(leftOrder);
    const int *rightOrder = rightInpSortOrder->Order();
    assert(rightOrder);

    SchemaProp_t *leftSchemaProp = leftLP->SchemaProp();
    assert(leftSchemaProp);
    SchemaProp_t *rightSchemaProp = rightLP->SchemaProp();
    assert(rightSchemaProp);

    // Ravi: Made changes to account for orders of size > 1
    Card_t leftDV  = 1;
    Card_t rightDV = 1;
    for (int i = 0; i < leftInpSortOrder->OrderSize(); i++) {
        AttrProp_t *leftAttrProp = leftSchemaProp->AttributeProp(leftOrder[i]);
        assert(leftAttrProp);
        leftDV *= leftAttrProp->NumDistinct();
    }

    for (int i = 0; i < rightInpSortOrder->OrderSize(); i++) {
        AttrProp_t *rightAttrProp = rightSchemaProp->AttributeProp(rightOrder[i]);
        assert(rightAttrProp);
        rightDV *= rightAttrProp->NumDistinct();
    }
    assert(leftDV  > 0);
    assert(rightDV > 0);

    Card_t leftRelSize_Blocks = leftLP->RelSize_Blocks();
    assert(leftRelSize_Blocks > 0);
    Card_t rightRelSize_Blocks = rightLP->RelSize_Blocks();
    assert(rightRelSize_Blocks > 0);

    Card_t outRelSize_Blocks = outLP->RelSize_Blocks();
    assert(outRelSize_Blocks > 0);

    // If too many duplicates in both relations on join attributes ...
    //  Cutoff is a blockfull

    if( leftRelSize_Blocks/leftDV > 1 && rightRelSize_Blocks/rightDV > 1 )
        return Config_t::CostLimit();

    // Arbitrary factor of 2 to account for merge costs

    return Config_t::Cost(0, 2*ceil(outRelSize_Blocks));
}

CostVal_t NestedLoopsJoin_t::Cost(LogProp_t *leftLP, LogProp_t *rightLP,
                                                        LogProp_t *outLP)
{
    assert(leftLP);
    assert(rightLP);
    assert(outLP);

    Card_t leftSize_Tuples = leftLP->RelSize_Tuples();
    Card_t rightSize_Tuples = rightLP->RelSize_Tuples();
    Card_t leftSize_Blocks = leftLP->RelSize_Blocks();
    Card_t rightSize_Blocks = rightLP->RelSize_Blocks();
    Card_t outSize_Blocks = outLP->RelSize_Blocks();
    

    // Note: We are assuming input is pipelined -- a relscan below
    //       ensures this if select is on a db relation, and accounts
    //       for the read cost

    if (leftSize_Blocks <= NumBuffers*0.5 )
        return Config_t::Cost(0, rightSize_Tuples*leftSize_Blocks +
                                 outSize_Blocks); 
    if ( rightSize_Blocks <= NumBuffers*0.5)
        return Config_t::Cost(0, leftSize_Tuples*rightSize_Blocks +
                                 outSize_Blocks); 

    // Otherwise do a block nested loops join

    assert(NumBuffers > 1);
    Card_t io_Blocks = (leftSize_Blocks*rightSize_Blocks)/(NumBuffers-1);
    Card_t io_cost = io_Blocks*ReadTime;

    return Config_t::Cost(io_cost, ceil(leftSize_Tuples * rightSize_Blocks 
                                     + outSize_Blocks));
}

CostVal_t IndexedNestedLoopsJoin_t::Cost(IndexOrder_t *indexOrder,
                    LogProp_t *indexLP, LogProp_t *probeLP, LogProp_t *outLP)
{
    assert(indexOrder);
    assert(indexLP);
    assert(probeLP);
    assert(outLP);

    Card_t indexSize_Blocks = indexLP->RelSize_Blocks();
    Card_t probeSize_Tuples = probeLP->RelSize_Tuples();
    Card_t outSize_Blocks = outLP->RelSize_Blocks();
    
    Card_t indexFanout = Config_t::IndexFanout();
    assert(indexFanout > 1);

    // Note: We are assuming input is pipelined -- a relscan below
    //       ensures this if select is on a db relation, and accounts
    //       for the read cost
    // If input rel << memory, assume it's read only once
    //  But add an 
    // else assume at least all two levels of index are in memory

    const int *order = indexOrder->Order();
    assert(order);

    SchemaProp_t *indexSchemaProp = indexLP->SchemaProp();
    assert(indexSchemaProp);

    AttrProp_t *indexAttrProp = indexSchemaProp->AttributeProp(order[0]);
    assert(indexAttrProp);

    Card_t indexDV = indexAttrProp->NumDistinct();
    assert(indexDV > 0);

    Card_t probeIO_Blocks =
        my_max(0,(ceil(log(indexSize_Blocks)/log(indexFanout)))-1)
                                            + indexSize_Blocks/indexDV;


    Card_t io_Blocks = probeSize_Tuples * probeIO_Blocks;

    // Assume index got partially cached in mem if smaller than mem size
    // The smaller the index the more of it is already in memory

    if (indexSize_Blocks < NumBuffers * 0.5 &&
            io_Blocks > (indexSize_Blocks/NumBuffers)*indexSize_Blocks)
        io_Blocks = (indexSize_Blocks/NumBuffers)*indexSize_Blocks;
    CostVal_t iocost = (SeekTime+ReadTime)*io_Blocks;

    // cpu cost weightage is 0.2ms and 1 probe = 1000 instr at 100MIPS 
    CostVal_t costVal = Config_t::Cost(iocost, 
                                ceil(probeSize_Tuples*0.05 + outSize_Blocks));
    assert(costVal >= 0);

    return costVal;
}

CostVal_t Merge_t::Cost(LogProp_t *, LogProp_t *, LogProp_t *)
{
    // dummy operator

    return 0;
}

#if 0
CostVal_t Merge_t::Cost(LogProp_t *, LogProp_t *, LogProp_t *outLP)
{
    assert(outLP);

    Card_t outSize_Blocks = outLP->RelSize_Blocks();

    return Config_t::Cost(0, outSize_Blocks);
}
#endif

void Algorithm_t::PrintPlan(int indentLevel) const
{
    for( int j = 0 ; j < indentLevel ; j++ )
        cout << "| ";
    
    cout << "[ALG" << ID() << "]";
    cout << "[CC=" << CallCount() << "]";
    PrintName();
    cout << " Cost = " << CostVal();
    cout << endl;

    int n = NumInputs();

    for( int i = 0 ; i < n ; i++ ) {
        const PlanGroup_t *inpPG = ActualInput(i);
        if( !inpPG ) 
            inpPG = Input(i);
        assert(inpPG);

        Plan_t *inpPlan = inpPG->BestPlan();
        //Ravi: Could not understand the below logic.
        if( !inpPlan )
            inpPlan = inpPG->BestAlgPlan();
        else if( inpPlan->IsFailed() )
            inpPlan = inpPG->BestAlgPlan();
        assert(inpPlan);
        assert(!inpPlan->IsFailed());

        inpPlan->PrintPlan(indentLevel+1);
    }
}

void Enforcer_t::PrintPlan(int indentLevel) const
{
    for( int j = 0 ; j < indentLevel ; j++ )
        cout << "| ";
    
    cout << "[ENF" << ID() << "]";
    cout << "[CC=" << CallCount() << "]";
    PrintName();
    cout << " Cost = " << CostVal();
    cout << endl;

    int n = NumInputs();

    for( int i = 0 ; i < n ; i++ ) {
        const PlanGroup_t *inpPG = ActualInput(i);
        if( !inpPG )
            inpPG = Input(i);
        assert(inpPG);

        Plan_t *inpPlan = inpPG->BestAlgPlan();
        // assert(inpPlan);
        // dirty twisting for VolcanoMQ -- check if its is necessary!
        // VolcanoMQ has been thrown out
        if( !inpPlan )
            inpPlan = inpPG->BestPlan();
        assert(inpPlan);
        if( inpPlan->IsFailed() )
            inpPlan = inpPG->BestPlan();
        assert(inpPlan);
        assert(!inpPlan->IsFailed());

        inpPlan->PrintPlan(indentLevel+1);
    }
}

void Sort_t::PrintName(void) const
{
    assert(sortOrderProp);

    const PlanGroup_t *pg = Input(0);
    assert(pg);

    const Equivalence_t *eq = pg->EqClass();
    assert(eq);

    const LogProp_t *lp = eq->LogicalProp();
    assert(lp);

    const Schema_t *schema = lp->Schema();
    assert(schema);

    if (!partialSortOrderProp)
        cout << "SORT";
    else
        cout << "PARTIAL SORT";

    if (partialSortOrderProp) {
        cout << "( " ;
        partialSortOrderProp->Print(schema);
        cout << "--> "; 
        sortOrderProp->Print(schema);
        cout << ") " ;
    }
    else {
        sortOrderProp->Print(schema);
    }

}

void Index_t::PrintName(void) const
{
    assert(indexOrderProp);

    const PlanGroup_t *pg = Input(0);
    assert(pg);

    const Equivalence_t *eq = pg->EqClass();
    assert(eq);

    const LogProp_t *lp = eq->LogicalProp();
    assert(lp);

    const Schema_t *schema = lp->Schema();
    assert(schema);

    cout << "INDEX";
    indexOrderProp->Print(schema);
}

void IndexSort_t::PrintName(void) const
{
    assert(indexOrderProp);

    const PlanGroup_t *pg = Input(0);
    assert(pg);

    const Equivalence_t *eq = pg->EqClass();
    assert(eq);

    const LogProp_t *lp = eq->LogicalProp();
    assert(lp);

    const Schema_t *schema = lp->Schema();
    assert(schema);

    cout << "INDEXSORT";
    indexOrderProp->Print(schema);
}

void RelScan_t::PrintName(void) const
{
    assert(relName);
    cout << "RELSCAN< " << relName << " >";
}

void AggregateHash_t::PrintName(void) const
{
    cout << "AGGREGATEHASH< ";
    int numAggAttrs = aggAttrMap->NumProj();

    int i = 0;
    for( i = 0 ; i < numAggAttrs ; i++ ) {
        AttrMapEntry_t *aggEntry = aggAttrMap->Entry(i);
        assert(aggEntry);
        Expr_t *e = aggEntry->Expr();
        assert(e);

        e->PrintExpr();
        cout << " ";
    }
    cout << "| ";
    int numGroupByAttrs = groupByAttrMap->NumProj();
    for( i = 0 ; i < numGroupByAttrs ; i++ ) {
        AttrMapEntry_t *groupByEntry = groupByAttrMap->Entry(i);
        assert(groupByEntry);
        Expr_t *e = groupByEntry->Expr();
        assert(e);

        e->PrintExpr();
        cout << " ";
    }
    cout << ">";
}

void AggregateScan_t::PrintName(void) const
{
    cout << "AGGREGATESCAN< ";
    int numAggAttrs = aggAttrMap->NumProj();

    int i = 0;
    for( i = 0 ; i < numAggAttrs ; i++ ) {
        AttrMapEntry_t *aggEntry = aggAttrMap->Entry(i);
        assert(aggEntry);
        Expr_t *e = aggEntry->Expr();
        assert(e);

        e->PrintExpr();
        cout << " ";
    }
    cout << "| ";
    int numGroupByAttrs = groupByAttrMap->NumProj();
    for( i = 0 ; i < numGroupByAttrs ; i++ ) {
        AttrMapEntry_t *groupByEntry = groupByAttrMap->Entry(i);
        assert(groupByEntry);
        Expr_t *e = groupByEntry->Expr();
        assert(e);

        e->PrintExpr();
        cout << " ";
    }
    cout << ">";
}

void ProjectScan_t::PrintName(void) const
{
    cout << "PROJECT< ";
    int numProj = projMap->NumProj();
    for( int i = 0 ; i < numProj ; i++ ) {
        AttrMapEntry_t *entry = projMap->Entry(i);
        assert(entry);
        Expr_t *e = entry->Expr();
        assert(e);

        e->PrintExpr();
        cout << " ";
    }
    cout << ">";
}

void SelectScan_t::PrintName(void) const
{
    cout << "SELECT (state_retain=" << (int) retainState << ")< ";
    Predicate()->PrintExpr();
    cout << " >";
}

void IndexedSelect_t::PrintName(void) const
{
    cout << "INDEXEDSELECT< ";
    Predicate()->PrintExpr();
    cout << " >";
}

void MergeJoin_t::PrintName(void) const
{
    cout << "MERGEJOIN< ";
    Predicate()->PrintExpr();
    cout << " >";
}

void NestedLoopsJoin_t::PrintName(void) const
{
    cout << "NLJOIN< ";
    Predicate()->PrintExpr();
    cout << " >";
}

void IndexedNestedLoopsJoin_t::PrintName(void) const
{
    cout << "INDEXEDNLJOIN< ";
    Predicate()->PrintExpr();
    cout << " >";
}

void Merge_t::PrintName(void) const
{ cout << "UNION"; }

void PhysicalOp_t::PrintSQL(void) const
{
    PlanGroup_t *pg = PlanGroup();
    assert(pg);

    if( pg->IsInProgress() )
        return;
    pg->IncrInProgress();

    int n = NumInputs();

    for( int i = 0 ; i < n ; i++ ) {
        const PlanGroup_t *inpPG = ActualInput(i);
        if( !inpPG ) 
            inpPG = Input(i);
        assert(inpPG);

        Plan_t *inpPlan = inpPG->BestPlan();
        if( !inpPlan )
            inpPlan = inpPG->BestAlgPlan();
        assert(inpPlan);
        if( inpPlan->IsFailed() )
            inpPlan = inpPG->BestAlgPlan();
        assert(inpPlan);
        assert(!inpPlan->IsFailed());

        PhysicalOp_t *inpRoot = inpPlan->Root();
        assert(inpRoot);

        inpRoot->PrintSQL();
    }

    PrintSQLStmt();
}

void Sort_t::PrintSQLStmt(void) const
{
    assert(sortOrderProp);

    const PlanGroup_t *inpPG = Input(0);
    assert(inpPG);

    const Equivalence_t *inpEq = inpPG->EqClass();
    assert(inpEq);

    const LogProp_t *inpLP = inpEq->LogicalProp();
    assert(inpLP);

    const Schema_t *inpSchema = inpLP->Schema();
    assert(inpSchema);

    PlanGroup_t *pg = PlanGroup();
    assert(pg);

    if( pg->IsMarked() ) {
        Equivalence_t *eq = pg->EqClass();
        assert(eq);

        LogProp_t *lp = eq->LogicalProp();
        assert(lp);

        Schema_t *schema = lp->Schema();
        assert(schema);

        cout << "CREATE CLUSTER CLUSTER" << pg->ID() << "(";
        sortOrderProp->PrintSQL(schema);
        cout << ") INDEX";

        cout << "CREATE TABLE TMP" << pg->ID() << "(";
        schema->PrintSQL();
        cout << ") CLUSTER CLUSTER" << pg->ID() << " AS ";
        cout << "SELECT * FROM TMP" << inpPG->ID() << ";" << endl;
    } else {
        cout << "CREATE SYNONYM TMP" << pg->ID() << endl;
        cout << "FOR TMP" << inpPG->ID() << ";" << endl;
    }
}

void Index_t::PrintSQLStmt(void) const
{
    assert(indexOrderProp);

    const PlanGroup_t *inpPG = Input(0);
    assert(inpPG);

    const Equivalence_t *inpEq = inpPG->EqClass();
    assert(inpEq);

    const LogProp_t *inpLP = inpEq->LogicalProp();
    assert(inpLP);

    const Schema_t *inpSchema = inpLP->Schema();
    assert(inpSchema);

    PlanGroup_t *pg = PlanGroup();
    assert(pg);

    cout << "CREATE INDEX INDEX" << pg->ID() << endl;
    cout << "ON TMP" << inpPG->ID() << "(";
    indexOrderProp->PrintSQL(inpSchema);
    cout << ") NOSORT;" << endl;

    cout << "CREATE SYNONYM TMP" << pg->ID() << endl;
    cout << "FOR TMP" << inpPG->ID() << ";" << endl;
}

void IndexSort_t::PrintSQLStmt(void) const
{
    assert(indexOrderProp);

    const PlanGroup_t *inpPG = Input(0);
    assert(inpPG);

    const Equivalence_t *inpEq = inpPG->EqClass();
    assert(inpEq);

    const LogProp_t *inpLP = inpEq->LogicalProp();
    assert(inpLP);

    const Schema_t *inpSchema = inpLP->Schema();
    assert(inpSchema);

    PlanGroup_t *pg = PlanGroup();
    assert(pg);

    cout << "CREATE INDEX INDEX" << pg->ID() << endl;
    cout << "ON TMP" << inpPG->ID() << "(";
    indexOrderProp->PrintSQL(inpSchema);
    cout << ");" << endl;

    cout << "CREATE SYNONYM TMP" << pg->ID() << endl;
    cout << "FOR TMP" << inpPG->ID() << ";" << endl;
}

void RelScan_t::PrintSQLStmt(void) const
{
    assert(relName);

    PlanGroup_t *pg = PlanGroup();
    assert(pg);

    cout << "CREATE SYNONYM TMP" << pg->ID() << endl;
    cout << "FOR " << relName << ";" << endl;
}


void AggregateHash_t::PrintSQLStmt(void) const
{ cout << "NOT IMPLEMENTED" << endl; }

void AggregateScan_t::PrintSQLStmt(void) const
{
    const PlanGroup_t *inpPG = Input(0);
    assert(inpPG);

    PlanGroup_t *pg = PlanGroup();
    assert(pg);

    if( pg->IsMarked() ) {
        Equivalence_t *eq = pg->EqClass();
        assert(eq);

        LogProp_t *lp = eq->LogicalProp();
        assert(lp);

        Schema_t *schema = lp->Schema();
        assert(schema);

        cout << "CREATE TABLE TMP" << pg->ID() << "(";
        schema->PrintSQL();
        cout << ")";
    } else {
        cout << "CREATE VIEW TMP" << pg->ID();
    }

    cout << " AS" << endl;
    cout << "SELECT ";

    int numAggAttrs = aggAttrMap->NumProj();
    int i = 0;
    for( i = 0 ; i < numAggAttrs ; i++ ) {
        AttrMapEntry_t *aggEntry = aggAttrMap->Entry(i);
        assert(aggEntry);
        Expr_t *e = aggEntry->Expr();
        assert(e);
        assert(e->ExprType() == AGG_EXPR_T);
        
        e->PrintSQL();
        AggExpr_t *aggExpr = (AggExpr_t *) e;

        Expr_t *inpExpr = aggExpr->Input();
        assert(inpExpr);
        assert(inpExpr->ExprType() == REL_ARG_REF_T);

        cout << " AS ";
        inpExpr->PrintSQL();
        cout << ", ";
    }

    int numGroupByAttrs = groupByAttrMap->NumProj();
    for( i = 0 ; i < numGroupByAttrs ; i++ ) {
        AttrMapEntry_t *groupByEntry = groupByAttrMap->Entry(i);
        assert(groupByEntry);

        Expr_t *e = groupByEntry->Expr();
        assert(e);
        assert(e->ExprType() == REL_ARG_REF_T);

        e->PrintSQL();

        if( i < numGroupByAttrs-1 )
            cout << ", ";
    }
    cout << endl;

    cout << "FROM TMP" << inpPG->ID()  << endl;
    cout << "GROUP BY "; 

    for( i = 0 ; i < numGroupByAttrs ; i++ ) {
        AttrMapEntry_t *groupByEntry = groupByAttrMap->Entry(i);
        assert(groupByEntry);

        Expr_t *e = groupByEntry->Expr();
        assert(e);
        assert(e->ExprType() == REL_ARG_REF_T);

        e->PrintSQL();

        if( i < numGroupByAttrs-1 )
            cout << ", ";
    }
    cout << ";" << endl;
}

void ProjectScan_t::PrintSQLStmt(void) const
{
    PlanGroup_t *pg = PlanGroup();
    assert(pg);

    if( pg->IsMarked() ) {
        Equivalence_t *eq = pg->EqClass();
        assert(eq);

        LogProp_t *lp = eq->LogicalProp();
        assert(lp);

        Schema_t *schema = lp->Schema();
        assert(schema);

        cout << "CREATE TABLE TMP" << pg->ID() << "(";
        schema->PrintSQL();
        cout << ")";
    } else {
        cout << "CREATE VIEW TMP" << pg->ID();
    }

    cout << " AS SELECT " << endl;
    int numProj = projMap->NumProj();
    for( int i = 0 ; i < numProj ; i++ ) {
        AttrMapEntry_t *entry = projMap->Entry(i);
        assert(entry);
        Expr_t *e = entry->Expr();
        assert(e);

        e->PrintSQL();

        if( i < numProj - 1 )
            cout << ", ";
    }

    const PlanGroup_t *inpPG = Input(0);
    assert(inpPG);

    cout << "FROM TMP" << inpPG->ID() << ";" << endl;
}

void SelectScan_t::PrintSQLStmt(void) const
{
    PlanGroup_t *pg = PlanGroup();
    assert(pg);

    if( pg->IsMarked() ) {
        Equivalence_t *eq = pg->EqClass();
        assert(eq);

        LogProp_t *lp = eq->LogicalProp();
        assert(lp);

        Schema_t *schema = lp->Schema();
        assert(schema);

        cout << "CREATE TABLE TMP" << pg->ID() << "(";
        schema->PrintSQL();
        cout << ")";
    } else {
        cout << "CREATE VIEW TMP" << pg->ID();
    }
    cout << " AS SELECT *";

    const PlanGroup_t *inpPG = Input(0);
    assert(inpPG);

    cout << "FROM TMP" << inpPG->ID() << endl;
    cout << "WHERE ";

    Predicate()->PrintSQL();
    cout << ";" << endl;
}

void IndexedSelect_t::PrintSQLStmt(void) const
{
    PlanGroup_t *pg = PlanGroup();
    assert(pg);

    if( pg->IsMarked() ) {
        Equivalence_t *eq = pg->EqClass();
        assert(eq);

        LogProp_t *lp = eq->LogicalProp();
        assert(lp);

        Schema_t *schema = lp->Schema();
        assert(schema);

        cout << "CREATE TABLE TMP" << pg->ID() << "(";
        schema->PrintSQL();
        cout << ")";
    } else {
        cout << "CREATE VIEW TMP" << pg->ID();
    }

    cout << " AS SELECT *";

    const PlanGroup_t *inpPG = Input(0);
    assert(inpPG);

    cout << "FROM TMP" << inpPG->ID() << endl;
    cout << "WHERE ";

    Predicate()->PrintSQL();
    cout << ";" << endl;
}

void MergeJoin_t::PrintSQLStmt(void) const
{
    PlanGroup_t *pg = PlanGroup();
    assert(pg);

    if( pg->IsMarked() ) {
        Equivalence_t *eq = pg->EqClass();
        assert(eq);

        LogProp_t *lp = eq->LogicalProp();
        assert(lp);

        Schema_t *schema = lp->Schema();
        assert(schema);

        cout << "CREATE TABLE TMP" << pg->ID() << "(";
        schema->PrintSQL();
        cout << ")";
    } else {
        cout << "CREATE VIEW TMP" << pg->ID();
    }
    cout << " AS SELECT *";

    const PlanGroup_t *leftInpPG = Input(0);
    assert(leftInpPG);
    const PlanGroup_t *rightInpPG = Input(1);
    assert(rightInpPG);

    cout << "FROM TMP" << leftInpPG->ID() <<
                        ", TMP" << rightInpPG->ID() << endl;
    cout << "WHERE ";

    Predicate()->PrintSQL();
    cout << ";" << endl;
}

void NestedLoopsJoin_t::PrintSQLStmt(void) const
{
    PlanGroup_t *pg = PlanGroup();
    assert(pg);

    if( pg->IsMarked() ) {
        Equivalence_t *eq = pg->EqClass();
        assert(eq);

        LogProp_t *lp = eq->LogicalProp();
        assert(lp);

        Schema_t *schema = lp->Schema();
        assert(schema);

        cout << "CREATE TABLE TMP" << pg->ID() << "(";
        schema->PrintSQL();
        cout << ")";
    } else {
        cout << "CREATE VIEW TMP" << pg->ID();
    }
    cout << " AS SELECT *";

    const PlanGroup_t *leftInpPG = Input(0);
    assert(leftInpPG);
    const PlanGroup_t *rightInpPG = Input(1);
    assert(rightInpPG);

    cout << "FROM TMP" << leftInpPG->ID() <<
                        ", TMP" << rightInpPG->ID() << endl;
    cout << "WHERE ";

    Predicate()->PrintSQL();
    cout << ";" << endl;
}

void IndexedNestedLoopsJoin_t::PrintSQLStmt(void) const
{
    PlanGroup_t *pg = PlanGroup();
    assert(pg);

    if( pg->IsMarked() ) {
        Equivalence_t *eq = pg->EqClass();
        assert(eq);

        LogProp_t *lp = eq->LogicalProp();
        assert(lp);

        Schema_t *schema = lp->Schema();
        assert(schema);

        cout << "CREATE TABLE TMP" << pg->ID() << "(";
        schema->PrintSQL();
        cout << ")";
    } else {
        cout << "CREATE VIEW TMP" << pg->ID();
    }
    cout << " AS SELECT * ";

    const PlanGroup_t *leftInpPG = Input(0);
    assert(leftInpPG);
    const PlanGroup_t *rightInpPG = Input(1);
    assert(rightInpPG);
    cout << "FROM TMP" << leftInpPG->ID() <<
                        ", TMP" << rightInpPG->ID() << endl;
    cout << "WHERE ";

    Predicate()->PrintSQL();
    cout << ";" << endl;
}

void Merge_t::PrintSQLStmt(void) const
{
    PlanGroup_t *pg = PlanGroup();
    assert(pg);

    cout << "SELECT * ";
    const PlanGroup_t *leftInpPG = Input(0);
    assert(leftInpPG);
    cout << "FROM TMP" << leftInpPG->ID() << ";" << endl;

    cout << "SELECT * ";
    const PlanGroup_t *rightInpPG = Input(1);
    assert(rightInpPG);
    cout << "FROM TMP" << rightInpPG->ID() << ";" << endl;
}


CostVal_t BasicApplyOp_t::Cost(LogProp_t *leftLP, LogProp_t *rightLP,
                                                        LogProp_t *outLP)
{
    assert(leftLP);
    assert(rightLP);
    assert(outLP);

    Card_t leftSize_Tuples = leftLP->RelSize_Tuples();
    Card_t rightSize_Tuples = rightLP->RelSize_Tuples();
    Card_t leftSize_Blocks = leftLP->RelSize_Blocks();
    Card_t rightSize_Blocks = rightLP->RelSize_Blocks();
    Card_t outSize_Blocks = outLP->RelSize_Blocks();
    
    // Ravi: The following should be changed to take care 
    // of ops other than LeftSemiJoin.
    return LeftSemiJoin_t::Cost(leftLP, rightLP) + 
           Config_t::Cost(0, outSize_Blocks); 

    /*** Ignore the below. Earlier taken from NLJ ****/

    // Note: We are assuming input is pipelined -- a relscan below
    //       ensures this if select is on a db relation, and accounts
    //       for the read cost

    if (leftSize_Blocks <= NumBuffers*0.5 )
        return Config_t::Cost(0, rightSize_Tuples*leftSize_Blocks +
                                 outSize_Blocks); 
    if ( rightSize_Blocks <= NumBuffers*0.5)
        return Config_t::Cost(0, leftSize_Tuples*rightSize_Blocks +
                                 outSize_Blocks); 

    // Otherwise do a block nested loops join

    assert(NumBuffers > 1);
    Card_t io_Blocks = (leftSize_Blocks*rightSize_Blocks)/(NumBuffers-1);
    Card_t io_cost = io_Blocks*ReadTime;

    return Config_t::Cost(io_cost, ceil(leftSize_Tuples * rightSize_Blocks 
                                     + outSize_Blocks));
}

void BasicApplyOp_t::PrintSQLStmt(void) const
{
    PlanGroup_t *pg = PlanGroup();
    assert(pg);

    if( pg->IsMarked() ) {
        Equivalence_t *eq = pg->EqClass();
        assert(eq);

        LogProp_t *lp = eq->LogicalProp();
        assert(lp);

        Schema_t *schema = lp->Schema();
        assert(schema);

        cout << "CREATE TABLE TMP" << pg->ID() << "(";
        schema->PrintSQL();
        cout << ")";
    } else {
        cout << "CREATE VIEW TMP" << pg->ID();
    }
    cout << " AS SELECT *";

    const PlanGroup_t *leftInpPG = Input(0);
    assert(leftInpPG);
    const PlanGroup_t *rightInpPG = Input(1);
    assert(rightInpPG);

    cout << "FROM TMP" << leftInpPG->ID() <<
                        ", TMP" << rightInpPG->ID() << endl;
    cout << "WHERE ";

    Predicate()->PrintSQL();
    cout << ";" << endl;
}

void BasicApplyOp_t::PrintName(void) const
{
    cout << "BASICAPPLY< ";
    logOp->Op()->PrintName();
    // Predicate()->PrintExpr();
    cout << " >";
}



    
void RelScan_t::open(){
    //cout<<"In RelScan_t OPEN "<<relName<<endl;
    //RIterator* iter = this->Iter();
    iter = new BaseRelIterator("mydb.txt", relName);
    if(iter)
        iter->open();
    //cout<<"Leaving RelScan OPEN\n";
}

Record* RelScan_t::next(){
    //cout<<"In RelScan_t NEXT\n";
    if(iter)
        return iter->next();
    else {//cout<<"relscan NULL\n";
        return NULL;}
}

void RelScan_t::close(){
    //cout<<"In RelScan_t CLOSE\n";
    if(iter)
        iter->close();
    //cout<<"Leaving RelScan CLOSE\n";
}


    
void NestedLoopsJoin_t::open(){
    //cout<<"In NestedLoopsJoin_t OPEN\n";

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

    if(lPhyOp)
        lPhyOp->open();
    
    //cout<<"Leaving NLJoin_t OPEN\n";

}

Record* NestedLoopsJoin_t::next(){
    //cout<<"In NestedLoopsJoin_t NEXT\n";
    if(!(lPhyOp) || !(rPhyOp))
        return NULL;
        
    if(newOuterRec){
        outerRec = lPhyOp->next();
        newOuterRec = 0;
        if(rPhyOp)
            rPhyOp->open();
    }
    
    if(!outerRec)
    {return NULL;}
    
    Record* innerRec = rPhyOp->next();
    
    if(!innerRec){//cout<<"   innner rec null   ";
        newOuterRec = 1;
        if(rPhyOp)
            rPhyOp->close();
        this->next();
    }
    else{
        //Join records
        //cout<<"joining: ";outerRec->print();cout<<" and ";innerRec->print();cout<<endl;
        Record* newRec = NULL;
        newRec = outerRec->joinRecords(innerRec, newRec);
        //cout<<"Joined Rec:"; newRec->print();cout<<endl;
        return newRec;
    }
    //cout<<"leaving NLjoin Next"<<endl;
}

void NestedLoopsJoin_t::close(){
    //cout<<"In NestedLoopsJoin_t CLOSE\n";
    if(lPhyOp)
        lPhyOp->close();
    
    //cout<<"\nLeaving NLJoin CLOSE";
}


    
void ProjectScan_t::open(){
    //cout<<"In ProjectScan_t OPEN\n";
    PlanGroup_t *inpPG = this->Input(0);
    if(!inpPG)
        //cout<<"inpPG NULL\n";
    assert(inpPG);
    
    Plan_t *plan = inpPG->BestPlan();
    if( !plan ){
        //cout<<"BestPlan NULL.Assigning BestAlgPlan\n";
        plan = inpPG->BestAlgPlan();
    }
    else if( plan->IsFailed() ){
        //cout<<"BestApgPlan Failed.Assiging BestAlgPlan\n";
        plan = inpPG->BestAlgPlan();
    }
    assert(plan);
    assert(!plan->IsFailed());
    
    phyOp = plan->Root();
        
    if(phyOp)
        phyOp->open();
}

Record* ProjectScan_t::next(){
    //cout<<"In ProjectScan_t NEXT\n";
    if(!phyOp)
        return NULL;
    Record* rec = phyOp->next();
    if(!rec)
        return NULL;
    Schema* sch = rec->getSchema();
    int numProj = projMap->NumProj();
    
    if(!attrPos){
        //Fill attrPos array
        attrPos = new int[numProj];
        Expr_t *e;
        AttrMapEntry_t *entry;
        for( int i = 0 ; i < numProj ; i++ ) {
            entry = projMap->Entry(i);
            assert(entry);
            e = entry->Expr();
            assert(e);
            const char* name = e->AttrName();
        
            //if(!name)cout<<"NAME NULL!!!\n";
            //cout<<"COLUMN Name: "<<name<<endl;
            attrPos[i] = sch->getAttrIndex(name);
        }
    }
    
    string* recStr, *newStr;
    recStr = rec->getStringArray(recStr);
    
    short int attrType[numProj];
    char *attrName[numProj];
    short int sizeAttrs[numProj];
    Schema* newSchema;
    Record* newRec;
    
    newStr = new string[numProj];
    
    for(int i=0; i<numProj; i++){
        newStr[i] = recStr[attrPos[i]];
        attrType[i] = sch->getAttrType(attrPos[i]);
        attrName[i] = sch->getAttrName(attrPos[i]);
        sizeAttrs[i] = sch->getSizeAttrs(attrPos[i]);
    }
    newSchema = new Schema(numProj, attrType, attrName, sizeAttrs);
    
    newRec = new Record(newStr, newSchema, numProj);
    
    return newRec;
}

void ProjectScan_t::close(){
    //cout<<"In ProjectScan_t CLOSE\n";
    if(phyOp)
        phyOp->close();
}







