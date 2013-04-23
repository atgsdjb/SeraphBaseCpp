#include<iostream>
using namespace std;
class F{
public:
 virtual ~F(){ cout<<"F"<<endl;}
};
class S : public F{
public:
 ~S(){ cout<<"S"<<endl;}
};
