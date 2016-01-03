

#include "iter.h"

BaseRelIterator::BaseRelIterator(const char *dbName1, const char *relName1) {
    dbName = dbName1;
    relName = relName1;
    relSchemaPtr = new Schema(*(Relation::readSchema(dbName, relName)));
}

// Open the iterator, initializing its state

void BaseRelIterator::open() {
    currBlock = StorageManager::getFirstBlockId(dbName, relName);
    char* curBlockBuff = BufferManager::getInstance(dbName)->pinPage(currBlock);
    sp = new SlottedPage(curBlockBuff);
    currSlot = 0;
    totalSlots = sp->getTotalNoOfSlots();
    nextBlock = sp->getNextBlockId();
}

// Returns the next record, NULL if no more records.

Record* BaseRelIterator::next() {
    int l;
    //cout<<"\nIN NEXT\n";
    //cout<<"currSlot="<<currSlot<<" totalSlots="<<totalSlots<<endl;
    while (currSlot < totalSlots) {

        char * recBytes = sp->getRecord(currSlot, l);
        if (recBytes == (char*) - 1) {
            currSlot++;
            continue;
        } else {
            currRec = new Record(recBytes, relSchemaPtr, l);
            currSlot++;
            return currRec;
        }

    }

    if (currSlot >= totalSlots && nextBlock != -1) {
        BufferManager::getInstance(dbName)->unpinPage(currBlock, 0);
        currBlock = nextBlock;
        char* curBlockBuff = BufferManager::getInstance(dbName)->pinPage(currBlock);
        //delete sp;
        sp = new SlottedPage(curBlockBuff);
        currSlot = 0;
        totalSlots = sp->getTotalNoOfSlots();
        nextBlock = sp->getNextBlockId();

        //Assuming that next block will contain atleast one record, so that
        //fn will surely return after entering next while loop
        while (currSlot < totalSlots) {
            char * recBytes = sp->getRecord(currSlot, l);
            if (recBytes == (char*) - 1) {
                currSlot++;
                continue;
            } else {
                currRec = new Record(recBytes, relSchemaPtr, l);
                return currRec;
            }

        }

    }
    //If there are no more records & no more blocks, return NULL
    //cout<<"No More Records"<<endl;
    return NULL;

}

// Close the iterator, unpin any pinned buffer blocks

void BaseRelIterator::close() {
    //cout<<"in close\n";
    BufferManager::getInstance(dbName)->unpinPage(currBlock, 0);
    //delete sp;    
}
class Predicate;

void PredRelIterator::open() {
    base->open();
}

Record* PredRelIterator::next() {
    Record* rec;
    rec = base->next();
    while (rec != NULL) {
        if (pred->evaluate(rec))
            return rec;
        rec = base->next();
    }
    return NULL;
}

void PredRelIterator::close() {
    base->close();
}

bool BasePredicate::evaluate(Record *rec) {
    return val;
}

bool NotPredicate::evaluate(Record *rec, Schema* sch) {
    return (!(pred_exp->evaluate(rec)));
}

bool OrPredicate::evaluate(Record *rec) {
    return (leftPred->evaluate(rec) || rightPred->evaluate(rec));
}

bool AndPredicate::evaluate(Record *rec) {
    return (leftPred->evaluate(rec) && rightPred->evaluate(rec));
}

bool EqualPred::evaluate(Record *rec) {
    char* colgiven = &colName[0]; //converting the given colname of string type to char* type.(our Schema stored in char* format.) 
    Schema *s = rec->getSchema();
    int position = -1;
    for (int i = 0; i < (s->getNumAttrs()); i++) { //get index number(position) of attribute
        if (strcmp(s->getAttrName(i), colgiven) == 0) {
            position = i;
            //cout<<"Column Found at position:"<<position<<endl;
            break;
        }
    }
    if (position == -1)
        cout << "given coloumn name  " << colName << "does not exist" << endl;
    else {
        if (s->getAttrType(position) == 1) {
            int givenvalue;
            stringstream ss;
            ss << value;
            ss >> givenvalue;
            if (rec->getInt(position) == givenvalue)
                return true;
            else
                return false;
        } else if (s->getAttrType(position) == 2) {
            float givenvalue;
            stringstream ss;
            ss << value;
            ss >> givenvalue;
            if (rec->getFloat(position) == givenvalue)
                return true;
            else
                return false;
        } else if (s->getAttrType(position) == 3) {
            char* givenvalue = &value[0];
            if (strcmp(rec->getConstString(position), givenvalue) == 0)
                return true;
            else
                return false;
        } else if (s->getAttrType(position) == 4) {
            char* givenvalue = &value[0];
            //cout<<"rec->getVarString(position)="<<rec->getVarString(position)<<" GivenValue="<<givenvalue<<endl;
            if (strcmp(rec->getVarString(position), givenvalue) == 0) {
                return true;
            } else
                return false;
        }
    }
    return false;
}

bool NotEqualPred::evaluate(Record *rec) {
    char* colgiven = &colName[0]; //converting the given colname of string type to char* type.(our Schema stored in char* format.) 
    Schema *s = rec->getSchema();
    int position = -1;
    for (int i = 0; i < (s->getNumAttrs()); i++) { //get index number(position) of attribute
        if (strcmp(s->getAttrName(i), colgiven) == 0) {
            position = i;
            break;
        }
    }
    if (position == -1)
        cout << "given coloumn name  " << colName << "does not exist" << endl;
    else {
        if (s->getAttrType(position) == 1) {
            int givenvalue;
            stringstream ss;
            ss << value;
            ss >> givenvalue;
            if (rec->getInt(position) != givenvalue)
                return true;
            else
                return false;
        } else if (s->getAttrType(position) == 2) {
            float givenvalue;
            stringstream ss;
            ss << value;
            ss >> givenvalue;
            if (rec->getFloat(position) != givenvalue)
                return true;
            else
                return false;
        } else if (s->getAttrType(position) == 3) {
            char* givenvalue = &value[0];
            if (strcmp(rec->getConstString(position), givenvalue) != 0)
                return true;
            else
                return false;
        } else if (s->getAttrType(position) == 4) {
            char* givenvalue = &value[0];
            if (strcmp(rec->getVarString(position), givenvalue) != 0)
                return true;
            else
                return false;
        }
    }
}

bool GreaterPred::evaluate(Record *rec) {
    char* colgiven = &colName[0]; //converting the given colname of string type to char* type.(our Schema stored in char* format.) 
    Schema *s = rec->getSchema();
    int position = -1;
    for (int i = 0; i < (s->getNumAttrs()); i++) { //get index number(position) of attribute
        if (strcmp(s->getAttrName(i), colgiven) == 0) {
            position = i;
            break;
        }
    }
    if (position == -1)
        cout << "given coloumn name  " << colName << "does not exist" << endl;
    else {
        if (s->getAttrType(position) == 1) {
            int givenvalue;
            stringstream ss;
            ss << value;
            ss >> givenvalue;
            if (rec->getInt(position) > givenvalue)
                return true;
            else
                return false;
        } else if (s->getAttrType(position) == 2) {
            float givenvalue;
            stringstream ss;
            ss << value;
            ss >> givenvalue;
            if (rec->getFloat(position) > givenvalue)
                return true;
            else
                return false;
        } else if (s->getAttrType(position) == 3) {
            char* givenvalue = &value[0];
            if (strcmp(rec->getConstString(position), givenvalue) > 0)
                return true;
            else
                return false;
        } else if (s->getAttrType(position) == 4) {
            char* givenvalue = &value[0];
            if (strcmp(rec->getVarString(position), givenvalue) > 0)
                return true;
            else
                return false;
        }
    }
}

bool LesserPred::evaluate(Record *rec) {
    char* colgiven = &colName[0]; //converting the given colname of string type to char* type.(our Schema stored in char* format.) 
    Schema *s = rec->getSchema();
    int position = -1;
    for (int i = 0; i < (s->getNumAttrs()); i++) { //get index number(position) of attribute
        if (strcmp(s->getAttrName(i), colgiven) == 0) {
            position = i;
            break;
        }
    }
    if (position == -1)
        cout << "given coloumn name  " << colName << "does not exist" << endl;
    else {
        if (s->getAttrType(position) == 1) {
            int givenvalue;
            stringstream ss;
            ss << value;
            ss >> givenvalue;
            if (rec->getInt(position) < givenvalue)
                return true;
            else
                return false;
        } else if (s->getAttrType(position) == 2) {
            float givenvalue;
            stringstream ss;
            ss << value;
            ss >> givenvalue;
            if (rec->getFloat(position) < givenvalue)
                return true;
            else
                return false;
        } else if (s->getAttrType(position) == 3) {
            char* givenvalue = &value[0];
            if (strcmp(rec->getConstString(position), givenvalue) < 0)
                return true;
            else
                return false;
        } else if (s->getAttrType(position) == 4) {
            char* givenvalue = &value[0];
            if (strcmp(rec->getVarString(position), givenvalue) < 0)
                return true;
            else
                return false;
        }
    }
}

bool GreaterEqPred::evaluate(Record *rec) {
    char* colgiven = &colName[0]; //converting the given colname of string type to char* type.(our Schema stored in char* format.) 
    Schema *s = rec->getSchema();
    int position = -1;
    for (int i = 0; i < (s->getNumAttrs()); i++) { //get index number(position) of attribute
        if (strcmp(s->getAttrName(i), colgiven) == 0) {
            position = i;
            break;
        }
    }
    if (position == -1)
        cout << "given coloumn name  " << colName << "does not exist" << endl;
    else {
        if (s->getAttrType(position) == 1) {
            int givenvalue;
            stringstream ss;
            ss << value;
            ss >> givenvalue;
            if (rec->getInt(position) >= givenvalue)
                return true;
            else
                return false;
        } else if (s->getAttrType(position) == 2) {
            float givenvalue;
            stringstream ss;
            ss << value;
            ss >> givenvalue;
            if (rec->getFloat(position) >= givenvalue)
                return true;
            else
                return false;
        } else if (s->getAttrType(position) == 3) {
            char* givenvalue = &value[0];
            if (strcmp(rec->getConstString(position), givenvalue) >= 0)
                return true;
            else
                return false;
        } else if (s->getAttrType(position) == 4) {
            char* givenvalue = &value[0];
            if (strcmp(rec->getVarString(position), givenvalue) >= 0)
                return true;
            else
                return false;
        }
    }
}

bool LesserEqPred::evaluate(Record *rec) {
    char* colgiven = &colName[0]; //converting the given colname of string type to char* type.(our Schema stored in char* format.) 
    Schema *s = rec->getSchema();
    int position = -1;
    for (int i = 0; i < (s->getNumAttrs()); i++) { //get index number(position) of attribute
        if (strcmp(s->getAttrName(i), colgiven) == 0) {
            position = i;
            break;
        }
    }
    if (position == -1)
        cout << "given coloumn name  " << colName << "does not exist" << endl;
    else {
        if (s->getAttrType(position) == 1) {
            int givenvalue;
            stringstream ss;
            ss << value;
            ss >> givenvalue;
            if (rec->getInt(position) <= givenvalue)
                return true;
            else
                return false;
        } else if (s->getAttrType(position) == 2) {
            float givenvalue;
            stringstream ss;
            ss << value;
            ss >> givenvalue;
            if (rec->getFloat(position) <= givenvalue)
                return true;
            else
                return false;
        } else if (s->getAttrType(position) == 3) {
            char* givenvalue = &value[0];
            if (strcmp(rec->getConstString(position), givenvalue) <= 0)
                return true;
            else
                return false;
        } else if (s->getAttrType(position) == 4) {
            char* givenvalue = &value[0];
            if (strcmp(rec->getVarString(position), givenvalue) <= 0)
                return true;
            else
                return false;
        }
    }
}







