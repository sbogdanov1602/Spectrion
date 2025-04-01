// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the CHALG_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// CHALG_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef CHALG_EXPORTS
#define CHALG_API __declspec(dllexport)
#else
#define CHALG_API __declspec(dllimport)
#endif

struct TCHHeader{
   float A0;
   float T0;
   float M1;
   float T1;
   float M2;
   float T2;
   char Desc[2000]; 
   long StartTime;
   int DataLength; 
   short SamplingRate;
   short ExtNumber;
   int ExtFreq; 
   int NoiseLevel;
   short PeakBounds;
   short StartMass; 
};

CHALG_API int BeginChWriting(LPCWSTR ChFileName, int);
CHALG_API int BeginChWritingEx(LPCWSTR ChFileName, int, TCHHeader*);
CHALG_API int ChWriteMS(int* Data, int Length, int TimeStamp);
CHALG_API int EndChWriting();
