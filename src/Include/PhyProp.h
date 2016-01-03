// physical properties

#ifndef __PHYPROP_H__
#define __PHYPROP_H__

#include <iostream>
#include "RefCount.h"
#include "List.h"
#include "Set.h"
#include "LogProp.h"

using namespace std;

// Flags to indicate how am order should be translated
// from a given source schema to a target schema.
#define TRANSLATE_EXACT_ONLY     0
#define TRANSLATE_IGNORE_MISSING 1
#define TRANSLATE_PREFIX_ONLY    2

class PhyPropGroup_t;
class Plan_t;
class PlanGroup_t;
class Equivalence_t;
class Volcano_t;

// type of the physical property
enum PhyPropType_t {
    SORTORDER_T,
    INDEXORDER_T
};

// physical property virtual base class
class PhyProp_t : public RefC_t {
public:
    PhyProp_t(void) { }

    // type of the physical property
    virtual PhyPropType_t Type(void) const = 0;

    // check if the two properties are equivalent
    int IsEquivalent(const SchemaProp_t *schemaProp, const PhyProp_t *p) const
    { return IsCovered(schemaProp, p) && p->IsCovered(schemaProp, this); }

    // check if this property is covered by the given property
    virtual int IsCovered(const SchemaProp_t *schemaProp,
                                const PhyProp_t *p) const = 0;

    // check if this property covers the given property
    int Covers(const SchemaProp_t *schemaProp, const PhyProp_t *p) const
    { return p->IsCovered(schemaProp, this); }

    // property enforcer move
    virtual void Enforce(PlanGroup_t *planGroup, Volcano_t *opt,
            Cost_t *totalCost, Plan_t *bestPlan, CostVal_t *costLimit,
        Order_t *pso, int *plmap, int level, Card_t callCount) = 0;

    // translate this physical prop from src LP to dest LP
    virtual PhyProp_t *Translate(LogProp_t *srcLP, LogProp_t *destLP, 
                                 int translateMode=TRANSLATE_EXACT_ONLY) = 0;

    // print the property
    virtual void Print(void) const = 0;
    virtual void Print(const Schema_t *schema) const = 0;

    virtual ~PhyProp_t(void) { }
};

// derived classes
class Order_t;  // order

// order property base class
class Order_t : public PhyProp_t {
protected:
    int *order;         // list of attribute indices wrt the attribute list
                        // in the input schema --- defines the sort order
                        // Ravi: Why is the schema called as input schema?
                        // Ravi: Actuall the attr indices are wrt to the 
                        // Ravi: schema of the equivalence node associated
                        // Ravi: with the PlanGroup which contains this
                        // Ravi: Order_t.
    int orderSize;      // size of the above list

public:
    Order_t(const int *order_a, int orderSize_a)
        : order(NULL), orderSize(orderSize_a)
    {
        assert(orderSize > 0);
        order = new int[orderSize];

        assert(order_a);
        for( int i = 0 ; i < orderSize ; i++ )
            order[i] = order_a[i];
    }

    const int *Order(void) const
    { return order; }

    int OrderSize(void) const
    { return orderSize; }

    // print the property
    void Print(const Schema_t *schema) const
    { 
        assert(schema);

        cout << "< ";
        for( int i = 0 ; i < orderSize ; i++ ) {
            const Attr_t *a = schema->Attribute(order[i]);
            assert(a);

            cout << a->RelName() << ":" << a->Name() << " ";
        }
        cout << ">";
    }

    // print the property
    void Print(void) const
    { 
        cout << "< ";
        for( int i = 0 ; i < orderSize ; i++ )
            cout << order[i] << " ";
        cout << ">";
    }

    // print the property
    void PrintSQL(const Schema_t *schema) const
    { 
        assert(schema);

        for( int i = 0 ; i < orderSize ; i++ ) {
            const Attr_t *a = schema->Attribute(order[i]);
            assert(a);

            cout << a->Name() << " NUMBER";
            if( i < orderSize-1 )
                cout << ", ";
        }
    }

    virtual ~Order_t(void)
    { delete[] order; }
};
            
// derived classes
class SortOrder_t;      // sort order
class IndexOrder_t;     // index order

// sort order
class SortOrder_t : public Order_t {
public:
    SortOrder_t(const int *order_a, int orderSize_a) : Order_t(order_a, orderSize_a)
    { }

    PhyPropType_t Type(void) const
    { return SORTORDER_T; }

    int IsCovered(const SchemaProp_t *schemaProp, const PhyProp_t *p) const;

    // sort enforcer move
    void Enforce(PlanGroup_t *planGroup, Volcano_t *opt,
            Cost_t *totalCost, Plan_t *bestPlan, CostVal_t *costLimit,
        Order_t *pso, int *plmap, int level, Card_t callCount);

    // translate this physical prop from src LP to dest LP
    PhyProp_t *Translate(LogProp_t *srcLP, LogProp_t *destLP, int ignoreMissing=0);

    // Longest prefix of the order having attributes only from a given set 
    // of attributes. Returns NULL when the prefix length is 0.
    SortOrder_t *LongestCommonPrefix(const Set_t<int> *attrSet);

    // Longest prefix w.r.t the given order.
    // Returns NULL when the prefix length is 0.
    SortOrder_t *LongestCommonPrefix(const Order_t *ord);
   
    // Form a new order as (this + ord). There should be no common
    // attributes between this and ord.
    SortOrder_t *Concat(const Order_t *ord);

    Set_t<int> *AttrSet()
    {
        return new Set_t<int>(order, orderSize);
    }

    SortOrder_t *CanonicalOrder(LogProp_t *lp)
    {
        SchemaProp_t *sp = lp->SchemaProp();
        int *cOrdArray = new int[orderSize];
        for (int i = 0; i < orderSize; i++) {
            cOrdArray[i] = sp->AttrPropRep(order[i]);
        }

       SortOrder_t *cOrd = new SortOrder_t(cOrdArray, orderSize);
       delete[] cOrdArray;
       return cOrd;
    }

    // The order is assumed to be in canonical form w.r.t srcLP.
    // Translates the order w.r.t. dstLP and returns in canonical form.
    SortOrder_t *TranslateFromCanonical(LogProp_t *srcLP, LogProp_t *dstLP);

    // Make an arbitrary order from the given set of attributes
    // FIX: Take some fixed criterion like alphabetical order of the attrs.
    static SortOrder_t *MakeOrder(const Set_t<int> *attrSet)
    {
        return new SortOrder_t(attrSet->Elements(), attrSet->Size());
    }

    ~SortOrder_t(void)
    { }
};

// index order
class IndexOrder_t : public Order_t {
    // index enforcer move
    void EnforceIndex(PlanGroup_t *planGroup, Volcano_t *opt,
            Cost_t *totalCost, Plan_t *bestPlan, CostVal_t *costLimit,
        Order_t *pso, int *plmap, int level, Card_t callCount);

    // index sort enforcer move
    void EnforceIndexSort(PlanGroup_t *planGroup, Volcano_t *opt,
            Cost_t *totalCost, Plan_t *bestPlan, CostVal_t *costLimit,
        Order_t *pso, int *plmap, int level, Card_t callCount);

public:
    IndexOrder_t(int *order_a, int orderSize_a) : Order_t(order_a, orderSize_a)
    { }

    PhyPropType_t Type(void) const
    { return INDEXORDER_T; }

    int IsCovered(const SchemaProp_t *schemaProp, const PhyProp_t *p) const;

    // index enforcer move
    void Enforce(PlanGroup_t *planGroup, Volcano_t *opt,
            Cost_t *totalCost, Plan_t *bestPlan, CostVal_t *costLimit,
        Order_t *pso, int *plmap, int level, Card_t callCount);

    // translate this physical prop from src LP to dest LP
    PhyProp_t *Translate(LogProp_t *, LogProp_t *, int ignoreMissing=0)
    { return NULL; }

    ~IndexOrder_t(void)
    { }
};
            
            
List_t<Order_t *>* TranslateOrders(List_t<Order_t *> *ords, LogProp_t *srcLP, 
                   LogProp_t *destLP, int flag=TRANSLATE_IGNORE_MISSING);
void UnionOrders(List_t<Order_t *> *dest, List_t<Order_t *> *src, 
                 LogProp_t *logProp);
void PrintOrders(List_t<Order_t *> *ords, Schema_t *schema);


#endif // __PHYPROP_H__
