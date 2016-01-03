// interface library --- to interface with DALI parser code

#include "DaliParser/sql_parse.h"
#include "DaliParser/exprinterface.h"
#include <string.h>
#include "List.h"
#include "Memo.h"
#include "LogProp.h"
#include "Equivalence.h"
#include "LogicalOp.h"
#include "Volcano.h"
#include "Interface.h"

class DaliTableSchema;

extern int yyparse(void);               // parser
extern FILE *yyin;
int line_num = 1, stmtCount = 1;        // global counters
char *lexString = NULL;                 // used by parser
int lexInt = -1;                        // used by parser
float lexFloat = -1.0;                  // used by parser
List<SQLStmt *> stmtList;               // global expression list
extern ConstPreds_t ConstPreds;         // constant predicates

extern AliasTable Interface_t::aliasTable;
extern SortOrder_t *Interface_t::reqOrder;

// get the expression dag
Equivalence_t *Interface_t::ExpressionDAG(Optimizer_t& opt, char *inpFile)
{
    Memo_t& memo = opt.Memo();

    // TEMPCODE: 
    if(inpFile) {
        yyin = fopen(inpFile, "r");
    }

    yyparse();

    Equivalence_t *expr = NULL; // root of the expression dag

    Listiter<SQLStmt *> stmtIter(stmtList);
                                // iterator over the statement trees
    SQLStmt *stmt = NULL;       // statement parsed into a expr tree

    for( stmtIter.reset(), stmtCount = 1; !stmtIter.at_end(); stmtCount++ ) {
        // next statement expression tree
        stmtIter.next(stmt);     

        // get the expression dag for the statement
        Equivalence_t* eq = stmt->PostOrder(memo);
        if( eq ) {
            if( !expr ) {
                expr = eq;
            } else {
                // create a binary union node as a virtual root
                Union_t *rootOp = new Union_t(2);
                rootOp->SetInput(0, expr);
                rootOp->SetInput(1, eq);

                // determine logical property for the root
                // since this is the virtual root, any dummy value will do
                LogProp_t *logProp = rootOp->LogicalProp();
                Refer(logProp);

                // create an equivalence class for the virtual root
                Equivalence_t *rootEq = new Equivalence_t(logProp);
                DeRefer(logProp);

                // associate rootEq with rootOp
                rootOp->SetEqClass(rootEq);
                rootEq->AddLogExpr(rootOp);

                // add to the memo table
                memo.Insert(rootOp);

                expr = rootEq;
            }
        }
    }

    if( Config_t::IsDebug(1) ) {
        cout << "Initial Query Graph" << endl;
        expr->PrintExpressionDAG(0);
    }

    expr->SetRoot();

    // TEMPCODE: 
    if(inpFile) {
        fclose(yyin);
    }
    return expr;
}

Equivalence_t* RelScanExpr::PostOrder(Memo_t& memo)
{
    Rel_t* relOp = new Rel_t(name);

    LogicalOp_t *prevRelOp = memo.Search(relOp);
    Equivalence_t *relEq = NULL;

    if( prevRelOp ) {
        // operator present in the memo table
        delete relOp;

        relEq = prevRelOp->EqClass();
        assert(relEq);
    } else {
        LogProp_t *logProp = relOp->LogicalProp();
        if (!logProp) {
            if (Config_t::IsDebug(DEBUG_ERROR)) {
                cout << "Fatal Error: Catalog lookup for relation " 
                     << name << " failed!" << endl;
            }
            exit(1);
        }
        Refer(logProp);

        relEq = new Equivalence_t(logProp, 1);
        DeRefer(logProp);

        relOp->SetEqClass(relEq);
        relEq->AddLogExpr(relOp);

        memo.Insert(relOp);
    }

    return relEq;
}

Equivalence_t* RenameExpr::PostOrder(Memo_t& memo)
{
    /* not supported 
    assert(0);
    return NULL;
    */

    Equivalence_t *childEq = child->PostOrder(memo);
    assert(childEq);
    AppendList_t<LogicalOp_t *>& childList = childEq->LogExprList();
    ListIter_t<LogicalOp_t *> iter;
    iter.Attach(&childList);

    LogicalOp_t *child = NULL;
    if (!iter.IsEnd())        // This should always pass as there must be 
        child = iter.Next(); // exactly one

    assert(child);
    assert(child->Type() == REL_T);
    Rel_t *rel = (Rel_t *) child;
    const char *originalName = rel->Name();
    
    Interface_t::aliasTable.addMapping(name, originalName);

    return childEq;
}
  
Equivalence_t* SelectExpr::PostOrder(Memo_t& memo)
{
    // if no conditions exist then ignore this select expression
    if( !cond )
        return child->PostOrder(memo);

    Expr_t* expr = ExprInterface::ExprPostOrder(cond, memo);
    assert(expr->ExprType() == PREDICATE_T);
    Predicate_t* pred = (Predicate_t *) expr;

    Refer(pred);
    // FIX: ambiguous attribute checking dropped
    // FIX: duplicate elimination among expressions dropped

    // create the select operator
    Select_t* selOp = new Select_t(pred);
    Equivalence_t *childEq = child->PostOrder(memo);
    selOp->SetInput(0, childEq); 
    DeRefer(pred);

    LogicalOp_t *prevSelOp = memo.Search(selOp);
    Equivalence_t *selEq = NULL;

    if( prevSelOp ) {
        // operator present in the memo table
        delete selOp;

        selEq = prevSelOp->EqClass();
        assert(selEq);
    } else {
        LogProp_t *logProp = selOp->LogicalProp();
        Refer(logProp);

        selEq = new Equivalence_t(logProp);
        DeRefer(logProp);

        // associate selOp with selEq
        selOp->SetEqClass(selEq);
        selEq->AddLogExpr(selOp);
        memo.Insert(selOp);
    }

    return selEq;
}

Equivalence_t* JoinExpr::PostOrder(Memo_t& memo)
{
    // get the predicate
    Expr_t* expr = ExprInterface::ExprPostOrder(cond, memo);
    if( !expr ) expr = ConstPreds.True();

    assert(expr->ExprType() == PREDICATE_T);
    Predicate_t* pred = (Predicate_t *) expr;

    Refer(pred);
    // FIX: ambiguous attribute checking dropped
    // FIX: duplicate elimination among expressions dropped

    // create the join operator
    Join_t *joinOp = new Join_t(pred) ;
    Equivalence_t *leftEq = left->PostOrder(memo);
    Equivalence_t *rightEq = right->PostOrder(memo);
    joinOp->SetInput(0, leftEq);
    joinOp->SetInput(1, rightEq);
    DeRefer(pred);

    // search in the memo table
    LogicalOp_t *prevJoinOp = memo.Search(joinOp);
    Equivalence_t *joinEq = NULL;

    if( prevJoinOp ) {
        // operator present in the memo table
        delete joinOp;

        joinEq = prevJoinOp->EqClass();
        assert(joinEq);
    } else {
        // create the log prop of the output
        LogProp_t *logProp = joinOp->LogicalProp();
        Refer(logProp);

        // create the equivalence class
        joinEq = new Equivalence_t(logProp);
        DeRefer(logProp);

        // associate joinOp with joinEq
        joinOp->SetEqClass(joinEq);
        joinEq->AddLogExpr(joinOp);

        memo.Insert(joinOp);
    }

    return joinEq;
}

Equivalence_t* ProjectExpr::PostOrder(Memo_t& memo)
{
        // create the project operator
        Project_t* projOp = new Project_t(projList.numProjectElems);
        Equivalence_t *childEq = child->PostOrder(memo);
        projOp->SetInput(0, childEq);

        // set the projection mapping
        for( int i = 0 ; i <  projList.numProjectElems ; i++ ) {
            Expr_t *projExpr = ExprInterface::ExprPostOrder(
                                projList.projectElems[i].projectionExpr, memo);
            Refer(projExpr);
            // FIX: ambiguous attribute checking dropped FIX: duplicate
            // elimination among expressions dropped

            projOp->SetProj(i, projExpr, projList.projectElems[i].newName);
            DeRefer(projExpr);
        }

        // search in the memo table
        LogicalOp_t *prevProjOp = memo.Search(projOp); Equivalence_t *projEq
        = NULL;

        if( prevProjOp ) {
            // operator present in the memo table
            delete projOp;

            projEq = prevProjOp->EqClass(); assert(projEq);
        } else {
            // create the log prop of the output
            LogProp_t *logProp = projOp->LogicalProp(); Refer(logProp);

            // create the equivalence class
            projEq = new Equivalence_t(logProp); DeRefer(logProp);

            // associate aggOp with joinEq
            projOp->SetEqClass(projEq); projEq->AddLogExpr(projOp);

            memo.Insert(projOp);
        }

        return projEq;
}

Equivalence_t *GroupAggExpr::PostOrder(Memo_t& memo)
{
    Aggregate_t *aggOp = NULL;

    int numAggExprs = 0;
    if( groupByAttrs ) {
        // Ravi: Fix made: 2nd arg should be just the # of agg attrs
        numAggExprs = projList.numProjectElems - groupByAttrs->numAttrs;
        aggOp = new Aggregate_t(groupByAttrs->numAttrs, numAggExprs);
        for( int i = 0; i < groupByAttrs->numAttrs ; i++ ) {
            Expr_t *groupByExpr =
                ExprInterface::ExprPostOrder(groupByAttrs->attrs[i], memo);
            Refer(groupByExpr);

            aggOp->SetGroupByExpr(i, groupByExpr, NULL);
            DeRefer(groupByExpr);
        }
    } else {
        aggOp = new Aggregate_t(0, projList.numProjectElems);
    }

    int aggExprCount = 0;
    for (int i = 0 ; i < projList.numProjectElems ; i++) {
        if (projList.projectElems[i].projectionExpr->exprType != 
            Expr::D_AGG_EXPR) {
                continue;
        }

        Expr_t *aggExpr =
            ExprInterface::ExprPostOrder(
                                projList.projectElems[i].projectionExpr, memo);
        Refer(aggExpr);

        aggOp->SetAggExpr(aggExprCount++, aggExpr, 
                          projList.projectElems[i].newName);
        DeRefer(aggExpr);
    }

    Equivalence_t *childEq = child->PostOrder(memo);    
    aggOp->SetInput(0, childEq);

    // search in the memo table
    LogicalOp_t *prevAggOp = memo.Search(aggOp);
    Equivalence_t *aggEq = NULL;

    if( prevAggOp ) {
        // operator present in the memo table
        delete aggOp;

        aggEq = prevAggOp->EqClass();
        assert(aggEq);
    } else {
        // create the log prop of the output
        LogProp_t *logProp = aggOp->LogicalProp();
        Refer(logProp);

        // create the equivalence class
        aggEq = new Equivalence_t(logProp);
        DeRefer(logProp);

        // associate aggOp with joinEq
        aggOp->SetEqClass(aggEq);
        aggEq->AddLogExpr(aggOp);

        memo.Insert(aggOp);
    }

    return aggEq;
}

Equivalence_t* DupelimExpr::PostOrder(Memo_t& memo)
{
     return child->PostOrder(memo);
}

Equivalence_t* SortExpr::PostOrder(Memo_t& memo)
{
    // return child->PostOrder(memo);

    // Ravi: EIO: Implementing support for ORDER BY clause
    Equivalence_t *childEq = child->PostOrder(memo);
    Schema_t *inpSchema = childEq->LogicalProp()->Schema();
    int *bindArray = new int[inpSchema->NumAttr()];
    int  count = 0;
    int numSortAttrs = sortAttrs->numAttrs;
    for (int i = 0; i < numSortAttrs; i++) {
        Expr_t *sortAttrExpr =
                ExprInterface::ExprPostOrder(sortAttrs->attrs[i], memo);
        assert(sortAttrExpr->ExprType() == REL_ARG_REF_T);
        int bnd = ((RelArgRef_t *) sortAttrExpr)->Binding(inpSchema);
        assert(bnd >= 0);
        bindArray[count++] = bnd;
    }

    SortOrder_t *reqSortOrder = new SortOrder_t(bindArray, count);
    delete[] bindArray;
    Interface_t::reqOrder = reqSortOrder;

    if( Config_t::IsDebug(200) ) {
        cout << "Order Requirement on the Root EqNode:" << endl;
        reqSortOrder->Print(inpSchema);
    }
    return childEq;
}

Equivalence_t* RelValueExpr::PostOrder(Memo_t&)
{
    /* not supported */
    assert(0);
    return NULL;
}

/*
 * what follows below is a roundabout for simple inheritance; wonder
 * why it is coded this way -- Prasan
 */

Equivalence_t * SQLStmt::PostOrder(Memo_t& memo) {

        switch(op)
        {
          case D_REL_EXPR: return ((RelExpr *)this)->PostOrder(memo);
//          case D_NON_REL_EXPR: return ((NonRelExpr *)this)->PostOrder(memo);
            default: break;
        }

        return NULL;
}

Equivalence_t* RelExpr::PostOrder(Memo_t& memo) {

        switch(op)
        {
          case D_RELSCAN:       return ((RelScanExpr *)this)->PostOrder(memo);
          case D_RENAME:        return ((RenameExpr *)this)->PostOrder(memo);
          case D_SELECT:        return ((SelectExpr *)this)->PostOrder(memo);
          case D_GEN_PROJECT:
                        return ((GeneralProjectExpr *)this)->PostOrder(memo);
          case D_JOIN:          return ((JoinExpr *)this)->PostOrder(memo);
          case D_DUPELIM:       return ((DupelimExpr *)this)->PostOrder(memo);
          case D_SORT:          return ((SortExpr *)this)->PostOrder(memo);
          case D_REL_VALUES:    return ((RelValueExpr *)this)->PostOrder(memo);
          default:    fprintf(stderr,"PostOrder() invoked on BAD_RELOP");
                      exit(1);
        }
}
/*
Equivalence_t*  NonRelExpr::PostOrder(Memo_t& memo)
{
     switch(op) 
     {
       case D_CREATE_SCHEMA: return ((MultiTableSchema*)this)->PostOrder(memo);
                             break;   
       default: return ((Equivalence_t *) 0) ;
     }
   return ((Equivalence_t*) 0) ;
 
}
*/
Equivalence_t* GeneralProjectExpr::PostOrder(Memo_t& memo)
{  
 
        switch(op) {
         case D_PROJECT : return ((ProjectExpr *)this)->PostOrder(memo);
                          break;
         case D_GROUP_AGG : return ((GroupAggExpr*)this)->PostOrder(memo);
                           break;
         default :      fprintf(stderr,"\n Unknown Operator type is encountered in GeneralProjectExpr::PostOrder ");

        }

   return ((Equivalence_t*) 0) ;
}
