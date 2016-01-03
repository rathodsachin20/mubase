#ifndef SPUTIL_H
#define SPUTIL_H

/** ----------------------------------------------------
 * CS3010: Introduction to Database Systems
 * Department of Computer Science and Engineering
 * IIT Hyderabad
 *
 * File: sputil.h
 * Signature of helper functions for the slotted page.
 * Implementation to be provided in sputil.c
 * -----------------------------------------------------
 */

// Returns length of a record.  Assume records are NULL 
// terminated (not true in practice, but a shortcut).
int length(char *record);

// Returns 2 bytes starting at block + offset as an int.
short readShort(char *block, short offset);

// Writes a 2 byte short value at block + offset
void writeShort(char *block, short offset, short value);

// Returns a fragment of the byte array
char *readBytes(char *block, short recOffset, short recLength);

// Writes a fragment in the byte array
void writeBytes(char *block, short offset, char *rec);

void writeBytes(char *block, short offset, char *rec, short l);

// Move a chunk of bytes 
void moveBytes(char *block, short destOffset, short sourceOffest,
        short chunkLength);
// Writes a 4 byte int value at block + offset
void writeInt(char *block, short offset, int value);

// Writes a 4 byte float value at block + offset
void writeFloat(char *block, short offset, float value);

// Returns 4 bytes starting at block + offset as a int.
int readInt(char *block, short offset);

// Returns 4 bytes starting at block + offset as a float.
float readFloat(char *block, short offset);

#endif
