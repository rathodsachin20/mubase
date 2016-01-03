// expression class methods

#include <iostream>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "List.h"
#include "LogicalOp.h"
#include "LogProp.h"
#include "Plan.h"
#include "Expr.h"

int Expr_t::CurID = 0;

ConstPreds_t ConstPreds;

// handles only values and argrefs; TODO: handle full expressions
// is conservative on expressions
// parSchema is NULL => do not push correlated predicates
//
// Returns true if expr has valid bindings in childSchema OR
// expr has no bindings in childSchema as well as parSchema (presumed
// to be correlated). Returns false if no bindings in the childExpr
// bindings in the parentSchema. If parSchema is NULL decision is
// only based on the childSchema - if binding found in childSchema
// return true else return false.
int IsValidExpr(Schema_t *parSchema, Schema_t *childSchema, Expr_t *expr)
{
    assert(expr);
    ExprType_t exprType = expr->ExprType();

    if( exprType == VALUE_T )
        return 1;
    else if( exprType == REL_ARG_REF_T ) {
        RelArgRef_t *argRef = (RelArgRef_t *) expr;

        // if invalid in child and parent, must be a correlation var
        if( argRef->Binding(childSchema) != -1 )
            return 1;
        else if( parSchema ) {
            if( argRef->Binding(parSchema) == -1 )
                return 1;
        }
    }

    return 0;
}

// equivalence check for expressions
int Expr_t::IsEquivalent(const Expr_t *e) const
{
    if( IsEqual(e) )
        return 1;
    return (exprType == e->ExprType()) ? 1 : 0;
}

// equivalence check for values
int Value_t::IsEquivalent(const Expr_t *e) const
{
    // same object
    if( IsEqual(e) )
        return 1;

    if( e == NULL )
        return 0;

    if( !Expr_t::IsEquivalent(e) )
        return 0;

    // must be a VALUE_T -- so cast safe
    const Value_t *v = (const Value_t *) e;
    if( valType != v->ValType() )
        return 0;

    return 1;
}

// equivalence check for strings
int String_t::IsEquivalent(const Expr_t *e) const
{
    // same object
    if( IsEqual(e) )
        return 1;

    if( !Value_t::IsEquivalent(e) )
        return 0;
    
    // must be a STRING_T -- so cast safe
    const String_t *s = (const String_t *) e;
    if( len != s->Length() )
        return 0;

    const char *v = s->Value();
    for( int i = 0 ; i < len ; i++ )
        if( value[i] != v[i] )
            return 0;

    return 1;
}

// equivalence check for integers
int Int_t::IsEquivalent(const Expr_t *e) const
{
    // same object
    if( IsEqual(e) )
        return 1;

    if( !Value_t::IsEquivalent(e) )
        return 0;
    
    // must be a INT_T -- so cast safe
    const Int_t *i = (const Int_t *) e;
    if( i->Value() != value )
        return 0;

    return 1;
}

// equivalence check for floats
int Float_t::IsEquivalent(const Expr_t *e) const
{
    // same object
    if( IsEqual(e) )
        return 1;

    if( !Value_t::IsEquivalent(e) )
        return 0;
    
    // must be a FLOAT_T -- so cast safe
    const Float_t *f = (const Float_t *) e;
    if( f->Value() != value )
        return 0;

    return 1;
}

// equivalence check for double
int Double_t::IsEquivalent(const Expr_t *e) const
{
    // same object
    if( IsEqual(e) )
        return 1;

    if( !Value_t::IsEquivalent(e) )
        return 0;
    
    // must be a DOUBLE_T -- so cast safe
    const Double_t *d = (const Double_t *) e;
    if( d->Value() != value )
        return 0;

    return 1;
}

// equivalence check for aggregate exprs
int AggExpr_t::IsEquivalent(const Expr_t *e) const
{
    // same object
    if( IsEqual(e) )
        return 1;

    if( !Expr_t::IsEquivalent(e) )
        return 0;
    
    // must be a AGG_EXPR_T -- so cast safe
    const AggExpr_t *a = (const AggExpr_t *) e;
    if( a->AggOpType() != opType )
        return 0;

    Expr_t *i = a->Input();
    if( !input->IsEquivalent(i) )
        return 0;

    return 1;
}

// equivalence check for relation references
int RelArgRef_t::IsEquivalent(const Expr_t *e) const
{
    // same object
    if( IsEqual(e) )
        return 1;

    if( !Expr_t::IsEquivalent(e) )
        return 0;
    
    // must be a REL_ARG_REF_T -- so cast safe
    const RelArgRef_t *r = (const RelArgRef_t *) e;
    char const *rname = r->RelName();
    assert(rname);
    assert(relName);
    if( strcmp(relName, rname) != 0 )
        return 0;

    char const *aname = r->AttrName();
    assert(aname);
    assert(attrName);
    if( strcmp(attrName, aname) != 0 )
        return 0;

    return 1;
}

// equivalence check for predicates
int Predicate_t::IsEquivalent(const Expr_t *e) const
{
    assert(e);
    if( IsEqual(e) )
        return 1;

    if( !Expr_t::IsEquivalent(e) )
        return 0;

    // must be a PREDICATE_T -- so cast safe
    const Predicate_t *p = (const Predicate_t *) e;
    if( opType != p->PredOpType() )
        return 0;

    return 1;
}

// equivalence check for unary predicates
int UnaryPredicate_t::IsEquivalent(const Expr_t *e) const
{
    assert(e);
    if( IsEqual(e) )
        return 1;

    if( !Predicate_t::IsEquivalent(e) )
        return 0;

    // must be a unary predicate -- so cast safe
    const UnaryPredicate_t *u = (const UnaryPredicate_t *) e;

    Expr_t *uInp = u->Input(0);
    return inp->IsEquivalent(uInp);
}

// equivalence check for binary symmetric predicates
int BinarySymmetricPredicate_t::IsEquivalent(const Expr_t *e) const
{
    assert(e);
    if( IsEqual(e) )
        return 1;

    if( !Predicate_t::IsEquivalent(e) )
        return 0;

    // must be a unary predicate -- so cast safe
    const BinarySymmetricPredicate_t *b =
                                (const BinarySymmetricPredicate_t *) e;

    Expr_t *bInpL = b->Input(0);
    Expr_t *bInpR = b->Input(1);
    Expr_t *inpL = Input(0);
    Expr_t *inpR = Input(1);
    if( (inpL->IsEquivalent(bInpL) && inpR->IsEquivalent(bInpR))  ||
                (inpL->IsEquivalent(bInpR) && inpR->IsEquivalent(bInpL)) )
        return 1;
    return 0;
}

// equivalence check for binary assymmetric predicates
int BinaryAssymmetricPredicate_t::IsEquivalent(const Expr_t *e) const
{
    assert(e);
    if( IsEqual(e) )
        return 1;

    if( !Predicate_t::IsEquivalent(e) )
        return 0;

    // must be a unary predicate -- so cast safe
    const BinarySymmetricPredicate_t *b =
                        (const BinarySymmetricPredicate_t *) e;

    Expr_t *bInpL = b->Input(0);
    Expr_t *bInpR = b->Input(1);
    Expr_t *inpL = Input(0);
    Expr_t *inpR = Input(1);
    if( (inpL->IsEquivalent(bInpL) && inpR->IsEquivalent(bInpR)) )
        return 1;
    return 0;
}

// equivalence check for arithmetic expressions
int ArithExpr_t::IsEquivalent(const Expr_t *e) const
{
    assert(e);
    // same object
    if( IsEqual(e) )
        return 1;

    if( !Expr_t::IsEquivalent(e) )
        return 0;

    // must be a ARITH_EXPR -- so cast safe
    const ArithExpr_t *ae = (const ArithExpr_t *) e;
    if( opType != ae->ArithOpType() )
        return 0;

    return 1;
}

// equivalence check for commutative arithmetic operator expressions
int CommutativeArithExpr_t::IsEquivalent(const Expr_t *e) const
{
    assert(e);
    // same object
    if( IsEqual(e) )
        return 1;

    if( !ArithExpr_t::IsEquivalent(e) )
        return 0;

    const CommutativeArithExpr_t *ae = (const CommutativeArithExpr_t *) e;

    Expr_t *aeInpL = ae->Input(0);
    Expr_t *aeInpR = ae->Input(1);
    Expr_t *inpL = Input(0);
    Expr_t *inpR = Input(1);

    if( (!inpL->IsEquivalent(aeInpL) || !inpR->IsEquivalent(aeInpR)) &&
            (!inpR->IsEquivalent(aeInpL) || !inpL->IsEquivalent(aeInpR)) )
        return 0;

    return 1;
}

// equivalence check for non-commutative arithmetic operator expressions
int NonCommutativeArithExpr_t::IsEquivalent(const Expr_t *e) const
{
    assert(e);
    // same object
    if( IsEqual(e) )
        return 1;

    if( !ArithExpr_t::IsEquivalent(e) )
        return 0;

    const NonCommutativeArithExpr_t *ae = (const NonCommutativeArithExpr_t *) e;

    Expr_t *aeInpL = ae->Input(0);
    Expr_t *aeInpR = ae->Input(1);
    Expr_t *inpL = Input(0);
    Expr_t *inpR = Input(1);

    if( (!inpL->IsEquivalent(aeInpL) || !inpR->IsEquivalent(aeInpR)) )
        return 0;

    return 1;
}

// binding of relation attribute reference wrt the given schema
// binding is returned as the index into the schema
int RelArgRef_t::Binding(Schema_t *schema)
{
    assert(schema);
    int numAttr = schema->NumAttr();

    // search for relName, attrName 
    int bnd = -1;
    for( int i = 0 ; i < numAttr ; i++ ) {
        Attr_t *a = schema->Attribute(i);
        assert(a);

        char *aName = a->Name();

        // ignoring relations --- attr names unique across relations and
        // no support for AS --- so intermediate relation attributes are
        // also assumed to be unique
        if( strcmp(attrName, aName) == 0 ) {
            // not returning as soon as a binding is found to catch
            // ambiguity --- just a sanity check; to be removed later
            if( bnd != -1 ) {
                // multiple bindings possible!
                // flush(cerr << "Multiple Bindings for " <<
        //              relName << "." << attrName << endl); 
                // abort();
            }

            bnd = i;
        }
    }

    return bnd;
}

// infer schema properties in view of the predicate
float And_t::InferProp(Schema_t *schema, SchemaProp_t *schemaProp) const
{
    Predicate_t *right = (Predicate_t *) Input(1);
    float rightsel = right->InferProp(schema, schemaProp);

    Predicate_t *left = (Predicate_t *) Input(0);
    float leftsel = left->InferProp(schema, schemaProp);

    return leftsel*rightsel;
}

// infer schema properties in view of the predicate
// only consider the case when operands are values/relargrefs
float Eq_t::InferProp(Schema_t *schema, SchemaProp_t *schemaProp) const
{
    float sel = 1.0;

    Expr_t *left = Input(0);
    Expr_t *right = Input(1);

    ExprType_t leftExprType = left->ExprType();
    ExprType_t rightExprType = right->ExprType();

    if( leftExprType == REL_ARG_REF_T ) {
        RelArgRef_t *leftArgRef = (RelArgRef_t *) left;
        int leftBnd = leftArgRef->Binding(schema);
        if( leftBnd != -1 ) {
            if( rightExprType == VALUE_T ) {
                Value_t *rightVal = (Value_t *) right;

                sel = schemaProp->FixAttributeValue(leftBnd, rightVal);
            } else if( rightExprType == REL_ARG_REF_T ) {
                RelArgRef_t *rightArgRef = (RelArgRef_t *) right;

                int rightBnd = rightArgRef->Binding(schema);
                assert(rightBnd != -1); // Ravi: Why can't the rhs be a 
                                        // Ravi: correlated attribute?
                sel = schemaProp->EquateAttributes(leftBnd, rightBnd);
                // Ravi: update the attr eq classes
                schemaProp->UnifyAttrs(leftBnd, rightBnd);
            } else sel = 0.5;   // for arbit expressions
        } else {
            // left expr is a parameter --- assign arbitrary value

            if( rightExprType == REL_ARG_REF_T ) {
                RelArgRef_t *rightArgRef = (RelArgRef_t *) right;

                int rightBnd = rightArgRef->Binding(schema);
                if( rightBnd != -1 ) {
                    Int_t v(1);
                    sel = schemaProp->
                                FixAttributeValue(rightBnd, &v);
                } else sel = 0.0;               // parameter == parameter       
            } else if( rightExprType == VALUE_T ) {
                sel = 0.0;              // parameter == val
            } else sel = 0.5;           // arbitrary estimate
        }
    } else if( leftExprType == VALUE_T ) {
        Value_t *leftVal = (Value_t *) left;

        if( rightExprType == REL_ARG_REF_T ) {
            RelArgRef_t *rightArgRef = (RelArgRef_t *) right;

            int rightBnd = rightArgRef->Binding(schema);
            if( rightBnd != -1 )
                sel = schemaProp->FixAttributeValue(rightBnd, leftVal);
            else sel = 0.0;     // val == parameter
        } else if( rightExprType == VALUE_T ) {
            Value_t *rightVal = (Value_t *) right;

            if( leftVal->IsEq(rightVal) )
                sel = 1.0;
            else sel = 0.0;
        } else sel = 0.5;       // for arbit expression
    }

    return sel;
}

// infer schema properties in view of the predicate
float Less_t::InferProp(Schema_t *schema, SchemaProp_t *schemaProp) const
{
    float sel = 1.0;

    Expr_t *left = Input(0);
    Expr_t *right = Input(1);

    ExprType_t leftExprType = left->ExprType();
    ExprType_t rightExprType = right->ExprType();

    if( leftExprType == REL_ARG_REF_T ) {
        RelArgRef_t *leftArgRef = (RelArgRef_t *) left;
        int leftBnd = leftArgRef->Binding(schema);
        assert(leftBnd != -1);

        if( rightExprType == VALUE_T ) {
            Value_t *rightVal = (Value_t *) right;

            sel = schemaProp->UBoundAttributeValue(leftBnd, rightVal);
        } else if( rightExprType == REL_ARG_REF_T ) {
            RelArgRef_t *rightArgRef = (RelArgRef_t *) right;
            int rightBnd = rightArgRef->Binding(schema);
            assert(rightBnd != -1);

            sel = schemaProp->BoundAttributes(leftBnd, rightBnd);
        } else sel = 0.5;       // for arbit expressions
    } else if( leftExprType == VALUE_T ) {
        Value_t *leftVal = (Value_t *) left;

        if( rightExprType == REL_ARG_REF_T ) {
            RelArgRef_t *rightArgRef = (RelArgRef_t *) right;

            int rightBnd = rightArgRef->Binding(schema);
            assert(rightBnd != -1);
            sel = schemaProp->LBoundAttributeValue(rightBnd, leftVal);
        } else if( rightExprType == VALUE_T ) {
            Value_t *rightVal = (Value_t *) right;

            if( leftVal->IsLess(rightVal) )
                sel = 1.0;
            else sel = 0.0;
        } else sel = 0.5;       // for arbit expression
    }

    return sel;
}

// infer schema properties in view of the predicate
float LessEq_t::InferProp(Schema_t *schema, SchemaProp_t *schemaProp) const
{
    float sel = 1.0;

    Expr_t *left = Input(0);
    Expr_t *right = Input(1);

    ExprType_t leftExprType = left->ExprType();
    ExprType_t rightExprType = right->ExprType();

    if( leftExprType == REL_ARG_REF_T ) {
        RelArgRef_t *leftArgRef = (RelArgRef_t *) left;
        int leftBnd = leftArgRef->Binding(schema);
        assert(leftBnd != -1);

        if( rightExprType == VALUE_T ) {
            Value_t *rightVal = (Value_t *) right;

            schemaProp->UBoundAttributeValue(leftBnd, rightVal);
        } else if( rightExprType == REL_ARG_REF_T ) {
            RelArgRef_t *rightArgRef = (RelArgRef_t *) right;

            int rightBnd = rightArgRef->Binding(schema);
            assert(rightBnd != -1);
            schemaProp->BoundAttributes(leftBnd, rightBnd);
        }
    } else if( leftExprType == VALUE_T ) {
        Value_t *leftVal = (Value_t *) left;

        ExprType_t rightExprType = right->ExprType();

        if( rightExprType == REL_ARG_REF_T ) {
            RelArgRef_t *rightArgRef = (RelArgRef_t *) right;

            int rightBnd = rightArgRef->Binding(schema);
            assert(rightBnd != -1);
            schemaProp->LBoundAttributeValue(rightBnd, leftVal);
        } else if( rightExprType == VALUE_T ) {
            Value_t *rightVal = (Value_t *) right;

            if( leftVal->IsLess(rightVal) )
                sel = 1.0;
            else sel = 0.0;
        } else sel = 0.5;       // for arbit expression
    }

    return sel;
}

// infer selectivity in view of the predicate
float Not_t::Selectivity(Schema_t *schema, SchemaProp_t *schemaProp) const
{
    Expr_t *e = Input(0);
    assert(e->ExprType() == PREDICATE_T);

    Predicate_t *p = (Predicate_t *) e;
    float inpSel = p->Selectivity(schema, schemaProp);
    return 1.0 - inpSel;
}

// infer selectivity in view of the predicate
float Eq_t::Selectivity(Schema_t *schema, SchemaProp_t *schemaProp) const
{
    float sel = 0.0;

    Expr_t *left = Input(0);
    Expr_t *right = Input(1);

    ExprType_t leftExprType = left->ExprType();
    ExprType_t rightExprType = right->ExprType();

    if( leftExprType == REL_ARG_REF_T ) {
        RelArgRef_t *leftArgRef = (RelArgRef_t *) left;
        int leftBnd = leftArgRef->Binding(schema);
        if( leftBnd != -1 ) {
            if( rightExprType == REL_ARG_REF_T ) {
                RelArgRef_t *rightArgRef = (RelArgRef_t *) right;

                int rightBnd = rightArgRef->Binding(schema);
                assert(rightBnd != -1);
                sel = schemaProp->EquateAttributesSelectivity(leftBnd, rightBnd);
            } else if( rightExprType == VALUE_T ) {
                Value_t *rightVal = (Value_t *) right;

                sel = schemaProp->FixAttributeValueSelectivity(leftBnd, rightVal);
            } else sel = 0.5;           // arbitrary estimate
        } else {
            // left expr is a parameter --- assign arbitrary value

            if( rightExprType == REL_ARG_REF_T ) {
                RelArgRef_t *rightArgRef = (RelArgRef_t *) right;

                int rightBnd = rightArgRef->Binding(schema);
                if( rightBnd != -1 ) {
                    Int_t v(1);
                    sel = schemaProp->
                                FixAttributeValueSelectivity(rightBnd, &v);
                } else sel = 0.0;               // parameter == parameter       
            } else if( rightExprType == VALUE_T ) {
                sel = 0.0;              // parameter == val
            } else sel = 0.5;           // arbitrary estimate
        }
    } else if( leftExprType == VALUE_T ) {
        Value_t *leftVal = (Value_t *) left;

        if( rightExprType == REL_ARG_REF_T ) {
            RelArgRef_t *rightArgRef = (RelArgRef_t *) right;

            int rightBnd = rightArgRef->Binding(schema);
            if( rightBnd != -1 )
                sel = schemaProp->
                            FixAttributeValueSelectivity(rightBnd, leftVal);
            else sel = 0.0;             // val == parameter     
        } else if( rightExprType == VALUE_T ) {
            Value_t *rightVal = (Value_t *) right;

            if( leftVal->IsEquivalent(rightVal) )
                sel = 1.0;              // tautology
            else sel = 0.0;             // contradiction
        } else sel = 0.5;               // arbitrary estimate
    } else sel = 0.5;                   // arbitrary estimate

    return sel;
}

// infer selectivity in view of the predicate
float NEq_t::Selectivity(Schema_t *schema, SchemaProp_t *schemaProp) const
{
    float eqSel = 0.0;

    Expr_t *left = Input(0);
    Expr_t *right = Input(1);

    ExprType_t leftExprType = left->ExprType();
    ExprType_t rightExprType = right->ExprType();

    if( leftExprType == REL_ARG_REF_T ) {
        RelArgRef_t *leftArgRef = (RelArgRef_t *) left;
        int leftBnd = leftArgRef->Binding(schema);
        assert(leftBnd != -1);

        if( rightExprType == REL_ARG_REF_T ) {
            RelArgRef_t *rightArgRef = (RelArgRef_t *) right;

            int rightBnd = rightArgRef->Binding(schema);
            assert(rightBnd != -1);
            eqSel = schemaProp->EquateAttributesSelectivity(leftBnd, rightBnd);
        } else if( rightExprType == VALUE_T ) {
            Value_t *rightVal = (Value_t *) right;

            eqSel = schemaProp->FixAttributeValueSelectivity(leftBnd, rightVal);
        } else eqSel = 0.5;             // arbitrary estimate
    } else if( leftExprType == VALUE_T ) {
        Value_t *leftVal = (Value_t *) left;

        if( rightExprType == REL_ARG_REF_T ) {
            RelArgRef_t *rightArgRef = (RelArgRef_t *) right;

            int rightBnd = rightArgRef->Binding(schema);
            assert(rightBnd != -1);
            eqSel = schemaProp->FixAttributeValueSelectivity(rightBnd, leftVal);
        } else if( rightExprType == VALUE_T ) {
            Value_t *rightVal = (Value_t *) right;

            if( leftVal->IsEquivalent(rightVal) )
                eqSel = 1.0;            // tautology
            else eqSel = 0.0;           // contradiction
        } else eqSel = 0.5;             // arbitrary estimate
    } else eqSel = 0.5;                 // arbitrary estimate

    return 1.0 - eqSel;
}

// infer selectivity in view of the predicate
float And_t::Selectivity(Schema_t *schema, SchemaProp_t *schemaProp) const
{
    Predicate_t *left = (Predicate_t *) Input(0);
    assert(left);
    Predicate_t *right = (Predicate_t *) Input(1);
    assert(right);

    float leftsel = left->Selectivity(schema, schemaProp);

    if( left->Implies(right) )
        return leftsel;

    float rightsel = right->Selectivity(schema, schemaProp);

    if( right->Implies(left) )
        return rightsel;

    float sel = leftsel*rightsel;
    // arbitrary, to avoid considering null-result queries
    if( sel == 0 )
        sel = 0.0001;
    return sel;
}

// infer selectivity in view of the predicate
float Or_t::Selectivity(Schema_t *schema, SchemaProp_t *schemaProp) const
{
    Predicate_t *left = (Predicate_t *) Input(0);
    assert(left);
    Predicate_t *right = (Predicate_t *) Input(1);
    assert(right);

    float leftsel = left->Selectivity(schema, schemaProp);

    if( right->Implies(left) )
        return leftsel;

    float rightsel = right->Selectivity(schema, schemaProp);

    if( left->Implies(right) )
        return rightsel;

    float sel =  1.0 - (1.0 - leftsel) * (1.0 - rightsel);

    // arbitrary, to avoid considering null-result queries
    if( sel == 0 )
        sel = 0.0001;
    return sel;
}

// infer selectivity in view of the predicate
float Less_t::Selectivity(Schema_t *schema, SchemaProp_t *schemaProp) const
{ 
    float sel = 1.0;
    Expr_t *left = Input(0);
    Expr_t *right = Input(1);

    ExprType_t leftExprType = left->ExprType();

    if( leftExprType == REL_ARG_REF_T ) {
        RelArgRef_t *leftArgRef = (RelArgRef_t *) left;
        int leftBnd = leftArgRef->Binding(schema);
        assert(leftBnd != -1);

        ExprType_t rightExprType = right->ExprType();

        if( rightExprType == REL_ARG_REF_T ) {
            RelArgRef_t *rightArgRef = (RelArgRef_t *) right;

            int rightBnd = rightArgRef->Binding(schema);
            assert(rightBnd != -1);
            sel = schemaProp->BoundAttributesSelectivity(leftBnd, rightBnd);
        } else if( rightExprType == VALUE_T ) {
            Value_t *rightVal = (Value_t *) right;

            sel = schemaProp->UBoundAttributeValueSelectivity(leftBnd,
                                                                rightVal);
        } else sel = 0.5;               // arbitrary estimate
    } else if( leftExprType == VALUE_T ) {
        Value_t *leftVal = (Value_t *) left;

        ExprType_t rightExprType = right->ExprType();

        if( rightExprType == REL_ARG_REF_T ) {
            RelArgRef_t *rightArgRef = (RelArgRef_t *) right;

            int rightBnd = rightArgRef->Binding(schema);
            assert(rightBnd != -1);
            sel = schemaProp->LBoundAttributeValueSelectivity(rightBnd,
                                                                leftVal);
        } else if( rightExprType == VALUE_T ) {
            Value_t *rightVal = (Value_t *) right;

            if( leftVal->IsLess(rightVal) )
                sel = 1.0;
            else sel = 0.0;
        } else sel = 0.5;       // for arbit expression
    } else sel = 0.5;                   // arbitrary estimate

    return sel;
}

// infer selectivity in view of the predicate
float LessEq_t::Selectivity(Schema_t *schema, SchemaProp_t *schemaProp) const
{
    float sel = 1.0;
    Expr_t *left = Input(0);
    Expr_t *right = Input(1);

    ExprType_t leftExprType = left->ExprType();

    if( leftExprType == REL_ARG_REF_T ) {
        RelArgRef_t *leftArgRef = (RelArgRef_t *) left;
        int leftBnd = leftArgRef->Binding(schema);
        assert(leftBnd != -1);

        ExprType_t rightExprType = right->ExprType();

        if( rightExprType == REL_ARG_REF_T ) {
            RelArgRef_t *rightArgRef = (RelArgRef_t *) right;

            int rightBnd = rightArgRef->Binding(schema);
            assert(rightBnd != -1);
            sel = schemaProp->BoundAttributesSelectivity(leftBnd, rightBnd);
        } else if( rightExprType == VALUE_T ) {
            Value_t *rightVal = (Value_t *) right;

            sel = schemaProp->UBoundAttributeValueSelectivity(leftBnd,
                                                                rightVal);
        } else sel = 0.5;               // arbitrary estimate
    } else if( leftExprType == VALUE_T ) {
        Value_t *leftVal = (Value_t *) left;

        ExprType_t rightExprType = right->ExprType();

        if( rightExprType == REL_ARG_REF_T ) {
            RelArgRef_t *rightArgRef = (RelArgRef_t *) right;

            int rightBnd = rightArgRef->Binding(schema);
            assert(rightBnd != -1);
            sel = schemaProp->LBoundAttributeValueSelectivity(rightBnd,
                                                                leftVal);
        } else if( rightExprType == VALUE_T ) {
            Value_t *rightVal = (Value_t *) right;

            if( leftVal->IsLess(rightVal) )
                sel = 1.0;
            else sel = 0.0;
        } else sel = 0.5;       // for arbit expression
    } else sel = 0.5;                   // arbitrary estimate

    return sel;
}

// [not P] pushed down iff [P] pushed down
// parSchema is NULL => do not push correlated predicates
void Not_t::Decompose(Schema_t *parSchema, Schema_t *childSchema,
                Predicate_t **parPred, Predicate_t **childPred)
{
    assert(childSchema);
    assert(parPred);
    assert(childPred);

    Predicate_t *decParPred = NULL;
    Predicate_t *decChildPred = NULL;

    Predicate_t *inpPred = (Predicate_t *) Input(0);
    assert(inpPred);

    inpPred->Decompose(parSchema, childSchema, &decParPred, &decChildPred);

    if( decParPred->IsEquivalent(ConstPreds.True()) ) {
        // input can be pushed down
        *parPred = ConstPreds.True();
        *childPred = this;
    } else {
        // input cannot be pushed down
        *parPred = this;
        *childPred = ConstPreds.True();
    }

    Refer(*parPred);
    Refer(*childPred);

    DeRefer(decParPred);
    DeRefer(decChildPred);
}

// schema is valid for [not P] iff it is valid for [P]
int Not_t::IsValidSchema(Schema_t *parSchema, Schema_t *schema) const
{
    assert(schema);
    Predicate_t *inpPred = (Predicate_t *) Input(0);
    assert(inpPred);

    return inpPred->IsValidSchema(parSchema, schema);
}

// parSchema is NULL => do not push correlated predicates
void Eq_t::Decompose(Schema_t *parSchema, Schema_t *childSchema,
                    Predicate_t **parPred, Predicate_t **childPred)
{
    assert(childSchema);
    assert(parPred);
    assert(childPred);

    if( IsValidSchema(parSchema, childSchema) ) {
        // can be pushed down
        *parPred = ConstPreds.True();
        *childPred = this;
    } else {
        // cannot be pushed down
        *parPred = this;
        *childPred = ConstPreds.True();
    }

    Refer(*parPred);
    Refer(*childPred);
}

// schema valid if all the variables used bind successfully
int Eq_t::IsValidSchema(Schema_t *parSchema, Schema_t *childSchema) const
{
    assert(childSchema);

    Expr_t *left = Input(0);
    Expr_t *right = Input(1);

    return IsValidExpr(parSchema, childSchema, left) &&
                        IsValidExpr(parSchema, childSchema, right);
}

// parSchema is NULL => do not push correlated predicates
void Less_t::Decompose(Schema_t *parSchema, Schema_t *childSchema,
                    Predicate_t **parPred, Predicate_t **childPred)
{
    assert(childSchema);
    assert(parPred);
    assert(childPred);

    if( IsValidSchema(parSchema, childSchema) ) {
        // can be pushed down
        *parPred = ConstPreds.True();
        *childPred = this;
    } else {
        // cannot be pushed down
        *parPred = this;
        *childPred = ConstPreds.True();
    }

    Refer(*parPred);
    Refer(*childPred);
}

// schema valid if all the variables used bind successfully
int Less_t::IsValidSchema(Schema_t *parSchema, Schema_t *childSchema) const
{
    assert(childSchema);

    Expr_t *left = Input(0);
    Expr_t *right = Input(1);

    return IsValidExpr(parSchema, childSchema, left) &&
                        IsValidExpr(parSchema, childSchema, right);
}

// parSchema is NULL => do not push correlated predicates
void LessEq_t::Decompose(Schema_t *parSchema, Schema_t *childSchema,
                    Predicate_t **parPred, Predicate_t **childPred)
{
    assert(childSchema);
    assert(parPred);
    assert(childPred);

    if( IsValidSchema(parSchema, childSchema) ) {
        // can be pushed down
        *parPred = ConstPreds.True();
        *childPred = this;
    } else {
        // cannot be pushed down
        *parPred = this;
        *childPred = ConstPreds.True();
    }

    Refer(*parPred);
    Refer(*childPred);
}

// schema valid if all the variables used bind successfully
int LessEq_t::IsValidSchema(Schema_t *parSchema, Schema_t *childSchema) const
{
    assert(childSchema);

    Expr_t *left = Input(0);
    Expr_t *right = Input(1);

    return IsValidExpr(parSchema, childSchema, left) &&
                        IsValidExpr(parSchema, childSchema, right);
}

// parSchema is NULL => do not push correlated predicates
void NEq_t::Decompose(Schema_t *parSchema, Schema_t *childSchema,
                    Predicate_t **parPred, Predicate_t **childPred)
{
    assert(childSchema);
    assert(parPred);
    assert(childPred);

    if( IsValidSchema(parSchema, childSchema) ) {
        // can be pushed down
        *parPred = ConstPreds.True();
        *childPred = this;
    } else {
        // cannot be pushed down
        *parPred = this;
        *childPred = ConstPreds.True();
    }

    Refer(*parPred);
    Refer(*childPred);
}

// schema valid if all the variables used bind successfully
int NEq_t::IsValidSchema(Schema_t *parSchema, Schema_t *schema) const
{
    assert(schema);

    Expr_t *left = Input(0);
    Expr_t *right = Input(1);

    return IsValidExpr(parSchema, schema, left) &&
                        IsValidExpr(parSchema, schema, right);
}

// parSchema is NULL => do not push correlated predicates
void And_t::Decompose(Schema_t *parSchema, Schema_t *childSchema,
                    Predicate_t **parPred, Predicate_t **childPred)
{
    assert(childSchema);
    assert(parPred);
    assert(childPred);

    Predicate_t *left = (Predicate_t *) Input(0);
    assert(left);
    Predicate_t *right = (Predicate_t *) Input(1);
    assert(right);

    Predicate_t *leftParPred = NULL;
    Predicate_t *leftChildPred = NULL;
    left->Decompose(parSchema, childSchema, &leftParPred, &leftChildPred);

    Predicate_t *rightParPred = NULL;
    Predicate_t *rightChildPred = NULL;
    right->Decompose(parSchema, childSchema, &rightParPred, &rightChildPred);

    if( leftParPred->IsEquivalent(ConstPreds.True()) ) {
        // left input can be pushed down
        *parPred = rightParPred;
    } else if( rightParPred->IsEquivalent(ConstPreds.True()) ) {
        // right input can be pushed down
        *parPred = leftParPred;
    } else {
        // none of the inputs can be pushed down
        *parPred = new And_t(leftParPred, rightParPred);
    }

    if( leftChildPred->IsEquivalent(ConstPreds.True()) ) {
        // left input can be pushed down
        *childPred = rightChildPred;
    } else if( rightChildPred->IsEquivalent(ConstPreds.True()) ) {
        // right input can be pushed down
        *childPred = leftChildPred;
    } else {
        // none of the inputs can be pushed down
        *childPred = new And_t(leftChildPred, rightChildPred);
    }

    Refer(*parPred);
    Refer(*childPred);

    DeRefer(leftParPred);
    DeRefer(leftChildPred);
    DeRefer(rightParPred);
    DeRefer(rightChildPred);
}

int And_t::IsValidSchema(Schema_t *parSchema, Schema_t *childSchema) const
{
    assert(childSchema);

    Predicate_t *left = (Predicate_t *) Input(0);
    assert(left);
    Predicate_t *right = (Predicate_t *) Input(1);
    assert(right);

    return left->IsValidSchema(parSchema, childSchema) &&
                            right->IsValidSchema(parSchema, childSchema);
}

// parSchema is NULL => do not push correlated predicates
void Or_t::Decompose(Schema_t *parSchema, Schema_t *childSchema,
                    Predicate_t **parPred, Predicate_t **childPred)
{
    assert(childSchema);
    assert(parPred);
    assert(childPred);

    Predicate_t *left = (Predicate_t *) Input(0);
    assert(left);
    Predicate_t *right = (Predicate_t *) Input(1);
    assert(right);

    Predicate_t *leftParPred = NULL;
    Predicate_t *leftChildPred = NULL;
    left->Decompose(parSchema, childSchema, &leftParPred, &leftChildPred);

    Predicate_t *rightParPred = NULL;
    Predicate_t *rightChildPred = NULL;
    right->Decompose(parSchema, childSchema, &rightParPred, &rightChildPred);

    if( leftParPred->IsEquivalent(ConstPreds.True()) &&
                        rightParPred->IsEquivalent(ConstPreds.True()) ) {
        // both inputs can be pused --- so the or clause can be pushed
        *parPred = ConstPreds.True();
        *childPred = this;
    } else {
        // cannot be pushed
        *parPred = this;
        *childPred = ConstPreds.True();
    }

    Refer(*parPred);
    Refer(*childPred);

    DeRefer(leftParPred);
    DeRefer(leftChildPred);
    DeRefer(rightParPred);
    DeRefer(rightChildPred);
}

int Or_t::IsValidSchema(Schema_t *parSchema, Schema_t *schema) const
{
    assert(schema);

    Predicate_t *left = (Predicate_t *) Input(0);
    assert(left);
    Predicate_t *right = (Predicate_t *) Input(1);
    assert(right);

    return left->IsValidSchema(parSchema, schema) &&
                        right->IsValidSchema(parSchema, schema);
}

// nothing to decompose
void ConstPredicate_t::Decompose(Schema_t *, Schema_t *childSchema,
                Predicate_t **parPred, Predicate_t **childPred)
{
    assert(childSchema);
    assert(parPred);
    *parPred = this;
    assert(childPred);
    *childPred = ConstPreds.True();

    Refer(*parPred);
    Refer(*childPred);
}

// get the bindings that are part of the relevant join predicate
int Eq_t::SortAttrBnd(Schema_t *outSchema,
                            Schema_t *leftSchema, Schema_t *rightSchema,
                            int *outBnd, int *leftBnd, int *rightBnd) const
{
    assert(outSchema);
    assert(leftSchema);
    assert(rightSchema);
    assert(outBnd);
    assert(leftBnd);
    assert(rightBnd);

    Expr_t *leftInp = Input(0);
    assert(leftInp);
    Expr_t *rightInp = Input(1);
    assert(rightInp);

    if( leftInp->ExprType() == REL_ARG_REF_T &&
                rightInp->ExprType() == REL_ARG_REF_T ) {
        // join predicate

        RelArgRef_t *leftRef = (RelArgRef_t *) leftInp;
        RelArgRef_t *rightRef = (RelArgRef_t *) rightInp;

        int lBnd = -1;
        int rBnd = -1;
            
        lBnd = leftRef->Binding(leftSchema);

        if( lBnd != -1 ) {
            // left ref binds in the left input
            // now check for the right ref in the right input

            rBnd = rightRef->Binding(rightSchema);
        }

        if( lBnd == -1 || rBnd == -1 ) {
            // failed binding -- try again by commuting the inputs

            lBnd = -1;
            rBnd = -1;

            lBnd = rightRef->Binding(leftSchema);

            if( lBnd != -1 ) {
                // right ref binds in the left relation
                // now check for the left ref in the right relation

                rBnd = leftRef->Binding(rightSchema);
            }
        }

        if( lBnd != -1 && rBnd != -1 ) {
            // both values bind successfully

            *leftBnd = lBnd;
            *rightBnd = rBnd;
            // Ravi: Why is that only the binding of the leftRef in the out
            // Ravi: schema returned?
            *outBnd = leftRef->Binding(outSchema);

            // whatever binds in the inputs must also bind in
            // the output (we exclude expressions)
            assert(*outBnd != -1);

            return 1;
        }

    }

    return 0;
}

void Eq_t::JoinAttrs(Set_t<int> &attrSet, Schema_t *outSchema, 
                     Schema_t *leftSchema, Schema_t *rightSchema) 
{
    Expr_t *leftInp = Input(0);
    assert(leftInp);
    Expr_t *rightInp = Input(1);
    assert(rightInp);

    if( leftInp->ExprType() == REL_ARG_REF_T &&
        rightInp->ExprType() == REL_ARG_REF_T ) {

        RelArgRef_t *leftRef  = (RelArgRef_t *) leftInp;
        RelArgRef_t *rightRef = (RelArgRef_t *) rightInp;

        // Check if it is a join predicate
        int llBnd = leftRef->Binding(leftSchema);
        int rrBnd = rightRef->Binding(rightSchema);
        int lrBnd = leftRef->Binding(rightSchema);
        int rlBnd = rightRef->Binding(leftSchema);
        if ((llBnd != -1 && rrBnd != -1) || (lrBnd != -1 && rlBnd != -1)) {
            // We need any one binding in the outSchema since we anyway
            // replace it by eq class head.
            int loBnd = leftRef->Binding(outSchema);
            assert(loBnd != -1);
            attrSet.AddElement(loBnd); 
        }
    }
}

// get the bindings that are part of the relevant join predicate
int And_t::SortAttrBnd(Schema_t *outSchema,
                            Schema_t *leftSchema, Schema_t *rightSchema,
                            int *outBnd, int *leftBnd, int *rightBnd) const
{
    assert(outSchema);
    assert(leftSchema);
    assert(rightSchema);
    assert(outBnd);
    assert(leftBnd);
    assert(rightBnd);

    Expr_t *leftInp = Input(0);
    assert(leftInp);
    Expr_t *rightInp = Input(1);
    assert(rightInp);

    assert(leftInp->ExprType() == PREDICATE_T);
    Predicate_t *leftPred = (Predicate_t *) leftInp;

    int isLeftValid = leftPred->SortAttrBnd(outSchema, leftSchema,
                                rightSchema, outBnd, leftBnd, rightBnd);
            
    if( isLeftValid ) 
        return 1;

    assert(rightInp->ExprType() == PREDICATE_T);
    Predicate_t *rightPred = (Predicate_t *) rightInp;

    int isRightValid = rightPred->SortAttrBnd(outSchema, leftSchema,
                                rightSchema, outBnd, leftBnd, rightBnd);
            
    if( isRightValid ) 
        return 1;

    return 0;
}

void And_t::JoinAttrs(Set_t<int> &attrSet, Schema_t *outSchema, 
                      Schema_t *leftSchema, Schema_t *rightSchema) 
{
    Predicate_t *leftInp = (Predicate_t *) Input(0);
    assert(leftInp);
    Predicate_t *rightInp = (Predicate_t *) Input(1);
    assert(rightInp);
    
    leftInp->JoinAttrs(attrSet, outSchema, leftSchema, rightSchema);
    rightInp->JoinAttrs(attrSet, outSchema, leftSchema, rightSchema);
}

// get the bindings that are part of the relevant select predicate
int Less_t::IndexAttrBnd(Schema_t *schema, int *bnd) const
{
    assert(schema);
    assert(bnd);

    Expr_t *leftInp = Input(0);
    assert(leftInp);

    Expr_t *rightInp = Input(1);
    assert(rightInp);

    if( leftInp->ExprType() == REL_ARG_REF_T &&
                rightInp->ExprType() == REL_ARG_REF_T ) {
        RelArgRef_t *leftRef = (RelArgRef_t *) leftInp;
        int lBnd = leftRef->Binding(schema);

        RelArgRef_t *rightRef = (RelArgRef_t *) rightInp;
        int rBnd = rightRef->Binding(schema);

        if( lBnd != -1 && rBnd == -1 ) {
            // index is no help in secondary join predicate
            return 0;
        }

        if( lBnd == -1 && rBnd == -1 ) {
            // parameter == parameter
            return 0;
        }

        if( lBnd != -1 ) {
            *bnd = lBnd;

            return 1;
        }

        if( rBnd != -1 ) {
            *bnd = rBnd;

            return 1;
        }
    } else if( leftInp->ExprType() == REL_ARG_REF_T ) {
        assert(rightInp->ExprType() == VALUE_T );

        RelArgRef_t *leftRef = (RelArgRef_t *) leftInp;
        int lBnd = leftRef->Binding(schema);

        if( lBnd != -1 ) {
            *bnd = lBnd;

            return 1;
        }
    } else if( rightInp->ExprType() == REL_ARG_REF_T ) {
        assert(leftInp->ExprType() == VALUE_T );

        RelArgRef_t *rightRef = (RelArgRef_t *) rightInp;
        int rBnd = rightRef->Binding(schema);

        if( rBnd != -1 ) {
            *bnd = rBnd;

            return 1;
        }
    }

    return 0;
}

// get the bindings that are part of the relevant select predicate
int Eq_t::IndexAttrBnd(Schema_t *schema, int *bnd) const
{
    assert(schema);
    assert(bnd);

    Expr_t *leftInp = Input(0);
    assert(leftInp);

    Expr_t *rightInp = Input(1);
    assert(rightInp);

    if( leftInp->ExprType() == REL_ARG_REF_T &&
                rightInp->ExprType() == REL_ARG_REF_T ) {
        RelArgRef_t *leftRef = (RelArgRef_t *) leftInp;
        int lBnd = leftRef->Binding(schema);

        RelArgRef_t *rightRef = (RelArgRef_t *) rightInp;
        int rBnd = rightRef->Binding(schema);

        if( lBnd != -1 && rBnd == -1 ) {
            // index is no help in secondary join predicate
            return 0;
        }

        if( lBnd == -1 && rBnd == -1 ) {
            // parameter == parameter
            return 0;
        }

        if( lBnd != -1 ) {
            *bnd = lBnd;

            return 1;
        }

        if( rBnd != -1 ) {
            *bnd = rBnd;

            return 1;
        }
    } else if( leftInp->ExprType() == REL_ARG_REF_T ) {
        assert(rightInp->ExprType() == VALUE_T );

        RelArgRef_t *leftRef = (RelArgRef_t *) leftInp;
        int lBnd = leftRef->Binding(schema);

        if( lBnd != -1 ) {
            *bnd = lBnd;

            return 1;
        }
    } else if( rightInp->ExprType() == REL_ARG_REF_T ) {
        assert(leftInp->ExprType() == VALUE_T );

        RelArgRef_t *rightRef = (RelArgRef_t *) rightInp;
        int rBnd = rightRef->Binding(schema);

        if( rBnd != -1 ) {
            *bnd = rBnd;

            return 1;
        }
    }

    return 0;
}

// get the bindings that are part of the relevant select predicate
int And_t::IndexAttrBnd(Schema_t *schema, int *bnd) const
{
    assert(schema);
    assert(bnd);

    Expr_t *leftInp = Input(0);
    assert(leftInp);
    Expr_t *rightInp = Input(1);
    assert(rightInp);

    assert(leftInp->ExprType() == PREDICATE_T);
    Predicate_t *leftPred = (Predicate_t *) leftInp;

    int isLeftValid = leftPred->IndexAttrBnd(schema, bnd);
            
    if( isLeftValid ) 
        return 1;

    assert(rightInp->ExprType() == PREDICATE_T);
    Predicate_t *rightPred = (Predicate_t *) rightInp;

    int isRightValid = rightPred->IndexAttrBnd(schema, bnd);
            
    if( isRightValid ) 
        return 1;

    return 0;
}

// get the bindings that are part of the relevant join predicate
int Eq_t::IndexAttrBnd(Schema_t *leftSchema, Schema_t *rightSchema,
                                        int *leftBnd, int *rightBnd) const
{
    assert(leftSchema);
    assert(rightSchema);
    assert(leftBnd);
    assert(rightBnd);

    Expr_t *leftInp = Input(0);
    assert(leftInp);
    Expr_t *rightInp = Input(1);
    assert(rightInp);

    if( leftInp->ExprType() == REL_ARG_REF_T &&
                rightInp->ExprType() == REL_ARG_REF_T ) {
        // join predicate

        RelArgRef_t *leftRef = (RelArgRef_t *) leftInp;
        RelArgRef_t *rightRef = (RelArgRef_t *) rightInp;

        int lBnd = -1;
        int rBnd = -1;
            
        lBnd = leftRef->Binding(leftSchema);

        if( lBnd != -1 ) {
            // left ref binds in the left input
            // now check for the right ref in the right input

            rBnd = rightRef->Binding(rightSchema);
        }

        if( lBnd == -1 || rBnd == -1 ) {
            // failed binding -- try again by commuting the inputs

            lBnd = -1;
            rBnd = -1;

            lBnd = rightRef->Binding(leftSchema);

            if( lBnd != -1 ) {
                // right ref binds in the left relation
                // now check for the left ref in the right relation

                rBnd = leftRef->Binding(rightSchema);
            }
        }

        if( lBnd != -1 && rBnd != -1 ) {
            // both values bind successfully

            *leftBnd = lBnd;
            *rightBnd = rBnd;

            return 1;
        }
    } else if( leftInp->ExprType() == REL_ARG_REF_T ) {
        assert(rightInp->ExprType() == VALUE_T );

        RelArgRef_t *leftRef = (RelArgRef_t *) leftInp;
        int lBnd = leftRef->Binding(leftSchema);

        if( lBnd != -1 ) {
            *leftBnd = lBnd;

            return 1;
        }
    } else if( rightInp->ExprType() == REL_ARG_REF_T ) {
        assert(leftInp->ExprType() == VALUE_T );

        RelArgRef_t *rightRef = (RelArgRef_t *) rightInp;
        int rBnd = rightRef->Binding(rightSchema);

        if( rBnd != -1 ) {
            *rightBnd = rBnd;

            return 1;
        }
    }

    return 0;
}

// get the bindings that are part of the relevant join predicate
int And_t::IndexAttrBnd(Schema_t *leftSchema, Schema_t *rightSchema,
                                        int *leftBnd, int *rightBnd) const
{
    assert(leftSchema);
    assert(rightSchema);
    assert(leftBnd);
    assert(rightBnd);

    Expr_t *leftInp = Input(0);
    assert(leftInp);
    Expr_t *rightInp = Input(1);
    assert(rightInp);

    assert(leftInp->ExprType() == PREDICATE_T);
    Predicate_t *leftPred = (Predicate_t *) leftInp;

    int isLeftValid = leftPred->IndexAttrBnd(leftSchema, rightSchema,
                                                        leftBnd, rightBnd);
            
    if( isLeftValid ) 
        return 1;

    assert(rightInp->ExprType() == PREDICATE_T);
    Predicate_t *rightPred = (Predicate_t *) rightInp;

    int isRightValid = rightPred->IndexAttrBnd(leftSchema, rightSchema,
                                                        leftBnd, rightBnd);
            
    if( isRightValid ) 
        return 1;

    return 0;
}

// implication testing
int Not_t::Implies(const Predicate_t *p) const
{
    assert(p);
    if( Predicate_t::Implies(p) )
        return 1;

    if( p->PredOpType() == NOT_T ) {
        Predicate_t *x = (Predicate_t *) p->Input(0);
        assert(x);

        Predicate_t *y = (Predicate_t *) Input(0);
        assert(y);

        return x->Implies(y);
    }

    return 0;
}

// implication testing
int And_t::Implies(const Predicate_t *p) const
{
    assert(p);
    if( Predicate_t::Implies(p) )
        return 1;

    Predicate_t *left = (Predicate_t *) Input(0);
    assert(left);

    Predicate_t *right = (Predicate_t *) Input(1);
    assert(right);

    // return left->Implies(p) && right->Implies(p);
    return left->Implies(p) || right->Implies(p);
}

// implication testing
int Or_t::Implies(const Predicate_t *p) const
{
    assert(p);
    if( Predicate_t::Implies(p) )
        return 1;

    Predicate_t *left = (Predicate_t *) Input(0);
    assert(left);

    Predicate_t *right = (Predicate_t *) Input(1);
    assert(right);

    // return left->Implies(p) || right->Implies(p);
    return left->Implies(p) && right->Implies(p);
}

// implication testing
int Less_t::Implies(const Predicate_t *p) const
{
    assert(p);
    if( Predicate_t::Implies(p) )
        return 1;
    if( p->PredOpType() != LT_T )
        return 0;

    Expr_t *left = Input(0);
    assert(left);
    if( left->ExprType() != REL_ARG_REF_T )
        return 0;

    Expr_t *right = Input(1);
    assert(right);
    if( right->ExprType() != VALUE_T )
        return 0;

    Expr_t *pLeft = p->Input(0);
    assert(pLeft);
    if( pLeft->ExprType() != REL_ARG_REF_T )
        return 0;

    Expr_t *pRight = p->Input(1);
    assert(pRight);
    if( pRight->ExprType() != VALUE_T )
        return 0;

    if( !left->IsEquivalent(pLeft) )
        return 0;

    Value_t *val = (Value_t *) right;
    Value_t *pVal = (Value_t *) pRight;

    return val->IsLess(pVal);
}

// find the binding of the correlated predicate
// Ravi: Method unused? This is called only from InExpr_t::FindCorrelated
// Ravi: which in turn is an unused method!
int InExpr_t::FindCorrelatedTrav(Equivalence_t *eq, Schema_t *s, int level)
{
    assert(eq);
    assert(s);
    assert(level >= 0);

    // attach the iterators
    List_t<LogicalOp_t *>& logExprs = eq->LogExprList();
    ListIter_t<LogicalOp_t *> logExprIter;
    logExprIter.Attach(&logExprs);

    int bnd = -1;
    while( !logExprIter.IsEnd() ) {
        LogicalOp_t *op = logExprIter.Next();
        assert(op);
        if( op->IsDetached() )
            continue;

        int n = op->NumInputs();
        for( int i = 0 ; i < n && bnd == -1 ; i++ ) {
            Equivalence_t *inp = op->Input(i);
            assert(inp);

            bnd = FindCorrelatedTrav(inp, s, level);
        }

        if( bnd != -1 )
            break;

        if( op->Type() == SELECT_T || op->Type() == JOIN_T ) {
            // check the predicate
            Filter_t *f = (Filter_t *) op;
            Predicate_t *p = f->Predicate();
            assert(p);

            bnd = p->FindCorrelated(s, level + 1);
            if( bnd != -1 )
                break;
        }
    }

    return bnd;
}

// TO BE REMOVED
// mark the correlated nodes
// Ravi: Unused method.
int InExpr_t::MarkCorrelatedTrav(Equivalence_t *eq, Schema_t *, int) 
{
    assert(0);  // not used
    return eq->IsCorrelated();
}

#if 0
--- NOT USED ---
// mark the correlated nodes
int InExpr_t::MarkCorrelatedTrav(Equivalence_t *eq, Schema_t *s, int isNested) 
{
    assert(eq);
    assert(s);

    if( eq->IsCorrelated() )
        return 1;

    // attach the iterators
    List_t<LogicalOp_t *>& logExprs = eq->LogExprList();
    ListIter_t<LogicalOp_t *> logExprIter;
    logExprIter.Attach(&logExprs);

    int isCorrelated = 0;
    while( !logExprIter.IsEnd() ) {
        LogicalOp_t *op = logExprIter.Next();
        assert(op);
        if( op->IsDetached() )
            continue;

        int n = op->NumInputs();
        for( int i = 0 ; i < n ; i++ ) {
            Equivalence_t *inp = op->Input(i);
            assert(inp);

            int isInpCorrelated = MarkCorrelatedTrav(inp, s, isNested);
            if( !isCorrelated )
                isCorrelated = isInpCorrelated;
        }

        if( op->Type() == SELECT_T || op->Type() == JOIN_T ) {
            // check the predicate
            Filter_t *f = (Filter_t *) op;
            Predicate_t *p = f->Predicate();
            assert(p);

            if( !isCorrelated ) {
                if( !p->IsValidSchema(s) ) {
                    isCorrelated = 1;
                    if( Config_t::IsDebug(10) )
                        cout << "Found Correlated Eq = " << ID() << endl;
                    p->MarkCorrelated(s, 1);
                }
            }
        }
    }

    if( isCorrelated )
        eq->MarkCorrelated();

    return isCorrelated;
}
#endif

int InExpr_t::IsEquivalent (const Expr_t* e) const
{
    assert(e);

    assert(e->ExprType() == PREDICATE_T);
    const Predicate_t *p = (const Predicate_t *) e;

    if( IN_T != p->PredOpType() )
        return (0);

    const InExpr_t *pInExpr = (const InExpr_t *) p;

    Equivalence_t *pInpEq = pInExpr->InpEq();
    assert(pInpEq);

    assert(inpEq);
    if( inpEq->ID() != pInpEq->ID() )
        return 0;

    RelArgRef_t *pInpRelArgRef = pInExpr->InpRelArgRef();
    assert(pInpRelArgRef);

    assert(inpRelArgRef);
    return inpRelArgRef->IsEquivalent(pInpRelArgRef);
}
 
// TODO: Fix this to pass the input for estimation
Card_t InExpr_t::CalculateNumDistinct(LogProp_t *outLP)
{
    assert(outLP);
    Schema_t *s = outLP->Schema();
    SchemaProp_t *sp = outLP->SchemaProp();

    // Ravi: If subquery is not correlated, always 1 is returned!
    // Ravi: Changing to 10 for testing
    Card_t n = 10;

    int p = FindCorrelated(s, 0);

    if ( p != -1 ) {
        AttrProp_t *attrProp = sp->AttributeProp(p);
        assert(attrProp);

        // Number of distinct values of the attribute 
        n = attrProp->NumDistinct();
        if( n > outLP->RelSize_Tuples() )
            n = outLP->RelSize_Tuples();
    }

    // cout << "NumDistinct = " << n << endl;
    return n;
}

float InExpr_t::CalculateSelectivity(Schema_t *, SchemaProp_t *) const  
{
    return 1.0;

    /*----
    REMOVED -- PRASAN

    AttrProp_t *attr = NULL;
    const char *inpAttrName = inpRelArgRef->AttrName();
    assert(inpAttrName);
    int n = s->NumAttr();

    int i = -1;

    for( i = 0; i < n ; i++ ) {
        Attr_t *a = s->Attribute(i);
        assert(a);
        const char *aName = a->Name();
        assert(aName);

        if( strcmp (inpAttrName, aName) == 0 ) {
            attr = sp->AttributeProp (i);
            break;
        }
    }

    if ( attr == NULL )
    return (0.0);

    // Number of distinct values of the attribute 
    // occuring at left side of the inExpr
    Card_t i1 = attr->NumDistinct ();
    assert(i1 > 0);

    LogProp_t *inpLP = inpEq->LogicalProp();
    assert(inpLP);

    Card_t i2 = inpLP->RelSize_Tuples();

    if( i2 > i1 ) {
        return 1.0;
    } 

    return i2/i1;
    ----*/
}

// parSchema is NULL => do not push correlated predicates
void InExpr_t::Decompose(Schema_t *parSchema, Schema_t *childSchema, 
                 Predicate_t **parPred, Predicate_t **childPred)

{
    assert(childSchema);
    assert(parPred);
    assert(childPred);

    if( IsValidSchema(parSchema, childSchema) ) {
        // can be pushed down
        *parPred = ConstPreds.True();
        *childPred = this;
    } else {
        // cannot be pushed down
        *parPred = this;
        *childPred = ConstPreds.True();
    }

    Refer(*parPred);
    Refer(*childPred);
}

// TODO: not pushing in "in-expr" currently
int InExpr_t::IsValidSchema(Schema_t *, Schema_t *) const
{
   return 0;
}

void InExpr_t::PrintExpr(int isFullExp) const
{
    cout << "<" ;
    assert(inpRelArgRef);
    inpRelArgRef->PrintExpr();

    cout <<" IN\n";

    if( isFullExp ) {
        assert(inpEq);
        inpEq->PrintExpressionDAG(10);
    } else {
        assert(inpPlanGroup);
        Plan_t *bestPlan = inpPlanGroup->BestPlan();
        if( !bestPlan )
            bestPlan = inpPlanGroup->BestAlgPlan();
        assert(bestPlan);
        bestPlan->PrintPlan(10);
    }

    cout << ">" ;
}

void InExpr_t::PrintExprXML(int isFullExp) const
{
    cout << "<INEXPR>" << endl;
    assert(inpRelArgRef);
    cout << "<LHS>" << endl;
    inpRelArgRef->PrintExpr();
    cout << "</LHS>" << endl;

    cout << "<RHS>" << endl;

    if( isFullExp ) {
        assert(inpEq);
        inpEq->PrintExpressionDAGXML(10);
    } else {
        assert(inpPlanGroup);
        Plan_t *bestPlan = inpPlanGroup->BestPlan();
        if( !bestPlan )
            bestPlan = inpPlanGroup->BestAlgPlan();
        assert(bestPlan);
        bestPlan->PrintPlan(10); //TODO: Change after implementing PrintPlanXML
    }
    cout << "</RHS>" << endl;

    cout << "</INEXPR>" << endl;
}

/**
 * Populates the list "corrVarList" with the attributes referenced in the
 * expression and not having a binding in the schema given. These are
 * presumed to be correlated attributes.
 */
void RelArgRef_t::CorrelatedAttrList(Schema_t *schema, 
                                     AppendList_t<Attr_t *> *corrVarList)
{
        assert(schema);
        assert(corrVarList);

        int binding = Binding(schema);

        if (binding == -1) {    // Correlated. Add to the list.
            Attr_t *attr = new Attr_t(relName, attrName, 1);
            corrVarList->Insert(attr);
        }
}

void InExpr_t::CorrelatedAttrList(Schema_t *schema, 
                                  AppendList_t<Attr_t *> *corrVarList)
{
    assert(schema);
    assert(corrVarList);

    Input(0)->CorrelatedAttrList(schema, corrVarList);
 
    inpEq->SetOuterVarLogProp();

    // From the subquery's correlation var list remove all those
    // which have a binding in the local schema

    int localAttrNum = schema->NumAttr();
    int childAttrNum = inpEq->OuterVarLogProp()->Schema()->NumAttr();

    for (int j = 0; j < childAttrNum; j++) {
        Attr_t *childCorrAttr = inpEq->OuterVarLogProp()->Schema()->Attribute(j);
        assert(childCorrAttr);
        int found = 0;
        for (int i = 0; i < localAttrNum; i++) {
            if (childCorrAttr->IsEquivalent(schema->Attribute(i))) {
                found = 1;
                break;
            }
        }
        if (!found)
            corrVarList->Insert(childCorrAttr);
    }
}


// From the predicate find all the parameter orders of interest
// considering the built-in index orders present in inpLP. The
// generated orders will be w.r.t. to the outer var schama of 
// the eqClass
void Expr_t::GetInterestingOrders(List_t<Order_t *> *opOrds, Equivalence_t *inpEq, 
                                  Equivalence_t *parentEq, int *plmap, int level)
{
        if (!inpEq->IsBaseRel()) {
            return;
        }
    
        Schema_t *outerVarSchema = parentEq->OuterVarLogProp()->Schema();
        int numOuterVars = outerVarSchema->NumAttr();
        int *order = new int[numOuterVars];
        for (int i = 0; i < numOuterVars; i++) {
            order[i] = i;
        }
       
        SortOrder_t *sorder = new SortOrder_t(order, numOuterVars);
        // cout << "Constructed interesting orders for Eq:" << parentEq->ID();
        // sorder->Print(outerVarSchema);

        opOrds->Insert((Order_t *&)sorder);
        
        return; 

        /*
        
        BaseRelLogProp_t *baseRelLogProp = (BaseRelLogProp_t *) inpEq->LogicalProp();
        List_t<IndexOrder_t *> *indexOrderList = baseRelLogProp->BuiltInIndexList();
        ListIter_t<IndexOrder_t *> indexIter;
        indexIter.Attach(indexOrderList);
        
        while(!indexIter.IsEnd()) {
            IndexOrder_t *indexOrder = indexIter.Next();
            assert(indexOrder);
        
    
        }
        */           

}


void Eq_t::GetMatchingOrder(Order_t *inpOrder, Schema_t *inpSchema,
                            Order_t *outOrder, Schema_t *outSchema)
{
    assert(inpOrder);
    assert(inpSchema);
    assert(outOrder);
    assert(outSchema);
    assert(inpOrder->OrderSize() == outOrder->OrderSize());

    Expr_t *leftInp = Input(0);
    assert(leftInp);

    Expr_t *rightInp = Input(1);
    assert(rightInp);

    if( leftInp->ExprType()  == REL_ARG_REF_T && 
        rightInp->ExprType() == REL_ARG_REF_T ) {
        RelArgRef_t *leftRef  = (RelArgRef_t *) leftInp;
        RelArgRef_t *rightRef = (RelArgRef_t *) rightInp;
        
        int inpBnd = leftRef->Binding(inpSchema);
        int outBnd = rightRef->Binding(outSchema);
       
        // Either both should be +ve or both -ve. Otherwise
        // it's not a correlated comparison.
        if (inpBnd*outBnd < 0)
            return;
        if( inpBnd == -1) {
            // Both MUST be -1 (rev schema match rqd).
            assert(outBnd == -1);
            inpBnd = rightRef->Binding(inpSchema);
            outBnd = leftRef->Binding(outSchema);
        }

        if (inpBnd == -1 || outBnd == -1)
            return;

        // Ravi: casting const int* to int* (sorry :))
        int *inOrdArray  = (int *)inpOrder->Order();
        int *outOrdArray = (int *)outOrder->Order();

        for (int i = 0; i < inpOrder->OrderSize(); i++) {
            if (inOrdArray[i] == inpBnd) {
                outOrdArray[i] = outBnd;
                break;
            }
        }
        }
}

/*
void And_t::GetMatchingOrder(Order_t *inpOrder, Schema_t *inpSchema,
                             Order_t *outOrder, Schema_t *outSchema)
{
    Predicate_t *left  = (Predicate_t *) Input(0);
    Predicate_t *right = (Predicate_t *) Input(1);

    left->GetMatchingOrder(inpOrder, inpSchema, outOrder, outSchema);
}

void Or_t::GetMatchingOrder(Order_t *inpOrder, Schema_t *inpSchema,
                            Order_t *outOrder, Schema_t *outSchema)
{
    Predicate_t *left  = (Predicate_t *) Input(0);
    Predicate_t *right = (Predicate_t *) Input(1);

    left->GetMatchingOrder(inpOrder, inpSchema, outOrder, outSchema);
}
*/
