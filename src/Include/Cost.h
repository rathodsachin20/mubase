// Cost ADT implementation

#ifndef __COST_H__
#define __COST_H__

#include <assert.h>
#include <stddef.h>

#include "typedefs.h"
#include "RefCount.h"
#include "CostList.h"

// type of cost
enum CostType_t {
    COSTORD_T,
    COSTSET_T
};

// physical operators
class PhysicalOp_t;

// cost abstract base class
class Cost_t : public RefC_t {
public:
    Cost_t(void) { }

    virtual CostType_t CostType(void) const = 0;

    virtual CostVal_t Value(void) const = 0;

    // returns the value of the union of the two costs
    virtual CostVal_t SumCost(const Cost_t *cost) const = 0;

    virtual void Add(const Cost_t *x) = 0;
    virtual void Subtract(const Cost_t *x) = 0;
    virtual void Assign(const Cost_t *x) = 0;

    virtual void MakeZero(void) = 0;

    virtual Cost_t *Clone(void) const = 0;

    virtual ~Cost_t(void) { }
};

// ordinary cost
class CostOrd_t : public Cost_t {
    CostVal_t value;

public:
    CostOrd_t(CostVal_t value_a = 0) : value(value_a) { }

    CostType_t CostType(void) const
    { return COSTORD_T; }

    CostVal_t Value(void) const
    { return value; }

    // returns the value of the union of the two costs
    CostVal_t SumCost(const Cost_t *cost) const
    {
        assert(cost);
        assert(cost->CostType() == COSTORD_T);
        return value + cost->Value();
    }

    void Add(const Cost_t *x)
    {
        assert(x);
        value += x->Value();
    }

    void Subtract(const Cost_t *x)
    {
        assert(x);
        value -= x->Value();
    }

    void Assign(const Cost_t *x)
    {
        assert(x);
        value = x->Value();
    }

    void Assign(CostVal_t x)
    { value = x; }

    void MakeZero(void)
    { value = 0; }

    void Add(CostVal_t inc)
    { value += inc; }

    Cost_t *Clone(void) const
    {
        Cost_t *cost = new CostOrd_t(0);
        cost->Add(this);
        return cost;
    }

    ~CostOrd_t(void) { }
};

// set cost
class CostSet_t : public Cost_t {
    CostList_t list;

public:
    CostSet_t(void) { }

    CostSet_t(PhysicalOp_t *op_a, CostVal_t value_a)
    { list.Insert(op_a, value_a); }

    CostType_t CostType(void) const
    { return COSTSET_T; }

    CostVal_t Value(void) const
    { return list.Value(); }

    const CostList_t *List(void) const
    { return &list; }

    // returns the value of the union of the two costs
    CostVal_t SumCost(const Cost_t *cost) const;

    void Add(const Cost_t *x);

    void Subtract(const Cost_t *x);

    void MakeZero(void);

    void Assign(const Cost_t *x);

    Cost_t *Clone(void) const
    {
        Cost_t *cost = new CostSet_t;
        cost->Add(this);
        return cost;
    }

    ~CostSet_t(void) { }
};

#endif  // __COST_H__
