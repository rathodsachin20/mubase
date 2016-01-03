/** ----------------------------------------------------
 * CS3010: Introduction to Database Systems
 * Department of Computer Science and Engineering
 * IIT Hyderabad
 *
 * File: sputil.c
 * Implementation of helper functions for the slotted page.
 * To be provided by the students.
 * -----------------------------------------------------
 */

#include<iostream>
#include<cstring>
#include<cstdlib>
#include<cstdio>

#include "sputil.h"

using namespace std;

// Returns length of a record.  Assume records are NULL 
// terminated (not true in practice, but a shortcut).

inline int length(char *record) {
    return strlen(record);
}

// Returns 4 bytes starting at block + offset as a int.

int readInt(char *block, short offset) {
    int value;
    int* intPtr = &value;
    memcpy(intPtr, block + offset, sizeof (int));
    return value;
}
// Returns 4 bytes starting at block + offset as a float.

float readFloat(char *block, short offset) {
    float value;
    float* floatPtr = &value;
    memcpy(floatPtr, block + offset, sizeof (float));
    return value;
}

// Returns 2 bytes starting at block + offset as a short.

short readShort(char *block, short offset) {
    short value;
    short* shortPtr = &value;
    memcpy(shortPtr, block + offset, sizeof (short));
    return value;
}

// Writes a 2 byte short value at block + offset

void writeShort(char *block, short offset, short value) {
    short* shortPtr = &value;
    memcpy(block + offset, shortPtr, sizeof (short));
}

// Writes a 4 byte int value at block + offset

void writeInt(char *block, short offset, int value) {
    int* intPtr = &value;
    memcpy(block + offset, intPtr, sizeof (int));
}

// Writes a 4 byte float value at block + offset

void writeFloat(char *block, short offset, float value) {
    float* floatPtr = &value;
    memcpy(block + offset, floatPtr, sizeof (float));
}

// Returns a fragment of the byte array

char *readBytes(char *block, short recOffset, short recLength) {
    char* rec = (char*) malloc((size_t) recLength + 1);

    /*
    short endRecOffset;
    for(short i=0; block[recOffset+i]!='$'; i++)
    {
        rec[i] = block[recOffset+i];
        endRecOffset = i+1;
    }
    rec[endRecOffset]='\0';
     */

    memcpy(rec, block + recOffset, recLength);
    rec[recLength] = '\0';
    return rec;

}

// Writes a fragment in the byte array

void writeBytes(char *block, int offset, char *rec) {
    memcpy(block + offset, rec, strlen(rec));
    //block[offset+strlen(rec)] = '$';
}

void writeBytes(char *block, short offset, char *rec, short l) {
    //cout<<rec<<endl;
    memcpy(block + offset, rec, l);
    //block[offset+strlen(rec)] = '$';
    //cout<<block+offset<<endl;    
}


// Move a chunk of bytes. Use memcpy to implement this.

void moveBytes(char *block, short destOffset, short sourceOffset,
        short chunkLength) {
    // Write your code here
    memmove(block + destOffset, block + sourceOffset, chunkLength);

}

