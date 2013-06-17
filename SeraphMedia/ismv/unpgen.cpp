#include "unpgen.h"

static const int MAX_OBJ = 100;
int g_cur_obj = 0;

BYTE g_type = 0;
int g_subtype = 0;

char * headArray[MAX_OBJ] = {0};
unsigned int headoffsetArray[MAX_OBJ] = {0};
unsigned int headlenArray[MAX_OBJ] = {0};

char * dataArray[MAX_OBJ] = {0};
unsigned int dataoffsetArray[MAX_OBJ] = {0};
unsigned int datalenArray[MAX_OBJ] = {0};

char * unpArray[MAX_OBJ] = {0};
unsigned int unpoffsetArray[MAX_OBJ] = {0};
unsigned int unplenArray[MAX_OBJ] = {0};

char * packArray[MAX_OBJ] = {0};
unsigned int packoffsetArray[MAX_OBJ] = {0};
unsigned int packlenArray[MAX_OBJ] = {0};

unsigned int packnoArray[MAX_OBJ] = {0};
int init()
{
	if(g_cur_obj > MAX_OBJ)
		return -1;

	headlenArray[g_cur_obj] = 60 * 1024;
	headArray[g_cur_obj] = new char[headlenArray[g_cur_obj]];
	headoffsetArray[g_cur_obj] = 0;
	
	datalenArray[g_cur_obj] = 6 * 1024 * 1024;
	dataArray[g_cur_obj] = new char[datalenArray[g_cur_obj]];
	dataoffsetArray[g_cur_obj] = 0;

	unplenArray[g_cur_obj] = 1 * 1024 * 1024;
	unpArray[g_cur_obj] = new char[unplenArray[g_cur_obj]];
	unpoffsetArray[g_cur_obj] = 0;

	packlenArray[g_cur_obj] = 1 * 1024 * 1024;
	packArray[g_cur_obj] = new char[packlenArray[g_cur_obj]];
	packoffsetArray[g_cur_obj] = 0;
	
	return g_cur_obj++;
}
bool writeunp(int obj,char * unpbuf,unsigned int * unplen,int packlen = 5*1024)
{
	if(dataoffsetArray[obj] < packlen)
		return false;
	if(unpoffsetArray[obj] == 0 && dataoffsetArray[obj])
	{
		//clear
		memset(unpArray[obj],0,unplenArray[obj]);
		
		//unp head
		WEBTAG *webtag = (WEBTAG *)unpArray[obj];
		(*webtag).ver = 0x31080920;
		(*webtag).bitps = 0;
		(*webtag).crc = 0;
		(*webtag).datalen = WEBFILELEN - sizeof(WEBTAG);
		unpoffsetArray[obj] += sizeof(WEBTAG);
	
		//last pack
		if(packoffsetArray[obj] > 0)
		{
			memcpy(unpArray[obj] + unpoffsetArray[obj],&packoffsetArray[obj],sizeof(DWORD));
			unpoffsetArray[obj] += sizeof(DWORD);

			memcpy(unpArray[obj] + unpoffsetArray[obj],packArray[obj],packoffsetArray[obj]);
			unpoffsetArray[obj] += packoffsetArray[obj];

			packoffsetArray[obj] = 0;
		}

		//head
		if(headoffsetArray[obj] > 0)
		{
			memcpy(unpArray[obj] + unpoffsetArray[obj],&headoffsetArray[obj],sizeof(DWORD));
			unpoffsetArray[obj] += sizeof(DWORD);

			memcpy(unpArray[obj] + unpoffsetArray[obj],headArray[obj],headoffsetArray[obj]);
			unpoffsetArray[obj] += headoffsetArray[obj];
		}
	}

	
	while(((unpoffsetArray[obj] + 100) < WEBFILELEN) && (dataoffsetArray[obj] > packlen))
	{

			int len = 0;
			if(packlen > 0)
				len = packlen;
			else
				len = dataoffsetArray[obj];
		
		
			packoffsetArray[obj] = 0;
			memset(packArray[obj],0,packlenArray[obj]);
			
			
			TPackageDataEx * packageDataEx_ptr = (TPackageDataEx *)(packArray[obj] + packoffsetArray[obj]);
			packageDataEx_ptr->Data = NULL;
			packageDataEx_ptr->ID = packnoArray[obj];
			packnoArray[obj]++;
			packageDataEx_ptr->Len = sizeof(PACKStruct) + sizeof(PACK_LIVEStruct) + len;
			packageDataEx_ptr->mediaType = 1;
			packageDataEx_ptr->SendTime = UGetTime();

			packoffsetArray[obj] += sizeof(TPackageDataEx);

			PACKStruct * pACKStruct_ptr = (PACKStruct *)(packArray[obj] + packoffsetArray[obj]);
			pACKStruct_ptr->tag[0] = '@';
			pACKStruct_ptr->tag[1] = '2';
			pACKStruct_ptr->len = sizeof(PACK_LIVEStruct) + len;
			pACKStruct_ptr->checksum = 0;

			packoffsetArray[obj] += sizeof(PACKStruct);
			
			PACK_LIVEStruct * pACK_LIVEStruct_ptr = (PACK_LIVEStruct *)(packArray[obj] + packoffsetArray[obj]);
			pACK_LIVEStruct_ptr->type = g_type;
			if(g_type == TYPE_X264)
				pACK_LIVEStruct_ptr->subtype = SUBTYPE_X264_DATA3;

			if(g_type == TYPE_MMS)
				pACK_LIVEStruct_ptr->subtype = SUBTYPE_MMS_DATA3;


			packoffsetArray[obj] += sizeof(PACK_LIVEStruct);


			memcpy(packArray[obj] + packoffsetArray[obj],dataArray[obj],len);

			packoffsetArray[obj] += len;
			
			pACKStruct_ptr->checksum = in_cksum((unsigned char *)pACK_LIVEStruct_ptr,pACKStruct_ptr->len);
				
			memmove(dataArray[obj],dataArray[obj] + len,dataoffsetArray[obj] - len);
			dataoffsetArray[obj] -= len;



			int unpuselen = packoffsetArray[obj];
			if((WEBFILELEN - unpoffsetArray[obj] - 2 * sizeof(DWORD)) < packoffsetArray[obj])
			{
				unpuselen = WEBFILELEN - unpoffsetArray[obj] - 2 * sizeof(DWORD);
			}


			memcpy(unpArray[obj] + unpoffsetArray[obj],&unpuselen,sizeof(DWORD));
			unpoffsetArray[obj] += sizeof(DWORD);

			memcpy(unpArray[obj] + unpoffsetArray[obj],packArray[obj],unpuselen);
			unpoffsetArray[obj] += unpuselen;


			memmove(packArray[obj],packArray[obj] + unpuselen,packoffsetArray[obj] - unpuselen);
			

			if(unpuselen != packoffsetArray[obj])
			{
				unpoffsetArray[obj] = WEBFILELEN;
			}

			packoffsetArray[obj] -= unpuselen;
			
	}

	if((unpoffsetArray[obj] + 100) > WEBFILELEN)
	{	
		//todo unp head
		WEBTAG *webtag = (WEBTAG *)unpArray[obj];
		//(*webtag).bitps = Speed(obj,0);
		(*webtag).datalen = WEBFILELEN - sizeof(WEBTAG);
		(*webtag).timestamp = UGetTimeSec();
		(*webtag).crc = CalcCrc32Data(unpArray[obj],WEBFILELEN);
		
		//copy to unpbuf

		memcpy(unpbuf,unpArray[obj],WEBFILELEN);
		*unplen = WEBFILELEN;

		unpoffsetArray[obj] = 0;
		return true;
	}
	return false;
}
bool genunp(int obj,BYTE type,int subtype,char * buf,unsigned int len,char * unpbuf,unsigned int * unplen)
{

	if(len > 0)
	{

		if(subtype == SUBTYPE_X264_HEAD3 || subtype == SUBTYPE_MMS_HEAD3)
		{
			//insert last data.

			bool ret = writeunp(obj,unpbuf,unplen,0);

			dataoffsetArray[g_cur_obj] = 0;
			headoffsetArray[g_cur_obj] = 0;

			g_type = type;
			packnoArray[obj] = UGetTime(); 
			
			//proc head.
			
			packoffsetArray[obj] = 0;
			memset(packArray[obj],0,packlenArray[obj]);
			
			
			TPackageDataEx * packageDataEx_ptr = (TPackageDataEx *)(packArray[obj] + packoffsetArray[obj]);
			packageDataEx_ptr->Data = NULL;
			packageDataEx_ptr->ID = packnoArray[obj];
			packnoArray[obj]++;
			packageDataEx_ptr->Len = sizeof(PACKStruct) + sizeof(PACK_LIVEStruct) + len;
			packageDataEx_ptr->mediaType = 0;
			packageDataEx_ptr->SendTime = UGetTime();

			packoffsetArray[obj] += sizeof(TPackageDataEx);

			PACKStruct * pACKStruct_ptr = (PACKStruct *)(packArray[obj] + packoffsetArray[obj]);
			pACKStruct_ptr->tag[0] = '@';
			pACKStruct_ptr->tag[1] = '2';
			pACKStruct_ptr->len = sizeof(PACK_LIVEStruct) + len;
			pACKStruct_ptr->checksum = 0;

			packoffsetArray[obj] += sizeof(PACKStruct);
			
			PACK_LIVEStruct * pACK_LIVEStruct_ptr = (PACK_LIVEStruct *)(packArray[obj] + packoffsetArray[obj]);
			pACK_LIVEStruct_ptr->type = type;
			pACK_LIVEStruct_ptr->subtype = subtype;


			packoffsetArray[obj] += sizeof(PACK_LIVEStruct);


			memcpy(packArray[obj] + packoffsetArray[obj],buf,len);

			packoffsetArray[obj] += len;
			
			pACKStruct_ptr->checksum = in_cksum((unsigned char *)pACK_LIVEStruct_ptr,pACKStruct_ptr->len);
				




			memcpy(headArray[obj],packArray[obj],packoffsetArray[obj]);
			headoffsetArray[obj] = packoffsetArray[obj];

			packoffsetArray[obj] = 0;

			return ret;
		}

		if(subtype == SUBTYPE_X264_DATA3 || subtype == SUBTYPE_MMS_DATA3)
		{
			memcpy(dataArray[obj] + dataoffsetArray[obj],buf,len);
			dataoffsetArray[obj] += len;

			if(dataoffsetArray[obj] > 3*1024*1024)
			{
				int z = 0;
				z++;
			}
		}
	}
	
	return writeunp(obj,unpbuf,unplen);
}
int stop(int obj)
{
	if(headlenArray[obj] > 0)
		delete [] headArray[obj];

	headArray[obj] = NULL;
	headlenArray[obj] = 0;
	headoffsetArray[obj] = 0;

	if(datalenArray[obj] > 0)
		delete [] dataArray[obj];

	dataArray[obj] = NULL;
	datalenArray[obj] = 0;
	dataoffsetArray[obj] = 0;

	if(unplenArray[obj] > 0)
		delete [] unpArray[obj];

	

	unpArray[obj] = NULL;
	unplenArray[obj] = 0;
	unpoffsetArray[obj] = 0;


	if(packlenArray[obj] > 0)
		delete [] packArray[obj];

	packlenArray[obj] = 0;
	packArray[obj] = NULL;
	packoffsetArray[obj] = 0;

	return 0;
}