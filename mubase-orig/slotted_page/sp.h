/** ----------------------------------------------------
 * CS3010: Introduction to Database Systems
 * Department of Computer Science and Engineering
 * IIT Hyderabad
 *
 * File: sp.h
 * Header files for the class SlottedPage
 * Implementation to be provided in spimpl.cpp
 * -----------------------------------------------------
 */

#define BLOCKSIZE 128
#define MAX_INPUT_REC_SIZE 120

class SlottedPage {
    private:
    char *block;  // The underlying buffer block, to which this 
                  // class is a wrapper.
    public:

    // Constructor.
    SlottedPage(char *buffPage) {
        this->block = buffPage;
    }

    /**
     * Initialize (format) the block.
     */
    void initBlock();

    /**
     * Retrieve a record at the given slot number. Returns -1
     * if the slot number is invalid.
     */
    char *getRecord(short slotNum);

    /**
     * Stores a record if there is enough space.  Returns the 
     * slot number for the new record, and -1 if not enough space.
     */
     short storeRecord(char *rec);

    /**
     * Delete a record given its slot number and returns the
     * space available.
     */
    short deleteRecord(short slotNum);

    /**
     * Display the contents of the block
     */ 
    void display();

};
