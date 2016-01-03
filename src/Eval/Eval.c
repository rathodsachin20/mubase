#include "Eval.h"
#include "rm.h"
#include "sort.h"

void eval(Plan_t *plan){
    PhysicalOp_t * root = plan->Root();
    
    cout<<"\nPhysical Operator tree:\n\n";
    PrintOps(root,0 );
    cout<<endl<<endl;
    
//Mubase initialisation

    cout << "\n STARTING MUBASE\n";

    DiskManager::createDB("mydb.txt", 20);
    StorageManager::initDB("mydb.txt");
    DiskManager::openDB("mydb.txt");

    //Create Relations
    cout << "\nCreating Relations\n";
    const short int numAttrs1 = 4;
    short int attrType1[numAttrs1] = {1, 4, 4, 2};
    short int sizeAttrs1[numAttrs1] = {sizeof(int), 30, 30, sizeof(float)};
    char *attrName1[numAttrs1] = {"r1c1", "r1c2", "r1c3", "r1c4"}; //Column Names for relation r1
    Schema sch1(numAttrs1, attrType1, attrName1, sizeAttrs1);
    Relation r1("mydb.txt", "r1", true, &sch1);

    const short int numAttrs2 = 3;
    short int attrType2[numAttrs2] = {1, 4, 1};
    short int sizeAttrs2[numAttrs2] = {sizeof(int), 30, sizeof(int)};
    char* attrName2[numAttrs2] = {"r2c1", "r2c2", "r2c3"};  //Column Names for relation r2
    Schema sch2(numAttrs2, attrType2, attrName2, sizeAttrs2);
    Relation r2("mydb.txt", "r2", true, &sch2);
    
    const short int numAttrs3 = 2;
    short int attrType3[numAttrs3] = {1, 4};
    short int sizeAttrs3[numAttrs3] = { sizeof(int), 30};
    char* attrName3[numAttrs3] = {"r3c1", "r3c2"};  //Column Names for relation r3
    Schema sch3(numAttrs3, attrType3, attrName3, sizeAttrs3);
    Relation r3("mydb.txt", "r3", true, &sch3);
    
    
    
    //creating records
    string recString1[numAttrs1] = {"29", "sachin", "rathod", "11.11"};
    Record* recp1 = new Record(recString1, &sch1, numAttrs1);
    string recString2[numAttrs1] = {"09", "aniket", "deshmukh", "22.22"};
    Record* recp2 = new Record(recString2, &sch1, numAttrs1);
    
    string recString3[numAttrs2] = {"08", "chirag", "33"};
    Record* recp3 = new Record(recString3, &sch2, numAttrs2);
    string recString4[numAttrs2] = {"17", "manish", "44"};
    Record* recp4 = new Record(recString4, &sch2, numAttrs2);
    
    string recString5[numAttrs3] = { "431122", "beed"};
    Record* recp5 = new Record(recString5, &sch3, numAttrs3);
    string recString6[numAttrs3] = { "411004", "pune"};
    Record* recp6 = new Record(recString6, &sch3, numAttrs3);
    string recString7[numAttrs3] = { "431100", "abad"};
    Record* recp7 = new Record(recString7, &sch3, numAttrs3);
    
    //adding records to relations
    r1.addRecord(*recp1);
    //for (int i = 0; i < 10; i++) r1.addRecord(*recp1);
    r1.addRecord(*recp2);
    r1.addRecord(*recp2);
    r1.addRecord(*recp2);

    r2.addRecord(*recp3);
    r2.addRecord(*recp4);
    r2.addRecord(*recp3);
    r2.addRecord(*recp3);
    r2.addRecord(*recp4);
    
    r3.addRecord(*recp5);
    r3.addRecord(*recp6);
    r3.addRecord(*recp7);
    

//End- Mubase initialisation
    
    //Evaluation
    cout<<"\nQuery Result:\n\n";
    int rno = 1;
    root->open();
    Record* rec;
    
    rec = root->next();
    while(rec){
        cout<<"Record["<<rno<<"]: ";
        rec->print();
        rec = root->next();
        rno++;
    }
    
    root->close();
    //End Evaluation
    
//close Mubase db
    DiskManager::closeDB("mydb.txt");    
    cout << "\nTHE END\n";
}

void PrintOps(PhysicalOp_t *phyOp, int ind){
    if(!phyOp) return;
    
    for(int i=0; i<ind; i++)
        cout<<"| ";
    phyOp->PrintName();
    cout<<endl;    
    int n = phyOp->NumInputs();

    for( int i = 0 ; i < n ; i++ ) {
        const PlanGroup_t *inpPG = phyOp->ActualInput(i);
        if( !inpPG ) 
            inpPG = phyOp->Input(i);
        assert(inpPG);

        Plan_t *inpPlan = inpPG->BestPlan();
        if( !inpPlan )
            inpPlan = inpPG->BestAlgPlan();
        else if( inpPlan->IsFailed() )
            inpPlan = inpPG->BestAlgPlan();
        assert(inpPlan);
        assert(!inpPlan->IsFailed());
        
        PrintOps(inpPlan->Root(), ind+1);
        //inpPlan->PrintPlan(indentLevel+1);
    }
    
}

