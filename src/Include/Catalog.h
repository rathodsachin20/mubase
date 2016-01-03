// catalog

#ifndef __CATALOG_H__
#define __CATALOG_H__

#include "List.h"
#include "LogProp.h"

// caches the log props in the list instead of recomputing
class Catalog_t {
    PrependList_t<BaseRelLogProp_t *> relLogPropList;

public:
    Catalog_t(char *catalogFile=NULL);

    LogProp_t *GetRelLogProp(const char* relname);

    ~Catalog_t(void);
};

extern Catalog_t *Catalog;

/*** stuff below is from the original code ***/

enum OAttrType {
    O_UNKNOWN_TYPE = -1,
    O_INTEGER = 0,
    O_STRING = 1,
    O_FLOAT = 2,
    O_DOUBLE = 3,
    O_VARSTRING = 4,
    O_VARBINARY = 5,
    O_DATE = 6,
    O_TIME = 7,
    O_BINARY = 8
};

class Size_t {
public: 

        int numTuples;
        int tupleSize;
        int noOfBlocks;
        int tuplesPerBlock;
}; 

const int MAXINDEXES = 10;

class CatalogInfoForRel {
public:
        char* RelName;
        Size_t* RelSize;
        CatalogInfoForRel* Next;

        int NumIndexes;
        char **IndexAttrList[MAXINDEXES];
        int IndexAttrSizeList[MAXINDEXES];

        CatalogInfoForRel();
        ~CatalogInfoForRel();
};

class CatalogInfoForAttr {
public:
        double maxValue;
        double minValue;
        int noOfDistinct;
 
        int minValueValid;
        int maxValueValid;
        int noOfDistinctValid;
        int size;
 
        CatalogInfoForAttr();
        ~CatalogInfoForAttr(){};
        int PrintInfo(FILE* fp);
 
};
 
class Attribute_Def1 {
                        //// this class is used for representing the attributes
                        //// of a relation

public:
        OAttrType Type;         //// gives the type of attribute type
        char* relName;          //// the name of the attribute is stored here
        char* attrName;
        int len;                //// the length of the name is stored here

        CatalogInfoForAttr* catalogInfo;
                                /// the above is used for getting information
                                /// about this attribute

        Attribute_Def1* Next;   //// this is used for maintaining a list of
                                //// attributes
};


class Attribute {
                        //// this class is used for representing the attributes
                        //// of a relation

public:

        Attribute_Def1* AttrPtr;
        Attribute* Next;
};

class Schema_Def {
public:
        char* RelName;          /// for storing the name of a relation
                                /// the default value is ""

        Attribute* Attributes;  /// for storing the attributes of a relation
                                    /// named 'RelName", which belongs to the
                                    /// schema
        Schema_Def* NextRel;
                                //// this used for maintaining a linked list
                                //// of Schema_Def's as mentioned above
};

class Catalog {

        static Attribute* GetAttr(char* relname, char* attrname);
public :

        static  Schema_Def* Rel_Schemas;
        static CatalogInfoForRel* Rel_Sizes;
        
        Catalog(){};
        ~Catalog(){};   

        static int AddSchema(Schema_Def*);
        static Attribute_Def1* AddAttribute (char* attrName,char* relName = "",
                        OAttrType Type_a = O_UNKNOWN_TYPE,int len_a = 0);

        static Schema_Def* GetRelSchema(char* relname);

        static int AddAttrSize(char* relname,char* attrname,int size);
        static int AddAttrMinValue(char* relname,char* attrname,double minval);

        static int AddAttrMaxValue(char * relname,char *attrname,double maxval);

        static int AddAttrDistinct
                        (char* relname, char* attrname,int noofdistinct);

        static int AddRelSize(char *relname,int nooftuples);

        // static int AddTupleSize(char* relname,int tuplesize);
        static int AddNumOfBlocks(char* relname,int num_of_blocks);
        static int AddTuplesPerBlock(char* relname,int tuples_per_block);

        static int ComputeInfoFromExisting();
        static Attribute_Def1* GetAttrValInfo(char* attrname);
        static int AddSizeInfoForRel(char* relname, Size_t* relsize);
        static int AddIndexInfo(char* relname, int listSize, char **list);
};

#endif // __CATALOG_H__
