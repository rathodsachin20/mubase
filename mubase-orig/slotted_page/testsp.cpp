/** ----------------------------------------------------
 * CS3010: Introduction to Database Systems
 * Department of Computer Science and Engineering
 * IIT Hyderabad
 *
 * File: testsp.cpp
 * Test driver with main for the SlottedPage module.
 * Students SHOULD NOT change anything in this file. You can
 * make use of this to test your slotted page implementation.
 * -----------------------------------------------------
 */

#include<iostream>
#include<cstring>
#include<cstdlib>
#include<cstdio>
#include "sp.h"

using namespace std;

#define INSERT  1
#define GET     2
#define DELETE  3
#define DISPLAY 4
#define QUIT    5

int main() {
    char *block = (char *) malloc((size_t) BLOCKSIZE);
    SlottedPage *sp = new SlottedPage(block);
    sp->initBlock();

    cout << "A fresh block just after initialization" << endl;
    sp->display();
   
    int opcode;
    short slotNum;
    char *rec = (char *) malloc(MAX_INPUT_REC_SIZE);
 
    cout << "1: Insert, 2: Get, 3: Delete, 4: Display, 5: Quit \n";

    while (1) {
        scanf("%d", &opcode);
        switch (opcode) {
            case INSERT:
                scanf("%s", rec);
                sp->storeRecord(rec);
                break;
            case GET:
                scanf("%hu", &slotNum);
                cout << sp->getRecord(slotNum) << endl;
                break;
            case DELETE:
                scanf("%hu", &slotNum);
                sp->deleteRecord(slotNum);
                break;
            case DISPLAY:
                sp->display();
                break;
           case QUIT:
                return 0;
            default:
                cout << "1: Insert, 2: Get, 3: Delete, 4: Display, 5: Quit \n";
        }
    }  
    return 0;
}
