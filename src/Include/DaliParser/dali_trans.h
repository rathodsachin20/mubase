/* %W%  %G%     %R% */
/*********************************************************************/
/**        Copyright (c) 1993-94 AT&T.   All Rights Reserved.       **/
/**        Dali Storage Manager.                                    **/
/**        AT&T Bell Labs, Lab 1138.                                **/
/**                                                                 **/
/**        THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T.     **/
/**        The copyright notice above does not evidence any         **/
/**        actual or intended publication of such source code.      **/
/**                                                                 **/
/**   THIS SOFTWARE  COMES WITH NO  WARRANTIES.  ANY LIABILITY OF   **/
/**   ANY KIND OF DAMAGES RESULTING FROM THE USE OF THIS SOFTWARE   **/
/**   IS DISCLAIMED.                                                **/
/*********************************************************************/

/**
 *********************************************************************
 *
 *      Functions(s):
 *
 *      Description:
 *              your description goes here.
 *
 *********************************************************************
**/

#ifndef DALI_TRANS_H
#define DALI_TRANS_H

class DaliDB;

class LogRecord;
class Transaction;

typedef int DaliTransTimeStamp;

class DaliOpID {
  public:
    DaliTransTimeStamp ID;
    DaliOpID(){ ID = 0;}
    DaliOpID(DaliTransTimeStamp _TS){ID = _TS;}
    operator DaliTransTimeStamp () const {return ID;}
};
extern const DaliOpID DaliOpID_NULL; 

typedef unsigned long UniqueTid;
typedef int DaliLogOpCode;

enum DaliTransType {DaliTransient = 0, DaliReadOnly = 1, DaliUpdate = 2};

class TransID {
    UniqueTid uniqueid;
    int tt_slot;
  public:
    TransID() { uniqueid = tt_slot = 0; }
    static TransID beginTrans(DaliTransType = DaliUpdate /*, DaliDB *db */);
    int commit();
    int preCommit();
    int postCommit();
    int abort();

    void print(FILE *file);
    INLINE_LOG int operator==(const TransID &tid2) const;
    INLINE_LOG int operator!=(const TransID &tid2) const;
    INLINE_LOG operator Transaction * () const;
    INLINE_LOG UniqueTid id() const { return uniqueid; }

    INLINE_LOG int ttslot() const { return tt_slot;}
    INLINE_LOG Transaction *trans() const;


    /* Nested operation functions */

    INLINE_LOG int beginOp(DaliDB *db,DaliLogOpCode op, DaliOpID *idp = 0) const;
    INLINE_LOG int commitOp(DaliDB *db, void *descriptor, int size, 
                        DaliOpID *idp = 0) const;
    INLINE_LOG int commitPostOp(DaliTransTimeStamp _TS, DaliOpID *idp = 0) const;
    INLINE_LOG int commitUndoOp(DaliOpID *idp = 0) const;
    INLINE_LOG int abortOp(DaliOpID *idp = 0) const;

    INLINE_LOG int registerPostCommit(DaliDB *db, DaliLogOpCode op,void *descriptor, int size, DaliOpID *op_id = NULL) const;

    friend ostream & operator << (ostream& s, const TransID &tid);
    friend int lesser_tid(const TransID &t1, const TransID &t2);
    friend class TransList;
    friend class ActiveTransList;
    friend class RecoveryTransList;
    friend class SysDB;
};

extern const TransID TransID_NULL;  

inline int TransID::operator ==(const TransID &tid2) const {
        return (uniqueid == tid2.uniqueid && tt_slot == tid2.tt_slot);
}
inline int TransID::operator !=(const TransID &tid2) const {
        return (uniqueid != tid2.uniqueid || tt_slot != tid2.tt_slot);
}

inline TransID::operator Transaction *() const {
    return trans();
}

#endif /* DALI_TRANS_H */





