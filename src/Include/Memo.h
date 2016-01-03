// hash table for indexing logical expressions

#ifndef __MEMO_H__
#define __MEMO_H__

#include <assert.h>

#include "Lookup.h"
#include "LogicalOp.h"

const int MEMOSIZE = 1000;

// key for the lookup table
struct MemoKey_t {
    LogicalOp_t *op;

    MemoKey_t(LogicalOp_t *x = NULL) : op(x)
    { assert(op); }

    int HashValue(void)
    { return op->HashValue()%MEMOSIZE; }

    ~MemoKey_t(void) { }
};

inline int operator ==(MemoKey_t& x, MemoKey_t& y)
{
    assert(x.op);
    assert(y.op);

    return x.op->IsEquivalent(y.op);
}

class Memo_t {
    // lookup table
    Lookup_t<MEMOSIZE, MemoKey_t> table;
    int isUnify;

public:
    Memo_t(int isUnify_a) : isUnify(isUnify_a) { }

    // inserts the operator into the table with the equivalence class
    void Insert(LogicalOp_t *op);

    // searches for the logical operator -- returns the existing
    // operator if found; NULL otherwise
    LogicalOp_t *Search(LogicalOp_t *op);

    // removes the logical operator
    void Remove(LogicalOp_t *op);

    // unification enabled?
    int IsUnify(void)
    { return isUnify; }

    ~Memo_t(void) { }
};

#endif // __MEMO_H__
