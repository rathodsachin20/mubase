// expression interface

// Ravi:Mod:Commented out the following line to get it compiling 
// Ravi:Mod:with g++ 2.96 on info0.
// Ravi:Mod:extern "C" void bcopy(const void *s1, void *s2, unsigned int n);

#include "DaliParser/sql_parse.h"
#include "DaliParser/exprinterface.h"
#include "Expr.h"

Expr_t* ExprInterface::ExprPostOrder(Expr* InpExpr, Memo_t& memo)
{
    if( InpExpr == NULL )
        return NULL;

    switch(InpExpr->exprType) {
        case Expr::D_VALUE:
            return ExprPostOrder((Value *)InpExpr, memo);
        case Expr::D_LOGICAL_EXPR:
            return ExprPostOrder((LogicalExpr *)InpExpr, memo);
        case Expr::D_ARITH_EXPR:
            return ExprPostOrder((ArithExpr *)InpExpr, memo);
        case Expr::D_AGG_EXPR:
            return ExprPostOrder((AggExpr *)InpExpr, memo);
        case Expr::D_REL_ARG_REF:
            return ExprPostOrder((RelArgRef *)InpExpr, memo);
        default:
            return NULL;
    }
}

// FIX: Some of the operators below are redundant -- can be rewritten in
// terms of the other operators -- though this is going to take more
// space, it might be simpler to analyze --- if we ignore NULL semantics
// here
Expr_t* ExprInterface::ExprPostOrder(LogicalExpr* InpExpr, Memo_t& memo)
{
    Expr_t *left = ExprPostOrder(InpExpr->left, memo);

        if (InpExpr->op == LogicalExpr::D_IN )
        {
                // Project_t * pro = (((InExpr *)InpExpr)->rright)->MkProject_t();
                Equivalence_t *inEq = (((InExpr *)InpExpr)->rright)->PostOrder(memo);
                return  CreateIn((RelArgRef_t *)left, inEq);
        }


    Expr_t *right = ExprPostOrder(InpExpr->right, memo);

    switch( InpExpr->op ) {
        case  LogicalExpr::D_EQ :
            return CreateEq(left, right);
        case  LogicalExpr::D_NE :
            return CreateNEq(left, right);
        case  LogicalExpr::D_LT :
            return CreateLess(left, right);
        case  LogicalExpr::D_LEQ :
            return CreateLessEq(left, right);
        case  LogicalExpr::D_GT :
            return CreateGreater(left, right);
        case  LogicalExpr::D_GEQ :
            return CreateGreaterEq(left, right);
        case  LogicalExpr::D_OR :
            return CreateOr(left, right);
        case  LogicalExpr::D_AND :
            return CreateAnd(left, right);
        case  LogicalExpr::D_NOT :
            return CreateNot(left);
        default:
            break;
    }
    cout << "Invalid Logical Operator Type: " << InpExpr->op << endl;
    return NULL;
}

Expr_t *ExprInterface::CreateAnd(Expr_t *left, Expr_t *right)
{
    assert(left);
    assert(right);

    return new And_t(left, right);
}

Expr_t *ExprInterface::CreateOr(Expr_t *left, Expr_t *right)
{
    assert(left);
    assert(right);

    return new Or_t(left, right);
}

Expr_t *ExprInterface::CreateNot(Expr_t *inp)
{
    assert(inp);

    return new Not_t(inp);
}

Expr_t *ExprInterface::CreateEq(Expr_t *left, Expr_t *right)
{
    assert(left);
    assert(right);

    return new Eq_t(left, right);
}

Expr_t *ExprInterface::CreateLess(Expr_t *left, Expr_t *right)
{
    assert(left);
    assert(right);

    return new Less_t(left, right);
}

Expr_t *ExprInterface::CreateNEq(Expr_t *left, Expr_t *right)
{
    assert(left);
    assert(right);

    return CreateNot(CreateEq(left, right));
}

Expr_t *ExprInterface::CreateLessEq(Expr_t *left, Expr_t *right)
{
    assert(left);
    assert(right);

    return CreateNot(CreateGreater(left, right));
}

Expr_t *ExprInterface::CreateGreater(Expr_t *left, Expr_t *right)
{
    assert(left);
    assert(right);

    return CreateLess(right, left);
}

Expr_t *ExprInterface::CreateGreaterEq(Expr_t *left, Expr_t *right)
{
    assert(left);
    assert(right);

    return CreateNot(CreateLess(left, right));
}

Expr_t *ExprInterface::CreateIn(RelArgRef_t *left, Equivalence_t *right)
{
    assert(left);
    assert(right);

    return new InExpr_t(left, right);
}

Expr_t* ExprInterface::ExprPostOrder(ArithExpr* InpExpr, Memo_t& memo)
{
    Expr_t *left = ExprPostOrder(InpExpr->left, memo);
    Expr_t *right = ExprPostOrder(InpExpr->right, memo);

    switch( InpExpr->op ) {
        case ArithExpr::D_PLUS :
            return new Plus_t(left, right);
        case ArithExpr::D_MINUS :
            return new Minus_t(left, right);
        case ArithExpr::D_MULT :
            return new Mult_t(left, right);
        case ArithExpr::D_DIV :
            return new Div_t(left, right);
    }

    cout << "Invalid Arithmetic Operator Type: " << InpExpr->op << endl;
    return NULL;
}

Expr_t* ExprInterface::ExprPostOrder(AggExpr* InpExpr, Memo_t& memo)
{
    Expr_t *inp = ExprPostOrder(InpExpr->arg, memo);

    switch( InpExpr->op ) {
        case AggExpr::D_MAX :
            return new Max_t(inp);
        case AggExpr::D_MIN :
            return new Min_t(inp);
        case AggExpr::D_SUM :
            return  new Sum_t(inp);
        case AggExpr::D_AVG :
            return new Avg_t(inp);
        case AggExpr::D_COUNT :
            return new Count_t(inp);
        case AggExpr::D_COUNT_DISTINCT :
            return new CountDistinct_t(inp);
        case AggExpr::D_COUNT_STAR :
            return new CountStar_t(inp);
    }

    cout << "Invalid Aggregate Operator Type: " << InpExpr->op << endl;
    return NULL;
}

Expr_t* ExprInterface::ExprPostOrder(RelArgRef* InpExpr, Memo_t&)
{
    return new RelArgRef_t(InpExpr->relRef, InpExpr->attrName);
}

Expr_t* ExprInterface::ExprPostOrder(Value* InpExpr, Memo_t& memo)
{
    switch(InpExpr->type.type) {
        case D_Int:
            return new Int_t(((DaliInt *)InpExpr)->value);
        case D_Float:
            return new Float_t(((DaliFloat *)InpExpr)->value);
        case D_Double:
            return new Double_t(((DaliDouble *)InpExpr)->value);
        case D_String:
            return new String_t(&(((DaliString *)InpExpr)->value),
                                                InpExpr->type.length());
        case D_Truthval:
            return ExprPostOrder((DaliTruthVal *) InpExpr, memo);
        case D_Star:
            return new Star_t;
        case D_Null:
            return new Null_t;
        default:
            break;
    }

    cout << "Invalid Value Type: " << InpExpr->type.type << endl;
    return NULL;
}

Expr_t *ExprInterface::ExprPostOrder(DaliTruthVal *InpExpr, Memo_t&)
{
    switch( InpExpr->value ) {
        case D_FALSE:
            return new False_t();
        case D_TRUE:
            return new True_t();
        default:
            break;
    }

    cout << "Invalid Truth Value Type: " << InpExpr->type.type << endl;
    return NULL;
}
