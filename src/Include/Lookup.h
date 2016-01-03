// hash table implementation

#ifndef __LOOKUP_H__
#define __LOOKUP_H__

#include <assert.h>
#include <stdlib.h>
#include <iostream>

using namespace std;

#include "RefCount.h"

enum Error_t { E_OK, E_ERR };

template<class Key_t>
struct LookupEntry_t {
    Key_t key; /* the key for the entry */
    int deleteFlag; /* is this entry deleted ? 0=NO; 1=YES */
    int stateCount; /* No. of states pointing to the entry */
    LookupEntry_t<Key_t> *nextEntry;
            /* pointer to next entry in linked list */

    LookupEntry_t(Key_t &newKey)
        : key(newKey), deleteFlag(0), stateCount(0), nextEntry(NULL)
    { }
};

template<int TABLESIZE, class Key_t>
class Lookup_t : public RefC_t {
    LookupEntry_t<Key_t> *table[TABLESIZE]; /* the hashtable */

    Error_t BucketDelete( int bucketNo );

public:
    Lookup_t();
    ~Lookup_t();
    Error_t Insert( Key_t &key, Key_t **keyInserted = NULL );
    Error_t Remove( void **state );
    Error_t Search( Key_t &key, void **state,
            int *found = NULL, Key_t **k = NULL );
    Error_t IterNext( Key_t *key, void **state );
    Error_t IterStateInit( void **state );
    Error_t IterStateDelete( void **state );
};

/*
--------------------------------------------------------------------------------
*    Function:    Lookup_t::Lookup_t
*    Sets the tablesize and allocates the hashtable of this size
*    Sets the keySize, ie. the size of the key to be used with this table.
--------------------------------------------------------------------------------
*/
template<int TABLESIZE, class Key_t>
inline Lookup_t<TABLESIZE, Key_t>::Lookup_t()
{
    int bucketNo;
    
#ifdef __LOOKUP_DEBUG__
    cout << "Lookup_t::Lookup_t..." << endl;
#endif
    for( bucketNo = 0 ; bucketNo < TABLESIZE ; bucketNo++ ) {
        table[bucketNo] = NULL;
    }
}

/*
--------------------------------------------------------------------------------
*    Function:    Lookup_t::~Lookup_t
*    Called to release the hash-table resources after it is out of use.
*    Releases the resources (including that held by its buckets) and 
*    reinitializes the table entry to reflect that it is free.
--------------------------------------------------------------------------------
*/

template<int TABLESIZE, class Key_t>
Lookup_t<TABLESIZE, Key_t>::~Lookup_t()
{
    int bucketNo;

#ifdef __LOOKUP_DEBUG__
    cout <<  "Lookup_t::~Lookup_t..." << endl;
#endif
    for( bucketNo = 0 ; bucketNo < TABLESIZE ; bucketNo++ ) {
        BucketDelete(bucketNo);
    }
}

/*
--------------------------------------------------------------------------------
*    Function:    Lookup_t::Insert
*    Allocates, initializes and inserts a new bucket (a bucket contains only
*    one entry) and inserts it at the *head* of the table bucket
*    determined by the table's hash function.
*    Returns:    E_OK on success
*            E_ERR on failure
--------------------------------------------------------------------------------
*/
template<int TABLESIZE, class Key_t>
Error_t
Lookup_t<TABLESIZE, Key_t>::Insert(
    Key_t &key,
    Key_t **keyInserted )
{
    int bucketNo;
    LookupEntry_t<Key_t> *entry;

#ifdef __LOOKUP_DEBUG__
    cout << "Lookup_t::Insert..." << endl;
#endif
    bucketNo = key.HashValue()%TABLESIZE;
    if( bucketNo < 0 )
        return E_ERR;

    entry = new LookupEntry_t<Key_t>(key);
    entry->nextEntry = table[bucketNo];
    table[bucketNo] = entry;

    if( keyInserted )
        *keyInserted = &(entry->key);

    return E_OK;
}

/*
--------------------------------------------------------------------------------
*    Function:    Lookup_t::Remove
*    *Logically* deletes the entry last searched in the specified table
*    using the specified state. Physical deletion is done later when some 
*    search traversal realises that this entry is deleted, and no state 
*    holds a pointer to this entry.
*    Returns:    E_OK on success
*            E_ERR on failure
--------------------------------------------------------------------------------
*/
template<int TABLESIZE, class Key_t>
Error_t
Lookup_t<TABLESIZE, Key_t>::Remove(
    void **state )
{
    LookupEntry_t<Key_t> *entry;
    int bucketNo;

#ifdef __LOOKUP_DEBUG__
    cout << "Lookup_t::Remove..." << endl;
#endif
    if( state == NULL )
        return E_ERR;
    if( *state == NULL )
        return E_OK;
    entry = (LookupEntry_t<Key_t> *)(*state);
    bucketNo = entry->key.HashValue()%TABLESIZE;
    if( bucketNo < 0 )
        return E_ERR;

    entry->deleteFlag = 1;
    return E_OK;
}

/*
--------------------------------------------------------------------------------
*    Function:    Lookup_t::Search
*    Iterator - puts the pointer to the next entry in hashtable 
*    that has the specified key.
*    The search is initiated by calling the function with *state == NULL.
*    This searches for the first entry with the specified key, and updates
*    the state.
*    Further calls with the same state search for the next entries, and
*    update the state accordingly.
*    When no more entries exist with the specified key, *state becomes NULL
*    and the function returns E_OK.
*    When a state points to an entry, the stateCount of that entry is
*    incremented. This is decremented when this state is changed to point 
*    to a new entry.
*    Actually, transparent to the user, something more is done.
*    During the traversal, if the search comes across an entry that has been
*    logically deleted and has no state pointing to it, then the entry gets
*    *physically* deleted.
*    Returns:    E_OK on success 
*                        *state points to the new state
*                        *found == 1 if search succeeded; == 0 otherwise
*            E_ERR on failure.
--------------------------------------------------------------------------------
*/
template<int TABLESIZE, class Key_t>
Error_t
Lookup_t<TABLESIZE, Key_t>::Search(
    Key_t &key,
    void **state,
    int *found,
    Key_t **k )
{
    int bucketNo;
    LookupEntry_t<Key_t> *entry,*prevEntry;
    LookupEntry_t<Key_t> **prevPtr = (LookupEntry_t<Key_t> **) state; 

#ifdef __LOOKUP_DEBUG__
    cout << "Lookup_t::Search..." << endl;
#endif
    bucketNo = key.HashValue()%TABLESIZE;
    if( bucketNo < 0 )
            return E_ERR;
    assert( bucketNo >= 0 && bucketNo < TABLESIZE );
    /*
    *    prevEntry points to the entry prev to <entry>; NULL if <entry>
    *    is the first in the bucket.
    *    The previous state points to the previous found entry. We need
    *    to find the next. If the previous found entry == NULL, we are 
    *    searching for the first eligible entry.
    *    In view of the above, prevEntry and entry are initialized
    *    accordingly.
    */
    if( prevPtr == NULL )
        prevEntry = NULL;
    else     prevEntry = *prevPtr;
    if( prevEntry == NULL )
        entry = table[bucketNo];
    else {
        prevEntry->stateCount--;
        entry = prevEntry->nextEntry;
    }
    /*
    *    Loop till an eligible entry is found, or the list terminates
    */
    while( entry != NULL ){
        /*
        *    Delete the logically deleted useless entries on
        *    the way. prevEntry and entry are adjusted
        *    appropriately
        */
        if( entry->deleteFlag == 1 ) {
            if( entry->stateCount == 0 ){
                if( prevEntry == NULL )
                    table[bucketNo] = entry->nextEntry;
                else     prevEntry->nextEntry = entry->nextEntry;
                delete entry;
                if( prevEntry == NULL )
                    entry = table[bucketNo];
                else    entry = prevEntry->nextEntry;
                continue; 
            }
        }
        /*
        *    entry->deleteFlag == 0 ...
        *    Check for an eligible entry amidst the undeleted
        */
        else    if( key == entry->key ) break;

        prevEntry = entry;
        entry = entry->nextEntry;
    }
    /*
    *    Found an eligible entry ?
    */
    if( entry == NULL ){
        if( prevPtr != NULL )
            *prevPtr = NULL;
        if( found != NULL )
            *found = 0;
        return E_OK;
    }
    if( prevPtr != NULL ){
        *prevPtr = entry;    
        entry->stateCount++;
    }
    if( found != NULL )
        *found = 1;
    if( k != NULL )
        *k = &(entry->key);
    return E_OK;
}

/*
--------------------------------------------------------------------------------
*    Function:    Lookup_t::IterNext
*    Iterator - gets the key in the next entry in the hashtable.
*    If key is null, it is ignored.
*    The search is initiated by calling the function with *state == NULL.
*    This gets the first entry, and updates the state.
*    Further calls with the same state search for the next entries, and
*    update the state accordingly.
*    When no more entries exist, *state becomes NULL
*    and the function returns E_OK.
*    When a state points to an entry, the stateCount of that entry is
*    incremented. This is decremented when this state is changed to point 
*    to a new entry.
*    Actually, transparent to the user, something more is done.
*    During the traversal, if the search comes across an entry that has been
*    logically deleted and has no state pointing to it, then the entry gets
*    *physically* deleted.
*    Returns:    E_OK on success -     *key points to the key
*                        *state points to the new state
*            E_ERR on failure.
--------------------------------------------------------------------------------
*/
template<int TABLESIZE, class Key_t>
Error_t
Lookup_t<TABLESIZE, Key_t>::IterNext(
    Key_t *key,
    void **state )
{
    int bucketNo;
    LookupEntry_t<Key_t> *entry,*prevEntry;
    LookupEntry_t<Key_t> **prevPtr = (LookupEntry_t<Key_t> **) state; 

#ifdef __LOOKUP_DEBUG__
    cout << "Lookup_t::IterNext..." << endl;
#endif

    /*
    *    prevEntry points to the entry prev to <entry>; NULL if <entry>
    *    is the first in the bucket.
    *    The previous state points to the previous found entry. We need
    *    to find the next. If the previous found entry == NULL, we are 
    *    searching for the first entry.
    *    In view of the above, prevEntry and entry are initialized
    *    accordingly.
    */
    if( prevPtr == NULL )
        prevEntry = NULL;
    else     prevEntry = *prevPtr;
    if( prevEntry == NULL ) {
        for( bucketNo = 0 ; bucketNo < TABLESIZE ; bucketNo++ ) {
            if( table[bucketNo] != NULL )
                break;
        }
        if( bucketNo == TABLESIZE ) {
            /*
            *    nothing in table
            */
            if( prevPtr != NULL )
                *prevPtr = NULL;
            return E_OK;
        }
        /*
        *    first non-empty bucket
        */
        entry = table[bucketNo];
    }
    else {
        bucketNo = prevEntry->key.HashValue()%TABLESIZE;
        if( bucketNo < 0 )
            return E_ERR;

        prevEntry->stateCount--;
        entry = prevEntry->nextEntry;
    }

    /*
    *    Repeat till an undeleted entry is encountered
    *    or the table ends
    */
    for( ; ; ) {
        /*
        *    Delete the logically deleted useless entries on
        *    the way. prevEntry and entry are adjusted
        *    appropriately
        */
        if( entry != NULL ) {
            if( entry->deleteFlag == 1 ) {
                if( entry->stateCount == 0 ){
                    if( prevEntry == NULL )
                        table[bucketNo] =
                            entry->nextEntry;
                    else     prevEntry->nextEntry =
                            entry->nextEntry;
                    delete entry;
                    if( prevEntry == NULL )
                        entry = table[bucketNo];
                    else    entry = prevEntry->nextEntry;
                }
                else {
                    /*
                    * next entry
                    */
                    prevEntry = entry;
                    entry = prevEntry->nextEntry;
                }
            }
            /*
            *    entry->deleteFlag == 0 ...
            */
            else    break;
        }
        else {
            /*
            *    next bucket
            */
            prevEntry = NULL;
            bucketNo++;
            while( bucketNo < TABLESIZE ) {
                if( table[bucketNo] != NULL )
                    break;
                bucketNo++;
            }
            if( bucketNo == TABLESIZE ) {
                /*
                *    nothing more in table
                */
                if( prevPtr != NULL )
                    *prevPtr = NULL;
                return E_OK;
            }
            /*
            *    next non-empty bucket
            */
            entry = table[bucketNo];
        }
    }

    if( prevPtr != NULL ){
        *prevPtr = entry;    
        entry->stateCount++;
    }

    if( key != NULL )
        *key = entry->key;

    return E_OK;
}

/*
--------------------------------------------------------------------------------
*    Function:    Lookup_t::IterStateInit
*    Initializes *state to NULL.
*    Returns:    E_OK    on success
*            E_ERR on failure
--------------------------------------------------------------------------------
*/
template<int TABLESIZE, class Key_t>
Error_t
Lookup_t<TABLESIZE, Key_t>::IterStateInit( 
    void **state )
{
#ifdef __LOOKUP_DEBUG__
    cout << "Lookup_t::IterStateInit..." << endl;
#endif
    if( state == NULL )
        return E_ERR;
    *state = NULL;
    return E_OK;
}

/*
--------------------------------------------------------------------------------
*    Function:    Lookup_t::IterStateDelete
*    Decrements the stateCount of the entry held by the state, and 
*    sets *state to NULL.
*    Returns:    E_OK    on success
*            E_ERR on failure
--------------------------------------------------------------------------------
*/
template<int TABLESIZE, class Key_t>
Error_t
Lookup_t<TABLESIZE, Key_t>::IterStateDelete(
    void **state )
{
    LookupEntry_t<Key_t> **entry = (LookupEntry_t<Key_t> **)state;
    int bucketNo;

#ifdef __LOOKUP_DEBUG__
    cout << "IterStateDelete..." << endl;
#endif
    if( entry == NULL )
        return E_ERR;
    if( *entry == NULL )
        return E_OK;

    bucketNo = (*entry)->key.HashValue()%TABLESIZE;
    if( bucketNo < 0 )
        return E_ERR;

    (*entry)->stateCount--;

    *entry = NULL;
    return E_OK;
}

/*
--------------------------------------------------------------------------------
*    Function:    Lookup_t::BucketDelete
*    Deletes all bucket in the bucket bucketNo in the table.
*    Called from Lookup_t::~Lookup_t.
*    Returns:    E_OK    on success
*            E_ERR on failure
--------------------------------------------------------------------------------
*/
template<int TABLESIZE, class Key_t>
Error_t
Lookup_t<TABLESIZE, Key_t>::BucketDelete(
    int bucketNo )
{
    LookupEntry_t<Key_t> *p,*next;

#ifdef __LOOKUP_DEBUG__
    cout << "BucketDelete..." << endl;
#endif
    if( bucketNo < 0 || bucketNo >= TABLESIZE )
        return E_ERR;
    p = table[bucketNo];
    while( p != NULL ){
        next = p->nextEntry;
        delete p;
        p = next;
    }
    table[bucketNo] = NULL;
    return E_OK;
}

#endif // __LOOKUP_H__
