// physical property methods

#include <assert.h>
#include "PhyPropGroup.h"
#include "PhysicalOp.h"
#include "Equivalence.h"
#include "Volcano.h"
#include "PhyProp.h"

int SortOrder_t::IsCovered(const SchemaProp_t *schemaProp,
                                        const PhyProp_t *p) const
{
    assert(schemaProp);
    assert(p);

    if( p->Type() != SORTORDER_T && p->Type() != INDEXORDER_T )
        return 0;

    const Order_t *ps = (const Order_t *) p;

    int pOrderSize = ps->OrderSize();

    if( orderSize > pOrderSize )
        return 0;

    const int *pOrder = ps->Order();

    for( int i = 0 ; i < orderSize ; i++ ) {
        if( schemaProp->AttrPropRep(order[i]) !=
                            schemaProp->AttrPropRep(pOrder[i]) )
            return 0;
    }

    return 1;
}

// sort enforcer move
void SortOrder_t::Enforce(PlanGroup_t *planGroup, Volcano_t *opt,
                    Cost_t *totalCost, Plan_t *bestPlan, CostVal_t *costLimit,
            Order_t *pso, int *plmap, int level, Card_t callCount)
{
    assert(planGroup);
    assert(opt);
    assert(bestPlan);
    assert(totalCost);

    Equivalence_t *eq = planGroup->EqClass();
    assert(eq);

    // get the logical property of the input/output
    LogProp_t *inpLP = eq->LogicalProp();
    assert(inpLP);

    CostVal_t sortCost = callCount * Sort_t::Cost(this, inpLP);

    CostVal_t inpCostLimit = -1;
    
    if( costLimit ) {
        // pruning enabled
        // calculate the costlimit
        inpCostLimit = *costLimit - sortCost;

        // if not feasible, return
        if( inpCostLimit < totalCost->Value() ) 
            return;
    }

    // schema property of the input/output
    SchemaProp_t *inpSchemaProp = inpLP->SchemaProp();
    assert(inpSchemaProp);

    // modify the required physical properties
    PhyPropGroup_t *inpReqProp = new PhyPropGroup_t;
    Refer(inpReqProp);

    // properties required in the current plan
    PhyPropGroup_t *reqProp = planGroup->PropGroup();
    assert(reqProp);

    const List_t<PhyProp_t *>& reqPropList = reqProp->PropList();

    // populate inpReqProp
    // add all properties in reqProp except those covered by the one
    // being enforced
    ListIter_t<PhyProp_t *> reqIter;
    reqIter.Attach(&reqPropList);

    while( !reqIter.IsEnd() ) {
        PhyProp_t *prop = reqIter.Next();

        assert(prop);
        if( prop->IsCovered(inpSchemaProp, this) )
            continue;

        inpReqProp->AddProperty(prop);
    }

    // now optimize the input --- we only need to look at algorithm
    // plans, assuming the triangle property of the enforcers
    PlanGroup_t *inpPlanGroup = NULL;
    if( costLimit ) {
        // pruning enabled
        inpPlanGroup = opt->FindBestAlgPlan(eq, inpReqProp,
                                        totalCost, &inpCostLimit,
                    0, pso, plmap, level, callCount);
    } else {
        inpPlanGroup = opt->FindBestAlgPlan(eq, inpReqProp, totalCost, NULL,
                        0, pso, plmap, level, callCount);
        assert(inpPlanGroup);
    }

    DeRefer(inpReqProp);

    // if no feasible plan found, return
    if( !inpPlanGroup )
        return;

    // a feasible best plan found for the input
    Plan_t *inpBestPlan = inpPlanGroup->BestAlgPlan();
    assert(inpBestPlan);
    assert(!inpBestPlan->IsFailed());

    // input plan cost
    Cost_t *inpCost = inpBestPlan->Cost();
    if( opt->IsRU() && inpPlanGroup->IsMarked() &&
            inpPlanGroup->UseCost() < inpBestPlan->CostVal() )
        inpCost = inpPlanGroup->UseCostADT(opt);

    assert(inpCost);

    // now instantiate the sort operator and set its input as the
    // plan obtained as above to get the required plan
    Sort_t *sort = new Sort_t(this);
    sort->SetCallCount(callCount);
    sort->SetInput(0, inpPlanGroup);
    sort->SetCostVal(sortCost);

    // calculate the cost
    Cost_t *cost = opt->CreateCost(sort, sortCost);
    Refer(cost);
    cost->Add(inpCost);

    // add the plan to the plan group
    planGroup->AddEnfPlan(sort, cost);

    // current total plan cost
    CostVal_t curCost = totalCost->SumCost(cost);

    // update the best plan if found plan better
    int isBetterPlan = 1;

    // if pruning enabled, we know that we have arrived at a better
    // plan, so a check is not necessary
    if( !costLimit ) {
        Cost_t *prevCost = bestPlan->Cost();
        if( prevCost ) {
            CostVal_t prevBestCost = totalCost->SumCost(prevCost);
            if( curCost >= prevBestCost )
                isBetterPlan = 0;
        }
    }
    
    if( isBetterPlan ) {
        bestPlan->SetRoot(sort);
        bestPlan->SetCost(cost);

        if( Config_t::IsDebug(3) ) {
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

// translate this physical prop from src LP to dest LP
PhyProp_t *SortOrder_t::Translate(LogProp_t *srcLP, LogProp_t *destLP, 
                                  int translateMode)
{
    assert(srcLP);
    assert(destLP);

    Schema_t *srcSchema = srcLP->Schema();
    assert(srcSchema);

    Schema_t *destSchema = destLP->Schema();
    assert(destSchema);

    int destNumAttr = destSchema->NumAttr();

    int *destOrder = new int[destNumAttr];
    int destOrderSize = 0;

    for( int i = 0 ; i < orderSize ; i++ ) {
        int srcAttrIndex = order[i];

        Attr_t *srcAttr = srcSchema->Attribute(srcAttrIndex);
        assert(srcAttr);

        int attrFound = 0;
        for( int j = 0 ; j < destNumAttr ; j++ ) {
            Attr_t *destAttr = destSchema->Attribute(j);
            assert(destAttr);

            if( destAttr->IsEquivalent(srcAttr) ) {
                assert(destOrderSize < destNumAttr);
                destOrder[destOrderSize++] = j;
                attrFound = 1;

                break;
            }
        }

        if( !attrFound ) {
            if(translateMode == TRANSLATE_EXACT_ONLY) {
                delete[] destOrder;
                return NULL;
            }
            if(translateMode == TRANSLATE_PREFIX_ONLY) {
                break;
            }
        }
    }

    // the destination sort property
    SortOrder_t *dest = new SortOrder_t(destOrder, destOrderSize);

    delete[] destOrder;

    return dest;
}

SortOrder_t *SortOrder_t::LongestCommonPrefix(const Set_t<int> *attrSet)
{

    const int *attrs = attrSet->Elements();
    int attrSetSize  = attrSet->Size();

    int *newOrd = new int[orderSize];
    int count  = 0;

    for (int i = 0; i < orderSize; i++) {
        bool found = false;
        for (int j = 0; j < attrSetSize; j++) {
            if(order[i] == attrs[j]) {
                newOrd[count++] = order[i];
                found = true;
                break;
             }
        }
        if(!found)
            break;
    }

    if (count > 0)
        return new SortOrder_t(newOrd, count);
    else
        return NULL;
}

SortOrder_t *SortOrder_t::LongestCommonPrefix(const Order_t *ord)
{
    const int *ordArray = ord->Order();
    int  ordSize  = ord->OrderSize();
    int  newOrdMaxSize = orderSize < ordSize? orderSize : ordSize;
    int  *newOrd   = new int[newOrdMaxSize];

    int i;
    for (i = 0; i < newOrdMaxSize; i++) {
        if (order[i] == ordArray[i]) {
            newOrd[i] = order[i];
        }
        else {
            break;
        }
     }

     if (i > 0)
        return new SortOrder_t(newOrd, i);
    else
        return NULL;
}

SortOrder_t *SortOrder_t::Concat(const Order_t *ord)
{
    int  newOrdSize = orderSize + ord->OrderSize();
    int *newOrd = new int[newOrdSize];

    for (int i = 0; i < orderSize; i++) {
        newOrd[i] = order[i];
    }
    for (int i = 0; i < ord->OrderSize(); i++) {
        newOrd[orderSize + i] = ord->Order()[i];
    }
   
    return new SortOrder_t(newOrd, newOrdSize);
}

// The order is assumed to be in canonical form w.r.t srcLP.
// Translates the order w.r.t. dstLP and returns in canonical form.
SortOrder_t *SortOrder_t::TranslateFromCanonical(LogProp_t *srcLP, 
                                                 LogProp_t *dstLP)
{
    assert(srcLP);
    assert(dstLP);

    SchemaProp_t *srcSP = srcLP->SchemaProp();
    SchemaProp_t *dstSP = dstLP->SchemaProp();
    Schema_t *srcSchema = srcLP->Schema();
    Schema_t *dstSchema = dstLP->Schema();
    int numAttr = srcSP->NumAttr();
    int *resOrdArray = new int[orderSize];

    for (int i = 0; i < orderSize; i++) {
        resOrdArray[i] = -1;
        int rep = srcSP->AttrPropRep(order[i]);
        for (int j = 0; j < numAttr; j++) {
            if (srcSP->AttrPropRep(j) == rep) {
                Attr_t *srcAttr = srcSchema->Attribute(j);
                int dstBnd = dstSchema->AttrBinding(srcAttr);
                if (dstBnd != -1) {
                    resOrdArray[i] = dstSP->AttrPropRep(dstBnd);
                    break;
                }
            }
        }
        assert(resOrdArray[i] != -1);
    }

    SortOrder_t *res = new SortOrder_t(resOrdArray, orderSize);
    delete[] resOrdArray;
    return res;
}


int IndexOrder_t::IsCovered(const SchemaProp_t *schemaProp,
                                        const PhyProp_t *p) const
{
    assert(schemaProp);
    assert(p);

    if( p->Type() != INDEXORDER_T )
        return 0;

    const IndexOrder_t *ps = (const IndexOrder_t *) p;

    int pOrderSize = ps->OrderSize();

    if( orderSize > pOrderSize )
        return 0;

    const int *pOrder = ps->Order();

    for( int i = 0 ; i < orderSize ; i++ ) {
        if( schemaProp->AttrPropRep(order[i]) !=
                            schemaProp->AttrPropRep(pOrder[i]) )
            return 0;
    }

    return 1;
}

// index enforcer move
void IndexOrder_t::Enforce(PlanGroup_t *planGroup, Volcano_t *opt,
                    Cost_t *totalCost, Plan_t *bestPlan, CostVal_t *costLimit,
            Order_t *pso, int *plmap, int level, Card_t callCount)
{
    assert(planGroup);
    assert(opt);
    assert(bestPlan);
    assert(totalCost);

    EnforceIndex(planGroup, opt, totalCost, bestPlan, costLimit, 
                 pso, plmap, level, callCount);
    EnforceIndexSort(planGroup, opt, totalCost, bestPlan, costLimit,
                     pso, plmap, level, callCount);
}

// index enforcer move
void IndexOrder_t::EnforceIndex(PlanGroup_t *planGroup, Volcano_t *opt,
                    Cost_t *totalCost, Plan_t *bestPlan, CostVal_t *costLimit,
            Order_t *pso, int *plmap, int level, Card_t callCount)
{
    assert(planGroup);
    assert(opt);
    assert(bestPlan);
    assert(totalCost);

    Equivalence_t *eq = planGroup->EqClass();
    assert(eq);

    // get the logical property of the input/output
    LogProp_t *inpLP = eq->LogicalProp();
    assert(inpLP);

    // schema property of the input/output
    SchemaProp_t *inpSchemaProp = inpLP->SchemaProp();
    assert(inpSchemaProp);

    CostVal_t indexCost = callCount * Index_t::Cost(inpLP);
    CostVal_t inpCostLimit = -1;
    
    if( costLimit ) {
        // pruning enabled
        // calculate the costlimit
        inpCostLimit = *costLimit - indexCost;

        // if not feasible, return
        if( inpCostLimit < totalCost->Value() ) 
            return;
    }

    // modify the required physical properties
    PhyPropGroup_t *inpReqProp = new PhyPropGroup_t;
    Refer(inpReqProp);

    // properties required in the current plan
    PhyPropGroup_t *reqProp = planGroup->PropGroup();
    assert(reqProp);

    const List_t<PhyProp_t *>& reqPropList = reqProp->PropList();

    // populate inpReqProp
    // add all properties in reqProp except those covered by the one
    // being enforced
    ListIter_t<PhyProp_t *> reqIter;
    reqIter.Attach(&reqPropList);

    while( !reqIter.IsEnd() ) {
        PhyProp_t *prop = reqIter.Next();

        assert(prop);
        if( prop->IsCovered(inpSchemaProp, this) )
            continue;

        inpReqProp->AddProperty(prop);
    }

    // input should be sorted on the index order
    SortOrder_t *inpSortOrder = new SortOrder_t(order, orderSize);
    Refer(inpSortOrder);

    inpReqProp->AddProperty(inpSortOrder);
    DeRefer(inpSortOrder);

    // now optimize the input
    PlanGroup_t *inpPlanGroup = NULL;
    if( costLimit ) {
        // pruning enabled
        inpPlanGroup = opt->FindBestPlan(eq, inpReqProp,
                                        totalCost, &inpCostLimit,
                    0, pso, plmap, level, callCount);
    } else {
        inpPlanGroup = opt->FindBestPlan(eq, inpReqProp, totalCost, NULL,
                        0, pso, plmap, level, callCount);
        assert(inpPlanGroup);
    }

    DeRefer(inpReqProp);

    // if no feasible plan found, return
    if( !inpPlanGroup )
        return;

    // a feasible best plan found for the input
    Plan_t *inpBestPlan = inpPlanGroup->BestPlan();
    assert(inpBestPlan);
    assert(!inpBestPlan->IsFailed());

    // input plan cost
    Cost_t *inpCost = inpBestPlan->Cost();

    if( opt->IsRU() && inpPlanGroup->IsMarked() &&
            inpPlanGroup->UseCost() < inpBestPlan->CostVal() )
        inpCost = inpPlanGroup->UseCostADT(opt);

    assert(inpCost);

    // now instantiate the index operator and set its input as the
    // plan obtained as above to get the required plan
    Index_t *index = new Index_t(this);
    index->SetCallCount(callCount);
    index->SetInput(0, inpPlanGroup);
    index->SetCostVal(indexCost);

    // calculate the cost
    Cost_t *cost = opt->CreateCost(index, indexCost);
    Refer(cost);
    cost->Add(inpCost);

    // add the plan to the plan group
    planGroup->AddEnfPlan(index, cost);

    // current total plan cost
    CostVal_t curCost = totalCost->SumCost(cost);

    // update the best plan if found plan better
    int isBetterPlan = 1;

    // if pruning enabled, we know that we have arrived at a better
    // plan, so a check is not necessary
    if( !costLimit ) {
        Cost_t *prevCost = bestPlan->Cost();
        if( prevCost ) {
            CostVal_t prevBestCost = totalCost->SumCost(prevCost);
            if( curCost >= prevBestCost )
                isBetterPlan = 0;
        }
    }
    
    if( isBetterPlan ) {
        bestPlan->SetRoot(index);
        bestPlan->SetCost(cost);

        if( Config_t::IsDebug(3) ) {
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

// index sort enforcer move
void IndexOrder_t::EnforceIndexSort(PlanGroup_t *planGroup, Volcano_t *opt,
                    Cost_t *totalCost, Plan_t *bestPlan, CostVal_t *costLimit,
            Order_t *pso, int *plmap, int level, Card_t callCount)
{
    assert(planGroup);
    assert(opt);
    assert(bestPlan);
    assert(totalCost);

    Equivalence_t *eq = planGroup->EqClass();
    assert(eq);

    // get the logical property of the input/output
    LogProp_t *inpLP = eq->LogicalProp();
    assert(inpLP);

    // schema property of the input/output
    SchemaProp_t *inpSchemaProp = inpLP->SchemaProp();
    assert(inpSchemaProp);

    CostVal_t indexCost = callCount * IndexSort_t::Cost(this, inpLP);
    CostVal_t inpCostLimit = -1;
    
    if( costLimit ) {
        // pruning enabled
        // calculate the costlimit
        inpCostLimit = *costLimit - indexCost;

        // if not feasible, return
        if( inpCostLimit < totalCost->Value() ) 
            return;
    }

    // modify the required physical properties
    PhyPropGroup_t *inpReqProp = new PhyPropGroup_t;
    Refer(inpReqProp);

    // properties required in the current plan
    PhyPropGroup_t *reqProp = planGroup->PropGroup();
    assert(reqProp);

    const List_t<PhyProp_t *>& reqPropList = reqProp->PropList();

    // populate inpReqProp
    // add all properties in reqProp except those covered by the one
    // being enforced
    ListIter_t<PhyProp_t *> reqIter;
    reqIter.Attach(&reqPropList);

    while( !reqIter.IsEnd() ) {
        PhyProp_t *prop = reqIter.Next();

        assert(prop);
        if( prop->IsCovered(inpSchemaProp, this) )
            continue;

        inpReqProp->AddProperty(prop);
    }

    // now optimize the input
    PlanGroup_t *inpPlanGroup = NULL;
    if( costLimit ) {
        // pruning enabled
        inpPlanGroup = opt->FindBestPlan(eq, inpReqProp,
                                        totalCost, &inpCostLimit,
                    0, pso, plmap, level, callCount);
    } else {
        inpPlanGroup = opt->FindBestPlan(eq, inpReqProp, totalCost, NULL,
                        0, pso, plmap, level, callCount);
        assert(inpPlanGroup);
    }

    DeRefer(inpReqProp);

    // if no feasible plan found, return
    if( !inpPlanGroup )
        return;

    // a feasible best plan found for the input
    Plan_t *inpBestPlan = inpPlanGroup->BestPlan();
    assert(inpBestPlan);
    assert(!inpBestPlan->IsFailed());

    // input plan cost
    Cost_t *inpCost = inpBestPlan->Cost();

    if( opt->IsRU() && inpPlanGroup->IsMarked() &&
            inpPlanGroup->UseCost() < inpBestPlan->CostVal() )
        inpCost = inpPlanGroup->UseCostADT(opt);

    assert(inpCost);

    // now instantiate the index operator and set its input as the
    // plan obtained as above to get the required plan
    IndexSort_t *index = new IndexSort_t(this);
    index->SetCallCount(callCount);
    index->SetInput(0, inpPlanGroup);
    index->SetCostVal(indexCost);

    // calculate the cost
    Cost_t *cost = opt->CreateCost(index, indexCost);
    Refer(cost);
    cost->Add(inpCost);

    // add the plan to the plan group
    planGroup->AddEnfPlan(index, cost);

    // current total plan cost
    CostVal_t curCost = totalCost->SumCost(cost);

    // update the best plan if found plan better
    int isBetterPlan = 1;

    // if pruning enabled, we know that we have arrived at a better
    // plan, so a check is not necessary
    if( !costLimit ) {
        Cost_t *prevCost = bestPlan->Cost();
        if( prevCost ) {
            CostVal_t prevBestCost = totalCost->SumCost(prevCost);
            if( curCost >= prevBestCost )
                isBetterPlan = 0;
        }
    }
    
    if( isBetterPlan ) {
        bestPlan->SetRoot(index);
        bestPlan->SetCost(cost);

        if( Config_t::IsDebug(3) ) {
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


/**
 * Translate a set of orders from source schema to the destination schema
 * This method is used for getting ancestor orders and local orders. 
 */
List_t<Order_t *>* TranslateOrders(List_t<Order_t *> *ords, LogProp_t *srcLP, 
                                   LogProp_t *destLP, int translateMode)
{
    assert(ords);
    assert(srcLP);
    assert(destLP);

    List_t<Order_t *>* newOrds = new AppendList_t<Order_t *>();

    ListIter_t<Order_t *> ordIter;
    ordIter.Attach(ords);

    while(!ordIter.IsEnd()) {
        Order_t *ord = ordIter.Next();
        assert(ord);
        
        Order_t *destOrder = (Order_t *) ord->Translate(srcLP, destLP, 
                                              translateMode);
        newOrds->Insert(destOrder);
    }
    return newOrds;
}


void PrintOrders(List_t<Order_t *> *ords, Schema_t *schema)
{
    ListIter_t<Order_t *> ordIter;
    ordIter.Attach(ords);

    while(!ordIter.IsEnd()) {
        Order_t *ord = ordIter.Next();
        assert(ord);
        ord->Print(schema);
    }
}


/**
 * Add the orders from src to dest if not already present. 
 * logProp: log prop having the schema for both dest and src.
 */
void UnionOrders(List_t<Order_t *> *dest, List_t<Order_t *> *src, LogProp_t *logProp)
{
    ListIter_t<Order_t *> srcOrdIter;
    srcOrdIter.Attach(src);

    ListIter_t<Order_t *> destOrdIter;
    destOrdIter.Attach(dest);

    while(!srcOrdIter.IsEnd()) {
        Order_t *srcOrd = srcOrdIter.Next();
        destOrdIter.Reset();
        int found = 0;
        while(!destOrdIter.IsEnd()) {
            Order_t *destOrd = destOrdIter.Next();
            if (srcOrd->IsEquivalent(logProp->SchemaProp(), destOrd)) {
                found = 1;
                break;
            }
        }
        if (!found) {
            dest->Insert(srcOrd);
        }
    }
}


