/*      -*- c++ -*-  */
#include "sql_parse.h"
/**************************************************************************/

/** 
  FIX: All the create functions below will later change to return DaliPtr's
**/
template<class T>
void printList(FILE *file_a, List<T> &list_a)
{
        putc('(', file_a);
        Listiter<T> listIter(list_a);
        while(true)
        {
                T *temp;
                listIter.next(temp);
                temp->print(file_a);
                if(listIter.at_end())
                        break;
                fprintf(file_a, ", ");
        }
        putc(')', file_a);
        return;
}

void
InsertTable::Constructor(const char *relName_a, Tuple *argList_a, 
        RelExpr *tableExpr_a)
{
        NonRelExpr::Constructor(D_INSERT);
        strcpy(relName, relName_a);
        argList = argList_a;
        tableExpr = tableExpr_a;
        return;
}

InsertTable * 
InsertTable::create(const char *relName, Tuple *argList,
                                         RelExpr *tableExpr) 
{
    InsertTable *v = (InsertTable *) new char[sizeof(InsertTable)];
        v->Constructor(relName, argList, tableExpr);
    return v;
}

bool
InsertTable::typeCheck(TransID &tid_a)
{
        if(tableExpr->typeCheck(0, tid_a) < 0)
                return false;
        if(GetRelInfo(relName, tid_a, resultInfo) == false)
                SQLErrAR("Invalid table name %s in Insert stmt\n", relName, false);

        bool typeCheckingFailed = false;
        if(argList)
        {
                if(argList->numAttrs != tableExpr->resultInfo.numArgs)
                        typeCheckingFailed = true;      
                for(int sourceFieldNum = 0; (sourceFieldNum < argList->numAttrs) && 
                  !typeCheckingFailed; ++sourceFieldNum)
                        typeCheckingFailed = (typeCheckingFailed ||
                          (argList->attrs[sourceFieldNum]->exprType != Expr::D_REL_ARG_REF) 
                          || !ArgType::IsAsgCompatible(argList->attrs[sourceFieldNum]->
                          typeCheck(resultInfo), tableExpr->
                          resultInfo.args[sourceFieldNum].argType));
        }
        else
        {
                if(resultInfo.numArgs != tableExpr->resultInfo.numArgs)
                        typeCheckingFailed = true;      
                for(int sourceFieldNum = 0; (sourceFieldNum < resultInfo.numArgs) 
                  && !typeCheckingFailed; ++sourceFieldNum)
                        typeCheckingFailed = (typeCheckingFailed || 
                          !ArgType::IsAsgCompatible(resultInfo.args[sourceFieldNum].argType,
                          tableExpr->resultInfo.args[sourceFieldNum].argType));
        }

        if(typeCheckingFailed)
                SQLErrR("type checking of insert-table failed", false)
        else
                return true;
}

bool 
InsertTable::rewrite() 
{
        bool retVal = tableExpr->rewrite(); 
        if(argList)
                for(int i = 0; i < argList->numAttrs; ++i)
                {
                        Expr *newExpr;
                        if(((newExpr = argList->attrs[i]->rewrite(resultInfo, RESULT)) 
                        != argList->attrs[i]) && ((argList->attrs[i] = newExpr) == NULL))
                                SQLErrR("rewriting of insert-table failed", false)
                }
        return retVal;
}

/***************/

void
DeleteTable::Constructor(const char *relName_a, Expr *where_a)
{
        NonRelExpr::Constructor(D_DELETE);
        relScan = RelScanExpr::create(relName_a, where_a);
        return;
}

DeleteTable *
DeleteTable::create(const char *relName_a, Expr *whereCond_a) 
{
        DeleteTable * v = (DeleteTable *) new char[sizeof(DeleteTable)];
        v->Constructor(relName_a, whereCond_a);
        return v;
}

bool
DeleteTable::print(FILE *file) 
{
    fprintf(file, " DELETE FROM %s ", relScan->name); 
    if(relScan->cond) {
        fprintf(file, " WHERE "); 
        relScan->cond->print(file);
    }
    fprintf(file, "\n");
    return true;
}

/***************/

void 
UpdateExpr::print(FILE *out_a)
{
        attribute->print(out_a);
        fprintf(out_a, " := ");
        rhs->print(out_a);
        return;
}

bool 
UpdateExpr::typeCheck(TupleDef &bindings_a)
{
        if(attribute->exprType != Expr::D_REL_ARG_REF)
                return false;
    if(((attribute->typeCheck(bindings_a)).type == D_UnknownType) ||
      ((rhs->typeCheck(bindings_a)).type == D_UnknownType))
                return false;
        return ArgType::IsAsgCompatible(attribute->type, rhs->type);
}

bool
UpdateExpr::rewrite(TupleDef &bindings_a, ArgSource source_a)
{
        bool retVal = 
          ((attribute = attribute->rewrite(bindings_a, source_a)) != NULL);
        Expr *newExpr;
        if((newExpr = rhs->rewrite(bindings_a, source_a)) != rhs)
                retVal = (retVal && ((rhs = newExpr) != NULL));
        return retVal;
}

/***************/

void
UpdateTable::Constructor(const char *relName_a, UpdateExprList *avl_a, 
  Expr *where_a)
{
        NonRelExpr::Constructor(D_UPDATE);
        relScan = RelScanExpr::create(relName_a, where_a);

        attrValList.numUpdateExprs = avl_a->length();
        attrValList.updateExprs = new UpdateExpr[attrValList.numUpdateExprs];
        Listiter<UpdateExpr> attrValIter(*avl_a);
        for(int i = 0; !attrValIter.at_end(); ++i)
                attrValIter.next(attrValList.updateExprs[i]);
        delete avl_a;
        return;
}

UpdateTable *
UpdateTable::create(const char *relName, UpdateExprList *avl,
                                 Expr *whereCond) 
{
    UpdateTable *v = (UpdateTable *) new char[sizeof(UpdateTable)];
        v->Constructor(relName, avl, whereCond);
    return v;
}

bool
UpdateTable::print(FILE *out_a) 
{
    fprintf(out_a, " UPDATE %s SET (", relScan->name); 
        for(int i = 0; i < attrValList.numUpdateExprs - 1; ++i)
        {
                attrValList.updateExprs[i].print(out_a);
                fprintf(out_a, ", ");
        }
    if(attrValList.numUpdateExprs > 0) 
                attrValList.updateExprs[i].print(out_a);
    if(relScan->cond) {
        fprintf(out_a, ") WHERE "); 
                relScan->cond->print(out_a);
    }
    fprintf(out_a, "\n");
    return true;
}

bool
UpdateTable::typeCheck(TransID &tid_a)
{
        if(((RelExpr *)relScan)->typeCheck(0, tid_a) < 0)
                return false;
        resultInfo = ((RelExpr *)relScan)->resultInfo;

        for(int i = 0; i < attrValList.numUpdateExprs; ++i)
                if(!attrValList.updateExprs[i].typeCheck(resultInfo))
                        SQLErrR("Type checking of update-expr failed", false);

        return true;
}

bool 
UpdateTable::rewrite()
{
        for(int i = 0; i < attrValList.numUpdateExprs; ++i)
                if(!attrValList.updateExprs[i].rewrite(resultInfo, RESULT))
                        SQLErrR("Re-writing of $UpdateExpr$ failed", false);

        return ((RelExpr *)relScan)->rewrite();
}

/***************/

#define QUANTUM (50)
Table_descriptor::~Table_descriptor()
{
    if(name)
                free(name);
    name = 0;
    if(dbfile_name)
                free(dbfile_name);
        dbfile_name = 0;
    schema.destroy();
}

View_descriptor::~View_descriptor()
{
    if(name)
                free(name);
    if(sourceName)
                free(sourceName);
    schema.destroy();
}

MTM_descriptor::MTM_descriptor()
        : name(0), fromTableName(0), toTableName(0), addPointers(0),
          oneToManyFlag(0)
{
    fromAttrList.destroy();
    toAttrList.destroy();    
}

MTM_descriptor::~MTM_descriptor()
{
    if(name)
                free(name);
    if(fromTableName)
                free(fromTableName);
    if(toTableName)
                free(toTableName);
    fromAttrList.destroy();
    toAttrList.destroy();
}

RI_descriptor::RI_descriptor()
        : name(0), fromTableName(0), toTableName(0)
{
    fromTableList.destroy();
    toTableList.destroy();
}

RI_descriptor::~RI_descriptor()
{
    if(name)
                free(name);
    if(fromTableName)
                free(fromTableName);    
    if(toTableName)
                free(toTableName);
    fromTableList.destroy();
    toTableList.destroy();
}

Index_descriptor::Index_descriptor()
        : name(0), tableName(0)
{
    attrList.destroy();
}

Index_descriptor::~Index_descriptor()
{
    if(name)
                free(name);
    attrList.destroy();
}

/***************************/

int MultiTableSchema::addTable(Table_descriptor *ntable)
{
    if(tableCount>= tableMax) {
        tableMax += QUANTUM;
        tableList = (Table_descriptor **) realloc(tableList,
                                           tableMax * sizeof(Table_descriptor));
    }
    tableList[tableCount] = ntable;  
    tableCount++;
    return 1;
}

int MultiTableSchema::addView(View_descriptor *nview)
{
    if(viewCount>= viewMax) {
        viewMax += QUANTUM;
        viewList = (View_descriptor **) realloc(viewList,
                                           viewMax * sizeof(View_descriptor));
    }
    viewList[viewCount] = nview;  
    viewCount++;
    return 1;
}

int MultiTableSchema::addMTM(MTM_descriptor *nmtm)
{
    if(mtmCount>= mtmMax) {
        mtmMax += QUANTUM;
        mtmList = (MTM_descriptor **) realloc(mtmList,
                                           mtmMax * sizeof(MTM_descriptor));
    }
    mtmList[mtmCount] = nmtm;  
    mtmCount++;
    return 1;
}

int MultiTableSchema::addRI(RI_descriptor *nri)
{
    if(riCount>= riMax) {
        riMax += QUANTUM;
        riList = (RI_descriptor **) realloc(riList,
                                           riMax * sizeof(RI_descriptor));
    }
    riList[riCount] = nri;  
    riCount++;
    return 1;
}

int MultiTableSchema::addIndex(Index_descriptor *nindex)
{

    if(indexCount>= indexMax) {
        indexMax += QUANTUM;
        indexList = (Index_descriptor **) realloc(indexList,
                                           indexMax * sizeof(Index_descriptor));
    }
    indexList[indexCount] = nindex;  
    indexCount++;
    return 1;

}

void
PrintIndexType(DaliIndexType indexType_a, FILE *out_a)
{
        switch(indexType_a)
        {
          case DALI_HASH:
                fprintf(out_a, "hash"); break;
          case DALI_TTREE:
                fprintf(out_a, "ttree"); break;
          case DALI_UNIQUE_HASH:
                fprintf(out_a, "unique hash"); break;
          case DALI_UNIQUE_TTREE:
                fprintf(out_a, "unique ttree"); break;
          default:
                fprintf(out_a, "<unknown index type>");
        }
}

bool 
MultiTableSchema::evaluate(TransID &tid)
{
    int err;
    for(int i = 0;  i < tableCount; i++) 
        {
                Table_descriptor *td = tableList[i];
                err = DaliRelMgr::createTable(tid,td->name, td->schema, 
                  td->dbfile_name);
                if(err >= 0) 
                        fprintf(stdout, "Created table %s in statement no: %d\n\n",
                          td->name, stmtCount);
                else
                        SQLErrAR("Cannot create table %s", td->name, false)
    }

        for(i = 0;  i < indexCount; i++) 
        {
                Index_descriptor *id = indexList[i];
                err = DaliRelMgr::createIndex(tid,id->name, id->tableName,
                                                  id->attrList,id->type);
                if(err >= 0)
                {
                        fprintf(stdout, "Created ");
                        PrintIndexType(id->type, stdout);
                        fprintf(stdout, " index %s on table %s in statement no: "
                          "%d\n\n", id->name, id->tableName, stmtCount);
                }
                else
                        SQLErrAAR("Cannot create index %s on table %s", id->name, 
                          id->tableName, false)
    }

#if 0

    for(i = 0;  i < viewCount; i++) {
        View_descriptor *td = viewList[i];
        printf("Creating view %s\n",td->name);
        // td->sch.print(stdout);
        err = DaliRelMgr::createView(tid,td->name, td->sourceName,
                                     td->sch);
        if(err < 0) {
            printf("Cannot create view %s: %d\n",td->name,err);
            return -1;
        }
        /*
          DaliTableHandle newtab;
          DaliRelMgr::openTable(tid,$3,newtab);
          cout << * newtab.schemaInfo;
        */
    }

        for(i = 0;  i < mtmCount; i++) {
        MTM_descriptor *md = mtmList[i];
        printf("Creating inter-table mapping %s\n",md->name);
        err = DaliRelMgr::createOneToMany(tid,md->name, 
                                          md->fromTableName,
                                          md->fromAttrList,
                                          md->toTableName,
                                          md->toAttrList,
                                          md->addPointers,
                                          md->oneToManyFlag
                                          );
        if(err < 0) {
            printf("Cannot create mapping %s: %d\n",md->name,err);
            return -1;
        }
    }

    for(i = 0;  i < riCount; i++) {
        RI_descriptor *rd = riList[i];
        if(rd->name == 0) {
                char buf[1000];
            sprintf(buf,"TC_%d",i);
            rd->name = strdup(buf);
        }
        printf("Creating integrity constraint %s\n",rd->name);
        err = DaliRelMgr::createReferentialIntegrity(tid,
                                /* ri->name, */
                                                     rd->fromTableName,
                                                     rd->fromTableList,
                                                     rd->toTableName,
                                                     rd->toTableList);
        if(err < 0) {
            printf("Cannot create integrity constraint %s: %d\n",rd->name,err);
            return -1;
        }
    }
#endif

    return true;

}

MultiTableSchema::~MultiTableSchema()
{
    for(int i = 0; i < tableCount; i++) 
                delete tableList[i];
    free(tableList);  // called realloc above, so using free here

    for(i = 0; i < viewCount; i++)
                delete viewList[i];
    free(viewList);  // called realloc above, so using free here

    for(i = 0; i < mtmCount; i++)
                delete mtmList[i];
    free(mtmList);

    for(i = 0; i < riCount; i++)
                delete riList[i];
    free(riList);

    for(i = 0; i < indexCount; i++)
                delete indexList[i];
    free(indexList);
}

/********************/

bool
CreateTableSpace::evaluate(TransID &)
{
        DbInitInfo info;
        if(size > 0) 
                info.max_db_size = size;
        DaliDB *db= DaliDB::open(name, _DB_CREAT, &info);
        if(db)
        {
                db->close();
                fprintf(stdout, "Created table-space:%s in statement no:%d\n",
                  name, stmtCount);
                return true;
        }
        else
                SQLErrAE("Unable to create table-space:%s", name, -1)
}

/********************/

bool
DropDataBase::evaluate(TransID &)
{
        if((DaliDB::unlink("CatalogDB") < 0) || (DaliDB::unlink("REL_DB") < 0))
                SQLErrE("Unable to drop database", -1)
        else
        {
                fprintf(stdout, "Dropped Data Base in statement no:%d\n", stmtCount);
                return true;
        }
}

/********************/

bool
DropTableSpace::evaluate(TransID &)
{
        /*FIX: these checks can be fooled with simple path name changes. */
        if(!strcmp(name, "CatalogDB"))
                SQLErrR("Cannot drop CatalogDB, use DROP DATABASE", false)
        if(!strcmp(name, "REL_DB"))
                SQLErrR("Cannot drop REL_DB, use DROP DATABASE", false)
        if(!strcmp(name, SysDB::sysdb_start->name()))
                SQLErrR("Cannot drop the system database!!!", false)

        DALI_R_ASSERT(DaliDB::unlink(name) >= 0);
        fprintf(stdout, "Dropped table-space:%s in statement no:%d\n", 
          name, stmtCount);
        return true;
}

/********************/
