#include "sm.h"

class TreeNode {
    private:
    char *buffPage;

    public:
    TreeNode(char *buffPage) {
        this->buffPage = buffPage;
    }

    char *getBuffPage() {
        return buffPage;
    }
};


class BPlusTree {

    TreeNode *root;

    public:

    BPlusTree(const char *dbName, const char *indexName, bool create) {
        BufferManager *bm = BufferManager::getInstance(dbName);

        if(create) {
            StorageManager::createObject(dbName, indexName);
        }
        int rootBlockId = StorageManager::getFirstBlockId(dbName, 
                                          indexName);

        char *rootNodeBuff = bm->pinPage(rootBlockId);
        root = new TreeNode(rootNodeBuff);
    }


    /**
     * Insert an entry into the index.
     */
    void insertEntry(int key, int rid);


    /**
     * Delete an entry from the index. For now, assume the index
     * to be on a candidate key.
     */
    void deleteEntry(int key);


    /**
     * Index lookup for a key. Returns the rid if found, else -1.
     * Assuming the index on a candidate key.
     */
    int lookup(int key);

    /**
     * Destructor. Just force all the dirty buffer pages to disk.
     */
    ~BPlusTree() {
    }
};

