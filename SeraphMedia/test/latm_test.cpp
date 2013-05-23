#include<cstdio>
#include<iostream>
#include"../Saac/SAudioMuxElement.h"
using namespace std;
using namespace Seraphim;
const char* fileName="d:\\1video\\20130522_2.latm";
uint8_t rBuf[0x1d4]={0};
#include<fstream>
static int main2342(int argc,char** argv){
	FILE* file = fopen(fileName,"rb");
	int rLen = fread(rBuf,1,0x1d4,file);
	cout<<"-- read length = "<<rLen<<endl;
	int i=0;
	/*for(i;i<0x1d4;i++ ){
		printf("-----%d  = %x------\n",i,rBuf[i]);
	}
	cout<<"seraphim"<<endl;*/
	uint32_t len=0;
	SBitReader *reader= new SBitReader(rBuf,0x5725); 
	do{
		
		SAudioMuxElement audioMxElement(reader);
		len = audioMxElement.getRemainSize();
	}while(len >0);
	
	
	cin>>i;
	return 0;
}