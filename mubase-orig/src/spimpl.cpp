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
#include "sputil.c"

#define SLOTSIZE  (short)(sizeof(short)*2)
#define EOFOFFSET (short)(sizeof(short)*1)
#define SOFOFFSET (short)(sizeof(short)*2)
#define FFSOFFSET (short)(sizeof(short)*3)
#define NBIOFFSET (short)(sizeof(short)*4)
#define SLOTOFFSET (short)(sizeof(short)*4+sizeof(int))


using namespace std;

// Initializes (formats) the slotted page

void SlottedPage::initBlock() {
    writeShort(block, (short) 0, (short) 0); //No of slot entries
    writeShort(block, (short) EOFOFFSET, (short) BLOCKSIZE); //End of free space
    writeShort(block, (short) SOFOFFSET, (short) SLOTOFFSET); //Start of free space
    writeShort(block, (short) FFSOFFSET, (short) SLOTOFFSET); //First free slot
    writeShort(block, (short) NBIOFFSET, (int) - 1); //Next block id
}

//Returns total no of slot entries in the page

short SlottedPage::getTotalNoOfSlots() {
    short totalSlots = readShort(block, 0);
    return totalSlots;
}

//Returns id of next block, returns -1 if there is no next block

short SlottedPage::getNextBlockId() {
    short nextBlockId = readShort(block, (short) NBIOFFSET);
    return nextBlockId;
}

//Write next Block-id

void SlottedPage::writeNextBlockId(short nextBlockId) {
    writeShort(block, (short) NBIOFFSET, (short) nextBlockId);
    return;
}

// Shows the contents of the page

void SlottedPage::display() {
    short n = readShort(block, (short) 0);
    short eofs = readShort(block, (short) EOFOFFSET);
    short firstFreeSlot = readShort(block, (short) FFSOFFSET);
    short availableSpace = eofs - (n) * SLOTSIZE - SLOTOFFSET;

    cout << "Number of slot entries = " << n << endl
            << "Available free space = " << availableSpace << endl
            << "First Free slot at " << firstFreeSlot << endl;

    short recOffset, recLength;
    for (short i = 0; i < n; i++) {
        recOffset = readShort(block, SLOTOFFSET + i * SLOTSIZE);
        recLength = readShort(block, SLOTOFFSET + sizeof (short) +i * SLOTSIZE);
        cout << "\nRecord at slot " << i << " (Offset = " << recOffset << "  Length = " << recLength << ") : ";
        char *rec = getRecord(i);
        if (rec == (char*) - 1)
            cout << "None";
        else
            cout << rec;
    }
    cout << endl;
}

char *SlottedPage::getRecord(short slotNum) {
    short startOfFreeSpace = readShort(block, (short) SOFOFFSET);
    short lastValidSlotNum = (short) ((((startOfFreeSpace - SLOTOFFSET) / (SLOTSIZE)) - 1));

    //Check for the validity of slot number
    if (slotNum < 0 || slotNum > lastValidSlotNum)
        return (char*) - 1;

    short slotPosition = SLOTOFFSET + slotNum*SLOTSIZE; //Slot numbers start from 0
    short recOffset = readShort(block, (short) slotPosition);
    short recLength = readShort(block, (short) (slotPosition + (short) sizeof (short)));

    if (recLength <= 0)
        return (char*) - 1;

    char* rec = readBytes(block, recOffset, recLength);
    return rec;
}

short SlottedPage::storeRecord(char *record) {
    cout << "\nIN storeRecord\n";
    short slotEntries = readShort(block, (short) 0);
    short endOfFreeSpace = readShort(block, (short) EOFOFFSET);
    short startOfFreeSpace = readShort(block, (short) SOFOFFSET);
    short firstFreeSlot = readShort(block, (short) FFSOFFSET);

    short newFirstFreeSlot = readShort(block, firstFreeSlot);
    cout << "\nRecord Size = " << strlen(record) << endl;
    cout << "\nFree space = " << endOfFreeSpace - startOfFreeSpace - 4 << endl;

    //check for space
    if (startOfFreeSpace >= endOfFreeSpace) return -1;

    short recLength = strlen(record);
    short recSlot = firstFreeSlot;
    bool createNewSlot = false; //True if new slot gets created
    if (firstFreeSlot == startOfFreeSpace) createNewSlot = true;

    //Check if record length is larger than free space
    if (createNewSlot && (endOfFreeSpace - startOfFreeSpace - (short) SLOTSIZE < recLength)) return (short) - 1;
    if (createNewSlot && (endOfFreeSpace - startOfFreeSpace < recLength)) return (short) - 1;

    //Write record data
    short newEndOfFreeSpace = endOfFreeSpace - recLength;
    writeShort(block, recSlot, newEndOfFreeSpace);
    writeShort(block, recSlot + sizeof (short), recLength);
    writeBytes(block, (int) newEndOfFreeSpace, record);

    //Update No of Slot Entries
    slotEntries++;
    if (createNewSlot)writeShort(block, 0, slotEntries);

    //Update end of free space
    writeShort(block, EOFOFFSET, newEndOfFreeSpace);

    //Update start of free space
    if (createNewSlot) writeShort(block, SOFOFFSET, (short) (startOfFreeSpace + SLOTSIZE));

    //Update first free slot
    if (createNewSlot) newFirstFreeSlot = firstFreeSlot + SLOTSIZE;
    writeShort(block, FFSOFFSET, newFirstFreeSlot);

    short recSlotNum = (short) ((recSlot - SLOTOFFSET) / SLOTSIZE);
    return recSlotNum;
}

short SlottedPage::deleteRecord(short slotNum) {
    short slotEntries = readShort(block, (short) 0);
    short endOfFreeSpace = readShort(block, (short) EOFOFFSET);
    short startOfFreeSpace = readShort(block, (short) SOFOFFSET);
    short firstFreeSlot = readShort(block, (short) FFSOFFSET);
    short lastValidSlotNum = ((startOfFreeSpace - SLOTOFFSET) / SLOTSIZE) - 1;

    //Check for the validity of slot number
    if (slotNum < 0 || slotNum > lastValidSlotNum)
        return -1;

    short recSlot = (short) SLOTOFFSET + (short) slotNum*SLOTSIZE;
    short recOffset = readShort(block, (short) recSlot);
    short recLength = readShort(block, (short) (recSlot + (short) (sizeof (short))));

    if (recLength == (short) - 1)return -1;

    //Move Bytes
    moveBytes(block, endOfFreeSpace + recLength, endOfFreeSpace, recOffset - endOfFreeSpace);


    //Update Offsets of all the records whose data got moved
    for (int i = 0; i < slotEntries; i++) {
        short tempRecOffset = readShort(block, SLOTOFFSET + i * SLOTSIZE);
        short tempRecLength = readShort(block, SLOTOFFSET + (short) (sizeof (short)) + i * SLOTSIZE);
        if (tempRecLength > 0 && tempRecOffset < recOffset + recLength) {
            writeShort(block, SLOTOFFSET + i*SLOTSIZE, tempRecOffset + recLength);
        }
    }

    //Update end of free space
    writeShort(block, EOFOFFSET, endOfFreeSpace + recLength);

    //Update first free slot
    writeShort(block, recSlot, firstFreeSlot);
    writeShort(block, recSlot + (short) (sizeof (short)), (short) - 1);
    writeShort(block, FFSOFFSET, recSlot);

    //return space available
    return endOfFreeSpace + recLength - startOfFreeSpace;
}



//  Swaps records i.e. their slot numbers

void SlottedPage::swapSlots(short slot1, short slot2) {
    short slotOffset1 = SLOTOFFSET + (short) slot1*SLOTSIZE;
    short slotOffset2 = SLOTOFFSET + (short) slot2*SLOTSIZE;

    short recOffset1 = readShort(block, slotOffset1);
    short recLength1 = readShort(block, slotOffset1 + (short) sizeof (short));
    short recOffset2 = readShort(block, slotOffset2);
    short recLength2 = readShort(block, slotOffset2 + (short) sizeof (short));

    writeShort(block, slotOffset1, recOffset2);
    writeShort(block, slotOffset1 + sizeof (short), recLength2);
    writeShort(block, slotOffset2, recOffset1);
    writeShort(block, slotOffset2 + sizeof (short), recLength1);

    return;
}





