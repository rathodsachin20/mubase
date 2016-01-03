#ifndef ITER_H
#define ITER_H

class Predicate;

#include "rm.h"
#include "sp.h"
#include "sputil.h"

class Schema;
class Record;

// Interface to be implemented by all operators and base-rel scan

class RIterator {
public:
    virtual void open() = 0;

    virtual Record* next() = 0;
    virtual void close() = 0;
};


// Iterator for a relation.

class BaseRelIterator : public RIterator {
    // Here go the attributes to maintain iterator state
    const char* dbName;
    const char* relName;
    Schema* relSchemaPtr;
    int currBlock;
    short currSlot;
    SlottedPage *sp;
    short totalSlots;
    int nextBlock;
    Record* currRec;

public:

    BaseRelIterator(const char *dbName1, const char *relName1);


    // Open the iterator, initializing its state
    void open();
    // Returns the next record, NULL if no more records.
    Record* next();

    // Close the iterator, unpin any pinned buffer blocks
    void close();
};

class Predicate {
public:

    virtual bool evaluate(Record* rec) = 0;
};

class PredRelIterator : public RIterator {
    Predicate *pred;
    BaseRelIterator *base;
public:

    PredRelIterator(const char *dbName1, const char *relName1, Predicate *pred) {
        base = new BaseRelIterator(dbName1, relName1);
        this->pred = pred;
    }
    void open();
    Record* next();
    void close();
};

class BasePredicate : public Predicate {
public:
    bool val;

    BasePredicate(bool value) {
        val = value;
    }
    bool evaluate(Record *rec);

};

class UnaryPredicate : public Predicate {
public:
    Predicate *pred_exp;

    UnaryPredicate(Predicate *pred) {
        pred_exp = pred;
    }
};

class BinaryPredicate : public Predicate {
public:
    Predicate* leftPred;
    Predicate* rightPred;

    BinaryPredicate(Predicate * lP, Predicate* rP) {
        leftPred = lP;
        rightPred = rP;
    }
};

class NotPredicate : public UnaryPredicate {
public:

    NotPredicate(Predicate *predExp) : UnaryPredicate(predExp) {
    }
    bool evaluate(Record *rec, Schema* sch);
};

class OrPredicate : public BinaryPredicate {
public:

    OrPredicate(Predicate *lPred, Predicate *rPred) : BinaryPredicate(lPred, rPred) {
    }
    bool evaluate(Record *rec);
};

class AndPredicate : public BinaryPredicate {
public:

    AndPredicate(Predicate *lPred, Predicate *rPred) : BinaryPredicate(lPred, rPred) {
    }
    bool evaluate(Record *rec);
};

class CompPred : public Predicate {
public:
    string colName;
    string value;

    CompPred(string colName, string value) {
        
        this->colName = colName;
        this->value = value;
        
    }

};

class EqualPred : public CompPred {
public:

    EqualPred(string colName, string value) : CompPred(colName, value) {
        //cout << "called EqualPred constructor" << endl;
    }
    bool evaluate(Record *rec);
};

class NotEqualPred : public CompPred {
public:

    NotEqualPred(string colName, string value) : CompPred(colName, value) {
        //cout << "called NotEqualPred constructor" << endl;
    }
    bool evaluate(Record *rec);
};

class GreaterPred : public CompPred {
public:

    GreaterPred(string colName, string value) : CompPred(colName, value) {
        //cout << "called GreaterPred constructor" << endl;
    }
    bool evaluate(Record *rec);
};

class LesserPred : public CompPred {
public:

    LesserPred(string colName, string value) : CompPred(colName, value) {
        //cout << "called equalpred constructor" << endl;
    }
    bool evaluate(Record *rec);
};

class GreaterEqPred : public CompPred {
public:

    GreaterEqPred(string colName, string value) : CompPred(colName, value) {
        //cout << "called GreaterEqPred constructor" << endl;
    }
    bool evaluate(Record *rec);
};

class LesserEqPred : public CompPred {
public:

    LesserEqPred(string colName, string value) : CompPred(colName, value) {
        //cout << "called LesserEqPred constructor" << endl;
    }
    bool evaluate(Record *rec);
};




#endif
