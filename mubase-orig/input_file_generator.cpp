#include <iostream>
using namespace std;
int main (){
	
	for(int i=2;i <= 60001;i=i+3){
		cout<<"1\t"<<i+1<<"\t"<<(i+1)*10<<endl;
		cout<<"1\t"<<i<<"\t"<<i*10<<endl;
		cout<<"1\t"<<i-1<<"\t"<<(i-1)*10<<endl;
	}
	for(int i=2000;i <= 52000;i=i+3){
		cout<<"2\t"<<i+1<<endl;
		cout<<"2\t"<<i<<endl;
		cout<<"2\t"<<i-1<<endl;
	}
	/*for(int i=2000;i <= 52000;i=i+3){
		cout<<"1\t"<<i+1<<"\t"<<(i+1)*10<<endl;
		cout<<"1\t"<<i<<"\t"<<i*10<<endl;
		cout<<"1\t"<<i-1<<"\t"<<(i-1)*10<<endl;
	}*/
	for(int i=1;i <= 60001;i=i+1){
		cout<<"3\t"<<i<<endl;
	}
	cout<<"4"<<endl;
	return 0;
}
