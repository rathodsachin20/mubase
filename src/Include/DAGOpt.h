// dag optimization

#ifndef __DAGOPT_H__
#define __DAGOPT_H__

#include "typedefs.h"
#include "RefCount.h"
#include "Cost.h"
#include "List.h"
#include "Volcano.h"
#include "Optimizer.h"

class Equivalence_t;            // equivalence class
class PhysicalOp_t;             // physical operator
class Plan_t;                   // evaluation plan

// base class for dag optimization algorithms
class DAGOpt_t : public Optimizer_t {
protected:
    VolcanoOrd_t volcano;       // associated volcano optimizer

public:
    DAGOpt_t(int isUnify = 1) : volcano(isUnify, 1) // unify and fully expand
    { }

    Memo_t& Memo(void) { return volcano.Memo(); }

    virtual PlanGroup_t *FindBestPlan(Equivalence_t *eq,
                         CostVal_t *costLimit, SortOrder_t *reqOrder=NULL) = 0;

    virtual ~DAGOpt_t(void) { }
};

#endif  // __DAGOPT_H__
