#include "rm.h"
#include <cassert>
#include <sstream>
#include <string.h>
#include <cstring>

using namespace std;

//Schema class Functions	

void Schema::setSchema(short int numAttrs, short int* attrType, char **attrName, short int *sizeAttrs) {
    this->numAttrs = numAttrs;
    this->attrType = new short int[MAX_ATTRS];
    this->sizeAttrs = new short int[MAX_ATTRS];
    this->attrName = new char*[MAX_ATTRS];
    this->attrsOffset = new short int [MAX_ATTRS];
    int numOfVarChar = 0;
    for (short int i = 0; i < numAttrs; i++) {
        this->attrType[i] = attrType[i];
        this->sizeAttrs[i] = sizeAttrs[i];
        this->attrName[i] = new char[strlen(attrName[i])];
        for (short int j = 0; j < strlen(attrName[i]); j++) {

            this->attrName[i][j] = attrName[i][j];
        }
        if (attrType[i] == 4)
            numOfVarChar++;

    }

    for (short int i = numAttrs; i < MAX_ATTRS; i++) {
        this->attrType[i] = (short) (-1);
        this->sizeAttrs[i] = -1;      
        this->attrName[i] = "";
        this->attrsOffset[i] = -1;

    }

    short int offsetConst = numOfVarChar * sizeof (short int) *2 + numAttrs / 8;
    if (numAttrs % 8 != 0)
        offsetConst++;
    short int offsetVar = 0;
    for (short int i = 0; i < numAttrs; i++) {

        if (attrType[i] == 4) {
            this->attrsOffset[i] = offsetVar;
            offsetVar = offsetVar + sizeof (short int) *2;
        } else {
            this->attrsOffset[i] = offsetConst;
            offsetConst = offsetConst + sizeAttrs[i];
        }
    }
    this->numVarAttrs = numOfVarChar;
    return;

}

void Schema::print() {
    cout << "\nAttrName | AttrType | SizeAttrs \n";
    for (short int i = 0; i < numAttrs; i++) {
        cout << attrName[i] << ":" << attrType[i] << ":" << sizeAttrs[i] << endl;
    }
}

short Schema::getAttrsOffset(int index) {
    return attrsOffset[index];
}

short Schema::getAttrType(int index) {
    return attrType[index];
}

short Schema::getSizeAttrs(int index) {
    return sizeAttrs[index];
}

short Schema::getNumAttrs() {
    return numAttrs;
}

short Schema::getNumVarAttrs() {
    return numVarAttrs;
}

char* Schema::getAttrName(int index) {
    return attrName[index];
}



//Record class functions

	int Record::getInt(int attrIndex) {
    int value;
    int* intPtr = &value;
    value = readInt(recBytes, schema->getAttrsOffset(attrIndex));
    return value;
}

char* Record::getConstString(int attrIndex) {

    char* value = readBytes(recBytes, schema->getAttrsOffset(attrIndex), schema->getSizeAttrs(attrIndex));
    return value;
}

char* Record::getVarString(int attrIndex) {
    short varoffset = readShort(recBytes, schema->getAttrsOffset(attrIndex));
    short varLength = readShort(recBytes, schema->getAttrsOffset(attrIndex) + sizeof (short));
    char* value = readBytes(recBytes, varoffset, varLength);
    return value;
}

float Record::getFloat(int attrIndex) {
    float value;
    float* floatPtr = &value;
    value = readFloat(recBytes, schema->getAttrsOffset(attrIndex));
    return value;
}

char* Record::getRecBytes() {
    return recBytes;
}

Schema* Record::getSchema() {
    return schema;
}

int Record::getLength() {
    return length;
}

bool Record::isGreater(Record* r, int index) {
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

bool Record::isEqual(Record* r, int index) {
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

bool Record::isLesser(Record* r, int index) {
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

void Record::print() {
    for (int i = 0; i < schema->getNumAttrs(); i++) {
        if (schema->getAttrType(i) == 1) {
            cout << " : " << getInt(i);
        } else if (schema->getAttrType(i) == 2) {
            cout << " : " << getFloat(i);
        } else if (schema->getAttrType(i) == 3) {
            cout << " : " << getConstString(i);
        } else if (schema->getAttrType(i) == 4) {
            cout << " : " << getVarString(i);
        }
    }
    cout << endl;
    return;
}




//initialise the system tables , first through storage manager make 2 relations
//then initialise through slotted page and write them back

void Relation::initSystables() {
    StorageManager::createObject(dbName, "sysObjects");
    int block1 = StorageManager::getFirstBlockId(dbName, "sysObjects");
    char* sysobj = BufferManager::getInstance(dbName)->pinPage(block1);
    SlottedPage sp(sysobj);
    sp.initBlock();
    BufferManager::getInstance(dbName)->unpinPage(block1, 1);


    StorageManager::createObject(dbName, "sysColumns");
    int block2 = StorageManager::getFirstBlockId(dbName, "sysColumns");
    char* syscol = BufferManager::getInstance(dbName)->pinPage(block2);
    SlottedPage sp2(syscol);
    sp2.initBlock();
    BufferManager::getInstance(dbName)->unpinPage(block2, 1);
}

Schema Relation::getSysObjSchema() {
    short numAttrs = 3;
    short attrType[] = {4, 1, 1};
    short sizeOfShort = sizeof (short);
    short sizeAttrs[] = {MAX_RELNAME_SIZE, sizeof (int), sizeof (int)};
    char **attrName;
    string temp[3] = {"ObjectName", "type", "arity"};
    attrName = new char*[3];
    attrName[0] = &temp[0][0];
    attrName[1] = &temp[1][0];
    attrName[2] = &temp[2][0];
    Schema sch(numAttrs, attrType, attrName, sizeAttrs);
    return sch;
}

Schema Relation::getSysColSchema() {
    short numAttrs = 5;
    short attrType[] = {4, 1, 4, 1, 1};
    short sizeOfShort = sizeof (short);
    short sizeAttrs[] = {MAX_RELNAME_SIZE, sizeof (int), MAX_COLNAME_SIZE, sizeof (int), sizeof (int)};
    char **attrName;
    attrName = new char*[5];
    string temp[5] = {"ObjectName", "index", "colname", "coltype", "colsize"};
    attrName[0] = &temp[0][0];
    attrName[1] = &temp[1][0];
    attrName[2] = &temp[2][0];
    attrName[3] = &temp[3][0];
    attrName[4] = &temp[4][0];
    Schema sch(numAttrs, attrType, attrName, sizeAttrs);
    return sch;
}
/*
 * test schema
Schema Relation::getSomeSchema() {
    short int numAttrs = 3;
    short int attrType[3] = {4, 4, 4};
    char **attrName;
    attrName = new char*[3];

    string temp[3] = {"jhfhj", "iname", "icgpa"};
    attrName[0] = &temp[0][0];
    attrName[1] = &temp[1][0];
    attrName[2] = &temp[2][0];

    short int sizeAttrs[3] = {4, 4, 4};

    Schema sch(numAttrs, attrType, attrName, sizeAttrs);

    return sch;
}
 */

Schema* Relation::readSchema(const char* dbName1, const char* relName1) {
    Schema* relSchema;
    int relArity = 0;
    short int* attrType;
    char **attrName;
    short int *sizeAttrs;
   
    int soblock0 = StorageManager::getFirstBlockId(dbName1, "sysObjects");
    int scblock0 = StorageManager::getFirstBlockId(dbName1, "sysColumns");

    if (soblock0 == -1 || scblock0 == -1) {
        cout << "Systemtables not found!\n";
        return NULL;
    }

    char* soBlockBuff = BufferManager::getInstance(dbName1)->pinPage(soblock0);
    char* scBlockBuff = BufferManager::getInstance(dbName1)->pinPage(scblock0);

    SlottedPage sosp(soBlockBuff);
    SlottedPage scsp(scBlockBuff);

    Record* recPtr;
    char* recBytes;
    int l;
    Schema* soSchema = new Schema(getSysObjSchema());
    for (int i = 0; i < sosp.getTotalNoOfSlots(); i++) {
       
        recBytes = sosp.getRecord(i, l);

        if (recBytes == (char*) - 1)continue;

        recPtr = new Record(recBytes, soSchema, l);                      
        if (strcmp(recPtr->getVarString(0), relName1) == 0 && recPtr->getInt(1) == 1) {            
            relArity = recPtr->getInt(2);
            break;
        }

    }
    if (relArity == 0) {
        return NULL;
    }
    attrName = new char*[relArity];
    attrType = new short[relArity];
    sizeAttrs = new short[relArity];

    Schema* scSchema = new Schema(getSysColSchema());
    int colNum;
    int currColCount = 0;
    int l1;
 
    for (int i = 0; i < scsp.getTotalNoOfSlots(); i++) {
        recBytes = scsp.getRecord(i, l1);
        if (recBytes == (char*) - 1)continue;

        recPtr = new Record(recBytes, scSchema, l1);
        //recPtr->print();
        if (strcmp(recPtr->getVarString(0), relName1) == 0) {
            colNum = recPtr->getInt(1);
            attrName[colNum] = recPtr->getVarString(2);
            attrType[colNum] = recPtr->getInt(3);
            sizeAttrs[colNum] = recPtr->getInt(4);
            currColCount++;
            if (currColCount == relArity) {
                break;
            }
        }

    }
    if (currColCount != relArity) {
        return NULL;
    }
    relSchema = new Schema(relArity, attrType, attrName, sizeAttrs);
    return relSchema;
}

char* Record::dataToRecBytes(Schema *schema, int sizeOfData, string* dataArray, int& l) {
    if (sizeOfData != schema->getNumAttrs())
        return NULL;


    int lastVarAttr;
    string recbyte = "";

    char* nullBits;
    int numNullBytes;
    string offsetSize = "";
    if (schema->getNumAttrs() % 8 == 0)
        numNullBytes = schema->getNumAttrs() / 8;
    else
        numNullBytes = (schema->getNumAttrs() / 8) + 1;

    char *recByte;
    recByte = new char[500];
    int dataOffset = schema->getNumVarAttrs() * sizeof (short) *2 + numNullBytes;

    nullBits = new char[numNullBytes];
    for (short int i = 0; i < schema->getNumAttrs(); i++) { //char* cDataArray = &dataArray(i)[0];
        if (schema->getAttrType(i) == 3) {
            if (dataArray[i] == "")
                nullBits[i] = '1';
            else {          

                char* temp;
                temp = &dataArray[i][0];

                writeBytes(recByte, dataOffset, temp);

                nullBits[i] = '0';
                dataOffset += schema->getSizeAttrs(i);

            }
        } else if (schema->getAttrType(i) == 2) {
            if (dataArray[i] == "")
                nullBits[i] = '1';
            else {

                float floatData;
                stringstream ss;
                ss << dataArray[i];
                ss >> floatData;

                writeFloat(recByte, dataOffset, floatData);
                dataOffset += sizeof (float);
                nullBits[i] = '0';
            }
        } else if (schema->getAttrType(i) == 1) {
            if (dataArray[i] == "")
                nullBits[i] = '1';
            else {            
                int intData;
                stringstream ss;
                ss << dataArray[i];
                ss >> intData;
                writeInt(recByte, dataOffset, intData);
                dataOffset += sizeof (int);              
                nullBits[i] = '0';
            }
        }


    }

    int dataOffset2 = 0;
    for (int i = 0; i < schema->getNumAttrs(); i++) {
        if (schema->getAttrType(i) == 4) {
            if (dataArray[i] == "")
                nullBits[i] = '1';
            else {
                short sizeAttr = dataArray[i].size();
                stringstream ss, ss2;
                string tmp1, tmp2;
                ss << lastVarAttr;
                ss >> tmp1;
                ss2 << sizeAttr;
                ss >> tmp2;

                writeShort(recByte, dataOffset2, dataOffset);
                dataOffset2 += sizeof (short);
                writeShort(recByte, dataOffset2, sizeAttr);
                dataOffset2 += sizeof (short);

                char *temp;
                temp = &dataArray[i][0];
                writeBytes(recByte, dataOffset, temp);

                dataOffset += sizeAttr;

                nullBits[i] = '0';
            }
        }

    }

    for (int i = dataOffset2; i < numNullBytes + dataOffset2; i++) {
        recByte[i] = '0';
    }

    l = dataOffset;

    return recByte;

}

int Relation::getLastWrittenBlockId() {
    if (lastBlockId > 0)
        return lastBlockId;
    else {
        lastBlockId = StorageManager::getFirstBlockId(dbName, relName);
        char* buffData = BufferManager::getInstance(dbName)->pinPage(lastBlockId);
        SlottedPage sp(buffData);
        int nextBlockId = sp.getNextBlockId();
        BufferManager::getInstance(dbName)->unpinPage(lastBlockId, false);

        while (nextBlockId > 0) {
            lastBlockId = nextBlockId;
            buffData = BufferManager::getInstance(dbName)->pinPage(lastBlockId);
            SlottedPage sp(buffData);
            int nextBlockId = sp.getNextBlockId();
            BufferManager::getInstance(dbName)->unpinPage(lastBlockId, false);
        }

        return lastBlockId;
    }

}

int* Relation::addRecord(Record rec) {

    if (lastBlockId < 0)
        lastBlockId = getLastWrittenBlockId();
    int * recordId = addRecord(rec, lastBlockId);

    return recordId;

}

int* Relation::addRecord(Record rec, int blockId) {
    int * recordId = new int[2];
    char* buffData = BufferManager::getInstance(dbName)->pinPage(blockId);
    SlottedPage sp(buffData);
    int slotId = sp.storeRecord(rec.getRecBytes(), rec.getLength());
    if (slotId < 0) {
        int newBlockId = StorageManager::allocateBlock(dbName);
        if (newBlockId < 0) {
            recordId[0] = -1;
            recordId[1] = -1;
            return recordId;
        }
        char* newBuffData = BufferManager::getInstance(dbName)->pinPage(newBlockId);
        SlottedPage spNew(newBuffData);
        spNew.initBlock();
        slotId = spNew.storeRecord(rec.getRecBytes(), rec.getLength());
        sp.writeNextBlockId(newBlockId);
        BufferManager::getInstance(dbName)->unpinPage(newBlockId, true);
        lastBlockId = newBlockId;
    }
    BufferManager::getInstance(dbName)->unpinPage(blockId, true);
    recordId[0] = blockId;
    recordId[1] = slotId;
    return recordId;
}

Record* Relation::getRecord(int *recId) {
    int blockId = recId[0];
    int slotNo = recId[1];
    if (blockId < 0 || slotNo < 0)return NULL;

    char* buffData = BufferManager::getInstance(dbName)->pinPage(blockId);
    SlottedPage sp(buffData);
    char* recBytes;
    int l;
    recBytes = sp.getRecord(slotNo, l);
    Record* recp;
    recp = new Record(recBytes, schema, l);
    BufferManager::getInstance(dbName)->unpinPage(blockId, false);
    return recp;
}

Record* Relation::getRecord(int blockId, int slotNo) {
    int* recId = new int[2];
    recId[0] = blockId;
    recId[1] = slotNo;
    return getRecord(recId);
}

int Relation::deleteRecord(int* recId) {
    int blockId = recId[0];
    int slotNo = recId[1];
    if (blockId < 0 || slotNo < 0)return 0;

    char* buffData = BufferManager::getInstance(dbName)->pinPage(blockId);
    SlottedPage sp(buffData);
    sp.deleteRecord(slotNo);
    BufferManager::getInstance(dbName)->unpinPage(blockId, true);
    return 1;
}

int Relation::deleteRecord(int blockId, int slotNo) {
    int* recId = new int[2];
    recId[0] = blockId;
    recId[1] = slotNo;
    return deleteRecord(recId);
}

RIterator* Relation::openScan() {
    RIterator* ri = new BaseRelIterator(dbName, relName);
    return ri;
}

// Predicated scan. Should make use of the B+ tree index
// when an index is available on the required column(s).

RIterator* Relation::openScan(Predicate* p) {
    RIterator* ri = new PredRelIterator(dbName, relName, p);
    return ri;
}





