#ifndef _SERAPHIM_DATE
#define _SERAPHIN_DATE
namespace Seraphim
{
    enum month
    {
      January = 1,
      February, March, April, May, June, July, August, September, October, November,December

    };
  class Date
  {
   private:
   int y;
   month m;
   int d;
   bool isIntercalary(int yy);
   public:
    Date(int yy=0,month mm=month(0),int dd=0);
    int getDay(){return d;};
    int getYer(){return y;};
    month getMonth(){return m;};
  };
  class BadDate{};

};
#endif