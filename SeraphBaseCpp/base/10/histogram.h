#ifndef _SERAPHIM_HISTOGRAM
#define _SERAPHIM_HISTOGRAM
namespace Seraphim{
	class Histogram{
		private: float *section;
			 int countSection;
			 float min;
			 float max;
			 float sizeSection;
		public:
			 Histogram(float min,float max,int count);
			 void addValue(float value);
			 void removeValue(float value);
			 float* getAllSection(){return section;};
			 int  getCountSection(){return countSection;};
			 float getMin(){return min;}
			 float getMax(){return max;}
			 float getSizeSection(){return sizeSection;};
			 void printToConsole();
			 ~Histogram(){ delete section;}
	};


};
#endif
