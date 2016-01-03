#include "bt.h"

void BPlusTree::insertEntry(int key, int rid) {

    BufferManager *bm = BufferManager::getInstance(dbName);
    int height;
    TreeNode **trace = new TreeNode *[10];
    for (int i = 0; i < 10; i++)
        trace[i] = NULL;

    TreeNode *leafNode = getLeaf(key, trace, &height);
    //cout<<endl<<"height of the tree: "<<height<<endl;
    if (leafNode == NULL)
        cout << "exception null node" << endl;

    cout << "lookup " << lookUpWithoutUnpin(key, leafNode) << endl;
    if (lookUpWithoutUnpin(key, leafNode) != -1) {
        cout << "key already exists" << endl;
        for (int i = height; i >= 1; i--) {
            //cout<<"unpinning trace:"<<endl;
            bm->unpinPage(trace[i]->getBuffPage(), false);
            delete trace[i];
        }
    }
    else {

        int nofPairs = readInt(leafNode->getBuffPage(), 0);
        //if num of pairs less than n-1
        if (nofPairs < maxNoPairs) {

            insertInLeaf(leafNode, key, rid);
            if (height != 0) {

                bm->unpinPage(leafNode->getBuffPage(), true);
                //cout<<"unpinning the leaf node which is not a root"<<endl;
            }
            //delete leafNode;			

            for (int i = height - 1; i >= 1; i--) {
                //cout<<"unpinning trace:"<<endl;
                bm->unpinPage(trace[i]->getBuffPage(), false);
                delete trace[i];
            }

        } else {
            int newLeafId = StorageManager::allocateBlock(dbName);
            char* newLeafNodeBuff = bm->pinPage(newLeafId);
            TreeNode *newLeafNode = new TreeNode(newLeafNodeBuff);

            int divideKeyOffset = maxNoPairs / 2;
            //cout<<"divideKeyOffset: "<<divideKeyOffset<<endl;
            int divideKey = readInt(leafNode->getBuffPage(),
                    2 * SIZE_INT + divideKeyOffset * PAIR_SIZE + PTR_SIZE);

            cout << "divideKey: " << divideKey << endl;
            if (key < divideKey) {

                moveBytes(leafNode->getBuffPage() +
                        2 * SIZE_INT + divideKeyOffset*PAIR_SIZE,
                        newLeafNode->getBuffPage() + 2 * SIZE_INT,
                        (maxNoPairs - divideKeyOffset) * PAIR_SIZE);

                //writing "nofPairs" into new leaf				
                //writeInt(newLeafNode->getBuffPage(),0,(maxNoPairs+1)/2);
                newLeafNode->setNoPairs((maxNoPairs + 1) / 2);

                //writing "isLeaf" into new leaf
                //writeInt(newLeafNode->getBuffPage(),SIZE_INT,1);
                newLeafNode->setIsLeaf(1);

                //copying P(n) of leaf to P(n) of new leaf
                moveBytes(leafNode->getBuffPage() +
                        BLOCK_SIZE - PTR_SIZE,
                        newLeafNode->getBuffPage() +
                        BLOCK_SIZE - PTR_SIZE, PTR_SIZE);

                //make p(n) of leaf to point to new leaf
                writeInt(leafNode->getBuffPage(),
                        BLOCK_SIZE - PTR_SIZE, newLeafId);

                leafNode->setNoPairs(maxNoPairs / 2);

                insertInLeaf(leafNode, key, rid);
                bm->unpinPage(newLeafId, true);

                insertInParent(leafNode, divideKey,
                        newLeafId, trace, height);
            } else {

                moveBytes(leafNode->getBuffPage() +
                        2 * SIZE_INT + (divideKeyOffset + 1) * PAIR_SIZE,
                        newLeafNode->getBuffPage() + 2 * SIZE_INT,
                        (maxNoPairs - divideKeyOffset - 1) * PAIR_SIZE);

                //writing "nofPairs" into new leaf				
                //writeInt(newLeafNode->getBuffPage(),0,maxNoPairs-divideKeyOffset-1);
                newLeafNode->setNoPairs(maxNoPairs - divideKeyOffset - 1);

                //writing "isLeaf" into new leaf
                //writeInt(newLeafNode->getBuffPage(),SIZE_INT,1);
                newLeafNode->setIsLeaf(1);

                //copying P(n) of leaf to P(n) of new leaf
                moveBytes(leafNode->getBuffPage() +
                        BLOCK_SIZE - PTR_SIZE, newLeafNode->getBuffPage() +
                        BLOCK_SIZE - PTR_SIZE, PTR_SIZE);

                //make p(n) of leaf to point to new leaf
                writeInt(leafNode->getBuffPage(),
                        BLOCK_SIZE - PTR_SIZE, newLeafId);

                //writing "nofPairs" into leaf	
                leafNode->setNoPairs(divideKeyOffset + 1);

                insertInLeaf(newLeafNode, key, rid);

                int startKey = readInt(newLeafNode->getBuffPage(),
                        2 * SIZE_INT + PTR_SIZE);

                bm->unpinPage(newLeafId, true);
                //cout<<"task completed and unpinnednew leaf id"<<endl;			
                insertInParent(leafNode, startKey,
                        newLeafId, trace, height);

            }
        }
        //bm->unpinPage(leafId,true);
        cout << endl << key << " inserted" << endl << endl;
    }
    //free (trace);	 	
    //cout<<"no of pairs in root: "<<readInt(root->getBuffPage(), 0)<<endl;   	
}

void BPlusTree::insertInParent(TreeNode *node,
        int startKeyInSplitNode,
        int splitNodeId,
        TreeNode** trace, int level) {

    BufferManager *bm = BufferManager::getInstance(dbName);
    //cout<<endl<<"level: "<<level<<"startKeyInSplitNode: "<<startKeyInSplitNode<<endl;
    if (level == 0) {
        //cout<<"parent is root"<<endl;
        int newId = StorageManager::allocateBlock(dbName);
        char* newNodeBuff = bm->pinPage(newId);
        TreeNode *newNode = new TreeNode(newNodeBuff);

        //copying the content of old root to new node
        moveBytes(root->getBuffPage(), newNode->getBuffPage(),
                BLOCK_SIZE);

        root->setNoPairs(1);
        root->setIsLeaf(0);

        writeInt(root->getBuffPage(), 2 * SIZE_INT, newId);

        writeInt(root->getBuffPage(),
                2 * SIZE_INT + PTR_SIZE, startKeyInSplitNode);

        writeInt(root->getBuffPage(),
                2 * SIZE_INT + PAIR_SIZE, splitNodeId);

        bm->unpinPage(newId, true);
        delete newNode;
        cout << "parent is root: done" << endl;
    } else {
        //cout<<"else case entered"<<endl;
        TreeNode *parentNode = trace[level - 1];
        int noPairs = parentNode->getNoPairs();
        cout << "no of pairs" << noPairs << endl;
        if (noPairs < maxNoPairs) {
            //int childId = getDiskBlockId( node->getBuffPage() );
            int firstKeyInNode = readInt(node->getBuffPage(),
                    2 * SIZE_INT + PTR_SIZE);

            int keyCount;
            int ithKey = readInt(parentNode->getBuffPage(),
                    2 * SIZE_INT + PTR_SIZE);

            for (keyCount = 0; keyCount < noPairs &&
                    firstKeyInNode >= ithKey; keyCount++) {

                ithKey = readInt(parentNode->getBuffPage(),
                        2 * SIZE_INT + (keyCount + 1) * PAIR_SIZE + PTR_SIZE);
            }

            cout << " keycount: " << keyCount << " stop Key " << ithKey << endl;

            if (keyCount < noPairs) {

                moveBytes(parentNode->getBuffPage() + 2 * SIZE_INT +
                        keyCount * PAIR_SIZE + PTR_SIZE,
                        parentNode->getBuffPage() + 2 * SIZE_INT +
                        (keyCount + 1) * PAIR_SIZE + PTR_SIZE,
                        (noPairs - keyCount) * PAIR_SIZE);

                //writes split key	
                writeInt(parentNode->getBuffPage(),
                        2 * SIZE_INT + keyCount * PAIR_SIZE +
                        PTR_SIZE, startKeyInSplitNode);

                //writes split node id just after split key
                writeInt(parentNode->getBuffPage(),
                        2 * SIZE_INT + (keyCount + 1) * PAIR_SIZE, splitNodeId);
            } else {

                //writes split key to k of nth pair(new apir)
                writeInt(parentNode->getBuffPage(),
                        2 * SIZE_INT + noPairs * PAIR_SIZE + PTR_SIZE,
                        startKeyInSplitNode);

                //writes splitNodeId to p(n)( at the end) 
                writeInt(parentNode->getBuffPage(),
                        2 * SIZE_INT + (noPairs + 1) * PAIR_SIZE, splitNodeId);
            }
            parentNode->setNoPairs(noPairs + 1);

            bm->unpinPage(trace[level]->getBuffPage(), true);
            delete trace[level];
            for (int i = level - 1; i >= 1; i--) {
                bm->unpinPage(trace[i]->getBuffPage(), false);
                delete trace[i];
            }
        } else {
            cout << "parent splitting" << endl;
            int newNodeId = StorageManager::allocateBlock(dbName);
            char* newNodeBuff = bm->pinPage(newNodeId);
            TreeNode *newNode = new TreeNode(newNodeBuff);

            char temp[BLOCK_SIZE + 1];
            moveBytes(parentNode->getBuffPage() + 2 * SIZE_INT,
                    temp, BLOCK_SIZE - 2 * SIZE_INT);

            int keyCount;
            int ithKey = readInt(temp, PTR_SIZE);

            for (keyCount = 0; keyCount < maxNoPairs &&
                    startKeyInSplitNode > ithKey; keyCount++) {

                ithKey = readInt(temp,
                        (keyCount + 1) * PAIR_SIZE + PTR_SIZE);
            }

            if (keyCount < maxNoPairs) {

                moveBytes(temp + keyCount * PAIR_SIZE + PTR_SIZE,
                        temp + (keyCount + 1) * PAIR_SIZE + PTR_SIZE,
                        (maxNoPairs - keyCount) * PAIR_SIZE);

                writeInt(temp, keyCount * PAIR_SIZE + PTR_SIZE,
                        startKeyInSplitNode);

                writeInt(temp, (keyCount + 1) * PAIR_SIZE, splitNodeId);
            } else {

                writeInt(temp, maxNoPairs * PAIR_SIZE + PTR_SIZE,
                        startKeyInSplitNode);

                writeInt(temp, (maxNoPairs + 1) * PAIR_SIZE, splitNodeId);
            }

            moveBytes(temp, parentNode->getBuffPage() + 2 * SIZE_INT,
                    (maxNoPairs / 2) * PAIR_SIZE + PTR_SIZE);

            int splitKey = readInt(temp,
                    (maxNoPairs / 2) * PAIR_SIZE + PTR_SIZE);

            moveBytes(temp + (maxNoPairs / 2 + 1) * PAIR_SIZE,
                    newNode->getBuffPage() + 2 * SIZE_INT,
                    ((maxNoPairs + 1) / 2) * PAIR_SIZE + PTR_SIZE);

            parentNode->setNoPairs(maxNoPairs / 2);
            newNode->setNoPairs((maxNoPairs + 1) / 2);
            newNode->setIsLeaf(0);

            bm->unpinPage(newNodeId, true);
            delete newNode;

            insertInParent(parentNode, splitKey,
                    newNodeId, trace, level - 1);

            bm->unpinPage(trace[level]->getBuffPage(), true);
            delete trace[level];
        }
    }
    cout << "done" << endl;
}

void BPlusTree::insertInLeaf(TreeNode *leafNode, int key, int rid) {
    int noPairs = leafNode->getNoPairs();
    int ithKey = readInt(leafNode->getBuffPage(),
            2 * SIZE_INT + PTR_SIZE);
    int keyCount;

    for (keyCount = 0; keyCount < noPairs && key > ithKey; keyCount++) {

        //cout<<"ithKey in loop"<<keyCount<<": "<<ithKey<<endl;
        ithKey = readInt(leafNode->getBuffPage(),
                2 * SIZE_INT + (keyCount + 1) * PAIR_SIZE + PTR_SIZE);
    }

    cout << "pair no. " << keyCount <<
            " among(0 to noPairs-1) " << noPairs << " pairs" << endl;

    if (keyCount < noPairs) {

        moveBytes(leafNode->getBuffPage() +
                2 * SIZE_INT + keyCount*PAIR_SIZE,
                leafNode->getBuffPage() +
                2 * SIZE_INT + (keyCount + 1) * PAIR_SIZE,
                (noPairs - keyCount) * PAIR_SIZE);

        writeInt(leafNode->getBuffPage(),
                2 * SIZE_INT + keyCount*PAIR_SIZE, rid);

        writeInt(leafNode->getBuffPage(),
                2 * SIZE_INT + keyCount * PAIR_SIZE + PTR_SIZE, key);
    } else {

        writeInt(leafNode->getBuffPage(),
                2 * SIZE_INT + noPairs*PAIR_SIZE, rid);
        writeInt(leafNode->getBuffPage(),
                2 * SIZE_INT + noPairs * PAIR_SIZE + PTR_SIZE, key);
        cout << "inserted at the end" << endl;
    }
    leafNode->setNoPairs(noPairs + 1);

    cout << "no of pairs updated to: " <<
            readInt(leafNode->getBuffPage(), 0);
}

/**
 * Delete an entry from the index. For now, assume the index
 * to be on a candidate key.
 */
void BPlusTree::deleteEntry(int key) {

    BufferManager *bm = BufferManager::getInstance(dbName);
    int height;
    //TreeNode **trace = (TreeNode **)malloc(10);

    TreeNode **trace = new TreeNode *[10];
    for (int i = 0; i < 10; i++)
        trace[i] = NULL;

    TreeNode *leafNode = getLeaf(key, trace, &height);

    int rid = lookUpWithoutUnpin(key, leafNode);

    if (rid == -1)
        cout << "key doesnot exists" << endl;
    else {

        recursiveDeleteEntry(leafNode, key, rid, trace, height);
    }
}

void BPlusTree::recursiveDeleteEntry(TreeNode *node, int key,
        int pointer, TreeNode** trace, int level) {

    BufferManager *bm = BufferManager::getInstance(dbName);
    int noPairsInNode = node->getNoPairs();
    int node_diskBlockId;
    int keyCount;
    int ithKey;

    if (node->isLeaf()) {

        cout << "delete key: " << key <<
                " pointer: " << pointer << " in leaf" << endl;

        ithKey = readInt(node->getBuffPage(), 2 * SIZE_INT + PTR_SIZE);
        for (keyCount = 0; keyCount < noPairsInNode &&
                key != ithKey; keyCount++) {

            ithKey = readInt(node->getBuffPage(),
                    2 * SIZE_INT + (keyCount + 1) * PAIR_SIZE + PTR_SIZE);
        }

        moveBytes(node->getBuffPage() +
                2 * SIZE_INT + (keyCount + 1) * PAIR_SIZE,
                node->getBuffPage() +
                2 * SIZE_INT + keyCount*PAIR_SIZE,
                (noPairsInNode - keyCount - 1) * PAIR_SIZE);

        node->setNoPairs(noPairsInNode - 1);
    } else {

        cout << "delete key: " << key <<
                " pointer: " << pointer << " in non leaf" << endl;

        ithKey = readInt(node->getBuffPage(), 2 * SIZE_INT + PTR_SIZE);

        for (keyCount = 0; keyCount < noPairsInNode &&
                key != ithKey; keyCount++) {

            //cout<<"ithKey "<<ithKey<<endl;
            ithKey = readInt(node->getBuffPage(),
                    2 * SIZE_INT + (keyCount + 1) * PAIR_SIZE + PTR_SIZE);
        }

        moveBytes(node->getBuffPage() + 2 * SIZE_INT +
                (keyCount + 1) * PAIR_SIZE + PTR_SIZE,
                node->getBuffPage() + 2 * SIZE_INT +
                keyCount * PAIR_SIZE + PTR_SIZE,
                (noPairsInNode - keyCount - 1) * PAIR_SIZE);

        node->setNoPairs(noPairsInNode - 1);
    }

    noPairsInNode = node->getNoPairs();
    //node is root and only one pointer in it after deletion
    if (level == 0) {
        //root is not a leaf
        if (node->isLeaf() == 0 && noPairsInNode == 0) {

            cout << "root is not a leaf(termination condition)" << endl;
            int rootChildId = readInt(node->getBuffPage(), 2 * SIZE_INT);
            TreeNode *rootChildNode =
                    new TreeNode(bm->pinPage(rootChildId));

            moveBytes(rootChildNode->getBuffPage(),
                    node->getBuffPage(),
                    BLOCK_SIZE);

            bm->unpinPage(rootChildId, false);
            StorageManager::freeBlock(dbName, rootChildId);
        }
        cout << "terminates" << endl;
        //no unpin needed here
    } else if (noPairsInNode < (maxNoPairs + 1) / 2 && level != 0) {

        TreeNode *parentNode = trace[level - 1];
        int noPairsInParent = parentNode->getNoPairs();
        int kDash;

        int searchKey = readInt(node->getBuffPage(),
                2 * SIZE_INT + PTR_SIZE);

        ithKey = readInt(parentNode->getBuffPage(),
                2 * SIZE_INT + PTR_SIZE);

        for (keyCount = 0; keyCount < noPairsInParent &&
                searchKey >= ithKey; keyCount++) {

            ithKey = readInt(parentNode->getBuffPage(),
                    2 * SIZE_INT + (keyCount + 1) * PAIR_SIZE + PTR_SIZE);
        }

        TreeNode *siblingNode;
        int siblingNodeId;
        int swapCheck = 0;
        int noPairsInSibling;
        int nodeToBeDeleted;

        //finding N'
        /**
         * sibling is the leftmost node wherein swapping 
         * the pointers N and N' is needed
         */
        if (keyCount == 0) {
            //sibling to unpined using siblingNodeId
            node_diskBlockId = readInt(parentNode->getBuffPage(),
                    2 * SIZE_INT);

            siblingNodeId = readInt(parentNode->getBuffPage(),
                    2 * SIZE_INT + PAIR_SIZE);

            siblingNode = new TreeNode(bm->pinPage(siblingNodeId));
            kDash = ithKey;
            swapCheck = 1;
            nodeToBeDeleted = siblingNodeId;
        }            /**
		  * sibling is not thethe leftmost node 
		  */
        else {
            node_diskBlockId = readInt(parentNode->getBuffPage(),
                    2 * SIZE_INT + keyCount * PAIR_SIZE);

            siblingNodeId = readInt(parentNode->getBuffPage(),
                    2 * SIZE_INT + (keyCount - 1) * PAIR_SIZE);

            siblingNode = new TreeNode(bm->pinPage(siblingNodeId));
            kDash = readInt(parentNode->getBuffPage(),
                    2 * SIZE_INT + (keyCount - 1) *
                    PAIR_SIZE + PTR_SIZE);

            nodeToBeDeleted = readInt(parentNode->getBuffPage(),
                    2 * SIZE_INT + (keyCount) * PAIR_SIZE);
        }
        noPairsInSibling = siblingNode->getNoPairs();


        int mergeCheck = 0;
        if (node->isLeaf()) {

            if (node->getNoPairs() + noPairsInSibling <= maxNoPairs) {

                mergeCheck = 1;
            }

        } else {

            if (node->getNoPairs() + noPairsInSibling +
                    1 <= maxNoPairs) {

                mergeCheck = 1;
            }

        }

        if (mergeCheck == 1) {
            cout << "merging case" << endl;
            /**
             * swaping the names of the node , sibling we always make
             * sure that we dump(or merge ) the content of 
             * left node into right node ,but the siblingNodeId is 
             * same(new treenode which is not in trace)  		
             */
            if (swapCheck == 1) {
                TreeNode *temp;
                temp = node;
                node = siblingNode;
                siblingNode = temp;
            }

            if (node->isLeaf()) {
                noPairsInNode = node->getNoPairs();
                noPairsInSibling = siblingNode->getNoPairs();

                moveBytes(node->getBuffPage() + 2 * SIZE_INT,
                        siblingNode->getBuffPage() +
                        2 * SIZE_INT + (noPairsInSibling) * PAIR_SIZE,
                        noPairsInNode * PAIR_SIZE);

                moveBytes(node->getBuffPage() + BLOCK_SIZE - PTR_SIZE,
                        siblingNode->getBuffPage() + BLOCK_SIZE - PTR_SIZE,
                        PTR_SIZE);

                siblingNode->setNoPairs(noPairsInSibling +
                        noPairsInNode);
            } else {
                noPairsInNode = node->getNoPairs();
                noPairsInSibling = siblingNode->getNoPairs();

                writeInt(siblingNode->getBuffPage(),
                        2 * SIZE_INT + noPairsInSibling *
                        PAIR_SIZE + PTR_SIZE,
                        kDash);

                moveBytes(node->getBuffPage() + 2 * SIZE_INT,
                        siblingNode->getBuffPage() + 2 * SIZE_INT +
                        (noPairsInSibling + 1) * PAIR_SIZE,
                        noPairsInNode * PAIR_SIZE + PTR_SIZE);

                siblingNode->setNoPairs(noPairsInSibling +
                        noPairsInNode + 1);
            }

            if (swapCheck == 1) {

                bm->unpinPage(trace[level]->getBuffPage(), true);
                bm->unpinPage(siblingNodeId, false);
                StorageManager::freeBlock(dbName, siblingNodeId);
                delete trace[level];
            } else {

                bm->unpinPage(trace[level]->getBuffPage(), false);
                bm->unpinPage(siblingNodeId, true);
                StorageManager::freeBlock(dbName, node_diskBlockId);
                delete trace[level];
            }

            cout << "kDash: " << kDash << endl;
            recursiveDeleteEntry(parentNode, kDash,
                    nodeToBeDeleted, trace, level - 1);
        }            //redistribution 
        else {
            cout << "redistribution ";
            //if N' is predecessor of N
            if (swapCheck != 1) {

                if (node->isLeaf()) {

                    cout << " leaf,middle node" << endl;
                    int lastKeyInSibling = readInt(
                            siblingNode->getBuffPage(),
                            2 * SIZE_INT + noPairsInSibling *
                            PAIR_SIZE - PTR_SIZE);

                    int lastRidInSibling = readInt(
                            siblingNode->getBuffPage(),
                            2 * SIZE_INT + (noPairsInSibling - 1) *
                            PAIR_SIZE);

                    moveBytes(node->getBuffPage() + 2 * SIZE_INT,
                            node->getBuffPage() +
                            2 * SIZE_INT + PAIR_SIZE,
                            noPairsInNode * PAIR_SIZE);

                    siblingNode->setNoPairs(noPairsInSibling - 1);
                    noPairsInSibling = siblingNode->getNoPairs();

                    writeInt(node->getBuffPage(), 2 * SIZE_INT,
                            lastRidInSibling);

                    writeInt(node->getBuffPage(),
                            2 * SIZE_INT + PTR_SIZE, lastKeyInSibling);

                    node->setNoPairs(noPairsInNode + 1);

                    //node is not the first child of parent node 			
                    writeInt(parentNode->getBuffPage(),
                            2 * SIZE_INT + (keyCount - 1) * PAIR_SIZE +
                            PTR_SIZE, lastKeyInSibling);


                } else {
                    cout << "      non leaf,middle node" << endl;
                    int lastKeyInSibling = readInt(
                            siblingNode->getBuffPage(), 2 * SIZE_INT +
                            noPairsInSibling * PAIR_SIZE - PTR_SIZE);

                    int lastPtrInSibling = readInt(
                            siblingNode->getBuffPage(), 2 * SIZE_INT +
                            noPairsInSibling * PAIR_SIZE);

                    moveBytes(node->getBuffPage() + 2 * SIZE_INT,
                            node->getBuffPage() + 2 * SIZE_INT +
                            PAIR_SIZE, noPairsInNode *
                            PAIR_SIZE + PTR_SIZE);

                    siblingNode->setNoPairs(noPairsInSibling - 1);
                    noPairsInSibling = siblingNode->getNoPairs();

                    writeInt(node->getBuffPage(), 2 * SIZE_INT,
                            lastPtrInSibling);

                    writeInt(node->getBuffPage(), 2 * SIZE_INT +
                            PTR_SIZE, kDash);

                    node->setNoPairs(noPairsInNode + 1);

                    writeInt(parentNode->getBuffPage(), 2 * SIZE_INT +
                            (keyCount - 1) * PAIR_SIZE + PTR_SIZE,
                            lastKeyInSibling);

                }
            } else {
                if (node->isLeaf()) {
                    cout << "    leaf,leftmost node" << endl;
                    int firstRidInSibling = readInt(
                            siblingNode->getBuffPage(), 2 * SIZE_INT);

                    int firstKeyInSibling = readInt(
                            siblingNode->getBuffPage(),
                            2 * SIZE_INT + PTR_SIZE);

                    moveBytes(siblingNode->getBuffPage() +
                            2 * SIZE_INT + PAIR_SIZE,
                            siblingNode->getBuffPage() + 2 * SIZE_INT,
                            (noPairsInSibling - 1) * PAIR_SIZE);

                    siblingNode->setNoPairs(noPairsInSibling - 1);
                    noPairsInSibling = siblingNode->getNoPairs();

                    writeInt(node->getBuffPage(), 2 * SIZE_INT +
                            noPairsInNode*PAIR_SIZE, firstRidInSibling);

                    writeInt(node->getBuffPage(), 2 * SIZE_INT +
                            noPairsInNode * PAIR_SIZE + PTR_SIZE,
                            firstKeyInSibling);

                    node->setNoPairs(noPairsInNode + 1);


                    //node is first child of parent node 					
                    firstKeyInSibling = readInt(
                            siblingNode->getBuffPage(),
                            2 * SIZE_INT + PTR_SIZE);

                    writeInt(parentNode->getBuffPage(),
                            2 * SIZE_INT + PTR_SIZE,
                            firstKeyInSibling);

                } else {
                    cout << "      non leaf,leftmost node" << endl;
                    int firstPtrInSibling = readInt(
                            siblingNode->getBuffPage(), 2 * SIZE_INT);

                    int firstKeyInSibling = readInt(
                            siblingNode->getBuffPage(),
                            2 * SIZE_INT + PTR_SIZE);

                    moveBytes(siblingNode->getBuffPage() +
                            2 * SIZE_INT + PAIR_SIZE,
                            siblingNode->getBuffPage() + 2 * SIZE_INT,
                            (noPairsInSibling - 1) * PAIR_SIZE +
                            PTR_SIZE);


                    siblingNode->setNoPairs(noPairsInSibling - 1);
                    noPairsInSibling = siblingNode->getNoPairs();

                    writeInt(node->getBuffPage(), 2 * SIZE_INT +
                            noPairsInNode * PAIR_SIZE +
                            PTR_SIZE, kDash);

                    writeInt(node->getBuffPage(), 2 * SIZE_INT +
                            (noPairsInNode + 1) * PAIR_SIZE,
                            firstPtrInSibling);

                    node->setNoPairs(noPairsInNode + 1);

                    writeInt(parentNode->getBuffPage(),
                            2 * SIZE_INT + PTR_SIZE,
                            firstKeyInSibling);

                }
            }

            bm->unpinPage(trace[level]->getBuffPage(), true);
            bm->unpinPage(siblingNodeId, true);
            delete trace[level];
            for (int i = level - 1; i >= 1; i--) {
                bm->unpinPage(trace[i]->getBuffPage(), false);
                delete trace[i];
            }

        }
    }
    cout << key << "key deleted at level " << level << endl << endl;
}

int BPlusTree::lookUpWithoutUnpin(int key, TreeNode *leafNode) {
    BufferManager *bm = BufferManager::getInstance(dbName);
    int nofPairs = leafNode->getNoPairs();
    int ithKey = readInt(leafNode->getBuffPage(),
            2 * SIZE_INT + PTR_SIZE);

    int keyCount;
    int rid;
    if (nofPairs == 0) {
        return -1;
    }

    for (keyCount = 0; keyCount < nofPairs && key > ithKey; keyCount++) {

        //cout<<"in lookup for insert ithkey "<<ithKey<<endl;
        ithKey = readInt(leafNode->getBuffPage(),
                2 * SIZE_INT + (keyCount + 1) * PAIR_SIZE + PTR_SIZE);
    }
    rid = readInt(leafNode->getBuffPage(),
            2 * SIZE_INT + keyCount * PAIR_SIZE);

    cout << "key " << key << " ithKey " << ithKey << endl;
    if (key == ithKey && keyCount < nofPairs)
        return rid;
    else
        return -1;

}

/**
 * Index lookup for a key. Returns the rid if found, else -1.
 * Assuming the index on a candidate key.
 */
int BPlusTree::lookUp(int key) {
    BufferManager *bm = BufferManager::getInstance(dbName);

    int height = 0;
    //TreeNode **trace = (TreeNode **)malloc(10);
    TreeNode **trace = new TreeNode *[10];
    for (int i = 0; i < 10; i++)
        trace[i] = NULL;

    TreeNode *leafNode = getLeaf(key, trace, &height);
    int nofPairs = leafNode->getNoPairs();
    int ithKey = readInt(leafNode->getBuffPage(),
            2 * SIZE_INT + PTR_SIZE);
    int keyCount;
    int rid;
    cout << nofPairs << endl;
    for (keyCount = 0; keyCount < nofPairs && key > ithKey; keyCount++) {

        //cout<<"in lookup for insert ithkey "<<ithKey<<endl;
        ithKey = readInt(leafNode->getBuffPage(),
                2 * SIZE_INT + (keyCount + 1) * PAIR_SIZE + PTR_SIZE);
        //cout<<"loop "<<keyCount<<endl;
    }
    rid = readInt(leafNode->getBuffPage(),
            2 * SIZE_INT + (keyCount) * PAIR_SIZE);
    cout << "key " << key << "rid " << rid << endl;
    //unpin the trace 
    for (int level = 1; trace[level] != NULL; level++) {
        bm->unpinPage(trace[level]->getBuffPage(), false);
        delete trace[level];
    }
    free(trace);
    if (nofPairs == 0)
        return -1;


    if (key == ithKey && keyCount < nofPairs)
        return rid;
    else
        return -1;
}

TreeNode* BPlusTree::getLeaf(int key, TreeNode *trace[], int *height) {

    BufferManager *bm = BufferManager::getInstance(dbName);
    return getToLeaf(root, key, trace, height, 0);
}

TreeNode* BPlusTree::getToLeaf(TreeNode *curNode,
        int key,
        TreeNode *trace[],
        int *height,
        int curLevel) {

    BufferManager *bm = BufferManager::getInstance(dbName);
    trace[curLevel] = curNode;
    *height = curLevel;
    cout << "Curlevel: " << curLevel << "leaf flag: " << curNode->isLeaf() << endl;
    if (curNode->isLeaf()) {
        return curNode;
    }
    return getToLeaf(getChild(curNode, key), key, trace,
            height, curLevel + 1);
}

TreeNode* BPlusTree::getChild(TreeNode *curNode, int key) {

    BufferManager *bm = BufferManager::getInstance(dbName);
    return new TreeNode(bm->pinPage(
            curNode->getChildPointer(key)));
}

/**
 * Destructor. Just force all the dirty buffer pages to disk.
 */
BPlusTree::~BPlusTree() {
    BufferManager *bm = BufferManager::getInstance(dbName);
    bm->~BufferManager();
}

