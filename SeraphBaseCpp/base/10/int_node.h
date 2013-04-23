#ifndef  _SERAPHIM_INT_NODE 
#define  _SERAPHIM_INT_NODE 
namespace Seraphim{
	class IntNode{
	private:
		int value;
		IntNode *pl;
		IntNode *pr;
	public:
		IntNode(int _value,IntNode *_pl=0,IntNode* _pr=0):value(_value),pl(_pl),pr(_pr){}
		void setLeftNote(IntNode* _node){pl=_node;}
		void setRightNode(IntNode* _node){pr=_node;}
		IntNode* getLeft(){return pl;}
		IntNode* getRight(){return pr;}
		bool operator>(IntNode* node){ return node->value >value?true:false;}
		bool operator<(IntNode* node){return node->value < value?true:false;}
		bool operator==(IntNode* node){return node->value==value?true:false;}
		int getValue(){return value;}
		friend bool operator>(IntNode node,int v){return node.value>v?true:false;}
		friend bool operator<(IntNode node,int v){return node.value <v?true:false;}
		friend bool operator==(IntNode node,int v){return node.value==v?true:false;}
		~IntNode();
	};
};
#endif
