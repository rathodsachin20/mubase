// Optimizer interface definition

#ifndef __OPTIMIZER_H__
#define __OPTIMIZER_H__

#include "typedefs.h"

class Memo_t;
class Equivalence_t;
class Plan_t;

// generic optimization algorithm interface
class Optimizer_t {
public:
    Optimizer_t(void) { }

    virtual Memo_t& Memo(void) = 0;

    virtual PlanGroup_t *FindBestPlan(Equivalence_t *eq,
                         CostVal_t *costLimit, SortOrder_t *reqOrder=NULL) = 0;

    CostVal_t BestPlanCost(Equivalence_t *eq)
    {
        assert(eq);
        CostVal_t costLimit = Config_t::CostLimit();
        PlanGroup_t *pg = FindBestPlan(eq, &costLimit);
        if( !pg ) costLimit = -1;

        return costLimit;
    }

    virtual ~Optimizer_t(void) { }
};

#endif //  __OPTIMIZER_H__
