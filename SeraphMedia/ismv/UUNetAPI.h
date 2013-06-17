#ifndef _UUNET_APL_H_
#define _UUNET_APL_H_

#define __cdecl
#define UUNET_DLL 


#ifdef __cplusplus
extern "C" {
#endif
//typedef unsigned int uint32_t;

typedef struct  TPackageData 
{
    uint32_t type;    //vod data
    uint32_t OffSet;  //vod file offset (out) live sendtime
    char * Data;      //data buffer pointer(in)
    uint32_t MaxLen;  //buffer max len(in)
    uint32_t ActLen;  //actual len (out)
    uint32_t ID;
}TPackageData,* PPackageData;


    uint32_t UUNET_DLL __cdecl CORE_init(char * param);                                    //初始化（参数没用）
    void UUNET_DLL __cdecl CORE_quit();                                                    //退出
                                    //播放        ChannelID               ChannelInfo               LocalInfo  
    void UUNET_DLL __cdecl CORE_play(uint32_t mode,char *pstr,uint32_t len,char *pstr2,uint32_t len2,char *pstr3,uint32_t len3);
    void UUNET_DLL __cdecl CORE_stop();                                                    //停止
    void UUNET_DLL __cdecl CORE_getdata(PPackageData t);                                   //弹出媒体数据包
	void UUNET_DLL __cdecl CORE_getdatabyid(uint32_t packId, PPackageData t);              //获取特定包号的媒体数据包
    void UUNET_DLL __cdecl CORE_dumpdata();
    void UUNET_DLL __cdecl CORE_seekpack(uint32_t packNum);                                //seek
    void UUNET_DLL __cdecl CORE_getstatusEx(char * pdata,uint32_t len);                    //获得状态串
#ifdef __cplusplus
}
#endif

#endif //_UUNET_APL_H_
