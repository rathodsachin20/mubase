// catalog manager

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>

#include "List.h"
#include "LogProp.h"
#include "PhyProp.h"
#include "Catalog.h"

Catalog_t *Catalog = NULL;
int trace_print = 0;

int LoadCatalogInfo(char *cataLogfile=NULL);

enum InputInfoType {
        NOTKNOWN,
        DISTINCT,
        SIZE_OF_REL,
        SIZE_OF_TUPLE,
        ATTR_MAX_VALUE, 
        ATTR_MIN_VALUE,
        NUM_OF_BLOCKS,
        TUPLES_PER_BLOCK,
        ATTR_SIZE_BYTES,
        INDEXINFO,
        MEM_BLOCKS
};

// load the catalog info
Catalog_t::Catalog_t(char *catalogFile)
{ LoadCatalogInfo(catalogFile); }

// long function --- all this does is to get the logical property
// information from earlier structures, and return it packaged in the
// new structures
LogProp_t* Catalog_t::GetRelLogProp(const char* relName)
{
    assert(relName);
    BaseRelLogProp_t *relLogProp = NULL;

    // first search in cached schemas
    ListIter_t<BaseRelLogProp_t *> iter;
    iter.Attach(&relLogPropList);

    while( !iter.IsEnd() ) {
        BaseRelLogProp_t *logProp = iter.Next();
        const char *name = logProp->Name();

        assert(name);
        if( !strcmp(name, relName) ) {
            // names match
            relLogProp = logProp;
            break;
        }
    }

    if( relLogProp ) {
        // logical property was cached in relLogPropList
        return relLogProp;
    }

    // logical property not cached in relLogPropList --- compute it and cache
            
    // get the schema entry
    Schema_Def* oldSchema = Catalog::Rel_Schemas;

    while( oldSchema ) {
        if( !strcmp(oldSchema->RelName, relName) )
            break;
        oldSchema = oldSchema->NextRel;
    }

    // if schema not found, give up
    if( !oldSchema ) return NULL;

    // find the number of attributes
    int numAttr = 0;
    Attribute* p = oldSchema->Attributes;
    while( p ) {
        numAttr++;
        p = p->Next;
    }

    // construct the schema in new format
    Schema_t *schema = new Schema_t(numAttr);
    Refer(schema);

    // schema property
    SchemaProp_t *schemaProp = new SchemaProp_t(numAttr);
    Refer(schemaProp);

    int i = 0;
    p = oldSchema->Attributes;
    while( p ) {
        Attribute_Def1 *a = p->AttrPtr;
        assert(a);
        CatalogInfoForAttr *c = a->catalogInfo;
        assert(c);

        Attr_t *attr = new Attr_t(a->relName, a->attrName, c->size);
        Refer(attr);
        schema->SetAttribute(i, attr);
        DeRefer(attr);


        AttrProp_t *attrProp = NULL;

        // default type taken as integer
        switch( a->Type ) {
            case O_INTEGER :
            default :
                attrProp = new IntegerProp_t(int(c->minValue),
                                        int(c->maxValue), c->noOfDistinct);
                break;
                // flush(cout << "Invalid attribute type" << endl);
                // abort();
        }

        Refer(attrProp);
        schemaProp->SetAttributeProp(i, attrProp);
        DeRefer(attrProp);

        i++;
        p = p->Next;
    }

    // get the size-info entry
    CatalogInfoForRel* relInfo = Catalog::Rel_Sizes;

    while( relInfo ) {
        if( !strcmp(relInfo->RelName, relName) )
            break;
        relInfo = relInfo->Next;
    }
    assert(relInfo);

    Size_t *oldSize = relInfo->RelSize;
    assert(oldSize);

    // construct the logical property
    relLogProp = new BaseRelLogProp_t(relName, schema, schemaProp,
                            oldSize->numTuples, oldSize->tuplesPerBlock);

    // add the built-in index info
    int order[100];
    for( int j = 0 ; j < relInfo->NumIndexes ; j++ ) {
        // add the j-th index
        // create the order

        int size = relInfo->IndexAttrSizeList[j];
        assert(size > 0);
        char **attrList = relInfo->IndexAttrList[j];
        assert(attrList);

        for( int k = 0 ; k < size ; k++ ) {
            char *s = attrList[k];
            assert(s);

            // find the attrnum for s in the schema
            order[k] = -1;
            for(int l = 0 ; l < numAttr ; l++ ) {
                Attr_t *a = schema->Attribute(l);
                assert(a);

                char *aName = a->Name();
                assert(aName);

                if( strcmp(s, aName) == 0 ) {
                    order[k] = l;
                    break;
                }
            }
            if( order[k] == -1 )
                flush(cout << "NOT FOUND: " << relName << ":" << s << endl);
            assert(order[k] != -1);
        }

        IndexOrder_t *indexOrder = new IndexOrder_t(order, size);
        Refer(indexOrder);

        relLogProp->AddBuiltInIndex(indexOrder);

        // Ravi:Debug:Printing the indexOrder being added for debug purpose
        // cout << "\n\n------------------\n";
        // cout << "Added the following Index Order as a built-in index" << endl;
        for (int ri = 0; ri < indexOrder->OrderSize(); ri++) {
                int attrIndex = indexOrder->Order()[ri];
                Attr_t *idxAttr = schema->Attribute(attrIndex);
                // cout << "Relation Name:" << idxAttr->RelName() << endl;
                // cout << "Attribute Name:" << idxAttr->Name() << endl;
        }
        // cout << "\n\n------------------\n";
        //Ravi:Debug:End

        DeRefer(indexOrder);
    }

    DeRefer(schema);
    DeRefer(schemaProp);

    // insert in the list
    Refer(relLogProp);
    relLogPropList.Insert(relLogProp);

    return relLogProp;
}

// derefers all the catalog information
Catalog_t::~Catalog_t(void)
{
    ListIter_t<BaseRelLogProp_t *> iter;
    iter.Attach(&relLogPropList);

    while( !iter.IsEnd() ) {
        BaseRelLogProp_t *logProp = iter.Next();
        DeRefer(logProp);
    }
}

/*** old code below ***/
static char* GetNextStr(char** str)
{
        while(**str == ' ') (*str)++;
        char* next_blank = *str;
        while( *next_blank != ' '  && *next_blank != '\0') next_blank++;
        *next_blank = '\0';

        char* returnval = strdup(*str);

        if( **str != '\0' )
            *str = next_blank+1;

        // convert to lower case
        for( char *s = returnval ; *s ; s++ )
            *s = tolower(*s);

        return returnval;
}
                        
static double GetNextDouble(char **str)
{
        double returnval = 0;
        while(**str == ' ') (*str)++;

        sscanf(*str,"%lf", &returnval);
        while(**str != ' ' && **str != '\0') (*str)++;
        
        return returnval;
}
        
static int GetNextInt(char **str)
{

        int returnval = 0;
        while(**str == ' ') (*str)++;

        sscanf(*str,"%i", &returnval);
        while(**str != ' ' && **str != '\0') (*str)++;
        
        return returnval;
}

int AddAttrMinValue(char* input_line)
{
        char* position = strstr(input_line,"AttrMinValue");
        position = position + strlen("AttrMinValue");

        char* relname = GetNextStr(&position);
        char* attrname = GetNextStr(&position);
        double minval = GetNextDouble(&position);

        Catalog::AddAttrMinValue(relname,attrname,minval);
        free(relname);
        free(attrname);
        
        return 1;
}

int AddAttrSize(char* input_line)
{
        char* position = strstr(input_line,"AttrSize");
        position = position + strlen("AttrSize");

        char* relname = GetNextStr(&position);
        char* attrname = GetNextStr(&position);
        int attrsize = GetNextInt(&position);

        Catalog::AddAttrSize(relname,attrname,attrsize);
        free(relname);
        free(attrname);
        
        return 1;
}

int AddAttrMaxValue(char* input_line)
{

        char* position = strstr(input_line,"AttrMaxValue");
        position = position + strlen("AttrMaxValue");

        char* relname = GetNextStr(&position);
        char* attrname = GetNextStr(&position);
        double maxval = GetNextDouble(&position);

        Catalog::AddAttrMaxValue(relname,attrname,maxval);
        free(relname);
        free(attrname);
        return 1;
}

int AddAttrDistinct(char* input_line)
{

        char* position = strstr(input_line,"DistinctAttr");
        position = position + strlen("DistinctAtrr");

        char* relname = GetNextStr(&position);
        char* attrname = GetNextStr(&position);
        int noofdistinct = GetNextInt(&position);

        Catalog::AddAttrDistinct(relname,attrname,noofdistinct);
        free(relname);
        free(attrname);
        return 1;
}

int AddSizeOfRel(char* input_line)
{

        char* position = strstr(input_line,"NumOfTuples");
        position = position + strlen("NumOfTuples");

        char* relname = GetNextStr(&position);
        int nooftuples = GetNextInt(&position);

        Catalog::AddRelSize(relname,nooftuples);
        free(relname);
        return 1;
}

#if 0
// removed -- prasan
int AddSizeOfTuple(char* input_line)
{

        char* position = strstr(input_line,"SizeOfTuple");
        position = position + strlen("SizeOfTuple");

        char* relname = GetNextStr(&position);
        int tuplesize = GetNextInt(&position);

        Catalog::AddTupleSize(relname,tuplesize);
        free(relname);

        return 1;
}       
#endif
        
int AddTuplesPerBlock(char* input_line)
{

        char* position = strstr(input_line,"TuplesPerBlock");
        position = position + strlen("TuplesPerBlock");

        char* relname = GetNextStr(&position);
        int tuples_per_block = GetNextInt(&position);

        Catalog::AddTuplesPerBlock(relname,tuples_per_block);
        free(relname);

        return 1;
}       
        
int AddNumOfBlocks(char* input_line)
{

        char* position = strstr(input_line,"NumOfBlocks");
        position = position + strlen("NumOfBlocks");

        char* relname = GetNextStr(&position);
        int no_of_blocks = GetNextInt(&position);

        Catalog::AddNumOfBlocks(relname,no_of_blocks);
        free(relname);

        return 1;
}       

int AddIndexInfo(char* input_line)
{
        char* position = strstr(input_line,"Index");
        position = position + strlen("Index");

        char* relname = GetNextStr(&position);
        int indexListSize = GetNextInt(&position);
        assert(indexListSize > 0);
        char **indexAttrList = (char **) malloc(indexListSize * sizeof(char *));

        int i = 0;
        for( i = 0 ; i < indexListSize ; i++ ) {
            indexAttrList[i] = GetNextStr(&position);
        }

        Catalog::AddIndexInfo(relname, indexListSize, indexAttrList);

        free(relname);
        return 1;
}

int AddMemBlocksInfo(char* input_line)
{
        char* position = strstr(input_line,"MemBlocks");
        position = position + strlen("MemBlocks");

        NumBuffers = GetNextInt(&position);
        cout << "NumBuffers = " << NumBuffers << endl;
        return 1;
}


InputInfoType FindLineInfoType(char* input_line) { 

        if(!input_line) return NOTKNOWN;

        if(strstr(input_line,"DistinctAttr")) return DISTINCT;
        if(strstr(input_line,"NumOfTuples")) return SIZE_OF_REL;
        if(strstr(input_line,"SizeOfTuple")) return SIZE_OF_TUPLE;
        if(strstr(input_line,"AttrMaxValue")) return ATTR_MAX_VALUE;
        if(strstr(input_line,"AttrMinValue")) return ATTR_MIN_VALUE;
        if(strstr(input_line,"NumOfBlocks")) return NUM_OF_BLOCKS;
        if(strstr(input_line,"TuplesPerBlock")) return TUPLES_PER_BLOCK;
        if(strstr(input_line,"AttrSize")) return ATTR_SIZE_BYTES;
        if(strstr(input_line,"Index")) return INDEXINFO;
        if(strstr(input_line,"MemBlocks")) return MEM_BLOCKS;

        return NOTKNOWN;
}       

CatalogInfoForAttr::CatalogInfoForAttr()
{
        minValue = 0;
        maxValue = 0;
        noOfDistinct = 0;
        size = 0;

        minValueValid = 0;
        maxValueValid = 0;
        noOfDistinctValid = 0;
}

int CatalogInfoForAttr::PrintInfo(FILE* fp)
{
        if(!trace_print) return 1;

        if(minValueValid)
                fprintf(fp,"\n min value = %f",double(minValue));
        else fprintf(fp,"\ninformation about min value is not available");

        if(maxValueValid)
                fprintf(fp,"\n max value = %f",double(maxValue));
        else fprintf(fp,"\ninformation about max value is not available");

        if(noOfDistinctValid)
                fprintf(fp,"\nnumber of distinct values = %d",noOfDistinct);
        else fprintf(fp,"\ninformation about number of distinct values"
                    " is not available");

        return 1;

}

CatalogInfoForRel::CatalogInfoForRel()
{

        RelName = NULL;
        RelSize = NULL;
        Next = NULL;

        NumIndexes = 0;
        for( int i = 0 ; i < MAXINDEXES ; i++ ) {
            IndexAttrList[i] = NULL;
            IndexAttrSizeList[i] = 0;
        }
}


Schema_Def* Catalog::Rel_Schemas = NULL;
CatalogInfoForRel* Catalog::Rel_Sizes = NULL;

int Catalog::AddSchema(Schema_Def* schema)
{

        if(!schema) {
                fprintf(stderr,"\nError: error in AddToCatalog"
                    " input schema is NULL");
                return 0;
        }

        schema->NextRel = Catalog::Rel_Schemas;
        Catalog::Rel_Schemas = schema;

        return 1;
}

int Catalog::AddSizeInfoForRel(char* relname, Size_t* relSize)
{

        CatalogInfoForRel* relSizeInfo = new CatalogInfoForRel;

        relSizeInfo->RelName = (char*)malloc(sizeof(char)* (strlen(relname)+1));
        strcpy(relSizeInfo->RelName,relname);

        relSizeInfo->RelSize = relSize;

        relSizeInfo->Next = Catalog::Rel_Sizes;
        Catalog::Rel_Sizes = relSizeInfo;

        return 1;
}

Attribute_Def1* Catalog::AddAttribute
(char* attrname,char* relname,OAttrType Type_a, int len_a)
{

        if(!relname) relname = "";

        Schema_Def* PtrSchema = Catalog::Rel_Schemas;

        while(PtrSchema && strcmp(PtrSchema->RelName,relname)) 
                PtrSchema = PtrSchema->NextRel;

        if(PtrSchema) {

                Attribute* ptrAttributes = PtrSchema->Attributes;

                while(ptrAttributes) {
                        if( ptrAttributes->AttrPtr  && 
                            !strcmp(ptrAttributes->AttrPtr->attrName,attrname))

                                return ptrAttributes->AttrPtr;
                        else 
                                ptrAttributes = ptrAttributes->Next;
                }
        }

        Attribute_Def1* newAttrDef = new Attribute_Def1;
        newAttrDef->Type = Type_a;
        // strdups added by prasan
        newAttrDef->relName = strdup(relname);
        newAttrDef->attrName = strdup(attrname);
        newAttrDef->len = len_a;
        newAttrDef->Next = NULL;

        Attribute* newAttr = new Attribute;
        newAttr->AttrPtr = newAttrDef;
        newAttr->Next = NULL;

        if(PtrSchema) {
                        newAttr->Next = PtrSchema->Attributes;
                        PtrSchema->Attributes = newAttr;
                      }
         else {
                 Schema_Def* newschema = new Schema_Def;
                 newschema->RelName = (char*) malloc
                                        ((strlen(relname)+1)*sizeof(char));
                 strcpy(newschema->RelName,relname);

                 newschema->Attributes = newAttr;

                 Catalog::AddSchema(newschema);

            }   

        return newAttrDef;              
}       

/**** changed this to return the orig object instead of copy ****/
#if 0
Schema_Def* Catalog::GetRelSchema(char* relname)
{

        if(!relname) return NULL;

        Schema_Def* ptrSchema = Catalog::Rel_Schemas;

        while(ptrSchema) {

                if(!strcmp(ptrSchema->RelName,relname)) {

                        Schema_Def* tempptr = ptrSchema->NextRel;

                        ptrSchema->NextRel = NULL;
                        Schema_Def* schema = ptrSchema->Copy(); 

                        ptrSchema->NextRel = tempptr;

                        return schema;
                }

                ptrSchema = ptrSchema->NextRel;
        }

        return NULL;
}
#endif

Schema_Def* Catalog::GetRelSchema(char* relname)
{

        if(!relname) return NULL;

        Schema_Def* ptrSchema = Catalog::Rel_Schemas;

        while(ptrSchema) {

                if(!strcmp(ptrSchema->RelName,relname)) {

                        return ptrSchema;
                }

                ptrSchema = ptrSchema->NextRel;
        }

        return NULL;
}

Attribute* Catalog::GetAttr(char* relname,char* attrname)
{


        if(!relname || !attrname) { return NULL;}

        Schema_Def* ptrSchema = Catalog::Rel_Schemas;

        while(ptrSchema && strcmp(ptrSchema->RelName,relname))
                                        ptrSchema = ptrSchema->NextRel;

        if(!ptrSchema) {
            // added by prasan -- quick fix
            AddAttribute(attrname, relname);

            ptrSchema = Catalog::Rel_Schemas;

            while(ptrSchema && strcmp(ptrSchema->RelName,relname))
                                            ptrSchema = ptrSchema->NextRel;
        }

        Attribute* ptrAttribute = ptrSchema->Attributes;

        while(ptrAttribute && strcmp(ptrAttribute->AttrPtr->attrName, attrname))
                                ptrAttribute = ptrAttribute->Next;
        if( !ptrAttribute ) {
            // added by prasan -- quick fix
            AddAttribute(attrname, relname);

            ptrAttribute = ptrSchema->Attributes;

            while(ptrAttribute &&
                    strcmp(ptrAttribute->AttrPtr->attrName, attrname))
                                    ptrAttribute = ptrAttribute->Next;
        }

        return ptrAttribute;
}

int Catalog::AddAttrSize(char* relname,char* attrname,int size)
{
        assert(size > 0);

        if(!relname || !attrname) { return(0); }

        Attribute* ptrAttribute = Catalog::GetAttr(relname,attrname);

        if(!ptrAttribute) {
                fprintf(stderr,"\nError: error in Catalog::AddAttrMinValue "
                                "attribute with name %s does not exist in the "
                                " relation %s",attrname,relname);
                return 0;       
                }

        if(!ptrAttribute->AttrPtr->catalogInfo)
                ptrAttribute->AttrPtr->catalogInfo = new CatalogInfoForAttr;

        ptrAttribute->AttrPtr->catalogInfo->size = size;
        
        return 1;
}       

int Catalog::AddAttrMinValue(char* relname,char* attrname,double minval)
{

        if(!relname || !attrname) { return(0); }

        Attribute* ptrAttribute = Catalog::GetAttr(relname,attrname);

        if(!ptrAttribute) {
                fprintf(stderr,"\nError: error in Catalog::AddAttrMinValue "
                                "attribute with name %s does not exist in the "
                                " relation %s",attrname,relname);
                return 0;       
                }

        if(!ptrAttribute->AttrPtr->catalogInfo)
                ptrAttribute->AttrPtr->catalogInfo = new CatalogInfoForAttr;

        ptrAttribute->AttrPtr->catalogInfo->minValue = minval;
        ptrAttribute->AttrPtr->catalogInfo->minValueValid = 1;
        
        return 1;
}       

int Catalog::AddAttrMaxValue(char * relname, char *attrname, double maxval)
{

        if(!relname || !attrname) { return(0); }

        Attribute* ptrAttribute = Catalog::GetAttr(relname,attrname);

        if(!ptrAttribute) {
                fprintf(stderr,"\nError: error in Catalog::AddAttrMinValue "
                                "attribute with name %s does not exist in the "
                                " relation %s",attrname,relname);
                return 0;       
                }

        if(!ptrAttribute->AttrPtr->catalogInfo)
                ptrAttribute->AttrPtr->catalogInfo = new CatalogInfoForAttr;

        ptrAttribute->AttrPtr->catalogInfo->maxValue = maxval;
        ptrAttribute->AttrPtr->catalogInfo->maxValueValid = 1;
        
        return 1;
}

int Catalog::AddAttrDistinct (char* relname, char* attrname,int noofdistinct)
{

        if(!relname || !attrname) { return(0); }

        Attribute* ptrAttribute = Catalog::GetAttr(relname,attrname);

        if(!ptrAttribute) {
                fprintf(stderr,"\nError: error in Catalog::AddAttrMinValue "
                                "attribute with name %s does not exist in the "
                                " relation %s",attrname,relname);
                return 0;       
                }

        if(!ptrAttribute->AttrPtr->catalogInfo)
                ptrAttribute->AttrPtr->catalogInfo = new CatalogInfoForAttr;

        ptrAttribute->AttrPtr->catalogInfo->noOfDistinct = noofdistinct;
        ptrAttribute->AttrPtr->catalogInfo->noOfDistinctValid = 1;
        
        return 1;
}
 
int Catalog::AddRelSize(char *relname,int nooftuples)
{

        if(!relname) { return(0); }

        CatalogInfoForRel* ptrRel = Catalog::Rel_Sizes;

        while(ptrRel && strcmp(ptrRel->RelName,relname))
                                        ptrRel = ptrRel->Next;
                
        if(!ptrRel) {
                        ptrRel = new CatalogInfoForRel;

                        ptrRel->RelName = strdup(relname);

                        ptrRel->RelSize = new Size_t;
                        ptrRel->RelSize->numTuples = nooftuples;

                        ptrRel->Next = Catalog::Rel_Sizes;
                        Catalog::Rel_Sizes = ptrRel;

                        return 1;
                    }

        if(!ptrRel->RelSize)
                        ptrRel->RelSize = new Size_t;
        ptrRel->RelSize->numTuples = nooftuples;
                        
        return 1;
}

 
int Catalog::AddTuplesPerBlock(char* relname,int tuples_per_block)
{

        if(!relname) { return(0); }

        CatalogInfoForRel* ptrRel = Catalog::Rel_Sizes;

        while(ptrRel && strcmp(ptrRel->RelName,relname))
                                        ptrRel = ptrRel->Next;
                
        if(!ptrRel) {
                        ptrRel = new CatalogInfoForRel;
                        ptrRel->RelSize = new Size_t;

                        ptrRel->RelName = (char*)
                                 malloc(sizeof(char)* (strlen(relname)+1));
                        strcpy(ptrRel->RelName, relname);
                        ptrRel->RelSize->tuplesPerBlock = tuples_per_block;

                        ptrRel->Next = Catalog::Rel_Sizes;
                        Catalog::Rel_Sizes = ptrRel;

                        return 1;
                    }

        if(!ptrRel->RelSize)
                        ptrRel->RelSize = new Size_t;
        ptrRel->RelSize->tuplesPerBlock = tuples_per_block;
                

        return 1;
}
        
  
int Catalog::AddNumOfBlocks(char* relname,int num_of_blocks)
{

        if(!relname) { return(0); }

        CatalogInfoForRel* ptrRel = Catalog::Rel_Sizes;

        while(ptrRel && strcmp(ptrRel->RelName,relname))
                                        ptrRel = ptrRel->Next;
                
        if(!ptrRel) {
                        ptrRel = new CatalogInfoForRel;
                        ptrRel->RelSize = new Size_t;

                        ptrRel->RelName = (char*)
                                 malloc(sizeof(char)* (strlen(relname)+1));
                        strcpy(ptrRel->RelName, relname);
                        ptrRel->RelSize->noOfBlocks = num_of_blocks;

                        ptrRel->Next = Catalog::Rel_Sizes;
                        Catalog::Rel_Sizes = ptrRel;

                        return 1;
                    }

        if(!ptrRel->RelSize)
                        ptrRel->RelSize = new Size_t;
        ptrRel->RelSize->noOfBlocks = num_of_blocks;
                

        return 1;
}
        
int Catalog::AddIndexInfo(char* relname, int listSize, char **list)
{

        if(!relname || !list || listSize <= 0 ) { return(0); }

        CatalogInfoForRel* ptrRel = Catalog::Rel_Sizes;

        while(ptrRel && strcmp(ptrRel->RelName,relname))
                                        ptrRel = ptrRel->Next;
                
        if(!ptrRel) {
                        ptrRel = new CatalogInfoForRel;

                        ptrRel->RelName = (char*)
                                 malloc(sizeof(char)* (strlen(relname)+1));
                        strcpy(ptrRel->RelName, relname);

                        for( int i = 0 ; i < MAXINDEXES ; i++ ) {
                            ptrRel->IndexAttrList[i] = NULL;
                            ptrRel->IndexAttrSizeList[i] = 0;
                        }

                        ptrRel->IndexAttrList[0] = list;
                        ptrRel->IndexAttrSizeList[0] = listSize;

                        ptrRel->NumIndexes = 1;

                        ptrRel->Next = Catalog::Rel_Sizes;

                        Catalog::Rel_Sizes = ptrRel;

                        return 1;
                    }

        if(!ptrRel->RelSize)
                        ptrRel->RelSize = new Size_t;

        assert(ptrRel->NumIndexes < MAXINDEXES);
        assert(ptrRel->IndexAttrSizeList[ptrRel->NumIndexes] == 0);
        ptrRel->IndexAttrList[ptrRel->NumIndexes] = list;
        ptrRel->IndexAttrSizeList[ptrRel->NumIndexes] = listSize;
        ptrRel->NumIndexes++;

        return 1;
}

#if 0 
// removed -- prasan
int Catalog::AddTupleSize(char* relname,int tuplesize)
{

        if(!relname) { return(0); }

        CatalogInfoForRel* ptrRel = Catalog::Rel_Sizes;

        while(ptrRel && strcmp(ptrRel->RelName,relname))
                                        ptrRel = ptrRel->Next;
                
        if(!ptrRel) {
                        ptrRel = new CatalogInfoForRel;
                        ptrRel->RelSize = new Size_t;

                        ptrRel->RelName = (char*)
                                 malloc(sizeof(char)* (strlen(relname)+1));
                        strcpy(ptrRel->RelName, relname);
                        ptrRel->RelSize->tupleSize = tuplesize;

                        ptrRel->Next = Catalog::Rel_Sizes;
                        Catalog::Rel_Sizes = ptrRel;

                        return 1;
                    }

        if(!ptrRel->RelSize)
                        ptrRel->RelSize = new Size_t;
        ptrRel->RelSize->tupleSize = tuplesize;
                

        return 1;
}
#endif
        
Attribute_Def1* Catalog::GetAttrValInfo(char* attrname)
{

        Schema_Def* ptr_schema = Catalog::Rel_Schemas;

        while(ptr_schema) {

           if(strcmp(ptr_schema->RelName,"")) {

                Attribute* ptr_attr = ptr_schema->Attributes;

                while(ptr_attr && strcmp(ptr_attr->AttrPtr->attrName,attrname)) 
                                ptr_attr = ptr_attr->Next;

                if(ptr_attr) return ptr_attr->AttrPtr;
           }

           ptr_schema = ptr_schema->NextRel;    
        }

        return NULL;    
}

/** some dead function **/
#if 0
int Catalog::ComputeInfoFromExisting()
{

        Schema_Def* ptrSchema = Catalog::Rel_Schemas;

        while(ptrSchema && strcmp(ptrSchema->RelName,"")) 
                ptrSchema = ptrSchema->NextRel;

#if 0
        if(ptrSchema) {

                Attribute* ptrAttr = ptrSchema->Attributes;

                while(ptrAttr) {

                   if(!ptrAttr->AttrPtr->catalogInfo || 
                                ptrAttr->AttrPtr->Type == O_UNKNOWN) {
                   
                      Attribute_Def1* returnval =
                         Catalog::GetAttrValInfo(ptrAttr->AttrPtr->attrName);
                        
                        if(returnval) {
                                
                                ptrAttr->AttrPtr->catalogInfo = 
                                                 returnval->catalogInfo;
                                ptrAttr->AttrPtr->Type = returnval->Type;
                                ptrAttr->AttrPtr->len = returnval->len;
                        }
                    }

                   ptrAttr = ptrAttr->Next;

                } // end of while(ptrAttr)
        }
#endif
        CatalogInfoForRel* ptrRelInfo = Catalog::Rel_Sizes;

        while(ptrRelInfo) {

                if(ptrRelInfo->RelSize) {
                        Size_t* ptrSize = ptrRelInfo->RelSize;

                        if(ptrSize->tupleSize == -1) {
                                Schema_Def* ptrSchema = 
                                     Catalog::GetRelSchema(ptrRelInfo->RelName);

                                ptrSize->tupleSize = ptrSchema ? 
                                        ptrSchema->FindSize() : NULL;

                                delete ptrSchema;
                         }
                        if(ptrSize->tuplesPerBlock == -1 && ptrSize->tupleSize) 
                                ptrSize->tuplesPerBlock = 
                                  floor(BLOCKSIZE/ptrSize->tupleSize);

                        if(ptrSize->numTuples == -1 && ptrSize->tupleSize) 
                                ptrSize->numTuples = 
                                  floor(BLOCKSIZE/ptrSize->tupleSize);

                        if(ptrSize->noOfBlocks == -1 && ptrSize->numTuples && 
                                                ptrSize->tuplesPerBlock) 
                          ptrSize->noOfBlocks = ceil( ptrSize->numTuples/
                                        ((double)ptrSize->tuplesPerBlock));
                }                                       

                ptrRelInfo = ptrRelInfo->Next;
        }

        return 1;

}
#endif

int LoadCatalogInfo(char *catalogFile)
{
        char input_line[81] = "";
        int end_of_input = 0;
        FILE* fp;
        if (catalogFile == NULL) {
            fp = fopen("Catalog","r");
        }
        else {
            fp = fopen(catalogFile,"r");
        }
        assert(fp != NULL);

        while(1) {

                if(!fgets(input_line,80,fp)) end_of_input = 1;
                while(!end_of_input && (!strcmp(input_line,"\n") ||
                                                input_line[0] == '#') )
                        if(!fgets(input_line,81,fp)) end_of_input = 1;

                if(end_of_input) return 1;

                int len = strlen(input_line);
                for(int i = 0; i < len ; i++) {
                    if( input_line[i] == ',' || input_line[i] == ')' || 
                            input_line[i] == '(' || input_line[i] == '=') {
                        input_line[i] = ' ';
                    }
                }

                int inserted = 0;

                switch(FindLineInfoType(input_line)) {

                case SIZE_OF_REL: 
                        inserted = AddSizeOfRel(input_line);
                        break;
                case SIZE_OF_TUPLE :
                        assert(0);
                        // removed -- prasan
                        // inserted = AddSizeOfTuple(input_line);
                        break;
                case DISTINCT : 
                        inserted = AddAttrDistinct(input_line);
                        break;
                case ATTR_MAX_VALUE : 
                        inserted = AddAttrMaxValue(input_line);
                        break;
                case ATTR_MIN_VALUE : 
                        inserted = AddAttrMinValue(input_line);
                        break;
                case TUPLES_PER_BLOCK : 
                        inserted = AddTuplesPerBlock(input_line);
                        break;
                case NUM_OF_BLOCKS : 
                        inserted = AddNumOfBlocks(input_line);
                        break;
                case ATTR_SIZE_BYTES : 
                        inserted = AddAttrSize(input_line);
                        break;
                case INDEXINFO : 
                        inserted = AddIndexInfo(input_line);
                        break;
                case MEM_BLOCKS : 
                        inserted = AddMemBlocksInfo(input_line);
                        break;
                default:
                        break;
                }

                if(!inserted)
                    cout << "Error: " << input_line;
                assert(inserted);
        } 
        // while (1)
}
