// expression classes
// instead of duplicating objects, we use reference counting
// this works because the structure is acyclic

#ifndef __EXPR_H__
#define __EXPR_H__

#include <stdlib.h>
#include <string.h>
#include <iostream>

#include "RefCount.h"
#include "typedefs.h"
#include "List.h"
#include "Set.h"

using namespace std;

class LogProp_t;
class Schema_t;
class SchemaProp_t;
class Equivalence_t;
class PlanGroup_t;
class Attr_t;
class Order_t;

// expression type
enum ExprType_t {
    VALUE_T,
    PREDICATE_T,
    ARITH_EXPR_T,
    AGG_EXPR_T,
    REL_ARG_REF_T
};

// base class for all expressions
class Expr_t : public RefC_t {
    static int CurID;           // unique id generator for the expression node
    int id;                     // unique id for the expression node
    ExprType_t exprType;        // type of the expression node

public:
    Expr_t(const ExprType_t exprType_a) : id(CurID++), exprType(exprType_a)
    { }

    // type of the expression node
    ExprType_t ExprType(void) const 
    { return exprType; }

    // unique id of the node
    int ID(void) const
    { return id; }

    // is the expression identical to the given expression?
    int IsEqual(const Expr_t *e) const
    { return e->ID() == id; }

    // is the expression equivalent to the given expression
    virtual int IsEquivalent(const Expr_t *e) const;

    // print expression
    virtual void PrintExpr(int isFullExp = 0) const = 0;

    /** 
     * Print the expression.
     */
    virtual void PrintExprXML(int isFullExp = 0) const = 0;

    virtual void PrintSQL(void) const
    { PrintExpr(); }

    /**
     * Ravi: Return a copy of the expression 
     * Ravi: Is this a deep copy?
     */
    virtual Expr_t *Copy(void) const = 0;

    /**
     * Ravi: This method is unused.
     * Ravi:If this expression is a correlated one it finds the index (in 
     * Ravi:the schema) of the correlated attribute.
     * Ravi:level is the depth of the expression in the nested structure
     * Ravi:of expressions.  
     * Ravi:See InExpr_t::FindCorrelated() overriden method.
     * Ravi:Doubt:Is it assumed that only one correlated attr can be present
     * Ravi:in the nested subquery.
     */
    virtual int FindCorrelated(Schema_t *, int)
    { return -1; }

    /**
     * Ravi: This method is unused.
     * Marks the correlated nodes. Returns 1 of any subexpression 
     * is correlated else 0. 
     * See also: Equivalence_t::IsCorrelated(), Equivalence_t::MarkCorrelated()
     */
    virtual int MarkCorrelated(Schema_t *, int)
    { return 0; }


    /**
     * Populates the list "corrVarList" with the attributes referenced in the
     * expression and not having a binding in the schema given. These are
     * presumed to be correlated attributes.
     */
    virtual void CorrelatedAttrList(Schema_t *schema, AppendList_t<Attr_t *> *corrVarList)
    {
        assert(schema);
        assert(corrVarList);
        return; // Default implementation does not add 
                // anything to the output list.
    }

    // From the predicate find all the parameter orders of interest
    // considering the built-in index orders present in inpLP. The
    // generated orders will be w.r.t. to the outer var schama of 
    // the eqClass
    virtual void GetInterestingOrders(List_t<Order_t *> *opOrds, Equivalence_t *inpEq, 
                                      Equivalence_t *parentEq, int *plmap, int level);

    virtual ~Expr_t(void) { }
    
    // sachin - added to get name of projection attribute
                //dont know if there is other way to get it
    virtual const char *AttrName(void) const {return NULL;}
    //end added by sachin
};

/**
 * Value types
 */
enum ValType_t {
    INT_T,
    FLOAT_T,
    DOUBLE_T,
    STRING_T,
    VARCHAR_T,
    TRUTHVAL_T,
    NULL_T,
    STAR_T
};

/**
 * Base class for primary values
 */
class Value_t : public Expr_t {
    ValType_t valType;

public: 
    Value_t(const ValType_t valType_a) : Expr_t(VALUE_T), valType(valType_a)
    { }

    ValType_t ValType(void) const
    { return valType; }

    virtual int IsEquivalent(const Expr_t *e) const;

    virtual int IsEq(const Value_t *e) const = 0;
    virtual int IsLess(const Value_t *e) const = 0;

    // print expression
    virtual void PrintExpr(int isFullExp = 0) const = 0;
    virtual void PrintExprXML(int isFullExp = 0) const = 0;

    virtual Expr_t *Copy(void) const = 0;

    virtual ~Value_t(void)
    { }
};

/**
 * The null value
 */
class Null_t : public Value_t {
public: 
    Null_t(void) : Value_t(NULL_T)
    { }

    Expr_t *Copy(void) const
    { return new Null_t; }

    int IsEq(const Value_t *) const
    { return 0; }

    int IsLess(const Value_t *) const
    { return 0; }

    // print expression
    void PrintExpr(int = 0) const
    { cout << "NULL"; }
    void PrintExprXML(int = 0) const
    { cout << "<NULL/>" << endl; }

    ~Null_t(void)
    { }
};

// star value
class Star_t : public Value_t {
public: 
    Star_t(void) : Value_t(STAR_T)
    { }

    Expr_t *Copy(void) const
    { return new Star_t; }

    int IsEq(const Value_t *) const
    { return 0; }

    int IsLess(const Value_t *) const
    { return 0; }

    // print expression
    void PrintExpr(int = 0) const
    { cout << "*"; }

    void PrintExprXML(int = 0) const
    { cout << "<STAR/>" << endl; }

    ~Star_t(void)
    { }
};

// string value
class String_t : public Value_t {
    char *value;
    int len;

public: 
    String_t(const char *v, int l) : Value_t(STRING_T), value(NULL), len(l)
    {
        assert(l > 0);
        assert(v);
        value = new char[l];
        for( int i = 0 ; i < l ; i++ )
            value[i] = v[i];
    }

    const char *Value(void) const
    { return value; }

    int Length(void) const
    { return len; }

    int IsEquivalent(const Expr_t *e) const;

    Expr_t *Copy(void) const
    { return new String_t(value, len); }

    int IsEq(const Value_t *) const
    { return 0; }

    int IsLess(const Value_t *) const
    { return 0; }

    // print expression
    void PrintExpr(int = 0) const
    { cout << value; }

    void PrintExprXML(int = 0) const
    { cout << "<STRING value=\"" << value << "\"/>" << endl; }

    ~String_t(void)
    { delete[] value; }
};

// integer value
class Int_t : public Value_t {
    int value;

public: 
    Int_t(int v = 0) : Value_t(INT_T), value(v)
    { }

    int Value(void) const
    { return value; }

    int IsEquivalent(const Expr_t *e) const;

    Expr_t *Copy(void) const
    { return new Int_t(value); }

    int IsEq(const Value_t *e) const
    {
        assert(e->ValType() == INT_T);
        const Int_t *v = (const Int_t *) e;
        return value == v->Value();
    }

    int IsLess(const Value_t *e) const
    {
        assert(e->ValType() == INT_T);
        const Int_t *v = (const Int_t *) e;
        return value < v->Value();
    }

    // print expression
    void PrintExpr(int = 0) const
    { cout << value; }

    // print expression
    void PrintExprXML(int = 0) const
    { cout << "<INT value=\"" << value << "\"/>" << endl; }

    ~Int_t(void)
    { }
};

// float value
class Float_t : public Value_t {
    float value;

public: 
    Float_t(float v = 0) : Value_t(FLOAT_T), value(v)
    { }

    float Value(void) const
    { return value; }

    int IsEquivalent(const Expr_t *e) const;

    Expr_t *Copy(void) const
    { return new Float_t(value); }

    int IsEq(const Value_t *e) const
    {
        assert(e->ValType() == FLOAT_T);
        const Float_t *v = (const Float_t *) e;
        return value == v->Value();
    }

    int IsLess(const Value_t *e) const
    {
        assert(e->ValType() == FLOAT_T);
        const Float_t *v = (const Float_t *) e;
        return value < v->Value();
    }

    // print expression
    void PrintExpr(int = 0) const
    { cout << value; }

    void PrintExprXML(int = 0) const
    { cout << "<FLOAT value=\"" << value << "\"/>" << endl; }

    ~Float_t(void)
    { }
};

// double float value
class Double_t : public Value_t {
    double value;

public: 
    Double_t(double v = 0) : Value_t(DOUBLE_T), value(v)
    { }

    double Value(void) const
    { return value; }

    int IsEquivalent(const Expr_t *e) const;

    Expr_t *Copy(void) const
    { return new Double_t(value); }

    int IsEq(const Value_t *e) const
    {
        assert(e->ValType() == DOUBLE_T);
        const Double_t *v = (const Double_t *) e;
        return value == v->Value();
    }

    // print expression
    void PrintExpr(int = 0) const
    { cout << value; }

    void PrintExprXML(int = 0) const
    { cout << "<DOUBLE value=\"" << value << "\"/>" << endl; }

    int IsLess(const Value_t *e) const
    {
        assert(e->ValType() == DOUBLE_T);
        const Double_t *v = (const Double_t *) e;
        return value < v->Value();
    }

    ~Double_t(void)
    { }
};

// aggregate types: group-by not supported yet
enum AggOpType_t 
{
    MAX_T,
    MIN_T,
    SUM_T,
    AVG_T,
    COUNT_T, 
    COUNTDISTINCT_T,
    COUNTSTAR_T
};

// aggregate expression
class AggExpr_t : public Expr_t {
    AggOpType_t opType;
    Expr_t *input;

public:
    //Ravi:Fixed to have deep copy of the input expr
    AggExpr_t(AggOpType_t opType_a, Expr_t *inp_a)
        : Expr_t(AGG_EXPR_T), opType(opType_a)
    { input = inp_a->Copy(); Refer(input); }

    AggOpType_t AggOpType(void) const
    { return opType; }

    Expr_t *Input(void) const
    { assert(input); return input; }


    // Ravi:Following method newly added.
    // void SetInput(int i, Expr_t *inp_a)
    // { assert(i==0); DeRefer(input); input = inp_a; }

    int IsEquivalent(const Expr_t *e) const;

    virtual void PrintExpr(int = 0) const { }
    virtual void PrintExprXML(int = 0) const { }

    virtual Expr_t *Copy(void) const = 0;

    virtual ~AggExpr_t(void)
    { DeRefer(input); }
};

// max
class Max_t : public AggExpr_t {
public:
    Max_t(Expr_t *inp_a) : AggExpr_t(MAX_T, inp_a)
    { }

    Expr_t *Copy(void) const
    { return new Max_t(Input()); }

    // print expression
    void PrintExpr(int isFullExp = 0) const
    {
        cout << "MAX(";
        Input()->PrintExpr(isFullExp);
        cout << ")";
    }

    void PrintExprXML(int isFullExp = 0) const
    {
        cout << "<MAX>" << endl;
        Input()->PrintExprXML(isFullExp);
        cout << "</MAX>" << endl;
    }

    void PrintSQL(void) const
    {
        cout << "MAX(";
        Input()->PrintSQL();
        cout << ")";
    }

    // print expression
    ~Max_t(void)
    { }
};

// min
class Min_t : public AggExpr_t {
public:
    Min_t(Expr_t *inp_a) : AggExpr_t(MIN_T, inp_a)
    { }

    Expr_t *Copy(void) const
    { return new Min_t(Input()); }

    // print expression
    void PrintExpr(int isFullExp = 0) const
    {
        cout << "MIN(";
        Input()->PrintExpr(isFullExp);
        cout << ")";
    }

    void PrintExprXML(int isFullExp = 0) const
    {
        cout << "<MIN>" << endl;
        Input()->PrintExprXML(isFullExp);
        cout << "</MIN>" << endl;
    }

    void PrintSQL(void) const
    {
        cout << "MIN(";
        Input()->PrintSQL();
        cout << ")";
    }

    // print expression
    ~Min_t(void)
    { }
};

// sum
class Sum_t : public AggExpr_t {
public:
    Sum_t(Expr_t *inp_a) : AggExpr_t(SUM_T, inp_a)
    { assert(0); }

    Expr_t *Copy(void) const
    { return new Sum_t(Input()); }

    // print expression
    void PrintExpr(int isFullExp = 0) const
    {
        cout << "SUM(";
        Input()->PrintExpr(isFullExp);
        cout << ")";
    }

    void PrintExprXML(int isFullExp = 0) const
    {
        cout << "<SUM>" << endl;
        Input()->PrintExprXML(isFullExp);
        cout << "</SUM>" << endl;
    }

    void PrintSQL(void) const
    {
        cout << "SUM(";
        Input()->PrintSQL();
        cout << ")";
    }

    // print expression
    ~Sum_t(void)
    { }
};

// avg
class Avg_t : public AggExpr_t {
public:
    Avg_t(Expr_t *inp_a) : AggExpr_t(AVG_T, inp_a)
    { assert(0); }

    Expr_t *Copy(void) const
    { return new Avg_t(Input()); }

    // print expression
    void PrintExpr(int isFullExp = 0) const
    {
        cout << "AVG(";
        Input()->PrintExpr(isFullExp);
        cout << ")";
    }

    void PrintExprXML(int isFullExp = 0) const
    {
        cout << "<AVG>" << endl;
        Input()->PrintExprXML(isFullExp);
        cout << "</AVG>" << endl;
    }

    void PrintSQL(void) const
    {
        cout << "AVG(";
        Input()->PrintSQL();
        cout << ")";
    }

    ~Avg_t(void)
    { }
};

// count
class Count_t : public AggExpr_t {
public:
    Count_t(Expr_t *inp_a) : AggExpr_t(COUNT_T, inp_a)
    { assert(0); }

    Expr_t *Copy(void) const
    { return new Count_t(Input()); }

    // print expression
    void PrintExpr(int isFullExp = 0) const
    {
        cout << "COUNT(";
        Input()->PrintExpr(isFullExp);
        cout << ")";
    }

    void PrintExprXML(int isFullExp = 0) const
    {
        cout << "<COUNT>" << endl;
        Input()->PrintExprXML(isFullExp);
        cout << "</COUNT>" << endl;
    }

    void PrintSQL(void) const
    {
        cout << "COUNT(";
        Input()->PrintSQL();
        cout << ")";
    }

    ~Count_t(void)
    { }
};

// count distinct
class CountDistinct_t : public AggExpr_t {
public:
    CountDistinct_t(Expr_t *inp_a) : AggExpr_t(COUNTDISTINCT_T, inp_a)
    { assert(0); }

    Expr_t *Copy(void) const
    { return new CountDistinct_t(Input()); }

    // print expression
    void PrintExpr(int isFullExp = 0) const
    {
        cout << "COUNT DISTINCT(";
        Input()->PrintExpr(isFullExp);
        cout << ")";
    }

    void PrintExprXML(int isFullExp = 0) const
    {
        cout << "<COUNT_DISTINCT>" << endl;
        Input()->PrintExprXML(isFullExp);
        cout << "</COUNT_DISTINCT>" << endl;
    }

    void PrintSQL(void) const
    {
        cout << "COUNT DISTINCT(";
        Input()->PrintSQL();
        cout << ")";
    }

    ~CountDistinct_t(void)
    { }
};

// count star
class CountStar_t : public AggExpr_t {
public:
    CountStar_t(Expr_t *inp_a) : AggExpr_t(COUNTSTAR_T, inp_a)
    { assert(0); }

    Expr_t *Copy(void) const
    { return new CountStar_t(Input()); }

    // print expression
    void PrintExpr(int = 0) const
    { cout << "COUNT(*)"; }

    void PrintExprXML(int = 0) const
    { cout << "<COUNT_STAR/>" << endl; }

    ~CountStar_t(void)
    { }
};

/**
 * Ravi:Mod:Attribute reference
 * Ravi:Earlier comment: relation reference
 */
class RelArgRef_t : public Expr_t {
    char   *relName;
    char   *attrName;
public:
    RelArgRef_t(const char *rel_a, const char *attr_a)
        : Expr_t(REL_ARG_REF_T), relName(NULL), attrName(NULL)
    {
        assert(rel_a);
        relName = strdup(rel_a);
        assert(attr_a);
        attrName = strdup(attr_a);
    }

    const char *RelName(void) const
    { return relName; }

    const char *AttrName(void) const
    { return attrName; }

    int IsEquivalent(const Expr_t *e) const;

    // binding of relation attribute reference wrt the given schema
    // binding is returned as the index into the schema
    int Binding(Schema_t *schema);

    Expr_t *Copy(void) const
    { return new RelArgRef_t(relName, attrName); }

    // print expression
    void PrintExpr(int = 0) const
    { cout << relName << ":" << attrName; }

    void PrintExprXML(int = 0) const
    { 
        cout << "<RelArgRef relarg=\"" << relName << ":" 
             << attrName << "\"/>" << endl; 
    }

    void PrintSQL(void) const
    { cout << attrName; }

    // find the correlated attribute -- returns the binding in the schema
    int FindCorrelated(Schema_t *s, int level)
    {
        assert(s);
        assert(level >= 0);

        int bnd = Binding(s);
        if( level > 0 )
            return bnd;

        // on the same level of nesting of predicates, there should be
        // no unbound rel arg ref
        assert(bnd != -1);
        return -1;
    }

    // mark the node with correlated pred
    int MarkCorrelated(Schema_t *s, int level)
    { return FindCorrelated(s, level) != -1; }

    /**
     * Populates the list "corrVarList" with the attributes referenced in the
     * expression and not having a binding in the schema given. These are
     * presumed to be correlated attributes.
     */
    void CorrelatedAttrList(Schema_t *schema, AppendList_t<Attr_t *> *corrVarList);
    ~RelArgRef_t(void)
    {
        free(relName);
        free(attrName);
    }
};

// predicate relation operator types
enum PredOpType_t
{
    EQ_T,
    NEQ_T,
    LT_T,
    LEQ_T,
    OR_T,
    AND_T,
    NOT_T,
    TRUE_T,
    FALSE_T,
    IN_T,
    //Ravi:Added.
    NOTIN_T,            // NOT IN subquery
    EXISTS_T,           // EXISTS subquery
    NOTEXISTS_T,        // NOT EXISTS subquery
    LTALL_T,            // < ALL subquery
    GTALL_T,            // > ALL subquery
    LEQALL_T,           // <= ALL subquery
    GEQALL_T,           // >= ALL subquery
};

// predicate
class Predicate_t : public Expr_t {
    PredOpType_t opType;

public:
    Predicate_t(PredOpType_t opType_a)
        : Expr_t(PREDICATE_T), opType(opType_a)
    { }

    PredOpType_t PredOpType(void) const
    { return opType; }

    virtual int NumInputs(void) const = 0;

    virtual Expr_t *Input(int i) const = 0;

    // Ravi:Added SetInput method
    virtual void SetInput(int i, Expr_t *inp) = 0;
  
    /*
    //Ravi: Added new method for getting the input's lvalue 
    //virtual Expr_t **InputLValue(int i) const = 0;
    */

    virtual int Implies(const Predicate_t *p) const
    { assert(p); return IsEquivalent(p); }

    virtual int IsEquivalent(const Expr_t *e) const;

    virtual Expr_t *Copy(void) const = 0;

    // Ravi: Returns selectivity of the predicate
    virtual float InferProp(Schema_t *schema,
                        SchemaProp_t *schemaProp) const = 0;

    virtual float Selectivity(Schema_t *schema,
                        SchemaProp_t *schemaProp) const = 0;

    // decomposes the predicate to a cascade of two predicates ---
    // returns the predicate to be pushed down as childPred and
    // remainder as parPred
    // parSchema is used to detect correlated variables
    // if a var is not in the par then it is assumed to be a correlated
    // variable from above
    // parSchema is NULL => do not push correlated predicates
    virtual void Decompose(Schema_t *parSchema, Schema_t *childSchema,
                    Predicate_t **parPred, Predicate_t **childPred) = 0;

    // is the predicate valid over the schema?
    virtual int IsValidSchema(Schema_t *parSchema, Schema_t *schema) const = 0;

    // get the bindings that are part of the relevant join predicate
    // returns 0 if not possible; 1 if possible
    virtual int SortAttrBnd(Schema_t *outSchema,
                        Schema_t *leftSchema, Schema_t *rightSchema,
                        int *outBnd, int *leftBnd, int *rightBnd) const = 0;

    virtual void JoinAttrs(Set_t<int> &attrSet, Schema_t *outSchema,
                           Schema_t *leftSchema, Schema_t *rightSchema)
    {
        // Ravi: Overridden only in And_t and Eq_t. 
        return; 
    }

    // get the bindings that are part of the relevant join predicate
    // returns 0 if not possible; 1 if possible
    virtual int IndexAttrBnd(Schema_t *leftSchema, Schema_t *rightSchema,
                                    int *leftBnd, int *rightBnd) const = 0;

    // get the bindings that are part of the relevant select predicate
    // returns 0 if not possible; 1 if possible
    virtual int IndexAttrBnd(Schema_t *schema, int *bnd) const = 0;

    // print expression
    virtual void PrintExpr(int isFullExp = 0) const = 0;
    virtual void PrintExprXML(int isFullExp = 0) const = 0;

    // find the in operator
    virtual Predicate_t *FindIn(void)
    {
        Predicate_t *p = NULL;
        int n = NumInputs();

        for( int i = 0 ; i < n && !p ; i++ ) {
            Expr_t *inp = Input(i);
            assert(inp);
            assert(inp->ExprType() == PREDICATE_T);

            Predicate_t *pInp = (Predicate_t *) inp;

            p = pInp->FindIn();
        }

        return p;
    }

    // find the correlated attribute -- returns the binding in the schema
    // Ravi: Why assuming there will be only one correlated attribute in
    // Ravi: in an expression? 
    virtual int FindCorrelated(Schema_t *s, int level)
    {
        assert(s);
        assert(level >= 0);

        int n = NumInputs();
        int bnd = -1;

        for( int i = 0 ; i < n && bnd == -1 ; i++ ) {
            Expr_t *inp = Input(i);
            assert(inp);

            bnd = inp->FindCorrelated(s, level);
        }

        return bnd;
    }

    /**
     * Ravi:Correction to the comment.
     * Ravi:Mark the correlated expressions.
     * Ravi:Returns 1 if the expression is correlated else 0.
     */
    virtual int MarkCorrelated(Schema_t *s, int level)
    {
        assert(s);
        assert(level >= 0);

        int n = NumInputs();
        int isCorrelated = 0;

        for( int i = 0 ; i < n ; i++ ) {
            Expr_t *inp = Input(i);
            assert(inp);

            int isInpCorrelated = inp->MarkCorrelated(s, level);
            if( isInpCorrelated )
                isCorrelated = 1;
        }

        return isCorrelated;
    }


    /**
     * Populates the list "corrVarList" with the attributes referenced in the
     * expression and not having a binding in the schema given. These are
     * presumed to be correlated attributes.
     */
    virtual void CorrelatedAttrList(Schema_t *schema, 
                                    AppendList_t<Attr_t *> *corrVarList)
    {
        assert(schema);
        assert(corrVarList);

        int numInputs = NumInputs();
        for (int i = 0; i < numInputs; i++)
            Input(i)->CorrelatedAttrList(schema, corrVarList);
    }

    virtual void GetMatchingOrder(Order_t *inpOrder, Schema_t *inpSchema,
                 Order_t *outOrder, Schema_t *outSchema)
    { 
        int n = NumInputs();

        for( int i = 0 ; i < n; i++ ) {
            Expr_t *inp = Input(i);
            assert(inp);
            assert(inp->ExprType() == PREDICATE_T);

            Predicate_t *pInp = (Predicate_t *) inp;
            pInp->GetMatchingOrder(inpOrder, inpSchema,outOrder, outSchema);
        }
    }

    virtual ~Predicate_t(void)
    { }
};

// constant predicate
class ConstPredicate_t : public Predicate_t {
public:
    ConstPredicate_t(PredOpType_t t) : Predicate_t(t)
    { }

    int NumInputs(void) const
    { return 0; }

    Expr_t *Input(int) const
    { assert(0); return NULL; }
 
    //Ravi:Following method newly added.
    void SetInput(int i, Expr_t *inp)
    { assert(0); }

    /*
    //Ravi:Added 
    //Expr_t **InputLValue(int) const
    //{ assert(0); return NULL; }
    */

    Expr_t *Copy(void) const = 0;

    virtual float InferProp(Schema_t *schema,
                        SchemaProp_t *schemaProp) const = 0;

    virtual float Selectivity(Schema_t *schema,
                        SchemaProp_t *schemaProp) const = 0;

    // get the bindings that are part of the relevant join predicate
    // returns 0 if not possible; 1 if possible
    int SortAttrBnd(Schema_t *, Schema_t *, Schema_t *,
                                    int *, int *, int *) const
    { return 0; }

    // get the bindings that are part of the relevant join predicate
    // returns 0 if not possible; 1 if possible
    int IndexAttrBnd(Schema_t *, Schema_t *, int *, int *) const
    { return 0; }

    // get the bindings that are part of the relevant select predicate
    // returns 0 if not possible; 1 if possible
    int IndexAttrBnd(Schema_t *, int *) const
    { return 0; }


    // nothing to decompose
    void Decompose(Schema_t *parSchema, Schema_t *childSchema,
                    Predicate_t **parPred, Predicate_t **childPred);

    // is the predicate valid over the schema?
    int IsValidSchema(Schema_t *, Schema_t *) const
    { return 1; }

    // print expression
    virtual void PrintExpr(int isFullExp = 0) const = 0;
    virtual void PrintExprXML(int isFullExp = 0) const = 0;

    // find the in operator
    virtual Predicate_t *FindIn(void)
    { return NULL; }

    virtual ~ConstPredicate_t(void)
    { }
};

// true predicate
class True_t : public ConstPredicate_t {
public:
    True_t(void) : ConstPredicate_t(TRUE_T)
    { }

    int NumInputs(void) const
    { return 0; }

    int Implies(const Predicate_t *p) const
    { assert(p); return IsEquivalent(p); }

    Expr_t *Copy(void) const
    { return new True_t; }

    float InferProp(Schema_t *schema, SchemaProp_t *schemaProp) const
    { return Selectivity(schema, schemaProp); }

    float Selectivity(Schema_t *, SchemaProp_t *) const
    { return 1.0; }

    // print expression
    void PrintExpr(int = 0) const
    { cout << "TRUE"; }

    void PrintExprXML(int = 0) const
    { cout << "<TRUE/>" << endl; }

    virtual ~True_t(void)
    { }
};

// false predicate
class False_t : public ConstPredicate_t {
public:
    False_t(void) : ConstPredicate_t(FALSE_T)
    { }

    int NumInputs(void) const
    { return 0; }

    int Implies(const Predicate_t *) const
    { return 1; }

    Expr_t *Copy(void) const
    { return new False_t; }

    float InferProp(Schema_t *schema, SchemaProp_t *schemaProp) const
    { return Selectivity(schema, schemaProp); }

    float Selectivity(Schema_t *, SchemaProp_t *) const
    { return 0.0; }

    // print expression
    void PrintExpr(int = 0) const
    { cout << "FALSE"; }

    void PrintExprXML(int = 0) const
    { cout << "<FALSE/>" << endl; }

    virtual ~False_t(void)
    { }
};

// unary predicate
class UnaryPredicate_t : public Predicate_t {
    Expr_t *inp;

public:
    UnaryPredicate_t(PredOpType_t opType_a, Expr_t *inp_a)
        : Predicate_t(opType_a)
    { inp = inp_a->Copy(); Refer(inp); }

    int NumInputs(void) const
    { return 1; }

    Expr_t *Input(int i) const
    { assert(i == 0); return inp; }
   
    //Ravi:Following method newly added.
    void SetInput(int i, Expr_t *inp_a)
    { assert (i == 0); DeRefer(inp); inp = inp_a; Refer(inp); }

    /*
    //Ravi:Added 
    //Expr_t **InputLValue(int) const
    //{ assert(i == 0); return &inp; }
    */

    int IsEquivalent(const Expr_t *e) const;

    Expr_t *Copy(void) const = 0;

    virtual float InferProp(Schema_t *schema,
                        SchemaProp_t *schemaProp) const = 0;

    virtual float Selectivity(Schema_t *schema,
                        SchemaProp_t *schemaProp) const = 0;

    virtual void Decompose(Schema_t *parSchema, Schema_t *childSchema,
                    Predicate_t **parPred, Predicate_t **childPred) = 0;

    // is the predicate valid over the schema?
    virtual int IsValidSchema(Schema_t *parSchema, Schema_t *schema) const = 0;

    // print expression
    virtual void PrintExpr(int isFullExp = 0) const = 0;
    virtual void PrintExprXML(int isFullExp = 0) const = 0;

    virtual ~UnaryPredicate_t(void)
    { DeRefer(inp); }
};

// not predicate
class Not_t : public UnaryPredicate_t  {
public :
    Not_t(Expr_t *inp_a)
        : UnaryPredicate_t(NOT_T, inp_a)
    { }

    // implication testing
    int Implies(const Predicate_t *p) const;

    Expr_t *Copy(void) const
    { return new Not_t(Input(0)); }

    // not considered for now -- so return selectivity
    virtual float InferProp(Schema_t *schema, SchemaProp_t *schemaProp) const
    { return Selectivity(schema, schemaProp); }

    float Selectivity(Schema_t *schema, SchemaProp_t *schemaProp) const;

    // [not P] pushed down iff [P] pushed down
    void Decompose(Schema_t *parSchema, Schema_t *childSchema,
                    Predicate_t **parPred, Predicate_t **childPred);

    // get the bindings that are part of the relevant join predicate
    // returns 0 if not possible; 1 if possible
    int SortAttrBnd(Schema_t *, Schema_t *, Schema_t *,
                                    int *, int *, int *) const
    { return 0; }

    // get the bindings that are part of the relevant join predicate
    // returns 0 if not possible; 1 if possible
    int IndexAttrBnd(Schema_t *, Schema_t *, int *, int *) const
    { return 0; }

    // get the bindings that are part of the relevant select predicate
    // returns 0 if not possible; 1 if possible
    int IndexAttrBnd(Schema_t *, int *) const
    { return 0; }

    // is the predicate valid over the schema?
    int IsValidSchema(Schema_t *parSchema, Schema_t *schema) const;

    // print expression
    void PrintExpr(int isFullExp = 0) const
    {
        cout << "NOT[ ";
        Input(0)->PrintExpr(isFullExp);
        cout << " ]";
    }

    void PrintExprXML(int isFullExp = 0) const
    {
        cout << "<NOT>" << endl;
        Input(0)->PrintExprXML(isFullExp);
        cout << "</NOT>" << endl;
    }

    void PrintSQL(void) const
    {
        cout << "NOT( ";
        Input(0)->PrintSQL();
        cout << " )";
    }

    ~Not_t(void)
    { }
};

// binary predicate
class BinaryPredicate_t : public Predicate_t {
    Expr_t *inp[2];

public:
    BinaryPredicate_t(PredOpType_t opType_a, Expr_t *left_a, Expr_t *right_a)
        : Predicate_t(opType_a)
    {
        inp[0] = left_a -> Copy();
        Refer(inp[0]);

        inp[1] = right_a -> Copy();
        Refer(inp[1]);
    }

    int NumInputs(void) const
    { return 2; }

    Expr_t *Input(int i) const
    { assert(i == 0 || i == 1); return inp[i]; }
   
    //Ravi:Following method newly added.
    void SetInput(int i, Expr_t *inp_a)
    { assert(i == 0 || i == 1); DeRefer(inp[i]); inp[i] = inp_a; Refer(inp[i]);}


    /*
    //Ravi:Added 
    //Expr_t **InputLValue(int) const
    //{ assert(i == 0 || i == 1); return &inp[i]; }
    */

    virtual int IsEquivalent(const Expr_t *e) const = 0;

    virtual Expr_t *Copy(void) const = 0;

    virtual float InferProp(Schema_t *schema,
                        SchemaProp_t *schemaProp) const = 0;

    virtual float Selectivity(Schema_t *schema,
                        SchemaProp_t *schemaProp) const = 0;

    virtual void Decompose(Schema_t *parSchema, Schema_t *childSchema,
                    Predicate_t **parPred, Predicate_t **childPred) = 0;

    // is the predicate valid over the schema?
    virtual int IsValidSchema(Schema_t *parSchema, Schema_t *schema) const = 0;

    // print expression
    virtual void PrintExpr(int isFullExp = 0) const = 0;
    virtual void PrintExprXML(int isFullExp = 0) const = 0;

    virtual ~BinaryPredicate_t(void)
    { DeRefer(inp[0]); DeRefer(inp[1]); }
};

// binary symmetric predicate
class BinarySymmetricPredicate_t : public BinaryPredicate_t {
public :
    BinarySymmetricPredicate_t(PredOpType_t opType_a,
            Expr_t *left_a, Expr_t *right_a)
        : BinaryPredicate_t(opType_a, left_a, right_a)
    { }

    int IsEquivalent(const Expr_t *e) const;

    virtual Expr_t *Copy(void) const = 0;

    virtual float InferProp(Schema_t *schema,
                        SchemaProp_t *schemaProp) const = 0;

    virtual float Selectivity(Schema_t *schema,
                        SchemaProp_t *schemaProp) const = 0;

    virtual void Decompose(Schema_t *parSchema, Schema_t *childSchema,
                    Predicate_t **parPred, Predicate_t **childPred) = 0;

    // is the predicate valid over the schema?
    virtual int IsValidSchema(Schema_t *parSchema, Schema_t *schema) const = 0;

    // print expression
    virtual void PrintExpr(int isFullExp = 0) const = 0;
    virtual void PrintExprXML(int isFullExp = 0) const = 0;

    virtual ~BinarySymmetricPredicate_t(void)
    { }
};

// binary assymmetric predicate
class BinaryAssymmetricPredicate_t : public BinaryPredicate_t {
public :
    BinaryAssymmetricPredicate_t(PredOpType_t opType_a,
            Expr_t *left_a, Expr_t *right_a)
        : BinaryPredicate_t(opType_a, left_a, right_a)
    { }

    int IsEquivalent(const Expr_t *e) const;

    virtual Expr_t *Copy(void) const = 0;

    virtual float InferProp(Schema_t *schema,
                        SchemaProp_t *schemaProp) const = 0;

    virtual float Selectivity(Schema_t *schema,
                        SchemaProp_t *schemaProp) const = 0;

    virtual void Decompose(Schema_t *parSchema, Schema_t *childSchema,
                    Predicate_t **parPred, Predicate_t **childPred) = 0;

    // is the predicate valid over the schema?
    virtual int IsValidSchema(Schema_t *parSchema, Schema_t *schema) const = 0;

    // print expression
    virtual void PrintExpr(int isFullExp = 0) const = 0;
    virtual void PrintExprXML(int isFullExp = 0) const = 0;

    virtual ~BinaryAssymmetricPredicate_t(void)
    { }
};

// equality predicate
class Eq_t : public BinarySymmetricPredicate_t  {
public :
    Eq_t(Expr_t *left_a, Expr_t *right_a)
        : BinarySymmetricPredicate_t(EQ_T, left_a, right_a)
    { }

    Expr_t *Copy(void) const
    { return new Eq_t(Input(0), Input(1)); }

    float InferProp(Schema_t *schema, SchemaProp_t *schemaProp) const;

    float Selectivity(Schema_t *schema, SchemaProp_t *schemaProp) const;

    void Decompose(Schema_t *parSchema, Schema_t *childSchema,
                    Predicate_t **parPred, Predicate_t **childPred);

    // is the predicate valid over the schema?
    int IsValidSchema(Schema_t *parSchema, Schema_t *schema) const;

    // get the bindings that are part of the relevant join predicate
    // returns 0 if not possible; 1 if possible
    int SortAttrBnd(Schema_t *outSchema,
                        Schema_t *leftSchema, Schema_t *rightSchema,
                        int *outBnd, int *leftBnd, int *rightBnd) const;

    void JoinAttrs(Set_t<int> &attrSet, Schema_t *outSchema,
                   Schema_t *leftSchema, Schema_t *rightSchema);

    // get the bindings that are part of the relevant join predicate
    // returns 0 if not possible; 1 if possible
    int IndexAttrBnd(Schema_t *leftSchema, Schema_t *rightSchema,
                                    int *leftBnd, int *rightBnd) const;

    // get the bindings that are part of the relevant select predicate
    // returns 0 if not possible; 1 if possible
    int IndexAttrBnd(Schema_t *schema, int *bnd) const;

    // print expression
    void PrintExpr(int isFullExp = 0) const
    {
        Input(0)->PrintExpr(isFullExp);
        cout << " == ";
        Input(1)->PrintExpr(isFullExp);
    }

    void PrintExprXML(int isFullExp = 0) const
    {
        cout << "<EQEQ>" << endl;
        cout << "<LHS>" << endl;
        Input(0)->PrintExprXML(isFullExp);
        cout << "</LHS>" << endl;
        cout << "<RHS>" << endl;
        Input(1)->PrintExprXML(isFullExp);
        cout << "</RHS>" << endl;
        cout << "</EQEQ>" << endl;
    }

    void PrintSQL(void) const
    {
        cout << "(";
        Input(0)->PrintSQL();
        cout << " == ";
        Input(1)->PrintSQL();
        cout << ")";
    }

    // find the in operator
    Predicate_t *FindIn(void)
    { return NULL; }

    void GetMatchingOrder(Order_t *inpOrder, Schema_t *inpSchema,
                          Order_t *outOrder, Schema_t *outSchema);

    ~Eq_t(void)
    { }
};

// non-equality predicate
class NEq_t : public BinarySymmetricPredicate_t  {
public :
    NEq_t(Expr_t *left_a, Expr_t *right_a)
        : BinarySymmetricPredicate_t(NEQ_T, left_a, right_a)
    { }

    Expr_t *Copy(void) const
    { return new NEq_t(Input(0), Input(1)); }

    // not considered for now
    float InferProp(Schema_t *schema, SchemaProp_t *schemaProp) const
    { return Selectivity(schema, schemaProp); }

    float Selectivity(Schema_t *schema, SchemaProp_t *schemaProp) const;

    // get the bindings that are part of the relevant join predicate
    // returns 0 if not possible; 1 if possible
    int SortAttrBnd(Schema_t *, Schema_t *, Schema_t *,
                                    int *, int *, int *) const
    { return 0; }

    // get the bindings that are part of the relevant join predicate
    // returns 0 if not possible; 1 if possible
    int IndexAttrBnd(Schema_t *, Schema_t *, int *, int *) const
    { return 0; }

    // get the bindings that are part of the relevant select predicate
    // returns 0 if not possible; 1 if possible
    int IndexAttrBnd(Schema_t *, int *) const
    { return 0; }

    void Decompose(Schema_t *parSchema, Schema_t *childSchema,
                    Predicate_t **parPred, Predicate_t **childPred);

    // is the predicate valid over the schema?
    int IsValidSchema(Schema_t *parSchema, Schema_t *schema) const;

    // print expression
    void PrintExpr(int isFullExp = 0) const
    {
        Input(0)->PrintExpr(isFullExp);
        cout << " != ";
        Input(1)->PrintExpr(isFullExp);
    }

    void PrintExprXML(int isFullExp = 0) const
    {
        cout << "<NOTEQ>" << endl;
        cout << "<LHS>" << endl;
        Input(0)->PrintExprXML(isFullExp);
        cout << "</LHS>" << endl;
        cout << "<RHS>" << endl;
        Input(1)->PrintExprXML(isFullExp);
        cout << "</RHS>" << endl;
        cout << "</NOTEQ>" << endl;
    }

    void PrintSQL(void) const
    {
        cout << "(";
        Input(0)->PrintSQL();
        cout << " != ";
        Input(1)->PrintSQL();
        cout << ")";
    }

    // find the in operator
    Predicate_t *FindIn(void)
    { return NULL; }

    ~NEq_t(void)
    { }
};

// and predicate
class And_t : public BinarySymmetricPredicate_t  {
public :
    And_t(Expr_t *left_a, Expr_t *right_a)
        : BinarySymmetricPredicate_t(AND_T, left_a, right_a)
    { }

    // implication testing
    int Implies(const Predicate_t *p) const;

    Expr_t *Copy(void) const
    { return new And_t(Input(0), Input(1)); }

    float InferProp(Schema_t *schema, SchemaProp_t *schemaProp) const;

    float Selectivity(Schema_t *schema, SchemaProp_t *schemaProp) const;

    void Decompose(Schema_t *parSchema, Schema_t *childSchema,
                    Predicate_t **parPred, Predicate_t **childPred);

    // is the predicate valid over the schema?
    int IsValidSchema(Schema_t *parSchema, Schema_t *schema) const;

    // get the bindings that are part of the relevant join predicate
    // returns 0 if not possible; 1 if possible
    int SortAttrBnd(Schema_t *outSchema,
                        Schema_t *leftSchema, Schema_t *rightSchema,
                        int *outBnd, int *leftBnd, int *rightBnd) const;

    void JoinAttrs(Set_t<int> &attrSet, Schema_t *outSchema,
                   Schema_t *leftSchema, Schema_t *rightSchema);

    // get the bindings that are part of the relevant join predicate
    // returns 0 if not possible; 1 if possible
    int IndexAttrBnd(Schema_t *leftSchema, Schema_t *rightSchema,
                                    int *leftBnd, int *rightBnd) const;

    // get the bindings that are part of the relevant select predicate
    // returns 0 if not possible; 1 if possible
    int IndexAttrBnd(Schema_t *schema, int *bnd) const;

    // print expression
    void PrintExpr(int isFullExp = 0) const
    {
        cout << "AND[ ";
        Input(0)->PrintExpr(isFullExp);
        cout << " ";
        Input(1)->PrintExpr(isFullExp);
        cout << " ]";
    }

    void PrintExprXML(int isFullExp = 0) const
    {
        cout << "<AND>" << endl;
        cout << "<LHS>" << endl;
        Input(0)->PrintExprXML(isFullExp);
        cout << "</LHS>" << endl;
        cout << "<RHS>" << endl;
        Input(1)->PrintExprXML(isFullExp);
        cout << "</RHS>" << endl;
        cout << "</AND>" << endl;
    }

    void PrintSQL(void) const
    {
        cout << "(";
        Input(0)->PrintSQL();
        cout << " AND ";
        Input(1)->PrintSQL();
        cout << ")";
    }

    ~And_t(void)
    { }
};

// or predicate
class Or_t : public BinarySymmetricPredicate_t  {
public :
    Or_t(Expr_t *left_a, Expr_t *right_a)
        : BinarySymmetricPredicate_t(OR_T, left_a, right_a)
    { }

    // implication testing
    int Implies(const Predicate_t *p) const;

    Expr_t *Copy(void) const
    { return new Or_t(Input(0), Input(1)); }

    // not considered for now
    float InferProp(Schema_t *schema, SchemaProp_t *schemaProp) const
    {
        float sel = Selectivity(schema, schemaProp);
        assert(sel > 0);
        return sel;
    }

    float Selectivity(Schema_t *schema, SchemaProp_t *schemaProp) const;

    // get the bindings that are part of the relevant join predicate
    // returns 0 if not possible; 1 if possible
    // not possible for disjunctions
    int SortAttrBnd(Schema_t *, Schema_t *, Schema_t *,
                                    int *, int *, int *) const
    { return 0; }

    // get the bindings that are part of the relevant join predicate
    // returns 0 if not possible; 1 if possible
    // not possible for disjunctions
    int IndexAttrBnd(Schema_t *, Schema_t *, int *, int *) const
    { return 0; }

    // get the bindings that are part of the relevant select predicate
    // returns 0 if not possible; 1 if possible
    // not possible for disjunctions
    int IndexAttrBnd(Schema_t *, int *) const
    { return 0; }

    void Decompose(Schema_t *parSchema, Schema_t *childSchema,
                    Predicate_t **parPred, Predicate_t **childPred);

    // is the predicate valid over the schema?
    int IsValidSchema(Schema_t *parSchema, Schema_t *schema) const;

    // print expression
    void PrintExpr(int isFullExp = 0) const
    {
        cout << "OR[ ";
        Input(0)->PrintExpr(isFullExp);
        cout << " ";
        Input(1)->PrintExpr(isFullExp);
        cout << " ]";
    }

    void PrintExprXML(int isFullExp = 0) const
    {
        cout << "<OR>" << endl;
        cout << "<LHS>" << endl;
        Input(0)->PrintExprXML(isFullExp);
        cout << "</LHS>" << endl;
        cout << "<RHS>" << endl;
        Input(1)->PrintExprXML(isFullExp);
        cout << "</RHS>" << endl;
        cout << "</OR>" << endl;
    }

    void PrintSQL(void) const
    {
        cout << "(";
        Input(0)->PrintSQL();
        cout << " ";
        Input(1)->PrintSQL();
        cout << ")";
    }

    ~Or_t(void)
    { }
};

// less predicate
class Less_t : public BinaryAssymmetricPredicate_t  {
public :
    Less_t(Expr_t *left_a, Expr_t *right_a)
        : BinaryAssymmetricPredicate_t(LT_T, left_a, right_a)
    { }

    // implication testing
    int Implies(const Predicate_t *p) const;

    Expr_t *Copy(void) const
    { return new Less_t(Input(0), Input(1)); }

    float InferProp(Schema_t *schema, SchemaProp_t *schemaProp) const;

    float Selectivity(Schema_t *schema, SchemaProp_t *schemaProp) const;

    // get the bindings that are part of the relevant join predicate
    // returns 0 if not possible; 1 if possible
    int SortAttrBnd(Schema_t *, Schema_t *, Schema_t *,
                                    int *, int *, int *) const
    { return 0; }

    // get the bindings that are part of the relevant join predicate
    // returns 0 if not possible; 1 if possible
    int IndexAttrBnd(Schema_t *, Schema_t *, int *, int *) const
    { return 0; }

    // get the bindings that are part of the relevant select predicate
    // returns 0 if not possible; 1 if possible
    int IndexAttrBnd(Schema_t *schema, int *bnd) const;

    void Decompose(Schema_t *parSchema, Schema_t *childSchema,
                    Predicate_t **parPred, Predicate_t **childPred);

    // is the predicate valid over the schema?
    int IsValidSchema(Schema_t *parSchema, Schema_t *schema) const;

    // print expression
    void PrintExpr(int isFullExp = 0) const
    {
        Input(0)->PrintExpr(isFullExp);
        cout << " < ";
        Input(1)->PrintExpr(isFullExp);
    }

    void PrintExprXML(int isFullExp = 0) const
    {
        cout << "<LT>" << endl;
        cout << "<LHS>" << endl;
        Input(0)->PrintExprXML(isFullExp);
        cout << "</LHS>" << endl;
        cout << "<RHS>" << endl;
        Input(1)->PrintExprXML(isFullExp);
        cout << "</RHS>" << endl;
        cout << "</LT>" << endl;

    }

    void PrintSQL(void) const
    {
        cout << "(";
        Input(0)->PrintSQL();
        cout << " < ";
        Input(1)->PrintSQL();
        cout << ")";
    }

    // find the in operator
    Predicate_t *FindIn(void)
    { return NULL; }

    ~Less_t(void)
    { }
};

// less-Eq predicate
class LessEq_t : public BinaryAssymmetricPredicate_t  {
public :
    LessEq_t(Expr_t *left_a, Expr_t *right_a)
        : BinaryAssymmetricPredicate_t(LEQ_T, left_a, right_a)
    { }

    Expr_t *Copy(void) const
    { return new LessEq_t(Input(0), Input(1)); }

    float InferProp(Schema_t *schema, SchemaProp_t *schemaProp) const;

    float Selectivity(Schema_t *schema, SchemaProp_t *schemaProp) const;

    // get the bindings that are part of the relevant join predicate
    // returns 0 if not possible; 1 if possible
    int SortAttrBnd(Schema_t *, Schema_t *, Schema_t *,
                                    int *, int *, int *) const
    { return 0; }

    // get the bindings that are part of the relevant join predicate
    // returns 0 if not possible; 1 if possible
    int IndexAttrBnd(Schema_t *, Schema_t *, int *, int *) const
    { return 0; }

    // get the bindings that are part of the relevant select predicate
    // returns 0 if not possible; 1 if possible
    int IndexAttrBnd(Schema_t *, int *) const
    { return 0; }

    void Decompose(Schema_t *parSchema, Schema_t *childSchema,
                    Predicate_t **parPred, Predicate_t **childPred);

    // is the predicate valid over the schema?
    int IsValidSchema(Schema_t *parSchema, Schema_t *schema) const;

    // print expression
    void PrintExpr(int isFullExp = 0) const
    {
        Input(0)->PrintExpr(isFullExp);
        cout << " <= ";
        Input(1)->PrintExpr(isFullExp);
    }

    void PrintExprXML(int isFullExp = 0) const
    {
        cout << "<LE>" << endl;
        cout << "<LHS>" << endl;
        Input(0)->PrintExprXML(isFullExp);
        cout << "</LHS>" << endl;
        cout << "<RHS>" << endl;
        Input(1)->PrintExprXML(isFullExp);
        cout << "</RHS>" << endl;
        cout << "</LE>" << endl;
    }

    void PrintSQL(void) const
    {
        cout << "(";
        Input(0)->PrintSQL();
        cout << " <= ";
        Input(1)->PrintSQL();
        cout << ")";
    }

    // find the in operator
    Predicate_t *FindIn(void)
    { return NULL; }

    ~LessEq_t(void)
    { }
};

// arithmetic expression type
enum ArithOpType_t {
    PLUS_T,
    MINUS_T,
    MULT_T,
    DIV_T
};

// arithmetic expression
class ArithExpr_t : public Expr_t {
    ArithOpType_t opType;
    Expr_t *inp[2];

public:
    ArithExpr_t(ArithOpType_t opType_a, Expr_t *left_a, Expr_t *right_a)
        : Expr_t(ARITH_EXPR_T), opType(opType_a)
    {
        inp[0] = left_a;
        Refer(inp[0]);

        inp[1] = right_a;
        Refer(inp[1]);
    }

    ArithOpType_t ArithOpType(void) const
    { return opType; }

    Expr_t *Input(int i) const
    { assert(i == 0 || i == 1); return inp[i]; }
   
    /*
    //Ravi:Added 
    //Expr_t **InputLValue(int) const
    //{ assert(i == 0 || i == 1); return &inp[i]; }
    */

    int IsEquivalent(const Expr_t *e) const;

    // print expression
    virtual void PrintExpr(int isFullExp = 0) const = 0;
    virtual void PrintExprXML(int isFullExp = 0) const = 0;

    virtual Expr_t *Copy(void) const = 0;

    virtual ~ArithExpr_t(void)
    { DeRefer(inp[0]); DeRefer(inp[1]); }
};

// commutative operator expression
class CommutativeArithExpr_t : public ArithExpr_t {
public:
    CommutativeArithExpr_t(ArithOpType_t opType_a,
                            Expr_t *left_a, Expr_t *right_a)
        : ArithExpr_t(opType_a, left_a, right_a)
    { }

    int IsEquivalent(const Expr_t *e) const;

    // print expression
    virtual void PrintExpr(int isFullExp = 0) const = 0;
    virtual void PrintExprXML(int isFullExp = 0) const = 0;

    virtual ~CommutativeArithExpr_t(void)
    { }
};

// non-commutative operator expression
class NonCommutativeArithExpr_t : public ArithExpr_t {
public:
    NonCommutativeArithExpr_t(ArithOpType_t opType_a,
                            Expr_t *left_a, Expr_t *right_a)
        : ArithExpr_t(opType_a, left_a, right_a)
    { }

    int IsEquivalent(const Expr_t *e) const;

    // print expression
    virtual void PrintExpr(int isFullExp = 0) const = 0;
    virtual void PrintExprXML(int isFullExp = 0) const = 0;

    virtual ~NonCommutativeArithExpr_t(void)
    { }
};

// plus
class Plus_t : public CommutativeArithExpr_t {
public:
    Plus_t(Expr_t *left, Expr_t *right)
        : CommutativeArithExpr_t(PLUS_T, left, right)
    { }

    Expr_t *Copy(void) const
    { return new Plus_t(Input(0), Input(1)); }

    // print expression
    void PrintExpr(int isFullExp = 0) const
    {
        cout << "( ";
        Input(0)->PrintExpr(isFullExp);
        cout << " + ";
        Input(1)->PrintExpr(isFullExp);
        cout << " )";
    }

    void PrintExprXML(int isFullExp = 0) const
    {
        cout << "<PLUS>" << endl;
        cout << "<LHS>" << endl;
        Input(0)->PrintExprXML(isFullExp);
        cout << "</LHS>" << endl;
        cout << "<RHS>" << endl;
        Input(1)->PrintExprXML(isFullExp);
        cout << "</RHS>" << endl;
        cout << "</PLUS>" << endl;
    }

    ~Plus_t(void)
    { }
};

// minus
class Minus_t : public NonCommutativeArithExpr_t {
public:
    Minus_t(Expr_t *left, Expr_t *right)
        : NonCommutativeArithExpr_t(MINUS_T, left, right)
    { }

    Expr_t *Copy(void) const
    { return new Minus_t(Input(0), Input(1)); }

    // print expression
    void PrintExpr(int isFullExp = 0) const
    {
        cout << "( ";
        Input(0)->PrintExpr(isFullExp);
        cout << " - ";
        Input(1)->PrintExpr(isFullExp);
        cout << " )";
    }

    void PrintExprXML(int isFullExp = 0) const
    {
        cout << "<MINUS>" << endl;
        cout << "<LHS>" << endl;
        Input(0)->PrintExprXML(isFullExp);
        cout << "</LHS>" << endl;
        cout << "<RHS>" << endl;
        Input(1)->PrintExprXML(isFullExp);
        cout << "</RHS>" << endl;
        cout << "</MINUS>" << endl;
    }

    ~Minus_t(void)
    { }
};

// mult
class Mult_t : public CommutativeArithExpr_t {
public:
    Mult_t(Expr_t *left, Expr_t *right)
        : CommutativeArithExpr_t(MULT_T, left, right)
    { }

    Expr_t *Copy(void) const
    { return new Mult_t(Input(0), Input(1)); }

    // print expression
    void PrintExpr(int isFullExp = 0) const
    {
        cout << "( ";
        Input(0)->PrintExpr(isFullExp);
        cout << " * ";
        Input(1)->PrintExpr(isFullExp);
        cout << " )";
    }

    void PrintExprXML(int isFullExp = 0) const
    {
        cout << "<MULT>" << endl;
        cout << "<LHS>" << endl;
        Input(0)->PrintExprXML(isFullExp);
        cout << "</LHS>" << endl;
        cout << "<RHS>" << endl;
        Input(1)->PrintExprXML(isFullExp);
        cout << "</RHS>" << endl;
        cout << "</MULT>" << endl;
    }

    ~Mult_t(void)
    { }
};

// div
class Div_t : public NonCommutativeArithExpr_t {
public:
    Div_t(Expr_t *left, Expr_t *right)
        : NonCommutativeArithExpr_t(DIV_T, left, right)
    { }

    Expr_t *Copy(void) const
    { return new Div_t(Input(0), Input(1)); }

    // print expression
    void PrintExpr(int isFullExp = 0) const
    {
        cout << "( ";
        Input(0)->PrintExpr(isFullExp);
        cout << " / ";
        Input(1)->PrintExpr(isFullExp);
        cout << " )";
    }

    void PrintExprXML(int isFullExp = 0) const
    {
        cout << "<DIV>" << endl;
        cout << "<LHS>" << endl;
        Input(0)->PrintExprXML(isFullExp);
        cout << "</LHS>" << endl;
        cout << "<RHS>" << endl;
        Input(1)->PrintExprXML(isFullExp);
        cout << "</RHS>" << endl;
        cout << "</DIV>" << endl;
    }

    ~Div_t(void)
    { }
};

// supplies shared copies of constant predicates
class ConstPreds_t {
    True_t *truePred;
    False_t *falsePred;

public:
    ConstPreds_t(void) : truePred(NULL), falsePred(NULL)
    {
        truePred = new True_t;
        Refer(truePred);
        falsePred = new False_t;
        Refer(falsePred);
    }

    Predicate_t *True(void) const { return truePred; }

    Predicate_t *False(void) const { return falsePred; }

    ~ConstPreds_t(void)
    { DeRefer(truePred); DeRefer(falsePred); }
};

extern ConstPreds_t ConstPreds;

/**
 * IN expression (nested query).
 */
class InExpr_t : public Predicate_t  {
    /*
     * Ravi:LHS of the IN expression. (Looks like arbitrary expressions are
     * Ravi:not yet supported on the LHS. Only a column reference is allowed)
     * Ravi:The RelArgRef holds a relation name and an attribute (column) name.
     */
    RelArgRef_t  *inpRelArgRef;

    /* 
     * Ravi:Equivalence node for the RHS of the IN expression (a subquery).
     * Ravi:Related Dali classes are InExpr and GeneralProjectExpr. 
     */
    Equivalence_t *inpEq;

    /**
     * Ravi:PlanGroup for the above (RHS) equivalence node.
     */
    PlanGroup_t *inpPlanGroup;

    /** 
     * Ravi:Number of distinct values in the LHS. Currently taken from 
     * Ravi:the catalog directly. See InExpr_t::CalculateNumDistinct()
     * Ravi:and the SubQueryCost function in LogicalOp.c
     */
    Card_t numDistinct;

    /**
     * Called by the FindCorrelated() method. See the contract of 
     * FindCorrelated() method in Expr_t.  
     * Ravi:Finds the index (in the schema) of the correlated attribute
     * Ravi:in the subquery represented by the equivalence node eq. level
     * Ravi:is the depth of nesting (subquery in an InExpr can in turn have
     * Ravi:another InExpr and a subquery). This method is initially called
     * Ravi:with level as 0. recursively calls itslef eavery time incrementing
     * Ravi:the level by 1.
     * Ravi:See Expr_t::FindCorrelated()
     * Ravi:Doubt:Is it assumed that only one correlated attr can be present
     * Ravi:in the nested subquery.
     */
    int FindCorrelatedTrav(Equivalence_t *eq, Schema_t *s, int level);

    /**
     * Ravi: This method is unused.
     * Called by the MarkCorrelated() method. See the contract of
     * MarkCorrelated() method in Expr_t.
     */
    int MarkCorrelatedTrav(Equivalence_t *eq, Schema_t *s, int level);

    /**
     * Calculates the number of distinct values for the LHS of the IN predicate.
     * The catalog is used for this.
     */
    Card_t CalculateNumDistinct(LogProp_t *outLP);

    /**
     * Calculates the selectivity. Currently returning 1.
     */
    float CalculateSelectivity(Schema_t *s , SchemaProp_t *sp) const;

public:
    InExpr_t(RelArgRef_t *inpRelArgRef_a, Equivalence_t *inpEq_a)
        : Predicate_t(IN_T),
                inpRelArgRef(inpRelArgRef_a), inpEq(inpEq_a),
                inpPlanGroup(NULL), numDistinct(-1.0)
    { Refer(inpRelArgRef); assert(inpEq); }

    int NumInputs() const { return 1; }

    Expr_t *Input(int i) const
    { assert(i == 0); return inpRelArgRef; }
   
    //Ravi:Following method newly added.
    void SetInput(int i, Expr_t *inp)
    { assert(i == 0); DeRefer(inpRelArgRef); 
      inpRelArgRef = (RelArgRef_t *) inp; 
      Refer(inpRelArgRef); 
    }

    /*
    //Ravi:Added 
    //Expr_t **InputLValue(int) const
    //{ assert(i == 0); return &inpRelArgRef; }
    */

    RelArgRef_t *InpRelArgRef(void) const
    { return inpRelArgRef; }

    Equivalence_t *InpEq(void) const
    { return inpEq; }

    PlanGroup_t *InpPlanGroup(void) const
    { return inpPlanGroup; }

    void SetInpPlanGroup(PlanGroup_t *pg)
    { assert(pg); inpPlanGroup = pg; }

    int IsEquivalent(const Expr_t* p) const;

    //Ravi:This does not implement deep copy properly.
    //Ravi:The inpEq will be shared between the new 
    //Ravi:instance and this one.
    Expr_t *Copy(void) const
    { 
        InExpr_t *newInstance = new InExpr_t(inpRelArgRef, inpEq);
        //newInstance->SetInpPlanGroup(this->InpPlanGroup());
        return newInstance;
    }

    float InferProp(Schema_t *s , SchemaProp_t * sp )  const
    { return Selectivity(s,sp); }

    float Selectivity(Schema_t *s , SchemaProp_t *sp) const
    { return CalculateSelectivity(s, sp); }

    // Decompose the predicate into parent and child predicates
    // such that all attributes referenced in the parent predicate
    // have bindings in the parSchema and all attributes referenced
    // in the childPred have bindings in the child schema.
    void Decompose(Schema_t *parSchema, Schema_t *childSchema,
                        Predicate_t **parPred, Predicate_t **childPred);        

    int IndexAttrBnd(Schema_t *, int *)  const
    { return (0); }

    Card_t NumDistinct(LogProp_t *outLP)
    {
        if( numDistinct < 0 )
            numDistinct = CalculateNumDistinct(outLP);
        return numDistinct;
    }

    int IsValidSchema(Schema_t *parSchema, Schema_t *schema) const;

    int SortAttrBnd(Schema_t *, Schema_t *, Schema_t *,
                                        int *, int *, int *) const
    { return (0); }

    int IndexAttrBnd(Schema_t *, Schema_t *, int *, int *)  const
    { return (0); }

    /* 
     * Find the IN operator
     */
    Predicate_t *FindIn(void)
    { return this; }

    void PrintExpr(int isFullExp = 0) const;
    void PrintExprXML(int isFullExp = 0) const;

    int FindCorrelated(Schema_t *s, int level)
    { return FindCorrelatedTrav(inpEq, s, level); }

    int MarkCorrelated(Schema_t *s, int level)
    { return MarkCorrelatedTrav(inpEq, s, level); }


    void CorrelatedAttrList(Schema_t *schema, 
                            AppendList_t<Attr_t *> *corrVarList);
    ~InExpr_t(void)
    { DeRefer(inpRelArgRef); }
};

#endif // __EXPR_H__
