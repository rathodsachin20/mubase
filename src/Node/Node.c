// node base class

#include <assert.h>

#include "List.h"
#include "Node.h"

int Node_t::CurID = 0;                  // node count

// degree of sharing of all equivalence nodes below this node
List_t<DegShar_t *> *LogicalNode_t::DegreeList(int *isLast)
{
    if( !degreeList ) {
        degreeList = CreateDegreeList(); 

        listRefCount = NumParents();
        assert(listRefCount >= 0);
        if( listRefCount == 0 ) {
            // root node
            listRefCount = 1;
        }
    }

    List_t<DegShar_t *> *tmp = degreeList;
    listRefCount--;

    // if this is the last reference, then make degreeList NULL
    // *isLast == 1 if this is the last ref; i.e. degree list should
    // be deleted by the caller; 0 otherwise
    if( listRefCount == 0 ) {
        degreeList = NULL;
        if( isLast ) 
            *isLast = 1;
    } else {
        if( isLast )
            *isLast = 0;
    }

    return tmp;
}

// delete degree list
void LogicalNode_t::DeleteDegreeList(List_t<DegShar_t *> *degreeList)
{
    assert(degreeList);

    ListIter_t<DegShar_t *> iter;
    iter.Attach(degreeList);

    while( !iter.IsEnd() ) {
        DegShar_t *deg = iter.Next();
        DeRefer(deg);
    }

    delete degreeList;
}

// delete degree list of this node
LogicalNode_t::~LogicalNode_t(void)
{
    if( !degreeList )
        return;

    ListIter_t<DegShar_t *> iter;
    iter.Attach(degreeList);
    
    while( !iter.IsEnd() ) {
        DegShar_t *degShar = iter.Next();

        DeRefer(degShar);
    }

    delete degreeList;
}
