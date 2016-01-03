#include <iostream>

#include "global.h"
#include "dm.h"
#include "bm.h"

using namespace std;

/**
 * Keeps track of which blocks are used by which objects (the 
 * object could be a table or index or any persistent entity.
 * Also maintains the free list to keep track of free (unallocated)
 * disk blocks.
 *
 * For simplicity assume all the metadata (head of the free list
 * and (object-name, firstBlockId) pairs are maintained in a 
 * single block the very first block of the database.
 *
 * The StorageManager makes use of the BufferManager to fetch
 * the disk blocks in memory and write them back.
 */
class StorageManager {

    public:
    /**
     * Initializes a freshly created database by linking all the disk 
     * blocks in the free list. To be called only once, just after
     * the creation of the database.
     */
    static void initDB(const char *dbName);

    /**
     * Creates an object and allocates the starting block
     * for it.
     */
    static void createObject(const char *dbName, const char *objectName);

    /**
     * Returns the id of the first (starting) block of an object.
     * In the case of a table, this would be the first block. In
     * the case of a B+-tree index, this would be the root node.
     */
    static int getFirstBlockId(const char *dbName, 
                               const char *objectName);

    /**
     * Allocates a new block from the free list. Returns the 
     * block-id.
     */
    static int allocateBlock(const char *dbName);

    /**
     * Frees (deallocates) a given block and adds it to the 
     * free list.
     */
    static int freeBlock(const char *dbName, int blockId);

    /**
     * Drops (removes) an object (table/index) freeing its starting
     * block.
     */
    static int dropObject(const char *dbName, char *objectName);
};

