// cost set list

#include <assert.h>
#include "typedefs.h"
#include "PhysicalOp.h"
#include "CostList.h"

// insert an element -- returns current count of the element
int CostList_t::Insert(PhysicalOp_t *op_a, CostVal_t val_a)
{
    // search for newcostEntry in the sorted list
    CostListEntry_t *p = first;
    CostListEntry_t *q = NULL;

    int id = op_a ? op_a->ID() : -1;
    int pid = -1;
    while( p ) {
        assert(p->data);
        assert(p->data->op);

        pid = p->data->op->ID();
        if( id <= pid )
            break;
        q = p;
        p = p->next;
    }

    if( p ) {
        if( id == pid ) {
            // entry present
            p->count++;
            return p->count;
        }
    }

    // create a new entry, initialize and insert
    CostListEntry_t *newEntry = new CostListEntry_t;

    newEntry->count = 1;
    newEntry->data = new CostSetEntry_t(op_a, val_a);
    newEntry->next = p;

    if( q ) // p is not the first entry
        q->next = newEntry;
    else first = newEntry;

    value += val_a;

    return 1;
}

// empty the list
void CostList_t::MakeEmpty(void)
{
    CostListEntry_t *p = NULL;

    while( first ) {
        p = first->next;
        delete first->data;
        delete first;
        first = p;
    }

    value = 0;
}

// cost of union of lists $x$ and $y$
CostVal_t CostListHandler_t::SumCost(const CostList_t *x, const CostList_t *y)
{
    assert(x);
    assert(y);

    CostVal_t sumVal = 0;

    CostListEntry_t *p = x->first;
    CostListEntry_t *q = y->first;

    while( q ) {
        CostListEntry_t *r = q;
        q = q->next;

        assert(r->data);
        int rid = r->data->op ? r->data->op->ID() : -1;
        int pid = -1;

        while( p ) {
            assert(p->data);
            assert(p->data->op);
            pid = p->data->op->ID();

            if( rid <= pid )
                break;

            sumVal += p->data->opCost;

            p = p->next;
        }

        if( p ) {
            if( rid == pid ) 
                continue;
        }

        sumVal += r->data->opCost;
    }

    while( p ) {
        assert(p->data);
        sumVal += p->data->opCost;

        p = p->next;
    }

    return sumVal;
}

// addition of list $y$ to list $x$ -- $x$ contains the result
void CostListHandler_t::Add(CostList_t *x, const CostList_t *y)
{
    assert(x);
    // trivial case
    if( !y ) return;

    // merge the two sorted lists -- final list in $x$
    CostListEntry_t *p = x->first;
    CostListEntry_t *q = y->first;
    CostListEntry_t *r = NULL;
    CostListEntry_t *s = NULL;

    while( q ) {
        r = q;
        q = q->next;

        assert(r->data);
        int rid = r->data->op ? r->data->op->ID() : -1;
        int pid = -1;

        while( p ) {
            assert(p->data);
            assert(p->data->op);
            pid = p->data->op->ID();

            if( rid <= pid )
                break;
            s = p;
            p = p->next;
        }

        if( p ) {
            if( rid == pid ) {
                // equal nodes
                p->count += r->count;
                continue;
            }
        }

        // insert a copy of r between s and p
        CostListEntry_t *newEntry = new CostListEntry_t;
        newEntry->data = r->data->Clone();
        newEntry->count = r->count;
        newEntry->next = p;

        if( s )
            s->next = newEntry;
        else x->first = newEntry;
        x->size++;

        x->value += r->data->opCost;

        s = newEntry;
    }

    return;
}

// subtraction of list $y$ from list $x$ -- $x$ contains the result
void CostListHandler_t::Subtract(CostList_t *x, const CostList_t *y)
{
    assert(x);
    // trivial case
    if( !y ) return;

    // merge the two sorted lists -- final list in $x$
    CostListEntry_t *p = x->first;
    CostListEntry_t *q = y->first;
    CostListEntry_t *r = NULL;
    CostListEntry_t *s = NULL;

    while( p && q ) {
        r = q;
        q = q->next;

        assert(r->data);
        int rid = r->data->op ? r->data->op->ID() : -1;
        int pid = -1;

        while( p ) {
            assert(p->data);
            assert(p->data->op);
            pid = p->data->op->ID();

            if( rid <= pid )
                break;
            s = p;
            p = p->next;
        }

        if( p ) {
            if( rid == pid ) {
                // equal nodes
                p->count -= r->count;
                assert(p->count >= 0);
                if( p->count == 0 ) {
                    // needs to be deleted
                    if( s )
                        s->next = p->next;
                    else x->first = p->next;

                    x->value -= p->data->opCost;

                    delete p->data;
                    delete p;

                    if( s )
                        p = s->next;
                    else p = x->first;
                }
            }
        }
    }

    return;
}
