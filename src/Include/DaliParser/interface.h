extern enum PrimType;
extern enum D_TruthVal;

class Interface {
                        ///// this class is used for transforming the data types
                        ///// used in the input query expression to a type 
                        ///// needed by the the optimization algorithm
                        


public :
 Interface(){};
 ~Interface(){};
                        ////// the following functions transforms the input
                        ////// logical or arithmatic and aggregate expressions
                        ////// into the forms that are used by the optimization
                        ////// algorithm

 static OExpr* ExprPostorder(Expr*);
 static OExpr* ExprPostorder(LogicalExpr*);
 static OExpr* ExprPostorder(ArithExpr*);
 static OExpr* ExprPostorder(AggExpr*);
 static OExpr* ExprPostorder(Value*);
 static OExpr* ExprPostorder(RelArgRef*);

                        ///// the following functions are used for getting
                        /////  types used by the optimizer corresponding
                        ///// to the types used by the input query expression

 static OExprType GetExprType(Expr::ExprType);
 static O_BoolVal GetTruthVal(D_TruthVal);
 static OLogicalOp GetLogicalOp(LogicalExpr::LogicalOp);
 static OArithOp GetArithOp(ArithExpr::ArithOp);
 static OAggFunc GetAggFunc(AggExpr::AggFunc);
 static OPrimType GetPrimType(PrimType);
 static OAttrType GetOAttrType(DaliAttrType);

};



