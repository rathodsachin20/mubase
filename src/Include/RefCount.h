// implements reference counting for space efficiency
// to be used only for acyclic structures

// Usage --- consider a derived class X
// const X *p is the pointer to be refered -- could be freshly allocated
// Refer(p);
// -- use p in a read-only mode --
// DeRefer(p)

#ifndef __REFCOUNT_H__
#define __REFCOUNT_H__

#include <assert.h>

// the base class for all the classes 
class RefC_t {
    int count;          // reference count

public:
    RefC_t(void) : count(0) { }

    virtual ~RefC_t(void) { assert(count == 0); }

    friend void Refer(RefC_t *e);
    friend void DeRefer(RefC_t *e);
    friend int IsShared(RefC_t *e);
};

// increment the ref count
inline void Refer(RefC_t *e)
{
    assert(e);
    e->count++;
}

// decrement the ref count and free if it is zero
inline void DeRefer(RefC_t *e)
{
    assert(e);
    assert(e->count > 0);
    e->count--;
    if( e->count == 0 )
        delete e;
}

// is the object shared?
inline int IsShared(RefC_t *e)
{ return e->count > 1; }

#endif // __REFCOUNT_H__
