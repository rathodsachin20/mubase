#include <iostream>

#include "global.h"

using namespace std;

/**
 * Class to deal with direct writing of disk blocks. Current 
 * implementation is over a file. Can be extended to work with
 * a raw disk partition as well.
 */
class DiskManager {

    public:
    /**
     * Create a new file and pre-allocates initialNumBlocks of 
     * disk blocks (i.e., file = initialNumBlocks * BLOCK_SIZE).
     */
    static void createDB(const char *dbName, int initialNumBlocks);
   
    /**
     * Opens the file for the given dbName in random access mode.
     * If the db is already open, returns without doing anything.
     */
    static void openDB(const char *dbName);

    /** 
     * Direct (unbufffered) write of disk block. If the db is
     * not open, throws an exception.
     */
    static void rawWrite(const char *dbName, int blockId, char *buff);
    
    /** 
     * Direct (unbufffered) read of disk block. If the db is
     * not open, throws an exception.
     */
    static void rawRead (const char *dbName, int blockId, char **buff);

    /**
     * Close the file.
     */
    static void closeDB(const char *dbName);
};

