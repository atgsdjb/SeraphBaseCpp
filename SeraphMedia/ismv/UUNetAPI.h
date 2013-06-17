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


    uint32_t UUNET_DLL __cdecl CORE_init(char * param);                                    //��ʼ��������û�ã�
    void UUNET_DLL __cdecl CORE_quit();                                                    //�˳�
                                    //����        ChannelID               ChannelInfo               LocalInfo  
    void UUNET_DLL __cdecl CORE_play(uint32_t mode,char *pstr,uint32_t len,char *pstr2,uint32_t len2,char *pstr3,uint32_t len3);
    void UUNET_DLL __cdecl CORE_stop();                                                    //ֹͣ
    void UUNET_DLL __cdecl CORE_getdata(PPackageData t);                                   //����ý�����ݰ�
	void UUNET_DLL __cdecl CORE_getdatabyid(uint32_t packId, PPackageData t);              //��ȡ�ض����ŵ�ý�����ݰ�
    void UUNET_DLL __cdecl CORE_dumpdata();
    void UUNET_DLL __cdecl CORE_seekpack(uint32_t packNum);                                //seek
    void UUNET_DLL __cdecl CORE_getstatusEx(char * pdata,uint32_t len);                    //���״̬��
#ifdef __cplusplus
}
#endif

#endif //_UUNET_APL_H_
