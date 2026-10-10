#ifdef METROLOGY_EXPORTS
#define METROLOGY_API __declspec(dllexport)
#else
#define METROLOGY_API __declspec(dllimport)
#endif

#include "../metrology/pd_alg/pd_data.h"

typedef struct peakinside {
	int Extrem;  //В качестве значений фактические индескы в массиве Data
	int Height;
	int LeftBound;
	int RightBound;
	int IndLeftMin;
	int IndRightMin;
	double Center;
	double Value;
	double SKO;
	//Ширинные параметры пика, при включенной калибровке имеют размерность массы
	double FWRL; //Ширина пика на уровне разрешения
	double ExactRightBound;  //Точн.значение правой границы по уровню разрешения
	double ExactLeftBound;   //Точн.значение левой границы по уровню разрешения
	double Resolution;
} TPeakInside;

class PDAlg 
{
	static TPeakInside* Peaks;
	static TPeakInside Tmp;

	static int* Data;
	static int* Extrems;
	static int* ExtremInd;

	static bool DispFiltered;

	static int NoiseLevel;
	static int DRNoiseLevel;
	static int DRLimit;
	static int PeakCount;
	static int FDataLength;
	static int CurrentDataLength;
	static int NoiseRate;
	static int PeakWidth;
	static int ExtremCount;
	static int LBound;
	static int RBound;

	static double PeakBounds;
	static double Resolution;
	static double DispRate;

private:
	static void NoiseLevelDetect(bool bDetect);
	static double PeakTimeResolution(int Peak);

public:
	static METROLOGY_API std::vector<TPeak> PeaksDetecting(
		int* RawData,
		int RawDataLength,
		double GSamplesRate,
		TPDAlgParams* PDAlgParams);
};