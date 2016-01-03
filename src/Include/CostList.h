// cost entry list

#ifndef __COSTLIST_H__
#define __COSTLIST_H__

#include <assert.h>
#include <stddef.h>
#include "typedefs.h"

// physical operators
class PhysicalOp_t;

// cost list class
class CostList_t;

// data to be stored per cost list entry
struct CostSetEntry_t {
    const PhysicalOp_t *op;
    CostVal_t opCost;

    CostSetEntry_t(const PhysicalOp_t *op_a = NULL, CostVal_t opCost_a = 0)
        : op(op_a), opCost(opCost_a) { }

    const PhysicalOp_t *PhysicalOp(void)
    { return op; }

    CostSetEntry_t *Clone(void)
    { return new CostSetEntry_t(op, opCost); }

    ~CostSetEntry_t(void) { }
};

// node in the linked list
struct CostListEntry_t {
    int count;

    CostSetEntry_t *data;
    CostListEntry_t *next;

    CostListEntry_t(void) : count(0), data(NULL), next(NULL) { }
    ~CostListEntry_t(void) { }
};

// handler for sorted linked list
// handles addition and subtraction of lists
class CostListHandler_t {
public:
    // cost of union of two lists
    static CostVal_t SumCost(const CostList_t *x, const CostList_t *y);

    // addition of list $y$ to list $x$ -- $x$ contains the result
    static void Add(CostList_t *x, const CostList_t *y);
    
    // subtraction of list $y$ from list $x$ -- $x$ contains the result
    static void Subtract(CostList_t *x, const CostList_t *y);
};
    
// sorted linked list
class CostList_t {
    CostVal_t value;            // cost value

    int size;                   // size of the list
    CostListEntry_t *first;     // first element in the list

public:
    CostList_t(void) : value(0), size(0), first(NULL)
    { }

    CostVal_t Value(void) const
    { return value; }

    int Size(void) const
    { return size; }

    // insert an element -- returns current count of the element
    int Insert(PhysicalOp_t *op_a, CostVal_t val_a);

    // value of the union of the two lists
    CostVal_t SumCost(const CostList_t *list_a) const
    { return CostListHandler_t::SumCost(this, list_a); }

    void Add(const CostList_t *list_a)
    { CostListHandler_t::Add(this, list_a); }

    void Subtract(const CostList_t *list_a)
    { CostListHandler_t::Subtract(this, list_a); }

    // empty the list
    void MakeEmpty(void);

    // delete the elements in the list
    ~CostList_t(void)
    { MakeEmpty(); }

    friend class CostListHandler_t;     // associated iterator class
};


#endif    // __COSTLIST_H__
