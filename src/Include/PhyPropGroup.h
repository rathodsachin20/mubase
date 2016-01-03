// physical property group

#ifndef __PHYPROPGROUP_H__
#define __PHYPROPGROUP_H__

#include "RefCount.h"
#include "List.h"
#include "PhyProp.h"

class PhyPropGroup_t : public RefC_t {
    PrependList_t<PhyProp_t *> propList;        // list of physical properties

public:
    PhyPropGroup_t(void) { }

    // add a new property
    // ref counting okay because deletions from propGroup not supported
    void AddProperty(PhyProp_t *prop)
    { Refer(prop); propList.Insert(prop); }

    // Make the group empty
    void MakeEmpty()
    { propList.MakeEmpty(); }

    // number of physical properties
    int NumProps(void) const
    { return propList.Size(); }

    // check if the property q is covered
    int Covers(SchemaProp_t *schemaProp, PhyProp_t *q) const;

    // check if some property is covered by p
    int IsCovered(SchemaProp_t *schemaProp, PhyProp_t *p) const;

    // check if two physical prop groups are equivalent
    int IsEquivalent(SchemaProp_t *schemaProp, PhyPropGroup_t *pg) const;

    // check if this covers pg
    int Covers(SchemaProp_t *schemaProp, PhyPropGroup_t *pg) const;

    // check if pg covers this
    int IsCovered(SchemaProp_t *schemaProp, PhyPropGroup_t *pg)
    { return pg->Covers(schemaProp, this); }

    // translate this physical property group on the src schema to a
    // physical property group on the dest schema
    PhyPropGroup_t *Translate(LogProp_t *srcLP, LogProp_t *destLP);

    // propGroup
    List_t<PhyProp_t *>& PropList(void)
    { return propList; }

    // print the physical properties in the group
    void Print(void) const
    {
        ListIter_t<PhyProp_t *> propIter;
        propIter.Attach(&propList);

        while( !propIter.IsEnd() ) {
            PhyProp_t *p = propIter.Next();
            p->Print();
            cout << endl;
        }
    }

    void Print(Schema_t *schema) const
    {
        assert(schema);
        ListIter_t<PhyProp_t *> propIter;
        propIter.Attach(&propList);

        while( !propIter.IsEnd() ) {
            PhyProp_t *p = propIter.Next();
            p->Print(schema);
            cout << endl;
        }
    }

    ~PhyPropGroup_t(void)
    {
        ListIter_t<PhyProp_t *> propIter;
        propIter.Attach(&propList);

        while( !propIter.IsEnd() ) {
            PhyProp_t *p = propIter.Next();
            DeRefer(p);
        }
    }
};

#endif // __PHYPROPGROUP_H__

