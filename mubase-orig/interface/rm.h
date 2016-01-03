#include "global.h"
#include "sm.h"
#include "iter.h"

using namespace std;

class Schema {
    int numAttrs;
    int attrType[MAX_ATTRS];
    char *attrName[MAX_ATTRS];
    public:
    // Add useful methods here
};

class Record {
    char *recBytes;
    Schema *schema;
    
    public: 
    Record(char *recBytes, Schema *schema);
    int getInt(int attrIndex);
    char *getString(int attrIndex);
    float getFloat(int attrIndex);
    //...
};

// Iterator for a relation.
class BaseRelIterator : public RIterator {
    // Here go the attributes to maintain iterator state

    public:

    BaseRelIterator(const char *dbName, const char *relName);

    // Open the iterator, initializing its state
    void open() {
        curBlock = StorageManager::getFirstBlockId(dbName, relName);
        curBlockBuff = BufferManager::getInstance(dbName)->
                                        pinPage(curBlock);
        //...
    }

    // Returns the next record, NULL if no more records.
    Record next();

    // Close the iterator, unpin any pinned buffer blocks
    void close();
};

class Predicate {

};

class Relation {
    
    const char *dbName;
    const char *relName;

    public:
    Relation(const char *dbName, const char *relName, bool create,
             Schema *schema) {
        this->dbName  = dbName;
        this->relName = relName;

        if(create) {
            assert(schema != NULL);
            StorageManager::createObject(dbName, relName);
            // ...
            // Store the schema in the metadata table
        }
    }

    RIterator *openScan() {
        return new BaseRelIterator(dbName, relName);
    }

    // Predicated scan. Should make use of the B+ tree index
    // when an index is available on the required column(s).
    RIterator *openScan(Predicate p);

    int addRecord(Record rec);

    int deleteRecord(int recId);
};

