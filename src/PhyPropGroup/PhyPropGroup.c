// physical property group class implementation

#include "List.h"
#include "PhyProp.h"
#include "PhyPropGroup.h"

// check if the physical property is covered
int PhyPropGroup_t::Covers(SchemaProp_t *schemaProp,
                                        PhyProp_t *q) const
{
    assert(q);
    assert(schemaProp);

    ListIter_t<PhyProp_t *> iter;
    iter.Attach(&propList);

    while( !iter.IsEnd() ) {
        PhyProp_t *p = iter.Next();
        if( p->Covers(schemaProp, q) )
            return 1;
    }

    return 0;
}

// check if some property is covered by p
int PhyPropGroup_t::IsCovered(SchemaProp_t *schemaProp, PhyProp_t *p) const
{
    assert(schemaProp);
    assert(p);

    ListIter_t<PhyProp_t *> iter;
    iter.Attach(&propList);

    while( !iter.IsEnd() ) {
        PhyProp_t *q = iter.Next();
        if( p->Covers(schemaProp, q) )
            return 1;
    }

    return 0;
}

// check if two physical prop groups are equal
int PhyPropGroup_t::IsEquivalent(SchemaProp_t *schemaProp,
                                            PhyPropGroup_t *pg) const
{
    assert(schemaProp);
    assert(pg);

    const List_t<PhyProp_t *>& pgPropList = pg->PropList();

    ListIter_t<PhyProp_t *> iter;
    iter.Attach(&propList);

    ListIter_t<PhyProp_t *> pgIter;
    pgIter.Attach(&pgPropList);

    int isEq = 1;       // equality flag

    // check if $propList$ contains $pgPropList$
    while( isEq && !pgIter.IsEnd() ) {
        PhyProp_t *p = pgIter.Next();
        iter.Reset();
        isEq = 0;
        while( !isEq && !iter.IsEnd() ) {
            PhyProp_t *q = iter.Next();
            if( q->Covers(schemaProp, p) )
                isEq = 1;
        }
    }

    // check if $pgPropList$ contains $propList$
    iter.Reset();
    while( isEq && !iter.IsEnd() ) {
        PhyProp_t *p = iter.Next();
        pgIter.Reset();
        isEq = 0;
        while( !isEq && !pgIter.IsEnd() ) {
            PhyProp_t *q = pgIter.Next();
            if( q->Covers(schemaProp, p) )
                isEq = 1;
        }
    }

    return isEq;
}

// check if $this$ covers the given physical property
int PhyPropGroup_t::Covers(SchemaProp_t *schemaProp,
                                        PhyPropGroup_t *pg) const
{
    assert(schemaProp);
    assert(pg);

    List_t<PhyProp_t *>& pgPropList = pg->PropList();

    ListIter_t<PhyProp_t *> iter;
    iter.Attach(&propList);

    ListIter_t<PhyProp_t *> pgIter;
    pgIter.Attach(&pgPropList);

    int isEq = 1;       // equality flag

    // check if $propList$ contains $pgPropList$
    while( isEq && !pgIter.IsEnd() ) {
        PhyProp_t *p = pgIter.Next();
        assert(p);

        iter.Reset();
        isEq = 0;
        while( !isEq && !iter.IsEnd() ) {
            PhyProp_t *q = iter.Next();
            if( q->Covers(schemaProp, p) )
                isEq = 1;
        }
    }

    return isEq;
}

// translate this physical property group on the src schema to a
// physical property group on the dest schema
PhyPropGroup_t *PhyPropGroup_t::Translate(LogProp_t *srcLP, LogProp_t *destLP)
{
    assert(srcLP);
    assert(destLP);

    PhyPropGroup_t *dest = new PhyPropGroup_t;

    ListIter_t<PhyProp_t *> iter;
    iter.Attach(&propList);

    while( !iter.IsEnd() ) {
        PhyProp_t *srcP = iter.Next();
        assert(srcP);

        PhyProp_t *destP = srcP->Translate(srcLP, destLP);
        if( !destP ) {
            // translation not possible
            delete dest;

            return NULL;
        }

        Refer(destP);
        dest->AddProperty(destP);
        DeRefer(destP);
    }

    return dest;
}
