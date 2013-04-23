/************************************************************************/
/*                                                                      */
/************************************************************************/
#ifndef  _SERAPHIM_INT_SET
#define  _SERAPHIM_INT_SET
#include"int_node.h"
#include<vector>
using namespace std;
namespace Seraphim{
	class IntSet{
	private:
		IntNode* root;
		int size;
	public:
		IntSet(int v);
		IntSet():root(0),size(0){}
		~IntSet();
		int getSize(){return size;}
		bool contain(int v);
		IntSet* f_and(IntSet  *set);
		IntSet* f_or(IntSet *set);
		 void insert(int v);
		 vector<int>* getAll();
		 
	/*	
		friend IntSet& operator||(IntSet s1,IntSet s2);
		friend IntSet& operator&&(IntSet s1,IntSet s2);
*/
	};
};
#endif
