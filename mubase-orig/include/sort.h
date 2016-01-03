#ifndef SORT_H
#define SORT_H

#include "rm.h"


class runlinklist			//Linklost containing runs
{
	public:
	int no;
	int RunIndex;
	
	
	runlinklist* next;
	
	runlinklist()
	{
		
		next=NULL;
	}
};

class Sort : public RIterator {
	
	Relation *rel;
	char *dbName,*relName,*buffer;
	int attrIndex;
	/* order is false if output is in ascending order*/
	bool order;
	int head,tail;
	
	public:
	
	/**
	 * constructor
	 * Takes database name relation name on which sorting has to be done
	 *Also takes the attribute index on which sorting has to be done 
	 */
	Sort(char * dbName1,char *relName1,int attrIndex1);		

	/**
	*Quicksort the slot no.s of the slotted page 
	*/
	void Quicksort(SlottedPage *sp,short left,short right);
	
	/**
	*Parition used for quicksort
	*/
	short Partition (SlottedPage *sp,short left,short right,short pivotIndex);
	
	/**
	*Returns 1 after the run is created 
	*/
	int CreateRun(int LastRunFlag,runlinklist run);
	
	/**
	*Runs generation takes place here
	*/
	void RunGeneration();
	
	/**
	*Merge Runs in asc order
	*/
	void Merge(int *BlockId,int size , runlinklist *run2 );
	
	/**
	*Merging phases take place here
	*Returns the first block id of the fully sorted relation
	*/
	short MergeGeneration(runlinklist *head0,int count1);
	
	



    virtual void open()
    {}
    virtual Record* next() 
    {
	    Record* R = NULL; 
	    return R;
    }
    virtual void close()
    {}



};


#endif
