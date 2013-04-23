#include"int_set.h"
#include"seraphim_global.h"
#include<vector>
#include<stdio.h>
using namespace std;
namespace Seraphim{ 
	/************************************************************************/
	/*ÕýÏò µÝ¹é±éÀú                                                                     */
	/************************************************************************/
	static void traversal(vector<int>& v,IntNode node){
		if(node.getLeft()!=null){
			traversal(v,*node.getLeft());
		}
		v.emplace_back(node.getValue());
		if(node.getRight()!=null){
			traversal(v,*node.getRight());
		}
	}  
	static void  deleteTree(IntNode* node){
		if(node->getLeft()!=null){
			deleteTree(node->getLeft());
		}
		if(node->getRight()!=null){
			deleteTree(node->getRight());
		}
		delete node;

	}
	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	static void insertInt(IntNode& node ,int v){
		if(node==v){
			return ;
		}
		bool b = node<v;
		if(b){
			if(node.getRight()==NULL){
				IntNode* n = new IntNode(v);
				node.setRightNode(n);
			}else{
				insertInt(*node.getRight(),v);
			}

		}else{
			if(node.getLeft()==NULL){
				IntNode* n = new IntNode(v);
				node.setLeftNote(n);
			}else{
				insertInt(*node.getLeft(),v);
			}
		}
	}
	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	static bool findInt(IntNode *node,int v){
		bool result = false;
		if(node==0){
			return false;
		}
		if(*node==v){
			return true;
		}
	IntNode *n;
	if(*node <v){
		n = node->getRight();
	}else{
		n=  node->getLeft();
	}
	return findInt(n,v);
	}
	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	IntSet::IntSet(int v){
		this->root = new IntNode(v);
		this->size=1;
	}
	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	IntSet::~IntSet(){
		deleteTree(root);
	}
	/**
	**/
	void IntSet::insert(int v){
		this->size++;
		if(root == NULL){
			root = new IntNode(v);
			return;
		}
		insertInt(*root,v);
	}
	bool IntSet::contain(int v){
		return findInt(root,v);
	}
	vector<int>* IntSet::getAll(){
		vector<int> *v = new vector<int>();
		traversal(*v,*root);
		return v;
	}
	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//IntSet& IntSet::f_and(IntSet& set){
	//	vector<int> *pv=set.getAll();
	//	vector<int>::iterator i = pv->begin();
	//	size = 0;
	//	IntNode *node=0;
	//	if(i!=pv->end()){
	//		node = new IntNode(*i++);
	//	}
	//	for(i;i!=pv->end();++i){
	//		if(contain(*i)){
	//			size++;
	//			insertInt(*node,*i);
	//		}
	//	}
	//	deleteTree(root);
	//	root = node;
	//	return *this;
	//}
	IntSet* IntSet::f_and(IntSet* set){
		/*****seraphim3*****/
		bool b = contain(12);
		printf("-----bool-----------%i",b);
		vector<int> *pv=set->getAll();
		vector<int>::iterator i = pv->begin();
		size = 0;
		//IntNode *node=0;
		if(i==pv->end()){
			return 0;
		}
		IntNode *node = new IntNode(*i++);
		for(i;i!=pv->end();++i){
			if(contain(*i)){
				size++;
				insertInt(*node,*i);
			}
		}
		deleteTree(root);
		root = node;
		return this;
	}
	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	IntSet* IntSet::f_or(IntSet *set){
		vector<int>  *s = set->getAll();
		vector<int>::iterator i = s->begin();
		while( i!=s->end()){
				insert(*i++);
		}
		delete s;
		return this;
	}
};
