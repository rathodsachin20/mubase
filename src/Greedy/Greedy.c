// Greedy heuristic for DAG optimization

#include "typedefs.h"
#include "Cost.h"
#include "List.h"
#include "Heap.h"
#include "PhysicalOp.h"
#include "Volcano.h"
#include "DAGOpt.h"
#include "HeapKey.h"
#include "Greedy.h"

int propCount = 0, numProps = 0;
int numRecomp = 0, numMaterialized = 0;

CostVal_t AbsThreshold = 0.005;
CostVal_t RelThreshold = 0.000;

// register that the cost of the plan group has changed
void Greedy_t::RegisterChange(Equivalence_t *eq)
{
    // the equivalence class can be NULL --- this means
    // that it has been deleted and the plangroup parPG is
    // unattached to any equivalence class.
    // In this case, the path is truncated
    if( !eq ) return;

    propCount++;

    // check if already in the heap; if so, ignore
    if( eq->IsAffected() )
        return;

    eq->MarkAffected();

    if( Config_t::IsDebug(7) )
        cout << "Registered equivalence class = " << eq->ID() <<
                                    " TopNo = " << eq->TopNo() << endl;

    PropKey_t propKey(eq);
    propHeap.Insert(propKey);
}

// propagate changes -- calls propagate until no more changes to propagate
// does not propagate along infeasible eq nodes
void Greedy_t::PropagateChanges(CostVal_t bestCost)
{
    while( !propHeap.IsEmpty() ) {
        PropKey_t& propKey = propHeap.Top();

        Equivalence_t *eq = propKey.EqClass();
        assert(eq);
        eq->UnMarkAffected();

        propHeap.DeleteTop();

        // eq cannot occur in the best plan if its lower bound is
        // greater than the current best plan cost
        if( eq->CostLB() > bestCost ) {
            if( Config_t::IsDebug(7) )
                cout << "Pruning off equivalence class = " << eq->ID() <<
                                    " TopNo = " << eq->TopNo() << endl;
            continue;
        }

        Propagate(eq);
    }

    numRecomp++;
}

// Recompute the cost of the plangroups and propagate the update to plans that
// can use it.
// Iterate over all operators having the pgs as inputs and update the cost
// of the corresponding plans. Then, for the updated plan check with each
// plangroup that can have the plan as a valid plan (i.e. the plan
// groups for the same equivalence class whose physical properties are
// covered by the physical property of the plan group of the updated
// plan)
void Greedy_t::Propagate(Equivalence_t *eq)
{
    assert(eq);

    if( Config_t::IsDebug(8) )
        cout << "Propagating to equivalence class = " << eq->ID() <<
                                    " TopNo = " << eq->TopNo() << endl;

    PlanGroup_t *pg = eq->PlanGroupBase();
    int travID = -1;

    for( int k = 0 ; k < PGTABSIZE ; k++ ) {
        pgList[k] = NULL;
        bestExtPlan[k] = -1;
        superIndex[k] = k;
        bestPlan[k] = -1;
        for( int i = 0 ; i < PGTABSIZE ; i++ )
            table[k][i] = NULL;
    }

    // update the best alg plans -- should be done before updating enf
    // plans
    if( Config_t::IsDebug() ) 
        cout << "Updating best algorithm plans ..." << endl;
    travID = pg->StartScan();
    UpdateBestAlgPlans(travID, pg);

    if( Config_t::IsDebug() ) 
        cout << "Creating table ..." << endl;
    travID = pg->StartScan();
    CreateTable(travID, pg);

    if( Config_t::IsDebug() ) 
        cout << "Table Closure ..." << endl;
    IntTableClosure(pg);

    if( Config_t::IsDebug() ) 
        cout << "Finding MCA ..." << endl;
    FindMinCostArb();

    if( Config_t::IsDebug() ) 
        cout << "Setting best plans ..." << endl;
    SetBestPlans();

    if( Config_t::IsDebug() ) 
        cout << "Updating parents ..." << endl;
    UpdateParents();

    if( Config_t::IsDebug() ) {
        for( int i = 1 ; i < PGTABSIZE ; i++ ) {
            if( !pgList[i] )
                break;
            assert(bestPlan[i] != -1);
            cout << "Plan for " << pgList[i]->ID() << " is ";
            if( bestPlan[i] )
                cout << " enforcer " << pgList[bestPlan[i]]->ID() << endl;
            else cout << " algorithm" << endl;
        }
    }
}

// update the best alg plan for the plan group
Plan_t *Greedy_t::UpdateBestAlgPlans(int travID, PlanGroup_t *pg)
{
    assert(pg);

    Plan_t *bestAlgPlan = pg->BestAlgPlan();
    assert(bestAlgPlan);

    if( pg->IsVisited(travID) )
        return bestAlgPlan;
    pg->Visit(travID);

    if( Config_t::IsDebug(10) ) {
        if( !bestAlgPlan->IsFailed() ) {
            cout << "[" << pg->EqClass()->ID() << ":" << pg->ID() <<
            "] old best alg plan op = " << bestAlgPlan->Root()->ID() <<
            " cost = " << bestAlgPlan->CostVal() << endl;
        }
    }

    bestAlgPlan->SetRoot(NULL);
    bestAlgPlan->SetCost(NULL);

    POEdgeList_t& super = pg->SuperList();
    POEdgeIter_t pgIter;
    pgIter.Attach(&super);

    Plan_t *covBestPlan = NULL;
    PlanGroup_t *nextPG = NULL;
    while( (nextPG = pgIter.Next()) ) {
        Plan_t *nextCovBestPlan = UpdateBestAlgPlans(travID, nextPG);

        if( !nextCovBestPlan )
            continue;
        if( nextCovBestPlan->IsFailed() )
            continue;

        if( covBestPlan ) 
            if( !covBestPlan->IsFailed() ) 
                if( nextCovBestPlan->CostVal() >= covBestPlan->CostVal() )
                    continue;

        covBestPlan = nextCovBestPlan;
    }

    pgIter.Detach();

    /* -- NOT NECESSARY -- PRASAN
    if( covBestPlan ) {
        bestAlgPlan->SetRoot(covBestPlan->Root());
        bestAlgPlan->SetCost(covBestPlan->Cost());
    }
    -- NOT NECESSARY -- */

    List_t<Plan_t *>& list = pg->AlgPlans();
    ListIter_t<Plan_t *> iter;
    iter.Attach(&list);

    // iterate for each plan under the plan group
    while( !iter.IsEnd() ) {
        Plan_t *plan = iter.Next();
        assert(plan);
        assert(!plan->IsFailed());

        if( !bestAlgPlan->IsFailed() ) {
            if( plan->CostVal() > bestAlgPlan->CostVal() )
                continue;
        }

        bestAlgPlan->SetRoot(plan->Root());
        bestAlgPlan->SetCost(plan->Cost());
    }

    if( Config_t::IsDebug(10) ) {
        if( !bestAlgPlan->IsFailed() ) {
            cout << "[" << pg->EqClass()->ID() << ":" << pg->ID() <<
            "] new best alg plan op = " << bestAlgPlan->Root()->ID() <<
            " cost = " << bestAlgPlan->CostVal() << endl;
        }
    }

    return bestAlgPlan;
}

// create the table for min cost arb computation
void Greedy_t::CreateTable(int travID, PlanGroup_t *pg)
{
    assert(pg);

    if( pg->IsVisited(travID) )
        return;
    pg->Visit(travID);

    int pgIndex = Index(pg);
    
    ListIter_t<Plan_t *> iter;

    List_t<Plan_t *>& enfPlans = pg->EnfPlans();
    iter.Attach(&enfPlans);

    while( !iter.IsEnd() ) {
        Plan_t *plan = iter.Next();
        assert(plan);
        
        PhysicalOp_t *op = plan->Root();
        assert(op);
        assert(op->Type() == ENFORCER_T);

        PlanGroup_t *childPG = op->Input(0);
        assert(childPG);

        int childIndex = Index(childPG);
        table[childIndex][pgIndex] = plan;
    }

    POEdgeList_t& super = pg->SuperList();
    POEdgeIter_t pgIter;
    pgIter.Attach(&super);

    PlanGroup_t *nextPG = NULL;
    while( (nextPG = pgIter.Next()) )
        CreateTable(travID, nextPG);

    pgIter.Detach();
}

// take closure of the plans
void Greedy_t::IntTableClosure(PlanGroup_t *pg)
{
    assert(pg);

    int pgIndex = Index(pg);

    POEdgeList_t& super = pg->SuperList();
    POEdgeIter_t pgIter;
    pgIter.Attach(&super);

    PlanGroup_t *nextPG = NULL;
    while( (nextPG = pgIter.Next()) ) {
        int parIndex = Index(nextPG);

        for( int i = 1 ; i < PGTABSIZE ; i++ ) {
            if( !pgList[i] )
                break;

            if( !table[parIndex][i] && table[pgIndex][i] )
                table[parIndex][i] = table[pgIndex][i];
        }

        IntTableClosure(nextPG);
    }

    pgIter.Detach();
}

// find min cost arb
void Greedy_t::FindMinCostArb(void)
{
    int bestPG[PGTABSIZE];
    int bestInpPG[PGTABSIZE];
    CostVal_t bestCost[PGTABSIZE];

    if( Config_t::IsDebug() ) {
        static int debug = 0;
        debug++;
        flush(cout << "DEBUG : " << debug << endl);
    }

    for( ; ; ) {
        int done = 1;

        // initialize the best plans for each super node
        for( int k = 0 ; k < PGTABSIZE ; k++ ) {
            if( pgList[k] && SuperPG(k) == k ) {
                // representative for a superNode is the pg with the
                // smallest cost ext plan
                bestPG[k] = k;
                bestInpPG[k] = 0;
            } else {
                bestPG[k] = -1;
                bestInpPG[k] = -1;
            }
            bestCost[k] = -1;
        }

        // supernode containing the acyclic component
        int acyclicSuperPG = SuperPG(0);

        // find the best plan for each cyclic component
        for( int i = 1 ; i < PGTABSIZE ; i++ ) {
            if( !pgList[i] )
                break;
            if( !pgList[i]->IsMarked() )
                continue;

            int superPG = SuperPG(i);

            // ignore the acyclic component
            if( superPG == acyclicSuperPG )
                continue;

            bestCost[superPG] = ComputationCost(bestInpPG[superPG],
                                                        bestPG[superPG]);
            // input plan
            for( int j = 1 ; j < PGTABSIZE ; j++ ) {
                if( !pgList[j] )
                    break;
                if( !pgList[j]->IsMarked() )
                    continue;

                // only consider the inputs from outside the super node
                int inpSuperPG = SuperPG(j);
                if( inpSuperPG == superPG )
                    continue;

                CostVal_t newBestCost = ComputationCost(j, i);
                assert(newBestCost != -1);

                if( bestCost[superPG] == -1 ||
                        bestCost[superPG] > newBestCost ) {
                    bestPG[superPG] = i;
                    bestInpPG[superPG] = j;
                    bestCost[superPG] = newBestCost;
                }
            }

            done = 0;
        }

        // no cyclic component found
        if( done ) return;

        int j = -1;
        // update the best plans
        for( j = 1 ; j < PGTABSIZE ; j++ ) {
            if( !pgList[j] )
                break;
            if( !pgList[j]->IsMarked() )
                continue;

            // consider only the supernode representatives
            if( SuperPG(j) != j )
                continue;

            assert(bestCost[j] != -1);

            // note the min cost plan
            assert(bestPG[j] > 0);
            assert(bestPG[j] < PGTABSIZE);
            assert(bestInpPG[j] >= 0);
            assert(bestInpPG[j] < PGTABSIZE);
            bestPlan[bestPG[j]] = bestInpPG[j];
        }

        // merge the supernodes for the higher level
        for( j = 1 ; j < PGTABSIZE ; j++ ) {
            if( !pgList[j] )
                break;
            if( !pgList[j]->IsMarked() )
                continue;

            // consider only the supernode representatives
            if( SuperPG(j) != j )
                continue;

            // merge with the supernode of input, if not already merged
            int inpPG = bestInpPG[j];
            int inpSuperPG = SuperPG(inpPG);

            if( inpSuperPG != j )
                MergeSuperPG(inpSuperPG, j);
        }
    }
}

void Greedy_t::SetBestPlans(void)
{
    int i = -1;

    // marked nodes first
    for( i = 1 ; i < PGTABSIZE ; i++ ) {
        PlanGroup_t *pg = pgList[i];
        if( !pg )
            break;
        if( pg->IsMarked() )
            SetBestPlan(i);
    }

    // unmarked nodes next
    for( i = 1 ; i < PGTABSIZE ; i++ ) {
        PlanGroup_t *pg = pgList[i];
        if( !pg )
            break;
        if( !pg->IsMarked() )
            SetBestPlan(i);
    }

    // sanity check and clean up
    for( i = 1 ; i < PGTABSIZE ; i++ ) {
        PlanGroup_t *pg = pgList[i];
        if( !pg )
            break;
        assert(pg->IsInProgress());
        pg->DecrInProgress();
    }
}

Plan_t *Greedy_t::SetBestPlan(int pgIndex)
{
    assert(pgIndex >= 1 && pgIndex < PGTABSIZE);
    PlanGroup_t *pg = pgList[pgIndex];
    if( !pg )
        return NULL;

    if( !pg->IsMarked() ) 
        return SetBestUnMarkedPlan(pgIndex);

    assert(bestPlan[pgIndex] >= 0 && bestPlan[pgIndex] < PGTABSIZE);

    if( bestPlan[pgIndex] == 0 ) {
        // best plan is the min cost ext plan
        return SetBestExtPlan(pgIndex);
    }

    // best plan is through an enforcer to a marked node

    Plan_t *pgBestPlan = pg->BestPlan();

    // check if already considered
    if( pg->IsInProgress() ) {
        assert(pgBestPlan);
        assert(!pgBestPlan->IsFailed());

        return pgBestPlan;
    }
    pg->IncrInProgress();

    if( !pgBestPlan ) {
        pgBestPlan = new Plan_t(pg);
        pg->SetBestPlan(pgBestPlan);
    } else {
        pgBestPlan->SetRoot(NULL);
        pgBestPlan->SetCost(NULL);
    }

    if( Config_t::IsDebug(8) ) {
        if( !pgBestPlan->IsFailed() ) {
            cout << "[" << pg->EqClass()->ID() << ":" << pg->ID() << "]" << 
                " old best plan op = " << pgBestPlan->Root()->ID() <<
                " cost = " << pgBestPlan->CostVal() << endl;
            pgBestPlan->Root()->PrintName();
            cout << endl;
        }
    }

    if( table[bestPlan[pgIndex]][pgIndex] ) {
        // build up the plan through the enforcer

        Plan_t *plan = table[bestPlan[pgIndex]][pgIndex];
        PlanGroup_t *inpPG = pgList[bestPlan[pgIndex]];
        assert(inpPG);
        assert(inpPG->IsMarked());

        PhysicalOp_t *op = plan->Root();
        assert(op);
        op->SetActualInput(0, inpPG);

        Equivalence_t *inpEq = inpPG->EqClass();
        Cost_t *inpCost = inpEq->UseCostADT(&volcano);
        assert(inpCost);

        CostOrd_t *planCost = (CostOrd_t *) inpCost->Clone();
        planCost->Add(op->CostVal());

        pgBestPlan->SetRoot(plan->Root());
        pgBestPlan->SetCost(planCost);
    } else {
        // should not be chosen by the parent ...
        assert(table[pgIndex][bestPlan[pgIndex]]);

        Plan_t *plan = SetBestPlan(bestPlan[pgIndex]);
        assert(plan);
        assert(!plan->IsFailed());

        pgBestPlan->SetRoot(plan->Root());
        pgBestPlan->SetCost(plan->Cost());
    }

    assert(!pgBestPlan->IsFailed());

    if( Config_t::IsDebug(8) ) {
        cout << "[" << pg->EqClass()->ID() << ":" << pg->ID() << "]" << 
            " new best plan op = " << pgBestPlan->Root()->ID() <<
            " cost = " << pgBestPlan->CostVal() << endl;
        pgBestPlan->Root()->PrintName();
        cout << endl;
    }

    return pgBestPlan;
}

// best plan for unmarked pg node
Plan_t *Greedy_t::SetBestUnMarkedPlan(int pgIndex)
{
    assert(pgIndex >= 1 && pgIndex < PGTABSIZE);
    PlanGroup_t *pg = pgList[pgIndex];
    if( !pg )
        return NULL;

    Plan_t *pgBestPlan = pg->BestPlan();

    // check if already considered
    if( pg->IsInProgress() ) {
        assert(pgBestPlan);
        assert(!pgBestPlan->IsFailed());

        return pgBestPlan;
    }
    pg->IncrInProgress();

    if( !pgBestPlan ) {
        pgBestPlan = new Plan_t(pg);
        pg->SetBestPlan(pgBestPlan);
    } else {
        pgBestPlan->SetRoot(NULL);
        pgBestPlan->SetCost(NULL);
    }

    if( Config_t::IsDebug(8) ) {
        if( !pgBestPlan->IsFailed() ) {
            cout << "[" << pg->EqClass()->ID() << ":" << pg->ID() << "]" << 
                " old best plan op = " << pgBestPlan->Root()->ID() <<
                " cost = " << pgBestPlan->CostVal() << endl;
        }
    }

    Plan_t *pgBestAlgPlan = pg->BestAlgPlan();
    if( pgBestAlgPlan ) {
        if( !pgBestAlgPlan->IsFailed() ) {
            pgBestPlan->SetRoot(pgBestAlgPlan->Root());
            pgBestPlan->SetCost(pgBestAlgPlan->Cost());
        }
    }

    for( int i = 1 ; i < PGTABSIZE ; i++ ) {
        if( i == pgIndex )
            continue;

        PlanGroup_t *inpPG = pgList[i];
        if( !inpPG )
            break;

        CostVal_t costVal = -1;
        if( !pgBestPlan->IsFailed() ) 
            costVal = pgBestPlan->CostVal();

        if( table[i][pgIndex] ) {
            // the plan through the enforcer
            Plan_t *plan = table[i][pgIndex];

            Cost_t *inpCost = NULL;
            if( inpPG->IsMarked() ) {
                Equivalence_t *inpEq = inpPG->EqClass();
                assert(inpEq);

                inpCost = inpEq->UseCostADT(&volcano);
            } else {
                // assuming triangle inequality --- implies that the
                // best alg plan is the optimal for the optimal input
                Plan_t *inpPlan = inpPG->BestAlgPlan();
                if( !inpPlan )
                    continue;
                if( inpPlan->IsFailed() )
                    continue;

                inpCost = inpPlan->Cost();
            }

            assert(inpCost);

            PhysicalOp_t *op = plan->Root();
            assert(op);

            if( costVal == -1 ||
                    costVal > inpCost->Value() + op->CostVal() ) {
                CostOrd_t *cost = (CostOrd_t *) inpCost->Clone();
                cost->Add(op->CostVal());

                op->SetActualInput(0, inpPG);
                pgBestPlan->SetRoot(op);
                pgBestPlan->SetCost(cost);
            }
        } else if( table[pgIndex][i] ) {
            Plan_t *inpBestPlan = SetBestPlan(i);
            // must have already been computed if inpPG is marked;
            // otherwise gets computed without recursion

            assert(inpBestPlan);
            assert(!inpBestPlan->IsFailed());

            if( costVal == -1 || costVal > inpBestPlan->CostVal() ) {
                pgBestPlan->SetRoot(inpBestPlan->Root());
                pgBestPlan->SetCost(inpBestPlan->Cost());
            }
        }
    }

    if( Config_t::IsDebug(8) ) {
        cout << "[" << pg->EqClass()->ID() << ":" << pg->ID() << "]" << 
            " new best unmarked plan op = " << pgBestPlan->Root()->ID() <<
            " cost = " << pgBestPlan->CostVal() << endl;
    }

    return pgBestPlan;
}

// set the best external plan
Plan_t *Greedy_t::SetBestExtPlan(int pgIndex)
{
    assert(pgIndex >= 1 && pgIndex < PGTABSIZE);
    PlanGroup_t *pg = pgList[pgIndex];
    if( !pg )
        return NULL;

    Plan_t *pgBestPlan = pg->BestPlan();

    // check if already considered
    if( pg->IsInProgress() ) {
        assert(pgBestPlan);
        assert(!pgBestPlan->IsFailed());

        return pgBestPlan;
    }
    pg->IncrInProgress();

    if( !pgBestPlan ) {
        pgBestPlan = new Plan_t(pg);
        pg->SetBestPlan(pgBestPlan);
    } else {
        pgBestPlan->SetRoot(NULL);
        pgBestPlan->SetCost(NULL);
    }

    if( Config_t::IsDebug(8) ) {
        if( !pgBestPlan->IsFailed() ) {
            cout << "[" << pg->EqClass()->ID() << ":" << pg->ID() << "]" << 
                " old best plan op = " << pgBestPlan->Root()->ID() <<
                " cost = " << pgBestPlan->CostVal() << endl;
        }
    }

    if( bestExtPlan[pgIndex] == 0 ) {
        // algorithm plan
        Plan_t *pgBestAlgPlan = pg->BestAlgPlan();
        assert(pgBestAlgPlan);
        assert(!pgBestAlgPlan->IsFailed());

        pgBestPlan->SetRoot(pgBestAlgPlan->Root());
        pgBestPlan->SetCost(pgBestAlgPlan->Cost());
    } else {
        assert(bestExtPlan[pgIndex] >= 0 && bestExtPlan[pgIndex] < PGTABSIZE);
        Plan_t *inpBestPlan = SetBestExtPlan(bestExtPlan[pgIndex]);
        assert(inpBestPlan);
        assert(!inpBestPlan->IsFailed());

        Plan_t *plan = table[bestExtPlan[pgIndex]][pgIndex];

        if( plan ) {
            // best plan is op followed by inpBestPlan
            Cost_t *inpCost = inpBestPlan->Cost();
            assert(inpCost);
            CostOrd_t *cost = (CostOrd_t *) inpCost->Clone();

            PhysicalOp_t *op = plan->Root();
            assert(op);
            cost->Add(op->CostVal());

            op->SetActualInput(0, pgList[bestExtPlan[pgIndex]]);

            pgBestPlan->SetRoot(op);
            pgBestPlan->SetCost(cost);
        } else {
            // pgList[bestExtPlan[pgIndex]] subsumes pgList[pgIndex] (== pg)
            pgBestPlan->SetRoot(inpBestPlan->Root());
            pgBestPlan->SetCost(inpBestPlan->Cost());
        }
    }

    if( Config_t::IsDebug(8) ) {
        cout << "[" << pg->EqClass()->ID() << ":" << pg->ID() << "]" << 
            " new best ext plan op = " << pgBestPlan->Root()->ID() <<
            " cost = " << pgBestPlan->CostVal() << endl;
    }

    return pgBestPlan;
}

// update the parent plans and the containing equivalence classes, if necessary
void Greedy_t::UpdateParents(void)
{
    for( int i = 1 ; i < PGTABSIZE ; i++ ) {
        PlanGroup_t *pg = pgList[i];
        if( !pg )
            break;

        List_t<PhysicalOp_t *>& algParents = pg->AlgParents();
        ListIter_t<PhysicalOp_t *> iter;

        iter.Attach(&algParents);

        if( Config_t::IsDebug(8) ) {
            cout << "--- Updating parents of pg " << pg->ID() << endl;
        }

        while( !iter.IsEnd() ) {
            PhysicalOp_t *op = iter.Next();
            assert(op);

            Plan_t *parPlan = op->Plan();
            assert(parPlan);

            PlanGroup_t *parPG = parPlan->PlanGroup();
            assert(parPG);

            if( Config_t::IsDebug(10) ) {
                cout << "Propagating through [" <<
                    parPG->EqClass()->ID() << ":" << parPG->ID() <<
                                        "] op = " << op->ID() << endl;
            }

            // update the cost of the parent plan
            RecomputeAlgCost(parPlan);

            Plan_t *parBestAlgPlan = parPG->BestAlgPlan();
            assert(parBestAlgPlan);

            PhysicalOp_t *parRoot = parPlan->Root();
            assert(parRoot);
            PhysicalOp_t *parBestAlgRoot = parBestAlgPlan->Root();
            assert(parBestAlgRoot);

            if( Config_t::IsDebug() ) {
                cout << "Plan Root " << parRoot->ID() <<
                            " Cost = " << parPlan->CostVal() << endl;
                cout << "Parent PG Best Alg Plan Root " <<
                                        parBestAlgRoot->ID() << endl;
            }

            // will not make any difference if the following is true
            if( parRoot->ID() == parBestAlgRoot->ID() ) {
                if( parPlan->CostVal() == parBestAlgPlan->CostVal() ) {
                    if( Config_t::IsDebug(8) )
                        cout << "Ignoring Parent PG " << parPG->ID() << endl;
                    continue;
                }
            } else {
                if( parPlan->CostVal() >= parBestAlgPlan->CostVal() ) {
                    if( Config_t::IsDebug(8) )
                        cout << "Ignoring Parent PG " << parPG->ID() << endl;
                    continue;
                }
            }

            Equivalence_t *eq = parPG->EqClass();
            RegisterChange(eq);

            if( Config_t::IsDebug(8) ) {
                cout << "Updating Parent [" << eq->ID() << ":" <<
                                                parPG->ID() << "]" << endl;
            }
        }
    }
}

// supernode representative
int Greedy_t::SuperPG(int i)
{
    assert(i >= 0 && i < PGTABSIZE);

    if( superIndex[i] != i )
        superIndex[i] = SuperPG(superIndex[i]);

    return superIndex[i];
}

// merge supernodes
// superPG for a superNode is the pg with the smallest cost ext plan
void Greedy_t::MergeSuperPG(int i, int j)
{
    assert(i >= 0);
    assert(i < PGTABSIZE);
    assert(j >= 1);
    assert(j < PGTABSIZE);

    if( i == 0 )
        superIndex[j] = i;
    else if( ComputationCost(0, i) < ComputationCost(0, j) )
        superIndex[j] = i;
    else superIndex[i] = j;
}

// cost for an entry in the plan matrix
CostVal_t Greedy_t::ComputationCost(int rowIndex, int colIndex)
{
    assert(rowIndex >= 0 && rowIndex < PGTABSIZE);
    assert(colIndex >= 1 && colIndex < PGTABSIZE);
    assert(rowIndex != colIndex);

    assert(pgList[colIndex]);

    if( rowIndex == 0 ) {
        // best external plan cost
        PlanGroup_t *pg = pgList[colIndex];
        assert(pg);

        if( bestExtPlan[colIndex] != -1 ) {
            // already computed
            if( bestExtPlan[colIndex] == 0 ) {
                Plan_t *plan = pg->BestAlgPlan();
                assert(plan);
                assert(!plan->IsFailed());

                return plan->CostVal();
            }

            CostVal_t tailCost = ComputationCost(0, bestExtPlan[colIndex]);
            assert(tailCost != -1);

            return tailCost + ComputationCost(bestExtPlan[colIndex], colIndex);
        }

        // compute the best external plan

        CostVal_t bestCost = -1;
        Plan_t *bestAlgPlan = pg->BestAlgPlan();
        if( bestAlgPlan ) {
            if( !bestAlgPlan->IsFailed() ) {
                // initialized to best alg plan -- assumed updated
                bestExtPlan[colIndex] = 0;
                bestCost = bestAlgPlan->CostVal();
            }
        }

        // actually, should be the shortest path --- the below is based
        // on the assumption of triangle-inequality
        for( int i = 1 ; i < PGTABSIZE ; i++ ) {
            if( i == colIndex )
                continue;

            PlanGroup_t *inpPG = pgList[i];
            if( !inpPG )
                break;
            if( inpPG->IsMarked() )
                continue;

            // only if a plan is possible between i and colIndex
            if( !table[i][colIndex] && !table[colIndex][i] )
                continue;

            Plan_t *inpBestAlgPlan = inpPG->BestAlgPlan();
            if( !inpBestAlgPlan )
                continue;
            if( inpBestAlgPlan->IsFailed() )
                continue;
                
            CostVal_t cost = inpBestAlgPlan->CostVal() +
                                        ComputationCost(i, colIndex);
            if( bestCost == -1 || cost < bestCost ) {
                bestCost = cost;
                bestExtPlan[colIndex] = i;
            }
        }

        // reflect the above choice of path (enf followed by alg)
        if( bestExtPlan[colIndex] > 0 )
            bestExtPlan[bestExtPlan[colIndex]] = 0;

        return bestCost;
    }

    PlanGroup_t *pg = pgList[rowIndex];
    assert(pg);

    if( !table[rowIndex][colIndex] ) {
        // no enforcer either way
        if( !table[colIndex][rowIndex] )
            return Config_t::CostLimit(); // +inf

        // pgList[rowIndex] subsumes pgList[colIndex]
        // assuming that pgList[rowIndex] can be directly obtained from
        // pgList[colIndex]
        // return 0 if pgList[rowIndex[ has a best algorithm plan
        // +inf otherwise

        Plan_t *pgBestAlgPlan = pg->BestAlgPlan();
        if( pgBestAlgPlan ) {
            if( !pgBestAlgPlan->IsFailed() )
                return 0;
        }

        return Config_t::CostLimit();   // +inf
    }

    Plan_t *p = table[rowIndex][colIndex];
    assert(p);

    PhysicalOp_t *op = p->Root();
    assert(op);

    CostVal_t costVal = op->CostVal();

    // all ops are enforcers, so cost is always the use cost
    if( pg->IsMarked() ) {
        Equivalence_t *eq = pg->EqClass();
        assert(eq);

        costVal += eq->UseCost();
    }

    return costVal;
}

// index for a plangroup in pgList
int Greedy_t::Index(PlanGroup_t *pg)
{
    assert(pg);

    int i = 1;
    while( pgList[i] ) {
        if( pgList[i]->ID() == pg->ID() )
            break;
        i++;
        assert(i < PGTABSIZE);
    }

    if( !pgList[i] )
        pgList[i] = pg;

    return i;
}

// greedy heuristic
PlanGroup_t *Greedy_t::FindBestNoMonoPlan(Equivalence_t *rootEq,
                                        CostVal_t *costLimit)
{
    assert(rootEq);
    assert(costLimit);

    // reset propagation counts
    numProps = 0;
    propCount = 0;

    // empty the heaps
    propHeap.Empty();

    // expand the eqNode-logicalOp dag into planGroup-physicalOp dag
    // also, initialize the best plans as best volcano plans
    PlanGroup_t *rootGroup = volcano.ExpressionDAG(rootEq, *costLimit);
    assert(rootGroup);

    Plan_t *initBestPlan = rootGroup->BestPlan();
    assert(initBestPlan);
    assert(!initBestPlan->IsFailed());

    CostVal_t bestCost = TotalCost(rootGroup);

    if( Config_t::IsDebug() ) {
        cout << "Initial Plan" << endl;
        initBestPlan->PrintPlan();
        cout << endl;
    }

    PlanGroup_t *allPGList[5000];
    for( int k = 0 ; k < 5000 ; k++ )
        allPGList[k] = NULL;

    int numPG = 0;
    int travID = PlanGroup_t::StartScan();
    PopulateAllPGList(travID, rootEq, allPGList, &numPG);
    assert(numPG < 5000);

    int done = 0;
    while( !done ) {
        int bestPGIndex = -1;
        done = 1;

        for( int pgNo = 0 ; pgNo < numPG ; pgNo++ ) {
            PlanGroup_t *pg = allPGList[pgNo];
            if( !pg )
                continue;

            Plan_t *pgBestAlgPlan = pg->BestAlgPlan();
            if( pgBestAlgPlan ) {
                if( !pgBestAlgPlan->IsFailed() ) {
                    if( pgBestAlgPlan->CostVal() <= pg->UseCost() ) {
                        // recomputation cost no more than use cost

                        if( Config_t::IsDebug(7) ) {
                            cout << "Ignoring " << pg->ID() << endl;
                            cout << "Computation Cost = " <<
                                        pgBestAlgPlan->CostVal() <<
                                        " UseCost = " << pg->UseCost() << endl;
                            cout << endl;
                        }

                        allPGList[pgNo] = NULL;
                        continue;
                    }
                }
            }

            /*--- NOT NECESSARY ---
            PlanGroup_t *coveringPG = MinCostCoveringPG(pg);
            assert(coveringPG);
            if( coveringPG->IsMarked() ) {
                // a covering node is already materialized; no point
                // materializing this node

                if( Config_t::IsDebug() )
                    cout << "Ignoring " << pg->ID() << endl;

                allPGList[pgNo] = NULL;
                continue;
            }
            --- NOT NECESSARY ---*/

            numProps++;

            if( Config_t::IsDebug(7) ) 
                cout << "Current Best Plan Cost = " << bestCost << endl;

            // unmaterialize any covered plangroups
            PlanGroup_t *coveredMarkedPG[PGTABSIZE];
            for( int k = 0 ; k < PGTABSIZE ; k++ )
                coveredMarkedPG[k] = NULL;

            int i = -1;
            int numCoveredMarkedPG = 0;
            CoveredMarkedPGList(pg, coveredMarkedPG, &numCoveredMarkedPG);
            for( i = 0 ; i < numCoveredMarkedPG ; i++ ) {
                // because a covering pg is getting materialized, no point
                // keeping this materialized
                UnMaterialize(coveredMarkedPG[i]);
            }

            // recomputes the benefit of materializing pg
            Materialize(pg);

            // propagate all the changes so far
            PropagateChanges(bestCost);

            CostVal_t newBestCost = TotalCost(rootGroup);
            if( Config_t::IsDebug(7) ) {
                cout << "New Plan Cost = " << newBestCost << endl;
            }

            // update the benefit of materializing pg
            if( bestCost > newBestCost ) {
                bestCost = newBestCost;
                bestPGIndex = pgNo;
            }

            UnMaterialize(pg);

            // coveredMarkedPGList pgs were unmaterialized; materialize them
            for( i = 0 ; i < numCoveredMarkedPG ; i++ )
                Materialize(coveredMarkedPG[i]);
        }

        if( bestPGIndex != -1 ) {
            assert(allPGList[bestPGIndex]);

            cout << "Sharing " << allPGList[bestPGIndex]->ID() << endl;
            Materialize(allPGList[bestPGIndex]);
            allPGList[bestPGIndex] = NULL;

            numMaterialized++;
            done = 0;
        }
    }

    PropagateChanges(bestCost);

    *costLimit = TotalCost(rootGroup);
    cout << "BESTCOST " << bestCost << endl;

    cout << "NUMPROPS " << numProps << endl;
    cout << "PROPCOUNT " << propCount << endl;
    cout << "RECOMP " << numRecomp << endl;
    cout << "NUMMAT " << numMaterialized << endl;

    return rootGroup;
}

// add pg and recursively all covering pgs
void Greedy_t::AddPG(int travID, PlanGroup_t *pg,
                        PlanGroup_t **allPGList, int *count)
{
    assert(pg);
    assert(allPGList);
    assert(count);

    if( pg->IsVisited(travID) )
        return;
    pg->Visit(travID);

    ListIter_t<Plan_t *> iter;

    POEdgeList_t& super = pg->SuperList();
    POEdgeIter_t pgIter;
    pgIter.Attach(&super);

    PlanGroup_t *nextPG = NULL;
    while( (nextPG = pgIter.Next()) )
        AddPG(travID, nextPG, allPGList, count);

    pgIter.Detach();

    assert(*count < 5000);
    allPGList[*count] = pg;
    (*count)++;
}

void Greedy_t::PopulateAllPGList(int travID, Equivalence_t *eq,
                                    PlanGroup_t *allPGList[], int *count)
{
    assert(eq);
    assert(count);

    List_t<LogicalOp_t *>& list = eq->LogExprList();
    ListIter_t<LogicalOp_t *> iter;
    iter.Attach(&list);

    while( !iter.IsEnd() ) {
        LogicalOp_t *op = iter.Next();
        assert(op);
        if( op->IsDetached() )
            continue;

        int n = op->NumInputs();
        for( int i = 0 ; i < n ; i++ ) {
            Equivalence_t *inp = op->Input(i);
            assert(inp);

            PopulateAllPGList(travID, inp, allPGList, count);
        }
    }

    PlanGroup_t *pg = eq->PlanGroupBase();
    assert(pg);

    AddPG(travID, pg, allPGList, count);
}

// greedy heuristic
PlanGroup_t *Greedy_t::FindBestPlan(Equivalence_t *rootEq,
                       CostVal_t *costLimit, SortOrder_t *reqOrder)
{
    // return FindBestNoMonoPlan(rootEq, costLimit);
    assert(rootEq);
    assert(costLimit);

    // reset propagation counts
    numProps = 0;
    propCount = 0;

    // empty the heaps
    sharHeap.Empty();
    propHeap.Empty();

    // expand the eqNode-logicalOp dag into planGroup-physicalOp dag
    // also, initialize the best plans as best volcano plans
    PlanGroup_t *rootGroup = volcano.ExpressionDAG(rootEq, *costLimit);
    assert(rootGroup);

    Plan_t *initBestPlan = rootGroup->BestPlan();
    assert(initBestPlan);
    assert(!initBestPlan->IsFailed());

    CostVal_t bestCost = TotalCost(rootGroup);
    cout << "INITCOST " << bestCost << endl;
    cout << "VOLCANOCOST " << initBestPlan->CostVal() << endl;

    if( Config_t::IsDebug() ) {
        cout << "Initial Plan" << endl;
        initBestPlan->PrintPlan();
        cout << endl;
    }

    if( isSharPrune ) {
        if( Config_t::IsUnifShar() ) {
            cout << "Sharable Nodes computed during unification" << endl;
            PopulateSharHeapUnifShar(rootEq);
        } else {
            cout << "Sharable Nodes computed using separate algo" << endl;
            PopulateSharHeap(rootEq);
        }
    } else {
        cout << "Sharable Nodes Optimization Disabled" << endl;
        PopulateSharHeapNoPrune(rootEq);
    }

    cout << "SHARCOUNT " << sharHeap.HeapSize() << endl;

    while( !sharHeap.IsEmpty() ) {
        SharKey_t top = sharHeap.Top();

        if( top.Benefit() <= RelThreshold*bestCost ||
                            top.Benefit() <= AbsThreshold ) {
            // further materialization will not lead to better cost plan
            break;
        }

        /* ---
        if( top.Benefit() <= 0 ) {
            // further materialization will not lead to better cost plan
            break;
        }
        --- */

        PlanGroup_t *pg = top.PlanGroup();
        assert(pg);

        Plan_t *pgBestAlgPlan = pg->BestAlgPlan();
        if( pgBestAlgPlan ) {
            if( !pgBestAlgPlan->IsFailed() ) {
                if( pgBestAlgPlan->CostVal() <= pg->UseCost() ) {
                    // recomputation cost no more than use cost

                    if( Config_t::IsDebug(11) ) {
                        cout << "Ignoring " << pg->ID() <<
                            " Estimated Benefit = " << top.Benefit() << endl;
                        cout << "Computation Cost = " <<
                                                pgBestAlgPlan->CostVal() <<
                            " UseCost = " << pg->UseCost() << endl;
                        cout << endl;
                    }

                    sharHeap.DeleteTop();
                    continue;
                }
            }
        }

        /*--- NOT NECESSARY ---
        PlanGroup_t *coveringPG = MinCostCoveringPG(pg);
        assert(coveringPG);
        if( coveringPG->IsMarked() ) {
            // a covering node is already materialized; no point
            // materializing this node

            if( Config_t::IsDebug() )
                cout << "Ignoring " << pg->ID() <<
                    " Estimated Benefit = " << top.Benefit() << endl;

            sharHeap.DeleteTop();
            continue;
        }
        --- NOT NECESSARY ---*/

        numProps++;

        if( Config_t::IsDebug(16) ) 
            cout << "Current Best Plan Cost = " << bestCost << endl;

        /* --- NOT NECESSARY ---
        // unmaterialize any covered plangroups
        PlanGroup_t *coveredMarkedPG[PGTABSIZE];
        for( int k = 0 ; k < PGTABSIZE ; k++ )
            coveredMarkedPG[k] = NULL;

        int numCoveredMarkedPG = 0;
        CoveredMarkedPGList(pg, coveredMarkedPG, &numCoveredMarkedPG);
        for( int i = 0 ; i < numCoveredMarkedPG ; i++ ) {
            // because a covering pg is getting materialized, no point
            // keeping this materialized
            UnMaterialize(coveredMarkedPG[i]);
        }
        --- NOT NECESSARY ---*/

        if( Config_t::IsDebug(16) )
                cout << "Estimated Benefit = " << top.Benefit() << endl;

        // recomputes the benefit of materializing pg
        Materialize(pg);

        // propagate all the changes so far
        PropagateChanges(bestCost);

        CostVal_t newBestCost = TotalCost(rootGroup);
        if( Config_t::IsDebug(16) ) {
            cout << "New Plan Cost = " << newBestCost << endl;
        }

        int isRetained = 0;             // not unmaterialized
        // update the benefit of materializing pg if > 0, otherwise drop
        // it from the heap
        CostVal_t benefit = bestCost - newBestCost;
        if( benefit <= 0 ) {
            sharHeap.DeleteTop();
        } else {
            top.SetBenefit(benefit);
            sharHeap.UpdateTop(top);

            SharKey_t newTop = sharHeap.Top();
            PlanGroup_t *newpg = newTop.PlanGroup();
            assert(newpg);

            // if the plan group stays at the top after materialization,
            // keep it materialized and delete from sharHeap; otherwise
            // unmaterialize it

            if( Config_t::IsDebug(16) )
                cout << "Benefit " << benefit << endl;

            if( pg->ID() == newpg->ID() )
                isRetained = 1;
        }
            
        if( isRetained ) {
            sharHeap.DeleteTop();
            bestCost = newBestCost;

            numMaterialized++;

            if( Config_t::IsDebug(100) ) {
                /* --- NOT NECESSARY ---
                for( int i = 0 ; i < numCoveredMarkedPG ; i++ )
                    cout << "UnSharing " << coveredMarkedPG[i]->ID() << endl;
                --- NOT NECESSARY ---*/
                cout << "Sharing [" << pg->EqClass()->ID() << ":" <<
                            pg->ID() << "] Benefit = " << benefit << endl;
            }

            if( Config_t::IsDebug(9) ) {
                Plan_t *curBestPlan = rootGroup->BestPlan();
                curBestPlan->PrintPlan();
                cout << endl;
                cout << endl;
            }
        } else {
            UnMaterialize(pg);

            /* --- NOT NECESSARY ---
            // coveredMarkedPGList pgs were unmaterialized; materialize them
            for( int i = 0 ; i < numCoveredMarkedPG ; i++ )
                Materialize(coveredMarkedPG[i]);
            --- NOT NECESSARY ---*/

            if( Config_t::IsDebug() ) {
                PropagateChanges(bestCost);

                CostVal_t x = TotalCost(rootGroup);
                if( x - bestCost > Config_t::MaxErr() ||
                                        bestCost - x > Config_t::MaxErr() ) {
                    cout << "Oops .. x = " << x << " bestCost = " <<
                                                        bestCost << endl;
                    cout << "pg = " << pg->ID() << endl;
                    // Plan_t *curBestPlan = rootGroup->BestPlan();
                    // curBestPlan->PrintPlan();
                    cout << endl;
                    exit(0);
                }
                cout << endl;
                cout << endl;
            }
        }
    }

    PropagateChanges(bestCost);

    *costLimit = TotalCost(rootGroup);
    cout << "BESTCOST " << bestCost << endl;

    cout << "NUMPROPS " << numProps << endl;
    cout << "PROPCOUNT " << propCount << endl;
    cout << "RECOMP " << numRecomp << endl;
    cout << "NUMMAT " << numMaterialized << endl;

    return rootGroup;
}

// materialize the given plan group and enlist as changed
void Greedy_t::Materialize(PlanGroup_t *pg)
{
    assert(pg);
    assert(!pg->IsMarked());

    if( Config_t::IsDebug(16) )
        cout << "Materializing " << 
            "[" << pg->EqClass()->ID() << ":" << pg->ID() << "]" << endl;
    pg->Mark();

    Equivalence_t *eq = pg->EqClass();
    assert(eq);

    RegisterChange(eq);
}

// unmaterialize the given plan group and enlist as changed
void Greedy_t::UnMaterialize(PlanGroup_t *pg)
{
    assert(pg);
    assert(pg->IsMarked());

    if( Config_t::IsDebug(16) )
        cout << "UnMaterializing " <<
            "[" << pg->EqClass()->ID() << ":" << pg->ID() << "]" << endl;

    pg->UnMark();

    Equivalence_t *eq = pg->EqClass();
    assert(eq);

    RegisterChange(eq);
}

// populate the sharHeap with all the plan groups
void Greedy_t::PopulateSharHeapNoPrune(Equivalence_t *eq)
{
    assert(eq);

    if( eq->TopNo() != -1 ) {
        // already visited
        return;
    }

    List_t<LogicalOp_t *>& list = eq->LogExprList();
    ListIter_t<LogicalOp_t *> iter;

    iter.Attach(&list);

    while( !iter.IsEnd() ) {
        LogicalOp_t *op = iter.Next();
        assert(op);
        if( op->IsDetached() )
            continue;

        int n = op->NumInputs();
        for( int i = 0 ; i < n ; i++ ) {
            Equivalence_t *inp = op->Input(i);

            PopulateSharHeapNoPrune(inp);
        }
    }

    // insert all plan groups for the eq class into the shar heap
    PlanGroup_t *planGroupBase = eq->PlanGroupBase();

    int travID = PlanGroup_t::StartScan();
    HeapInsertUniformBenefit(travID, &sharHeap, planGroupBase);

    eq->SetTopNo();
}

// populate the sharHeap with the sharable plan groups
void Greedy_t::PopulateSharHeap(Equivalence_t *rootEq)
{
    assert(rootEq);

    // find the degree list for the nested query
    PrependList_t<Equivalence_t *> nqEqList;
    rootEq->NQEqList(&nqEqList); 

    // find the degree list for the main query
    int isLast;
    List_t<DegShar_t *> *degreeList = rootEq->DegreeList(&isLast);
    assert(degreeList);

    ListIter_t<DegShar_t *> iter;
    iter.Attach(degreeList);

    // set the degrees of the eq nodes
    while( !iter.IsEnd() ) {
        DegShar_t *degShar = iter.Next();
        assert(degShar);

        Equivalence_t *eq = degShar->EqNode();
        assert(eq);
        int degree = degShar->Degree();

        eq->SetDegree(degree);
    }

    iter.Reset();
    while( !iter.IsEnd() ) {
        DegShar_t *degShar = iter.Next();
        assert(degShar);

        Equivalence_t *eq = degShar->EqNode();
        assert(eq);

        if( eq->IsSharable() ) {
            // insert all plan groups for the eq class into the shar heap
            int travID = PlanGroup_t::StartScan();
            PlanGroup_t *planGroupBase = eq->PlanGroupBase();
            HeapInsert(travID, &sharHeap, planGroupBase, 0);
        }
    }

    if( isLast )
        LogicalNode_t::DeleteDegreeList(degreeList);

    ListIter_t<Equivalence_t *> eqIter;
    eqIter.Attach(&nqEqList);

    while( !eqIter.IsEnd() ) {
        Equivalence_t *eq = eqIter.Next();
        assert(eq);

        int travID = PlanGroup_t::StartScan();
        PlanGroup_t *planGroupBase = eq->PlanGroupBase();
        HeapInsert(travID, &sharHeap, planGroupBase, 1);
    }
}

// recursively inserts all covering plan groups with given pg into the
// shar heap with inifinity benefit
void Greedy_t::HeapInsertUniformBenefit(int travID,
                                Heap_t<SharKey_t> *h, PlanGroup_t *pg)
{
    assert(h);
    assert(pg);

    if( pg->IsVisited(travID) )
        return;
    pg->Visit(travID);

    Plan_t *plan = pg->BestPlan();
    if( !plan )
        plan = pg->BestAlgPlan();
    if( !plan )
        plan = pg->BestEnfPlan();
    assert(plan);
    assert(!plan->IsFailed());

    CostVal_t benefit = Config_t::CostLimit(); // infinity

    if( Config_t::IsDebug() ) {
        cout << "Inserting into SharHeap: plan group id = " << pg->ID() <<
            " best plan id = " << plan->ID() << endl; 
        cout << " benefit = " << benefit << endl;
        cout << endl;

        cout << " root type = " << plan->Root()->Type() << endl;
        if( plan->Root()->Type() == ALGORITHM_T )
            cout << " algo type = " <<
                ((Algorithm_t *)plan->Root())->AlgoType() << endl;
    }

    SharKey_t key(pg, benefit);
    h->Insert(key);

    POEdgeList_t& superList = pg->SuperList();
    POEdgeIter_t iter;
    iter.Attach(&superList);

    PlanGroup_t *nextPG = NULL;
    while( (nextPG = iter.Next()) )
        HeapInsertUniformBenefit(travID, h, nextPG);

    iter.Detach();
}

// recursively inserts all plan groups covering with given pg into the
// shar heap
void Greedy_t::HeapInsert(int travID, Heap_t<SharKey_t> *h,
                                        PlanGroup_t *pg, int isNested)
{
    assert(h);
    assert(pg);

    if( pg->IsVisited(travID) )
        return;
    pg->Visit(travID);

    // Added -- PRASAN
    if( pg->IsBuiltIn() ) {
        pg->Mark();
    } else {
        CostVal_t benefit = 0;

        if( isNested ) {
            Equivalence_t *eq = pg->EqClass();
            assert(eq);

            benefit =  eq->TopNo() * Config_t::CostLimit(); // infinite cost
        } else {
            // put in the actual benefit
            Plan_t *plan = pg->BestPlan();
            if( !plan )
                plan = pg->BestAlgPlan();
            if( !plan )
                plan = pg->BestEnfPlan();
            assert(plan);
            assert(!plan->IsFailed());

            // cost of materializing the plan
            CostVal_t planCost = plan->CostVal();

            // degree of sharing
            int degree = pg->Degree();

            benefit =  planCost * degree;

            if( Config_t::IsDebug() ) {
                cout << "Inserting into SharHeap: plan group id = " << pg->ID() <<
                    " best plan id = " << plan->ID() << endl; 
                cout << " cost = " << planCost << " degree = " << degree <<
                                                " benefit = " << benefit << endl;
                plan->PrintPlan();
                cout << endl;

                cout << " root type = " << plan->Root()->Type() << endl;
                if( plan->Root()->Type() == ALGORITHM_T )
                    cout << " algo type = " <<
                        ((Algorithm_t *)plan->Root())->AlgoType() << endl;
            }
        }

        SharKey_t key(pg, benefit);
        h->Insert(key);
    }

    POEdgeList_t& superList = pg->SuperList();
    POEdgeIter_t iter;
    iter.Attach(&superList);

    PlanGroup_t *nextPG = NULL;
    while( (nextPG = iter.Next()) )
        HeapInsert(travID, h, nextPG, isNested);

    iter.Detach();
}

// update the actual inputs of the alg plan's root and recompute its cost
void Greedy_t::RecomputeAlgCost(Plan_t *plan)
{
    assert(plan);
    assert(!plan->IsFailed());

    PhysicalOp_t *op = plan->Root();
    assert(op);
    assert(op->Type() == ALGORITHM_T);

    op->RecomputeCostVal();
    CostVal_t costVal = op->CostVal();

    if( Config_t::IsDebug(10) )
        cout << "Recomputing Plan Cost: Root = " << op->ID() <<
                                " op cost = " << op->CostVal() << endl;

    // update the parents' cost

    int n = op->NumInputs();
    for( int i = 0 ; i < n ; i++ ) {
        PlanGroup_t *inp = op->Input(i);
        assert(inp);

        PlanGroup_t *actualInp = NULL;
        CostVal_t actualInpCost = -1;
        MinCostCoveringPG((Algorithm_t *) op, inp, i, &actualInp, &actualInpCost);
        assert(actualInp);
        assert(actualInpCost >= 0);

        op->SetActualInput(i, actualInp);
        costVal += actualInpCost;
    }

    Cost_t *planCost = plan->Cost();
    assert(planCost);

    if( Config_t::IsDebug(10) ) {
        cout << "op = " << op->ID() << " old cost = " << planCost->Value() <<
                                            " new cost = " << costVal << endl;
    }

    if( planCost->Value() != costVal ) {
        Cost_t *cost = NULL;
        if( IsShared(planCost) ) {
            cost = planCost->Clone();
        } else {
            cost = planCost;
        }

        Refer(cost);
        assert(cost->CostType() == COSTORD_T);
        ((CostOrd_t *) cost)->Assign(costVal);
        plan->SetCost(cost);
        DeRefer(cost);
    }
}

// returns the min cost covering node for pg
void Greedy_t::MinCostCoveringPG(Algorithm_t *op, PlanGroup_t *pg,
                                int inpNo,
                                PlanGroup_t **covPG, CostVal_t *covCost)
{
    assert(pg);
    assert(covPG);
    assert(covCost);

    int travID = PlanGroup_t::StartScan();
    PlanGroup_t *retPG = NULL;
    CostVal_t retCost = -1;
    TravCoveringPG(travID, op, pg, inpNo, &retPG, &retCost);

    *covPG = retPG;
    *covCost = retCost;
}

void Greedy_t::TravCoveringPG(int travID, Algorithm_t *op, PlanGroup_t *pg,
                            int inpNo,
                            PlanGroup_t **retBestPG, CostVal_t *retBestCost)
{
    assert(op);
    assert(pg);
    assert(retBestPG);
    assert(retBestCost);

    if( pg->IsVisited(travID) )
        return;
    pg->Visit(travID);

    register PlanGroup_t *bestPG = NULL;
    register CostVal_t bestCost = -1;
    // TODO: Check!
    if( pg->IsBuiltIn() || pg->IsMarked() ) {
        if( pg->IsIndexed() && (op->AlgoType() == INDEXEDSELECT_T ||
                                op->AlgoType() == INDEXEDNESTEDLOOPSJOIN_T) 
                            && inpNo==0 ) 
        {
            bestCost = 0;
        } else {
            /* With assert */ 
            // Equivalence_t *eq = pg->EqClass();
            // assert(eq);
            // bestCost = eq->UseCost();
            bestCost = pg->EqClass()->UseCost();
        }

        bestPG = pg;
    } else {
        register Plan_t *pgBestPlan = pg->BestPlan();
        if( !pgBestPlan )
            pgBestPlan = pg->BestAlgPlan();
        assert(pgBestPlan);

        if( !pgBestPlan->IsFailed() ) {
            bestPG = pg;
            bestCost = pgBestPlan->CostVal();
        }
    }

    register POEdgeList_t& superList = pg->SuperList();
    register POEdgeIter_t iter;
    iter.Attach(&superList);

    PlanGroup_t *nextPG = NULL;
    register PlanGroup_t *retPG = NULL;
    register CostVal_t retCost = -1;
    while( (nextPG = iter.Next()) ) {
        TravCoveringPG(travID, op, nextPG, inpNo, &retPG, &retCost);
        if( !retPG )
            continue;

        if( bestCost == -1 || bestCost >= retCost ) {
            bestPG = retPG;
            bestCost = retCost;
        }
    }

    iter.Detach();

    *retBestPG = bestPG;
    *retBestCost = bestCost;
}

// returns the marked nodes covered by pg
void Greedy_t::CoveredMarkedPGList(PlanGroup_t *pg,
                                        PlanGroup_t *coveredMarkedPG[],
                                                    int *numCoveredMarkedPG)
{

    assert(0); // NOT being called

    assert(pg);
    assert(numCoveredMarkedPG);
    *numCoveredMarkedPG = 0;

    int travID = PlanGroup_t::StartScan();
    TravCoveredPGList(travID, pg, coveredMarkedPG, numCoveredMarkedPG);
}

void Greedy_t::TravCoveredPGList(int travID, PlanGroup_t *pg,
                                    PlanGroup_t *coveredMarkedPG[],
                                                int *numCoveredMarkedPG)
{

    assert(0); // NOT being called

    assert(pg);
    assert(numCoveredMarkedPG);

    if( pg->IsVisited(travID) )
        return;
    pg->Visit(travID);

    if( pg->IsMarked() ) {
        int n = (*numCoveredMarkedPG)++;
        assert(n < PGTABSIZE);
        coveredMarkedPG[n] = pg;
    }

    POEdgeList_t& subList = pg->SubList();
    POEdgeIter_t iter;
    iter.Attach(&subList);

    PlanGroup_t *nextPG = NULL;
    while( (nextPG = iter.Next()) )
        TravCoveredPGList(travID, nextPG, coveredMarkedPG, numCoveredMarkedPG);

    iter.Detach();
}

// returns the total cost of the plan rooted at plan group
CostVal_t Greedy_t::TotalCost(PlanGroup_t *planGroup)
{
    assert(planGroup);
    CostVal_t matCost = 0;

    int travID = planGroup->StartScan();
    CostVal_t planCost =  TotalCostTrav(travID, NULL, 0, planGroup, &matCost);

    if( Config_t::IsDebug() ) {
        cout << endl;
        cout << "PlanCost = " << planCost << " MatCost = " << matCost << endl;
    }

    return planCost + matCost;
}

// used by TotalCost
CostVal_t Greedy_t::TotalCostTrav(int travID, PhysicalOp_t *parOp, int inpNo,
                                        PlanGroup_t *pg, CostVal_t *matCost)
{
    assert(pg);
    assert(matCost);

    if( pg->IsBuiltIn() ) {
        // if indexed and is the 0th (index) input of a index-based
        // operator and is built-in then the cost of index creation is zero

        if( pg->IsIndexed() && inpNo == 0 ) {
            if( parOp->Type() == ALGORITHM_T ) {
                Algorithm_t *alg = (Algorithm_t *) parOp;
                if( (alg->AlgoType() == INDEXEDSELECT_T ||
                    alg->AlgoType() == INDEXEDNESTEDLOOPSJOIN_T) ) {
                    if( Config_t::IsDebug(16) )
                        cout << "NO cost for pg " << pg->ID() << endl;
                    return 0;
                }
            }
        }

        Equivalence_t *eq = pg->EqClass();
        assert(eq);

        if( Config_t::IsDebug(16) ) {
            cout << "Use Cost for pg " << pg->ID() <<
                            " = " << eq->UseCost() << endl;
        }
        return eq->UseCost();
    }

    Plan_t *plan = pg->BestPlan();
    if( !plan )
        plan = pg->BestAlgPlan();
    else if( plan->IsFailed() )
        plan = pg->BestAlgPlan();
    assert(plan);
    assert(!plan->IsFailed());

    if( pg->IsVisited(travID) ) {
        if( pg->IsMarked() ) {
            Equivalence_t *eq = pg->EqClass();
            assert(eq);

            if( !pg->IsShared() ) {
                // found that a new node is shared ...

                if( !pg->IsIndexed() ) {
                    // add the materialization cost 

                    *matCost += eq->MatCost();
                }

                pg->MarkShared();
            }

            if( pg->IsIndexed() && parOp ) {
                if( parOp->Type() == ALGORITHM_T ) {
                    Algorithm_t *alg = (Algorithm_t *) parOp;
                    if( (alg->AlgoType() == INDEXEDSELECT_T ||
                        alg->AlgoType() == INDEXEDNESTEDLOOPSJOIN_T) &&
                                                                inpNo == 0 ) {
                            return 0;
                    }
                }
            }

            return eq->UseCost();
        }
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
                CostVal_t nqFirstCostVal = TotalCostTrav(travID,
                                                    NULL, 0, nqPG, matCost);
                costVal += nqFirstCostVal;
                if( dist > 1 ) {
                    CostVal_t nqNextCostVal = TotalCostTrav(travID,
                                                    NULL, 0, nqPG, matCost);
                    costVal += (dist-1) * nqNextCostVal;
                }
            }
        }
    }

    int n = op->NumInputs();
    for( int i = 0 ; i < n ; i++ ) {
        PlanGroup_t *inp = NULL;
        inp = op->ActualInput(i);
        if( !inp )
            inp = op->Input(i);
        assert(inp);

        costVal += TotalCostTrav(travID, op, i, inp, matCost);
    }

    /*
    if( plan->CostVal() != costVal ) {
        Cost_t *cost = (CostOrd_t *) plan->Cost();
        assert(cost->CostType() == COSTORD_T);
        ((CostOrd_t *)cost)->Assign(costVal);
    }
    assert(costVal == plan->CostVal());
    */

    if( Config_t::IsDebug(16) ) {
        cout << "Traversed [" << pg->EqClass()->ID() << ":" << pg->ID() <<
                "]" << plan->Root()->ID() << " Cost (excl matcost) = " <<
                            costVal << endl;
    }

    return costVal;
}

// assign topological numbers, mark sharable nodes and populate sharheap
void Greedy_t::PopulateSharHeapUnifShar(Equivalence_t *eq, int markSharable)
{
    assert(eq);

    if( eq->TopNo() != -1 && (eq->IsSharable() || !markSharable) ) {
        // already visited and shared, if needed
        return;
    }

    // get the schema
    LogProp_t *logProp = eq->LogicalProp();
    assert(logProp);
    Schema_t *schema = logProp->Schema();
    assert(schema);

    // traverse the descendents
    List_t<LogicalOp_t *>& list = eq->LogExprList();
    ListIter_t<LogicalOp_t *> iter;

    iter.Attach(&list);

    while( !iter.IsEnd() ) {
        LogicalOp_t *op = iter.Next();
        assert(op);
        if( op->IsDetached() )
            continue;

        if( op->Type() == SELECT_T || op->Type() == JOIN_T ) {
            // check the predicate
            Filter_t *f = (Filter_t *) op;
            Predicate_t *p = f->Predicate();
            assert(p);

            if( !eq->IsCorrelated() ) {
                if( !p->IsValidSchema(NULL, schema) ) {
                    eq->MarkCorrelated();
                    if( Config_t::IsDebug(10) )
                        cout << "Found Correlated Predicate in Eq = " <<
                                                            eq->ID() << endl;
                }
            }

            InExpr_t *inP = (InExpr_t *) p->FindIn();
            if( inP ) {
                // nested predicate
                Equivalence_t *inp = inP->InpEq();
                assert(inp);

                PopulateSharHeapUnifShar(inp, 1);
            }
        }

        int n = op->NumInputs();
        for( int i = 0 ; i < n ; i++ ) {
            Equivalence_t *inp = op->Input(i);

            if( markSharable || eq->IsSharable() )
                PopulateSharHeapUnifShar(inp, 1);
            else PopulateSharHeapUnifShar(inp, 0);

            if( inp->IsCorrelated() )
                eq->MarkCorrelated();
        }
    }

    if( eq->TopNo() == -1 ) 
        eq->SetTopNo();

    if( !eq->IsSharable() && markSharable )
        eq->MarkSharable();
    if( eq->IsSharable() && !eq->IsCorrelated() ) {
        // insert all plan groups for the eq class into the shar heap
        int travID = PlanGroup_t::StartScan();
        PlanGroup_t *planGroupBase = eq->PlanGroupBase();
        HeapInsertUniformBenefit(travID, &sharHeap, planGroupBase);
    }
}
