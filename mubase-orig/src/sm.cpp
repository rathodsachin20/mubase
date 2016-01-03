#include "sm.h"

void StorageManager::initDB(const char *dbName) {
    BufferManager *bm = BufferManager::getInstance(dbName);
    int initialNumBlocks = bm->getTotNoBlocks();
    cout << initialNumBlocks << endl;
    char *diskBlock;
    for (int i = 0; i < initialNumBlocks - 1; i++) {
        //cout<<i;
        diskBlock = bm->pinPage(i);
        writeInt(diskBlock, 0, i + 1);
        bm->unpinPage(i, true);
    }

    diskBlock = bm->pinPage(initialNumBlocks - 1);
    writeInt(diskBlock, 0, -1);
    bm->unpinPage(initialNumBlocks - 1, true);
    diskBlock = bm->pinPage(0);
    //initialise the value of nofObjects(see createObject) with 0
    writeInt(diskBlock, SIZE_INT, 0);
    bm->unpinPage(0, true);

}

void StorageManager::createObject(const char *dbName,
        const char *objectName) {
    BufferManager *bm = BufferManager::getInstance(dbName);
    char *diskFirstBlock = bm->pinPage(0);

    int nofObjects = readInt(diskFirstBlock, SIZE_INT);

    //if(nofObjects >= (BLOCK_SIZE/22)-1 )
    //throw exception saying that not enough sapce 4 new object

    char name[OBJNAME_SIZE];
    for (int i = 0; objectName[i] != '\0'; i++) {
        name[i] = objectName[i];
        name[i + 1] = '\0';
    }
    //objectName assumed to take not more than OBJNAME_SIZE characters
    writeBytes(diskFirstBlock,
            2 * SIZE_INT + nofObjects * (OBJ_SIZE), name);
    //allocating new block
    int ObjectFirstBlockId = allocateBlock(dbName);
    char *firstFreeBlock = bm->pinPage(ObjectFirstBlockId);
    writeInt(firstFreeBlock, 0, 0);
    writeInt(firstFreeBlock, PTR_SIZE, 1);
    bm->unpinPage(ObjectFirstBlockId, true);

    // writing first blockId alloted to the created object 
    writeInt(diskFirstBlock,
            2 * SIZE_INT + nofObjects * (OBJ_SIZE) +
            OBJNAME_SIZE, ObjectFirstBlockId);
    //updating the number of objects
    writeInt(diskFirstBlock, SIZE_INT, nofObjects + 1);
    bm->unpinPage(0, true);
}

int StorageManager::getFirstBlockId(const char *dbName,
        const char *objectName) {
    BufferManager *bm = BufferManager::getInstance(dbName);
    char *diskFirstBlock = bm->pinPage(0);
    int nofObjects = readInt(diskFirstBlock, SIZE_INT);
    char *objName;
    int ObjectFirstBlockId;
    //cout << " nofObjects " << nofObjects << endl;
    for (int i = 0; i < nofObjects; i++) {
        objName = readBytes(diskFirstBlock,
                2 * SIZE_INT + i*OBJ_SIZE,
                OBJNAME_SIZE - 1);
        
        if (strcmp(objName, objectName) == 0) {
            ObjectFirstBlockId = readInt(diskFirstBlock,
                    2 * SIZE_INT + i * OBJ_SIZE +
                    OBJNAME_SIZE);
            bm->unpinPage(0, false);
            free(objName);
            return ObjectFirstBlockId;
        }
    }
    //cout << "Object Not found" << endl;
    return -1;
}

int StorageManager::allocateBlock(const char *dbName) {
    BufferManager *bm = BufferManager::getInstance(dbName);
    char *diskFirstBlock = bm->pinPage(0);
    int firstFreeBlockId = readInt(diskFirstBlock, 0);

    char *firstFreeBlock = bm->pinPage(firstFreeBlockId);
    int secondFreeBlockId = readInt(firstFreeBlock, 0);

    writeInt(diskFirstBlock, 0, secondFreeBlockId);

    bm->unpinPage(0, true);
    bm->unpinPage(firstFreeBlockId, false);
    return firstFreeBlockId;
}

int StorageManager::freeBlock(const char *dbName, int blockId) {
    BufferManager *bm = BufferManager::getInstance(dbName);
    char *diskFirstBlock = bm->pinPage(0);
    char *newFreeBlock = bm->pinPage(blockId);

    int firstFreeBlockId = readInt(diskFirstBlock, 0);

    writeInt(diskFirstBlock, 0, blockId);
    writeInt(newFreeBlock, 0, firstFreeBlockId);

    bm->unpinPage(0, true);
    bm->unpinPage(blockId, true);
}

void StorageManager::dropObject(const char *dbName,
        char *objectName) {
    BufferManager *bm = BufferManager::getInstance(dbName);
    char *diskFirstBlock = bm->pinPage(0);
    int nofObjects = readInt(diskFirstBlock, SIZE_INT);
    char *objName;
    int ObjectFirstBlockId;

    for (int i = 0; i < nofObjects; i++) {
        objName = readBytes(diskFirstBlock,
                2 * SIZE_INT + i*OBJ_SIZE, OBJNAME_SIZE);

        if (strcmp(objName, objectName)) {
            ObjectFirstBlockId = readInt(diskFirstBlock,
                    2 * SIZE_INT + i * OBJ_SIZE +
                    OBJNAME_SIZE);
            freeBlock(dbName, ObjectFirstBlockId);

            makeCompact(diskFirstBlock,
                    2 * SIZE_INT + i*OBJ_SIZE,
                    2 * SIZE_INT + (i + 1) * OBJ_SIZE,
                    (nofObjects - i - 1) * OBJ_SIZE);
            writeInt(diskFirstBlock, SIZE_INT, nofObjects - 1);

            bm->unpinPage(0, false);
            free(objName);
            break;
        }
    }
}

