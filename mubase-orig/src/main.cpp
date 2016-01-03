#include "rm.h"
#include "sort.h"
#include "y.tab.h"
#include "y.tab.cpp"
//#include "lex.yy.cpp"
using namespace std;

int main() {

    cout << "\n STARTING MUBASE\n";


    DiskManager::createDB("mydb.txt", 20);
    StorageManager::initDB("mydb.txt");
    DiskManager::openDB("mydb.txt");



    short int numAttrs = 3;
    short int attrType[3] = {1, 4, 1};
    char *attrName[3] = {"idiii", "namei", "cgpai"};
    short int sizeAttrs[3] = {4, 30, 4};

    Schema *abc;
    abc = new Schema(Relation::getSysObjSchema());
    //abc->print();
    Schema sch(numAttrs, attrType, attrName, sizeAttrs);
    cout << "Pritning Schema of sch:\n";
    sch.print();
    cout<<endl;

    //cout << "\nCreating Relation\n";

    Relation r("mydb.txt", "rel1", true, &sch);
    Relation r2("mydb.txt", "rel2", true, &sch);

    abc = Relation::readSchema("mydb.txt", "rel1");
    //abc->print();

    string recString1[3] = {"29", "sachin", "9"};
    Record* recp1 = new Record(recString1, &sch, 3);
    string recString2[3] = {"8", "chirag", "9"};
    Record* recp2 = new Record(recString2, &sch, 3);
    string recString3[3] = {"1", "ajaya", "9"};
    Record* recp3 = new Record(recString3, &sch, 3);
    string recString4[3] = {"30", "sameer", "9"};
    Record* recp4 = new Record(recString4, &sch, 3);

    int* id[4];
    id[0] = new int[2];
    id[1] = new int[2];
    id[2] = new int[2];
    id[3] = new int[2];
    id[0] = r.addRecord(*recp1);
    for (int i = 0; i < 10; i++)
        r.addRecord(*recp1);
    id[1] = r.addRecord(*recp2);
    id[1] = r.addRecord(*recp3);
    id[1] = r.addRecord(*recp4);

    id[2] = r2.addRecord(*recp1);
    id[2] = r2.addRecord(*recp1);
    id[2] = r2.addRecord(*recp2);
    id[2] = r2.addRecord(*recp3);
    id[3] = r2.addRecord(*recp4);
    //for (int i = 0; i < 4; i++)
    //    cout << " recid " << id[i][0] << " " << id[i][1] << endl << endl << endl;

    //r.deleteRecord(id[1]);
    //r.deleteRecord(id[0]);

    RIterator * ri;
    ri = r.openScan();
    ri->open();

    Record* recGot;
    recGot = new Record(*(ri->next()));
    
    cout<<"\nPrinting Records:"<<endl;
    while (recGot != NULL) {
        cout << "RECORD:";
        recGot->print();
        recGot = ri->next();
        // recGot = new Record(*));
    }
    ri->close();


    RIterator * ri2;
    ri2 = r2.openScan();
    ri2->open();
    recGot = new Record(*(ri2->next()));
    int j = 0;
    while (recGot != NULL) {
        j++;
        cout << "RECORD:" << j << ":";
        recGot->print();
        recGot = ri2->next();
        // recGot = new Record(*));
    }
    ri2->close();

    Predicate* p = new EqualPred("namei", "sachin");
    Predicate* p2 = new GreaterPred("idiii", "20");
    Predicate* p3 = new OrPredicate(p,p2);
    RIterator* pri;
    pri = r.openScan(p3);
    pri->open();

    cout<<"\nPrinting predicated record:"<<endl;
    recGot = new Record(*(pri->next()));
    while (recGot != NULL) {
        cout << "PREDICATED RECORD:";
        recGot->print();
        recGot = pri->next();
        // recGot = new Record(*));
    }
    pri->close();

    cout << "last block of relation = " << r.getLastWrittenBlockId() << endl;
    
    
    
    cout<<"Printing sorted Records\n"<<endl;
    
    Sort* s=new Sort("mydb.txt","rel2",1);

    char *dbName ="mydb.txt";
    char*relName ="rel2";
    int blockId = StorageManager::getFirstBlockId(dbName, relName );
            char * buff = BufferManager::getInstance(dbName)->pinPage(blockId);
            SlottedPage* sp=new SlottedPage(buff);
            cout<<"get Total slots"<<sp->getTotalNoOfSlots()<<endl;
            for(int i =0 ; i<sp->getTotalNoOfSlots();i++)
            {
                    int len;
                    char *recByte = sp->getRecord(i,len);
                    if(recByte == (char*)(-1))
                    continue;
                    Record* R = new Record (recByte , &sch , len);
                    R->print();
            }
     
     //yyparse();
    
     
    DiskManager::closeDB("mydb.txt");
    //exit(0);
    //int a;
    //cin>>a;
    
    cout << "\nTHE END\n";
     
    return 0;
}





