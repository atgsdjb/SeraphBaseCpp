#include<cstdio>
#include<iostream>
#include"../Saac/SAudioMuxElement.h"
using namespace std;
using namespace Seraphim;
const char* fileName="d:\\1video\\4.latm";
uint8_t rBuf[0x1d4]={0};
#include<fstream>
int main(int argc,char** argv){
	FILE* file = fopen(fileName,"rb");
	int rLen = fread(rBuf,1,0x1d4,file);
	cout<<"-- read length = "<<rLen<<endl;
	int i=0;
	/*for(i;i<0x1d4;i++ ){
		printf("-----%d  = %x------\n",i,rBuf[i]);
	}
	cout<<"seraphim"<<endl;*/

	SBitReader *reader= new SBitReader(rBuf,0x1d4); 
	SAudioMuxElement audioMxElement(reader);
	ofstream of("d:\\1video\\latm.4");
	
	of<<audioMxElement<<endl;
	cin>>i;
	return 0;
}