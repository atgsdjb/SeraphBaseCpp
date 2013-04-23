#include<iostream>
#include"date.h"
#include"histogram.h"
#include"exception.h"
#include"int_node.h"
#include"int_set.h"
#include<vector>
#include<iterator>
using namespace std;
using namespace Seraphim;
void f(vector<int>& v){
	vector<int>::iterator  pi = v.begin();
	while(pi != v.end()){
		cout<<*pi++<<endl;

	}

}
int main(int argc,char** argv){
	
	int i,j ;
	IntSet *s1 = new IntSet(0);
	for(i=0;i<50;i+=2){
	s1->insert(i);
	}
	vector<int> *v = s1->getAll();
	f(*v);
	cout<<"-----------------------------"<<endl;
	IntSet *s2 = new IntSet(1);
	for(j=1;j<50;j+=5){
		s2->insert(j);
	}
	cout<<"-----------------------------"<<endl;
	v = s2->getAll();
	f(*v);
	s1->f_and(s2);
	vector<int> *vo = s1->getAll();
	f(*vo);
	cin>>i;
	return 0;
}
