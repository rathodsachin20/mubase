// logical expression cache

#include "LogicalOp.h"
#include "Memo.h"

// inserts the operator into the table with the equivalence class
void Memo_t::Insert(LogicalOp_t *op)
{
    assert(op);

    MemoKey_t key(op);
    Error_t retval = table.Insert(key);
    assert(retval == E_OK);
}

// searches for the logical operator -- returns the existing
// operator if found; NULL otherwise
LogicalOp_t *Memo_t::Search(LogicalOp_t *op)
{
    assert(op);

    MemoKey_t key(op);
    MemoKey_t *keyPresent = NULL;

    int found = 0;
    Error_t retval = table.Search(key, NULL, &found, &keyPresent);
    assert(retval == E_OK);

    LogicalOp_t *prevOp = NULL;
    if( found ) {
        prevOp = keyPresent->op;
        assert(prevOp);
    }
        
    return (LogicalOp_t *) prevOp;
}

// removes the logical operator
void Memo_t::Remove(LogicalOp_t *op)
{
    assert(op);

    MemoKey_t key(op);
    void *state = NULL;

    table.IterStateInit(&state);

    int found = 0;
    Error_t retval = table.Search(key, &state, &found);
    assert(retval == E_OK);

    assert(found);

    retval = table.Remove(&state);
    assert(retval == E_OK);

    table.IterStateDelete(&state);
}

