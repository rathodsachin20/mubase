/* -*- c++ -*-  */
#ifndef __REL_ALG_H__
#define __REL_ALG_H__

#include "ucbproto.h"
#include "list.h"
//#include "dali_rel_mgr.h"
//#include "dali_item_mgr.h"

class Memo_t;
class Project_t;
class SQLIterator;
class Equivalence_t;
class HashTable;
//bool GetRelInfo(const char *name_a, TransID &tid_a, TupleDef &result_a);
bool GetRelInfo(char *name_a, TupleDef &result_a);

class SQLStmt
        /* Base class for all SQL operations, whether they evaluate
           to tables or not. Therefore an $SQLStmt$ object should never
           be the root of a query subtree. It should represent a full
           SQL statement. */
{
  protected:
        enum SQLStmtOp
        {
                D_REL_EXPR,
                D_NON_REL_EXPR
        };
  private:
        SQLStmtOp  op;    /* Set when parsing */
  protected:
        TupleDef params;  /* What all parameters are passed into this op
                                                 node when it is opened */
        void Constructor(SQLStmtOp op_a) 
        {
                op = op_a;
        }
        bool LocalPrint(FILE *file);
  public:
        TupleDef resultInfo;   /*  Set on type analysis. */
        /* FIX: Add this
           Alg method; Set after selecting evaluation plan. */
        /* FIX: Need to convert pointers into Dali pointers */
        
        bool print(FILE *file);
//      bool typeCheck(TransID &tid_a);
        bool typeCheck();
//      bool optimize(TransID &tid_a);
        bool rewrite();
//      bool evaluate(TransID &tid_a);
        Equivalence_t* PostOrder(Memo_t& memo);
};

class RelExpr : public SQLStmt
        /* Base class for all those SQL operations whose result is a 
           table. */
{
  protected:
        enum RelExprOp 
        {
                D_RELSCAN,
                D_RENAME,
                D_SELECT,
                D_GEN_PROJECT,
                D_JOIN,
                D_DUPELIM,
                D_SORT,
                D_REL_VALUES
        };
  private:
        RelExprOp op;    /* Set when parsing */
   
  protected:
        int      iterOffset; 

        void Constructor(RelExprOp op_a) 
        {
                SQLStmt::Constructor(D_REL_EXPR);
                op = op_a;
        }
  public: 
        bool print(FILE *file);
//      int typeCheck(int, TransID &);
        int typeCheck(int);
                /* Performs type-checking of $Expr$s and $RelExpr$s. Dis-ambiguates
                   those $RelArgRef$s which have a null $relRef$ field. As a result
                   of these operations, it fills up the $resultInfo$ field, and also
                   sets $iterOffset$ for all the nodes in the query tree.
                   It makes only one assumption about the query tree, i.e. there
                   are no $LocalArgRef$s in it. If $resultInfo$ is already allocated
                   when this function is called, its previous contents are erased
                   and it is filled up newly.
                   The integer argument is the next free $iterOffset$ from which 
                   $RelExpr$ objects belonging to the subtree rooted at $this$ 
                   object can be assigned.
                   Return value is the next free $iterOffset$ after $RelExpr$
                   objects belonging to the subtree rooted at $this$ object have
                   been assigned. -1 if $typeCheck$ fails for some reason.
                   The query tree nodes are assigned $iterOffset$s in post 
                   order. */
//      bool optimize(TransID &);
        bool rewrite();
                /* Replaces all $RelArgRef$s in the tree by $LocalArgRef$s. Assumes
                   that $typeCheck$ has already been called on the tree. */
//      bool evaluate(TransID &tid_a);
        bool push_down_preds(ClauseList *clauseList);
                /* Pushes predicates as far down as possible. Removes empty
                   $SelectExpr$s which may be left behind as a consequence. This
                   function also makes two assumptions: 1) There are no 
                   $LocalArgRef$s in the tree 2) $typeCheck$ has already been called,
                   before calling this function. 
                   When a node X receives a predicate
                   from its parent, the predicate doesn't have $RelArgRef$s, but instead
                   it has $LocalArgRef$s of the form <RESULT, field no>. This 
                   $LocalArgRef$ refers to X's $resultInfo$. Before pushing the 
                   predicate to its child, it rewrites the $LocalArgRef$ in terms of
                   the child's $resultInfo$. If it cannot push the predicate to the 
                   child, it reconverts the $LocalArgRef$s in it to $RelArgRef$s
                   and retains the predicate. */
        SQLIterator *createIters();
                /* Creates and returns a tree of iterators, which is a mirror
                   of the query tree. */
       
       RelExprOp GetOpType(void) {
           return op;
       }
       Equivalence_t* PostOrder(Memo_t& memo);
};
/*
class NonRelExpr : public SQLStmt
         Base class for all those SQL operations whose result is not a
           table, eg CREATE TABLE, INSERT TABLE, DELETE TABLE ... 
{
  protected:
        enum NonRelExprOp
        {
                D_INSERT,
                D_DELETE,
                D_UPDATE,
                D_CREATE_SCHEMA,
                D_CREATE_DB,
                D_CREATE_TS,
                D_DROP_DB,
                D_DROP_TS,
                D_DROP_TABLE,
                D_DROP_INDEX
        };
  private:
        NonRelExprOp op;    Set when parsing 
  protected:
        void Constructor(NonRelExprOp op_a) 
        {
                SQLStmt::Constructor(D_NON_REL_EXPR);
                op = op_a;
        }
  public: 
        bool print(FILE *file);
        bool typeCheck(TransID &);
        bool typeCheck();
        bool optimize(TransID &);
        bool rewrite();
        bool evaluate(TransID &tid_a);
       Equivalence_t* PostOrder(Memo_t& memo);
};
*/
class RelScanExpr : public RelExpr 
{
        void Constructor(const char *name_a, Expr *cond_a);
        bool addAndExpr(Expr *clause);
  public:
        char name[DALIMAXRELNAME];
        Expr *cond;
        /* NOTE: the $cond$ could be NULL. In fact it will usually be
           NULL, unless push_down_preds has been called. */
//      DaliSearchCriteria searchCrit;
                /* This is set by $push_down_preds$ for now. Later maybe we
                   should have a separate function for this which 
                   would be called after $optimize$ and before $rewrite$. */
        
        static RelScanExpr *create(const char *name, Expr *cond = NULL);
        bool print(FILE *out);
//      int typeCheck(int, TransID &);
        int typeCheck(int);
        bool rewrite();
        SQLIterator *createIters();
        bool push_down_preds(ClauseList *clauseList);
        Equivalence_t* PostOrder(Memo_t& memo);
};


class RenameExpr : public RelExpr {
        struct AttrList
        {
                int numAttrs;
                char **attrList;
        };
        char name[DALIMAXRELNAME];
        AttrList attrList;
        RelExpr *child;
  public:
        void Constructor(const char *name_a, List<char *> *attrList_a, 
          RelExpr *child_a);
        static RenameExpr *create(const char *name, List<char *> *attrList_a, 
          RelExpr *child);
        bool print(FILE *out);
//      int typeCheck(int, TransID &);
        int typeCheck(int);
        bool rewrite() { return child->rewrite(); }
        bool push_down_preds(ClauseList *clauseList)
        {
                return child->push_down_preds(clauseList);
        }
        SQLIterator *createIters();
        Equivalence_t* PostOrder(Memo_t& memo);

        friend class RenameIter;
};

class SelectExpr : public RelExpr {
  public:
        Expr      *cond;
        /* $cond$ could be NULL */
        RelExpr    *child;
        
        void Constructor(Expr *cond_a, RelExpr *child_a);
        static SelectExpr *create(Expr *condition, RelExpr *in);
        bool print(FILE *out);
//      int typeCheck(int, TransID &);
        int typeCheck(int);
        bool rewrite();
        bool push_down_preds(ClauseList *clauseList);
        SQLIterator *createIters();
        Equivalence_t* PostOrder(Memo_t& memo);
};

class ProjectElem
// To store each projection attribute
{
        bool rewrite(TupleDef &bindings_a, ArgSource source_a);
                // $rewrite$ assumes that $typeCheck$ has been performed.
        bool typeCheck(TupleDef &bindings_a, ArgDef &result_a);
                // Type checks $projectionExpr$. Also sets the members of $result_a$.
        bool print(FILE *);

        friend class GeneralProjectExpr;
  public:
        char *newName; // introduced by the "AS" clause
        Expr *projectionExpr;
};
typedef List<ProjectElem> ProjectElemList;

class GeneralProjectExpr : public RelExpr
        // This is the immediate super-class of $ProjectExpr$ and $GroupAggExpr$
{
  protected:
        enum GeneralProjectExprOp
        {
                D_PROJECT, D_GROUP_AGG
        };
  private:
        GeneralProjectExprOp op;
  protected:
        struct 
        {
                int numProjectElems;
                ProjectElem *projectElems;
        } projList;     /* In case of $GroupAggExpr$, each $Expr$ in 
                                   $projList$ should be single valued per 
                                   group. */
        RelExpr *child;

        void Constructor(GeneralProjectExprOp op_a, ProjectElemList *projList_a,
          RelExpr *child_a);
//      int LocalTypeCheck(int, TransID &);
        int LocalTypeCheck(int);
                   /* This function typechecks the $projList$
                   member, and then sets the $resultInfo$ member. */
        bool LocalRewrite();
        int LocalPrint(FILE *out);
  public:
        bool print(FILE *out);
//      int typeCheck(int, TransID &);
        int typeCheck(int);
        bool rewrite();
        bool push_down_preds(ClauseList *clauseList);
        SQLIterator *createIters();
        Equivalence_t* PostOrder(Memo_t& memo);

                                // Added to Implement typeCheck method of InExpr
        ProjectElem*    GetProjList()
                {
                        if ( projList.numProjectElems != 1 )            return (NULL);
                        
                        // Not applicable to InExpr Class
                        // If Number of attribute projected is more then one
                        //
                
                        return(projList.projectElems);

                }
 // Project_t *MkProject_t();
                
};

class ProjectExpr : public GeneralProjectExpr {
        void Constructor(ProjectElemList *pList_a, RelExpr *child_a);
  public:
        static ProjectExpr *create(ProjectElemList *projlist, RelExpr *child);
        bool print(FILE *out);
//      int typeCheck(int, TransID &);
        int typeCheck(int);
        bool rewrite();
        SQLIterator *createIters();
        Equivalence_t* PostOrder(Memo_t& memo);

        friend class ProjectIter;
};

class JoinExpr : public RelExpr {
  public:
        Expr       *cond;
        /* The $cond$ could be NULL */
        RelExpr    *left;
        RelExpr    *right;
        
        void Constructor(Expr *cond_a, RelExpr *left_a, RelExpr *right_a);
        static JoinExpr *create(Expr *condition, RelExpr *left, RelExpr
                            *right);
        void addAndExpr(Expr *clause);
        bool print(FILE *out);
//      int typeCheck(int, TransID &);
        int typeCheck(int);
        bool rewrite();
        bool push_down_preds(ClauseList *clauseList);
        SQLIterator *createIters();
        Equivalence_t* PostOrder(Memo_t& memo);
};

class GroupAggExpr : public GeneralProjectExpr {
        void Constructor(Tuple *gBAttrs_a, ProjectElemList *aggList_a, 
          RelExpr *child_a);
  public:
        Tuple      *groupByAttrs; // self-suggestive
        
        static GroupAggExpr *create(Tuple *groupByAttrs,
                    ProjectElemList *aggList, RelExpr *child);
        bool print(FILE *out);
//      int typeCheck(int, TransID &);
        int typeCheck(int);
        bool rewrite();
        Equivalence_t* PostOrder(Memo_t& memo);
};


class DupelimExpr : public RelExpr {
  public:
        RelExpr *child;
        
        void Constructor(RelExpr *child_a);
        static DupelimExpr *create(RelExpr *child);
        bool print(FILE *out);
//      int typeCheck(int, TransID &);
        int typeCheck(int);
        bool rewrite() { return child->rewrite(); }
        bool push_down_preds(ClauseList *clauseList)
        {
                return child->push_down_preds(clauseList);
        }
        Equivalence_t* PostOrder(Memo_t& memo);
};


class SortExpr : public RelExpr 
{
  public:
        Tuple *sortAttrs;
        int direction;
        RelExpr *child;

        void Constructor(Tuple *sortAttrs_a, int dir_a, RelExpr *child_a);
        static SortExpr *create(Tuple *sortAttrs, int dir, RelExpr *child);
        bool print(FILE *out);
//      int typeCheck(int, TransID &);
        int typeCheck(int);
        bool rewrite();
        bool push_down_preds(ClauseList *clauseList)
        {
                 return child->push_down_preds(clauseList);
        }
        Equivalence_t* PostOrder(Memo_t& memo);
};

class RelValueExpr : public RelExpr {
  public:
        Tuple *value;
            /* FIX:  generalize to allow multiple rows */

        void Constructor(Tuple *value_a);
        static RelValueExpr *create(Tuple *value);
        bool print(FILE *out);
//      int typeCheck(int, TransID &);
        int typeCheck(int);
        bool rewrite() { return true; }
        bool push_down_preds(ClauseList *) { return true; }
        SQLIterator *createIters();
        Equivalence_t* PostOrder(Memo_t& memo);
};

#endif // __REL_ALG_H__
