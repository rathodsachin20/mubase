/** ----------------------------------------------------
 * CS3010: Introduction to Database Systems
 * Department of Computer Science and Engineering
 * IIT Hyderabad
 *
 * File: spimpl.cpp
 * Implementation of the methods provided by class SlottedPage.
 * Implementation to be provided by students. 
 * -----------------------------------------------------
 */

#include<iostream>
#include<cstring>
#include<cstdlib>
#include<cstdio>

#include "sp.h"
#include "sputil.h"

using namespace std;

// Initializes (formats) the slotted page
void SlottedPage::initBlock() {
    writeShort(block, (short)0, (short)0);
    writeShort(block, (short)2, (short)BLOCKSIZE);
}

// Shows the contents of the page
void SlottedPage::display() {
    short n    = readShort(block, (short) 0);
    short eofs = readShort(block, (short) 2);
   
    short availableSpace = eofs - (n + 1)*4;
 
    cout << "Number of slot entries = " << n << endl
         << "Available free space = " << availableSpace << endl;

    for (short i = 0; i < n; i++) {
        cout << "Record at slot " << i << endl;
        char *rec = getRecord(i);
        if (rec == 0) 
            cout << "None"; 
        else
            cout << rec;
    }
    cout << endl;
}


char *SlottedPage::getRecord(short slotNum) {
    return (char *)-1; // Fill your implementation here
}

short SlottedPage::storeRecord(char *record) {
    return -1; // Fill your implementation here
}

short SlottedPage::deleteRecord(short slotNum) {
    return -1; // Fill your implementation here
}

