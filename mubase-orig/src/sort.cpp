/*
-------------------
External Merge Sort

For run generation use EMSort_BUFFS no. of frames in which 1 is 
used for output/creating run and rest are used to sort the blocks
------------------- 
*/

#include "sort.h"
//#include "Bheap.h"
//#include "bm.h"

using namespace std;
//QUICK SORT ON A SLOTTED PAGE
Sort::Sort(char * dbName1,char *relName1,int attrIndex1)
{	
	dbName=dbName1;
	relName=relName1;
	attrIndex=attrIndex1;
	//RunGeneration();
	int blockId = StorageManager::getFirstBlockId(dbName, relName );
	char * buff = BufferManager::getInstance(dbName)->pinPage(blockId);
	SlottedPage* sp=new SlottedPage(buff);
	Quicksort(sp ,0 , sp->getTotalNoOfSlots());
	
}

void Sort::Quicksort(SlottedPage *sp,short left,short right)
{
	if (left < right)
	{
		int pivotIndex = left + rand()%(right-left); 				
		//Choose random pivotindex call partition 
		short pivotNew = Partition( sp,left, right, pivotIndex);		
		//Partition returns the value of pivotIndex at a new index that is pivotNew all elements to left of pivotNew are less than value of pivotNew on right theey are greater 
		
		Quicksort(sp,left, pivotNew - 1);
		Quicksort( sp,pivotNew + 1, right);
	}
}

//Partition function for quick sort
short Sort::Partition (SlottedPage *sp,short left,short right,short pivotIndex)///reccbytes,relationschema
{
	Schema* relSchema ;
	relSchema=new Schema( *(Relation::readSchema(dbName,relName)));		//Get the schema for making an object for record
	
	Record* pivot;
	int lenPivotRec;
	char *pivotRecBytes= sp->getRecord(pivotIndex,lenPivotRec);			//GET THE VALUE OF RECORD...
    	pivot = new Record(pivotRecBytes , relSchema ,lenPivotRec );
	while(left<right)
	{
		
		Record* temp_l;
		int lenLeftRec;
		char * tempLRecBytes = sp->getRecord(left,lenLeftRec);
		
		
		while( tempLRecBytes == (char*)(-1))
		{
			left++;
			tempLRecBytes=sp->getRecord(left,lenLeftRec);
		}
		temp_l=new Record(tempLRecBytes,relSchema , lenLeftRec);
		
		while (temp_l->isLesser(pivot,attrIndex) ==true) 
		{
			left++;
			char* tempLRecBytes= sp->getRecord(left,lenLeftRec);
				
			while(tempLRecBytes == (char*)(-1))
			{
				left++;
				tempLRecBytes=sp->getRecord(left,lenLeftRec);
			}
			temp_l=new Record(tempLRecBytes,relSchema , lenLeftRec);
			
		}
		
		Record* temp_r;
		char* tempRRecByte;
		int lenRightRec;
		tempRRecByte=sp->getRecord(right,lenRightRec);
		while( tempRRecByte == (char*)(-1))
		{
			right--;
			tempRRecByte=sp->getRecord(right,lenRightRec);
		}
		temp_r=new Record(tempRRecByte,relSchema , lenRightRec);
		
		while (temp_r->isGreater(pivot,attrIndex) ==true )
		{
			right--;
			char* tempRRecByte= sp->getRecord(right,lenRightRec);

			while(tempRRecByte== (char*)(-1))
			{
				right--;
				tempRRecByte=sp->getRecord(right,lenRightRec);
			}
			temp_r=new Record(tempRRecByte,relSchema , lenRightRec);
		}
		if (left < right && temp_l->isEqual(temp_r,attrIndex)==false)		//Swap the two numbers... if they are same there is no point in swapping
        {
            sp->swapSlots(left,right);							//Swapping function  made by relational manager 
        }
		
		else if (left < right)									// If sp(left)=sp(right) but still there are terms in between left and right on which swapping has to be done 
		{
			right--;											//Move right by one in left direction 
		}
		
		else {return right;} 									//Any other case
		
    }
    return right;
}


/*

//Run Generation
int Sort::CreateRun(int LastRunFlag,runlinklist run)
{
	
	int FrameId[EMSort_BUFFS];								//Frame IDs used for sorting 
	for (int j=0;j<EMSort_BUFFS;j++)
	{
		FrameID[j]=BufferManager::getFreeFrame();			// blockInFrame[frameid]
		if (FrameID[j]==-1)
		{
			cout<<"Enough frames not available for sorting ...Terminating !!!";
			exit(0);
		}
	}	
	
	
	
	
	short BlockId[EMSort_BUFFS];		
	SlottedPage *SpFrame[EMSort_BUFFS-1];						//Slotted page array for particular frame
		
	BlockId[0]=StorageManager::getFirstBlockId(dbName,relName);
	
	
	for(int i=0;i<EMSort_BUFFS-2;i++)		
	{
		SpFrame[i]= new SlottedPage(BufferManager::getInstance(dbName)->pinPage(BlockId[i]));		//Pin the block to buffer and create a slotted page for that block
		short last =( SpFrame[i]->getTotalNoOfSlots() )-1;											//Get the last slot no. "-1" as it starts from zero	
		Quicksort(SpFrame[i],0,last)																//QuickSort the slotted page
		short t;
		t=SpFrame[i]->getNextBlockId();												//Get next block id using this slotted page 
		if(t==-1)
		{
			LastRunFlag=1;
		}
		else
		{
			BlockId[i+1]=t;
		}
	}
	
	
	int FreeBlockId=StorageManager::allocateBlock(dbName);								//Returns a free block form the free list 
	SlottedPage *SpforRun;
	SpforRun=new SlottedPage(BufferManager::getInstance(dbName)->pinPage(FreeBlockId));		//Convert that free block frame into slotted page so that we can write into it 
	
	Bheap RHeap(EMSort_BUFFS,attrIndex)							//Run- Heap ie heap for run generation... Using Min Heap 
	

	for(int z=0;z<EMSort_BUFFS-2;z++)						//Insert first record of each EMSort_BUFFS-1 blocks into the heap
	{
		RHeap.insert( SpFrame[0]->)getRecord(0),0,0 );		//Return the pointer of record at 0 slot no. in slotted page .. as the slottedpage is sorted slot no 0 has min record
	}
	
	bool firstBlock=0;
	bool flag=0; 
	while (flag==0)												//Main Run generation done here 
	{
		Record Rec;
		char *Recbyt;
		int x=0;
		while(x!=-1)									//If -1 => the block is full, or all the blocks might be empty
		{
			int RecSlotNo;
			RecSlotNo=RHeap.getSlotNo();
			int RecSpIndex;
			RecSpIndex=RHeap.getSpIndex();
			
			Rec = RHeap.ExtractMin();								
			
			Recbyt= Rec.getRecBytes();																		//Convert record into recbytes
		
			x=SpforRun.storeRecord(Recbyt,Rec.length());													//Insert the record into the output block/frame
			if(firstBlock==0)
			{
				run.RunIndex=FreeBlockId;																	//Insert the first block id into the run array 
				firstBlock=1;
			}
			
			if ( RecSlotNo == SpFrame[RecSpIndex]->getTotalNoOfSlots()-1 )									//If the end of block is reached dont insert into the heap
			{
				EndofSlotCount++;																			//Insert nothing and increment the EndofSlotCount 
			}
			
			else
			{
				RHeap.insert( SpFrame[RecSpIndex]->getRecord(RecSlotNo+1), RecSpIndex , RecSlotNo+1 );		//Insert next element from the same block into the heap 
			}
		}
		
		
		if(EndofSlotCount==EMSort_BUFFS-1)																	//When all blocks are finsihed get out of the loop 
		{
			BufferManager::getInstance(dbName)->unpinPage(FreeBlockId, 1)									//is dirty 1
			flag=1;
		}
		
		else
		{
			int FreeBlockId2=StorageManager::allocateBlock(dbName);
			SpforRun.writeNextBlockId(FreeBlockId2);
			BufferManager::getInstance(dbName)->unpinPage(FreeBlockId, 1)									
			
			SpforRun= new SlottedPage(BufferManager::getInstance(dbName)->pinPage(FreeBlockId2));			//Convert that free block frame into slotted page so that we can write into it 
			x=SpforRun.storeRecord(Recbyt,Rec.length());													//when x became -1 it came out of the loop to take new block insert into this new block				
		
		}
		
		//When all Records are inserted in a sorted order ... run generation is completed ...after set flag 1
	}	
	
	
	for(int i=0;i<EMSort_BUFFS-2;i++)																		//Unpinning all the blocks
	{
		BufferManager::getInstance(dbName)->unpinPage(BlockId[i], 0);										//Unpin all the EMSort_BUFFS from the buffer
	}
	
	return LastRunFlag;			//return value of LastRunFlag 
	
}


void Sort::RunGeneration()
{
	runlinklist *head;
	runlinklist *run;
	runlinklist *prevrun;
	
	count=0;
	bool LastRunFlag=0;
	
	while (LastRunFlag!=1)
	{
		run = new runlinklist;
		
		if (count ==0)
		{
			head=run;
			prevrun=run;
		}
		else 
		{
			prevrun->next=run;
		}
		run.no=count;
		LastRunFlag=createrun(LastRunFlag,run); 			//If LastRunFlag =-1 run creation is complete 
		
		prevrun=run;
		count++;
	
	}
	
	MergeGeneration(head,count);		//Once run generation is complete call merge by passing the link list of run 
}

void Sort::Merge(int *BlockId,int size , runlinklist *run2 )
{
	
	int FrameId[EMSort_BUFFS];								//Frame IDs used for sorting 
	for (int j=0;j<EMSort_BUFFS;j++)
	{
		FrameID[j]=BufferManager::getFreeFrame();			// blockInFrame[frameid]
		if (FrameID[j]==-1)
		{
			cout<<"Enough frames not available for sorting ...Terminating !!!";
			exit(0);
		}
	}	
	
	
	int RunCount = count;
	SlottedPage *SpFrame[EMSort_BUFFS-1];
	
	for (int i=0; i<size-1;i++ )		//EMSort_BUFFS-1 blocks pinned to buffer
	{	
		
		SpFrame[i]= new SlottedPage(BufferManager::getInstance(dbName)->pinPage(BlockId[i]);
		
		
		if (i==RunCount-1)
		{
			break;
		}
	
	}
	Bheap MHeap(EMSort_BUFFS,attrIndex)	 ;
	for(int z=0;z<EMSort_BUFFS-2;z++)						//Insert first record of each EMSort_BUFFS-1 blocks into the heap
	{
		RHeap.insert( SpFrame[0]->)getRecord(0),0,0 )	;	//Return the pointer of record at 0 slot no. in slotted page .. as the slottedpage is sorted slot no 0 has min record
	}
	Record Rec;
	char *Recbyt;
	int FreeBlockId=StorageManager::allocateBlock(dbName);								//Returns a free block form the free list 
	SlottedPage *SpforMerge;
	SpforMerge=new SlottedPage(BufferManager::getInstance(dbName)->pinPage(FreeBlockId));		//Convert that free block frame into slotted page so that we can write into it 
	bool firstBlock;	
	bool flag=0;
	int EndofBlockCount;
	while(flag==0)
	{
		int x;
		while(x!=-1)
		{
			int RecSlotNo;
			RecSlotNo=MHeap.getSlotNo();
			int RecSpIndex;
			RecSpIndex=MHeap.getSpIndex();
			Rec = MHeap.ExtractMin();			
			
			Recbyt= Rec.getRecBytes();																					//Convert record into recbytes
			x=SpforMerge.storeRecord(Recbyt,Rec.length());																//Insert the record into the output block/frame 
	
			if(firstBlock == 0)
			{
				run2.RunIndex=FreeBlockId;					 						//Insert the first block id into the run array 
				firstBlock=1;
			}
			
			if ( RecSlotNo == SpFrame[RecSpIndex]->getTotalNoOfSlots()-1 )			//If the end of block is reached dont insert into the heap
			{
				short temp=SpFrame[RecSpIndex]->getNextBlockId();					//Insert nothing and get the next block
				if (temp==-1)
				{
					EndofBlockCount++;
				}
				else
				{
					BufferManager::getInstance(dbName)->unpinPage(BlockId[RecSpIndex], 0);		//Unpin the block and pin the next block in particular run
					deallocate (BlockId[RecSpIndex]);
					BlockId[RecSpIndex]=temp;		
					SpFrame[RecSpIndex]= new SlottedPage(BufferManager::getInstance(dbName)->pinPage(BlockId[RecSpIndex]);		//Pin new block
					MHeap.insert( SpFrame[RecSpIndex]->getRecord(0) , RecSpIndex , 0 );	
				}
			}
			
			else
			{
				MHeap.insert( SpFrame[RecSpIndex]->getRecord(RecSlotNo+1) , RecSpIndex , RecSlotNo+1 );		//Insert next element from the same block into the heap 
			}
		
		}
		
		if(EndofBlockCount==EMSort_BUFFS-1)		//When all blocks are finsihed get out of the loop 
		{
			BufferManager::getInstance(dbName)->unpinPage(FreeBlockId, 1);
			flag=1;
		}
	
		else
		{
			int FreeBlockId2=StorageManager::allocateBlock(dbName);
			SpforMerge.writeNextBlockId(FreeBlockId2);
			BufferManager::getInstance(dbName)->unpinPage(FreeBlockId, 1);
			
			SpforMerge= new SlottedPage(BufferManager::getInstance(dbName)->pinPage(FreeBlockId2));	//Convert that free block frame into slotted page so that we can write into it 
			x=SpforMerge.storeRecord(Recbyt,Rec.length());															//when x became -1 it came out of the loop to take new block insert into this new block				
		
		}

	}
	for(int i=0;i<EMSort_BUFFS-2;i++)				//Unpinning all the blocks
	{
		BufferManager::getInstance(dbName)->unpinPage(BlockId[i], 0);			//Unpin all the EMSort_BUFFS from the buffer
	}
	
}


short Sort::MergeGeneration(runlinklist *head0,int count1)				//Returns block id for the first the first block of sorted db
{
	runlinklist *head=head0;
	runlinklist *run1;
	runlinklist *run2;
	runlinklist *head1;
	runlinklist *head2;
	runlinklist *prevrun;
	int count =count1;
	run1=head;
	head1=run1;
	head2=run2;
	while (head1->next!=NULL)							//When there is only one element left in the  linklist
	{
		run1=head1;
		int counter =0;
		while(run1!=NULL)
		{
			short BlockId[EMSort_BUFFS-1];
			for (int i=0;i<EMSort_BUFFS-2;i++)
			{
				BlockId[i]=run1->RunIndex;
				run1=run1->next;
				if(run1==NULL)							//If the linklist end get out of the loop
				{break;}
			}
			run2 = new runlinklist;
			if (counter == 0)
			{
				head2=run2;
				prevrun=run2;
			}
			else
			{
				prevrun->next=run2;
			}
			run2.no=counter;
			
			Merge(BlockId,i+1,run2);			//Call merge where the run2 is created
			
			prevrun=run2;
			counter++;
		
		}
		head1=head2;			//Equate run2 to run1 and again start the loop
	}
	return head->RunIndex;
	
}
*/
