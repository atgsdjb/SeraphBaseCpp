//#define DEBUG 
#include<cassert>
#include<iostream>
#include<vector>
#include"pthread.h"
//
#include"mp4_creater.h"
#include"track_param.h"
#include"sync_buf.h"


using namespace std;
using namespace Seraphim;

static void* encodeTask(void* p){


}



int main(int argc,char* argv){
	char* name ="seraphim.mp4";
	vector<SyncBuffer*> buf_v;
	vector<STrackParam*> param_v;
	SyncBuffer * g_buf = new SyncBuffer;
	STrackParam *param = new SVideoTrackParm(90000,320,240,800000,15,60);
	buf_v.push_back(g_buf);
	param_v.push_back(param);
	SMp4Creater creater(name,60,param_v,buf_v);
	creater.startEncode();

	
return 0;
}