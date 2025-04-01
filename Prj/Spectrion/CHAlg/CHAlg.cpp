// CHAlg.cpp : Defines the entry point for the DLL application.
//
#include "pch.h"
#include "stdafx.h"
#include "CHAlg.h"
#include "..\CompressAlg\CompressAlg.h"
TCHHeader Header;

HANDLE CHFile=0;
short PacketType = 1;


//    void* (* GetCompressed)(int* Data, int DataLength, int* PacketLength, int PacketType);
//=================================================================================
//=================================================================================
//=================================================================================
CHALG_API int BeginChWriting(LPCWSTR ChFileName, int DataLength){

/*******************************/
	Header.Desc[0]=0;
/*******************************/

  DWORD WrittenBytes;
   CHFile = CreateFile(ChFileName,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
  if(CHFile!=INVALID_HANDLE_VALUE){
  
   WriteFile(CHFile,(void*)"CCH1",4,&WrittenBytes,NULL);
  
   WriteFile(CHFile,(void*)&(Header.A0),4,&WrittenBytes,NULL);
   WriteFile(CHFile,(void*)(&(Header.T0)),4,&WrittenBytes,NULL);
   WriteFile(CHFile,(void*)(&(Header.M1)),4,&WrittenBytes,NULL);
   WriteFile(CHFile,(void*)(&(Header.T1)),4,&WrittenBytes,NULL);
   WriteFile(CHFile,(void*)(&(Header.M2)),4,&WrittenBytes,NULL);
   WriteFile(CHFile,(void*)(&(Header.T2)),4,&WrittenBytes,NULL);
  
   WriteFile(CHFile,(void*)Header.Desc,(DWORD)(strlen(Header.Desc)+1),&WrittenBytes,NULL);
  
   WriteFile(CHFile,(void*)&(Header.StartTime),4,&WrittenBytes,NULL);
   WriteFile(CHFile,(void*)(&(DataLength)),4,&WrittenBytes,NULL);
   WriteFile(CHFile,(void*)(&(Header.SamplingRate)),2,&WrittenBytes,NULL);
   WriteFile(CHFile,(void*)(&(Header.ExtNumber)),2,&WrittenBytes,NULL);
   WriteFile(CHFile,(void*)(&(Header.ExtFreq)),4,&WrittenBytes,NULL);
   WriteFile(CHFile,(void*)(&(Header.NoiseLevel)),4,&WrittenBytes,NULL);
   WriteFile(CHFile,(void*)(&(Header.PeakBounds)),2,&WrittenBytes,NULL);
   WriteFile(CHFile,(void*)(&(Header.StartMass)),2,&WrittenBytes,NULL);
  }else{
   CHFile = 0; return 1;
  }
   return 0;

};
//---------------------------------------------------------------------------------
CHALG_API int BeginChWritingEx(LPCWSTR ChFileName, int DataLength, TCHHeader* pHead)
{
/*******************************/
	pHead->Desc[0]=0;
/*******************************/

  DWORD WrittenBytes;
   CHFile = CreateFile(ChFileName,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
  if(CHFile!=INVALID_HANDLE_VALUE){
  
   WriteFile(CHFile,(void*)"CCH1",4,&WrittenBytes,NULL);
  
   WriteFile(CHFile,(void*)&(pHead->A0),4,&WrittenBytes,NULL);
   WriteFile(CHFile,(void*)(&(pHead->T0)),4,&WrittenBytes,NULL);
   WriteFile(CHFile,(void*)(&(pHead->M1)),4,&WrittenBytes,NULL);
   WriteFile(CHFile,(void*)(&(pHead->T1)),4,&WrittenBytes,NULL);
   WriteFile(CHFile,(void*)(&(pHead->M2)),4,&WrittenBytes,NULL);
   WriteFile(CHFile,(void*)(&(pHead->T2)),4,&WrittenBytes,NULL);
  
   WriteFile(CHFile,(void*)pHead->Desc,(DWORD)(strlen(pHead->Desc)+1),&WrittenBytes,NULL);
  
   WriteFile(CHFile,(void*)&(pHead->StartTime),4,&WrittenBytes,NULL);
   WriteFile(CHFile,(void*)(&(DataLength)),4,&WrittenBytes,NULL);
   WriteFile(CHFile,(void*)(&(pHead->SamplingRate)),2,&WrittenBytes,NULL);
   WriteFile(CHFile,(void*)(&(pHead->ExtNumber)),2,&WrittenBytes,NULL);
   WriteFile(CHFile,(void*)(&(pHead->ExtFreq)),4,&WrittenBytes,NULL);
   WriteFile(CHFile,(void*)(&(pHead->NoiseLevel)),4,&WrittenBytes,NULL);
   WriteFile(CHFile,(void*)(&(pHead->PeakBounds)),2,&WrittenBytes,NULL);
   WriteFile(CHFile,(void*)(&(pHead->StartMass)),2,&WrittenBytes,NULL);
  }else{
   CHFile = 0; return 1;
  }
   return 0;

};
//---------------------------------------------------------------------------------
CHALG_API int ChWriteMS(int* Data, int DataLength, int TimeStamp){
  if(CHFile){
    DWORD WrittenBytes;
    int PacketLength=0, TIC=0;
    void* Packet = NULL;
   try{
    WriteFile(CHFile,(void*)&TimeStamp,4,&WrittenBytes,NULL);
    WriteFile(CHFile,(void*)&PacketType,2,&WrittenBytes,NULL);
    Packet = GetCompressed(Data, DataLength, &PacketLength, PacketType);
	for(int i=0;i<DataLength;i++)TIC+=Data[i];
    WriteFile(CHFile,(void*)&(TIC),4,&WrittenBytes,NULL);
    WriteFile(CHFile,(void*)&(PacketLength),4,&WrittenBytes,NULL);
    WriteFile(CHFile,Packet,PacketLength*4,&WrittenBytes,NULL);
   }catch(...){return 0;};
    return PacketLength;
  }else return 0;
};
//---------------------------------------------------------------------------------
CHALG_API int EndChWriting(){
    if(CHFile) CloseHandle(CHFile);
    CHFile=0;
	return 0;
};

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	HMODULE CompressLib;

	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		CompressLib = LoadLibrary(L"CompressAlg.dll");
		//GetCompressed = (void* (*)(int* Data, int DataLength, int* PacketLength, int PacketType))GetProcAddress(CompressLib,"GetCompressed");
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
    return TRUE;
}