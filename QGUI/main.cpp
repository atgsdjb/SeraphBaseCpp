#include "qgui.h"
#include <QtGui/QApplication>
#include <opencv2\opencv.hpp>
#include <iostream>
#include <string>
using namespace cv;
using namespace std;
int main(int argc, char *argv[])
{
	//QApplication a(argc, argv);
	//QGUI w;
	cout<<"hello opencV"<<endl;
	Mat img = imread("D:\\pp.jpg");
	if(img.empty()){
		cout<<"error"<<endl;
	}
	imshow("HELLO OPENCV",img);
	//w.show();
	int i;
	cout<<"pass any key to exit"<<endl;
	cin>>i;
	return 0;//a.exec();
}
 