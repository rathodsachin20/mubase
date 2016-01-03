// logical properties

#ifndef __LOGPROP_H__
#define __LOGPROP_H__

extern "C" {
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
// #include <ieeefp.h>
}

#include "typedefs.h"
#include "List.h"
#include "RefCount.h"
#include "Config.h"
#include "Expr.h"

// physical props
class PhyProp_t;
class SortOrder_t;
class IndexOrder_t;

// attribute types
enum AttrType_t {     
    INTATTR_T,
    FLOATATTR_T,
    DOUBLEATTR_T,
    STRINGATTR_T,
    VARSTRINGATTR_T,
    VARBINARYATTR_T,
    DATEATTR_T,
    TIMEATTR_T,
    BINARYATTR_T 
};

// attribute information
class Attr_t : public RefC_t {
    char *relName;              // name of the base relation
    char *name;                 // name of the attribute
    DataSize_t size_Bytes;      // size of the attribute

public:
    Attr_t(char *relName_a, char *name_a, int size_Bytes_a)
        : relName(NULL), name(NULL), size_Bytes(size_Bytes_a)
    {
        assert(size_Bytes > 0);
        if( relName_a ) relName = strdup(relName_a);
        assert(name_a);
        name = strdup(name_a);
    }

    int IsEquivalent(const Attr_t *p)
    {
        assert(p);

        const char *pRelName = p->RelName();

        if( relName && pRelName )
            if( strcmp(relName, pRelName) != 0 )
                return 0;

        const char *pName = p->Name();
        assert(pName);
        assert(name);
        if( strcmp(name, pName) != 0 )
            return 0;

        return 1;
    }

    char *RelName(void) const { return relName; }

    char *Name(void) const { return name; }

    DataSize_t Size_Bytes(void) const { return size_Bytes; }

    virtual ~Attr_t(void)
    {
        if( relName ) free(relName);
        free(name);
    }
};

// attribute property
class AttrProp_t : public RefC_t {
    AttrType_t type;            // type of the attribute

protected:
    Card_t numDistinct;         // number of distinct values

public:
    AttrProp_t(AttrType_t type_a, Card_t numDistinct_a)
        : type(type_a), numDistinct(numDistinct_a)
    { assert(numDistinct > 0); assert(finite(numDistinct)); }

    AttrType_t Type(void) const { return type; }

    Card_t NumDistinct(void) const { return numDistinct; }

    void SetNumDistinct(Card_t n) { numDistinct = n; }

    virtual int IsEquivalent(const AttrProp_t *prop) const
    { return (type == prop->Type()) && (numDistinct == prop->NumDistinct()); }

    virtual int Subsumes(const AttrProp_t *prop) const = 0;

    // property manipulation functions
    virtual float Equate(AttrProp_t *p) = 0;
    virtual float UBound(AttrProp_t *p) = 0;
    virtual float LBound(AttrProp_t *p) = 0;

    virtual float FixValue(Value_t *val) = 0;
    virtual float UBound(Value_t *val) = 0;
    virtual float LBound(Value_t *val) = 0;

    virtual float EquateSelectivity(AttrProp_t *p) const = 0;
    virtual float UBoundSelectivity(AttrProp_t *p) const = 0;
    virtual float LBoundSelectivity(AttrProp_t *p) const = 0;

    virtual float FixValueSelectivity(Value_t *val) const = 0;
    virtual float UBoundSelectivity(Value_t *val) const = 0;
    virtual float LBoundSelectivity(Value_t *val) const = 0;

    // returns the union of the $this$ annd $prop$
    // that is, the strongest property that implies both
    virtual AttrProp_t *UnionProp(AttrProp_t *prop) = 0;

    virtual AttrProp_t *Copy(void) const = 0;

    virtual ~AttrProp_t(void)
    { }
};

// integer attribute property
class IntegerProp_t : public AttrProp_t {
    int maxval;                 // max value
    int minval;                 // min value

public:
    IntegerProp_t(int minval_a, int maxval_a, Card_t numDistinct_a)
        : AttrProp_t(INTATTR_T, numDistinct_a),
                    maxval(maxval_a), minval(minval_a)
    { assert(maxval >= minval); }

    int MaxVal(void) const { return maxval; }

    int MinVal(void) const { return minval; }

    int IsEquivalent(const AttrProp_t *prop) const
    {
        if( !AttrProp_t::IsEquivalent(prop) )
            return 0;
        const IntegerProp_t *iprop = (const IntegerProp_t *) prop;
        return (maxval == iprop->MaxVal()) && (minval == iprop->MinVal());
    }

    int Subsumes(const AttrProp_t *prop) const
    {
        if( !AttrProp_t::IsEquivalent(prop) )
            return 0;
        const IntegerProp_t *iprop = (const IntegerProp_t *) prop;
        return (maxval >= iprop->MaxVal()) && (minval <= iprop->MinVal());
    }

    // property manipulation functions
    float Equate(AttrProp_t *p);
    float UBound(AttrProp_t *p);
    float LBound(AttrProp_t *p);

    float EquateSelectivity(AttrProp_t *p) const;
    float UBoundSelectivity(AttrProp_t *p) const;
    float LBoundSelectivity(AttrProp_t *p) const;

    float FixValue(Value_t *val);
    float UBound(Value_t *val);
    float LBound(Value_t *val);

    float FixValueSelectivity(Value_t *val) const;
    float UBoundSelectivity(Value_t *val) const;
    float LBoundSelectivity(Value_t *val) const;

    AttrProp_t *UnionProp(AttrProp_t *prop);

    AttrProp_t *Copy(void) const
    { return new IntegerProp_t(minval, maxval, NumDistinct()); }

    ~IntegerProp_t(void)
    { }
};

// schema information
class Schema_t : public RefC_t {
    int numAttr;                // number of attributes
    Attr_t **attr;              // attribute array
    DataSize_t tupleSize_Bytes; // tuple length in bytes

public:
    Schema_t(int numAttr_a)
        : numAttr(numAttr_a), attr(NULL), tupleSize_Bytes(0)
    {
        assert(numAttr > 0);
        attr = new Attr_t *[numAttr];
        for( int i = 0 ; i < numAttr ; i++ )
            attr[i] = NULL;
    }

    // name of the relation
    // NULL for temporary relations
    virtual const char *Name(void) const
    { return NULL; }

    // number of attributes
    int NumAttr(void) const
    { return numAttr; }

    // get the $i$th attribute 
    Attr_t *Attribute(int i) const
    {
        assert(i >= 0 && i < numAttr);
        return attr[i];
    }

    // set the $i$th attribute
    void SetAttribute(int i, Attr_t *a)
    {
        assert(i >= 0 && i < numAttr);
        attr[i] = a;
        Refer(attr[i]);
        tupleSize_Bytes += attr[i]->Size_Bytes();
    }

    // returns the index (binding) in the schema for a given attr
    int AttrBinding(const Attr_t *a_attr)
    {
        for (int i = 0; i < numAttr; i++) {
            if (attr[i]->IsEquivalent(a_attr))
                return i;
        }
        return -1;
    }

    // size of the tuple in bytes
    DataSize_t TupleSize_Bytes(void) const
    { return tupleSize_Bytes; }

    // print out the sql
    void PrintSQL(SchemaProp_t *sp = NULL) const;

    ~Schema_t(void)
    {
        for( int i = 0 ; i < numAttr ; i++ )
            if( attr[i] != NULL ) DeRefer(attr[i]);
        delete[] attr;
    }
};

// properties of the attributes in the schema
// separated from Schema_t so that schema can be shared even if the
// properties differ
class SchemaProp_t : public RefC_t {
    int numAttr;                // number of attributes
    AttrProp_t **prop;          // property list
    int *rep;                   // table for union-find

public:
    SchemaProp_t(int numAttr_a) : numAttr(numAttr_a), prop(NULL)
    {
        assert(numAttr > 0);
        prop = new AttrProp_t *[numAttr];
        rep = new int [numAttr];
        for( int i = 0 ; i < numAttr ; i++ ) {
            prop[i] = NULL;
            rep[i] = -1;
        }
    }

    // get the representative
    int AttrPropRep(int bnd) const;
    
    // get the representative attribute set for the given attrset
    Set_t<int> *AttrPropRep(Set_t<int> &attrSet) const;
    
    void SetAttrPropRep(int bnd, int head)
    {
        // IMP: Do not check validity of bnd here
        // as transiently old bindings will be set 
        // when translating from one schema to another.
        rep[bnd] = head;
    }

    // number of attributes
    int NumAttr(void) const
    { return numAttr; }

    // get the $i$th attribute's property
    AttrProp_t *AttributeProp(int i) const
    {
        assert(i >= 0 && i < numAttr);
        int irep = AttrPropRep(i);
        return prop[irep];
    }

    // set the $i$th attribute's property
    void SetAttributeProp(int i, AttrProp_t *p)
    {
        assert(i >= 0 && i < numAttr);
        assert(rep[i] == -1 || rep[i] == i);

        Refer(p);
        if( prop[i] )
            DeRefer(prop[i]);
        prop[i] = p;

        // put in a separate partition
        if( rep[i] == -1 )
            rep[i] = i;
    }

    // attribute property manipulation functions
    float LBoundAttributeValue(int bnd, Value_t *val);
    float UBoundAttributeValue(int bnd, Value_t *val);
    float FixAttributeValue(int bnd, Value_t *val);
    float EquateAttributes(int leftbnd, int rightbnd);
    float BoundAttributes(int leftbnd, int rightbnd);

    float LBoundAttributeValueSelectivity(int bnd, Value_t *val) const;
    float UBoundAttributeValueSelectivity(int bnd, Value_t *val) const;
    float FixAttributeValueSelectivity(int bnd, Value_t *val) const;
    float EquateAttributesSelectivity(int leftbnd, int rightbnd) const;
    float BoundAttributesSelectivity(int leftbnd, int rightbnd) const;

    // Unify the attribute equivalence classes
    void UnifyAttrs(int attrBnd1, int attrBnd2) 
    {
        assert(attrBnd1 >= 0 && attrBnd1 < numAttr);
        assert(attrBnd2 >= 0 && attrBnd2 < numAttr);
        int head1 = rep[attrBnd1]; 
        int head2 = rep[attrBnd2];
        assert(head1 != -1 && head2 != -1);

        // Merge the classes making head1 the head of the new eqclass
        for (int i = 0; i < numAttr; i++) {
            if (rep[i] == head2)
                rep[i] = head1;
        }
    }

/*
    int AttrClassRep(int attrBnd)
    {
        assert(attrBnd >= 0 && attrBnd < numAttr);
        assert(rep[attrBnd] != -1);
        return rep[attrBnd];
    }
*/
    ~SchemaProp_t(void)
    {
        for( int i = 0 ; i < numAttr ; i++ )
            if( prop[i] != NULL ) DeRefer(prop[i]);
        delete[] prop;
        delete[] rep;
    }
};

// contains all the logical properties of the relation to be stored in
// the catalog
class LogProp_t : public RefC_t {
    char *name;                 // name of the relation
    Schema_t *schema;           // schema of the relation
    SchemaProp_t *schemaProp;   // properties of the attributes in the schema
    Card_t relSize_Tuples;      // size in number of tuples

public:
    LogProp_t(const char *name_a, Schema_t *schema_a,
            SchemaProp_t *schemaProp_a, Card_t relSize_Tuples_a)
        : name(NULL), schema(schema_a), schemaProp(schemaProp_a),
                                    relSize_Tuples(relSize_Tuples_a)
    {
        if( name_a ) name = strdup(name_a);
        Refer(schema);
        Refer(schemaProp);
        assert(relSize_Tuples >= 0);
        assert(finite(relSize_Tuples));
    }

    char *Name(void) const { return name; }

    Schema_t *Schema(void) const { return schema; }

    SchemaProp_t *SchemaProp(void) const { return schemaProp; }

    Card_t RelSize_Tuples(void) const { return relSize_Tuples; }

    virtual Card_t TuplesPerBlock(void) const = 0;

    Card_t RelSize_Blocks(void) const
    { return ceil(relSize_Tuples/TuplesPerBlock()); }

    virtual int IsBuiltIn(PhyProp_t *) const
    { return 0; }

    virtual ~LogProp_t(void)
    {
        DeRefer(schemaProp);
        DeRefer(schema);
        if( name ) free(name);
    }
};

// base relation logical property --- blocks are assumed to be
// arbitrarily packed
class BaseRelLogProp_t : public LogProp_t {
    Card_t tuplesPerBlock;              // average number of tuples per block
    PrependList_t<IndexOrder_t *> builtInIndexList;
                                // built-in index List

public:
    BaseRelLogProp_t(const char *name_a, Schema_t *schema_a,
        SchemaProp_t *schemaProp_a, Card_t relSize_tuples_a,
        Card_t tuplesPerBlock_a);

    Card_t TuplesPerBlock(void) const
    { return tuplesPerBlock; }

    void AddBuiltInIndex(IndexOrder_t *p);

    int IsBuiltIn(PhyProp_t *p) const;

    SortOrder_t *BuiltInProp(SortOrder_t *ord) const;

    // NQO: Added by Ravi
    const PrependList_t<IndexOrder_t *> *BuiltInIndexList(void) const
    { return &builtInIndexList; }

    ~BaseRelLogProp_t(void);
};

// intermediate relation logical property --- blocks are assumed to be
// fully packed
class IntRelLogProp_t : public LogProp_t {
public:
    IntRelLogProp_t(const char *name_a, Schema_t *schema_a,
        SchemaProp_t *schemaProp_a, Card_t relSize_tuples_a)
        : LogProp_t(name_a, schema_a, schemaProp_a, relSize_tuples_a)
    { }

    Card_t TuplesPerBlock(void) const
    {
        DataSize_t blockSize_Bytes = Config_t::BlockSize_Bytes();
        DataSize_t tupleSize_Bytes = Schema()->TupleSize_Bytes();

        return ceil(float(blockSize_Bytes)/tupleSize_Bytes);
    }

    ~IntRelLogProp_t(void)
    { }
};

// returns the merge of the two schemas
Schema_t *MergeSchema(Schema_t *left, Schema_t *right);

// returns the merge of the two schema properties
SchemaProp_t *MergeSchemaProp(SchemaProp_t *left, SchemaProp_t *right);

/**
 * Convert a list of attributes to Schema. Utility method used
 * while building the outer variable schema.
 */
Schema_t *MakeSchema(List_t<Attr_t *> &attrList);

LogProp_t *MakeLogProp(Schema_t *s, char *name=NULL);

#endif // __LOGPROP_H__
