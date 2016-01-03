#ifndef RM_H
#define RM_H


#include "global.h"
#include "sm.h"
#include "iter.h"
#include <sstream>
#include <string.h>
#include "sp.h"
#include "sputil.h"
#include <cassert>

using namespace std;
class Relation;
class Schema;
class RIterator;

class Schema {
    short int numAttrs;
    short int *sizeAttrs; // size of all attributes
    short int *attrsOffset; // stores offset of fixed length records and offset of offset data of variable length records 
    short int *attrType;
    char **attrName;
    short int numVarAttrs;
public:

    Schema() {

    }

    Schema(short int numAttrs, short int* attrType, char **attrName, short int *sizeAttrs) {
        setSchema(numAttrs, attrType, attrName, sizeAttrs);

    }

    void setSchema(short int numAttrs, short int* attrType, char **attrName, short int *sizeAttrs) {
        this->numAttrs = numAttrs;
        this->attrType = new short int[MAX_ATTRS];
        this->sizeAttrs = new short int[MAX_ATTRS];
        this->attrName = new char*[MAX_ATTRS];

        int numOfVarChar = 0;
        for (short int i = 0; i < numAttrs; i++) {
            this->attrType[i] = attrType[i];

            this->attrName[i] = new char[strlen(attrName[i])];
            for (short int j = 0; j < strlen(attrName[i]); j++) {

                this->attrName[i][j] = attrName[i][j];
            }
            if (attrType[i] == 4)
                numOfVarChar++;

        }
        for (short int i = numAttrs; i < MAX_ATTRS - numAttrs; i++) {
            attrType[i] = -1;
            sizeAttrs[i] = -1;
            string s = "";
            attrName[i] = &s[0];
            attrsOffset[i] = -1;

        }
        short int offsetConst = numOfVarChar * sizeof (short int) *2 + numAttrs / 8;
        if (numAttrs % 8 != 0)
            offsetConst++;
        short int offsetVar = 0;
        for (short int i = 0; i < numAttrs; i++) {
            if (attrType[i] == 4) {
                attrsOffset[i] = offsetVar;
                offsetVar = offsetVar + sizeof (short int) *2;
            } else {
                attrsOffset[i] = offsetConst;
                offsetConst = offsetConst + sizeAttrs[i];
            }
        }



    }

    Schema getSchema() {
        return *this;
    }

    void Print() {
        for (short int i = 0; i < numAttrs; i++) {
            cout << attrName[i] << ":" << attrType[i] << endl;
        }
    }

    short getAttrsOffset(int index) {
        return attrsOffset[index];
    }

    short getAttrType(int index) {
        return attrType[index];
    }

    short getSizeAttrs(int index) {
        return sizeAttrs[index];
    }

    short getNumAttrs() {
        return numAttrs;
    }

    short getNumVarAttrs() {
        return numVarAttrs;
    }

    char *getAttrName(int index) {
        return attrName[index];
    }
};

class Record {
    char *recBytes;
    Schema *schema;

public:

    Record(char *recBytes, Schema *schema) {
        this->recBytes = recBytes;
        this->schema = schema;
    }

    float getInt(int attrIndex) {
        int value;
        int* intPtr = &value;
        value = readInt(recBytes, schema->getAttrsOffset(attrIndex));

        return value;
    }

    char* getConstString(int attrIndex) {

        char* value = readBytes(recBytes, schema->getAttrsOffset(attrIndex), schema->getSizeAttrs(attrIndex));
        return value;
    }

    char* getVarString(int attrIndex) {
        short varoffset = readShort(recBytes, schema->getAttrsOffset(attrIndex));
        short varLength = readShort(recBytes, schema->getAttrsOffset(attrIndex) + sizeof (short));

        char* value = readBytes(recBytes, varoffset, varLength);

        return value;
    }

    float getFloat(int attrIndex) {
        float value;
        float* floatPtr = &value;
        value = readFloat(recBytes, schema->getAttrsOffset(attrIndex));

        return value;
    }

    bool isGreater(Record* r, int index) {
        if (schema->getAttrType(index) == 1) {
            if (this->getInt(index) > r->getInt(index))
                return true;
            else
                return false;
        } else if (schema->getAttrType(index) == 2) {
            if (this->getFloat(index) > r->getFloat(index))
                return true;
            else
                return false;
        } else if (schema->getAttrType(index) == 3) {
            string s1, s2;
            s1 = this->getConstString(index);
            s2 = r->getConstString(index);
            if (s1 > s2)
                return true;
            else
                return false;
        } else if (schema->getAttrType(index) == 4) {
            string s1, s2;
            s1 = this->getVarString(index);
            s2 = r->getVarString(index);
            if (s1 > s2)
                return true;
            else
                return false;
        }
    }

    bool isEqual(Record* r, int index) {
        if (schema->getAttrType(index) == 1) {
            if (this->getInt(index) == r->getInt(index))
                return true;
            else
                return false;
        } else if (schema->getAttrType(index) == 2) {
            if (this->getFloat(index) == r->getFloat(index))
                return true;
            else
                return false;
        } else if (schema->getAttrType(index) == 3) {
            string s1, s2;
            s1 = this->getConstString(index);
            s2 = r->getConstString(index);
            if (s1 == s2)
                return true;
            else
                return false;
        } else if (schema->getAttrType(index) == 4) {
            string s1, s2;
            s1 = this->getVarString(index);
            s2 = r->getVarString(index);
            if (s1 == s2)
                return true;
            else
                return false;
        }
    }

    bool isLesser(Record* r, int index) {
        if (schema->getAttrType(index) == 1) {
            if (this->getInt(index) < r->getInt(index))
                return true;
            else
                return false;
        } else if (schema->getAttrType(index) == 2) {
            if (this->getFloat(index) < r->getFloat(index))
                return true;
            else
                return false;
        } else if (schema->getAttrType(index) == 3) {
            string s1, s2;
            s1 = this->getConstString(index);
            s2 = r->getConstString(index);
            if (s1 < s2)
                return true;
            else
                return false;
        } else if (schema->getAttrType(index) == 4) {
            string s1, s2;
            s1 = this->getVarString(index);
            s2 = r->getVarString(index);
            if (s1 < s2)
                return true;
            else
                return false;
        }
    }


    //...
};

class Relation {
    const char *dbName;
    const char *relName;
    Schema* schema;
public:

    void addRecord() {

    }

    //initialise the system tables , first through storage manager make 2 relations
    //then initialise through slotted page and write them back

    void initSystables() {
        StorageManager::createObject(dbName, "sysobjects");
        int block1 = StorageManager::getFirstBlockId(dbName, "sysobjects");
        char* sysobj = BufferManager::getInstance(dbName)->pinPage(block1);
        SlottedPage sp(sysobj);
        sp.initBlock();
        BufferManager::getInstance(dbName)->unpinPage(block1, 1);


        StorageManager::createObject(dbName, "syscols");
        int block2 = StorageManager::getFirstBlockId(dbName, "syscols");
        char* syscol = BufferManager::getInstance(dbName)->pinPage(block2);
        SlottedPage sp2(syscol);
        sp2.initBlock();
        BufferManager::getInstance(dbName)->unpinPage(block2, 1);
    }

    static Schema getSysObjSchema() {
        short numAttrs = 3;
        short attrType[] = {4, 1, 1};
        short sizeOfShort = sizeof (short);
        short sizeAttrs[] = {50, sizeOfShort, sizeOfShort};
        char **attrName;
        string temp[3] = {"ObjectName", "type", "arity"};
        attrName = new char*[3];
        attrName[0] = &temp[0][0];
        attrName[1] = &temp[1][0];
        attrName[2] = &temp[2][0];
        Schema sch(numAttrs, attrType, attrName, sizeAttrs);
        return sch;
    }

    static Schema getSysColSchema() {
        short numAttrs = 5;
        short attrType[] = {4, 1, 4, 1, 1};
        short sizeOfShort = sizeof (short);
        short sizeAttrs[] = {50, sizeOfShort, 50, sizeOfShort, sizeOfShort};
        char **attrName;
        string temp[5] = {"ObjectName", "index", "colname", "coltype", "colsize"};
        attrName[0] = &temp[0][0];
        attrName[1] = &temp[1][0];
        attrName[2] = &temp[2][0];
        attrName[3] = &temp[3][0];
        attrName[4] = &temp[4][0];
        Schema sch(numAttrs, attrType, attrName, sizeAttrs);
        return sch;
    }

    static Schema* readSchema(const char* dbName1, const char* relName1) {
        Schema* relSchema;
        int relArity = 0;
        short int* attrType;
        char **attrName;
        short int *sizeAttrs;

        int soblock0 = StorageManager::getFirstBlockId(dbName1, "sysObjects");
        int scblock0 = StorageManager::getFirstBlockId(dbName1, "sysColumns");

        char* soBlockBuff = BufferManager::getInstance(dbName1)->pinPage(soblock0);
        char* scBlockBuff = BufferManager::getInstance(dbName1)->pinPage(scblock0);

        SlottedPage sosp(soBlockBuff);
        SlottedPage scsp(scBlockBuff);

        //sysObjects : 0-ObjectName 1-type 2-arity
        //sysColumns : 0-ObjectName 1-index 2-colname 3-coltype 4-colsize

        BaseRelIterator soBaseRI(dbName1, "sysObjects");
        soBaseRI.open();

        Record* recPtr;
        recPtr = soBaseRI.next();
        while (recPtr != NULL) {
            if (strcmp(recPtr->getVarString(0), relName1) && recPtr->getInt(1) == 1) {
                relArity = recPtr->getInt(2);
                break;
            }
            recPtr = soBaseRI.next();
        }
        soBaseRI.close();
        if (relArity == 0) {
            cout << "No such relation\n";
            exit(1);
        }

        attrName = new char*[relArity];
        attrType = new short[relArity];
        sizeAttrs = new short[relArity];

        BaseRelIterator scBaseRI(dbName1, "sysColumns");
        scBaseRI.open();

        int i = 0;
        int colNum;
        recPtr = scBaseRI.next();
        while (recPtr != NULL) {
            if (strcmp(recPtr->getVarString(0), relName1)) {
                colNum = recPtr->getInt(1);
                attrName[colNum] = recPtr->getVarString(2);
                attrType[colNum] = recPtr->getInt(3);
                sizeAttrs[colNum] = recPtr->getInt(4);
                relArity = recPtr->getInt(2);
                i++;
                if (i == relArity) {
                    break;
                }
            }
            recPtr = scBaseRI.next();
        }
        scBaseRI.close();
        if (i < relArity) {
            exit(1);
        }

        relSchema = new Schema(relArity, attrType, attrName, sizeAttrs);

        return relSchema;

    }

    char *dataToRecord(Schema *schema, int sizeOfData, string* dataArray) {
        int lastVarAttr;
        string recbyte = "";
        char* nullBits;
        int numNullBytes;
        string offsetSize = "";
        if (schema->getNumAttrs() % 8 == 0)
            numNullBytes = schema->getNumAttrs() / 8;
        else
            numNullBytes = (schema->getNumAttrs() / 8) + 1;
        nullBits = new char[numNullBytes];
        for (short int i = 0; i < schema->getNumAttrs(); i++) { //char* cDataArray = &dataArray(i)[0];

            if (schema->getAttrType(i) == 3) {
                if (strcmp(dataArray[i].c_str(), ""))
                    nullBits[i] = '1';
                else {
                    recbyte = recbyte + dataArray[i];
                    nullBits[i] = '0';
                }
            } else if (schema->getAttrType(i) == 2) {
                if (strcmp(dataArray[i].c_str(), ""))
                    nullBits[i] = '1';
                else {
                    float floatData;
                    stringstream ss;
                    ss << dataArray[i];
                    ss >> floatData;

                    char *tempFloat;
                    tempFloat = new char[sizeof (float) ];
                    writeFloat(tempFloat, 0, floatData);



                    recbyte = recbyte + tempFloat;
                    nullBits[i] = '0';
                }
            } else if (schema->getAttrType(i) == 1) {
                if (strcmp(dataArray[i].c_str(), ""))
                    nullBits[i] = '1';
                else {
                    int intData;
                    stringstream ss;
                    ss << dataArray[i];
                    ss >> intData;

                    char *tempInt;
                    tempInt = new char[sizeof (int) ];
                    writeInt(tempInt, 0, intData);
                    recbyte = recbyte + tempInt;
                    nullBits[i] = '0';
                }
            }


        }
        lastVarAttr = schema->getNumVarAttrs() * sizeof (short) *2 + numNullBytes + recbyte.size();
        for (int i = 0; i < schema->getNumAttrs(); i++) {
            if (schema->getAttrType(i) == 4) {
                if (strcmp(dataArray[i].c_str(), ""))
                    nullBits[i] = '1';
                else {
                    short sizeAttr = dataArray[i].size();
                    stringstream ss, ss2;
                    string tmp1, tmp2;
                    ss << lastVarAttr;
                    ss >> tmp1;
                    ss2 << sizeAttr;
                    ss >> tmp2;
                    short lastVarAttrShort = lastVarAttr;
                    char * lastVarAttrChar;
                    lastVarAttrChar = new char[sizeof (short) ];
                    writeShort(lastVarAttrChar, 0, lastVarAttrShort);

                    char *sizeAttrChar;
                    sizeAttrChar = new char[sizeof (short) ];
                    writeShort(sizeAttrChar, 0, sizeAttr);


                    offsetSize = offsetSize + lastVarAttrChar + sizeAttrChar;
                    recbyte = recbyte + dataArray[i];
                    lastVarAttr += sizeAttr;
                    nullBits[i] = '0';
                }
            }

        }

        char* nullBytes;
        nullBytes = new char[numNullBytes];
        recbyte = offsetSize + nullBytes + recbyte;

        char* recbytes = &recbyte[0];
        return recbytes;

    }

    Relation(const char *dbName, const char *relName, bool create,
            Schema *schema) {
        if (StorageManager::getFirstBlockId(dbName, "sysobjects") == -1) {
            Relation::initSystables();
        }
        this->dbName = dbName;
        this->relName = relName;

        if (create) {
            assert(schema != NULL);
            StorageManager::createObject(dbName, relName);

            int block1 = StorageManager::getFirstBlockId(dbName, "sysobjects");
            char* sysobj = BufferManager::getInstance(dbName)->pinPage(block1);
            SlottedPage sp(sysobj);

            string *dataArray;
            dataArray = new string[3];
            dataArray[0] = relName;
            dataArray[1] = "1";
            dataArray[2] = "3";
            Schema *schemaObj;
            schemaObj = new Schema(getSysColSchema());
            char * sysObjData = dataToRecord(schemaObj, 3, dataArray);
            if (sp.storeRecord(sysObjData) < 0)
                cout << "error : systable full" << endl;
            BufferManager::getInstance(dbName)->unpinPage(block1, 1);



            int block2 = StorageManager::getFirstBlockId(dbName, "syscols");
            char* syscols = BufferManager::getInstance(dbName)->pinPage(block2);
            SlottedPage sp2(syscols);

            Schema *schemaCols = new Schema(getSysObjSchema());


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

                char * sysColData = dataToRecord(schemaCols, 5, dataArray2);
                if (sp2.storeRecord(sysColData) < 0)
                    cout << "error : systable full" << endl;
                BufferManager::getInstance(dbName)->unpinPage(block1, 1);
            }
            // ...
            // Store the schema in the metadata table
        } else {
            Schema* relSchema = Relation::readSchema(dbName, relName);
            this->schema = relSchema;
        }
    }

    Relation(const char* dbName, const char* relName) {
        if (StorageManager::getFirstBlockId(dbName, "sysobjects") == -1) {
            Relation::initSystables();
        }

        this->dbName = dbName;
        this->relName = relName;

        Schema* relSchema = Relation::readSchema(dbName, relName);
        this->schema = relSchema;

    }

    RIterator *openScan() {
        RIterator* ri = new BaseRelIterator(dbName, relName);
        return ri;
    }

    // Predicated scan. Should make use of the B+ tree index
    // when an index is available on the required column(s).
    //RIterator *openScan(Predicate p);

    int addRecord(Record rec);

    int deleteRecord(int recId);

    Record getRecord();
};

BaseRelIterator::BaseRelIterator(const char *dbName1, const char *relName1) {
    dbName = dbName1;
    relName = relName1;
    Schema* relSchemaPtr = Relation::readSchema(dbName, relName);
}



// Iterator for a relation.

class Predicate {
};

#endif
