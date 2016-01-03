// node base class

#ifndef __NODE_H__
#define __NODE_H__

#include <assert.h>

#include "List.h"
#include "RefCount.h"

class Equivalence_t;

// base class for equivalence and operator nodes
class Node_t {
    static int CurID;           // current id for next assignment
    int id;                     // id of the node

public:
    Node_t(void) : id(CurID++) { }

    // get the node's id
    int ID(void) const { return id; }

    // check for equality
    int IsEqual(const Node_t *x) const
    {
        assert(x);
        return x->ID() == id;
    }

    virtual ~Node_t(void)
    { }
};

// degree of sharing associated with an equivalence class
class DegShar_t : public RefC_t {
    Equivalence_t *eq;
    int degree;

public:
    DegShar_t(Equivalence_t *eq_a, int degree_a)
        : eq(eq_a), degree(degree_a)
    { assert(eq); assert(degree >= 0); }

    Equivalence_t *EqNode(void) const
    { return eq; }

    int Degree(void) const
    { return degree; }

    ~DegShar_t(void)
    { }
};

// base class for equivalence and logical operator nodes
class LogicalNode_t {
    AppendList_t<DegShar_t *> *degreeList;
                                        // the degree of sharing
                                        // of all equivalence nodes
                                        // below this node
    int listRefCount;                   // number of references for
                                        // which degreeList need sto be cached

    // create the list of degree of sharing given the lists of the children
    // logical op nodes
    // the children lists are topologically sorted wrt the eq nodes
    virtual AppendList_t<DegShar_t *> *CreateDegreeList(void) = 0;

public:
    LogicalNode_t(void) : degreeList(NULL), listRefCount(0) { }

    // number of parents
    virtual int NumParents(void) const = 0;

    // degree of sharing of all equivalence nodes below this node
    List_t<DegShar_t *> *DegreeList(int *isLast);

    // delete degree list
    static void DeleteDegreeList(List_t<DegShar_t *> *degreeList);

    virtual ~LogicalNode_t(void);
};

#endif // __NODE_H__
