#include "Expr.h"

class ExprInterface {
                        ///// this class is used for transforming the data types
                        ///// used in the input query expression to a type 
                        ///// needed by the the optimization algorithm
                        


public :
                        ////// the following functions transforms the input
                        ////// logical or arithmatic and aggregate expressions
                        ////// into the forms that are used by the optimization
                        ////// algorithm

 static Expr_t* ExprPostOrder(Expr*, Memo_t&);
 static Expr_t* ExprPostOrder(LogicalExpr*, Memo_t&);
 static Expr_t* ExprPostOrder(ArithExpr*, Memo_t&);
 static Expr_t* ExprPostOrder(AggExpr*, Memo_t&);
 static Expr_t* ExprPostOrder(Value*, Memo_t&);
 static Expr_t* ExprPostOrder(RelArgRef*, Memo_t&);
 static Expr_t* ExprPostOrder(DaliTruthVal*, Memo_t&);

 static Expr_t *CreateAnd(Expr_t *left, Expr_t *right);
 static Expr_t *CreateOr(Expr_t *left, Expr_t *right);
 static Expr_t *CreateNot(Expr_t *inp);
 static Expr_t *CreateEq(Expr_t *left, Expr_t *right);
 static Expr_t *CreateLess(Expr_t *left, Expr_t *right);
 static Expr_t *CreateNEq(Expr_t *left, Expr_t *right);
 static Expr_t *CreateLessEq(Expr_t *left, Expr_t *right);
 static Expr_t *CreateGreater(Expr_t *left, Expr_t *right);
 static Expr_t *CreateGreaterEq(Expr_t *left, Expr_t *right);
 static Expr_t *CreateIn(RelArgRef_t *left, Equivalence_t *right);

};

