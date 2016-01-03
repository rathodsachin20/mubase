#include <iostream>

#include "global.h"

using namespace std;

/**
 * Buffer Manager for MuBase. Implements LRU replacement policy.
 */
class BufferManager {

    private:
    /**
     * BufferManager will have a singleton class (direct 
     * instantiation is prohibited - so the constructor is 
     * private.
     */
    static BufferManager *loneInstance;

    /**
     * The db name.
     */
    char *dbName;

    /**
     * The actual buffer pool, which stores the pages.
     */
    char *buffer;

    /**
     * Private constructor.
     */
    BufferManager(const char *dbName) {
        // Open the db and allocate memory for the buffer pool
        // to hold NUM_BUFFS number of blocks.
        return;
    }

    public:

    static BufferManager *getInstance(const char *dbName) {
        if(BufferManager::loneInstance == NULL)
            BufferManager::loneInstance = new BufferManager(dbName);
        return BufferManager::loneInstance;
    }

    /**
     * Fetch the block in memory (if not already present), increment
     * its pinCount and return its address.
     */
    char *pinPage(int blockId);

    /**
     * Reduce the pinCount of the buffer block. If the pinCount 
     * becomes 0, the buffer block will be a candidate for being
     * evicted (eviction happens lazily when the need arises).
     */
    void unpinPage(int blockId, bool isDirty);

    /**
     * Forcibly write a given buffer block to the disk.
     */
    void forcePage(int blockId);

    /**
     * Destructor. Write all dirty pages to disk.
     */
     ~BufferManager();
};

BufferManager *BufferManager::loneInstance = NULL;

