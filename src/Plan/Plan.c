// evaluation plan class

#include <assert.h>
#include <iostream>

#include "Node.h"
#include "PhysicalOp.h"
#include "Volcano.h"
#include "Plan.h"

int PlanGroup_t::Count = 0;             // plan group instance count
int PlanGroup_t::CurID = 0;             // plan group id generator
int PlanGroup_t::CurTravID = 0;         // traversal count

// id of the plan is taken to be the same as the id of the root of the plan
int Plan_t::ID(void) const
{
    assert(root);
    return root->ID();
}

// print the plan
void Plan_t::PrintPlan(int indentLevel) const
{
    assert(planGroup);

    Equivalence_t *eq = planGroup->EqClass();
    assert(eq);

    for( int i = 0 ; i < indentLevel ; i++ )
        cout << "| ";
    
    cout << "[EQCLASS" << eq->ID() << "]";
    cout << "[PLANGRP" << planGroup->ID() << "]";
    cout << "[CC=" << planGroup->CallCount() << "]";
    LogProp_t *outerVarLogProp = eq->OuterVarLogProp();
    if (outerVarLogProp) {
        cout << "[PSO=";
        Order_t *gpso = planGroup->ParameterSortOrder();
        if (gpso)
                gpso->Print(outerVarLogProp->Schema());
        else
                cout << "NULL";
        cout << "]";
    }
    cout << " UseCost = " << eq->UseCost();
    cout << " Plan Cost = " << CostVal();
    cout << " #Tuples = " << eq->LogicalProp()->RelSize_Tuples();
    cout << " #Blocks = " << eq->LogicalProp()->RelSize_Blocks();
    if( eq->IsCorrelated() )
        cout << " CORRELATED";
    if( planGroup->IsMarked() )
        cout << " MARKED";
    if( planGroup->IsShared() )
        cout << " SHARED";
    if( planGroup->IsBuiltIn() )
        cout << " BUILTIN";
    cout << endl;

    if( root )
        root->PrintPlan(indentLevel+1);
    else cout << "[[FAILED]]";
}

// add an algorithm plan to the plan group
Plan_t *PlanGroup_t::AddAlgPlan(PhysicalOp_t *op, Cost_t *cost)
{
    assert(op);
    assert(cost);
    // assert(!IsIndexed());

    Plan_t *p = new Plan_t(this, op, cost);
    algPlans.Insert(p);

    // back pointer
    op->SetPlan(p);

    return p;
}

// add an enforcer plan to the plan group
Plan_t *PlanGroup_t::AddEnfPlan(PhysicalOp_t *op, Cost_t *cost)
{
    assert(op);
    assert(cost);

    Plan_t *p = new Plan_t(this, op, cost);
    enfPlans.Insert(p);

    // back pointer
    op->SetPlan(p);

    return p;
}

// cost adt for using the materialized intermediate result
Cost_t *PlanGroup_t::UseCostADT(Volcano_t *opt)
{
    if( !useCost ) {
        useCost = IsIndexed() ?
                    opt->CreateCost(NULL, 0) : eqClass->UseCostADT(opt);
        Refer(useCost);
    }

    return useCost;
}

// merge pg with this
void PlanGroup_t::Merge(PlanGroup_t *pg)
{
    assert(pg);

    if( Config_t::IsDebug() )
        cout << "Merging pg " << pg->ID() << " into pg " << ID() << endl;

    List_t<Plan_t *>& pgAlgPlans = pg->AlgPlans();

    while( !pgAlgPlans.IsEmpty() ) {
        Plan_t *p = pgAlgPlans.DeleteTop();
        assert(p);

        algPlans.Insert(p);
    }

    List_t<Plan_t *>& pgEnfPlans = pg->EnfPlans();

    while( !pgEnfPlans.IsEmpty() ) {
        Plan_t *p = pgEnfPlans.DeleteTop();
        assert(p);

        enfPlans.Insert(p);
    }

    List_t<PhysicalOp_t *>& pgAlgParents = pg->AlgParents();

    while( !pgAlgParents.IsEmpty() ) {
        PhysicalOp_t *op = pgAlgParents.DeleteTop();
        assert(op);

        algParents.Insert(op);
    }

    List_t<PhysicalOp_t *>& pgEnfParents = pg->EnfParents();

    while( !pgEnfParents.IsEmpty() ) {
        PhysicalOp_t *op = pgEnfParents.DeleteTop();
        assert(op);

        enfParents.Insert(op);
    }
}

// detach the plangroup from the equivalence class
void PlanGroup_t::Detach(void)
{
    eqClass = NULL;

    // detach members of super
    POEdgeIter_t pgIter;
    pgIter.Attach(&super);

    PlanGroup_t *pg = NULL;
    while( (pg = pgIter.Next()) )
        pg->Detach();

    pgIter.Detach();
}

// does the pg contain an indexorder req prop?
int PlanGroup_t::IsIndexed(void)
{
    if( isIndexed == -1 ) {
        assert(prop);

        List_t<PhyProp_t *>& list = prop->PropList();
        ListIter_t<PhyProp_t *> iter;
        iter.Attach(&list);

        isIndexed = 0;
        while( !isIndexed && !iter.IsEnd() ) {
            PhyProp_t *p = iter.Next();
            assert(p);

            if( p->Type() == INDEXORDER_T )
                isIndexed = 1;
        }
    }

    return isIndexed;
}

// destructor for PlanGroup_t
PlanGroup_t::~PlanGroup_t(void)
{
    if( bestPlan ) delete bestPlan;
    if( bestAlgPlan ) delete bestAlgPlan;
    if( bestEnfPlan ) delete bestEnfPlan;

    // delete all plans
    ListIter_t<Plan_t *> iter;

    iter.Attach(&algPlans);
    while( !iter.IsEnd() ) {
        Plan_t *p = iter.Next();
        assert(p);

        PhysicalOp_t *root = p->Root();
        if( root )
            delete root;

        delete p;
    }

    iter.Attach(&enfPlans);
    while( !iter.IsEnd() ) {
        Plan_t *p = iter.Next();
        assert(p);

        PhysicalOp_t *root = p->Root();
        if( root )
            delete root;

        delete p;
    }

    DeRefer(prop);

    // derefer members of super
    POEdgeIter_t pgIter;
    pgIter.Attach(&super);

    PlanGroup_t *pg = NULL;
    while( (pg = pgIter.Next()) )
        DeRefer(pg);

    pgIter.Detach();

    if( useCost ) DeRefer(useCost);

    Count--;
}

