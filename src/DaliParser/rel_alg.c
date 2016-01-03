/*      -*- c++ -*-  */
#include "ucbproto.h"
//#include "dali.h" ARU
//#include "dali_rel_mgr.h" ARU

#include "sql_defs.h"
#include "sql_error.h"
#include "prim_types.h"
#include "expr.h"
//#include "optimize.h"
#include "rel_alg.h"
//#include "ddl.h"
/**************************************************************************/

/** 
  FIX: All the create functions below will later change to return DaliPtr's
**/

//bool GetRelInfo(const char *name_a, TransID &tid_a, TupleDef &result_a)
bool GetRelInfo(char *name_a, TupleDef &result_a)
        /* Looks up in the schema to get the type of the given table */
{
//      RELCATCatalogDBPtr relcatPtr = DaliRelMgr::getRelid(tid_a, name_a);
//      DALI_R_ASSERT(relcatPtr != CatalogDBPtr_NULL);
//      result_a.numArgs = relcatPtr->numAttrs;
        if(result_a.numArgs < 0)
                return false;
        result_a.args = (ArgDef *) new char[sizeof(ArgDef) * result_a.numArgs];
        /* FIX: if $args$ has already been allocated, try to either re-use
           the memory or at least free it. */

        void *key_val[] = {NULL, NULL, NULL, NULL, NULL, NULL};
                // The number of attributes in the ATTRCAT relation is = 6
        key_val[0] = (void *)name_a;
/*
        DaliHashIter<tuplePtr> *iterPtr = 
          DaliRelMgr::attrcat_index_on_relname->initScan(tid_a, EQ, 
            key_val);
        if(iterPtr == NULL)
                return false;
        tuplePtr *attrCatPtr;
        for(int i = 0; iterPtr->next(&attrCatPtr); i++)
        {
                DALI_R_ASSERT(strcmp(name_a, 
                  ATTRCATCatalogDBPtr(*attrCatPtr)->relName) == 0); 
                int position_in_rel = ATTRCATCatalogDBPtr(*attrCatPtr)->position_in_rel;

                strcpy(result_a.args[position_in_rel].rangeVariable, name_a);
                strcpy(result_a.args[position_in_rel].argName,
                  ATTRCATCatalogDBPtr(*attrCatPtr)->attrName);

                if(ATTRCATCatalogDBPtr(*attrCatPtr)->length > 0)
                        result_a.args[position_in_rel].argType = 
                          ArgType(daliToSQLType(ATTRCATCatalogDBPtr(*attrCatPtr)->type),
                          ATTRCATCatalogDBPtr(*attrCatPtr)->length);
                else
                        result_a.args[position_in_rel].argType = 
                          ArgType(daliToSQLType(ATTRCATCatalogDBPtr(*attrCatPtr)->type));

                result_a.args[position_in_rel].defaultValue = 
                  result_a.args[position_in_rel].argType.DefaultValue();

                 The constructors will check whether the the length should be 
                   provided or not. 
                 FIX: The $defaultValue$ should actually be read in from the 
                   catalogue. 
        }

        DaliRelMgr::attrcat_index_on_relname->closeScan(tid_a, iterPtr);
*/
        return true;
}

/* *********** */

void RelArgRef::Constructor(const char *rel_a, const char *attr_a)
{ 
        exprType = D_REL_ARG_REF; 
        if(rel_a && strlen(rel_a) >= DALIMAXRELNAME) 
        {
                fprintf(stderr, "Error:  relation name (%s) too long in Stmt No:%d\n", 
                  rel_a, stmtCount);
                return;
        }
        if(attr_a && strlen(attr_a) >= DALIMAXATTRNAME) 
        {
                fprintf(stderr, "Error:  attribute name (%s) too long in Stmt No:%d\n",
                  attr_a, stmtCount);
                return;
        }
        strcpy(relRef, (rel_a ? rel_a : ""));
        strcpy(attrName, (attr_a ? attr_a : ""));
        return;
}

RelArgRef * RelArgRef::create(const char *rel_a, const char *attr_a) 
{
        RelArgRef *v = (RelArgRef *)new char[sizeof(RelArgRef)];
        v->Constructor(rel_a, attr_a);
        return v;
}

int
RelArgRef::print(FILE *file) 
{
        if(strlen(relRef) > 0)
        {
                fprintf(file, "%s.", relRef);
        }
        fprintf(file, "%s", attrName);
        return 1;
}

int
RelArgRef::GetMatchPos(TupleDef &bindings_a)
{
        /* kvr-  I  don't  know  if what  I  am  doing  here  is  correct for more
           complicated language structures when they  are permitted later. For now
           I think it is correct. */
        bool disAmbiguationRequired = (strcmp(relRef, "") == 0);

        int i = -1;
        int firstMatch = -1;
        for(i = 0, firstMatch = -1; i < bindings_a.numArgs; ++i)
                if((strcmp(attrName, bindings_a.args[i].argName) == 0) &&
                  (disAmbiguationRequired || 
                  (strcmp(relRef, bindings_a.args[i].rangeVariable) == 0)))
                        if(firstMatch == -1)
                                firstMatch = i;
                        else
                                break;
        if((firstMatch > -1) && (i >= bindings_a.numArgs))
                return firstMatch;
        else
                return -1;
}

ArgType
RelArgRef::typeCheck(TupleDef &bindings_a)
{
        int matchPos = GetMatchPos(bindings_a);
        if(matchPos > -1)
        {
                strcpy(relRef, bindings_a.args[matchPos].rangeVariable);
                return (type = bindings_a.args[matchPos].argType);
        }
        else
                return (type = ArgType(D_UnknownType));
}

Expr *
RelArgRef::rewrite(TupleDef &bindings_a, ArgSource source_a)
{
        int matchPos = GetMatchPos(bindings_a);
        if(matchPos > -1)
                return LocalArgRef::create(source_a, matchPos, type);
        else
        {
                fprintf(stderr, "RelArgRef::rewrite() failed in stmt no:%d\n", 
                  stmtCount);
                return NULL;
        }
}

/***************/

ArgDef::ArgDef() {
  strcpy(argName, "");
}

ArgDef::ArgDef(const char *name, ArgType atype) {
        strcpy(argName, name);
        argType = atype;
}

int 
ArgDef::print(FILE *file) 
{
        fprintf(file, "%s ", argName);
        argType.print(file);
        return 0;
}

#define D_INIT_TUPLEDEF_ATTRS 5

TupleDef *
TupleDef::create() {
        TupleDef *v = (TupleDef *) new char [sizeof(TupleDef)];
        v->numArgs = 0;
        v->args = NULL;
        return v;
}

int
TupleDef::addAttr(ArgDef arg) 
{
        if(numArgs%D_INIT_TUPLEDEF_ATTRS == 0 ) {
            ArgDef *v = (ArgDef*) new char [sizeof(ArgDef)*D_INIT_TUPLEDEF_ATTRS*
                ((numArgs+D_INIT_TUPLEDEF_ATTRS)/D_INIT_TUPLEDEF_ATTRS)];
            if (!v) return -1;
            for(int i=0; i<numArgs; i++) 
                    v[i] = args[i];
                delete args;
            args = v;
        }
        args[numArgs] = arg;
        numArgs++;
        return 0;
}

int
TupleDef::addAttr(const char *name, ArgType atype) 
{
        return addAttr(ArgDef(name, atype));
}

void
TupleDef::operator = (TupleDef &rhs)
{
        numArgs = rhs.numArgs;
        args = (ArgDef*) new char [sizeof(ArgDef)*numArgs];
        // FIX: Try to reclaim the storage if $args$ has already been allocated
        for(int i=0; i < numArgs; ++i)
                args[i] = rhs.args[i];
        return;
}

void
TupleDef::operator += (TupleDef &rhs)
{
        if((numArgs == 0) && (rhs.numArgs == 0))
                return;

        ArgDef *v = (ArgDef *) new char[sizeof(ArgDef) * (numArgs + rhs.numArgs)];
        int i = -1;
        for(i = 0; i < numArgs; ++i)
                v[i] = args[i];
        for(i = 0; i < rhs.numArgs; ++i)
                v[numArgs + i] = rhs.args[i];

        args = v;
        numArgs += rhs.numArgs;
        return;
}


int 
TupleDef::print(FILE *file) 
{
        for(int i=0; i < numArgs; i++) {
        if(i>0) {
            fprintf(file, ", ");
        }
        args[i].print(file);
        }
        return 0;
}

/*****************/

bool 
SQLStmt::print(FILE *file)
{
        switch(op)
        {
          case D_REL_EXPR: return ((RelExpr *)this)->print(file);
//        case D_NON_REL_EXPR: return ((NonRelExpr *)this)->print(file);
          default: break;
        }
        return false;
}

bool 
//SQLStmt::typeCheck(TransID &tid_a)
SQLStmt::typeCheck()
{
        switch(op)
        {
//        case D_REL_EXPR: return (((RelExpr *)this)->typeCheck(0, tid_a) >= 0);
          case D_REL_EXPR: return (((RelExpr *)this)->typeCheck(0) >= 0);
//        case D_NON_REL_EXPR: return ((NonRelExpr *)this)->typeCheck(tid_a);
          default: break;
        }
        return false;
}
/*
bool 
SQLStmt::optimize(TransID &tid_a)
SQLStmt::optimize()
{
        switch(op)
        {
          case D_REL_EXPR: return ((RelExpr *)this)->optimize(tid_a);
          case D_NON_REL_EXPR: return ((NonRelExpr *)this)->optimize(tid_a);
          default: break;
        }
        return false;
}
*/
bool 
SQLStmt::rewrite()
{
        switch(op)
        {
          case D_REL_EXPR: return ((RelExpr *)this)->rewrite();
//        case D_NON_REL_EXPR: return ((NonRelExpr *)this)->rewrite();
          default: break;
        }
        return false;
}
/*
bool 
SQLStmt::evaluate(TransID &tid_a)
{
        switch(op)
        {
          case D_REL_EXPR: return ((RelExpr *)this)->evaluate(tid_a);
          case D_NON_REL_EXPR: return ((NonRelExpr *)this)->evaluate(tid_a);
          default: break;
        }
}
*/
bool
SQLStmt::LocalPrint(FILE *file)
{
        fprintf(file, " <");
        int i = -1;
        resultInfo.numArgs = 0;                 // ??? Added for debuging
        for(i = 0; i < resultInfo.numArgs - 1; ++i)
        {
                printf("in--\n");
                fprintf(file, "(%s.%s : ", resultInfo.args[i].rangeVariable,
                  resultInfo.args[i].argName);
                resultInfo.args[i].argType.print(file);
                printf("out--\n");
                fprintf(file, "), ");
        }
        if(resultInfo.numArgs > 0)
        {
                printf("in-c-\n");
                fprintf(file, "(%s.%s : ", resultInfo.args[i].rangeVariable,
                  resultInfo.args[i].argName);
                resultInfo.args[i].argType.print(file);
                fputc(')', file);
                printf("out-c-\n");
        }
        fprintf(file,"> ");
        return true;
}

/*****************/
/*
bool
NonRelExpr::print(FILE *file_a)
{
        switch(op)
        {
          case D_INSERT:                return ((InsertTable *)this)->print(file_a);
          case D_DELETE:                return ((DeleteTable *)this)->print(file_a);
          case D_UPDATE:                return ((UpdateTable *)this)->print(file_a);
          case D_CREATE_SCHEMA: return ((MultiTableSchema *)this)->print(file_a);
          case D_CREATE_DB:             return ((CreateDataBase *)this)->print(file_a);
          case D_CREATE_TS:             return ((CreateTableSpace *)this)->print(file_a);
          case D_DROP_DB:               return ((DropDataBase *)this)->print(file_a);
          case D_DROP_TS:               return ((DropTableSpace *)this)->print(file_a);
          case D_DROP_TABLE:    return ((DropTable *)this)->print(file_a);
          case D_DROP_INDEX:    return ((DropIndex *)this)->print(file_a);
          default:      SQLErrAR("print() invoked for BAD_NON_RELOP(%d) ", op, false)
        }
        return false;
}

bool 
NonRelExpr::typeCheck(TransID &tid_a)
{
        switch(op)
        {
          case D_INSERT:
                return ((InsertTable *)this)->typeCheck(tid_a);
          case D_DELETE:
                return ((DeleteTable *)this)->typeCheck(tid_a);
          case D_UPDATE:
                return ((UpdateTable *)this)->typeCheck(tid_a);
          case D_CREATE_SCHEMA:
                return ((MultiTableSchema *)this)->typeCheck(tid_a);
          case D_CREATE_DB:
                return ((CreateDataBase *)this)->typeCheck(tid_a);
          case D_CREATE_TS:
                return ((CreateTableSpace *)this)->typeCheck(tid_a);
          case D_DROP_DB:
                return ((DropDataBase *)this)->typeCheck(tid_a);
          case D_DROP_TS:
                return ((DropTableSpace *)this)->typeCheck(tid_a);
          case D_DROP_TABLE:
                return ((DropTable *)this)->typeCheck(tid_a);
          case D_DROP_INDEX:
                return ((DropIndex *)this)->typeCheck(tid_a);
          default:
                SQLErrAR("typeCheck() invoked for bad operator BAD_NON_RELOP(%d)", op,
                  -1)
        }
        return false;
}

bool
NonRelExpr::rewrite()
{
        switch(op)
        {
          case D_INSERT:                return ((InsertTable *)this)->rewrite(); 
          case D_DELETE:                return ((DeleteTable *)this)->rewrite(); 
          case D_UPDATE:                return ((UpdateTable *)this)->rewrite(); 
          case D_CREATE_SCHEMA: return ((MultiTableSchema *)this)->rewrite(); 
          case D_CREATE_DB:             return ((CreateDataBase *)this)->rewrite();
          case D_CREATE_TS:             return ((CreateTableSpace *)this)->rewrite();
          case D_DROP_DB:               return ((DropDataBase *)this)->rewrite();
          case D_DROP_TS:               return ((DropTableSpace *)this)->rewrite();
          case D_DROP_TABLE:    return ((DropTable *)this)->rewrite();
          case D_DROP_INDEX:    return ((DropIndex *)this)->rewrite();
          default:
                SQLErrAR("rewrite() invoked for bad operator BAD_NON_RELOP(%d)", op,
                  false)
        }
        return false;
}
*/
/*****************/

bool
RelExpr::print(FILE *file) 
{
        switch(op) 
        {
          case D_RELSCAN:               return ((RelScanExpr *)this)->print(file);
          case D_RENAME:                return ((RenameExpr *)this)->print(file);
          case D_SELECT:                return ((SelectExpr *)this)->print(file);
          case D_GEN_PROJECT:   return ((GeneralProjectExpr *)this)->print(file);
          case D_JOIN:                  return ((JoinExpr *)this)->print(file);
          case D_DUPELIM:               return ((DupelimExpr *)this)->print(file);
          case D_SORT:                  return ((SortExpr *)this)->print(file);
          case D_REL_VALUES:    return ((RelValueExpr *)this)->print(file);
          default:                      SQLErrAR("print() invoked on BAD_RELOP(%d) ", op, false)
        }
        return false;
}

int 
//RelExpr::typeCheck(int iterOffset_a, TransID &tid_a)
RelExpr::typeCheck(int iterOffset_a)
{
        switch(op)
        {
          case D_RELSCAN: 
//              return ((RelScanExpr *)this)->typeCheck(iterOffset_a, tid_a);
                return ((RelScanExpr *)this)->typeCheck(iterOffset_a);
          case D_RENAME:
//              return ((RenameExpr *)this)->typeCheck(iterOffset_a, tid_a);
                return ((RenameExpr *)this)->typeCheck(iterOffset_a);
          case D_SELECT:
//              return ((SelectExpr *)this)->typeCheck(iterOffset_a, tid_a);
                return ((SelectExpr *)this)->typeCheck(iterOffset_a);
          case D_GEN_PROJECT:
//              return ((GeneralProjectExpr *)this)->typeCheck(iterOffset_a, tid_a);
                return ((GeneralProjectExpr *)this)->typeCheck(iterOffset_a);
          case D_JOIN:
//              return ((JoinExpr *)this)->typeCheck(iterOffset_a, tid_a);
                return ((JoinExpr *)this)->typeCheck(iterOffset_a);
          case D_DUPELIM:
//              return ((DupelimExpr *)this)->typeCheck(iterOffset_a, tid_a);
                return ((DupelimExpr *)this)->typeCheck(iterOffset_a);
          case D_SORT:
//              return ((SortExpr *)this)->typeCheck(iterOffset_a, tid_a);
                return ((SortExpr *)this)->typeCheck(iterOffset_a);
          case D_REL_VALUES:
//              return ((RelValueExpr *)this)->typeCheck(iterOffset_a, tid_a); 
                return ((RelValueExpr *)this)->typeCheck(iterOffset_a);
          default:
                SQLErrAR("typeCheck() invoked on bad operator BAD_RELOP(%d)", op, -1)
        }
        return false;
}

bool
RelExpr::rewrite()
{
        switch(op)
        {
          case D_RELSCAN: return ((RelScanExpr *)this)->rewrite(); 
          case D_RENAME:  return ((RenameExpr *)this)->rewrite(); 
          case D_SELECT:  return ((SelectExpr *)this)->rewrite(); 
          case D_GEN_PROJECT: return ((GeneralProjectExpr *)this)->rewrite();
          case D_JOIN:    return ((JoinExpr *)this)->rewrite(); 
          case D_DUPELIM: return ((DupelimExpr *)this)->rewrite(); 
          case D_SORT:    return ((SortExpr *)this)->rewrite(); 
          case D_REL_VALUES: return ((RelValueExpr *)this)->rewrite(); 
          default:
                SQLErrAR("rewrite() invoked on bad operator BAD_RELOP(%d)", op, false)
        }
        return false;
}

/***************/

void 
RelScanExpr::Constructor(const char *name_a, Expr *cond_a)
{
        RelExpr::Constructor(D_RELSCAN);
        cond = cond_a;
        strcpy(name, name_a);
        return;
}

RelScanExpr * RelScanExpr::create(const char *relName, Expr *cond) 
{
        RelScanExpr *v = (RelScanExpr *) new char[sizeof(RelScanExpr)];
        v->Constructor(relName, cond);
        return v;
}

//int RelScanExpr::typeCheck(int iterOffset_a, TransID &tid_a)
int RelScanExpr::typeCheck(int iterOffset_a)
{
        if((iterOffset = iterOffset_a) < 0)
                SQLErrR("RelScanExpr::typeCheck(): Invalid iter_offset passed as"
                  " argument", -1)
//      if(GetRelInfo(name, tid_a, resultInfo) == false)
        if(GetRelInfo(name, resultInfo) == false)
                SQLErrAR("rel-scan has invalid table name: %s", name, -1)
        if((cond != NULL) && ((cond->typeCheck(resultInfo)).type != D_Truthval))
                SQLErrR("Type checking of rel-scan's condition failed", -1)
        else
                return iterOffset+1;
        return false;
}

bool
RelScanExpr::rewrite()
{
        Expr *newExpr;
        if((cond != NULL) && 
          ((newExpr = cond->rewrite(resultInfo, RESULT)) != cond))
                return ((cond = newExpr) != NULL);
        return true;
}

bool
RelScanExpr::print(FILE *file) 
{
        fprintf(file, " scan["); 
        if(cond) 
                cond->print(file);
        fprintf(file, "](%s)", name); 
        return LocalPrint(file);
}

/***************/

//int RenameExpr::typeCheck(int iterOffset_a, TransID &tid_a)
int RenameExpr::typeCheck(int iterOffset_a)
{
//      if((iterOffset = child->typeCheck(iterOffset_a, tid_a)) < 0)
        if((iterOffset = child->typeCheck(iterOffset_a)) < 0)
        {
                fprintf(stderr, "RenameExpr::typeCheck(): illegal iterOffset in stmt"
                  " no:%d\n", stmtCount);
                return -1;
        }
        if((attrList.numAttrs > 0) && 
          (child->resultInfo.numArgs != attrList.numAttrs))
        {
                fprintf(stderr, "RenameExpr has wrong number of attribute names"
                  " in stmt no:%d\n", stmtCount);
                return -1;
        }

        /* Now we have to copy the child's 'resultInfo' into the parent's
           field. I have defined operator= for 'TupleDef' */
        resultInfo = child->resultInfo; // that is the overloaded operator=

        int i = -1;
        for(i = 0; i < resultInfo.numArgs; ++i)
                strcpy(resultInfo.args[i].rangeVariable, name);
        if(attrList.numAttrs > 0)
                for(i = 0; i < attrList.numAttrs; ++i)
                        strcpy(resultInfo.args[i].argName, attrList.attrList[i]);
        return iterOffset + 1;
}

void RenameExpr::Constructor(const char *name_a, List<char *> *attrList_a, 
  RelExpr *child_a)
{
        RelExpr::Constructor(D_RENAME);
        strcpy(name, name_a);
        if(attrList_a)
        {
                attrList.numAttrs = attrList_a->length();
                attrList.attrList = new char *[attrList.numAttrs];
                Listiter<char *> attrIter(*attrList_a);
                for(int i = 0; !attrIter.at_end(); ++i)
                        attrIter.next(attrList.attrList[i]);
                delete attrList_a;
                /* kvr- FIX: Add a check to see whether all the attribute names
                   provided are unique. */
        }
        else
                attrList.numAttrs = 0;
        child = child_a;
}

RenameExpr *RenameExpr::create(const char *relName, List<char *> *attrList_a, 
  RelExpr *child) 
{
        RenameExpr *v = (RenameExpr *) new char[sizeof(RenameExpr)];
        v->Constructor(relName, attrList_a, child);
        return v;
}

bool
RenameExpr::print(FILE *file) 
{
        child->print(file);
        fprintf(file, " AS %s ", name); 
        if(attrList.numAttrs > 0) {
                putc('(', file);
                int i = -1;
                for(i = 0; i < attrList.numAttrs - 1; ++i)
                        fprintf(file, "%s, ", attrList.attrList[i]);
                fprintf(file, "%s)", attrList.attrList[i]);
        }
        return LocalPrint(file);
}

/***************/

void SelectExpr::Constructor(Expr *cond_a, RelExpr *child_a)
{
        RelExpr::Constructor(D_SELECT);
        cond = cond_a;
        child = child_a;
        return;
}

SelectExpr * SelectExpr::create(Expr *cond, RelExpr *child) 
{
        SelectExpr *v = (SelectExpr *) new char[sizeof(SelectExpr)];
        v->Constructor(cond, child);
        return v;
}

bool
SelectExpr::print(FILE *file) 
{
        fprintf(file, " select[ "); 
        if(cond) {
           cond->print(file);
                
        }
        fprintf(file, "](");

        child->print(file);

        fprintf(file, ")");
        LocalPrint(file);
        return true;

}

//int SelectExpr::typeCheck(int iterOffset_a, TransID &tid_a)
int SelectExpr::typeCheck(int iterOffset_a)
{
//      if((iterOffset = child->typeCheck(iterOffset_a, tid_a)) < 0)
        if((iterOffset = child->typeCheck(iterOffset_a)) < 0)
        {
                fprintf(stderr, "SelectExpr::typeCheck(): illegal iterOffset in line no:%d\n", stmtCount);
                return -1;
        }
        /* Now we have to copy the child's 'resultInfo' into the parent's
           field. I have defined operator= for 'TupleDef' */
        resultInfo = child->resultInfo; // that is the overloaded operator=
        if(cond && ((cond->typeCheck(resultInfo)).type != D_Truthval))
        {
                fprintf(stderr,
                  "Type checking of Select expression's condition failed in Stmt no:%d\n"
                  , stmtCount);
                return -1;
        }
        else
                return iterOffset + 1;
}

bool
SelectExpr::rewrite()
{
        bool retVal = child->rewrite();
        Expr *newExpr;
        if(cond && ((newExpr = cond->rewrite(resultInfo, RESULT)) != cond))
                return (((cond = newExpr) != NULL) && retVal);
        else
                return retVal;
}

/***************/

void JoinExpr::Constructor(Expr *cond_a, RelExpr *left_a, RelExpr *right_a)
{
        RelExpr::Constructor(D_JOIN);
        cond = cond_a;
        left = left_a;
        right = right_a;
        return;
}

JoinExpr * JoinExpr::create(Expr *cond, RelExpr *left, RelExpr *right) 
{
        JoinExpr * v = (JoinExpr *) new char[sizeof(JoinExpr)];
        v->Constructor(cond, left, right);
        return v;
}

bool
JoinExpr::print(FILE *file) 
{
        putc('(', file);
        fprintf(file, " join[ "); 
        if(cond) 
        {
            cond->print(file);
        }
        fprintf(file, "] ");
        putc('(', file);
        left->print(file);
        fprintf(file, ") JOIN ");
        putc('(', file);
        right->print(file);
        putc(')', file);
        putc(')', file);
        return LocalPrint(file);
}

//int JoinExpr::typeCheck(int iterOffset_a, TransID &tid_a)
int JoinExpr::typeCheck(int iterOffset_a)
{
        int tempIterOffset;
//      if((tempIterOffset = left->typeCheck(iterOffset_a, tid_a)) < 0)
        if((tempIterOffset = left->typeCheck(iterOffset_a)) < 0)
        {
                fprintf(stderr, "JoinExpr::typeCheck(): illegal iterOffset in line no:%d\n", stmtCount);
                return -1;
        }
//      if((iterOffset  = right->typeCheck(tempIterOffset, tid_a)) < 0)
        if((iterOffset  = right->typeCheck(tempIterOffset)) < 0)
        {
                fprintf(stderr, "JoinExpr::typeCheck(): illegal iterOffset in line no:%d\n", stmtCount);
                return -1;
        }
        resultInfo =  left->resultInfo;
        resultInfo += right->resultInfo;
        /* Those were the overloaded operator= and operator+= */

        if(cond && ((cond->typeCheck(resultInfo)).type == D_UnknownType))
        {
                fprintf(stderr, 
                  "Type checking of Join expression's condition failed in Stmt no:%d\n"
                  , stmtCount);
                return -1;
        }
        else
                return iterOffset + 1;
}

bool
JoinExpr::rewrite()
{
        bool retVal = left->rewrite() && right->rewrite();
        Expr *newExpr;
        if(cond && ((newExpr = cond->rewrite(resultInfo, RESULT)) != cond))
                return (((cond = newExpr) != NULL) && retVal);
        else
                return retVal;
}

/***************/

bool 
ProjectElem::print(FILE *out_a)
{
        projectionExpr->print(out_a);
        if(newName)
                fprintf(out_a, " AS %s", newName);
        return true;
}

bool 
ProjectElem::rewrite(TupleDef &bindings_a, ArgSource source_a)
{
        Expr *newExpr;
        if(((newExpr = projectionExpr->rewrite(bindings_a, source_a)) 
          != projectionExpr) && ((projectionExpr = newExpr) == NULL))
        {
                fprintf(stderr, "Rewriting of projection expression failed in stmt"
                  " no: %d\n", stmtCount);
                return false;
        }
        return true;
}

bool 
ProjectElem::typeCheck(TupleDef &bindings_a, ArgDef &result)
{
        if((result.argType = projectionExpr->typeCheck(bindings_a)).type ==
          D_UnknownType)
        {
                fprintf(stderr, "Type checking of projection expression failed in "
                  "Stmt No:%d\n", stmtCount);
                return false;
        }
        else
        {
                /* Now we have to set $rangeVariable$ and $argName$ */
                /* FIX: This is done in a half-hearted manner for now */
                if(projectionExpr->exprType == Expr::D_REL_ARG_REF)
                {
                        strcpy(result.argName, 
                          ((RelArgRef *)projectionExpr)->attrName);
                        strcpy(result.rangeVariable,
                          ((RelArgRef *)projectionExpr)->relRef);
                        result.defaultValue = 
                          bindings_a.args[((RelArgRef *)projectionExpr)->GetMatchPos(
                          bindings_a)].defaultValue;
                }
                else
                {
                        strcpy(result.rangeVariable, "");
                        strcpy(result.argName, "");
                        result.defaultValue = projectionExpr->type.DefaultValue();
                }

                if(newName)
                        strcpy(result.argName, newName);
                return true;
        }
}

/***************/

void
GeneralProjectExpr::Constructor(GeneralProjectExprOp op_a, 
  ProjectElemList *projList_a, RelExpr *child_a)
{
        RelExpr::Constructor(D_GEN_PROJECT);
        op = op_a;

        projList.numProjectElems = projList_a->length();
        // added by prasan ...
        if( projList.numProjectElems ) {
            projList.projectElems = new ProjectElem[projList.numProjectElems];
            Listiter<ProjectElem> projIter(*projList_a);
            for(int i = 0; !projIter.at_end(); ++i)
                    projIter.next(projList.projectElems[i]);
        } else projList.projectElems = NULL;

        delete projList_a;
                
        child    = child_a;
        return;
}

bool
GeneralProjectExpr::print(FILE *file)
{
        switch(op)
        {
          case D_PROJECT:       
                return ((ProjectExpr *)this)->print(file); 
                break; 
          case D_GROUP_AGG:
                return ((GroupAggExpr *)this)->print(file);
            default: break;
        }
        return false;
}

int
//GeneralProjectExpr::typeCheck(int iterOffset_a, TransID &tid_a)
GeneralProjectExpr::typeCheck(int iterOffset_a)
{
        switch(op)
        {
          case D_PROJECT:
//              return ((ProjectExpr *)this)->typeCheck(iterOffset_a, tid_a);
                return ((ProjectExpr *)this)->typeCheck(iterOffset_a);
          case D_GROUP_AGG:
//              return ((GroupAggExpr *)this)->typeCheck(iterOffset_a, tid_a); 
                return ((GroupAggExpr *)this)->typeCheck(iterOffset_a);
            default: break;
        }
        return 0;
}

bool
GeneralProjectExpr::rewrite()
{
        switch(op)
        {
          case D_PROJECT:
                return ((ProjectExpr *)this)->rewrite();
          case D_GROUP_AGG:
                return ((GroupAggExpr *)this)->rewrite(); 
          default: break;
        }
        return false;
}

int
//GeneralProjectExpr::LocalTypeCheck(int iterOffset_a, TransID &tid_a)
GeneralProjectExpr::LocalTypeCheck(int iterOffset_a)
{
        if(resultInfo.numArgs != 0)
        {
                resultInfo.numArgs = 0;
                delete resultInfo.args;
        }
        resultInfo.numArgs = projList.numProjectElems;
        resultInfo.args = (ArgDef*) new char [sizeof(ArgDef)*resultInfo.numArgs];
        
//      int retVal = child->typeCheck(iterOffset_a, tid_a);
        int retVal = child->typeCheck(iterOffset_a);

        int i = 0;
        bool typeCheckSuccess = true;
        for(typeCheckSuccess = true; 
          (i < projList.numProjectElems) && typeCheckSuccess; ++i)
                typeCheckSuccess = (typeCheckSuccess && 
                  projList.projectElems[i].typeCheck(
                  child->resultInfo, resultInfo.args[i]));

        if(!typeCheckSuccess)
        {
                fprintf(stderr, "Type checking of $GeneralProjectExpr$'s projection"
                  " attribute failed in stmt no: %d\n", stmtCount);
                return -1;
        }
        return retVal;
}

int
GeneralProjectExpr::LocalPrint(FILE *out_a)
{
        switch(op)
        {
          case D_PROJECT:
                fprintf(out_a, " project[("); 
                break;
          case D_GROUP_AGG:
                fprintf(out_a, " groupagg[("); 
            break;
          default:
                fprintf(stderr, "ILLEGAL operator %d", op);
                return -1;
        }

        int i = -1;
        for(i = 0; i < projList.numProjectElems - 1; ++i)
        {
                projList.projectElems[i].print(out_a);
                fprintf(out_a, ", ");
        }
        if(projList.numProjectElems > 0)
                projList.projectElems[i].print(out_a);
        fprintf(out_a, ")]");
        return 0;
}

bool
GeneralProjectExpr::LocalRewrite()
{
        bool retVal = child->rewrite();
        for(int i = 0; i < projList.numProjectElems; ++i)
        {
                if(!projList.projectElems[i].rewrite(child->resultInfo, LEFT))
                {
                        fprintf(stderr, "GeneralProjectExpr::rewrite() failed in "
                          "stmt no: %d\n", stmtCount);
                        return false;
                }
        }
        return retVal;
}

/**********************/

void ProjectExpr::Constructor(ProjectElemList *pList_a, RelExpr *child_a)
{
        GeneralProjectExpr::Constructor(D_PROJECT, pList_a, child_a);
        return;
}

ProjectExpr * ProjectExpr::create(ProjectElemList *plist, RelExpr *child) 
{
        ProjectExpr * v = (ProjectExpr *) new char[sizeof(ProjectExpr)];
        v->Constructor(plist, child);
        return v;
}

bool
ProjectExpr::print(FILE *file) 
{
        GeneralProjectExpr::LocalPrint(file);
        putc('(', file);
        child->print(file);
        putc(')', file);
        return SQLStmt::LocalPrint(file);
}

//int ProjectExpr::typeCheck(int iterOffset_a, TransID &tid_a)
int ProjectExpr::typeCheck(int iterOffset_a)
{
//      if((iterOffset = GeneralProjectExpr::LocalTypeCheck(iterOffset_a, tid_a))
        if((iterOffset = GeneralProjectExpr::LocalTypeCheck(iterOffset_a))
          < 0)
                return -1;
        else
                return iterOffset + 1;
}

bool
ProjectExpr::rewrite()
{
        return GeneralProjectExpr::LocalRewrite();
}

/***************/

void GroupAggExpr::Constructor(Tuple *gBAttrs_a, ProjectElemList *aggList_a, 
        RelExpr *child_a)
{
        GeneralProjectExpr::Constructor(D_GROUP_AGG, aggList_a, child_a);
        groupByAttrs = gBAttrs_a;
        return;
}

GroupAggExpr * GroupAggExpr::create(Tuple *gblist, ProjectElemList *aggList, 
                                        RelExpr *child) 
{
        GroupAggExpr * v = (GroupAggExpr *) new char[sizeof(GroupAggExpr)];
        v->Constructor(gblist, aggList, child);
        return v;
}

bool
GroupAggExpr::print(FILE *file) 
{
        GeneralProjectExpr::LocalPrint(file);
        putc('[', file);
        if(groupByAttrs) {
           groupByAttrs->print(file);
        }
        fprintf(file, "](");
        child->print(file);
        fprintf(file, ")");
        return SQLStmt::LocalPrint(file);
}

//int GroupAggExpr::typeCheck(int iterOffset_a, TransID &tid_a)
int GroupAggExpr::typeCheck(int iterOffset_a)
{
//      if((iterOffset = GeneralProjectExpr::LocalTypeCheck(iterOffset_a, tid_a)) 
        if((iterOffset = GeneralProjectExpr::LocalTypeCheck(iterOffset_a)) 
          >= 0)
        {
                for(int i = 0; i < groupByAttrs->numAttrs; ++i)
                        if((groupByAttrs->attrs[i]->typeCheck(child->resultInfo)).type ==
                           D_UnknownType)
                        {
                                fprintf(stderr,
                                  "GroupBy expression fails to type check correctly in "
                                  "stmt no: %d", stmtCount);
                                return -1;
                        }

                /* FIX: It is not enuf to just typeCheck $groupByAttrs->attrs[i]$. 
                   We should also check whether it is single valued per group.
                   Yet to be done. */

                return iterOffset + 1;
        }
        else
                return -1;
}

bool
GroupAggExpr::rewrite()
{
        bool retVal = GeneralProjectExpr::LocalRewrite();
        
        for(int i = 0; (i < groupByAttrs->numAttrs) && retVal; ++i)
        {
                Expr *newExpr;
                if((newExpr = groupByAttrs->attrs[i]->rewrite(child->resultInfo, 
                  LEFT)) != groupByAttrs->attrs[i])
                        retVal = (retVal && ((groupByAttrs->attrs[i] = newExpr) != NULL));
        }

        return retVal;
}

/***************/

void DupelimExpr::Constructor(RelExpr *child_a)
{
        RelExpr::Constructor(D_DUPELIM);
        child = child_a;
        return;
}

DupelimExpr *
DupelimExpr::create(RelExpr *child) 
{
        DupelimExpr *v = (DupelimExpr *) new char[sizeof(DupelimExpr)];
        v->Constructor(child);
        return v;
}

//int DupelimExpr::typeCheck(int iterOffset_a, TransID &tid_a)
int DupelimExpr::typeCheck(int iterOffset_a)
{
//      if((iterOffset = child->typeCheck(iterOffset_a, tid_a)) < 0)
        if((iterOffset = child->typeCheck(iterOffset_a)) < 0)
        {
                fprintf(stderr, "DupeLimExpr::typeCheck(): illegal iterOffset in line no:%d\n", stmtCount);
                return -1;
        }
        /* Now we have to copy the child's 'resultInfo' into the parent's
           field. I have defined operator= for 'TupleDef' */
        resultInfo = child->resultInfo; // that is the overloaded operator=
        return iterOffset + 1;
}

bool
DupelimExpr::print(FILE *file) 
{
        fprintf(file, " dupelim("); 
        child->print(file);
        fprintf(file, ")"); 
        return LocalPrint(file);
}
/***************/

void SortExpr::Constructor(Tuple *sortAttrs_a, int dir_a, RelExpr *child_a)
{
        RelExpr::Constructor(D_SORT);
        sortAttrs = sortAttrs_a;
        direction = dir_a;
        child = child_a;
        return;
}

SortExpr * SortExpr::create(Tuple *slist, int dir, RelExpr *child) 
{
        SortExpr * v = (SortExpr *) new char[sizeof(SortExpr)];
        v->Constructor(slist, dir, child);
        return v;
}

bool
SortExpr::print(FILE *file) 
{
        fprintf(file, " sort[ "); 
        if(sortAttrs) {
           sortAttrs->print(file);
        }
        fprintf(file, "][");
        if (direction == 1) {
            fprintf(file, "ASC");
        }
        else {
            fprintf(file, "DESC");
        }
        fprintf(file, "](");
        child->print(file);
        fprintf(file, ")");
        return LocalPrint(file);
}

//int SortExpr::typeCheck(int iterOffset_a, TransID &tid_a)
int SortExpr::typeCheck(int iterOffset_a)
{
//      if((iterOffset = child->typeCheck(iterOffset_a, tid_a)) < 0)
        if((iterOffset = child->typeCheck(iterOffset_a)) < 0)
        {
                fprintf(stderr, "SortExpr::typeCheck(): illegal iterOffset in line no:%d\n", stmtCount);
                return -1;
        }

        /* Now we have to copy the child's 'resultInfo' into the parent's
           field. I have defined operator= for 'TupleDef' */
        resultInfo = child->resultInfo; // that is the overloaded operator=

        for(int i = 0; i < sortAttrs->numAttrs; ++i)
                if((sortAttrs->attrs[i]->typeCheck(resultInfo)).type == D_UnknownType)
                {
                        fprintf(stderr, "Sort expression failed to typecheck in "
                          "Stmt No:%d\n" , stmtCount);
                        return -1;
                }
        return iterOffset + 1;
}

bool
SortExpr::rewrite()
{
        bool retVal = child->rewrite();
        for(int i = 0; i < sortAttrs->numAttrs; ++i)
        {
                Expr *newExpr;
                if((newExpr = sortAttrs->attrs[i]->rewrite(resultInfo, RESULT))
                  != sortAttrs->attrs[i])
                        retVal = (retVal && (( sortAttrs->attrs[i] = newExpr) != NULL));
        }
        return retVal;
}

/***************/

void RelValueExpr::Constructor(Tuple *value_a)
{
        RelExpr::Constructor(D_REL_VALUES);
        value = value_a;
        return;
}

RelValueExpr * RelValueExpr::create(Tuple *value)
{
        RelValueExpr * v = (RelValueExpr *) new char[sizeof(RelValueExpr)];
        v->Constructor(value);
        return v;
}

bool
RelValueExpr::print(FILE *file) 
{
        fprintf(file, " VALUES "); 
        if(value)
           value->print(file);
        return true;
}

//int RelValueExpr::typeCheck(int iterOffset_a, TransID &tid_a)
int RelValueExpr::typeCheck(int iterOffset_a)
{
        if((iterOffset = iterOffset_a) < 0)
        {
                fprintf(stderr," RelValueExpr::typeCheck(): Invalid iter_offset "
                  "passed as argument in Stmt no:%d\n", stmtCount);
                return -1;
        }
        resultInfo.numArgs = value->numAttrs;
        resultInfo.args = (ArgDef *) new char[sizeof(ArgDef) * resultInfo.numArgs];
        for(int i = 0; i < resultInfo.numArgs; ++i)
        {
                if((value->attrs[i]->exprType != Expr::D_VALUE) || 
                  ((resultInfo.args[i].argType = value->attrs[i]->type).type == 
                  D_UnknownType))
                        return -1;
                resultInfo.args[i].defaultValue = 
                  resultInfo.args[i].argType.DefaultValue();
        }
        /* A $RelValueExpr$ cannot have any $RelArgRef$s to replace by
           $LocalArgRef$s. */
        return iterOffset + 1;
}

/***************/
