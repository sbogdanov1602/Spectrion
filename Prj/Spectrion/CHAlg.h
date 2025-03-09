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
#pragma once

struct TCHHeader {
	float A0;
	float T0;
	float M1;
	float T1;
	float M2;
	float T2;
	char Desc[2000];
	long StartTime;
	int DataLength;
	short SamplingRateMegaSamples;
	short ExtNumber;
	int ExtFreq;
	int NoiseLevel;
	short PeakBounds;
	short StartMass;
};

class CCHHandle {
public:
	static int BeginChWriting(const char* ChFileName, int);
	static int BeginChWritingEx(const char* ChFileName, int, TCHHeader*);
	static int ChWriteMS(int* Data, int Length, int TimeStamp);
	static int EndChWriting();
	static CStringArray* GetAllFilesNames(CString dir, CString ext);
};
