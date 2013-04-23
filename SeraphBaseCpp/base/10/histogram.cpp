#include"histogram.h"
#include<iostream>
#include"exception.h"
using namespace std;
namespace Seraphim{
	Histogram::Histogram(float _min,float _max,int count):min(_min),max(_max),countSection(count){
		if(_min > _max)
			throw TDInvalidArgumentException();
		section = new float[count];
		sizeSection = (max-min)/countSection;

	}
	void Histogram::addValue(float value){
		if(value <min || value >= max)
			throw TDInvalidArgumentException();
		int d = (int)(value/sizeSection);
		++section[d];//  d+1 -1;[x,x+sizeSection)
	};
	void Histogram::removeValue(float value){
		if(value <min || value >= max)
			throw TDInvalidArgumentException();
		int d = (int)(value/sizeSection);
		if(++section[d] < 0)
			throw TDInvalidArgumentException();
	}
	void Histogram::printToConsole(){
	}
};
