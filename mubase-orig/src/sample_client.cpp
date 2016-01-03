#include <iostream>

#include "bt.h"

int main() {
    // One-time operations.
    DiskManager::createDB("testdb", 1000);
    StorageManager::initDB("testdb");

    BPlusTree *bpTree = new BPlusTree("testdb", "index1", true);

    bpTree->insertEntry(1, 100);
    bpTree->insertEntry(2, 200);

    // More operations - can be menu driven.
}

