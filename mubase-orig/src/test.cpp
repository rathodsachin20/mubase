#include <iostream>
#include "bt.h"
using namespace std;

int main() {


    int i = 1;
    int tester, key, record;

    DiskManager::createDB("mydb.txt", 2000);
    StorageManager::initDB("mydb.txt");
    BPlusTree *bpTree1 = new BPlusTree("mydb.txt", "obj1", true);
    /* for(i=1;i<=2000;i++)
         bpTree1->insertEntry(i, i*10);	
     bpTree1->~BPlusTree();	
     */
    cin >> tester;
    while (tester != 4) {
        if (tester == 1) {
            cin >> key;
            cin >> record;
            bpTree1->insertEntry(key, record);
        }
        if (tester == 2) {
            cin >> key;
            bpTree1->deleteEntry(key);
        }
        if (tester == 3) {
            cin >> key;
            cout << "look up in main for " <<
                    key << ": " << bpTree1->lookUp(key) << endl << endl;
        }
        cin >> tester;

    }
    bpTree1->~BPlusTree();


    /*for(i=91;i<=95;i++)
            bpTree1->insertEntry(i, i*100);
    for(i=131;i<=135;i++)
            bpTree1->insertEntry(i, i*100);
     //cout<<"look up in main for "<<i*10<<": "<<bpTree1->lookUp(160)<<endl<<endl;	
//bpTree1->insertEntry(11, 3000);
//bpTree1->insertEntry(8, 4000);
//bpTree1->insertEntry(3, 5000);
    //for(i=25;i>=1;i--)
    bpTree1->deleteEntry(140);
    bpTree1->deleteEntry(150);
    bpTree1->deleteEntry(160);
    bpTree1->deleteEntry(170);
    bpTree1->deleteEntry(180);
    bpTree1->deleteEntry(133);
    bpTree1->deleteEntry(134);
    bpTree1->deleteEntry(135);
    bpTree1->deleteEntry(132);
    //bpTree1->deleteEntry(14*10);
    //bpTree1->deleteEntry(5*10);
    ///bpTree1->deleteEntry(6*10);
/*	//bpTree1->deleteEntry(18*10);
    //bpTree1->deleteEntry(11*10);
cout<<"look up in main for "<<i*10<<": "<<bpTree1->lookUp(91)<<endl<<endl;
cout<<"look up in main for "<<i*10<<": "<<bpTree1->lookUp(92)<<endl<<endl;
cout<<"look up in main for "<<i*10<<": "<<bpTree1->lookUp(93)<<endl<<endl;
cout<<"look up in main for "<<i*10<<": "<<bpTree1->lookUp(94)<<endl<<endl;
cout<<"look up in main for "<<i*10<<": "<<bpTree1->lookUp(95)<<endl<<endl;
cout<<"look up in main for "<<i*10<<": "<<bpTree1->lookUp(80)<<endl<<endl;
cout<<"look up in main for "<<i*10<<": "<<bpTree1->lookUp(90)<<endl<<endl;
//bpTree1->deleteEntry(150);
    
    //for(i=1;i<=45;i++)
    cout<<"look up in main for "<<i*10<<": "<<bpTree1->lookUp(i*10)<<endl<<endl;	
//cout<<"look up in main for "<<i*10<<": "<<bpTree1->lookUp(12*10)<<endl<<endl;
//cout<<"look up in main for "<<i*10<<": "<<bpTree1->lookUp(13*10)<<endl<<endl;
  //  cout<<"look up in main for "<<i*10<<": "<<bpTree1->lookUp(15*10)<<endl<<endl;
//cout<<"look up in main for "<<i*10<<": "<<bpTree1->lookUp(17*10)<<endl<<endl;	
//bpTree1->insertEntry(2);
/*bpTree1->insertEntry(4, 200);
bpTree1->insertEntry(24, 300);
bpTree1->insertEntry(8, 400);
bpTree1->insertEntry(3, 500);

bpTree1->deleteEntry(3);
//bpTree1->insertEntry(25, 1000);
	
bpTree1->deleteEntry(20);
bpTree1->deleteEntry(24);

bpTree1->deleteEntry(8);
cout<<"look up in main for 4: "<<bpTree1->lookUp(4)<<endl;
cout<<"look up in main for 8: "<<bpTree1->lookUp(8)<<endl;
//cout<<"look up in main for 16: "<<bpTree1->lookUp(3)<<endl;
//cout<<"look up in main for 20: "<<bpTree1->lookUp(20)<<endl;
//cout<<"look up in main for 24: "<<bpTree1->lookUp(24)<<endl;
//cout<<bpTree1->lookUp(1)<<endl;    
bpTree1->~BPlusTree();*/
    //bpTree2->~BPlusTree();
    return 0;
}
