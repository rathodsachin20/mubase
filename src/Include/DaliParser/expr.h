/* -*- c++ -*- */
class ClauseList;
class TupleDef;
class SQLResult;
class ExprIter;
class Schema_Def;
class GeneralProjectExpr ;

struct SearchTermFlag
{
        bool equal;
        bool greater;
        bool lesser;
};

class Expr
{
  protected:
    void Constructor()
    {
    }
  public:
        enum ExprType {
                D_VALUE         = 1,
                D_LOGICAL_EXPR  = 2,
                D_ARITH_EXPR    = 3,
                D_AGG_EXPR      = 4,
                D_LOCAL_ARG_REF = 5,
                D_REL_ARG_REF   = 6,
                D_TUPLE_CONS    = 7,
                D_MAX_EXPRS     = 8
        };
    ExprType exprType; 
    ArgType type;
    /* Both $exprType$ and $type$ are set either  by the constructors of
           Expr's child classes, or by their 'create' functions. */

    ClauseList *compute_contained_clauses(); 
                /* If $this$ is a conjunction of several expressions, returns
                   a list of those expressions, else returns itself. */
//      Expr *extract_special_clauses(DaliSearchCriteria &retVal, 
//        TupleDef &resultInfo, SearchTermFlag *flagArray);
                /* If $this$ is a conjunction of several expressions, then
                   removes those expressions which are of the form
                   <RelArgRef <relop> constant_value>, and puts them into
                   $retVal$. Then returns what remains of $this$. */
                   
    int print(FILE *file);
    ArgType typeCheck(TupleDef &bindings);
    /* Given the names of visible range variables, their attributes and types,
       this function type checks the expression and returns its type. Any 
           $RelArgRef$s which have a null $relRef$ field are dis-ambiguated. 
           Also sets the $type$ field. */

        Expr *rewrite(TupleDef &bindings_a, ArgSource source_a);
                /* Replaces $RelArgRef$s in $this$ by $LocalArgRef$. The $LocalArgRef$
                   numbers are decided by the position of the $RelArgRef$
                   being replaced, in $bindings_a$. */
        Expr *Copy();
                // returns a copy of this expression
        Expr *Substitute(Expr **exprList_a);
                /* Replaces $LocalArgRef$s of the form <RESULT, i> in $this$, 
                   by freshly allocated copies of $exprList_a[i]$ */
        bool CheckLocalArgBounds(int lowerBound_a, int upperBound_a);
                /* Returns true iff there are no $RelArgRef$s, and all 
                   the $LocalAegRef$s have $source$ == RESULT and 
                   $lowerBound_a <= argNum <= upperBound_a$. */
        void ShiftLocalArgs(int howMuchToShift_a);
                /* Assumes that $CheckLocalArgBounds$ had been called prior to this,
                   and that the return value from that was true. This function
                   adds $howMuchToShift_a$ to all the $argNum$s in the $LocalArgRef$s.
                   User's responsibility to see that the new values are meaningful. */
        ExprIter *createIters(const SQLResult &left_a, const SQLResult &right_a, 
          const SQLResult &result_a);

        Schema_Def* FindAttributes(Schema_Def*);
        static int IsEquivalent(Expr*,Expr*);
        static Expr* Negate(Expr*);
};

class Value : public Expr {
  protected:
    void Constructor()
    {
                Expr::Constructor();
                exprType = D_VALUE;
    }
  public: 
    int print(FILE *file);
        ArgType typeCheck(TupleDef &)
        { return type; }
        ExprIter *createIters(const SQLResult &, const SQLResult &, 
          const SQLResult &);
        void  *evaluate(void *defValue);
                /* Returns a pointer to data which can be inserted into the db.
                   If $type.type$ is D_Null, returns $defValue$. */
        Value *Copy();
        static int IsEquivalent(Value*,Value*);
};

class DaliNull : public Value {
    void Constructor();
  public: 
    static DaliNull *create();
    int print(FILE *file);
};

class DaliStar : public Value {
        void Constructor();
    public: 
        static DaliStar *create();
        int print(FILE *file);
};

/* BEWARE: Do not inherit from this class --- value should be the last
 * member in the class --- a buffer follows it for the string
 *
 * FIX: This is VERY crazy IMHO -- Prasan 
 */
class DaliString : public Value {
        /* Variable size struct, with value starting here */
    /* FIX: We are assuming that $value$ will be the last member to be 
           allocated in each $DaliString$ object by the compiler. */
        void Constructor(const char *value, int len);
  public: 
        char value;
        static DaliString *create(const char *val, int len);
        int print(FILE * file); 

        friend class Value;
};

class DaliInt : public Value {
        void Constructor(int val_a);
  public: 
        int value;
        static DaliInt *create(int val = 0);
        int print(FILE * file); 

        friend class Value;
};

class DaliFloat : public Value {
        void Constructor(float value_a); 
  public: 
        float value;
        static DaliFloat *create(float val = 0);
        int print(FILE * file); 

        friend class Value;
};

class DaliDouble : public Value {
        void Constructor(double value_a);
  public:
        double value;
        static DaliDouble *create(double val = 0);
        int print(FILE * file); 

        friend class Value;
};

class LogicalExpr : public Expr 
{
  public:
        enum LogicalOp 
        {
                D_NO_LOP = 0,
                D_EQ  = 1,
                D_NE  = 2,
                D_LT  = 3,
                D_LEQ = 4,
                D_GT  = 5,
                D_GEQ = 6,
                D_IS  = 7,
                D_IS_NOT = 8,
                D_OR  = 11,
                D_AND = 12,
                D_NOT = 13,
                D_IN = 14                       // Added InExpr
        };
        LogicalOp op;
        Expr * left;
        Expr * right;
  public:
        void Constructor(LogicalOp op_a, Expr *left_a, Expr *right_a)
        {
                Expr::Constructor();
                exprType = D_LOGICAL_EXPR;
                op = op_a;
                left = left_a;
                right = right_a;
        }
        char *OpToString();
  public:
        static LogicalExpr *create(LogicalOp op_a, Expr *left_a,
                                   Expr *right_a = NULL)
        {
                LogicalExpr *v = (LogicalExpr *)new char[sizeof(LogicalExpr)];
                v->Constructor(op_a, left_a, right_a);
                return v;
        }
        LogicalExpr *Copy()
        {
                LogicalExpr *retVal =  LogicalExpr::create(op, left, right);
                retVal->type = type;
                return retVal;
        }
        bool CheckLocalArgBounds(int lowerBound_a, int upperBound_a)
        {
                return (left->CheckLocalArgBounds(lowerBound_a, upperBound_a) &&
                  (!right || right->CheckLocalArgBounds(lowerBound_a, upperBound_a)));
        }
        void ShiftLocalArgs(int howMuchToShift_a)
        {
                left->ShiftLocalArgs(howMuchToShift_a);
                if(right)
                        right->ShiftLocalArgs(howMuchToShift_a);
        }
        ArgType typeCheck(TupleDef &bindings);
        int print(FILE * file); 
        Expr *rewrite(TupleDef &bindings_a, ArgSource source_a);
        ExprIter *createIters(const SQLResult &left_a, const SQLResult &right_a, 
          const SQLResult &result_a);
        ClauseList *compute_contained_clauses();
//      Expr *extract_special_clauses(DaliSearchCriteria &retVal, 
//        TupleDef &resultInfo, SearchTermFlag *flagArray);
        LogicalExpr *Substitute(Expr **exprList);

        Schema_Def* FindAttributes(Schema_Def*);
        static int IsEquivalent(LogicalExpr*,LogicalExpr*);
        static Expr* Negate(LogicalExpr*);
};      

class InExpr : public LogicalExpr
{
 
public:
        GeneralProjectExpr  *rright ;


    InExpr( Expr *_left , GeneralProjectExpr *_right)

        {
                ((LogicalExpr *)this)->Constructor(D_IN, _left, NULL);

                        rright = _right;

                        left = _left ;

                exprType = D_LOGICAL_EXPR;

                        op = D_IN;

        }

        void  Constructor ( Expr *_left , GeneralProjectExpr *_right)
        {
                        LogicalExpr::create(D_IN, _left, NULL);

                        rright = _right;
        }

        static InExpr *create ( Expr *_left , GeneralProjectExpr  *_right)

        {
                InExpr *v = new InExpr(_left, _right);




                        return(v);

        }

        InExpr *Copy()
        {
                Expr *_left = ((LogicalExpr *)this)->left;

                InExpr *retVal =  InExpr::create( _left, rright); 

                return retVal;
        }

        ArgType typeCheck(TupleDef &bindings);

        int print(FILE * file); 
};

enum D_TruthVal {
     D_FALSE = 0,
     D_UNKNOWN = 1,
     D_TRUE  = 2
};

// FIX: This enum is exposed outside only because of grammar.y

class DaliTruthVal : public Value {
        void Constructor(D_TruthVal value_a);
  public:
        D_TruthVal value;
        enum { DB_TRUE = 0x01, DB_FALSE = 0x00};
        static DaliTruthVal *create(D_TruthVal val = D_UNKNOWN);
        int print(FILE * file); 
        operator bool() { return (value == D_TRUE); }

        friend class Value;
};

class ArithExpr : public Expr 
{
  public:
        enum ArithOp 
        {
                D_PLUS,
                D_MINUS,
                D_MULT,
                D_DIV
        };
    ArithOp op;
    Expr *left;
    Expr *right;
  private:
        void Constructor(ArithOp op_a, Expr *left_a, Expr *right_a)
        {
                Expr::Constructor();
                exprType = D_ARITH_EXPR;
                op = op_a;
                left = left_a;
                right = right_a;
        }
        char *OpToString();
  public:
    static ArithExpr *create(ArithOp op_a, Expr *left_a, Expr *right_a)
        {
                ArithExpr *v = (ArithExpr *)new char[sizeof(ArithExpr)];
                v->Constructor(op_a, left_a, right_a);
                return v;
        }
        ArithExpr *Copy()
        {
                ArithExpr *retVal = ArithExpr::create(op, left, right);
                retVal->type = type;
                return retVal;
        }
        bool CheckLocalArgBounds(int lowerBound_a, int upperBound_a)
        {
                return (left->CheckLocalArgBounds(lowerBound_a, upperBound_a) &&
                  right->CheckLocalArgBounds(lowerBound_a, upperBound_a));
        }
        void ShiftLocalArgs(int howMuchToShift_a)
        {
                left->ShiftLocalArgs(howMuchToShift_a);
                right->ShiftLocalArgs(howMuchToShift_a);
        }
    ArgType typeCheck(TupleDef &bindings);
    int print(FILE *file); 
        Expr *rewrite(TupleDef &bindings_a, ArgSource source_a);
        ExprIter *createIters(const SQLResult &left_a, const SQLResult &right_a, 
          const SQLResult &result_a);
        ArithExpr *Substitute(Expr **exprList);
       
        Schema_Def* FindAttributes(Schema_Def*);
        static int IsEquivalent(ArithExpr*,ArithExpr*);
};

class AggExpr : public Expr 
{
  public:
        enum AggFunc 
        {
                D_MAX = 1,
                D_MIN = 2,
                D_SUM = 3,
                D_AVG = 4,
                D_COUNT = 5,
                D_COUNT_DISTINCT = 6,
                D_COUNT_STAR = 7
        };
    AggFunc op;
    Expr *arg;

  private:
        void Constructor(AggFunc op_a, Expr *arg_a)
        {
                Expr::Constructor();
                exprType = D_AGG_EXPR;
                op = op_a;
                arg = arg_a;
        }
        char *OpToString();
  public:
    static AggExpr *create(AggFunc op_a, Expr *arg_a = NULL)
        {
                AggExpr *v = (AggExpr *)new char[sizeof(AggExpr)];
                v->Constructor(op_a, arg_a);
                return v;
        }
        AggExpr *Copy()
        {
                AggExpr *retVal = AggExpr::create(op, arg);
                retVal->type = type;
                return retVal;
        }
        bool CheckLocalArgBounds(int lowerBound_a, int upperBound_a)
        {
                return (!arg || arg->CheckLocalArgBounds(lowerBound_a, upperBound_a));
        }
        void ShiftLocalArgs(int howMuchToShift_a)
        {
                if(arg)
                        arg->ShiftLocalArgs(howMuchToShift_a);
        }
    ArgType typeCheck(TupleDef &bindings_a);
    int print(FILE * file);
        Expr *rewrite(TupleDef &bindings_a, ArgSource source_a);
        AggExpr *Substitute(Expr **exprList);

        Schema_Def* FindAttributes(Schema_Def*);
        static int IsEquivalent(AggExpr*,AggExpr*);
};      

#define D_INIT_TUPLE_ATTRS 5

class Tuple: public Expr {
    void Constructor(Expr *expr_a);
  public:
    int numAttrs;
    Expr *attrs[D_INIT_TUPLE_ATTRS];
    /* Variable length array */
    /* Allocated to size D_INIT_TUPLE_ATTRS initially and expanded in 
       multiples of D_INIT_TUPLE_ATTRS 
       */
    
    static Tuple* create(Expr *exp); 
    static Tuple* appendExpr(Tuple *tup, Expr *exp); 
    int print(FILE * file); 
        Expr *rewrite(TupleDef &bindings_a, ArgSource source_a);
};

/** FIX:
   The following two #defines are also defined in Dali (dali_rel_mgr.h)
   under a worse name.
   These should supercede the dali definitions eventually
**/

const int DALIMAXRELNAME = 100;
const int DALIMAXATTRNAME = 100;

class ArgDef {
  public:
    char rangeVariable[100]; /* This member is used only during pre-processing
                         of the expression tree. I.e.  it is used only in  the
                         'resultInfo' field of 'RelExpr'. */
    char argName[100];
    ArgType  argType;
        void *defaultValue;
                /* FIX: This field is probably not properly set in some places in 
                   the code */
    
    /* FIX: add later:
       Nullable ...
       attribute constraints, ...
       int argNum;
       int  offset;
       */
    
    ArgDef();
    ArgDef(const char *name, ArgType atype);
    int print(FILE *file);
};

/** FIX:
   TupleDef in Dali is called DaliTableSchema.
   This defn. should supercede the dali definitions eventually,
   but under the name DaliTableSchema.
**/

class TupleDef {
  public: 
        int numArgs;
        ArgDef *args; /* Pointer to variable sized array of ArgDefs */

        /* FIX:  add later:  constraints, NULL value */

        static TupleDef * create();
        int addAttr(ArgDef arg);
        int addAttr(const char *attrName, ArgType atype);
        int print(FILE *file);
        void operator = (TupleDef &); // returns 'void' for now.
        void operator += (TupleDef &); // returns 'void' for now.
};

class ArgRef : public Expr 
{
  public:
        void Constructor()
        {
                Expr::Constructor();
                return;
        }
};

class LocalArgRef : public ArgRef  
{
        ArgSource source;
        int       argNum;
        void Constructor(ArgSource source_a, int argNum_a, ArgType type_a) 
        { 
                ((ArgRef *)this)->Constructor();
                exprType = D_LOCAL_ARG_REF; 
                source = source_a;
                argNum = argNum_a;
                type = type_a;
                return;
        }
        friend class InsertTable;
        friend class UpdateTable;
  public:
        static LocalArgRef* create(ArgSource src_a, int arg_a, ArgType type_a)
        {
                LocalArgRef *retVal = (LocalArgRef *)new char[sizeof(LocalArgRef)];
                retVal->Constructor(src_a, arg_a, type_a);
                return retVal;
        }
        int print(FILE *file)
        {
                fprintf(file, "%s.%d", argSourceToString(source), argNum);
                return 1;
        }
        LocalArgRef *Copy() 
        {
                return LocalArgRef::create(source, argNum, type);
        }
        Expr *Substitute(Expr **)
        { return NULL; }
        /***
        Expr *Substitute(Expr **exprList)
        {
                return (source != RESULT) ? NULL : exprList[argNum]->Copy();

                // The call to Copy() is rather dirty. Ideally
                // $exprList$ itself should have had fresh copies of the
                // expressions to substitute.  But that would mean
                // unnecessary wastage of memory because at the point of
                // the call $expr_node->Substitute(exprList)$, we don't
                // know which of the expressions in the $exprList$
                // passed are actually referred to by $expr_node$.
        }
        ***/
        bool CheckLocalArgBounds(int lowerBound_a, int upperBound_a)
        {
                return ((source == RESULT) && (argNum >= lowerBound_a) &&
                  (argNum <= upperBound_a));
        }
        void ShiftLocalArgs(int howMuchToShift_a) 
        { 
                argNum += howMuchToShift_a;
        }
        ExprIter *createIters(const SQLResult &left_a, const SQLResult &right_a, 
          const SQLResult &result_a);
};

class RelArgRef : public ArgRef 
{
public:
    char   relRef[DALIMAXRELNAME];
    char   attrName[DALIMAXATTRNAME];
        int GetMatchPos(TupleDef &bindings_a);
        /* Returns the position in $bindings_a$ at which $relRef$ and
           $attrName$ match. Returns -1 if no match or multiple matches. */
    void Constructor(const char *rel_a, const char *attr_a); 
    static RelArgRef* create(const char *rel, const char *attrName);
    int print(FILE *file);
    ArgType typeCheck(TupleDef &bindings);
        Expr *rewrite(TupleDef &bindings_a, ArgSource source_a);
        RelArgRef *Copy() 
        {
                RelArgRef *retVal = RelArgRef::create(relRef, attrName);
                retVal->type = type;
                return retVal;
        }

        friend class ProjectElem;

        Schema_Def* FindAttributes(Schema_Def*);
        static int IsEquivalent(RelArgRef*,RelArgRef*);
};

//DaliAttrType sQLToDaliType(PrimType type_a);
//PrimType daliToSQLType(DaliAttrType type_a);
