/* -*- c++ -*- */
#include "ucbproto.h"
//#include "dali.h" ARU
//#include "dali_rel_mgr.h" ARU

#include "sql_defs.h"
#include "sql_error.h"
#include "prim_types.h"
#include "expr.h"
#include "rel_alg.h"

/**************************************************************************/

/** 
  FIX: All the create functions below will later change to return DaliPtr's
**/

int
Expr::print(FILE *file) {
    switch (exprType) {
        case D_VALUE:         return ((Value *)this)->print(file);
        case D_ARITH_EXPR:    return ((ArithExpr *)this)->print(file);
        case D_LOGICAL_EXPR:  return ((LogicalExpr *)this)->print(file);
        case D_LOCAL_ARG_REF: return ((LocalArgRef *)this)->print(file);
        case D_REL_ARG_REF:   return ((RelArgRef *)this)->print(file);
        case D_AGG_EXPR:      return ((AggExpr *)this)->print(file);
        case D_TUPLE_CONS:    return ((Tuple *)this)->print(file);
        default:        fprintf(stderr, "BAD EXPR TYPE %d in Stmt No: %d\n", exprType,
          stmtCount);
    }
    return -1;
}

ArgType
Expr::typeCheck(TupleDef &bindings_a)
{
    switch(exprType)
    {
      case D_VALUE:
        return ((Value *)this)->typeCheck(bindings_a);
      case D_ARITH_EXPR: 
        return ((ArithExpr *)this)->typeCheck(bindings_a);
      case D_LOGICAL_EXPR:
        return ((LogicalExpr *)this)->typeCheck(bindings_a);
      case D_REL_ARG_REF:
        return ((RelArgRef *)this)->typeCheck(bindings_a);
      case D_AGG_EXPR:
        return ((AggExpr *)this)->typeCheck(bindings_a);
      default:
        return ArgType(D_UnknownType);
        /* We will  not  see LocalArgRefs now  at  all. They are created  only
           after  type  checking is  over. And   Tuple's do not  have a  valid
           ArgType.  */
    }
}

Expr *
Expr::rewrite(TupleDef &bindings_a, ArgSource source_a)
{
        switch(exprType)
        {
          case D_VALUE:
                return this;
          case D_ARITH_EXPR:
                return ((ArithExpr *)this)->rewrite(bindings_a, source_a);
          case D_LOGICAL_EXPR:
                return ((LogicalExpr *)this)->rewrite(bindings_a, source_a);
          case D_REL_ARG_REF:
                return ((RelArgRef *)this)->rewrite(bindings_a, source_a);
          case D_AGG_EXPR:
                return ((AggExpr *)this)->rewrite(bindings_a, source_a);
          default:
                fprintf(stderr, "In Expr::rewrite(). Illegal exprType\n");
                DALI_R_ASSERT(0);
        }
}

Expr *
Expr::Copy()
{
        switch(exprType)
        {
          case D_VALUE: return ((Value *)this)->Copy();
          case D_LOGICAL_EXPR:  return ((LogicalExpr *)this)->Copy();
          case D_ARITH_EXPR: return ((ArithExpr *)this)->Copy();
          case D_AGG_EXPR:  return ((AggExpr *)this)->Copy();
          case D_LOCAL_ARG_REF: return ((LocalArgRef *)this)->Copy();
          case D_REL_ARG_REF: return ((RelArgRef *)this)->Copy();
          case D_TUPLE_CONS: return NULL; // FIX: this will have to done later
          default: return NULL;
        }
}

Expr *
Expr::Substitute(Expr **exprList)
{
        switch(exprType)
        {
          case D_VALUE: return this;
          case D_LOGICAL_EXPR:  return ((LogicalExpr *)this)->Substitute(exprList);
          case D_ARITH_EXPR: return ((ArithExpr *)this)->Substitute(exprList);
          case D_AGG_EXPR:  return ((AggExpr *)this)->Substitute(exprList);
          case D_LOCAL_ARG_REF: return ((LocalArgRef *)this)->Substitute(exprList);
          case D_REL_ARG_REF: return NULL;
                /* This is intentional. $Substitute$ must not see any $RelArgRef$s */
          case D_TUPLE_CONS: return NULL; // FIX: this will have to done later
          default: return NULL;
        }
}

bool 
Expr::CheckLocalArgBounds(int lowerBound_a, int upperBound_a)
{
        switch(exprType)
        {
          case D_VALUE: return true;
          case D_LOGICAL_EXPR:  return ((LogicalExpr *)this)->
            CheckLocalArgBounds(lowerBound_a, upperBound_a);
          case D_ARITH_EXPR: return ((ArithExpr *)this)->
            CheckLocalArgBounds(lowerBound_a, upperBound_a);
          case D_AGG_EXPR:  return ((AggExpr *)this)->
            CheckLocalArgBounds(lowerBound_a, upperBound_a);
          case D_LOCAL_ARG_REF: return ((LocalArgRef *)this)->
            CheckLocalArgBounds(lowerBound_a, upperBound_a);
          case D_REL_ARG_REF: return false;
                /* $CheckLocalArgBounds$ must not see any $RelArgRef$s */
          case D_TUPLE_CONS: return false; // FIX: this will have to done later
          default: return false;
        }
}

void 
Expr::ShiftLocalArgs(int howMuchToShift_a)
{
        switch(exprType)
        {
          case D_VALUE: return;
          case D_LOGICAL_EXPR:  
                ((LogicalExpr *)this)->ShiftLocalArgs(howMuchToShift_a);
                return;
          case D_ARITH_EXPR: 
                ((ArithExpr *)this)->ShiftLocalArgs(howMuchToShift_a);
                return;
          case D_AGG_EXPR:  
                ((AggExpr *)this)->ShiftLocalArgs(howMuchToShift_a);
                return;
          case D_LOCAL_ARG_REF: 
                ((LocalArgRef *)this)->ShiftLocalArgs(howMuchToShift_a);
                return;
          case D_REL_ARG_REF: return;
                /* $ShiftLocalArgs$ must not see any $RelArgRef$s */
          case D_TUPLE_CONS: return; // FIX: this will have to done later
          default: return;
        }
}

/***********************/

void  *
Value::evaluate(void *defValue)
{
    switch (type.type) 
        {
          case D_Null:          return defValue;
          case D_Int:           return &(((DaliInt *)this)->value); 
          case D_Float:         return &(((DaliFloat *)this)->value);
          case D_Double:        return &(((DaliDouble *)this)->value);
          case D_String:        return &(((DaliString *)this)->value);
          case D_Truthval:      
                switch(((DaliTruthVal *)this)->value)
                {
                  case D_TRUE:  return new bool(DaliTruthVal::DB_TRUE);
                  case D_FALSE: return new bool(DaliTruthVal::DB_FALSE);
                  default: SQLErrE("Value::evaluate: illegal boolean value", -1)
                }
          default:
                SQLErrE("Trying to evaluate unknown type of Value", -1)
    }
    return NULL;
}

int
Value::print(FILE *file) {
    switch (type.type) {
        case D_Null:         return ((DaliNull *)this)->print(file);
        case D_Star:         return ((DaliStar *)this)->print(file);
        case D_Int:          return ((DaliInt *)this)->print(file); 
        case D_Float:        return ((DaliFloat *)this)->print(file);
        case D_Double:       return ((DaliDouble *)this)->print(file);
        case D_String:       return ((DaliString *)this)->print(file);
        case D_Truthval:     return ((DaliTruthVal *)this)->print(file);
        default:        fprintf(stderr, "BAD VALUE TYPE %d in Stmt No:%d\n", type.type
          , stmtCount);
    }
    return -1;
}

Value *
Value::Copy()
{
        switch(type.type)
        {
          case D_Int: return DaliInt::create(((DaliInt *)this)->value);
          case D_Float: return DaliFloat::create(((DaliFloat *)this)->value);
          case D_Double: return DaliDouble::create(((DaliDouble *)this)->value);
          case D_String: 
                return DaliString::create(&(((DaliString *)this)->value), 
                  type.length());
          case D_Truthval:
                return DaliTruthVal::create(((DaliTruthVal *)this)->value);
          case D_Star: return DaliStar::create();
          case D_Null: return DaliNull::create();
          default: break;
        }
    return NULL;
}

/*************/

void DaliNull::Constructor()
{
    Value::Constructor();
    type = ArgType(D_Null);
};

DaliNull *
DaliNull::create()
{
    DaliNull *v = (DaliNull *) new char[sizeof(DaliNull)];
    v->Constructor();
    return v;
}

int
DaliNull::print(FILE *file)
{
    return fprintf(file, " NULL ");
}
/*************/

void DaliStar::Constructor() 
{
    Value::Constructor();
    type = ArgType(D_Star);
};

DaliStar *
DaliStar::create() {
    DaliStar *v = (DaliStar *) new char[sizeof(DaliStar)];
    v->Constructor();
    return v;
}

int
DaliStar::print(FILE *file) {
    return fprintf(file, " * ");
}
/*************/

void DaliString::Constructor(const char *val, int len)
{
    Value::Constructor();
    type = ArgType(D_String, len);
        if(val) {
                strncpy(&value, val, len - 1);
                (&value)[len - 1] = '\0';
        } else (&value)[0] = '\0';
    /* FIX:  Perhaps string should be blank padded according to SQL? */
        return;
};

DaliString *
DaliString::create(const char *val, int len) {
    DaliString *v = (DaliString *) new char[sizeof(DaliString)+len];
    v->Constructor(val, len);
    return v;
}

int
DaliString::print(FILE *file) {
    return fprintf(file, "%s", &value);
}
/*************/

void DaliInt::Constructor(int val_a)
{
    Value::Constructor();
    type = ArgType(D_Int);
        value = val_a;
        return;
};

DaliInt *
DaliInt::create(int val_a) 
{
    DaliInt *v =  (DaliInt *) new char[sizeof(DaliInt)];
    v->Constructor(val_a);
    return v;
}

int
DaliInt::print(FILE *file) {
    return fprintf(file, "%d", value);
}
/*************/

void DaliFloat::Constructor(float value_a)
{
    Value::Constructor();
    type = ArgType(D_Float); 
        value = value_a;
        return;
};

DaliFloat *
DaliFloat::create(float value_a) 
{
    DaliFloat *v =  (DaliFloat *)new char[sizeof(DaliFloat)];
    v->Constructor(value_a);
    return v;
}

int DaliFloat::print(FILE *file) {
    return fprintf(file, "%g", value);
}
/*************/

void DaliDouble::Constructor(double value_a)
{
        Value::Constructor();
    type = ArgType(D_Double);
        value = value_a;
        return;
};

DaliDouble *
DaliDouble::create(double value_a) 
{
    DaliDouble *v =  (DaliDouble *)new char[sizeof(DaliDouble)];
        v->Constructor(value_a);
    return v;
}

int
DaliDouble::print(FILE *file)
{
    return fprintf(file, "%g", value);
}
/*************/

void DaliTruthVal::Constructor(D_TruthVal value_a)
{
    Value::Constructor();
    type = ArgType(D_Truthval);
    value = value_a;
};

DaliTruthVal *
DaliTruthVal::create(D_TruthVal value_a)
{
    DaliTruthVal *v =  (DaliTruthVal *)new char[sizeof(DaliTruthVal)];
        v->Constructor(value_a);
    return v;
}

int
DaliTruthVal::print(FILE *file) {
    switch(value) {
       case D_FALSE: { return fprintf(file, "FALSE");}
       case D_TRUE:  { return fprintf(file, "TRUE"); }
       case D_UNKNOWN: { return fprintf(file, "UNKNOWN"); }
    }
    return 0;
}

/*************/

char * 
ArithExpr::OpToString() 
{
    switch(op) 
        {
        case D_PLUS:    return "+";
        case D_MINUS:   return "-";
        case D_MULT:    return "*";
        case D_DIV:             return "/";
    }
    return "BAD_ARITH_OP";
}

int
ArithExpr::print(FILE *file)
{
    if (!right)
    { 
                fprintf(file, "%s(", OpToString());
                left->print(file);
                fprintf(file, ")");
    }
    else
    {
                fprintf(file, "(");
                left->print(file);
                fprintf(file, "%s", OpToString());
                right->print(file);
                fprintf(file, ")");
    }
    return 0;
}

ArgType
ArithExpr::typeCheck(TupleDef &bindings_a)
{
    left->typeCheck(bindings_a);
        right->typeCheck(bindings_a);
        return (type = ArgType::MixedModeArithResult(left->type,
          right->type));
}

Expr *
ArithExpr::rewrite(TupleDef &bindings_a, ArgSource source_a)
{
        Expr *retVal;
        if((retVal = left->rewrite(bindings_a, source_a)) != left)
                left = retVal;
                // FIX: What will happen to the old $left$? It should be destroyed
        if((retVal = right->rewrite(bindings_a, source_a)) != right)
                right = retVal;
        return this;
}

ArithExpr *
ArithExpr::Substitute(Expr **exprList)
{
        bool success = true;
        Expr *newLhs;
        if(((newLhs = left->Substitute(exprList)) != left) && 
          (success = (success && (newLhs != NULL))))
        {
                delete left;
                left = newLhs;
        }
        Expr *newRhs;
        if(((newRhs = right->Substitute(exprList)) != right) &&
          (success = (success && (newRhs != NULL))))
        {
                delete right;
                right = newRhs;
        }

        if( success )
            return this;
        return NULL;
}

/*************/

char *
LogicalExpr::OpToString() 
{
    switch(op) 
        {
        case D_EQ:              return "==";
        case D_NE:              return "!=";
        case D_LT:              return "<";
        case D_LEQ:             return "<=";
        case D_GT:              return ">";
        case D_GEQ:             return ">=";
        case D_IS:              return " IS ";
        case D_IS_NOT:  return " IS NOT ";
        case D_AND:             return " AND ";
        case D_OR:              return " OR ";
        case D_NOT:             return " NOT ";
        case D_IN:              return "-IN->";
                                                
                                                
        default:                break;
    }
    return "BAD_LOGICAL_OP";
}

int
LogicalExpr::print(FILE *file) 
{
        if ( op == D_IN )               // Added For InExpr
        {
                return(((InExpr *)this)->print(file));

        }

        else 
        {
                
    if (!right) 
        {
            /* FIX: This should be refined to handle postfix notation for some
                   comparison operations. */
                fprintf(file, "%s(", OpToString());
                left->print(file);
                fprintf(file, ")");
    }
    else 
        {
                fprintf(file, "(");
                left->print(file);
                fprintf(file, "%s", OpToString());
                right->print(file);
                fprintf(file, ")");
    }

        }

    return 0;
}

ArgType
LogicalExpr::typeCheck(TupleDef &bindings_a)
{
        if(((left->typeCheck(bindings_a)).type == D_UnknownType) ||
       ((right != NULL) && ((right->typeCheck(bindings_a)).type ==
                            D_UnknownType)))
                return (type = ArgType(D_UnknownType));

    switch(left->type.type)
    {
      case D_Tuple: case D_Star: 
                return (type = ArgType(D_UnknownType));
     default: break;
    }

    if(right != NULL)
        switch(right->type.type)
        {
          case D_Tuple: case D_Star: 
            return (type = ArgType(D_UnknownType));
         default: break;
        }
    
    switch(op)
    {
      case D_IS: case D_IS_NOT:
                /* FIX: This case is not handled properly as yet. For example,
                   if $right$ were of type $D_Null$, shouldn't $left$ necessarily
                   be a $RelArgRef$? */
                if(right->type.type != D_Null)
                {
                        if((left->type.type != D_Truthval) || (right->type.type !=
                                                           D_Truthval))
                                return (type = ArgType(D_UnknownType));
                }
                else
                        return (type = ArgType(D_Truthval));

      case D_AND: case D_OR:
                if((left->type.type != D_Truthval) || (right->type.type != D_Truthval))
                        return (type = ArgType(D_UnknownType));
                else
                        return (type = ArgType(D_Truthval));

      case D_NOT:
                if(left->type.type != D_Truthval)
                        return (type = ArgType(D_UnknownType));
                else
                        return (type = ArgType(D_Truthval));
                
      case D_EQ: case D_NE: case  D_LT: case  D_LEQ:
      case D_GT: case D_GEQ:
                if(left->type.IsNumericType() &&
                   right->type.IsNumericType())
                                return (type = ArgType(D_Truthval));
                else if(left->type.IsStringType() &&
                                right->type.IsStringType())
                                return (type = ArgType(D_Truthval));
                else
                        return (type = ArgType(D_UnknownType));
      default: break;
    }
    return (type = ArgType(D_UnknownType));
}

Expr *
LogicalExpr::rewrite(TupleDef &bindings_a, ArgSource source_a)
{
        Expr *retVal;
        if((retVal = left->rewrite(bindings_a, source_a)) != left)
                left = retVal;
                // What will happen to the old $left$? It should be destroyed
        if(right)
                if((retVal = right->rewrite(bindings_a, source_a)) != right)
                        right = retVal;
        return this;
}

LogicalExpr *
LogicalExpr::Substitute(Expr **exprList)
{
        bool success = true;
        Expr *newLhs;
        if(((newLhs = left->Substitute(exprList)) != left) && 
          (success = (success && (newLhs != NULL))))
        {
                delete left;
                left = newLhs;
        }
        Expr *newRhs;
        if(right && ((newRhs = right->Substitute(exprList)) != right) &&
          (success = (success && (newRhs != NULL))))
        {
                delete right;
                right = newRhs;
        }

        if( success )
            return this;
        return NULL;
}
/********************/
//      InExpr Class
//

int 
InExpr::print(FILE * file)
{
                
                fprintf(file, "(");

                
                left->print(file);
                fprintf(file,"%s","-IN->");
                                                
                fprintf(file, "%s","");
                rright->print(file);
                fprintf(file, ")");

                return 0;
}

ArgType 
InExpr::typeCheck(TupleDef &bindings)
{
        ProjectElem *p = rright->GetProjList();

                if ( p == NULL  )       // NULL is returned by function GetProjList
                                                        // if number of attribute projected is 
                                                        // more then one
                                                        //

                                return (type = ArgType(D_UnknownType));         

                if (((p->projectionExpr)->typeCheck(bindings)).type ==

                                                                        (left->typeCheck(bindings)).type ) 

                                return (type = ArgType(D_Truthval));            

        else
                                return (type = ArgType(D_UnknownType));         

}

/*************/

char *
AggExpr::OpToString() 
{
    switch (op) 
        {
        case D_MAX:             return "MAX";
        case D_MIN:             return "MIN";
        case D_SUM:             return "SUM";
        case D_AVG:             return "AVG";
        case D_COUNT:   return "COUNT";
        case D_COUNT_DISTINCT:  return "COUNT DISTINCT";
        case D_COUNT_STAR:              return "COUNT ( * )";
    }
    return "BAD_AGG_EXPR";
};

int
AggExpr::print(FILE *file) 
{
    fprintf(file, "%s", OpToString());
    if (arg)
    {   
                fprintf(file, "(");
                arg->print(file);
                fprintf(file, ")");
    }
        return 0;
}

ArgType
AggExpr::typeCheck(TupleDef &bindings_a)
{
    if((arg->typeCheck(bindings_a)).type == D_UnknownType)
        return (type = ArgType(D_UnknownType));
    switch(op)
    {
      case D_SUM:
                if(arg->type.IsNumericType())
                        return (type = arg->type);
                else
                        return (type = ArgType(D_UnknownType));
      case D_AVG:
                if(arg->type.IsNumericType())
                        return (type = ArgType(D_Double));
                else
                        return (type = ArgType(D_UnknownType));
      case D_MIN: case D_MAX:
                if(arg->type.IsNumericType() || 
                   arg->type.IsStringType())
                        return (type = arg->type);
                else
                        return (type = ArgType(D_UnknownType));
      default:
                /* The COUNT family */
                return (type = ArgType(D_Int));
    }
}
        
Expr * 
AggExpr::rewrite(TupleDef &bindings_a, ArgSource source_a)
{
        Expr *newExpr;
        if(arg && ((newExpr = arg->rewrite(bindings_a, source_a)) != arg))
                arg = newExpr;
        return this;
}

AggExpr *
AggExpr::Substitute(Expr **exprList)
{
        Expr *newExpr;
        if(arg && ((newExpr = arg->Substitute(exprList)) != arg))
        {
                delete arg;
                arg = newExpr;
                if( newExpr )
                    return this;
                return NULL;
        }
        return this;
}

/*************/

void Tuple::Constructor(Expr *expr_a)
{
        Expr::Constructor();
        exprType = D_TUPLE_CONS;
    numAttrs = 1;
    /* FIX: Do some type checking here */
    attrs[0] = expr_a;
        return;
}

Tuple *
Tuple::create(Expr *expr_a) 
{
    Tuple *v = (Tuple *)new char[sizeof(Tuple)];
        v->Constructor(expr_a);
    return v;
}

#define D_INIT_TUPLE_ATTRS 5

Tuple *
Tuple::appendExpr(Tuple *tuple, Expr *expr)
{
    Tuple *v;
    if(tuple->numAttrs != 0 && (tuple->numAttrs%D_INIT_TUPLE_ATTRS)==0) {
        /* Need to expand */ 
        v = (Tuple *)new char[sizeof(Tuple)+
                (tuple->numAttrs/D_INIT_TUPLE_ATTRS)*sizeof(tuple->attrs)];
        //Ravi:Mod:Changed the below statement to the above one. 
        //Ravi:Mod:Member attrs in Tuple is non-static, a reference to it 
        //Ravi:Mod:using sizeof(Tuple::attrs) gives compilation error with 
        //Ravi:Mod:g++ 2.96.
        //v = (Tuple *)new char[sizeof(Tuple)+
        //      (tuple->numAttrs/D_INIT_TUPLE_ATTRS)*sizeof(Tuple::attrs)];
        *v = *tuple;
        for(int i=0; i < tuple->numAttrs; i++) {
           v->attrs[i] = tuple->attrs[i];
        }
        delete tuple;
    }
    else v = tuple;
    v->attrs[v->numAttrs] = expr;
    v->numAttrs++;
    return v;
}

int 
Tuple::print(FILE *file) {
    fprintf(file, "(");
    for(int i=0; i < numAttrs; i++) {
        if(i>0) {
            fprintf(file, ", ");
        }
        attrs[i]->print(file);
    }
    fprintf(file, ")");
    return 0;
}

Expr * 
Tuple::rewrite(TupleDef &bindings_a, ArgSource source_a)
{
        for(int i = 0; i < numAttrs; ++i)
        {
                Expr *retVal;
                if((retVal = attrs[i]->rewrite(bindings_a, source_a)) != attrs[i])
                        attrs[i] = retVal;
        }
        return this;
}

/*************/

void *
ArgType::ValueToPtr(ArgType &lhsType, Value *value_a, 
  void *defaultValue)
{
        void *lhs = (void *)new char[lhsType.length()];
        void *rhs = value_a->evaluate(defaultValue);
        PtrToPtr(lhs, lhsType, rhs, value_a->type);
        return lhs;

}

