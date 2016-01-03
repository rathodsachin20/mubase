// cost adt

#include "PhysicalOp.h"
#include "Cost.h"

// returns the value of the union of the two costs
CostVal_t CostSet_t::SumCost(const Cost_t *cost_a) const
{
    assert(cost_a);
    assert(cost_a->CostType() == COSTSET_T);

    const CostSet_t *cost = (const CostSet_t *) cost_a;
    return list.SumCost(cost->List());
}

// add cost
void CostSet_t::Add(const Cost_t *x)
{
    assert(x);
    assert(x->CostType() == COSTSET_T);

    const CostSet_t *cost = (const CostSet_t *) x;
    list.Add(cost->List());
}

// subtract cost 
void CostSet_t::Subtract(const Cost_t *x)
{
    assert(x);
    assert(x->CostType() == COSTSET_T);

    const CostSet_t *cost = (const CostSet_t *) x;
    list.Subtract(cost->List());
}

// make the cost zero
void CostSet_t::MakeZero(void)
{ list.MakeEmpty(); }

// assign a list
void CostSet_t::Assign(const Cost_t *x)
{
    MakeZero();
    Add(x);
}
