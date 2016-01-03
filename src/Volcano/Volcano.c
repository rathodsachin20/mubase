// Volcano optimizer

#include <assert.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <math.h>

#include "Config.h"
#include "Memo.h"
#include "PhyPropGroup.h"
#include "Plan.h"
#include "Equivalence.h"
#include "LogicalOp.h"
#include "PhysicalOp.h"
#include "TransRule.h"
#include "Volcano.h"
#include "Interface.h"
#include "Map.h"

const int MAXGROUPBYATTRS = 20;

// expand the dag rooted at rootEq without pruning
// returns the plan group of the root eq node corresponding to no
// physical property
// Returns NULL if a valid best plan exists but does not follow the
// above constraints or if the plan is "failed" due to b&b pruning.
PlanGroup_t *Volcano_t::ExpressionDAG(Equivalence_t *rootEq, CostVal_t costUB)
{
    assert(rootEq);

    struct rusage rusageBeg, rusageEnd;

    if( Config_t::PrintDAGExpansionTime() )
        getrusage(RUSAGE_SELF, &rusageBeg);

    if( isFullExpand ) {
        // expand the dag by applying all possible transformations
        ExpandDAG(rootEq);

        rootEq->PrintExpressionDAG(0);

        if( Config_t::PrintDAGExpansionTime() ) {
            getrusage(RUSAGE_SELF, &rusageEnd);

            int utimetaken =
                (rusageEnd.ru_utime.tv_sec-rusageBeg.ru_utime.tv_sec)*1000 +
                (rusageEnd.ru_utime.tv_usec-rusageBeg.ru_utime.tv_usec)/1000;
            int stimetaken =
                (rusageEnd.ru_stime.tv_sec-rusageBeg.ru_stime.tv_sec)*1000 +
                (rusageEnd.ru_stime.tv_usec-rusageBeg.ru_stime.tv_usec)/1000;

            cout << "LDAGUSR " << utimetaken << endl;
            cout << "LDAGSYS " << stimetaken << endl;
        }

        if( Config_t::IsPruning() )
            PruneDAG(rootEq, costUB);

        if( Config_t::IsSubsumption() ) {
            // apply logical subsumption
            ExpandSubsumption(rootEq);
        }
    }

    // required physical properties kept empty
    PhyPropGroup_t *reqProp = new PhyPropGroup_t;
    Refer(reqProp);

    // initial previously computed plan cost
    Cost_t *initTotalCost = CreateCost();

    // physical dag to be expanded fully --- so costLimit passed as NULL
    PlanGroup_t *pg = FindBestPlan(rootEq, reqProp, initTotalCost, NULL);

    if( Config_t::PrintDAGExpansionTime() ) {
        getrusage(RUSAGE_SELF, &rusageEnd);

        int utimetaken =
                (rusageEnd.ru_utime.tv_sec-rusageBeg.ru_utime.tv_sec)*1000 +
                (rusageEnd.ru_utime.tv_usec-rusageBeg.ru_utime.tv_usec)/1000;
        int stimetaken =
                (rusageEnd.ru_stime.tv_sec-rusageBeg.ru_stime.tv_sec)*1000 +
                (rusageEnd.ru_stime.tv_usec-rusageBeg.ru_stime.tv_usec)/1000;

        cout << "PDAGUSR " << utimetaken << endl;
        cout << "PDAGSYS " << stimetaken << endl;
    }

    delete initTotalCost;

    DeRefer(reqProp);

    return pg;
}

// Find best plan for given equivalence class rootEq.
// The cost of the plan must not exceed costLimit (if not NULL)
// no pruning if costLimit is NULL
// Returns NULL if a valid best plan exists but does not follow the
// above constraints or if the plan is "failed" due to pruning.
// Ravi: is costLimit updated?   How? 
PlanGroup_t *Volcano_t::FindBestPlan(Equivalence_t *rootEq,
                                     CostVal_t *costLimit,
                                     SortOrder_t *reqOrder)
{
    assert(rootEq);

    /*
    // Ravi: Begin temporary code - for testing sort cost model

    LogProp_t *templp = rootEq->LogicalProp();
    int outOrd[4]; int inOrd[4];
    
    outOrd[0] = 0; outOrd[1] = 1; outOrd[2] = 2; outOrd[3] = 3;
    inOrd[0]  = 0; inOrd[1]  = 1;

    Order_t *ord  = new SortOrder_t(outOrd, 4);
    Order_t *partialOrd  = new SortOrder_t(inOrd, 2);

    // Order_t *partialOrd  = NULL;
    cout << "Sort Cost Model Test: " << endl;
    cout << "Input Order : " << endl;
    if (partialOrd)
        partialOrd->Print(templp->Schema());
    else
        cout << "NULL";
    cout  << endl;
    cout << "Output Order : " << endl;
    ord->Print(templp->Schema());
    cout  << endl;
    cout << "Cost: " << Sort_t::Cost(ord, templp, partialOrd) << endl;

    // Ravi: End  temporary code - for testing sort cost model

    */

    if( isFullExpand ) {
        // expand the dag by applying all possible transformations
        CostVal_t costUB = -1;

        if( costLimit )
                costUB = *costLimit;

        struct rusage rusageBeg, rusageEnd;

        if( Config_t::PrintDAGExpansionTime() )
                getrusage(RUSAGE_SELF, &rusageBeg);
        
        ExpandDAG(rootEq);
       
        // cout << "=========== Aliase Table =============" << endl;
        // Interface_t::aliasTable.PrintTable();

        // IMPORTANT: Commenting out just for EIO Tests. Uncomment later
        // rootEq->SetOuterVarLogProp();

        // Ravi:Add:Print the expanded DAG.
        if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) ) {
            cout << "Expanded LQDAG" << endl; 
            rootEq->PrintExpressionDAGXML(0);
            cout << "End of Expanded LQDAG" << endl; 
        }

        // cout << "Calling GetInteresting Order -- Debug Print" << endl; 
        List_t<Order_t *> *intOrds = rootEq->GetInterestingOrders(NULL, 0);
        // cout << "GetInterestingOrders... Success!" << endl; 

        List_t<Order_t *> *favOrds = rootEq->GetFavorableOrders();
        PrintOrders(favOrds, rootEq->LogicalProp()->Schema());

        if( Config_t::PrintDAGExpansionTime() ) {
                getrusage(RUSAGE_SELF, &rusageEnd);

                int utimetaken =
                    (rusageEnd.ru_utime.tv_sec-rusageBeg.ru_utime.tv_sec)*1000 +
                    (rusageEnd.ru_utime.tv_usec-rusageBeg.ru_utime.tv_usec)/1000;
                int stimetaken =
                    (rusageEnd.ru_stime.tv_sec-rusageBeg.ru_stime.tv_sec)*1000 +
                    (rusageEnd.ru_stime.tv_usec-rusageBeg.ru_stime.tv_usec)/1000;

                cout << "LDAGUSR " << utimetaken << endl;
                cout << "LDAGSYS " << stimetaken << endl;
            }

            if( Config_t::IsPruning() )
                PruneDAG(rootEq, costUB);

            if( Config_t::IsSubsumption() ) {
                // apply logical subsumption
                ExpandSubsumption(rootEq);
            }
    }

    // required physical properties kept empty
    PhyPropGroup_t *reqProp = new PhyPropGroup_t;
    Refer(reqProp);

    if (reqOrder) {
        reqProp->AddProperty(reqOrder);
    }

    // initial previously computed plan cost
    Cost_t *initTotalCost = CreateCost();

    if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) )
        flush(cout << "Optimizing Eq: " << rootEq->ID() << endl);

    PlanGroup_t *pg = FindBestPlan(rootEq, reqProp, initTotalCost, costLimit);

    if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) )
        flush(cout << "Done Optimizing Eq: " << rootEq->ID() << endl);

    delete initTotalCost;

    DeRefer(reqProp);

    return pg;
}

// Find best plan for given equivalence class eq
PlanGroup_t *Volcano_t::FindBestPlan(Equivalence_t *eq,
                                     PhyPropGroup_t *reqProp, Cost_t *totalCost,
                                     CostVal_t *costLimit, int isInpIndexed,
                                     Order_t *pso, int *plmap, 
                                     int level, Card_t callCount)
{
    assert(eq);
    assert(reqProp);
    assert(totalCost);

    // get the plan group of eq for property reqProp
    PlanGroup_t *planGroup = eq->PlanGroup(reqProp, pso, callCount);
    assert(planGroup);

    if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) )
        flush(cout << "FindBestPlan Eq: " << eq->ID() << endl);

    Plan_t *bestPlan = FindBestPlan(planGroup, totalCost,
                                    costLimit, isInpIndexed,
                                    pso, plmap, level, callCount);

    if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) )
        flush(cout << "Finished FindBestPlan Eq: " << eq->ID() << endl);

    if( !bestPlan )
        return NULL;

    return planGroup;
}

// Find best algo plan for given equivalence class eq
PlanGroup_t *Volcano_t::FindBestAlgPlan(Equivalence_t *eq,
                        PhyPropGroup_t *reqProp, Cost_t *totalCost,
                        CostVal_t *costLimit, int isInpIndexed,
                        Order_t *pso, int *plmap, 
                        int level, Card_t callCount)
{
    assert(eq);
    assert(reqProp);
    assert(totalCost);

    // get the plan group of eq for property reqProp
    PlanGroup_t *planGroup = eq->PlanGroup(reqProp, pso, callCount);
    assert(planGroup);

    if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) )
        flush(cout << "FindBestAlgPlan Eq: " << eq->ID() << endl);

    Plan_t *bestAlgPlan = FindBestAlgPlan(planGroup, totalCost,
                                          costLimit, isInpIndexed,
                                          pso, plmap, level, callCount);

    if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) )
        flush(cout << "Finished FindBestAlgPlan Eq: " << eq->ID() << endl);

    if( !isFullExpand ) {
        if( Config_t::IsSubsumption() )
            ApplyLogicalSubs(eq);
    }

    if( !bestAlgPlan )
        return NULL;

    return planGroup;
}

// Find best enforcer plan for given equivalence class eq
PlanGroup_t *Volcano_t::FindBestEnfPlan(Equivalence_t *eq,
                        PhyPropGroup_t *reqProp, Cost_t *totalCost,
                        CostVal_t *costLimit, int isInpIndexed,
                        Order_t *pso, int *plmap, 
                        int level, Card_t callCount)
{
    assert(eq);
    assert(reqProp);
    assert(totalCost);

    // get the plan group of eq for property reqProp
    PlanGroup_t *planGroup = eq->PlanGroup(reqProp, pso, callCount);
    assert(planGroup);

    if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) )
        flush(cout << "FindBestEnfPlan Eq: " << eq->ID() << endl);

    Plan_t *bestEnfPlan = FindBestEnfPlan(planGroup, totalCost,
                                          costLimit, isInpIndexed,
                                          pso, plmap, level, callCount);

    if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) )
        flush(cout << "FindBestEnfPlan Eq: " << eq->ID() << endl);

    if( !bestEnfPlan )
        return NULL;

    return planGroup;
}

// The plan must satisfy required properties reqProp
// If costLimit is NULL, the whole DAG is generated without pruning.
// The cost of the plan must not exceed costLimit.
// Returns NULL if a valid best plan exists but does not follow the
// above constraints or if the plan is failed due to pruning.
// Found best plan is set as the best plan for the plan group associated
// with the required property.
// A failed plan has root == NULL and cost as costLimit
// isInpIndexed == 1 if the parent op uses the index
Plan_t *Volcano_t::FindBestPlan(PlanGroup_t *planGroup,
                    Cost_t *totalCost, CostVal_t *costLimit, int isInpIndex,
                    Order_t *pso, int *plmap, int level, Card_t callCount)
{
    assert(planGroup);
    assert(totalCost);

    // get the best plan for this plan group
    Plan_t *bestPlan = planGroup->BestPlan();

    if( bestPlan ) {
        // a best plan exists -- there was an attempt to optimize earlier
        if( !IsRU() || CurQueryID() == planGroup->BestPlanQueryID() ) {
            // the attempt was during the optimization of the current
            // query --- relevant only if MQO is turned on
            if( !costLimit ) {
                // because there is no pruning, bestPlan is a valid best plan
                return bestPlan;
            }

            Cost_t *cost = bestPlan->Cost();
            assert(cost);

            // the cost of the union of totalCost and cost
            CostVal_t totalCostVal = totalCost->SumCost(cost);

            // totalCostVal is a lower bound on the optimal cost
            if( totalCostVal > *costLimit )
                return NULL;

            // if the expression has been successfully optimized earlier
            // then plan must be optimal
            if( !bestPlan->IsFailed() ) {
                *costLimit = totalCostVal;
                return bestPlan;
            }

            // optimization had failed for cost stored with bestPlan
            if( totalCostVal == *costLimit )
                return NULL;
        } else {
            // MQO enabled; optimization was attempted by a prev query

            if( planGroup->IsMarked() && !bestPlan->IsFailed() ) {
                // planGroup is a part of a prev best plan

                // assert(!bestPlan->IsFailed());

                // if pruning disabled, then return the best plan
                // if( !costLimit )
                //    return bestPlan;
                assert(costLimit);

                // for costing, check with the min of reuse cost and
                // plan cost
                Equivalence_t *eq = planGroup->EqClass();
                assert(eq);

                CostVal_t useCost = 0;
                if( !isInpIndex || !planGroup->IsIndexed() ) {
                    // use cost for non-index or for index used by
                    // non-index algos
                    useCost = eq->UseCost();
                }

                CostVal_t matCost = 0;
                if( !planGroup->IsIndexed() ) {
                    // add the materialization cost also for non-index
                    matCost = eq->MatCost();
                }

                CostVal_t cost = 0;
                if( useCost + matCost < bestPlan->CostVal() ) {
                    cost = useCost + matCost;
                } else {
                    cost = bestPlan->CostVal();
                }

                // the cost of the union of totalCost and cost
                CostVal_t totalCostVal = totalCost->Value() + cost;

                if( totalCostVal > *costLimit )
                    return NULL;

                if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) ) {
                    if( useCost + matCost < bestPlan->CostVal() ) {
                        cout << "To be shared pg = " <<
                                        planGroup->ID() << endl;
                    }
                }

                *costLimit = totalCostVal;
                return bestPlan;
            }
        }

        // if is in progress, do not optimize
        if( planGroup->IsInProgress() )
            return NULL;

        // dispose bestplan and reoptimize
        planGroup->SetBestPlan(NULL);
    } else {
        // create the bestPlan
        bestPlan = new Plan_t(planGroup);
    }

    // optimization required
    planGroup->SetBestPlanQueryID(CurQueryID());

    Plan_t *curBestPlan = NULL;

    // find the best plan among the algo and enforcer plans
    // returns NULL if no algo plan within the costLimit exists
    Plan_t *bestAlgPlan = FindBestAlgPlan(planGroup, totalCost,
                                          costLimit, isInpIndex,
                                          pso, plmap, level, callCount);

    // Ravi: The above should also update costLimit based on the bestAlgPlan
    // Ravi:  Verify above....

    // returns NULL if no enforcer plan within the costLimit exists
    Plan_t *bestEnfPlan = FindBestEnfPlan(planGroup, totalCost,
                                          costLimit, isInpIndex,
                                          pso, plmap, level, callCount);

    // Ravi: The above will be null if no plan cheaper than the bestAlgPlan could
    // Ravi: be found
    // Ravi:  Verify above....

    if( !costLimit ) {
        // pruning disabled; check for the best plan
        // this is not proper for Volcano-RU; but that will have a costLimit

        if( bestAlgPlan ) {
            if( bestEnfPlan ) {
                Cost_t *bestAlgCost = bestAlgPlan->Cost();
                assert(bestAlgCost);

                Cost_t *bestEnfCost = bestEnfPlan->Cost();
                assert(bestEnfCost);

                if( totalCost->SumCost(bestAlgCost) <=
                                    totalCost->SumCost(bestEnfCost) ) {
                    curBestPlan = bestAlgPlan;
                } else {
                    curBestPlan = bestEnfPlan;
                }
            } else {
                curBestPlan = bestAlgPlan;
            }
        } else {
            curBestPlan = bestEnfPlan;
        }
    } else {
        // Ravi: bestEnfPlan is non-null only if it's cheaper than the 
        // Ravi: bestAlgPlan (because first the bestAlgPlan is found
        // Ravi: and then the bestEnfPlan is looked for with the updated
        // Ravi: cost limit.)
        curBestPlan = bestAlgPlan;
        if( bestEnfPlan )
            curBestPlan = bestEnfPlan;
    }

    if( curBestPlan ) {
        // a valid best plan exists
        assert(!curBestPlan->IsFailed());

        bestPlan->SetCost(curBestPlan->Cost());
        bestPlan->SetRoot(curBestPlan->Root());
    } else {
        // sanity check: pruning must be enabled
        assert(costLimit);

        // no feasible plan is found; store the dummy failed best plan
        // which keeps track of the cost at which the failure occured
        Cost_t *bestCost =
                    CreateCost(NULL, *costLimit - totalCost->Value());
        Refer(bestCost);

        bestPlan->SetRoot(NULL);
        bestPlan->SetCost(bestCost);

        DeRefer(bestCost);
    }

    planGroup->SetBestPlan(bestPlan);

    if( bestPlan->IsFailed() )
        return NULL;

    return bestPlan;
}

// Find best algo plan for given planGroup
// If costLimit is NULL, the whole DAG is generated without pruning.
// The cost of the plan must not exceed costLimit.
// Returns NULL if a valid best plan exists but does not follow the
// above constraints or if the plan is failed due to pruning.
// Found best plan is set as the best plan for the plan group associated
// with the required property.
// A failed plan has root == NULL and cost as costLimit
// isInpIndexed == 1 if the parent op uses the index
Plan_t *Volcano_t::FindBestAlgPlan(PlanGroup_t *planGroup, 
                   Cost_t *totalCost, CostVal_t *costLimit, int isInpIndex,
                   Order_t *pso, int *plmap, 
                   int level, Card_t callCount)
{
    assert(planGroup);
    assert(totalCost);

    // get the best plan for this plan group
    Plan_t *bestAlgPlan = planGroup->BestAlgPlan();

    if( bestAlgPlan ) {
        // a best plan exists -- there was an attempt to optimize earlier
        if( !IsRU() || CurQueryID() == planGroup->BestAlgPlanQueryID() ) {
            // the attempt was during the optimization of the current
            // query --- relevant only if MQO is turned on
            if( !costLimit ) {
                // because there is no pruning, bestAlgPlan is a valid best plan
                return bestAlgPlan;
            }

            Cost_t *cost = bestAlgPlan->Cost();
            assert(cost);

            // the cost of the union of totalCost and cost
            CostVal_t totalCostVal = totalCost->SumCost(cost);

            // totalCostVal is a lower bound on the optimal cost
            if( totalCostVal > *costLimit )
                return NULL;

            // if the expression has been successfully optimized earlier
            // then plan must be optimal
            if( !bestAlgPlan->IsFailed() ) {
                *costLimit = totalCostVal;
                return bestAlgPlan;
            }

            // optimization had failed for cost stored with bestAlgPlan
            if( totalCostVal == *costLimit )
                return NULL;
        } else {
            // MQO enabled; optimization was attempted by a prev query

            if( planGroup->IsMarked() ) {
                // planGroup is a part of a prev best plan

                assert(!bestAlgPlan->IsFailed());

                // if pruning disabled, then return the best plan
                // if( !costLimit )
                //     return bestAlgPlan;
                assert(costLimit);

                // for costing, check with the min of reuse cost and
                // plan cost
                Equivalence_t *eq = planGroup->EqClass();
                assert(eq);

                CostVal_t useCost = 0;
                if( !isInpIndex || !planGroup->IsIndexed() ) {
                    // use cost for non-index or for index used by
                    // non-index algos
                    useCost = eq->UseCost();
                }

                CostVal_t matCost = 0;
                if( !planGroup->IsIndexed() ) {
                    // add the materialization cost also for non-index

                    matCost = eq->MatCost();
                }

                CostVal_t cost = 0;
                if( useCost + matCost < bestAlgPlan->CostVal() ) {
                    cost = useCost + matCost;
                } else {
                    cost = bestAlgPlan->CostVal();
                }

                // the cost of the union of totalCost and cost
                CostVal_t totalCostVal = totalCost->Value() + cost;

                if( totalCostVal > *costLimit )
                    return NULL;

                *costLimit = totalCostVal;
                return bestAlgPlan;
            }
        }

        // if is in progress, do not optimize
        if( planGroup->IsInProgress() )
            return NULL;

        // dispose bestplan and reoptimize
        planGroup->SetBestPlan(NULL);
    } else {
        // create the bestAlgPlan
        bestAlgPlan = new Plan_t(planGroup);
    }

    // optimization required
    planGroup->SetBestAlgPlanQueryID(CurQueryID());

    if( costLimit ) {
        // if no feasible plan is found; store the dummy failed best plan
        // which keeps track of the cost at which the failure occured
        Cost_t *bestCost =
                    CreateCost(NULL, *costLimit - totalCost->Value());
        Refer(bestCost);

        bestAlgPlan->SetRoot(NULL);
        bestAlgPlan->SetCost(bestCost);

        DeRefer(bestCost);
    }

    Equivalence_t *eq = planGroup->EqClass();
    assert(eq);

    const List_t<LogicalOp_t *>& exprList = eq->LogExprList();

    // the list exprList is append-only; this ensures that the iterator
    // encounters the insertions made during the iterator scan

    // apply the algorithms
    Memo_t& exprMemo = Memo();

    ListIter_t<LogicalOp_t *> exprIter;
    exprIter.Attach(&exprList);

    eq->MarkInProgress();

    if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) )
        cout << "Volcano_t::FindBestAlgPlan Iterating over " << eq->ID() <<
                            " for PlanGroup = " << planGroup->ID() << endl;

    while( !exprIter.IsEnd() ) {
        LogicalOp_t *logExpr = exprIter.Next();
        assert(logExpr);
        if( logExpr->IsDetached() )
            continue;

        if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) )
            cout << "Eq = " << eq->ID() << " PlanGroup = " << planGroup->ID()
                                << " LogExpr = " << logExpr->ID() << endl;
        // applies algorithms and recursively optimizes the inputs
        // and returns the overall best plan
        // updates bestAlgPlan

        // no algo plans if index prop is reqd
        // if( !planGroup->IsIndexed() )
            logExpr->ApplyAlgorithms(planGroup, this, totalCost,
                                     bestAlgPlan, costLimit,
                                     pso, plmap, level, callCount);
        if( !isFullExpand )
            logExpr->Expand(exprMemo);
    }

    eq->UnMarkInProgress();

    if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) )
        cout << "Volcano_t::FindBestAlgPlan Finished Iterating over " <<
                    eq->ID() << " for PlanGroup = " << planGroup->ID() << endl;

    // properties required in the current plan
    PhyPropGroup_t *reqProp = planGroup->PropGroup();
    assert(reqProp);

    if( reqProp->NumProps() == 0 && exprList.Size() > 0 ) {
        // recurse over covering plangroups --- created if not present

        SortOrder_t **sortOrderList = NULL;
        int numExprs = exprList.Size();

        sortOrderList = new SortOrder_t *[numExprs];
        int numSortOrders = 0;

        for( int j = 0 ; j < numExprs ; j++ )
            sortOrderList[j] = NULL;

        exprIter.Reset();
        while( !exprIter.IsEnd() ) {
            LogicalOp_t *logExpr = exprIter.Next();
            assert(logExpr);

            if( logExpr->Type() == JOIN_T ) {
                Join_t *joinExpr = (Join_t *) logExpr;

                // find out if sort-order is possible
                SortOrder_t *sortOrder = NULL;
                SortOrder_t *leftInpSortOrder = NULL;
                SortOrder_t *rightInpSortOrder = NULL;

                int isSortOrderPossible =
                                    joinExpr->SortProperty(&sortOrder,
                                        &leftInpSortOrder, &rightInpSortOrder);

                if( isSortOrderPossible ) {
                    // now we know that sort-order is possible
                    Refer(sortOrder);

                    Refer(leftInpSortOrder);
                    Refer(rightInpSortOrder);

                    sortOrderList[numSortOrders++] = sortOrder;

                    DeRefer(leftInpSortOrder);
                    DeRefer(rightInpSortOrder);
                }
            }
        }

        for( int i = 0 ; i < numSortOrders ; i++ ) {
            SortOrder_t *sortOrder = sortOrderList[i];
            assert(sortOrder);

            // include the sort order and recurse with the new req prop
            PhyPropGroup_t *newReqProp = new PhyPropGroup_t;
            Refer(newReqProp);
            newReqProp->AddProperty(sortOrder);

            // Ravi: Passing extra params (including the isInpIndex (0)
            PlanGroup_t *newPlanGroup =
                        FindBestAlgPlan(eq, newReqProp, totalCost, costLimit,
                                        0, pso, plmap, level, callCount);

            /*---
            PlanGroup_t *newPlanGroup =
                        FindBestPlan(eq, newReqProp, totalCost, costLimit);
            ---*/

            DeRefer(newReqProp);

            if( newPlanGroup ) {
                // a plan found
                Plan_t *newBestPlan = newPlanGroup->BestAlgPlan();
                assert(newBestPlan);
                assert(!newBestPlan->IsFailed());

                PhysicalOp_t *newRoot = newBestPlan->Root();
                assert(newRoot);
                Cost_t *newCost = newBestPlan->Cost();
                assert(newCost);

                // update the best plan if found plan better
                int isBetterPlan = 1;

                // if pruning enabled, we know that we have arrived at a better
                // plan, so a check is not necessary
                if( !costLimit ) {
                    // pruning disabled
                    Cost_t *prevCost = bestAlgPlan->Cost();
                    if( prevCost ) {
                        CostVal_t prevBestCost = totalCost->SumCost(prevCost);
                        CostVal_t newBestCost = totalCost->SumCost(newCost);

                        if( newBestCost >= prevBestCost )
                            isBetterPlan = 0;
                    }
                }
        
                if( isBetterPlan ) {
                    bestAlgPlan->SetRoot(newRoot);
                    bestAlgPlan->SetCost(newCost);

                    if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) ) {
                        bestAlgPlan->PrintPlan();
                        cout << endl;
                        cout << endl;
                    }
                }
            }

            DeRefer(sortOrder);
        }

        delete[] sortOrderList;
    }

    planGroup->SetBestAlgPlan(bestAlgPlan);

    // Ravi: Doubt: When is a plan marked as failed? Isn't it the case that 
    // Ravi: if a plan within the cost limit is not found NULL is returned? 
    // Ravi: Then why do we ever mark a plan as failed? Does this correspond 
    // Ravi: to the case when we create a dummy plan just to remember for 
    // Ravi: what cost the failure occured?
    if( bestAlgPlan->IsFailed() ) {
        // bestAlgPlan invalid
        return NULL;
    }

    return bestAlgPlan;
}

// Find best enforcer plan for given planGroup
// The plan must satisfy required properties reqProp
// If costLimit is NULL, the whole DAG is generated without pruning.
// The cost of the plan must not exceed costLimit.
// Returns NULL if a valid best plan exists but does not follow the
// above constraints or if the plan is failed due to pruning.
// Found best plan is set as the best plan for the plan group associated
// with the required property.
// A failed plan has root == NULL and cost as costLimit
// isInpIndexed == 1 if the parent op uses the index
Plan_t *Volcano_t::FindBestEnfPlan(PlanGroup_t *planGroup,
                    Cost_t *totalCost, CostVal_t *costLimit, int isInpIndex,
                    Order_t *pso, int *plmap, 
                    int level, Card_t callCount)
{
    assert(planGroup);
    assert(totalCost);

    // get the best plan for this plan group
    Plan_t *bestEnfPlan = planGroup->BestEnfPlan();

    if( bestEnfPlan ) {
        // a best plan exists -- there was an attempt to optimize earlier
        if( !IsRU() || CurQueryID() == planGroup->BestEnfPlanQueryID() ) {
            // the attempt was during the optimization of the current
            // query --- relevant only if MQO is turned on
            if( !costLimit ) {
                // because there is no pruning, bestEnfPlan is a valid best plan
                return bestEnfPlan;
            }

            Cost_t *cost = bestEnfPlan->Cost();
            assert(cost);

            // the cost of the union of totalCost and cost
            CostVal_t totalCostVal = totalCost->SumCost(cost);

            // totalCostVal is a lower bound on the optimal cost
            if( totalCostVal > *costLimit )
                return NULL;

            // if the expression has been successfully optimized earlier
            // then plan must be optimal
            if( !bestEnfPlan->IsFailed() ) {
                *costLimit = totalCostVal;
                return bestEnfPlan;
            }

            // optimization had failed for cost stored with bestEnfPlan
            if( totalCostVal == *costLimit )
                return NULL;
        } else {
            // MQO enabled; optimization was attempted by a prev query

            if( planGroup->IsMarked() && !bestEnfPlan->IsFailed() ) {
                // planGroup is a part of a prev best plan

                // assert(!bestEnfPlan->IsFailed());

                // if pruning disabled, then return the best plan
                // if( !costLimit )
                //     return bestEnfPlan;
                assert(costLimit);

                // for costing, check with the min of reuse cost and
                // plan cost
                Equivalence_t *eq = planGroup->EqClass();
                assert(eq);

                CostVal_t useCost = 0;
                if( !isInpIndex || !planGroup->IsIndexed() ) {
                    // use cost for non-index or for index used by
                    // non-index algos
                    useCost = eq->UseCost();
                }

                CostVal_t matCost = 0;
                    // add the materialization cost also for non-index

                if( !planGroup->IsIndexed() )
                    matCost = eq->MatCost();

                CostVal_t cost = 0;
                if( useCost + matCost < bestEnfPlan->CostVal() ) {
                    cost = useCost + matCost;
                } else {
                    cost = bestEnfPlan->CostVal();
                }

                // the cost of the union of totalCost and cost
                CostVal_t totalCostVal = totalCost->Value() + cost;

                if( totalCostVal > *costLimit )
                    return NULL;

                *costLimit = totalCostVal;
                return bestEnfPlan;
            }
        }

        // if is in progress, do not optimize
        if( planGroup->IsInProgress() )
            return NULL;

        // dispose bestplan and reoptimize
        planGroup->SetBestPlan(NULL);
    } else {
        // create the bestEnfPlan
        bestEnfPlan = new Plan_t(planGroup);
    }

    // optimization required
    planGroup->SetBestEnfPlanQueryID(CurQueryID());

    if( costLimit )  {
        // if no feasible plan is found; store the dummy failed best plan
        // which keeps track of the cost at which the failure occured
        Cost_t *bestCost =
                    CreateCost(NULL, *costLimit - totalCost->Value());
        Refer(bestCost);

        bestEnfPlan->SetRoot(NULL);
        bestEnfPlan->SetCost(bestCost);

        DeRefer(bestCost);
    }

    // apply the enforcers to relax the reqProp and recursively optimize
    // updates bestEnfPlan, costLimit
    ApplyEnforcers(planGroup, totalCost, bestEnfPlan, costLimit, 
                   pso, plmap, level, callCount);

    planGroup->SetBestEnfPlan(bestEnfPlan);

    if( bestEnfPlan->IsFailed() ) {
        // bestEnfPlan invalid
        return NULL;
    }

    return bestEnfPlan;
}

// applies enforcers and recursively optimizes the expression with the
// relaxed physical properties
// updates bestPlan
// Ravi:Todo:Check difference between Cost_t and CostVal_t
// Ravi: SS says Cost_t used to support an old setcost model which would 
// Ravi:    avoid double counting shared operations.  This setcost model is 
// Ravi:    no longer in use...
void Volcano_t::ApplyEnforcers(PlanGroup_t *planGroup,
                                    Cost_t *totalCost, Plan_t *bestEnfPlan,
                                    CostVal_t *costLimit, 
                    Order_t *pso, int *plmap, int level, Card_t callCount)
{
    assert(planGroup);
    assert(totalCost);
    assert(bestEnfPlan);

    PhyPropGroup_t *reqProp = planGroup->PropGroup();
    assert(reqProp);

    const List_t<PhyProp_t *>& reqPropList = reqProp->PropList();

    // enforce each physical property one by one
    ListIter_t<PhyProp_t *> iter;
    iter.Attach(&reqPropList);

    while( !iter.IsEnd() ) {
        PhyProp_t *prop = iter.Next();
        prop->Enforce(planGroup, this,
                            totalCost, bestEnfPlan, costLimit, 
                pso, plmap, level, callCount);
    }
}

// expand the dag rooted at eq fully by applying all possible
// transformations
void Volcano_t::ExpandDAG(Equivalence_t *eq)
{
    assert(eq);
    if( eq->IsExpanded() )
        return;

    if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) ) {
        cout << endl;
        cout << "Expanding SubDAG for eq " << eq->ID() << endl;
    }

    // iterate over all logical expressions
    const List_t<LogicalOp_t *>& exprList = eq->LogExprList();

    ListIter_t<LogicalOp_t *> iter;
    iter.Attach(&exprList);

    while( !iter.IsEnd() ) {
        LogicalOp_t *op = iter.Next();
        assert(op);
        if( op->IsDetached() )
            continue;

        int n = op->NumInputs();
        for( int i = 0 ; i < n ; i++ ) {
            // op can get detached midway due to unification!
            if( op->IsDetached() )
                break;
            Equivalence_t *childEq = op->Input(i);
            assert(childEq);

            ExpandDAG(childEq);
        }

        if( !op->IsDetached() )
            op->Expand(memo);
    }

    if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) ) {
        cout << "Finished Expanding SubDAG for eq " << eq->ID() << endl;
        cout << endl;
    }

    eq->MarkExpanded();
}

// prune the dag
void Volcano_t::PruneDAG(Equivalence_t *eq, CostVal_t costUB)
{
    assert(Config_t::IsPruning());
    assert(eq);

    if( eq->IsPruned() )
        return;
    if( costUB == -1 )
        return;

    assert(Config_t::IsPruning());

    // iterate over all logical expressions
    const List_t<LogicalOp_t *>& exprList = eq->LogExprList();

    // list of non-admissible logical expressions
    PrependList_t<LogicalOp_t *> delList;

    ListIter_t<LogicalOp_t *> iter;
    iter.Attach(&exprList);

    while( !iter.IsEnd() ) {
        LogicalOp_t *op = iter.Next();
        assert(op);
        if( op->IsDetached() )
            continue;

        int n = op->NumInputs();
        int i = -1;
        for( i = 0 ; i < n ; i++ ) {
            Equivalence_t *childEq = op->Input(i);
            assert(childEq);

            PruneDAG(childEq, costUB);
            if( childEq->NumLogExprs() == 0 )
                break;
        }

        if( i < n || !op->IsAdmissible(costUB) )
            delList.Insert(op);
    }

    // prune the non-admissible logical expressions
    PruneLogExprs(delList);
    eq->MarkPruned();
}

// prune off the given logical expression
void Volcano_t::PruneLogExprs(List_t<LogicalOp_t *>& list)
{
    assert(Config_t::IsPruning());

    while( !list.IsEmpty() ) {
        LogicalOp_t *op = list.DeleteTop();
        assert(op);

        // only the derived logical expressions can be deleted
        if( op->IsDerived() ) {
            if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) )
                cout << "Deleting logexpr " << op->ID() << endl;

            memo.Remove(op);
            delete op;
        }
    }
}

// expand the dag rooted at eq fully by applying subsumption transformations
void Volcano_t::ExpandSubsumption(Equivalence_t *eq)
{
    assert(Config_t::IsSubsumption());
    assert(eq);
    if( eq->IsSubsExpanded() )
        return;

    if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) ) {
        cout << "Applying logical subsumption at SubDAG for eq " <<
                                                        eq->ID() << endl;
    }

    // iterate over all logical expressions
    // this is for bottom-up application of subsumption
    const List_t<LogicalOp_t *>& exprList = eq->LogExprList();

    ListIter_t<LogicalOp_t *> iter;
    iter.Attach(&exprList);

    while( !iter.IsEnd() ) {
        LogicalOp_t *op = iter.Next();
        assert(op);
        if( op->IsDetached() )
            continue;

        int n = op->NumInputs();
        for( int i = 0 ; i < n ; i++ ) {
            Equivalence_t *childEq = op->Input(i);
            assert(childEq);

            ExpandSubsumption(childEq);
        }
    }

    // apply the subsumption transformations now
    ApplyLogicalSubs(eq);

    if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) ) {
        cout << "Finished Applying logical subsumption at SubDAG for eq " <<
                                                            eq->ID() << endl;
        cout << endl;
    }

    eq->MarkSubsExpanded();
}

// apply logical subsumption
void Volcano_t::ApplyLogicalSubs(Equivalence_t *eq)
{
    assert(Config_t::IsSubsumption());
    assert(eq);

    if( eq->IsSubsExpanded() )
        return;

    if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) ) {
        cout << "Applying logical subsumption at SubDAG for eq " <<
                                                        eq->ID() << endl;
    }

    const List_t<LogicalOp_t *>& parents = eq->ParList();
    int numPars = parents.Size();
    assert(numPars >= 0);

    // holds the parent select operators -- +1 to prevent zero allocation
    Select_t **selList = new Select_t *[numPars+1];

    // holds the parent aggregate operators -- +1 to prevent zero allocation
    Aggregate_t **aggList = new Aggregate_t *[numPars+1];

    // flags that the parent operator is covered by some other parent operator
    char *isCovered = new char[numPars+1];

    // number of parent selects
    int numSel = 0;

    // number of parent aggregates
    int numAgg = 0;

    
    ListIter_t<LogicalOp_t *> iter;
    iter.Attach(&parents);

    while( !iter.IsEnd() ) {
        LogicalOp_t *op = iter.Next();
        assert(op);

        if( op->Type() == SELECT_T ) {
            // schema --- to detect correlated predicates
            LogProp_t *lp = op->LogicalProp();
            assert(lp);
            Schema_t *schema = lp->Schema();
            assert(schema);

            Select_t *sel = (Select_t *) op;
            Predicate_t *p = sel->Predicate();
            assert(p);
            if( p->IsValidSchema(NULL, schema) ) {
                // not correlated
                selList[numSel++] = (Select_t *) op;
            }
        }

        if( op->Type() == AGGREGATE_T )
            aggList[numAgg++] = (Aggregate_t *) op;
    }

    int i = 0;
    for( i = 0 ; i < numSel ; i++ )
        isCovered[i] = 0;

    // for each pair of select parents, check if one subsumes the other;
    // if so, add the path due to subsumption
    int j = 0;
    for( i = 0 ; i < numSel ; i++ ) {
        for( j = i+1 ; j < numSel ; j++ ) {
            // compare selList[i] and selList[j]
            Predicate_t *leftPred = selList[i]->Predicate();
            assert(leftPred);
            Equivalence_t *leftEq = selList[i]->EqClass();
            assert(leftEq);

            Predicate_t *rightPred = selList[j]->Predicate();
            assert(rightPred);
            Equivalence_t *rightEq = selList[j]->EqClass();
            assert(rightEq);

            if( leftPred->Implies(rightPred) ) {
                AddSelect(leftEq, rightEq, leftPred);
                isCovered[i] = 1;
            } else if( rightPred->Implies(leftPred) ) {
                AddSelect(rightEq, leftEq, rightPred);
                isCovered[j] = 1;
            }
        }
    }

    // the covering predicate
    Predicate_t *pred = NULL;
    int isNewPred = 0;
    for( i = 0 ; i < numSel ; i++ ) {
        if( !isCovered[i] ) {
            // this predicate not covered by some other predicate
            Predicate_t *p = selList[i]->Predicate();
            assert(p);

            if( !pred ) {
                pred = p;
            } else {
                pred = new Or_t(p, pred);
                isNewPred = 1;
            }
        }
    }

    if( isNewPred ) {
        // put in a new subsuming subsuming logical expr
        // we know that this logical expr does not exist earlier and
        // therefore do not check

        Refer(pred);

        // subsuming selection
        Select_t *subSel = new Select_t(pred);
        subSel->MarkDerived();
        subSel->MarkExpanded();

        subSel->SetInput(0, eq);

        LogProp_t *lp = subSel->LogicalProp();
        Refer(lp);

        Equivalence_t *subEq = new Equivalence_t(lp);
        subEq->MarkSubsExpanded();
        subEq->MarkPruned();
        subEq->MarkExpanded();
        subEq->MarkSharable();

        DeRefer(lp);

        if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) )
            cout << "Created subsuming eq " << subEq->ID() << endl;

        subSel->SetEqClass(subEq);
        subEq->AddLogExpr(subSel);

        memo.Insert(subSel);

        // now add the subsumed selects
        // again, we know that these logical exprs do not exist earlier and
        // therefore do not check
        for( i = 0 ; i < numSel ; i++ ) {
            Equivalence_t *selEq = selList[i]->EqClass();
            assert(selEq);

            Predicate_t *p = selList[i]->Predicate();
            assert(p);

            Select_t *sel = new Select_t(p);
            sel->MarkDerived();
            sel->MarkExpanded();

            sel->SetInput(0, subEq);

            sel->SetEqClass(selEq);
            selEq->AddLogExpr(sel);
        }

        DeRefer(pred);
    }

    for( i = 0 ; i < numAgg ; i++ )
        isCovered[i] = 0;

    // for each pair of aggregate parents, check if one subsumes the other;
    // if so, add the path due to subsumption
    for( i = 0 ; i < numAgg ; i++ ) {
        for( j = i+1 ; j < numAgg ; j++ ) {
            // compare aggList[i] and aggList[j]

            Aggregate_t *leftAgg = aggList[i];
            Equivalence_t *leftEq = aggList[i]->EqClass();
            assert(leftEq);

            Aggregate_t *rightAgg = aggList[j];
            Equivalence_t *rightEq = aggList[j]->EqClass();
            assert(rightEq);

            if( leftAgg->IsAggEquivalent(rightAgg) ) {
                if( leftAgg->IsGroupBySubsumedBy(rightAgg) ) {
                    AddAggregate(leftEq, rightEq, leftAgg);
                    isCovered[i] = 1;
                } else if( rightAgg->IsGroupBySubsumedBy(leftAgg) ) {
                    AddAggregate(rightEq, leftEq, rightAgg);
                    isCovered[j] = 1;
                }
                // else add a new node -- not implemented 
            } else if( leftAgg->IsGroupByEquivalent(rightAgg) ) {
                if( leftAgg->IsAggSubsumedBy(rightAgg) ) {
                    AddProjection(leftEq, rightEq, leftAgg);
                } else if( rightAgg->IsAggSubsumedBy(leftAgg) ) {
                    AddProjection(rightEq, leftEq, rightAgg);
                } else {
                    // add a new subsuming node
                    AttrMap_t *leftAttrMap = leftAgg->AggAttrMap();
                    assert(leftAttrMap);

                    AttrMap_t *rightAttrMap = rightAgg->AggAttrMap();
                    assert(rightAttrMap);

                    AttrMap_t *groupByAttrMap = leftAgg->GroupByAttrMap();
                    assert(groupByAttrMap);

                    AddSubsAggregate(leftEq, leftAttrMap, rightEq, rightAttrMap,
                                                            groupByAttrMap, eq);
                }
            }
        }
    }

    delete[] aggList;
    delete[] selList;
    delete[] isCovered;

    eq->MarkSubsExpanded();

    if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) ) {
        cout << "Finished Applying logical subsumption at SubDAG for eq " <<
                                                        eq->ID() << endl;
    }
}

// add a project as subsumption path
void Volcano_t::AddProjection(Equivalence_t *parEq, Equivalence_t *childEq,
                                                            Aggregate_t *agg)
{
    assert(parEq);
    assert(childEq);
    assert(agg);
    
    AttrMap_t *aggAttrMap = agg->AggAttrMap();
    assert(aggAttrMap);
    int numAggAttrs = aggAttrMap->NumProj();

    AttrMap_t *groupByAttrMap = agg->GroupByAttrMap();
    assert(groupByAttrMap);
    int numGroupByAttrs = groupByAttrMap->NumProj();

    // projection list
    AttrMap_t *projMap = new AttrMap_t(numAggAttrs + numGroupByAttrs);
    Refer(projMap);

    int i = 0, j = 0;
    for( i = 0 ; i < numGroupByAttrs ; i++ ) {
        AttrMapEntry_t *entry = groupByAttrMap->Entry(i);
        projMap->SetProj(j++, entry);
    }

    for( i = 0 ; i < numAggAttrs ; i++ ) {
        AttrMapEntry_t *entry = aggAttrMap->Entry(i);
        projMap->SetProj(j++, entry);
    }

    Project_t *proj = new Project_t(projMap);
    DeRefer(projMap);

    proj->MarkDerived();
    proj->MarkExpanded();

    proj->SetInput(0, childEq);

    LogicalOp_t *prevProj = memo.Search(proj);
    if( prevProj ) {
        // already present
        delete proj;
        proj = NULL;

        Equivalence_t *prevEq = prevProj->EqClass();
        assert(prevEq);

        if( parEq->ID() != prevEq->ID() ) {
            if( memo.IsUnify() ) {
                parEq->Unify(prevEq, memo);
                delete prevEq;
            }
        }
    } else {
        // not present

        proj->SetEqClass(parEq);
        parEq->AddLogExpr(proj);

        memo.Insert(proj);
    }

    childEq->MarkSharable();
}

// add subsuming aggregate operator and put in the connecting paths
void Volcano_t::AddSubsAggregate(Equivalence_t *leftEq, AttrMap_t *leftAttrMap,
                            Equivalence_t *rightEq, AttrMap_t *rightAttrMap,
                            AttrMap_t *groupByAttrMap, Equivalence_t *eq)
{
    assert(leftEq);
    assert(leftAttrMap);
    assert(rightEq);
    assert(rightAttrMap);
    assert(groupByAttrMap);
    assert(eq);

    int leftNumAggAttrs = leftAttrMap->NumProj();
    int rightNumAggAttrs = rightAttrMap->NumProj();
    

    // merge the aggregate attributes
    AttrMap_t *aggAttrMap = new AttrMap_t(leftNumAggAttrs + rightNumAggAttrs);
    Refer(aggAttrMap);

    int i = 0, j = 0;
    for( i = 0 ; i < leftNumAggAttrs ; i++ ) {
        AttrMapEntry_t *entry = leftAttrMap->Entry(i);
        aggAttrMap->SetProj(j++, entry);
    }

    for( i = 0 ; i < rightNumAggAttrs ; i++ ) {
        AttrMapEntry_t *entry = rightAttrMap->Entry(i);
        aggAttrMap->SetProj(j++, entry);
    }

    Aggregate_t *subAgg = new Aggregate_t(groupByAttrMap, aggAttrMap);
    DeRefer(aggAttrMap);

    subAgg->MarkDerived();
    subAgg->MarkExpanded();

    subAgg->SetInput(0, eq);

    LogProp_t *lp = subAgg->LogicalProp();
    Refer(lp);

    Equivalence_t *subEq = new Equivalence_t(lp);
    subEq->MarkSharable();
    DeRefer(lp);

    subAgg->SetEqClass(subEq);
    subEq->AddLogExpr(subAgg);

    int numGroupByAttrs = groupByAttrMap->NumProj();

    // left subsumption path
    // left projection list
    AttrMap_t *leftProjMap = new AttrMap_t(leftNumAggAttrs + numGroupByAttrs);
    Refer(leftProjMap);

    j = 0;
    for( i = 0 ; i < numGroupByAttrs ; i++ ) {
        AttrMapEntry_t *entry = groupByAttrMap->Entry(i);
        leftProjMap->SetProj(j++, entry);
    }

    for( i = 0 ; i < leftNumAggAttrs ; i++ ) {
        AttrMapEntry_t *entry = leftAttrMap->Entry(i);
        leftProjMap->SetProj(j++, entry);
    }

    Project_t *leftProj = new Project_t(leftProjMap);
    DeRefer(leftProjMap);

    leftProj->MarkDerived();
    leftProj->MarkExpanded();

    leftProj->SetInput(0, subEq);

    leftProj->SetEqClass(leftEq);
    leftEq->AddLogExpr(leftProj);

    // right subsumption path
    // right projection list
    AttrMap_t *rightProjMap = new AttrMap_t(rightNumAggAttrs + numGroupByAttrs);
    Refer(rightProjMap);

    j = 0;
    for( i = 0 ; i < numGroupByAttrs ; i++ ) {
        AttrMapEntry_t *entry = groupByAttrMap->Entry(i);
        rightProjMap->SetProj(j++, entry);
    }

    for( i = 0 ; i < rightNumAggAttrs ; i++ ) {
        AttrMapEntry_t *entry = rightAttrMap->Entry(i);
        rightProjMap->SetProj(j++, entry);
    }

    Project_t *rightProj = new Project_t(rightProjMap);
    DeRefer(rightProjMap);

    rightProj->MarkDerived();
    rightProj->MarkExpanded();

    rightProj->SetInput(0, subEq);

    rightProj->SetEqClass(rightEq);
    rightEq->AddLogExpr(rightProj);
}

// add a select operator as a subsumption path
void Volcano_t::AddSelect(Equivalence_t *parEq, Equivalence_t *childEq,
                                                        Predicate_t *pred)
{
    assert(Config_t::IsSubsumption());
    assert(parEq);
    assert(childEq);
    assert(pred);

    if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) ) {
        cout << "Adding select subsumption path from [EQCLASS" <<
            parEq->ID() << "] to [EQCLASS" << childEq->ID() << "]" << endl;
    }

    Select_t *sel = new Select_t(pred);
    sel->MarkDerived();
    sel->MarkExpanded();

    sel->SetInput(0, childEq);

    LogicalOp_t *prevSel = memo.Search(sel);
    if( prevSel ) {
        // already present
        delete sel;
        sel = NULL;

        Equivalence_t *prevEq = prevSel->EqClass();
        assert(prevEq);

        if( parEq->ID() != prevEq->ID() ) {
            if( memo.IsUnify() ) {
                parEq->Unify(prevEq, memo);
                delete prevEq;
            }
        }
    } else {
        // not present

        sel->SetEqClass(parEq);
        parEq->AddLogExpr(sel);

        memo.Insert(sel);
    }

    childEq->MarkSharable();

    if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) ) {
        cout << "Finished adding select subsumption path from [EQCLASS" <<
            parEq->ID() << "] to [EQCLASS" << childEq->ID() << "]" << endl;
    }
}

// add a select operator as a subsumption path
void Volcano_t::AddAggregate(Equivalence_t *parEq, Equivalence_t *childEq,
                                                            Aggregate_t *agg)
{
    assert(Config_t::IsSubsumption());
    assert(parEq);
    assert(childEq);
    assert(agg);

    AttrMap_t *groupByAttrMap = agg->GroupByAttrMap();
    assert(groupByAttrMap);

    AttrMap_t *aggAttrMap = agg->AggAttrMap();
    assert(aggAttrMap);

    if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) ) {
        cout << "Adding aggregation subsumption path from [EQCLASS" <<
            parEq->ID() << "] to [EQCLASS" << childEq->ID() << "]" << endl;
    }

    Aggregate_t *newAgg = new Aggregate_t(groupByAttrMap, aggAttrMap);
    newAgg->MarkDerived();
    newAgg->MarkExpanded();

    newAgg->SetInput(0, childEq);

    LogicalOp_t *prevAgg = memo.Search(newAgg);
    if( prevAgg ) {
        // already present
        delete newAgg;
        newAgg = NULL;

        Equivalence_t *prevEq = prevAgg->EqClass();
        assert(prevEq);

        if( parEq->ID() != prevEq->ID() ) {
            if( memo.IsUnify() ) {
                parEq->Unify(prevEq, memo);
                delete prevEq;
            }
        }
    } else {
        // not present

        newAgg->SetEqClass(parEq);
        parEq->AddLogExpr(newAgg);

        memo.Insert(newAgg);
    }

    childEq->MarkSharable();

    if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) ) {
        cout << "Finished adding subsumption path from [EQCLASS" <<
            parEq->ID() << "] to [EQCLASS" << childEq->ID() << "]" << endl;
    }
}

// mark the best plan rooted at pg
void Volcano_t::MarkBestPlan(PlanGroup_t *pg)
{
    assert(IsRU());
    assert(pg);

    if( pg->IsMarked() )
        return;
    pg->Mark();

    Plan_t *plan = pg->BestPlan();
    if( !plan )
        plan = pg->BestAlgPlan();
    assert(plan);
    // Ravi: The next if would be more clear if it were within an else for the
    // Ravi:     previous if.
    if( plan->IsFailed() )
        plan = pg->BestAlgPlan();

    assert(plan);
    assert(!plan->IsFailed());

    PhysicalOp_t *op = plan->Root();
    assert(op);

    if( op->Type() == ALGORITHM_T ) {
        Algorithm_t *alg = (Algorithm_t *) op;
        Predicate_t *pred = alg->Predicate();
        if( pred ) {
            // a filter operation
            InExpr_t *inPred = (InExpr_t *) pred->FindIn();
            if( inPred ) {
                // a nested query
                LogProp_t *lp = pg->LogicalProp();
                assert(lp);

                Card_t dist = inPred->NumDistinct(lp);
                assert(dist > 0);

                PlanGroup_t *nqPG = inPred->InpPlanGroup();
                assert(nqPG);

                MarkBestPlan(nqPG);
            }
        }
    }

    int n = op->NumInputs();
    for( int i = 0 ; i < n ; i++ ) {
        PlanGroup_t *inp = NULL;
        inp = op->Input(i);
        assert(inp);

        MarkBestPlan(inp);
    }
}

// unmark the best plan rooted at pg
void Volcano_t::UnMarkBestPlan(PlanGroup_t *pg)
{
    assert(IsRU());
    assert(pg);

    if( !pg->IsMarked() )
        return;
    pg->UnMark();

    Plan_t *plan = pg->BestPlan();
    if( !plan )
        plan = pg->BestAlgPlan();
    assert(plan);
    if( plan->IsFailed() )
        plan = pg->BestAlgPlan();

    assert(plan);
    assert(!plan->IsFailed());

    PhysicalOp_t *op = plan->Root();
    assert(op);

    if( op->Type() == ALGORITHM_T ) {
        Algorithm_t *alg = (Algorithm_t *) op;
        Predicate_t *pred = alg->Predicate();
        if( pred ) {
            // a filter operation
            InExpr_t *inPred = (InExpr_t *) pred->FindIn();
            if( inPred ) {
                // a nested query
                LogProp_t *lp = pg->LogicalProp();
                assert(lp);

                Card_t dist = inPred->NumDistinct(lp);
                assert(dist > 0);

                PlanGroup_t *nqPG = inPred->InpPlanGroup();
                assert(nqPG);

                UnMarkBestPlan(nqPG);
            }
        }
    }

    int n = op->NumInputs();
    for( int i = 0 ; i < n ; i++ ) {
        PlanGroup_t *inp = NULL;
        inp = op->Input(i);
        assert(inp);

        UnMarkBestPlan(inp);
    }
}

// returns the total cost of the best plan rooted at plan group
CostVal_t VolcanoOrd_t::SharCost(PlanGroup_t *planGroup)
{
    assert(planGroup);
    CostVal_t matCost = 0;

    // unmark shared
    int travID = planGroup->StartScan();
    UnMarkShared(travID, planGroup);

    // compute the usecounts without materialization
    // (upper bounds on actual value)
    travID = planGroup->StartScan();
    ComputeUseCount(travID, planGroup, 1);

    // mark the nodes that are potentially sharable
    travID = planGroup->StartScan();
    MarkShared(travID, planGroup);

    // compute the usecounts with all shared nodes materialized
    // (lower bounds on actual value)
    travID = planGroup->StartScan();
    ComputeUseCount(travID, planGroup, 1);

    // use the lower bounds to select the actual nodes to materialize in
    // a bottom-up traversal
    travID = planGroup->StartScan();
    CostVal_t planCost = SharCostTrav(travID, NULL, 0, planGroup, &matCost);

    cout << "PlanCost = " << planCost << " MatCost = " << matCost << endl;
    return planCost + matCost;
}

// reset the isshared flag of the plan nodes 
void VolcanoOrd_t::UnMarkShared(int travID, PlanGroup_t *pg)
{
    assert(pg);

    if( pg->IsVisited(travID) ) {
        assert(!pg->IsShared());
        return;
    }
    pg->Visit(travID);
    pg->UnMarkShared();

    Plan_t *plan = pg->BestPlan();
    if( !plan )
        plan = pg->BestAlgPlan();
    assert(plan);
    if( plan->IsFailed() )
        plan = pg->BestAlgPlan();

    assert(plan);
    assert(!plan->IsFailed());

    PhysicalOp_t *op = plan->Root();
    assert(op);

    if( op->Type() == ALGORITHM_T ) {
        Algorithm_t *alg = (Algorithm_t *) op;
        Predicate_t *pred = alg->Predicate();
        if( pred ) {
            // a filter operation
            InExpr_t *inPred = (InExpr_t *) pred->FindIn();
            if( inPred ) {
                // a nested query
                LogProp_t *lp = pg->LogicalProp();
                assert(lp);

                Card_t dist = inPred->NumDistinct(lp);
                assert(dist > 0);

                PlanGroup_t *nqPG = inPred->InpPlanGroup();
                assert(nqPG);

                UnMarkShared(travID, nqPG);
            }
        }
    }

    int n = op->NumInputs();
    for( int i = 0 ; i < n ; i++ ) {
        PlanGroup_t *inp = NULL;
        inp = op->Input(i);
        assert(inp);

        UnMarkShared(travID, inp);
    }
}

// set the isshared flag of the plan nodes based on prev computed usecounts
void VolcanoOrd_t::MarkShared(int travID, PlanGroup_t *pg)
{
    assert(pg);

    if( pg->IsVisited(travID) )
        return;
    pg->Visit(travID);
    assert(!pg->IsShared());

    Plan_t *plan = pg->BestPlan();
    if( !plan )
        plan = pg->BestAlgPlan();
    assert(plan);
    if( plan->IsFailed() )
        plan = pg->BestAlgPlan();

    assert(plan);
    assert(!plan->IsFailed());

    PhysicalOp_t *op = plan->Root();
    assert(op);

    if( op->Type() == ALGORITHM_T ) {
        Algorithm_t *alg = (Algorithm_t *) op;
        Predicate_t *pred = alg->Predicate();
        if( pred ) {
            // a filter operation
            InExpr_t *inPred = (InExpr_t *) pred->FindIn();
            if( inPred ) {
                // a nested query
                LogProp_t *lp = pg->LogicalProp();
                assert(lp);

                Card_t dist = inPred->NumDistinct(lp);
                assert(dist > 0);

                PlanGroup_t *nqPG = inPred->InpPlanGroup();
                assert(nqPG);

                MarkShared(travID, nqPG);
            }
        }
    }

    int n = op->NumInputs();
    for( int i = 0 ; i < n ; i++ ) {
        PlanGroup_t *inp = NULL;
        inp = op->Input(i);
        assert(inp);

        MarkShared(travID, inp);
    }

    // check if the node is sharable
    // the usecounts are the upper bounds
    if( pg->UseCount() > 1 ) {
        if( pg->MatCost()/(pg->UseCount()-1) + pg->UseCost() <
                                                plan->CostVal() )
            pg->MarkShared();
    }
}

// compute the usecount -- number of times a node is use in the plan
// given that some nodes are materialized (marked as shared)
void VolcanoOrd_t::ComputeUseCount(int travID, PlanGroup_t *pg, int mult)
{
    assert(pg);

    if( !pg->IsVisited(travID) ) {
        // visiting for the first time -- initialize the use counts
        pg->ResetUseCount();
        pg->Visit(travID);
    }

    pg->IncrUseCount(mult);

    int inpMult = mult;                 // multiplicity of the inputs
    if( pg->IsShared() || pg->IsIndexed() ) {
        if( pg->UseCount() > 1 )        // input already traversed
            return;

        // input computed only once
        inpMult = 1;
    }

    Plan_t *plan = pg->BestPlan();
    if( !plan )
        plan = pg->BestAlgPlan();
    assert(plan);
    if( plan->IsFailed() )
        plan = pg->BestAlgPlan();

    assert(plan);
    assert(!plan->IsFailed());

    PhysicalOp_t *op = plan->Root();
    assert(op);

    if( op->Type() == ALGORITHM_T ) {
        Algorithm_t *alg = (Algorithm_t *) op;
        Predicate_t *pred = alg->Predicate();
        if( pred ) {
            // a filter operation
            InExpr_t *inPred = (InExpr_t *) pred->FindIn();
            if( inPred ) {
                // a nested query
                LogProp_t *lp = pg->LogicalProp();
                assert(lp);

                Card_t dist = inPred->NumDistinct(lp);
                assert(dist > 0);

                PlanGroup_t *nqPG = inPred->InpPlanGroup();
                assert(nqPG);

                ComputeUseCount(travID, nqPG, int(inpMult*dist));
            }
        }
    }

    int n = op->NumInputs();
    for( int i = 0 ; i < n ; i++ ) {
        PlanGroup_t *inp = NULL;
        inp = op->Input(i);
        assert(inp);

        ComputeUseCount(travID, inp, inpMult);
    }
}

// in a bottom-up pass, select the actual set of nodes to materialize
// and compute the cost of the plans based on the same
CostVal_t VolcanoOrd_t::SharCostTrav(int travID, PhysicalOp_t *parOp,
                            int inpNo, PlanGroup_t *pg, CostVal_t *matCost)
{
    assert(pg);
    assert(matCost);

    Equivalence_t *eq = pg->EqClass();
    assert(eq);

    if( pg->IsBuiltIn() ) {
        if( pg->IsIndexed() && parOp ) {
            if( parOp->Type() == ALGORITHM_T ) {
                Algorithm_t *alg = (Algorithm_t *) parOp;
                if( (alg->AlgoType() == INDEXEDSELECT_T ||
                        alg->AlgoType() == INDEXEDNESTEDLOOPSJOIN_T) &&
                                                            inpNo == 0 ) {
                    if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) )
                        cout << pg->ID() << " : 0" << endl;

                    return 0;
                }
            }
        }

        if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) )
            cout << pg->ID() << " : " << eq->UseCost() << endl;
        return eq->UseCost();
    }

    Plan_t *plan = pg->BestPlan();
    if( !plan )
        plan = pg->BestAlgPlan();
    assert(plan);
    if( plan->IsFailed() )
        plan = pg->BestAlgPlan();

    assert(plan);
    assert(!plan->IsFailed());

    if( pg->IsVisited(travID) && !eq->IsCorrelated() ) {
        CostVal_t useCost = eq->UseCost();
        if( pg->IsIndexed() && parOp ) {
            if( parOp->Type() == ALGORITHM_T ) {
                Algorithm_t *alg = (Algorithm_t *) parOp;
                if( (alg->AlgoType() == INDEXEDSELECT_T ||
                    alg->AlgoType() == INDEXEDNESTEDLOOPSJOIN_T) &&
                                                        inpNo == 0 ) {
                        useCost = 0;
                }
            }
        }

        if( !pg->IsIndexed() && !pg->IsShared() )
            useCost += eq->MatCost()/pg->UseCount();

        if( useCost < plan->CostVal() ) {
            if( !pg->IsShared() ) {
                // found that a new node is shared ...

                if( !pg->IsIndexed() ) {
                    // add the materialization cost 

                    *matCost += eq->MatCost();
                }

                pg->MarkShared();

                if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) )
                    cout << "Sharing " << pg->ID() << endl;
            }

            if( pg->IsIndexed() && parOp ) {
                if( parOp->Type() == ALGORITHM_T ) {
                    Algorithm_t *alg = (Algorithm_t *) parOp;
                    if( (alg->AlgoType() == INDEXEDSELECT_T ||
                        alg->AlgoType() == INDEXEDNESTEDLOOPSJOIN_T) &&
                                                                inpNo == 0 ) {
                            if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) )
                                cout << pg->ID() << " : 0" << endl;
                            return 0;
                    }
                }
            }

            if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) )
                cout << pg->ID() << " : " << eq->UseCost() << endl;
            return eq->UseCost();
        }

        if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) )
            cout << pg->ID() << " : " << plan->CostVal() << endl;
        return plan->CostVal();
    } else {
        pg->UnMarkShared();
    }

    pg->Visit(travID);

    PhysicalOp_t *op = plan->Root();
    assert(op);

    CostVal_t costVal = op->LocalCostVal();

    if( op->Type() == ALGORITHM_T ) {
        Algorithm_t *alg = (Algorithm_t *) op;
        Predicate_t *pred = alg->Predicate();
        if( pred ) {
            // a filter operation
            InExpr_t *inPred = (InExpr_t *) pred->FindIn();
            if( inPred ) {
                // a nested query
                LogProp_t *lp = pg->LogicalProp();
                assert(lp);

                Card_t dist = inPred->NumDistinct(lp);
                assert(dist > 0);

                PlanGroup_t *nqPG = inPred->InpPlanGroup();
                assert(nqPG);
                CostVal_t nqFirstCostVal = SharCostTrav(travID,
                                                    NULL, 0, nqPG, matCost);
                costVal += nqFirstCostVal;
                if( dist > 0 ) {
                    CostVal_t nqNextCostVal = SharCostTrav(travID,
                                                    NULL, 0, nqPG, matCost);
                    costVal += (dist-1) * nqNextCostVal;
                }
            }
        }
    }

    int n = op->NumInputs();
    for( int i = 0 ; i < n ; i++ ) {
        PlanGroup_t *inp = NULL;
        inp = op->Input(i);
        assert(inp);

        costVal += SharCostTrav(travID, op, i, inp, matCost);
    }

    Cost_t *cost = plan->Cost();
    assert(cost);
    assert(cost->CostType() == COSTORD_T);

    CostOrd_t *costOrd = (CostOrd_t *) cost;
    costOrd->Assign(costVal);

    if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) )
        cout << pg->ID() << " : " << plan->CostVal() << endl;
    return costVal;
}

// find best plan for the expression dag rooted at rootEq
PlanGroup_t *VolcanoShar_t::FindBestPlan(Equivalence_t *rootEq, 
                            CostVal_t *costLimit, SortOrder_t *reqOrder)
{
    assert(rootEq);
    assert(costLimit);

    PlanGroup_t *pg = VolcanoOrd_t::FindBestPlan(rootEq, costLimit);
    if( pg ) {
        // plan found 
        if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) ) {
            Plan_t *bestPlan = pg->BestPlan();
            assert(bestPlan);
            assert(!bestPlan->IsFailed());

            bestPlan->PrintPlan();
        }

        int travID = PlanGroup_t::StartScan();
        MarkPlan(travID, pg);

        travID = PlanGroup_t::StartScan();
        ApplyLogicalSubs(travID, pg);

        if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) ) {
            Plan_t *bestPlan = pg->BestPlan();
            assert(bestPlan);
            assert(!bestPlan->IsFailed());

            bestPlan->PrintPlan();
        }

        *costLimit = VolcanoOrd_t::SharCost(pg);
    }

    return pg;
}

// mark the best plan for pg
int VolcanoShar_t::MarkPlan(int travID, PlanGroup_t *pg)
{
    assert(pg);

    Equivalence_t *eq = pg->EqClass();
    assert(eq);

    int isCorrelated = eq->IsCorrelated();

    if( pg->IsBuiltIn() ) {
        assert(!isCorrelated);
        pg->Mark();
        return 0;
    }

    if( pg->IsVisited(travID) )
        return isCorrelated;
    pg->Visit(travID);

    // pg's bestplan's cost assumed current
    Plan_t *pgBestPlan = pg->BestPlan();
    if( !pgBestPlan )
        pgBestPlan = pg->BestAlgPlan();
    assert(pgBestPlan);
    if( pgBestPlan->IsFailed() )
        pgBestPlan = pg->BestAlgPlan();

    assert(pgBestPlan);
    assert(!pgBestPlan->IsFailed());

    PhysicalOp_t *op = pgBestPlan->Root();
    assert(op);

    if( op->Type() == ALGORITHM_T ) {
        Algorithm_t *alg = (Algorithm_t *) op;
        Predicate_t *pred = alg->Predicate();
        if( pred ) {
            // a filter operation

            if( !isCorrelated ) {
                LogProp_t *lp = pg->LogicalProp();
                assert(lp);
                Schema_t *s = lp->Schema();
                assert(s);
                if( !pred->IsValidSchema(NULL, s) )
                    isCorrelated = 1;
            }

            InExpr_t *inPred = (InExpr_t *) pred->FindIn();
            if( inPred ) {
                // a nested query
                PlanGroup_t *nqPG = inPred->InpPlanGroup();
                assert(nqPG);

                MarkPlan(travID, nqPG);
            }
        }
    }

    int n = op->NumInputs();
    for( int i = 0 ; i < n ; i++ ) {
        PlanGroup_t *inp = op->Input(i);
        assert(inp);

        int isInpCorrelated = MarkPlan(travID, inp);
        if( !isCorrelated )
            isCorrelated = isInpCorrelated;
    }

    pg->Mark();

    if( isCorrelated )
        eq->MarkCorrelated();

    return isCorrelated;
}

// apply logical subsumption postpass on the optimal plan
void VolcanoShar_t::ApplyLocalLogicalSubs(int travID, PlanGroup_t *pg)
{
    assert(pg);

    if( !pg->IsMarked() )
        return;

    if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) ) {
        cout << "Applying local logical subsumption at SubDAG for planGroup " <<
                                                        pg->ID() << endl;
    }

    // pg's bestplan's cost assumed current
    Plan_t *pgBestPlan = pg->BestPlan();
    if( !pgBestPlan )
        pgBestPlan = pg->BestAlgPlan();
    assert(pgBestPlan);
    if( pgBestPlan->IsFailed() )
        pgBestPlan = pg->BestAlgPlan();

    assert(pgBestPlan);
    assert(!pgBestPlan->IsFailed());

    const List_t<PhysicalOp_t *>& parents = pg->AlgParents();
    int numPars = parents.Size();
    assert(numPars >= 0);

    // holds the parent select operators -- +1 to prevent zero allocation
    SelectScan_t **selList = new SelectScan_t *[numPars+1];

    // flags that the parent operator is covered by some other parent operator
    char *isCovered = new char[numPars+1];

    // number of parent selects
    int numSel = 0;

    ListIter_t<PhysicalOp_t *> iter;
    iter.Attach(&parents);

    while( !iter.IsEnd() ) {
        PhysicalOp_t *op = iter.Next();
        assert(op);

        Plan_t *plan = op->Plan();
        assert(plan);
        PlanGroup_t *parPG = plan->PlanGroup();

        if( !parPG->IsMarked() )
            continue;

        Plan_t *parBestPlan = parPG->BestPlan();
        if( !parBestPlan )
            parBestPlan = parPG->BestAlgPlan();
        assert(parBestPlan);
        if( parBestPlan->IsFailed() )
            parBestPlan = parPG->BestAlgPlan();

        assert(parBestPlan);
        assert(!parBestPlan->IsFailed());

        // check if the op is included in the best plan
        if( parBestPlan->Root()->ID() != op->ID() )
            continue;

        // update the cost of parPG's bestplan
        CostVal_t costVal = op->CostVal();

        int numInp = op->NumInputs();
        for( int i = 0 ; i < numInp ; i++ ) {
            PlanGroup_t *inpPG = op->Input(i);
            assert(inpPG);

            Plan_t *inpBestPlan = inpPG->BestPlan();
            if( !inpBestPlan )
                inpBestPlan = inpPG->BestAlgPlan();
            assert(inpBestPlan);
            if( inpBestPlan->IsFailed() )
                inpBestPlan = inpPG->BestAlgPlan();

            assert(inpBestPlan);
            assert(!inpBestPlan->IsFailed());

            assert(inpPG->IsMarked());

            Equivalence_t *inpEq = inpPG->EqClass();
            assert(inpEq);

            // not adding the mat cost here
            CostVal_t useCost = inpEq->UseCost();
            if( inpPG->IsIndexed() ) {
                if( op->Type() == ALGORITHM_T ) {
                    Algorithm_t *alg = (Algorithm_t *) op;
                    if( (alg->AlgoType() == INDEXEDSELECT_T ||
                        alg->AlgoType() == INDEXEDNESTEDLOOPSJOIN_T) &&
                                                                i == 0 ) {
                            useCost = 0;
                    }
                }
            }

            if( inpPG->IsShared() )
                costVal += useCost;
        }

        CostOrd_t *parCost = (CostOrd_t *) parBestPlan->Cost();
        assert(parCost);

        parCost->Assign(costVal);

        if( op->Type() == ALGORITHM_T ) {
            Algorithm_t *alg = (Algorithm_t *) op;

            if( alg->AlgoType() == SELECTSCAN_T ) {
                SelectScan_t *sel = (SelectScan_t *) alg;

                if( !sel->IsInProgress() ) {
                    selList[numSel++] = sel;
                    sel->MarkInProgress();

                    if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) ) {
                        cout << op->ID() << " ";
                        alg->PrintName();
                        cout << endl;
                    }
                }
            }
        }
    }

    if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) )
        cout << "NumSel = " << numSel << endl;

    int i = 0;
    for( i = 0 ; i < numSel ; i++ )
        isCovered[i] = 0;

    // for each pair of select parents, check if one subsumes the other;
    // if so, add the path due to subsumption
    int j = 0;
    for( i = 0 ; i < numSel ; i++ ) {
        Predicate_t *leftPred = selList[i]->Predicate();
        assert(leftPred);
        Plan_t *leftPlan = selList[i]->Plan();
        assert(leftPlan);
        PlanGroup_t *leftPG = leftPlan->PlanGroup();
        assert(leftPG);

        for( j = i+1 ; j < numSel ; j++ ) {
            // compare selList[i] and selList[j]
            Predicate_t *rightPred = selList[j]->Predicate();
            assert(rightPred);
            Plan_t *rightPlan = selList[j]->Plan();
            assert(rightPlan);
            PlanGroup_t *rightPG = rightPlan->PlanGroup();
            assert(rightPG);

            if( leftPred->Implies(rightPred) ) {
                isCovered[i] = AddSelectScan(pg, rightPG, selList[i]);
            } else if( rightPred->Implies(leftPred) ) {
                isCovered[j] = AddSelectScan(pg, leftPG, selList[j]);
            }
        }
    }

    // the covering predicate
    Predicate_t *pred = NULL;
    // number of components in the composite predicate
    int numSelComp = 0; 
    for( i = 0 ; i < numSel ; i++ ) {
        if( !isCovered[i] ) {
            // this predicate not covered by some other predicate
            Predicate_t *p = selList[i]->Predicate();
            assert(p);
            Plan_t *selPlan = selList[i]->Plan();
            assert(selPlan);
            PlanGroup_t *selPG = selPlan->PlanGroup();
            assert(selPG);

            if( !selPG->IsMarked() )
                continue;

            if( !pred ) {
                pred = p;
            } else {
                pred = new Or_t(p, pred);
            }
            numSelComp++;
        }
    }

    if( numSelComp > 1 ) {
        // put in a new subsuming select scan

        // instantiate the sel -- needed to find the logical props
        Refer(pred);

        Equivalence_t *eq = pg->EqClass();
        assert(eq);

        // subsuming selection
        Select_t *subSel = new Select_t(pred);
        subSel->MarkDerived();
        subSel->MarkExpanded();

        subSel->SetInput(0, eq);

        LogProp_t *outLP = subSel->LogicalProp();
        Refer(outLP);

        LogProp_t *inpLP = eq->LogicalProp();

        CostVal_t subSelCost = SelectScan_t::Cost(inpLP, outLP);

        Equivalence_t *subEq = new Equivalence_t(outLP);
        subEq->MarkSubsExpanded();
        subEq->MarkPruned();
        subEq->MarkExpanded();

        DeRefer(outLP);

        if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) )
            cout << "Created subsuming eq " << subEq->ID() << endl;

        subSel->SetEqClass(subEq);
        subEq->AddLogExpr(subSel);

        // leaving this out ... memo is now inconsistent
        // memo.Insert(subSel);

        PhyPropGroup_t *reqProp = pg->PropGroup();
        assert(reqProp);

        PlanGroup_t *subPG = subEq->PlanGroup(reqProp);
        assert(subPG);

        Cost_t *pgCost = pgBestPlan->Cost();
        assert(pgCost);

        SelectScan_t *subSelScan = new SelectScan_t(subSel);
        subSelScan->SetCostVal(subSelCost);
        subSelScan->SetInput(0, pg);

        // calculate the cost of the plan
        Cost_t *cost = CreateCost(subSelScan, subSelCost);
        Refer(cost);
        cost->Add(pgCost);

        // add the plan to the plan group as the best plan
        subPG->AddAlgPlan(subSelScan, cost);
        Plan_t *subBestPlan = new Plan_t(subPG, subSelScan, cost);
        DeRefer(cost);

        subPG->SetBestPlan(subBestPlan);

        // calculate the benefit
        if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) ) {
            cout << "subPG usecost = " << subPG->UseCost() << endl;
            cout << "subBestPlan costVal = " << subBestPlan->CostVal() << endl;
            cout << "pg usecost = " << pg->UseCost() << endl;
            cout << "pgBestPlan costVal = " << pgBestPlan->CostVal() << endl;
        }

        CostVal_t benefit = 0;
        if( eq->UseCost() != pgBestPlan->CostVal() ) {
            // not a base relation
            if( eq->UseCost() < pgBestPlan->CostVal() ) {
                benefit = (numSelComp-1)*eq->UseCost() +
                            pgBestPlan->CostVal() + eq->MatCost();
            } else {
                benefit = numSelComp * pgBestPlan->CostVal();
            }
        } else {
            // benefit = (numSelComp-1)*eq->UseCost();
            benefit = numSelComp*eq->UseCost();
        }
                        
        if( subEq->UseCost() < subBestPlan->CostVal() ) {
            benefit -= (numSelComp-1)*subEq->UseCost() +
                    subBestPlan->CostVal() + subEq->MatCost();
        } else {
            benefit -= numSelComp * subBestPlan->CostVal();
        }

        if( benefit > 0 ) {
            // now add the subsumed selects
            for( i = 0 ; i < numSel ; i++ ) {
                SelectScan_t *selOp = selList[i];
                assert(selOp);

                // not changing any parents etc here -- so the physical
                // DAG is inconsistent
                selOp->SetInput(0, subPG);

                Plan_t *selPlan = selList[i]->Plan();
                assert(selPlan);
                PlanGroup_t *selPG = selPlan->PlanGroup();
                assert(selPG);
                assert(selPG->IsMarked());

                Plan_t *selBestPlan = selPG->BestPlan();
                if( !selBestPlan )
                    selBestPlan = selPG->BestAlgPlan();
                assert(pgBestPlan);
                if( selBestPlan->IsFailed() )
                    selBestPlan = selPG->BestAlgPlan();

                assert(selBestPlan);
                assert(!selBestPlan->IsFailed());

                Cost_t *cost = selBestPlan->Cost();

                CostVal_t costVal = selOp->CostVal();
                if( subPG->UseCost() < subBestPlan->CostVal() )
                    costVal += subPG->UseCost();
                else costVal += subBestPlan->CostVal();

                ((CostOrd_t *) cost)->Assign(costVal);
            }

            subPG->Visit(travID);
            subPG->Mark();
            subPG->MarkShared();
            pg->UnMarkShared();
        }

        DeRefer(pred);
    }

    delete[] selList;
    delete[] isCovered;

    if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) ) {
        cout << "Finished Applying local logical subsumption at SubDAG for pg "
            << pg->ID() << endl;
    }
}

int VolcanoShar_t::AddSelectScan(PlanGroup_t *prevInpPG,
                                    PlanGroup_t *newInpPG, SelectScan_t *op)
{
    assert(prevInpPG);
    assert(newInpPG);
    assert(op);

    // best plans costs for prevInpPG and newInpPG assumed current
    Plan_t *prevInpBestPlan = prevInpPG->BestPlan();
    if( !prevInpBestPlan )
        prevInpBestPlan = prevInpPG->BestAlgPlan();
    assert(prevInpBestPlan);
    if( prevInpBestPlan->IsFailed() )
        prevInpBestPlan = prevInpPG->BestAlgPlan();

    assert(prevInpBestPlan);
    assert(!prevInpBestPlan->IsFailed());

    Plan_t *newInpBestPlan = newInpPG->BestPlan();
    if( !newInpBestPlan )
        newInpBestPlan = newInpPG->BestAlgPlan();
    assert(newInpBestPlan);
    if( newInpBestPlan->IsFailed() )
        newInpBestPlan = newInpPG->BestAlgPlan();

    assert(newInpBestPlan);
    assert(!newInpBestPlan->IsFailed());

    // calculate the benefit
    Equivalence_t *prevInpEq = prevInpPG->EqClass();
    assert(prevInpEq);
    Equivalence_t *newInpEq = newInpPG->EqClass();
    assert(newInpEq);

    CostVal_t benefit = 0;
    if( prevInpEq->UseCost() < prevInpBestPlan->CostVal() )
        benefit = prevInpEq->UseCost();
    else benefit = prevInpBestPlan->CostVal();

    if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) ) {
        cout << "PrevInpPG = " << prevInpPG->ID() << 
                    " newInpPG = " << newInpPG->ID() <<
                            " benefit = " << benefit << endl;
    }

    if( newInpEq->UseCost() < newInpBestPlan->CostVal() )
        benefit -= newInpEq->UseCost();
    else benefit -= newInpBestPlan->CostVal();
                    
    if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) ) {
        cout << "PrevInpPG = " << prevInpPG->ID() << 
                    " newInpPG = " << newInpPG->ID() <<
                            " benefit = " << benefit << endl;
    }

    if( benefit > 0 ) {
        // not changing any parents etc here -- so the physical
        // DAG is inconsistent
        op->SetInput(0, newInpPG);

        Plan_t *selPlan = op->Plan();
        assert(selPlan);
        PlanGroup_t *selPG = selPlan->PlanGroup();
        assert(selPG);
        assert(selPG->IsMarked());

        Plan_t *selBestPlan = selPG->BestPlan();
        if( !selBestPlan )
            selBestPlan = selPG->BestAlgPlan();
        assert(selBestPlan);
        if( selBestPlan->IsFailed() )
            selBestPlan = selPG->BestAlgPlan();

        assert(selBestPlan);
        assert(!selBestPlan->IsFailed());

        CostOrd_t *cost = (CostOrd_t *) selBestPlan->Cost();

        CostVal_t costVal = op->CostVal();
        if( newInpEq->UseCost() < selBestPlan->CostVal() )
            costVal += newInpEq->UseCost();
        else costVal += selBestPlan->CostVal();

        cost->Assign(costVal);

        // do checks here
        prevInpPG->UnMarkShared();
        newInpPG->MarkShared();

        return 1;
    }

    return 0;
}

// apply subsumption on the best plan
void VolcanoShar_t::ApplyLogicalSubs(int travID, PlanGroup_t *pg)
{
    assert(pg);

    if( !pg->IsMarked() )
        return;

    if( pg->IsVisited(travID) )
        return;
    pg->Visit(travID);

    if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) ) {
        cout << "Applying logical subsumption at SubDAG for pg " <<
                                                        pg->ID() << endl;
    }

    // this is for bottom-up application of subsumption
    const List_t<Plan_t *>& planList = pg->AlgPlans();

    ListIter_t<Plan_t *> iter;
    iter.Attach(&planList);

    while( !iter.IsEnd() ) {
        Plan_t *plan = iter.Next();
        assert(plan);

        PhysicalOp_t *op = plan->Root();
        assert(op);

        if( op->Type() == ALGORITHM_T ) {
            Algorithm_t *alg = (Algorithm_t *) op;
            Predicate_t *pred = alg->Predicate();
            if( pred ) {
                // a filter operation
                InExpr_t *inPred = (InExpr_t *) pred->FindIn();
                if( inPred ) {
                    // a nested query
                    PlanGroup_t *nqPG = inPred->InpPlanGroup();
                    assert(nqPG);

                    ApplyLogicalSubs(travID, nqPG);
                }
            }
        }

        int n = op->NumInputs();
        for( int i = 0 ; i < n ; i++ ) {
            PlanGroup_t *childPG = op->Input(i);
            assert(childPG);

            ApplyLogicalSubs(travID, childPG);
        }
    }

    // apply the subsumption transformations locally
    ApplyLocalLogicalSubs(travID, pg);

    if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) ) {
        cout << "Finished Applying logical subsumption at SubDAG for pg " <<
                                                            pg->ID() << endl;
        cout << endl;
    }
}
