#include"date.h"
namespace Seraphim{
bool Date::isIntercalary(int yy){
	if(yy%4 !=0)
	       return false;
	if(yy%100==0 && yy%400 != 0)
		return false;
	return true;	
	}
Date::Date(int yy,month mm,int dd):y(yy),m(mm),d(dd){
 bool intercalary = isIntercalary(yy);
 int maxDay;
 switch(mm){
	 case January:
	 case March:
	 case May:
	 case July:
	 case August:
	 case October:
	 case December:
		 maxDay=31;
		 break;
	 case April:
	 case June:
	 case September:
	 case November:
		 maxDay=30;
		 break;
	 case February:
		 if(intercalary){
			 maxDay=29;
		 }else{
			 maxDay=28;
		 }
	 default :
		 throw BadDate();

 }
 if( dd<0 || dd>maxDay)
	 throw BadDate();
}
};
