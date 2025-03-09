///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
//
// CHAlg.cpp contains a description of the CCHHandle class static methods
// 
// Author  :   Nikita Shulgin      :    ShulginNA@Lumex.ru
// Company :   Lumex Instruments   :    https://www.lumexinstruments.com/
//
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
//
//
//
//---------------------------------------------------------------------------------
// структура записи файла cch (псевдокод)
//---------------------------------------------------------------------------------
// 
// int buf[CSp::Qpoint]
// CCHHandle::BeginChWriting(ChFileName, CSp::Qpoint)
// for (int i = 0; i < CSp::Qplot; i++)
// {
// 		for (int j  = 0  i < CSp::Qpoint; i++)
// 				buf[j] = static_cast <int> (  CSp::pf[i *  CSp::Qpoint + j] )
// 		CCHHandle::ChWriteMS(buf, CSp::Qpoint, time);
// }
// CCHHandle::EndChWriting();
//
//---------------------------------------------------------------------------------
#include "pch.h"
#include <windows.h>
#include "CHAlg.h"
#include "CompressAlg.h"

TCHHeader Header;
HANDLE CHFile = 0;
short PacketType = 1;

//---------------------------------------------------------------------------------
//
// ChFileName - абсолютный (полный) путь до записываемого файла
//                                               пример :  C:\Projects\cch\test.cch
//
// DataLength - длинна массива суммированного спектра,
//      размер типа t_size = 32 бита (int, float) : в вашей программе : CSp::Qpoint
//
//---------------------------------------------------------------------------------

int CCHHandle::BeginChWriting(const char* ChFileName, int DataLength) {

	Header.Desc[0] = 0;

	DWORD WrittenBytes;

	CHFile = CreateFileA(ChFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (CHFile != INVALID_HANDLE_VALUE) {

		WriteFile(CHFile, (void*)"CCH1", 4, &WrittenBytes, NULL);

		WriteFile(CHFile, (void*)(&(Header.A0)), 4, &WrittenBytes, NULL);
		WriteFile(CHFile, (void*)(&(Header.T0)), 4, &WrittenBytes, NULL);
		WriteFile(CHFile, (void*)(&(Header.M1)), 4, &WrittenBytes, NULL);
		WriteFile(CHFile, (void*)(&(Header.T1)), 4, &WrittenBytes, NULL);
		WriteFile(CHFile, (void*)(&(Header.M2)), 4, &WrittenBytes, NULL);
		WriteFile(CHFile, (void*)(&(Header.T2)), 4, &WrittenBytes, NULL);

		WriteFile(CHFile, (void*)Header.Desc, (DWORD)(strlen(Header.Desc) + 1), &WrittenBytes, NULL);

		WriteFile(CHFile, (void*)&(Header.StartTime), 4, &WrittenBytes, NULL);
		WriteFile(CHFile, (void*)(&(DataLength)), 4, &WrittenBytes, NULL);
		WriteFile(CHFile, (void*)(&(Header.SamplingRateMegaSamples)), 2, &WrittenBytes, NULL);
		WriteFile(CHFile, (void*)(&(Header.ExtNumber)), 2, &WrittenBytes, NULL);
		WriteFile(CHFile, (void*)(&(Header.ExtFreq)), 4, &WrittenBytes, NULL);
		WriteFile(CHFile, (void*)(&(Header.NoiseLevel)), 4, &WrittenBytes, NULL);
		WriteFile(CHFile, (void*)(&(Header.PeakBounds)), 2, &WrittenBytes, NULL);
		WriteFile(CHFile, (void*)(&(Header.StartMass)), 2, &WrittenBytes, NULL);
	}
	else {
		CHFile = 0; return 1;
	}
	return 0;

};
//---------------------------------------------------------------------------------
// этот метод вам не нужен 
//---------------------------------------------------------------------------------
int CCHHandle::BeginChWritingEx(const char* ChFileName, int DataLength, TCHHeader* pHead) {
	//pHead->Desc[0] = 0;

	DWORD WrittenBytes;

	CHFile = CreateFileA(ChFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (CHFile != INVALID_HANDLE_VALUE) {

		WriteFile(CHFile, (void*)"CCH1", 4, &WrittenBytes, NULL);

		WriteFile(CHFile, (void*)&(pHead->A0), 4, &WrittenBytes, NULL);
		WriteFile(CHFile, (void*)(&(pHead->T0)), 4, &WrittenBytes, NULL);
		WriteFile(CHFile, (void*)(&(pHead->M1)), 4, &WrittenBytes, NULL);
		WriteFile(CHFile, (void*)(&(pHead->T1)), 4, &WrittenBytes, NULL);
		WriteFile(CHFile, (void*)(&(pHead->M2)), 4, &WrittenBytes, NULL);
		WriteFile(CHFile, (void*)(&(pHead->T2)), 4, &WrittenBytes, NULL);

		//auto _size = strlen(pHead->Desc) + 1;
		WriteFile(CHFile, (void*)pHead->Desc, (DWORD)(strlen(pHead->Desc) + 1), &WrittenBytes, NULL);

		WriteFile(CHFile, (void*)&(pHead->StartTime), 4, &WrittenBytes, NULL);
		WriteFile(CHFile, (void*)(&(DataLength)), 4, &WrittenBytes, NULL);
		WriteFile(CHFile, (void*)(&(pHead->SamplingRateMegaSamples)), 2, &WrittenBytes, NULL);
		WriteFile(CHFile, (void*)(&(pHead->ExtNumber)), 2, &WrittenBytes, NULL);
		WriteFile(CHFile, (void*)(&(pHead->ExtFreq)), 4, &WrittenBytes, NULL);
		WriteFile(CHFile, (void*)(&(pHead->NoiseLevel)), 4, &WrittenBytes, NULL);
		WriteFile(CHFile, (void*)(&(pHead->PeakBounds)), 2, &WrittenBytes, NULL);
		WriteFile(CHFile, (void*)(&(pHead->StartMass)), 2, &WrittenBytes, NULL);
	}
	else {
		CHFile = 0; return 1;
	}
	return 0;

};
//---------------------------------------------------------------------------------
//
// Data адрес на начало массива суммированного спектра
//
// DataLength - длинна массива суммированного спектра,
//      размер типа t_size = 32 бита (int, float) : в вашей программе : CSp::Qpoint
//
// TimeStamp - инкрементальное время начала записи суммированного спектра
//                                                                   в наносекундах 
//---------------------------------------------------------------------------------
int CCHHandle::ChWriteMS(int* Data, int DataLength, int TimeStamp) {
	if (CHFile) {
		DWORD WrittenBytes;
		int PacketLength = 0, TIC = 0;
		void* Packet = NULL;
		try {
			WriteFile(CHFile, (void*)&TimeStamp, 4, &WrittenBytes, NULL);
			WriteFile(CHFile, (void*)&PacketType, 2, &WrittenBytes, NULL);
			Packet = CompressAlg::GetCompressed(Data, DataLength, &PacketLength, PacketType);
			for (int i = 0; i < DataLength; i++)TIC += Data[i];
			WriteFile(CHFile, (void*)&(TIC), 4, &WrittenBytes, NULL);
			WriteFile(CHFile, (void*)&(PacketLength), 4, &WrittenBytes, NULL);
			WriteFile(CHFile, Packet, PacketLength * 4, &WrittenBytes, NULL);
		}
		catch (...)
		{ return 0; };
		return PacketLength;
	}
	else return 0;
};
//---------------------------------------------------------------------------------
int CCHHandle::EndChWriting() {
	if (CHFile)
		CloseHandle(CHFile);
	CHFile = 0;
	return 0;
};

CStringArray* CCHHandle::GetAllFilesNames(CString dir, CString ext)
{
	WIN32_FIND_DATA fileData;
	memset(&fileData, 0, sizeof(WIN32_FIND_DATA));
	HANDLE handle = FindFirstFile(dir + ext, &fileData);

	CStringArray* strArray = new CStringArray;

	while (handle != INVALID_HANDLE_VALUE)
	{
		strArray->Add(fileData.cFileName); // the problem is that the fileData.cFileName always contains "."

		if (FALSE == FindNextFile(handle, &fileData))
			break;
	}
	FindClose(handle);

	return strArray;
}