#include "bm.h"

BufferManager *BufferManager::loneInstance = NULL;

int BufferManager::getTotNoBlocks() {
    return DiskManager::getTotNoBlocks();
}

BufferManager::BufferManager(const char *dbName) {

    DiskManager::openDB(dbName);
    buffer = (char*) malloc(NUM_BUFFS * BLOCK_SIZE);

    long int count;
    // Initializing the BufferPool.
    for (count = 0; count < NUM_BUFFS * BLOCK_SIZE; count++) {
        buffer[count] = '#';
    }
    buffer[count - 2] = '\0';
    filledFrames = 0;

    for (int i = 0; i < NUM_BUFFS; i++) {
        pinCount[i] = 0;
        dirtyFlag[i] = false;
        blockInFrame[i] = -1;
    }
    // Initializing the Queue pointers.
    lruHead = NULL;
    lruTail = NULL;

}

char *BufferManager::pinPage(int blockId) {
    int frameId = getBlockInFrame(blockId);

    // If the block already exists in the BufferPool. 
    if (frameId != -1) {

        if (pinCount[frameId] == 0) {
            deleteNode(frameId);
        }
        pinCount[frameId]++;
        return buffer + frameId * BLOCK_SIZE;
    }

    // If there is enough space in the BufferPool.
    if (filledFrames < NUM_BUFFS) {
        frameId = getFreeFrame();
        assert(frameId != -1);
        DiskManager::rawRead(dbName, blockId,
                buffer + frameId * BLOCK_SIZE);

        blockInFrame[frameId] = blockId;
        filledFrames++;
        pinCount[frameId]++;
        return buffer + frameId*BLOCK_SIZE;
    } else {

        int frameId = evict();
        if (frameId == -2) {
            return NULL;
        }

        if (dirtyFlag[frameId]) {
            forcePage(frameId);
        }

        DiskManager::rawRead(dbName, blockId,
                buffer + frameId * BLOCK_SIZE);
        pinCount[frameId]++;
        blockInFrame[frameId] = blockId;
        return buffer + frameId*BLOCK_SIZE;
    }
}

void BufferManager::unpinPage(char* blockPtr, bool isDirty) {
    int frameId = (blockPtr - buffer) / BLOCK_SIZE;

    if (blockInFrame[frameId] != -1) {

        if (isDirty) {

            dirtyFlag[frameId] = true;
        }

        if (pinCount[frameId] > 0)
            pinCount[frameId]--;
        else {
            cout << "pinCount of block in frame no. " << frameId << endl;
        }

        if (pinCount[frameId] == 0) {

            insertNode(frameId);
        }
    } else {

        cout << "No block pinned in the given frame" << endl;
    }
}

void BufferManager::unpinPage(int blockId, bool isDirty) {
    int frameId = getBlockInFrame(blockId);

    if (frameId != -1) {

        if (isDirty) {
            dirtyFlag[frameId] = true;
        }

        if (pinCount[frameId] > 0)
            pinCount[frameId]--;
        else {
            cout << "pinCount of block " << blockId;
            cout << " in frame no. " << frameId << endl;
        }

        if (pinCount[frameId] == 0) {
            insertNode(frameId);
        }
    } else {
        cout << "No such block pinned  " << blockId << endl;
    }

}

int BufferManager::getDiskBlockId(char* blockPtr) {
    int frameId = (blockPtr - buffer) / BLOCK_SIZE;

    return blockInFrame[frameId];
}

int BufferManager::getBlockInFrame(int blockId) {
    for (int frameId = 0; frameId < NUM_BUFFS; frameId++) {
        if (blockInFrame[frameId] == blockId)
            return frameId;
    }
    return -1;
}

int BufferManager::getFreeFrame() {
    for (int frameId = 0; frameId < NUM_BUFFS; frameId++) {

        if (blockInFrame[frameId] == -1)
            return frameId;
    }
    return -1;
}

void BufferManager::forcePage(int frameId) {
    if (blockInFrame[frameId] != -1) {
        int blockId = blockInFrame[frameId];
        DiskManager::rawWrite(dbName, blockId,
                buffer + frameId * BLOCK_SIZE);
    } else
        cout << "invalid frameId for forcePage" << endl;
}

BufferManager::~BufferManager() {
    int blockId;
    for (int frameId = 0; frameId < NUM_BUFFS; frameId++) {
        blockId = blockInFrame[frameId];

        cout << "bm destructor" << endl;
        if (dirtyFlag[frameId]) {
            DiskManager::rawWrite(dbName, blockId,
                    buffer + frameId * BLOCK_SIZE);
        }
    }
}

void BufferManager::insertNode(int frameId) {
    lruNode *newNode = new lruNode;
    newNode->frameId = frameId;
    newNode->prevPtr = NULL;
    newNode->nextPtr = lruHead;

    if (lruHead != NULL) {
        lruHead->prevPtr = newNode;
    } else lruTail = newNode;

    lruHead = newNode;
}

void BufferManager::deleteNode(int frameId) {
    if (lruHead == NULL && lruTail == NULL) {
        cout << "no node to delete !" << endl;
    } else {
        lruNode *tempPtr = lruHead;
        if (tempPtr == lruHead && tempPtr == lruTail) {
            lruHead = NULL;
            lruTail = NULL;
        } else {
            while (tempPtr != NULL) {
                if (tempPtr->frameId == frameId) {
                    if (tempPtr == lruHead) {
                        tempPtr->nextPtr->prevPtr = NULL;
                        lruHead = tempPtr->nextPtr;
                    } else if (tempPtr == lruTail) {
                        tempPtr->prevPtr->nextPtr = NULL;
                        lruTail = tempPtr->prevPtr;
                    } else {
                        tempPtr->prevPtr->nextPtr = tempPtr->nextPtr;
                        tempPtr->nextPtr->prevPtr = tempPtr->prevPtr;
                    }
                }
                tempPtr = tempPtr->nextPtr;
            }
        }
    }
}

int BufferManager::evict() {
    if (lruHead == NULL && lruTail == NULL) {
        return -2;
    }
    int returnValue = lruTail->frameId;
    if (lruHead != lruTail) {
        (lruTail->prevPtr)->nextPtr = NULL;
        lruTail = lruTail->prevPtr;
    } else {
        lruHead = NULL;
        lruTail = NULL;
    }
    return returnValue;
}

