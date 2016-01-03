// keys of heaps used in greedy heuristic

#ifndef __HEAPKEY_H__
#define __HEAPKEY_H__

#include "typedefs.h"
#include "Plan.h"

// sharable heap key -- ordered by upper bound on benefit
class SharKey_t {
    PlanGroup_t *pg;    // plan group
    CostVal_t benefit;  // upper bound on benefit of materializing
                        // this plan group

public:
    SharKey_t(PlanGroup_t *pg_a = NULL, CostVal_t benefit_a = -1)
        : pg(pg_a), benefit(benefit_a)
    { if( pg ) Refer(pg); }

    // copy constructor
    SharKey_t(const SharKey_t& k)
    {
        pg = k.PlanGroup();
        if( pg ) Refer(pg);
        benefit = k.Benefit();
    }

    // assignment
    SharKey_t& operator = (const SharKey_t& k)
    {
        PlanGroup_t *oldpg = pg;

        pg = k.PlanGroup();
        if( pg ) Refer(pg);

        if( oldpg ) DeRefer(oldpg);

        benefit = k.Benefit();
        return *this;
    }

    // ordering relation for shar heap based on materialization benefit
    // highest benefit first
    int operator <= (const SharKey_t& k) const
    { return benefit >= k.Benefit(); }

    PlanGroup_t *PlanGroup(void) const
    { return pg; }

    void SetBenefit(CostVal_t benefit_a)
    { benefit = benefit_a; }

    CostVal_t Benefit(void) const
    { return benefit; }

    ~SharKey_t(void)
    { if( pg ) DeRefer(pg); }
};

// propagation heap key -- ordered by topological number
class PropKey_t {
    Equivalence_t *eq;  // equivalence class

public:
    PropKey_t(Equivalence_t *eq_a = NULL) : eq(eq_a)
    { }

    // ordering relation for prop heap
    int operator <= (const PropKey_t& k) const
    {
        assert(eq);
        Equivalence_t *keq = k.EqClass();
        assert(keq);
        return eq->TopNo() <= keq->TopNo();
    }

    Equivalence_t *EqClass(void) const
    { return eq; }

    ~PropKey_t(void)
    { }
};

#endif // __HEAPKEY_H__
