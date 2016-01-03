// logical property manipulation routines

#include <assert.h>
#include "RefCount.h"
#include "Expr.h"
#include "PhyProp.h"
#include "LogProp.h"

// TODO -- for all integer UBound/LBound, split "<" case and a <= case

// equate attributes
float IntegerProp_t::Equate(AttrProp_t *p)
{
    assert(p);
    assert(p->Type() == INTATTR_T);
    IntegerProp_t *ip = (IntegerProp_t *) p;

    if( maxval > ip->MaxVal() )
        maxval = ip->MaxVal();

    if( minval < ip->MinVal() )
        minval = ip->MinVal();

    if( maxval < minval ) {
        cerr << "===> SELECTIVITY INFERRED AS 0 <===" << endl;
        numDistinct = 0;
        return 0.0;
    }

    assert(numDistinct >= 1);
    assert(ip->NumDistinct() >= 1);

    float sel;
    if( numDistinct > ip->NumDistinct() ) 
        sel = 1.0/numDistinct;
    else sel = 1.0/ip->NumDistinct();;

    if( numDistinct > ip->NumDistinct() ) 
        numDistinct = ip->NumDistinct();

    if( numDistinct > maxval - minval + 1 )
        numDistinct = maxval - minval + 1;

    return sel;
}

// attribute upper bound
float IntegerProp_t::UBound(AttrProp_t *p)
{
    assert(p);
    assert(p->Type() == INTATTR_T);
    IntegerProp_t *ip = (IntegerProp_t *) p;

    if( maxval <= ip->MinVal() ) {
        // no effect
        return 1.0;
    }

    if( maxval > ip->MaxVal() )
        maxval = ip->MaxVal();

    if( maxval < minval ) {
        numDistinct = 0;
        return 0.0;
    }

    if( numDistinct > maxval - minval + 1 )
        numDistinct = maxval - minval + 1;

    return 0.5;
}

// attribute lower bound
float IntegerProp_t::LBound(AttrProp_t *p)
{
    assert(p);
    assert(p->Type() == INTATTR_T);
    IntegerProp_t *ip = (IntegerProp_t *) p;

    if( minval >= ip->MaxVal() ) {
        // no effect
        return 1.0;
    }

    if( minval < ip->MinVal() )
        minval = ip->MinVal();

    if( maxval < minval ) {
        numDistinct = 0;
        return 0.0;
    }

    if( numDistinct > maxval - minval + 1 )
        numDistinct = maxval - minval + 1;

    return 0.5;
}

// fix attribute value
float IntegerProp_t::FixValue(Value_t *val)
{
    assert(val);
    assert(val->ValType() == INT_T);
    Int_t *intval = (Int_t *) val;
    int v = intval->Value();

    if( v < minval || v > maxval ) {
        // unsatisfiable ub or lb
        numDistinct = 0;
        return 0.0;
    }

    float sel = 1.0/numDistinct;
    numDistinct = 1;
    maxval = v;
    minval = v;

    return sel;
}

// value upper bound
float IntegerProp_t::UBound(Value_t *val)
{
    assert(val);
    assert(val->ValType() == INT_T);
    Int_t *intval = (Int_t *) val;
    int v = intval->Value();

    if( maxval <= v ) {
        // no effect
        return 1.0;
    }

    float sel = float(v - minval)/(maxval - minval);
    maxval = v;

    if( maxval < minval ) {
        numDistinct = 0;
        return 0.0;
    }

    if( numDistinct > maxval - minval + 1 )
        numDistinct = maxval - minval + 1;

    return sel;
}

// value lower bound
float IntegerProp_t::LBound(Value_t *val)
{
    assert(val);
    assert(val->ValType() == INT_T);
    Int_t *intval = (Int_t *) val;
    int v = intval->Value();

    if( minval >= v ) {
        // no effect
        return 1.0;
    }

    float sel = float(maxval - v)/(maxval - minval);
    minval = v;

    if( maxval < minval ) {
        numDistinct = 0;
        return 1.0;
    }

    if( numDistinct > maxval - minval + 1 )
        numDistinct = maxval - minval + 1;

    return sel;
}

// returns the union of the $this$ annd $prop$
// that is, the strongest property that implies both
AttrProp_t *IntegerProp_t::UnionProp(AttrProp_t *prop)
{
    assert(prop);
    assert(Type() == prop->Type());

    if( Subsumes(prop) )
        return (AttrProp_t *) this;

    if( prop->Subsumes(this) )
        return prop;

    const IntegerProp_t *iprop = (IntegerProp_t *) prop;

    int newMinVal = minval;
    if( newMinVal > iprop->MinVal() )
        newMinVal = iprop->MinVal();

    int newMaxVal = maxval;
    if( newMaxVal < iprop->MaxVal() )
        newMaxVal = iprop->MaxVal();

    Card_t newNumDistinct = numDistinct + iprop->NumDistinct();

    return new IntegerProp_t(newMinVal, newMaxVal, newNumDistinct);
}

// equate attributes
float IntegerProp_t::EquateSelectivity(AttrProp_t *p) const
{
    assert(p);
    assert(p->Type() == INTATTR_T);
    IntegerProp_t *ip = (IntegerProp_t *) p;

    int newmaxval = maxval;
    int newminval = minval;

    if( newmaxval > ip->MaxVal() )
        newmaxval = ip->MaxVal();

    if( newminval < ip->MinVal() )
        newminval = ip->MinVal();

    if( newmaxval < newminval ) {
        cerr << "===> SELECTIVITY INFERRED AS 0 <===" << endl;
        return 0.0;
    }

    assert(numDistinct > 1);
    assert(ip->NumDistinct() > 1);

    float sel;
    if( numDistinct > ip->NumDistinct() ) 
        sel = 1.0/numDistinct;
    else sel = 1.0/ip->NumDistinct();;

    return sel;
}

// attribute upper bound
float IntegerProp_t::UBoundSelectivity(AttrProp_t *) const
{ return 0.5; }

// attribute lower bound
float IntegerProp_t::LBoundSelectivity(AttrProp_t *) const
{ return 0.5; }

// fix attribute value selectivity
float IntegerProp_t::FixValueSelectivity(Value_t *val) const
{
    assert(val);
    assert(val->ValType() == INT_T);
    Int_t *intval = (Int_t *) val;
    int v = intval->Value();

    if( v < minval || v > maxval ) {
        // unsatisfiable ub or lb
        return 0.0;
    }

    return 1.0/numDistinct;
}

// value upper bound selectivity
float IntegerProp_t::UBoundSelectivity(Value_t *val) const
{
    assert(val);
    assert(val->ValType() == INT_T);
    Int_t *intval = (Int_t *) val;
    int v = intval->Value();

    if( minval > v ) {
        // unsatisfiable ub
        return 0.0;
    }

    if( maxval <= v ) {
        // no effect
        return 1.0;
    }

    return float(v - minval)/(maxval - minval);
}

// value lower bound selectivity
float IntegerProp_t::LBoundSelectivity(Value_t *val) const
{
    assert(val);
    assert(val->ValType() == INT_T);
    Int_t *intval = (Int_t *) val;
    int v = intval->Value();

    if( maxval < v ) {
        // unsatisfiable lb
        return 0.0;
    }

    if( minval >= v ) {
        // no effect
        return 1.0;
    }

    return float(maxval - v)/(maxval - minval);
}

// get the representative
int SchemaProp_t::AttrPropRep(int bnd) const
{
    assert(bnd >= 0 && bnd < numAttr);
    assert(rep[bnd] != -1);

    /* Ravi: Commenting out the following from Prasan's code as the
       eq class head is directly available at rep[bnd].

    if( rep[bnd] != bnd )
        rep[bnd] = AttrPropRep(rep[bnd]); 
    */
    return rep[bnd];
}


Set_t<int> *SchemaProp_t::AttrPropRep(Set_t<int> &attrSet) const
{
    const int *setElts = attrSet.Elements();
    int setSize = attrSet.Size();
    int repSet[setSize];

    for (int i = 0; i < setSize; i++) {
        repSet[i] = AttrPropRep(setElts[i]);
    }
    
    return new Set_t<int>(repSet, setSize);
}


// fix attribute value to the given
float SchemaProp_t::FixAttributeValueSelectivity(int bnd, Value_t *val) const
{
    assert(val);
    // get the representative
    int repbnd = AttrPropRep(bnd);

    float sel = prop[repbnd]->FixValueSelectivity(val);
    return sel;
}

// fix lower bound for attribute
float SchemaProp_t::LBoundAttributeValueSelectivity(int bnd, Value_t *val) const
{
    assert(val);
    // get the representative
    int repbnd = AttrPropRep(bnd);

    float sel = prop[repbnd]->LBoundSelectivity(val);
    return sel;
}

// fix upper bound for attribute
float SchemaProp_t::UBoundAttributeValueSelectivity(int bnd, Value_t *val) const
{
    assert(val);
    // get the representative
    int repbnd = AttrPropRep(bnd);

    float sel = prop[repbnd]->UBoundSelectivity(val);
    return sel;
}

// equate the attributes
float SchemaProp_t::EquateAttributesSelectivity(int leftbnd, int rightbnd) const
{
    // get the representatives
    int leftrepbnd = AttrPropRep(leftbnd);
    int rightrepbnd = AttrPropRep(rightbnd);

    if( leftrepbnd == rightrepbnd )
        // already equal
        return 1.0;

    return prop[leftrepbnd]->EquateSelectivity(prop[rightrepbnd]);
}

// constrain attr[lbnd] < attr[ubnd]
float SchemaProp_t::BoundAttributesSelectivity(int, int) const
{
    return 0.5;
}

// fix attribute value to the given
float SchemaProp_t::FixAttributeValue(int bnd, Value_t *val)
{
    assert(val);
    // get the representative
    int repbnd = AttrPropRep(bnd);

    // if the attribute property is shared, make a copy
    if( IsShared(prop[repbnd]) ) {
        DeRefer(prop[repbnd] );
        prop[repbnd] = prop[repbnd]->Copy();
        Refer(prop[repbnd]);
    }

    return prop[repbnd]->FixValue(val);
}

// fix lower bound for attribute
float SchemaProp_t::LBoundAttributeValue(int bnd, Value_t *val)
{
    assert(val);
    // get the representative
    int repbnd = AttrPropRep(bnd);

    // if the attribute property is shared, make a copy
    if( IsShared(prop[repbnd]) ) {
        DeRefer(prop[repbnd] );
        prop[repbnd] = prop[repbnd]->Copy();
        Refer(prop[repbnd]);
    }

    return prop[repbnd]->LBound(val);
}

// fix upper bound for attribute
float SchemaProp_t::UBoundAttributeValue(int bnd, Value_t *val)
{
    assert(val);
    // get the representative
    int repbnd = AttrPropRep(bnd);

    // if the attribute property is shared, make a copy
    if( IsShared(prop[repbnd]) ) {
        DeRefer(prop[repbnd] );
        prop[repbnd] = prop[repbnd]->Copy();
        Refer(prop[repbnd]);
    }

    return prop[repbnd]->UBound(val);
}

// equate the attributes
float SchemaProp_t::EquateAttributes(int leftbnd, int rightbnd)
{
    // get the representatives
    int leftrepbnd = AttrPropRep(leftbnd);
    int rightrepbnd = AttrPropRep(rightbnd);

    if( leftrepbnd == rightrepbnd )
        // already equal
        return 1.0;

    int parbnd = rightrepbnd;
    int childbnd = leftrepbnd;

    // if the right attribute property is shared, make a copy
    if( IsShared(prop[parbnd]) ) {
        DeRefer(prop[parbnd]);
        prop[parbnd] = prop[parbnd]->Copy();
        Refer(prop[parbnd]);
    }

    float sel = prop[parbnd]->Equate(prop[childbnd]);

    /* Ravi: Commented out the following

    DeRefer(prop[childbnd]);
    prop[childbnd] = NULL;
    rep[childbnd] = parbnd;
    */

    return sel;
}

// constrain attr[lbnd] < attr[ubnd]
float SchemaProp_t::BoundAttributes(int lbnd, int ubnd)
{
    // get the representatives
    int lrepbnd = AttrPropRep(lbnd);
    int urepbnd = AttrPropRep(ubnd);

    // if the attribute property is shared, make a copy
    if( IsShared(prop[lrepbnd]) ) {
        DeRefer(prop[lrepbnd] );
        prop[lrepbnd] = prop[lrepbnd]->Copy();
        Refer(prop[lrepbnd]);
    }

    if( IsShared(prop[urepbnd]) ) {
        DeRefer(prop[urepbnd] );
        prop[urepbnd] = prop[urepbnd]->Copy();
        Refer(prop[urepbnd]);
    }

    prop[lrepbnd]->UBound(prop[urepbnd]);
    prop[urepbnd]->LBound(prop[lrepbnd]);

    return 0.5; // naive approximation
}

// returns the merge of the given schemas
Schema_t *MergeSchema(Schema_t *leftSchema, Schema_t *rightSchema)
{
    assert(leftSchema);
    assert(rightSchema);

    int leftNumAttr = leftSchema->NumAttr();
    int rightNumAttr = rightSchema->NumAttr();
    Schema_t *schema = new Schema_t(leftNumAttr + rightNumAttr);

    int i = 0;
    while( i < leftNumAttr ) {
        Attr_t *a = leftSchema->Attribute(i);
        schema->SetAttribute(i, a);
        i++;
    }

    int j = 0;
    while( j < rightNumAttr ) {
        Attr_t *a = rightSchema->Attribute(j);
        schema->SetAttribute(i, a);
        i++;
        j++;
    }

    return schema;
}

// returns the merge of the given schema properties
SchemaProp_t *MergeSchemaProp(SchemaProp_t *leftSchemaProp,
                                        SchemaProp_t *rightSchemaProp)
{
    assert(leftSchemaProp);
    assert(rightSchemaProp);

    int leftNumAttr = leftSchemaProp->NumAttr();
    int rightNumAttr = rightSchemaProp->NumAttr();
    SchemaProp_t *schemaProp = new SchemaProp_t(leftNumAttr + rightNumAttr);

    int i = 0;
    while( i < leftNumAttr ) {
        AttrProp_t *ap = leftSchemaProp->AttributeProp(i);
        schemaProp->SetAttributeProp(i, ap);
        schemaProp->SetAttrPropRep(i, leftSchemaProp->AttrPropRep(i));
        i++;
    }

    int j = 0;
    while( j < rightNumAttr ) {
        AttrProp_t *ap = rightSchemaProp->AttributeProp(j);
        schemaProp->SetAttributeProp(i, ap);
        int newRep = rightSchemaProp->AttrPropRep(j) + leftNumAttr;
        schemaProp->SetAttrPropRep(i, newRep);
        i++;
        j++;
    }

    return schemaProp;
}

// print out the sql
void Schema_t::PrintSQL(SchemaProp_t *sp) const
{
    for( int i = 0 ; i < numAttr ; i++ ) {
        assert(attr[i]);
        cout << attr[i]->Name() << " NUMBER"; // hardcoded
        if (sp) {
            int head = sp->AttrPropRep(i);
            cout << " ClassHead : " << attr[head]->Name();
        }
        if( i < numAttr-1 )
            cout << ", ";
    }
}

// base relation log prop constructor
BaseRelLogProp_t::BaseRelLogProp_t(const char *name_a, Schema_t *schema_a,
    SchemaProp_t *schemaProp_a, Card_t relSize_tuples_a,
    Card_t tuplesPerBlock_a)
    : LogProp_t(name_a, schema_a, schemaProp_a, relSize_tuples_a),
                                        tuplesPerBlock(tuplesPerBlock_a)
{ assert(tuplesPerBlock >= 0); assert(finite(tuplesPerBlock)); }

// built in index list
void BaseRelLogProp_t::AddBuiltInIndex(IndexOrder_t *p)
{
    Refer(p);
    builtInIndexList.Insert(p);
}

// check if an index is built-in
int BaseRelLogProp_t::IsBuiltIn(PhyProp_t *p) const
{
    assert(p);

    SchemaProp_t *sp = SchemaProp();
    assert(sp);

    ListIter_t<IndexOrder_t *> iter;
    iter.Attach(&builtInIndexList);

    while( !iter.IsEnd() ) {
        IndexOrder_t *q = iter.Next();
        assert(q);

        // if( p->IsCovered(sp, q) && q->IsCovered(sp, p) )
        if( p->IsCovered(sp, q) )
            return 1;
    }

    return 0;
}

// returns the maximum common prefix of the given order that is built-in
SortOrder_t *BaseRelLogProp_t::BuiltInProp(SortOrder_t *ord) const
{
    assert(ord);

    ListIter_t<IndexOrder_t *> iter;
    iter.Attach(&builtInIndexList);

    SortOrder_t *maxBuiltInOrder = NULL;

    while( !iter.IsEnd() ) {
        // Treating the IndexOrder as a SortOrder
        SortOrder_t *q = (SortOrder_t *) iter.Next();
        assert(q);
        SortOrder_t *lcp = ord->LongestCommonPrefix(q);
        if (lcp) {
            if (!maxBuiltInOrder) {
                maxBuiltInOrder = lcp;
            }
            else {
                if (lcp->OrderSize() > maxBuiltInOrder->OrderSize()) {
                    maxBuiltInOrder = lcp;
                }
            }
        }
    }
    return maxBuiltInOrder;
}

// destructor
BaseRelLogProp_t::~BaseRelLogProp_t(void)
{
    ListIter_t<IndexOrder_t *> iter;
    iter.Attach(&builtInIndexList);

    while( !iter.IsEnd() ) {
        IndexOrder_t *q = iter.Next();
        DeRefer(q);
    }
}

/**
 * Convert a list of attributes to Schema. Utility method used
 * while building the outer variable schema.
 */
Schema_t *MakeSchema(List_t<Attr_t *> &attrList) 
{
    int n = attrList.Size();
    Schema_t *s = new Schema_t(n);
    ListIter_t<Attr_t *> tmpIter;
    tmpIter.Attach(&attrList);
       
    int attrIndex = 0;
    while(!tmpIter.IsEnd()) {
        Attr_t *outerVar = tmpIter.Next();
        assert(outerVar);
        s->SetAttribute(attrIndex++, outerVar);
    }
    return s;
}

/**
 * Make a LogProp_t from the schema. 
 */
LogProp_t *MakeLogProp(Schema_t *s, char *name)
{
    assert(s);
    SchemaProp_t *sp = new SchemaProp_t(s->NumAttr());
    // Set some cardinality. Should get updated from the top. 
    Card_t card = Config_t::CardLimit();
   
    // FIX: For now creating dummy attr props as this will 
    // be required for IsCovered/IsEquivalent methods of 
    // Order_t class.
    for (int i = 0; i < s->NumAttr(); i++) {
        AttrProp_t *attrProp = new IntegerProp_t(-32000, 32000, card ); 
        sp->SetAttributeProp(i, attrProp);
    }

    return new IntRelLogProp_t(name, s, sp, card);
}

