#ifndef RM_H
#define RM_H
#include <iostream>
#include <cassert>
#include <sstream>
#include <string.h>
#include "sm.h"
#include "iter.h"
#include "sp.h"
#include "sputil.h"


class RIterator;
using namespace std;
class Relation;

class Schema {
    short int numAttrs;		//Number of Attributes
    short int *sizeAttrs; 	// size of all attributes
    short int *attrsOffset; // stores offset of fixed length records and offset of offset data of variable length records 
    short int *attrType;	//Attribute types of all Attributes
                            // 1 - int;  2-float; 3-char;  4-varchar
    char **attrName;		//Attribute Names
    short int numVarAttrs;	//Number of Varchars
public:
    Schema() {

    }
	//Constructor of schema takes values and pass them to a function setschema
    Schema(short int numAttrs, short int* attrType, char **attrName, short int *sizeAttrs) {
        setSchema(numAttrs, attrType, attrName, sizeAttrs);

    }
	
    void setSchema(short int numAttrs, short int* attrType, char **attrName, short int *sizeAttrs);
  
	//Prints the schema
    void print();
	
	//gets Attribute offset of the corresponing index
    short getAttrsOffset(int index);
	
	//gets Attribute type of the corresponing index
    short getAttrType(int index);
	
	//gets size of Attribute of the corresponing index
    short getSizeAttrs(int index);
	
	//returns the numof attributes of the schema
    short getNumAttrs();
	
	//returns number of varchars of the schema
    short getNumVarAttrs();
	
	//returns Attribute Name of the corresponing index
    char *getAttrName(int index);
    
    //returns index of attribute
    int getAttrIndex(const char* name);
    
    //concatenates two schemas
    Schema* concateSchema(Schema* sch, Schema* newSchema);

};

class Record {
	//Stores the data of record in form of char
    char *recBytes;
	
	//stores schema of the record
    
	Schema *schema;
	//stores length of record array
    int length;

public:

    Record(char *recBytes, Schema *schema, int length) {

        this->recBytes = new char[MAX_REC_BYTE_SIZE];
        this->recBytes = recBytes;
        this->length = length;
        this->schema = new Schema(*schema);
    }

    Record(string* strArray, Schema* schema, int size) {
        int l;
        this->recBytes = new char[MAX_REC_BYTE_SIZE];
        this->recBytes = dataToRecBytes(schema, size, strArray, l);
        this->length = l;
        this->schema = new Schema(*schema);

    }
	
	//Convert A record to recbytes by taking schema , data in the form of string Array
	//takes size of data Array (to check if its equal to num of Attrs in Schema) and sets 
	//an int L to length of recbytes that is returned
    
	static char *dataToRecBytes(Schema *schema, int sizeOfData, string* dataArray, int& l);
    
	//following functions returns the values from recbytes by taking the index of the Attribute
	int getInt(int attrIndex);
    char* getConstString(int attrIndex);
    char* getVarString(int attrIndex);
    float getFloat(int attrIndex);
   
    //returns string array of record values
    string* getStringArray(string* s);	
	
	//Joins two records
	Record* joinRecords( Record* rec2, Record* newRec);
	
	char* getRecBytes();
    Schema* getSchema();
    
	//returns length of recBytes
	int getLength();
	
	//Comparison operators of Record which takes another record and compares on an attribute
    bool isGreater(Record* r, int index);
    bool isEqual(Record* r, int index);
    bool isLesser(Record* r, int index);
	
	//Prints the record
    void print();
    

};

class Relation {
    const char *dbName;
    const char *relName;
    Schema* schema;
    int lastBlockId;            //Stores id of last filled block of relation
                                //Should be stored in super block after a new block is allocated
                                //to the relation (NOT implemented here)
public:
	//Initialises the two system tables by getting them into buffer and calling initialise of slotted page
	//now records can be added to these tables
    void initSystables();
	
	//get schema of sysObject Table
    static Schema getSysObjSchema();
    
	//Get Schema of sys columns table
	static Schema getSysColSchema();
	
	//A schema is generated for testing purposes
    static Schema getSomeSchema();
	
	//Reads schema from the database by taking DBName and RelationName
    static Schema* readSchema(const char* dbName1, const char* relName1);
	
	//Constructor of Relation which first Initialises system Tables if not
	//Stores schema data in systables
	//Initialses first block with slotted Page
    Relation(const char *dbName, const char *relName, bool create,
            Schema *schema) {
            
        lastBlockId = -1;
        if (StorageManager::getFirstBlockId(dbName, "sysObjects") == -1) {           
            Relation::initSystables();
        }
        this->dbName = dbName;
        this->relName = relName;
        int existsCheck = StorageManager::getFirstBlockId(dbName, relName);
        if (create && existsCheck < 0) {
            assert(schema != NULL);
            StorageManager::createObject(dbName, relName);
            int block1 = StorageManager::getFirstBlockId(dbName, "sysObjects");
            char* sysobj = BufferManager::getInstance(dbName)->pinPage(block1);
            SlottedPage sp(sysobj);
            string *dataArray;
            dataArray = new string[3];
            dataArray[0] = relName;
            dataArray[1] = "1";
            int num = schema->getNumAttrs();
            stringstream ss;
            string str;
            ss<<num;
            str = ss.str();
            dataArray[2] = str.c_str();
            Schema *schemaObj;
            string *dataArray2;
            schemaObj = new Schema(getSysObjSchema());
            int l1;
            char * sysObjData;
            sysObjData = new char[MAX_REC_BYTE_SIZE];
            sysObjData = Record::dataToRecBytes(schemaObj, 3, dataArray, l1);
            
            Record rec1(sysObjData, schemaObj, l1);
   
            int slotNum = sp.storeRecord(rec1.getRecBytes(), rec1.getLength());
            char* recbb = new char[MAX_REC_BYTE_SIZE];
            int l;
            recbb = sp.getRecord(slotNum, l);          
            if (slotNum < 0)
                cout << "error : systable full" << endl;         

            BufferManager::getInstance(dbName)->unpinPage(block1, 1);
            int block2 = StorageManager::getFirstBlockId(dbName, "sysColumns");
            char* sysColumns = BufferManager::getInstance(dbName)->pinPage(block2);
            SlottedPage sp2(sysColumns);
            
            Schema *schemaCols = new Schema(getSysColSchema());
            
            for (int i = 0; i < schema->getNumAttrs(); i++) {
                string *dataArray2;
                dataArray2 = new string[5];
                dataArray2[0] = relName;
                stringstream ss, ss2, ss3;
                ss << i;
                ss >> dataArray2[1];

                dataArray2[2] = schema->getAttrName(i);
                ss2 << schema->getAttrType(i);
                ss2 >> dataArray2[3];
                ss3 << schema->getSizeAttrs(i);
                ss3 >> dataArray2[4];

                int l2;
                char * sysColData;
                sysColData = new char[MAX_REC_BYTE_SIZE];
                sysColData = Record::dataToRecBytes(schemaCols, 5, dataArray2, l2);
                Record rec2(sysColData, schemaCols, l2);

                int slotNum2 = sp2.storeRecord(rec2.getRecBytes(), rec2.getLength());
                if (slotNum2 < 0)
                    cout << "error : systable full" << endl;

            }
            BufferManager::getInstance(dbName)->unpinPage(block2, 1);

            int relFirstBlock = StorageManager::getFirstBlockId(dbName, relName);
            char* relFirstBlockData = BufferManager::getInstance(dbName)->pinPage(relFirstBlock);
            SlottedPage relSP(relFirstBlockData);
            relSP.initBlock();
            BufferManager::getInstance(dbName)->unpinPage(relFirstBlockData, true);
            
        }
        else {
            Schema* relSchema = Relation::readSchema(dbName, relName);
            this->schema = relSchema;
        }
    }
	
    Relation(const char* dbName, const char* relName) {
        if (StorageManager::getFirstBlockId(dbName, "sysObjects") == -1) {
            Relation::initSystables();
        }

        this->dbName = dbName;
        this->relName = relName;
        Schema* relSchema = Relation::readSchema(dbName, relName);
        this->schema = relSchema;
    }
	
    RIterator* openScan();
    RIterator* openScan(Predicate* p);

	//Adds record in relation by writing it in Slotted Page of corresponding BlockId
    int* addRecord(Record rec, int blockId);
	
	//Adds record 
    int* addRecord(Record rec);
    int getLastWrittenBlockId();
    int deleteRecord(int* recId);
    int deleteRecord(int blockId, int slotNo);
    Record* getRecord(int* recId);
    Record* getRecord(int blockId, int slotNo);
};


#endif


