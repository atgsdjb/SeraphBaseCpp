#include<iostream>
#include<fstream>
using namespace std;
class Sera{
	int i;
	char* name;
public:
	Sera():i(12),name("seraph"){};
	friend ofstream& operator<<(ofstream& out,Sera& s);
	friend ofstream& operator<<(ofstream& out,Sera* s){
		out<<"~~~~~~~~~~~~~new~~~~~~~~~~~~~~~~~~";
		operator<<(out,*s);
		return out;
	}
};
ofstream& operator<<(ofstream& out,Sera& s){
	out<<s.i<<"name="<<s.name;
	return out;
}
int main(int argc,char** argv){
	int i;
	ofstream o("ins2.tst");
	Sera *_s = new Sera() ;
	o<<_s;
	return 0;
}