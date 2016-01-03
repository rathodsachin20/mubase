// exhaustive search dag optimization algorithm

#include <assert.h>
#include <stdlib.h>

#include "Config.h"
#include "PhysicalOp.h"
#include "DAGOpt.h"
#include "ExhSearch.h"

// quick sort
static void SortActiveStackEntries(int numElements, ActiveStackEntry_t **base);

// quick sort
static void SortPlanGroups(int numElements, PlanGroup_t **base);

// comparison function used in sorting of active stack entries list
static int CompareActiveStackEntries(const void *x, const void *y);

// comparison function used in sorting of plangroup list
static int ComparePlanGroups(const void *x, const void *y);

int ActiveStackConfig_t::CurID = 0;
int SharStackConfig_t::CurID = 0;

// wrapper for preprocessing, etc
PlanGroup_t *ExhSearch_t::FindBestPlan(Equivalence_t *rootEq,
                          CostVal_t *costLimit, SortOrder_t *reqOrder)
{
    assert(rootEq);
    assert(costLimit);

    PlanGroup_t *rootGroup = PreProcessDAG(rootEq, *costLimit);
    assert(rootGroup);

    Plan_t *volcanoBestPlan = rootGroup->BestPlan();
    assert(volcanoBestPlan);
    assert(!volcanoBestPlan->IsFailed());

    // a plan with cost volcanoBestCost exists --- update upper bound
    // for better pruning
    CostVal_t volcanoBestCost = volcanoBestPlan->CostVal();
    if( volcanoBestCost < *costLimit )
        *costLimit = volcanoBestCost;

    *costLimit += Config_t::MaxErr();
    activeStack.Insert(rootGroup);
    Refer(rootGroup);

    PlanConfig_t *planConfig = Enumerate(costLimit);

    if( !planConfig )
        return NULL;

    if( Config_t::IsDebug() ) {
        cout << "Best Plan Config" << endl;
        planConfig->Print();
    }

    // translates the configuration of best plan to the actual plan by
    // setting the bestplan pointers of the appropriate plan groups
    planConfig->TranslateBestPlan(rootGroup);

    DeRefer(planConfig);

    activeStack.Delete();
    assert(IsShared(rootGroup));
    DeRefer(rootGroup);

    return rootGroup;
}

// preprocessing
PlanGroup_t *ExhSearch_t::PreProcessDAG(Equivalence_t *rootEq, CostVal_t costUB)
{
    assert(rootEq);

    // expand the eqNode-logicalOp dag into planGroup-physicalOp dag
    // also, initialize the best plans as best volcano plans
    PlanGroup_t *rootGroup = volcano.ExpressionDAG(rootEq, costUB);
    assert(rootGroup);

    return rootGroup;
}

// put pg in current plan
void ExhSearch_t::Mark(PlanGroup_t *pg)
{
    assert(pg);
    pg->Mark();
}

// remove pg from current plan; if sharable, pop from the sharStack
void ExhSearch_t::UnMark(PlanGroup_t *pg)
{
    assert(pg);
    pg->UnMark();
}

// is pg in the current plan?
int ExhSearch_t::IsMarked(PlanGroup_t *pg)
{
    assert(pg);
    return pg->IsMarked();
}

// is the pg in progress?
int ExhSearch_t::IsInProgress(PlanGroup_t *pg) const
{
    return pg->IsInProgress();
}

// increment in-progress counts of pg and all the nodes that physically
// subsume pg
void ExhSearch_t::SetInProgress(PlanGroup_t *pg)
{
    if( !pg->IsInProgress() ) {
        POEdgeList_t& super = pg->SuperList();
        POEdgeIter_t iter;
        iter.Attach(&super);

        PlanGroup_t *nextPG = NULL;
        while( (nextPG = iter.Next()) )
            SetInProgress(nextPG);

        iter.Detach();
    }

    pg->IncrInProgress();
}

// decrement in-progress counts of pg and all the nodes that physically
// subsume pg
void ExhSearch_t::ClearInProgress(PlanGroup_t *pg)
{
    pg->DecrInProgress();

    if( !pg->IsInProgress() ) {
        POEdgeList_t& super = pg->SuperList();
        POEdgeIter_t iter;
        iter.Attach(&super);

        PlanGroup_t *nextPG = NULL;
        while( (nextPG = iter.Next()) )
            ClearInProgress(nextPG);

        iter.Detach();
    }
}

// enumerator --- actual interface function
// is wrapped around by other algorithms (like cached version of exhaustive)
// acts as a wrapper for EnumerateFilter
//  --- this is to save unnecessary calls to Enumerate
PlanConfig_t *ExhSearch_t::Enumerate(CostVal_t *costLimit)
{
    assert(costLimit);
    return EnumerateFilter(costLimit);
}

// filter for EnumeratePlans
// copies any inprogress nodes at the top of the activeStack into the
// system stack before calling Enumerate and restores the same while
// unwinding the recursion
PlanConfig_t *ExhSearch_t::EnumerateFilter(CostVal_t *costLimit)
{
    assert(costLimit);
    assert(*costLimit >= 0);

    PlanConfig_t *planConfig = NULL;

    if( activeStack.IsEmpty() ) {
        // return zero cost valid plan
        planConfig = new PlanConfig_t(0, 0);
        Refer(planConfig);

        *costLimit = 0;

        return planConfig;
    }

    // pop a plan group from active stack
    PlanGroup_t *pg = activeStack.Delete();
    assert(pg);

    if( IsInProgress(pg) ) {
        // backtracking on the current plan
        ClearInProgress(pg);
        planConfig = EnumerateFilter(costLimit);
        SetInProgress(pg);
    } else {
        // enumerate the plans, considering one by one the plan groups in
        // the eq class that subsume pg
        int travID = pg->StartScan();
        EnumeratePlans(travID, pg, costLimit, &planConfig);
    }

    // push back pg into the active stack
    activeStack.Insert(pg);

    return planConfig;
}

// actual algorithm -- enumerates all plans
// returns 1 if pg was available for free, 0 otherwise
// this helps in pruning further unnecessary alternatives for pg's
// parents in case pg is available for free
int ExhSearch_t::EnumeratePlans(int travID, PlanGroup_t *pg,
                        CostVal_t *costLimit, PlanConfig_t **planConfig)
{
    assert(costLimit);
    assert(planConfig);
    assert(!*planConfig);

    if( pg->IsVisited(travID) )
        return -1;

    pg->Visit(travID);

    if( IsInProgress(pg) ) {
        // either pg or a node that subsumes pg makes a cycle and is
        // therefore invalid

        // we rule out nodes that physically
        // subsume pg because that will be considered when
        // alternatives to the instance of pg already in the plan
        // are explored; and that alternative plan
        // is clearly going to have a cheaper cost

        return -1;
    }

    if( IsMarked(pg) ) {
        CostVal_t useCost = pg->UseCost();
        // pg is available at useCost

        CostVal_t newCostLimit = *costLimit - useCost;
        if( newCostLimit < 0 )
            return -1;

        PlanConfig_t *newPlanConfig = Enumerate(&newCostLimit);
        assert(newCostLimit <= *costLimit - useCost + Config_t::MaxErr());
        if( newPlanConfig ) {
            // valid plan found
            // update the costLimit
            *costLimit = newCostLimit + useCost;

            *planConfig = new PlanConfig_t(newPlanConfig, NULL, *costLimit);
            Refer(*planConfig);

            if( Config_t::IsDebug() ) {
                cout << "Active Stack" << endl;
                activeStack.Print();
                cout << "Current Best Plan Config" << endl;
                (*planConfig)->Print();
                cout << "Current Best Cost = " << *costLimit << endl;
                cout << endl;
            }

            DeRefer(newPlanConfig);
        }

        return 1;
    }

    // put pg in current plan
    Mark(pg);

    // indicates that pg is in the current root to current node path
    SetInProgress(pg);
    activeStack.Insert(pg);

    // iterate over the algorithm plans under the plan group
    ListIter_t<Plan_t *> planIter;

    List_t<Plan_t *>& algPlans = pg->AlgPlans();
    planIter.Attach(&algPlans);

    // iterate for each plan under the plan group
    while( !planIter.IsEnd() ) {
        Plan_t *p = planIter.Next();
        assert(p);

        FixPlan(p, costLimit, planConfig);
    }

    // iterate over the enforcer plans under the plan group
    List_t<Plan_t *>& enfPlans = pg->EnfPlans();
    planIter.Attach(&enfPlans);

    // iterate for each plan under the plan group
    while( !planIter.IsEnd() ) {
        Plan_t *p = planIter.Next();
        assert(p);

        FixPlan(p, costLimit, planConfig);
    }

    PlanGroup_t *rempg = activeStack.Delete();

    // sanity checks
    assert(pg);
    assert(pg->ID() == rempg->ID());

    ClearInProgress(pg);

    // remove pg from current plan
    UnMark(pg);

    // recurse on the plan groups with properties covered by baseProp
    POEdgeList_t& super = pg->SuperList();

    POEdgeIter_t iter;
    iter.Attach(&super);

    PlanGroup_t *nextPG = NULL;
    while( (nextPG = iter.Next()) ) {
        PlanConfig_t *newPlanConfig = NULL;
        CostVal_t newCostLimit = *costLimit;

        int retval = EnumeratePlans(travID, nextPG,
                                            &newCostLimit, &newPlanConfig);
        assert(newCostLimit <= *costLimit + Config_t::MaxErr());

        if( newPlanConfig ) {
            if( newCostLimit <= *costLimit ) {
                // better plan
                if( *planConfig )
                    DeRefer(*planConfig);

                *planConfig = newPlanConfig;
                Refer(*planConfig);

                *costLimit = newCostLimit;
            }
            DeRefer(newPlanConfig);
        }

        if( retval == 1 ) {
            // nextPG available for free, cannot beat this cost; so no
            // point trying other alternatives
            return 1;
        }
    }

    iter.Detach();

    return 0;
}

// enumerates the plans with the plan node p fixed in the current plan
void ExhSearch_t::FixPlan(Plan_t *p, CostVal_t *costLimit,
                                        PlanConfig_t **planConfig)
{
    assert(p);
    assert(costLimit);
    assert(planConfig);

    PhysicalOp_t *op = p->Root();
    assert(op);
    
    if( Config_t::IsDebug() ) {
        cout << "op: ID = " << op->ID() << endl;
        cout << "op: Name = ";
        op->PrintName();
        cout << endl;
        cout << "op: NumInputs = " << op->NumInputs() << endl;
        cout << "op: CostVal = " << op->CostVal() << endl;
    }

    // new cost limit
    CostVal_t newCostLimit = *costLimit - op->CostVal();
    if( newCostLimit < 0 ) {
        // operator inclusion infeasible
        return;
    }

    // push op's inputs
    int opNumInputs = op->NumInputs();

    for( int i = 0 ; i < opNumInputs ; i++ ) {
        PlanGroup_t *inp = op->Input(i);
        assert(inp);

        activeStack.Insert(inp);
        Refer(inp);
    }

    PlanConfig_t *newPlanConfig = Enumerate(&newCostLimit);
    assert(newCostLimit <= *costLimit - op->CostVal() + Config_t::MaxErr());

    // remove op's inputs
    for( int j = opNumInputs-1 ; j >= 0 ; j-- ) {
        PlanGroup_t *inp = op->Input(j);
        assert(inp);

        activeStack.Delete();
        DeRefer(inp);
    }

    if( newPlanConfig ) {
        // plan found
        // update the costLimit
        *costLimit = newCostLimit + op->CostVal();

        // update the plan
        if( *planConfig ) 
            DeRefer(*planConfig);

        *planConfig =
            new PlanConfig_t(newPlanConfig, p, *costLimit);
        Refer(*planConfig);

        if( Config_t::IsDebug() ) {
            cout << "Active Stack" << endl;
            activeStack.Print();
            cout << "Current Best Plan Config" << endl;
            (*planConfig)->Print();
            cout << "Current Best Cost = " << *costLimit << endl;
            cout << endl;
        }

        DeRefer(newPlanConfig);
    }
}

// preprocessing
PlanGroup_t *CachedExhSearch_t::PreProcessDAG(Equivalence_t *rootEq,
                                                        CostVal_t costUB)
{
    assert(rootEq);

    PlanGroup_t *rootGroup = ExhSearch_t::PreProcessDAG(rootEq, costUB);

    // find the degree list
    int isLast;
    List_t<DegShar_t *> *degreeList = rootEq->DegreeList(&isLast);
    assert(degreeList);

    ListIter_t<DegShar_t *> iter;
    iter.Attach(degreeList);

    while( !iter.IsEnd() ) {
        DegShar_t *degShar = iter.Next();
        assert(degShar);

        Equivalence_t *eq = degShar->EqNode();
        assert(eq);
        int degree = degShar->Degree();

        eq->SetDegree(degree);
    }

    if( isLast )
        LogicalNode_t::DeleteDegreeList(degreeList);

    return rootGroup;
}

// put pg in current plan; if sharable, push in the sharStack
void CachedExhSearch_t::Mark(PlanGroup_t *pg)
{
    assert(pg);

    if( pg->IsSharable() ) {
        int count = pg->SharStackCount();
        if( count == 0 ) {
            sharStack.Insert(pg);
            Refer(pg);
        }

        pg->IncrSharStackCount();
    }

    ExhSearch_t::Mark(pg);
}

// remove pg from current plan; if sharable, pop from the sharStack
void CachedExhSearch_t::UnMark(PlanGroup_t *pg)
{
    assert(pg);

    if( pg->IsSharable() ) {
        pg->DecrSharStackCount();

        int count = pg->SharStackCount();
        if( count == 0 ) {
            PlanGroup_t *rempg = sharStack.Delete();
            assert(pg->ID() == rempg->ID());
            DeRefer(pg);
        }
    }

    ExhSearch_t::UnMark(pg);
}

// wrapper on the actual search algorithm to to enable caching
// returns 1 if a plan completed, 0 otherwise
PlanConfig_t *CachedExhSearch_t::Enumerate(CostVal_t *costLimit)
{
    assert(costLimit);

    PlanConfig_t *planConfig = planCache.Search(activeStack, sharStack);
    if( planConfig ) {
        // min cost plan stored

        Refer(planConfig);

        if( planConfig->CostVal() > *costLimit ) {
            DeRefer(planConfig);
            return NULL;
        }

        if( !planConfig->IsFailed() ) {
            *costLimit = planConfig->CostVal();
            return planConfig;
        }

        // optimization had failed for planConfig->CostVal() earlier
        if( planConfig->CostVal() == *costLimit ) {
            DeRefer(planConfig);
            return NULL;
        }
    }

    // optimization needed

    // find and cache the best plan configuration
    PlanConfig_t *newPlanConfig = ExhSearch_t::Enumerate(costLimit);

    planConfig = newPlanConfig;

    // if no feasible configuration exits, cache a failed configuration
    if( !newPlanConfig )
        newPlanConfig = new PlanConfig_t(*costLimit, 1);

    Refer(newPlanConfig);

    // cache the configuration
    planCache.Insert(activeStack, sharStack, newPlanConfig);

    DeRefer(newPlanConfig);

    return planConfig;
}

// construct the active stack entry --- notes the weakest covering
// property plan group nodes that are in progress
ActiveStackEntry_t::ActiveStackEntry_t(PlanGroup_t *pg_a)
                                : pg(pg_a), numInProgress(0)
{
    assert(pg);

    for( int i = 0 ; i < MaxInProgress ; i++ )
        inProgress[i] = NULL;

    Refer(pg);

    // do not maintain the set for inprogress pgs
    if( pg->IsInProgress() )
        return;

    FindInProgress(pg, inProgress, &numInProgress);
}

// find the weakest covering property node that is in progress
void ActiveStackEntry_t::FindInProgress(PlanGroup_t *planGroup,
                                        PlanGroup_t *a[], int *n)
{
    assert(n);

    int travId = PlanGroup_t::StartScan();
    FindInProgressTrav(planGroup, travId, a, n);

    // sort inprogress list  for efficient comparison
    SortPlanGroups(*n, inProgress);
}

// traversal for finding in-progress pgs
void ActiveStackEntry_t::FindInProgressTrav(PlanGroup_t *planGroup,
                                    int travId, PlanGroup_t *a[], int *n)
{
    assert(n);
    assert(planGroup);
    if( planGroup->IsVisited(travId) )
        return;
    planGroup->Visit(travId);

    if( planGroup->IsInProgress() ) {
        a[(*n)++] = planGroup;
        Refer(planGroup);
        return;
    }

    POEdgeList_t& super = planGroup->SuperList();
    POEdgeIter_t iter;
    iter.Attach(&super);

    PlanGroup_t *nextPG = NULL;
    while( (nextPG = iter.Next()) )
        FindInProgressTrav(nextPG, travId, a, n);

    iter.Detach();
}

// active stack entry equality comparison
int ActiveStackEntry_t::operator == (const ActiveStackEntry_t& e) const
{
    PlanGroup_t *epg = e.PlanGroup();
    assert(epg);

    if( pg->ID() != epg->ID() )
        return 0;

    if( numInProgress != e.NumInProgress() )
        return 0;

    for( int i = 0 ; i < numInProgress ; i++ ) {
        PlanGroup_t *x = inProgress[i];
        assert(x);

        PlanGroup_t *y = e.InProgress(i);
        assert(y);

        if( x->ID() != y->ID() )
            return 0;
    }

    return 1;
}

// destruct the active stack entry
ActiveStackEntry_t::~ActiveStackEntry_t(void)
{
    for( int i = 0 ; i < numInProgress ; i++ )
        DeRefer(inProgress[i]);
    DeRefer(pg);
}

// invalidate configuration -- called after insert and delete
// if config is shared, then it must be completely evaluated now for
// consistency sake
void ActiveStack_t::InvalidateConfig(void)
{ 
    if( config ) {
        if( IsShared(config) )
            config->Evaluate();

        DeRefer(config);
        config = NULL;
    }
}

// insert into plan-group stack --- sum maintained incrementally
void ActiveStack_t::Insert(PlanGroup_t *pg)
{
    assert(pg);

    // config must be invalidated before the stack is changed
    InvalidateConfig();

    if( !pg->IsInProgress() ) {
        sum += pg->ID();
        size++;
    } else {
        // is in progress
        if( pg->ActiveStackCount() > 0 ) {
            // already present in the active stack
            // as not in progress but is now being inserted as in
            // progress

            sum -= pg->ID();
            size--;
            assert(size >= 0);
            assert(sum >= 0);
        }
    }

    ActiveStackEntry_t *e = new ActiveStackEntry_t(pg);
    Refer(e);

    list.Insert(e);
    pg->IncrActiveStackCount();

    if( Config_t::IsDebug() ) {
        cout << "Inserted into ActiveStack " << pg->ID() <<
                        " InProgress = " << pg->IsInProgress() <<
                        " stackCount = " << pg->ActiveStackCount() << endl;
        cout << "size = " << size << " sum = " << sum << endl;
        Print();
        cout << endl;
        cout << endl;
    }
}

// delete from plan-group stack --- sum maintained incrementally
PlanGroup_t *ActiveStack_t::Delete(void)
{
    // config must be invalidated before the stack is changed
    InvalidateConfig();

    if( list.IsEmpty() )
        return NULL;

    ActiveStackEntry_t *e = list.DeleteTop();
    assert(e);

    PlanGroup_t *pg = e->PlanGroup();
    assert(pg);

    pg->DecrActiveStackCount();

    if( !pg->IsInProgress() ) {
        size--;
        sum -= pg->ID();
        assert(size >= 0);
        assert(sum >= 0);
    } else {
        // in progress

        if( pg->ActiveStackCount() > 0 ) {
            // is still present in the active stack
            size++;
            sum += pg->ID();
        }
    }

    DeRefer(e);

    if( Config_t::IsDebug() ) {
        cout << "Deleted from ActiveStack " << pg->ID() <<
                        " InProgress = " << pg->IsInProgress() <<
                        " stackCount = " << pg->ActiveStackCount() << endl;
        cout << "size = " << size << " sum = " << sum << endl;
        Print();
        cout << endl;
        cout << endl;
    }

    return pg;
}

// returns the stack configuration; creates if does not exist
// sets the lazy evaluation flag
ActiveStackConfig_t *ActiveStack_t::Config(void)
{
    if( !config ) {
        config = new ActiveStackConfig_t(this, 1);
        // 1 is for lazy evaluation
        Refer(config);
    }

    return config;
}

// print
void ActiveStack_t::Print(void) const
{
    ListIter_t<ActiveStackEntry_t *> iter;
    iter.Attach(&list);

    while( !iter.IsEnd() ) {
        ActiveStackEntry_t *e = iter.Next();
        assert(e);

        PlanGroup_t *pg = e->PlanGroup();
        assert(pg);

        if( pg->IsMarked() )
            cout << "$";
        if( pg->IsInProgress() )
            cout << "#";
        cout << pg->ID() << " ";
    }

    cout << "END" << endl;
}

// invalidate configuration -- called after insert and delete
// if config is shared, then it must be completely evaluated now for
// consistency sake
void SharStack_t::InvalidateConfig(void)
{ 
    if( config ) {
        if( IsShared(config) )
            config->Evaluate();

        DeRefer(config);
        config = NULL;
    }
}

// insert into plan-group stack --- sum maintained incrementally
void SharStack_t::Insert(PlanGroup_t *pg)
{
    assert(pg);

    // config must be invalidated before the stack is changed
    InvalidateConfig();

    sum += pg->ID();

    list.Insert(pg);
}

// delete from plan-group stack --- sum maintained incrementally
PlanGroup_t *SharStack_t::Delete(void)
{
    // config must be invalidated before the stack is changed
    InvalidateConfig();

    if( list.IsEmpty() )
        return NULL;

    PlanGroup_t *pg = list.DeleteTop();
    assert(pg);

    sum -= pg->ID();

    return pg;
}

// returns the stack configuration; creates if does not exist
// sets the lazy evaluation flag
SharStackConfig_t *SharStack_t::Config(void)
{
    if( !config ) {
        config = new SharStackConfig_t(this, 1);
        // 1 is for lazy evaluation
        Refer(config);
    }

    return config;
}

// print
void SharStack_t::Print(void) const
{
    ListIter_t<PlanGroup_t *> iter;
    iter.Attach(&list);

    while( !iter.IsEnd() ) {
        PlanGroup_t *pg = iter.Next();
        assert(pg);
        if( pg->IsMarked() )
            cout << "$";
        cout << pg->ID() << " ";
    }

    cout << "END" << endl;
}

// constructs a configuration given an earlier configuration and a plan
// to be included
PlanConfig_t::PlanConfig_t(const PlanConfig_t *config,
                        Plan_t *plan, CostVal_t costVal_a)
        : planList(NULL), size(0), costVal(costVal_a), isFailed(0)
{
    assert(config);
    assert(costVal >= 0);

    int configSize = config->Size();
    Plan_t **configPlanList = config->PlanList();

    size = configSize;
    if( plan )
        size++;
    assert(size >= 0);

    if( size > 0 )
        planList = new Plan_t *[size];

    int i = 0;
    int j = 0;

    // insert planID at appropriate point in sorted order
    if( plan ) {
        int planID = plan->ID();

        while( i < configSize ) {
            assert(configPlanList[i]);
            if( configPlanList[i]->ID() > planID )
                break;

            planList[j++] = configPlanList[i++];
        }

        planList[j++] = plan;
    }

    while( i < configSize )
        planList[j++] = configPlanList[i++];
}

// print the plan configuration
void PlanConfig_t::Print(void) const
{
    cout << "size = " << size <<
        " costVal = " << costVal << " isFailed = " << isFailed << endl;
    for( int i = 0 ; i < size ; i++ )
        cout << planList[i]->ID() << " ";
    cout << "END" << endl;
}

// translates the configuration of best plan to the actual plan
void PlanConfig_t::TranslateBestPlan(PlanGroup_t *pg) const
{
    assert(pg);

    int i = -1;

    for( i = 0 ; i < size ; i++ ) {
        Plan_t *plan = planList[i];
        assert(plan);

        PlanGroup_t *pg = plan->PlanGroup();
        assert(pg);
        pg->SetBestPlan(plan);
        pg->Mark();
    }

    for( i = 0 ; i < size ; i++ ) {
        Plan_t *plan = planList[i];
        assert(plan);

        PhysicalOp_t *op = plan->Root();
        assert(op);

        int n = op->NumInputs();
        for( int j = 0 ; j < n ; j++ ) {
            PlanGroup_t *inp = op->Input(j);
            assert(inp);

            PlanGroup_t *markedInp = MarkedSuper(inp);
            assert(markedInp);

            op->SetActualInput(j, markedInp);
        }
    }
}
    
// called by TranslateBestPlan
// returns a nearest covering marked node 
PlanGroup_t *PlanConfig_t::MarkedSuper(PlanGroup_t *pg) const
{

    assert(pg);

    if( pg->IsMarked() )
        return pg;

    // recurse on the plan groups with properties that cover pg
    POEdgeList_t& super = pg->SuperList();

    POEdgeIter_t pgIter;
    pgIter.Attach(&super);

    PlanGroup_t *markedPG = NULL;
    PlanGroup_t *nextPG = NULL;
    while( !markedPG && (nextPG = pgIter.Next()) )
        markedPG = MarkedSuper(nextPG);

    pgIter.Detach();

    return markedPG;
}

// stack configuration constructor
ActiveStackConfig_t::ActiveStackConfig_t(const ActiveStack_t *stack_a,
                                                                int isLazy)
            : id(CurID++), stack(stack_a), entryList(NULL), size(0), sum(0)
{
    assert(stack);

    size = stack->Size();
    assert(size >= 0);

    sum = stack->Sum();
    assert(sum >= 0);

    if( !isLazy )
        Evaluate();
}

// evaluate idList if not evaluated before
// stack == NULL if Evaluate has been invoked earlier
void ActiveStackConfig_t::Evaluate(void)
{
    if( stack )
        CreateEntryList();

    stack = NULL;
}

// create the sorted list of non-inprogress plangroup nodes from the stack
void ActiveStackConfig_t::CreateEntryList(void)
{
    assert(stack);
    assert(!entryList);

    // the stack should not have changed in the meantime --- the
    // following check might detect any inconsistency, but is not
    // guaranteed to work always
    // In our implementation, this is guaranteed by forcing the
    // configuration to be computed if it is shared and the stack
    // contents are going to change
    assert(stack->Sum() == sum);
    assert(stack->Size() == size);

    if( size == 0 ) {
        entryList = NULL;
        return;
    }

    assert(size > 0);
    entryList = new ActiveStackEntry_t *[size];

    const List_t<ActiveStackEntry_t *>& list = stack->List();

    ListIter_t<ActiveStackEntry_t *> iter;
    iter.Attach(&list);

    int i = 0;
    while( !iter.IsEnd() ) {
        ActiveStackEntry_t *e = iter.Next();
        assert(e);

        PlanGroup_t *pg = e->PlanGroup();
        assert(pg);

        if( !pg->IsInProgress() ) {
            assert(i < size);
            entryList[i++] = e;
            Refer(e);
        }
    }
    assert(i == size);

    // sort entryList for efficient comparison
    SortActiveStackEntries(size, entryList);

    if( Config_t::IsDebug() ) {
        cout << "Created Active Stack Config" << endl;
        Print();
    }
}

// equality operator for stack configuration
// uses lazy evaluation of the configuration array for efficiency
int ActiveStackConfig_t::operator == (ActiveStackConfig_t& config)
{
    // check if same
    if( config.ID() == id )
        return 1;

    // compare the size of the lists
    if( size != config.Size() )
        return 0;

    // compare the sums od the ids in the list
    if( sum != config.Sum() )
        return 0;

    // evaluate entryList if not evaluated before
    Evaluate();

    ActiveStackEntry_t **configEntryList = config.EntryList();

    // if both ID lists are NULL, consider as equal
    if( !configEntryList && !entryList )
        return 1;

    // if either but not both is NULL, consider unequal
    if( !configEntryList || !entryList )
        return 0;

    // both are not NULL
    for( int i = 0 ; i < size ; i++ ) {
        if( *(configEntryList[i]) != *(entryList[i]) )
            return 0;
    }

    return 1;
}

// print
void ActiveStackConfig_t::Print(void) const
{
    if( stack ) {
        cout << "Stack" << endl;
        stack->Print();
    } else {
        cout << "Active Stack Config" << endl;
        cout << "size = " << size << " sum = " << sum << endl;
        for( int i = 0 ; i < size ; i++ ) {
            assert(entryList[i]);

            PlanGroup_t *pg = entryList[i]->PlanGroup();
            assert(pg);

            cout << pg->ID() << " ";
        }
        cout << "END" << endl;
    }
}

// stack configuration destructor
ActiveStackConfig_t::~ActiveStackConfig_t(void)
{
    if( entryList ) {
        for( int i = 0 ; i < size ; i++ )
            DeRefer(entryList[i]);

        delete[] entryList;
    }
}

// stack configuration constructor
SharStackConfig_t::SharStackConfig_t(const SharStack_t *stack_a, int isLazy)
        : id(CurID++), stack(stack_a), planGroupList(NULL), size(0), sum(0)
{
    const List_t<PlanGroup_t *>& list = stack->List();

    size = list.Size();
    assert(size >= 0);

    sum = stack->Sum();
    assert(sum >= 0);

    if( !isLazy )
        Evaluate();
}

// evaluate idList if not evaluated before
// stack == NULL if Evaluate has been invoked earlier
void SharStackConfig_t::Evaluate(void)
{
    if( stack )
        CreatePlanGroupList();

    stack = NULL;
}

// create the idList from the stack
void SharStackConfig_t::CreatePlanGroupList(void)
{
    assert(stack);
    assert(!planGroupList);

    const List_t<PlanGroup_t *>& list = stack->List();

    // the stack should not have changed in the meantime --- the
    // following check might detect any inconsistency, but is not
    // guaranteed to work always
    assert(stack->Sum() == sum);
    assert(list.Size() == size);

    if( size == 0 ) {
        planGroupList = NULL;
        return;
    }

    assert(size > 0);
    planGroupList = new PlanGroup_t *[size];

    ListIter_t<PlanGroup_t *> iter;
    iter.Attach(&list);
    for( int i = 0 ; i < size ; i++ ) {
        planGroupList[i] = iter.Next();
        assert(planGroupList[i]);
        Refer(planGroupList[i]);
    }

    // sort idList for efficient comparison
    SortPlanGroups(size, planGroupList);

    if( Config_t::IsDebug() ) {
        cout << "Created Stack Config" << endl;
        Print();
    }
}

// equality operator for stack configuration
// uses lazy evaluation of the configuration array for efficiency
int SharStackConfig_t::operator == (SharStackConfig_t& config)
{
    // check if same
    if( config.ID() == id )
        return 1;

    // compare the size of the lists
    if( size != config.Size() )
        return 0;

    // compare the sums od the ids in the list
    if( sum != config.Sum() )
        return 0;

    // evaluate idList if not evaluated before
    Evaluate();

    PlanGroup_t **configPlanGroupList = config.PlanGroupList();

    // if both PlanGroup lists are NULL, consider as equal
    if( !configPlanGroupList && !planGroupList )
        return 1;

    // if either but not both is NULL, consider unequal
    if( !configPlanGroupList || !planGroupList )
        return 0;

    // both are not NULL
    for( int i = 0 ; i < size ; i++ ) {
        assert(configPlanGroupList[i]);
        assert(planGroupList[i]);
        if( configPlanGroupList[i]->ID() != planGroupList[i]->ID() )
            return 0;
    }

    return 1;
}

// print
void SharStackConfig_t::Print(void) const
{
    if( stack ) {
        cout << "Stack" << endl;
        stack->Print();
    } else {
        cout << "Stack Config" << endl;
        cout << "size = " << size << " sum = " << sum << endl;
        for( int i = 0 ; i < size ; i++ )
            cout << planGroupList[i]->ID() << " ";
        cout << "END" << endl;
    }
}

// stack configuration destructor
SharStackConfig_t::~SharStackConfig_t(void)
{
    if( planGroupList ) {
        for( int i = 0 ; i < size ; i++ )
            DeRefer(planGroupList[i]);
        delete[] planGroupList;
    }
}

// plan entry constructor
PlanEntry_t::PlanEntry_t(SharStack_t& sharStack, PlanConfig_t *planConfig_a)
                    : adornment(NULL), planConfig(planConfig_a)
{
    adornment = sharStack.Config();
    Refer(adornment);
    if( planConfig )
        Refer(planConfig);
}

// planentry copy constructor
PlanEntry_t::PlanEntry_t(const PlanEntry_t& e)
{
    adornment = e.Adornment();
    Refer(adornment);

    planConfig = e.PlanConfig();
    if( planConfig )
        Refer(planConfig);
}

// planentry equality comparison
int PlanEntry_t::operator == (PlanEntry_t& entry)
{
    SharStackConfig_t *entryAdornment = entry.Adornment();
    assert(entryAdornment);
    assert(adornment);

    return *entryAdornment == *adornment;
}

// sets the plan configuration for the entry
void PlanEntry_t::SetPlanConfig(PlanConfig_t *planConfig_a)
{
    PlanConfig_t *oldPlanConfig = planConfig;

    planConfig = planConfig_a;
    if( planConfig ) 
        Refer(planConfig);

    if( oldPlanConfig ) 
        DeRefer(oldPlanConfig);
}

// prints the plan entry
void PlanEntry_t::Print(void) const
{
    cout << "Adornment" << endl;
    adornment->Print();
        
    cout << "Plan" << endl;
    if( planConfig )
        planConfig->Print();
    else cout << "NULL" << endl;
}

// plan entry destructor
PlanEntry_t::~PlanEntry_t(void)
{
    DeRefer(adornment);
    if( planConfig )
        DeRefer(planConfig);
}

// plan cache entry constructor
PlanCacheEntry_t::PlanCacheEntry_t(ActiveStack_t& stack)
                                : stackConfig(NULL), table(NULL)
{
    stackConfig = stack.Config();
    Refer(stackConfig);
}

// plan cache entry copy constructor
PlanCacheEntry_t::PlanCacheEntry_t(const PlanCacheEntry_t& e)
{
    stackConfig = e.StackConfig();
    Refer(stackConfig);

    table = e.Table();
    if( table )
        Refer(table);
}

// plan cache entry equality comparison
int PlanCacheEntry_t::operator == (PlanCacheEntry_t& entry)
{
    ActiveStackConfig_t *entryStackConfig = entry.StackConfig();
    assert(entryStackConfig);
    assert(stackConfig);

    return *entryStackConfig == *stackConfig;
}

// plan cache entry destructor
PlanCacheEntry_t::~PlanCacheEntry_t(void)
{
    DeRefer(stackConfig);
    if( table )
        DeRefer(table);
}

// search for plan in the table corresponding to a stack configuration
PlanConfig_t *PlanCacheEntry_t::SearchPlan(SharStack_t& sharStack)
{
    // table not initialized --- nothing inserted yet
    if( !table )
        return NULL;

    PlanEntry_t key(sharStack);
    int found = 0;

    PlanEntry_t *foundKey = NULL;
    Error_t errVal = table->Search(key, NULL, &found, &foundKey);
    assert(errVal == E_OK);

    if( !found )
        return NULL;

    assert(foundKey);
    PlanConfig_t *planConfig = foundKey->PlanConfig();

    assert(planConfig);
    return planConfig;
}

// insert a plan in the table corresponding to a stack configuration
void PlanCacheEntry_t::InsertPlan(SharStack_t& sharStack,
                                        PlanConfig_t *planConfig)
{
    assert(planConfig);

    PlanEntry_t key(sharStack, planConfig);

    if( table ) {
        // check if an entry exists for given stack configuration
        int found = 0;
        PlanEntry_t *foundKey = NULL;
        Error_t errVal = table->Search(key, NULL, &found, &foundKey);
        assert(errVal == E_OK);

        if( found ) {
            assert(foundKey);

            PlanConfig_t *prevConfig = foundKey->PlanConfig();
            assert(prevConfig);

            if( Config_t::IsDebug() )
                cout << "Plan Updated ... " << endl;

            foundKey->SetPlanConfig(planConfig);
            return;
        }
    } else {
        // create the table if not yet created
        table = new Lookup_t<INNERTABLESIZE, PlanEntry_t>;
        Refer(table);
    }

    Error_t errVal = table->Insert(key);
    assert(errVal == E_OK);

    if( Config_t::IsDebug() )
        cout << "Plan Stored" << endl;
}

// search for a plan corresponding to a stack config and adornment
PlanConfig_t *PlanCache_t::Search(ActiveStack_t& activeStack,
                                        SharStack_t& sharStack)
{
    PlanCacheEntry_t key(activeStack);
    int found = 0;

    PlanCacheEntry_t *foundKey = NULL;
    Error_t errVal = table.Search(key, NULL, &found, &foundKey);
    assert(errVal == E_OK);

    if( !found )
        return NULL;

    assert(foundKey);

    return foundKey->SearchPlan(sharStack);
}

// insert a plan corresponding to a stack config and adornment
void PlanCache_t::Insert(ActiveStack_t& activeStack, SharStack_t& sharStack,
                                                    PlanConfig_t *planConfig)
{
    PlanCacheEntry_t key(activeStack);
    int found = 0;

    PlanCacheEntry_t *foundKey = NULL;
    Error_t errVal = table.Search(key, NULL, &found, &foundKey);
    assert(errVal == E_OK);

    if( !found ) {
        errVal = table.Insert(key, &foundKey);
        assert(errVal == E_OK);
    }

    if( Config_t::IsDebug() ) {
        cout << "------------------" << endl;
        cout << "Inserting ..." << endl;
        cout << "Active Stack" << endl;
        activeStack.Print();
        cout << "Shar Stack" << endl;
        sharStack.Print();
        cout << "Plan Config" << endl;
        planConfig->Print();
        cout << "------------------" << endl;
    }

    assert(foundKey);
    foundKey->InsertPlan(sharStack, planConfig);
}

// quick sort for active stack entry list
static void SortActiveStackEntries(int numElements,
                                            ActiveStackEntry_t **base)
{
    assert(base);
    if( numElements <= 1 )
        return;

    qsort(base, numElements, sizeof(ActiveStackEntry_t *),
                                    CompareActiveStackEntries);
}

// comparison function used in sorting and binary search
static int CompareActiveStackEntries(const void *x, const void *y)
{
    assert(x);
    assert(y);

    const ActiveStackEntry_t * const *xEntry =
                                (const ActiveStackEntry_t * const *) x;
    assert(*xEntry);

    const ActiveStackEntry_t * const *yEntry =
                                (const ActiveStackEntry_t * const *) y;
    assert(*yEntry);

    PlanGroup_t *xpg = (*xEntry)->PlanGroup();
    PlanGroup_t *ypg = (*yEntry)->PlanGroup();

    return xpg->ID() - ypg->ID();
}

// quick sort for plan group entries list
static void SortPlanGroups(int numElements, PlanGroup_t **base)
{
    assert(base);
    if( numElements <= 1 )
        return;

    qsort(base, numElements, sizeof(PlanGroup_t *), ComparePlanGroups);
}

// comparison function used in sorting and binary search
static int ComparePlanGroups(const void *x, const void *y)
{
    assert(x);
    assert(y);

    const PlanGroup_t * const *xpg = (const PlanGroup_t * const *) x;
    assert(*xpg);

    const PlanGroup_t * const *ypg = (const PlanGroup_t * const *) y;
    assert(*ypg);

    return (*xpg)->ID() - (*ypg)->ID();
}
