#pragma once

typedef struct _peak
{
   int Extrem;  //index of apex point in the incompressed raw data array
   int Height;  //value of apex point in the incompressed raw data array
   double Center;  //exact centroid
   double Value;   //integral intensity of peak
   double FWRL; //full width at the specified resolution level
   double Resolution; //peak resolution
   double RightBound;  //exact value of right bound at the level of resolution
   double LeftBound;   //exact value of left bound at the level of resolution
} TPeak;


typedef struct pdalgprm{
	int bAutoNoise;
	int iNoiseLevel;
	double dPeaksBounds;
	int bAllData;
	int iLeftLim{};	// IGNORED if bAllData = TRUE
	int iRightLim{};  // IGNORED if bAllData = TRUE
	int iMaxPeakCount; // IGNORED if iMaxPeakCount = 0
	int iFilterWinCount; // no filtration if==0
	int iDRLimit; // Factor of dynamic range limit
	double dFilterUpLimit{}; // IGNORED if iFilterWinCount==0
	double dFilterLowLimit{}; // IGNORED if iFilterWinCount==0
	int iNoiseRate{};

	pdalgprm()
	{
		bAutoNoise=true;
		iNoiseLevel=0;
		dPeaksBounds=0.5;
		bAllData=true;
		iFilterWinCount=0;
        iMaxPeakCount=0;
        iDRLimit=0;
		iNoiseRate = 3;
	}
} TPDAlgParams;

