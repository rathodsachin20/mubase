Relation Manager and External Sort Implementation in MuBase
===========================================================

Our implementation supports following features:

Short summary: 
==============
variable length records, multiple db and relations,
relations extending to multiple blocks, openscan on relation,
sorting of relation with one block, limited predicated scan


In Detail:
==========

1. Slotted page: 
    Structure:
        First 2 bytes  : No of slot entries
        Next 2 bytes : End of free space   
        Next 2 bytes : Start of free space    
        Next 2 bytes : First free slot
        Next 4 bytes : Next block id
    Slots are stored from left and record bytes are store from right.
    
    Note: In storerecord() function, along with recbytes, length of 
    recbytes is also passed as a parameter. This is done to ensure that
    recbytes containing '\0' can also be stored and read.
    
2. Class Schema : 
    Additionally stores total number of varchars, size
    of each attribute, offset of fixed length attributes along with 
    their type and name.
    
3. Class Record :
    Stuture of recbytes: 
    Positon of varchars|Null Bytes|Fixed length attrs|variable length attrs
    
4. Class Relation :
    a. Supports read and write of schema and records
    
    b. lastBlockId is stored to help in storing new record. New record is
       stored in this block. It should be stored in super block to improve
       performance, the next time database starts (Not done in this
       implementation).
    
    c. To get schema of system tables, two static functions are defined
       in relation class, namely: getSysObjSchema() and getSysColSchema()
    
    d. Currently system tables are stored only in one block each
    
    e. Implementation supports multiple databases and relations.
    
    f. Relations can also extend to multiple blocks.
    
    

5. Scan :
    a. Supports openscan()
    
    b. Predicated scan without using index files and
       can be tested on one predicate at a time
       
6. Sort :
    a. Supports sorting of relation in one block using quick sort.

NOTE: Run 'execute.sh' file to run mubase.

=============================================================================


=================================
Submitted By:                   |
Ajaya Agrawal                   |
Chirag Choudhary                |
Sachin Rathod                   |
Sameer Wasnik                   |
=================================







