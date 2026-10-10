// PDAlg.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "pd_alg.h"

TPeakInside* PDAlg::Peaks = nullptr;
TPeakInside PDAlg::Tmp{};

int* PDAlg::Data = nullptr;
int* PDAlg::Extrems = nullptr;
int* PDAlg::ExtremInd = nullptr;

bool PDAlg::DispFiltered{};

int  PDAlg::PeakCount = {};
int  PDAlg::FDataLength = {0};
int  PDAlg::ExtremCount{0};
int  PDAlg::CurrentDataLength{};
int  PDAlg::NoiseRate{};
int  PDAlg::NoiseLevel;
int  PDAlg::DRNoiseLevel;
int  PDAlg::DRLimit{};
int  PDAlg::PeakWidth{};
int  PDAlg::LBound{};
int  PDAlg::RBound{};
double PDAlg::Resolution{};
double PDAlg::DispRate{};
double PDAlg::PeakBounds{};



//-------------------------------------------------------------------------------------
BOOL APIENTRY DllMain(HANDLE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		//FDataLength = 0;
		//Extrems = NULL;
		//ExtremInd = NULL;
		//Peaks = NULL;
		//ExtremCount = 0;
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
void PDAlg::NoiseLevelDetect(bool bDetect)
{
	//Ищем все эктремумы

	int i, j, Count = 0, DCount, DefeatedCount;
	double Sum = 0, Avg;
	int nMax = 0;
	//если объем данных вырос - увеличиваем объем памяти

   //При расчете порога используем данные всего масс-спектра, независимо от диапазона
	for (j = 1; j <= CurrentDataLength - 2; j++)
	{
		if (Data[j] > Data[j - 1] && Data[j] >= Data[j + 1] && (bDetect || Data[j] > NoiseLevel))
		{
			Extrems[Count] = Data[j];
			if (Data[j] > nMax) nMax = Data[j];
			ExtremInd[Count] = j;
			Sum += Data[j];
			Count++;
		}
	}
	ExtremCount = Count;
	DCount = Count;

	if (bDetect)
	{
		//Удаляем шумовые экстремумы
		//шумовым экстремумом считаетсям экстремум чей уровень не превышает
		//средний уровень шумового экстремума болеее чем в три раза
		//сначала определяем средний уровень всех экстремумов
		//потом удаляем из выборки все нешумовые
		//потом повторяем процесс для всех оставшихся
		//делаем проходы пока не удалим последний нештатный экстремум
		do {
			//Определяем средний уровень
			Avg = (Sum / DCount) * NoiseRate;
			//выкидываем пики не превышающие этот уровень
			DefeatedCount = 0;
			for (j = 0; j < Count; j++) {
				if (Extrems[j] > Avg) {
					Extrems[j] = 0;
					DefeatedCount++;
				}
			}
			DCount = 0;
			//Определяем новую среднюю величину экстремума
			for (j = 0, Sum = 0; j < Count; j++) {
				if (Extrems[j] != 0) {
					Sum += Extrems[j];
					DCount++;
				}
			}
		} while (DefeatedCount > 0);
		NoiseLevel = (int)Avg;
	}
	DRNoiseLevel = 0;
	if (DRLimit)
	{
		int nTemp = (int)ceil((double)nMax / pow(10.0, (double)DRLimit));
		if (nTemp > NoiseLevel) { NoiseLevel = nTemp; DRNoiseLevel = nTemp; }
	}

	//сдвигаем в начало массива экстремумы, которые выше порога и внутри диапазона 
	Count = 0;
	for (i = 0; i < ExtremCount; i++)
	{
		if (ExtremInd[i] > LBound && ExtremInd[i]<RBound && Data[ExtremInd[i]]>NoiseLevel) {
			ExtremInd[Count] = ExtremInd[i]; Count++;
		}
	}
	ExtremCount = Count;

	//По окончании у имеется массив ExtremInd[i], первые ExtremCount элементов которого
	//указывают на экстремумы пиков, которые выше порога и лежат внутри исследуемого диапазона
}

//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
double PDAlg::PeakTimeResolution(int Peak) {

	try {
		double Width = Peaks[Peak].ExactRightBound - Peaks[Peak].ExactLeftBound;
		if (Width <= 0) return 0;
		else  return (double)Peaks[Peak].Extrem / (2 * Width);
	}
	catch (...) { return 0; };
}

//-------------------------------------------------------------------------------------

METROLOGY_API std::vector<TPeak> PDAlg::PeaksDetecting(int* RawData, int RawDataLength, double _GSamplesRate, TPDAlgParams* PDAlgParams)
{
	double GSRation = 1.0 / _GSamplesRate;
	float ResLevel;
	int i, j, WorkCount, Count, cnt, nResult;

	int nErrPoint = 0;

	//int IndMax, IndMinRight, IndMinLeft,
	int LastMinRight;

	Data = RawData;

	Resolution = 0;
	DispRate = 0;
	//NoiseRate = 3;
	NoiseRate = PDAlgParams->iNoiseRate;
	DispFiltered = false;
	PeakWidth = 1;
	NoiseLevel = PDAlgParams->iNoiseLevel;
	DRLimit = (int)floor(PDAlgParams->dFilterLowLimit);
	PeakBounds = PDAlgParams->dPeaksBounds;
	CurrentDataLength = RawDataLength;

	nErrPoint = 10;

	if (Peaks) {
		delete[] Peaks;
		Peaks = nullptr;
	}
	if (Extrems) {
		delete[] Extrems;
		Extrems = nullptr;
	}
	if (ExtremInd) {
		delete[] ExtremInd;
		ExtremInd = nullptr;
	}

	try
	{

		if ((Peaks == nullptr) || (FDataLength < RawDataLength))
		{
			FDataLength = RawDataLength;
			Peaks = new TPeakInside[FDataLength];
			Extrems = new int[CurrentDataLength];
			ExtremInd = new int[CurrentDataLength];

		}

		nErrPoint = 20;

		//Определяем границы поиска
		if (!PDAlgParams->bAllData)
		{
			LBound = PDAlgParams->iLeftLim;
			RBound = PDAlgParams->iRightLim;
		}
		else
		{
			LBound = 1;
			RBound = RawDataLength;
		}

		nErrPoint = 30;
		//Если задано определение уровня шума - определяем его
		ExtremCount = 0;
		NoiseLevel = PDAlgParams->iNoiseLevel;
		NoiseLevelDetect(PDAlgParams->bAutoNoise);
		PDAlgParams->iNoiseLevel = NoiseLevel;
		PDAlgParams->dFilterUpLimit = DRNoiseLevel; //это поле структуры используется не по назначению - через него возвращается значение DRNoiseLevel;

		nErrPoint = 40;
		if (!ExtremCount)//случай, когда спектр пустой
		{
			//throw 2;
			if (Peaks) {
				delete[] Peaks;
				Peaks = nullptr;
			}
			if (Extrems) {
				delete[] Extrems;
				Extrems = nullptr;
			}
			if (ExtremInd) {
				delete[] ExtremInd;
				ExtremInd = nullptr;
			}
			return std::vector<TPeak>();
		}

		nErrPoint = 41;
		cnt = 0; nResult = 0; WorkCount = 0; LastMinRight = LBound;

		while (cnt < ExtremCount)
		{
			nResult = 0;

			nErrPoint = 42;
			Peaks[WorkCount].Extrem = ExtremInd[cnt];

			nErrPoint = 43;
			//Интенсивность, на которой проверяется разрешение пика
			ResLevel = static_cast<float>(PeakBounds * Data[ExtremInd[cnt]]);
			//Идем влево - ищем первый минимум, разрешающий пик
			i = ExtremInd[cnt] - 1;
			while (1)
			{
				if (i <= LastMinRight) { nResult = 1; break; } //уперлись в границу обработанной части спектра
				if (Data[i] <= ResLevel)//Найден разрешающий минимум
				{
					nResult = 0; break;
				}
				i--;
			}

			nErrPoint = 44;
			if (!nResult) //Скан завершился нахождением разрешающего минимума слева
			{
				Peaks[WorkCount].LeftBound = i + 1;

				nErrPoint = 45;
				//Идем вправо - ищем первый минимум, разрешающий пик
				i = ExtremInd[cnt] + 1;
				while (1)
				{
					if (i >= RBound - 1) { nResult = 2; break; }
					if (Data[i] > Data[Peaks[WorkCount].Extrem])
					{
						nResult = 1; break;
					} //По ходу скана вместо минимума обнаружен более интенсивный максимум
					if (Data[i] <= ResLevel)//Найден разрешающий минимум
					{
						nResult = 0; break;
					}
					i++;
				}
				nErrPoint = 46;
				if (!nResult) //Скан завершился нахождением разрешающего минимума справа
				{
					Peaks[WorkCount].RightBound = i - 1;

					Peaks[WorkCount].Height = Data[Peaks[WorkCount].Extrem];

					//Уточняем положение левой границы пика
					Peaks[WorkCount].ExactLeftBound = Peaks[WorkCount].LeftBound;
					if ((Data[Peaks[WorkCount].LeftBound] - Data[Peaks[WorkCount].LeftBound - 1]) != 0)
						Peaks[WorkCount].ExactLeftBound -= ((double)Data[Peaks[WorkCount].LeftBound] - ResLevel) / ((double)Data[Peaks[WorkCount].LeftBound] - (double)Data[Peaks[WorkCount].LeftBound - 1]);

					//Уточняем положение правой границы пика
					Peaks[WorkCount].ExactRightBound = Peaks[WorkCount].RightBound;
					if ((Data[Peaks[WorkCount].RightBound + 1] - Data[Peaks[WorkCount].RightBound]) != 0)
						Peaks[WorkCount].ExactRightBound += ((double)Data[Peaks[WorkCount].RightBound] - ResLevel) / ((double)Data[Peaks[WorkCount].RightBound] - (double)Data[Peaks[WorkCount].RightBound + 1]);

					LastMinRight = Peaks[WorkCount].RightBound;
					//Передвигаем ограничитель обработанной части спектра
					//на правую границу последнего найденного пика
					WorkCount++; //Передвигаем курсор на следующий пик только если
					//последний найденный пик нас устраивает
				}
			}
			switch (nResult)
			{
			case(2):  cnt = ExtremCount; break; //дошли до конца диапазона - инициируем завершение цикла
			default:  cnt++;    break;
			}
		}
		PeakCount = WorkCount;

		nErrPoint = 50;

		if (PDAlgParams->iFilterWinCount > 0) //задана фильтрация по разрешению
		{
			//Фильтрация по разрешению
			double* LocalResolution = new double[PDAlgParams->iFilterWinCount]; //Массив значений разрешения наиболее интенсивного пика в пяти различных частях спектра
			int* IndLocalMaxPeak = new int[PDAlgParams->iFilterWinCount]; memset(IndLocalMaxPeak, 0, 4 * PDAlgParams->iFilterWinCount);
			int* LocalMaxIntens = new int[PDAlgParams->iFilterWinCount]; memset(LocalMaxIntens, 0, 4 * PDAlgParams->iFilterWinCount);
			int IndCurArea = 0;
			double PeakRes;

			//Расчет локальных значений разрешения на наиболее интенсивных пиках
			for (i = 0, IndCurArea = 0; i < WorkCount; i++)
			{
				IndCurArea = static_cast<int>(floor((double)PDAlgParams->iFilterWinCount * (double)Peaks[i].Extrem / (double)FDataLength));
				//IndCurArea = floor((double)PDAlgParams->iFilterWinCount * (double)Peaks[i].Extrem / (double)FDataLength);
				IndCurArea = static_cast<int>(floor(
					static_cast<double>(PDAlgParams->iFilterWinCount * Peaks[i].Extrem) / 
					static_cast<double>(FDataLength)));
				if (Data[Peaks[i].Extrem] > LocalMaxIntens[IndCurArea])
				{
					LocalMaxIntens[IndCurArea] = Data[Peaks[i].Extrem];
					IndLocalMaxPeak[IndCurArea] = i;
				}
			}

			for (i = 0; i < PDAlgParams->iFilterWinCount; i++)
			{
				LocalResolution[i] = PeakTimeResolution(IndLocalMaxPeak[i]);
			}


			//Проверка пика на соответствие ограничениям по разрешению
			//Полагаем, что корректное разрешение лежит в диапазоне [0.6 - 3]
			//от разрешения локально максимального пика
			WorkCount = 0;
			for (i = 0; i < PeakCount; i++)
			{
				IndCurArea = static_cast<int>(floor((double)PDAlgParams->iFilterWinCount * (double)Peaks[i].Extrem / (double)FDataLength));
				//IndCurArea = floor((double)PDAlgParams->iFilterWinCount * (double)Peaks[i].Extrem / (double)FDataLength);
				IndCurArea = static_cast<int>(floor(
					static_cast<double>(PDAlgParams->iFilterWinCount * Peaks[i].Extrem) /
					static_cast<double>(FDataLength)));

				PeakRes = PeakTimeResolution(i);

				if (LocalResolution[IndCurArea] == 0 || (PeakRes > PDAlgParams->dFilterLowLimit * LocalResolution[IndCurArea] && PeakRes < PDAlgParams->dFilterUpLimit * LocalResolution[IndCurArea]))
				{
					Peaks[WorkCount].Extrem = Peaks[i].Extrem;
					Peaks[WorkCount].LeftBound = Peaks[i].LeftBound;
					Peaks[WorkCount].RightBound = Peaks[i].RightBound;
					Peaks[WorkCount].Value = Peaks[i].Value;
					Peaks[WorkCount].Height = Peaks[i].Height;
					Peaks[WorkCount].Center = Peaks[i].Center;
					Peaks[WorkCount].SKO = Peaks[i].SKO;
					Peaks[WorkCount].ExactRightBound = Peaks[i].ExactRightBound;
					Peaks[WorkCount].ExactLeftBound = Peaks[i].ExactLeftBound;
					WorkCount++;
				}
			}
			if (LocalResolution) delete[] LocalResolution;
			if (IndLocalMaxPeak) delete[] IndLocalMaxPeak;
			if (LocalMaxIntens) delete[] LocalMaxIntens;

		}//фильтрация по разрешению

		Count = WorkCount;
		PeakCount = WorkCount;

		nErrPoint = 60;

		//отбор нужного числа пиков
		int MaxPeak = 0;
		if (PDAlgParams->iMaxPeakCount && PeakCount > PDAlgParams->iMaxPeakCount)
		{
			//перераспределяем память для набора лучших пиков
			for (i = 0; i < PDAlgParams->iMaxPeakCount; i++)
			{
				MaxPeak = i;
				for (j = i; j < Count - 1; j++)
				{
					if (Data[Peaks[j].Extrem] > Data[Peaks[MaxPeak].Extrem])	MaxPeak = j;
				}
				Tmp = Peaks[i];
				Peaks[i] = Peaks[MaxPeak];
				Peaks[MaxPeak] = Tmp;
			}

			Count = PDAlgParams->iMaxPeakCount;
			PeakCount = PDAlgParams->iMaxPeakCount;
		}

		nErrPoint = 70;

		//Интегрирование, поиск центроидов
		//в данный момент границы пиков проложены по фактической точке уровня обрезки пиков
		//центроид оценивается по центру масс фигуры выше уровня обрезки пиков
		//оценка пика включает и нижний прямоугольник (до уровня обрезки) тоже

		double TruncLevel; //уровень обрезки пика
		double fPeakSum = 0.0;
		double fMoment = 0.0;
		double fTemp, fDelta;
		for (i = 0; i < Count; i++)
		{
			TruncLevel = ((double)RawData[Peaks[i].Extrem]) * PeakBounds;
			fPeakSum = 0.0;
			fMoment = 0.0;

			for (j = Peaks[i].LeftBound; j < Peaks[i].RightBound; j++)
			{
				//Aa?oiee o?aoaieuiee
				fTemp = ((double)RawData[j + 1] - (double)RawData[j]) * 0.5;
				fPeakSum += fTemp;
				fMoment += fTemp * ((double)j + 2.0 / 3.0);
				//I?yiioaieuiee
				fTemp = (double)RawData[j];
				fPeakSum += fTemp;
				fMoment += fTemp * ((double)j + 0.5);
			}
			//Eaaue iaiieiue aei
			fDelta = (double)Peaks[i].LeftBound - Peaks[i].ExactLeftBound;
			//Aa?oiee o?aoaieuiee
			fTemp = ((double)RawData[Peaks[i].LeftBound] - TruncLevel) * fDelta * 0.5;
			fPeakSum += fTemp;
			fMoment += fTemp * (Peaks[i].ExactLeftBound + fDelta * 2.0 / 3.0);
			//I?yiioaieuiee
			fTemp = TruncLevel * fDelta;
			fPeakSum += fTemp;
			fMoment += fTemp * (Peaks[i].ExactLeftBound + fDelta * 0.5);


			//I?aaue iaiieiue aei
			fDelta = Peaks[i].ExactRightBound - (double)Peaks[i].RightBound;
			//Aa?oiee o?aoaieuiee
			fTemp = (TruncLevel - (double)RawData[Peaks[i].RightBound]) * fDelta * 0.5;
			fPeakSum += fTemp;
			fMoment += fTemp * ((double)Peaks[i].RightBound + fDelta * 2.0 / 3.0);
			//I?yiioaieuiee
			fTemp = (double)RawData[Peaks[i].RightBound] * fDelta;
			fPeakSum += fTemp;
			fMoment += fTemp * ((double)Peaks[i].RightBound + fDelta * 0.5);


			Peaks[i].Value = fPeakSum;
			Peaks[i].Center = fMoment / fPeakSum;
			Peaks[i].FWRL = Peaks[i].ExactRightBound - Peaks[i].ExactLeftBound;
			Peaks[i].Resolution = PeakTimeResolution(i);
		}

		nErrPoint = 80;
		//отсеиваем пики с нулевыми центроидами или интенсивностью
		WorkCount = 0;
		for (i = 0; i < PeakCount; i++)
		{
			if (Peaks[i].Extrem != 0 && Peaks[i].Resolution > 0 && Peaks[i].FWRL > 0 && Peaks[i].Center > 0 && Peaks[i].Value > 0)
			{
				Peaks[WorkCount].Extrem = Peaks[i].Extrem;
				Peaks[WorkCount].LeftBound = Peaks[i].LeftBound;
				Peaks[WorkCount].RightBound = Peaks[i].RightBound;
				Peaks[WorkCount].ExactRightBound = Peaks[i].ExactRightBound;
				Peaks[WorkCount].ExactLeftBound = Peaks[i].ExactLeftBound;
				Peaks[WorkCount].Value = Peaks[i].Value;
				Peaks[WorkCount].Height = Peaks[i].Height;
				Peaks[WorkCount].Center = Peaks[i].Center;
				Peaks[WorkCount].SKO = Peaks[i].SKO;
				Peaks[WorkCount].FWRL = Peaks[i].FWRL;
				Peaks[WorkCount].Resolution = Peaks[i].Resolution;

				WorkCount++;
			}
			else
			{
				i = i;
			}
		}
		Count = WorkCount;
		PeakCount = WorkCount;

		nErrPoint = 90;
		//Сортируем набор пиков по времени 
		if (PDAlgParams->iMaxPeakCount)
		{
			for (i = 0; i < PeakCount - 1; i++) {
				for (j = 0; j < PeakCount - i - 1; j++) {
					if (Peaks[j].Extrem > Peaks[j + 1].Extrem) {
						Peaks[PeakCount] = Peaks[j];
						Peaks[j] = Peaks[j + 1];
						Peaks[j + 1] = Peaks[PeakCount];
					}
				}
			}
		}

		//int* c = (int*)*outPeaksStream;
		std::vector<TPeak> OutPeaks;

		nErrPoint = 100;
		//Формат буфера результатов пик-детектинга: 
		//c[0] = PeakCount; //первые 4 байта - кол-во пиков
		//Это сдвиг на + 4
		//c += 1;
		//OutPeaks = (TPeak*)c;

		for (i = 0; i < PeakCount; i++) {
			TPeak appendedPeak;
			appendedPeak.Extrem = static_cast<int>((Peaks[i].Extrem) * GSRation);
			appendedPeak.Center = (Peaks[i].Center) * GSRation;
			appendedPeak.FWRL = Peaks[i].FWRL;
			appendedPeak.Value = (Peaks[i].Value)* GSRation;
			appendedPeak.Resolution = Peaks[i].Resolution;
			appendedPeak.Height = Data[Peaks[i].Extrem];
			appendedPeak.LeftBound = (Peaks[i].ExactLeftBound) * GSRation;
			appendedPeak.RightBound = (Peaks[i].ExactRightBound) * GSRation;
			OutPeaks.push_back(appendedPeak);
		}

		if (Peaks) {
			delete[] Peaks;
			Peaks = nullptr;
		}
		if (Extrems) {
			delete[] Extrems;
			Extrems = nullptr;
		}
		if (ExtremInd) {
			delete[] ExtremInd;
			ExtremInd = nullptr;
		}

		return OutPeaks;
	}
	catch (...)
	{

		if (Peaks) {
			delete[] Peaks;
			Peaks = nullptr;
		}
		if (Extrems) {
			delete[] Extrems;
			Extrems = nullptr;
		}
		if (ExtremInd) {
			delete[] ExtremInd;
			ExtremInd = nullptr;
		}
	};

	return std::vector<TPeak>();
	/*catch (int nMsgCode)
	{
		PeakCount = 0;
		int* c = (int*)*outPeaksStream;
		c[0] = PeakCount;
		return 0;
	}
	catch (...)
	{
		PeakCount = 0;
		int* c = (int*)*outPeaksStream;
		c[0] = PeakCount;
		return nErrPoint;
	};
	return 0;*/
}

//-------------------------------------------------------------------------------------

//void PeaksDetectingOld(int* RawData, int RawDataLength,
//	TPDAlgParams* PDAlgParams,
//	void** outPeaksStream) {
//	Data = RawData;
//	NoiseLevel = PDAlgParams->iNoiseLevel;
//	PeakBounds = PDAlgParams->dPeaksBounds;
//	CurrentDataLength = RawDataLength;
//
//	Resolution = 0;
//	DispRate = 0;
//	NoiseRate = 3;
//
//	DispFiltered = false;
//	PeakWidth = 1;
//
//
//	int i, j, k, n;
//	int Count = 0, WorkCount, Max, MaxPeak;
//	double Sum = 0;
//	//TPeak *NewPeaks;
//
//	try {
//
//		if ((Peaks == NULL) || (FDataLength < RawDataLength)) {
//			FDataLength = RawDataLength;
//			if (Peaks) delete[] Peaks;
//			if (Extrems) delete[] Extrems;
//			Peaks = new TPeakInside[FDataLength];
//			Extrems = new int[CurrentDataLength];
//		}
//
//
//		//Если задано определение уровня шума - определяем его
//		if (PDAlgParams->bAutoNoise) {
//			NoiseLevelDetect(PDAlgParams->bAutoNoise);
//			PDAlgParams->iNoiseLevel = NoiseLevel;
//		}
//
//		//Определяем границы поиска
//		int LBound, RBound;
//		if (!PDAlgParams->bAllData) {
//			LBound = PDAlgParams->iLeftLim;
//			RBound = PDAlgParams->iRightLim;
//		}
//		else {
//			LBound = 1;
//			RBound = RawDataLength;
//		}
//
//		//Ищем экстремумы превышающие уровень шума
//		for (j = LBound + 1; j <= RBound - 2; j++) {
//			if (Data[j] > NoiseLevel && Data[j] > Data[j - 1] && Data[j] > Data[j + 1]) {
//				Peaks[Count].Extrem = j;
//				Peaks[Count].Value = 0;
//				Count++;
//			}
//			//      if(Count>100) break;
//		}
//
//		WorkCount = 0;
//
//		//Цикл групп
//
//		for (i = 0; i < Count; i++) {
//			//Выделяем группу пиков между которыми не шум
//			k = i; //k - индекс первого из пиков группы
//			for (j = Peaks[i].Extrem; Data[j] > NoiseLevel && i < Count && j < RawDataLength; j++)
//				if (j == Peaks[i + 1].Extrem) i++;
//			if (i == Count) i--;
//			//на этот момент i - это индекс последнего из пиков группы
//			//Цикл истинных пиков
//			if (k == 1) {
//				WorkCount = 0;
//			}
//
//			while (1)
//			{
//				//Ищем максимальный необработанный пик
//				for (j = k, Max = 0; j <= i; j++)
//					if (Data[Peaks[j].Extrem] > Max && Peaks[j].Value == 0) {
//						Max = Data[Peaks[j].Extrem];
//						MaxPeak = j;
//					}
//				//если необработанных пиков больше не осталось, то выход
//				if (Max == 0) break;
//				//Максимальный пик это действительный пик
//				Peaks[MaxPeak].Value = 2;
//				//От максимального пика ищем его падение до уровня
//				for (j = Peaks[MaxPeak].Extrem, n = MaxPeak - 1; (Data[j] >= (((double)Max) * PeakBounds)); j--) {
//					//Если по дороге обнаружили что уровень стал больше пикового то текущий пик недействителен
//					// Какой знак здесь нужно использовать > или >= ?!!!
//					if (Data[j] > Max) {
//						Peaks[MaxPeak].Value = 1;
//						break;
//					}
//					//если по дороге встречаются другие пики то считаем их входящими
//					// в максимальный пик
//					if ((n >= 0) && (Peaks[n].Extrem == j)) {
//						Peaks[n].Value = 1;
//						n--;
//					}
//					if (j == 0) break;
//				}
//				//Запоминаем левую границу пика
//				if (Peaks[MaxPeak].Value == 1) continue;
//				else Peaks[MaxPeak].LeftBound = j + 1;
//				//тоже самое в другую сторону
//				for (j = Peaks[MaxPeak].Extrem, n = MaxPeak + 1; (Data[j] >= (((double)Max) * PeakBounds)); j++) {
//					if (Data[j] > Max) {
//						Peaks[MaxPeak].Value = 1;
//						break;
//					}
//					if (Peaks[n].Extrem == j) {
//						Peaks[n].Value = 1;
//						if (n < Count)n++;
//					}
//					if (j >= FDataLength) break;
//				}
//				if (Peaks[MaxPeak].Value == 2) Peaks[MaxPeak].RightBound = j - 1;
//				//Проверяем ширину пика - указанное число - это минимальное количество точек входящих в пик
//				if (Peaks[MaxPeak].RightBound - Peaks[MaxPeak].LeftBound + 1 < PeakWidth && (Peaks[MaxPeak].Value == 2))
//					Peaks[MaxPeak].Value = 1;
//			}
//		}
//		//убираем отфилтрованные пики
//		//уплотняем массив
//		WorkCount = 0;
//		for (i = 0; i < Count; i++)
//		{
//			if (Peaks[i].Value == 2)
//			{
//				Peaks[WorkCount].Extrem = Peaks[i].Extrem;
//				Peaks[WorkCount].LeftBound = Peaks[i].LeftBound;
//				Peaks[WorkCount].RightBound = Peaks[i].RightBound;
//				//          Peaks[WorkCount].inSvert      = 0;
//				WorkCount++;
//			}
//		}
//
//		/*Peaks[WorkCount].Center =0.0;
//		Peaks[WorkCount].Extrem =0;
//		Peaks[WorkCount].Resolution =1000000000.0;
//		Peaks[WorkCount].Value =0.0;
//		WorkCount++;*/
//
//		Count = WorkCount;
//		PeakCount = WorkCount;
//
//		//отбор нужного числа пиков
//		int MaxPeak = 0;
//		if (PDAlgParams->iMaxPeakCount && PeakCount > PDAlgParams->iMaxPeakCount)
//		{
//
//			//перераспределяем память для набора лучших пиков
//			for (i = 0; i < PDAlgParams->iMaxPeakCount; i++)
//			{
//				MaxPeak = i;
//				for (j = i; j < Count - 1; j++)
//				{
//					if (Data[Peaks[j].Extrem] > Data[Peaks[MaxPeak].Extrem])	MaxPeak = j;
//				}
//				Tmp = Peaks[i];
//				Peaks[i] = Peaks[MaxPeak];
//				Peaks[MaxPeak] = Tmp;
//			}
//			//делаем так, чтобы нулевой пик вошел в число "лучших"
//			//Peaks[PDAlgParams->iMaxPeakCount-1] = Peaks[WorkCount-1];
//
//			Count = PDAlgParams->iMaxPeakCount;
//			PeakCount = PDAlgParams->iMaxPeakCount;
//		}
//
//		double USum, LSum, SSum;
//		double UFSum = 0, LFSum = 0, SFSum = 0, FRel1, FRel2, FAvg;
//		//Интегрирование, поиск центроидов, поиск дисперсии
//		//в данный момент границы пиков проложены по фактической точке уровня обрезки пиков
//		//центроид оценивается по центру масс фигуры выше уровня обрезки пиков
//		//оценка пика включает и нижний прямоугольник (до уровня обрезки) тоже
//		double TruncLevel; //уровень обрезки пиков
//		for (i = 0; i < Count; i++) {
//			TruncLevel = ((double)Data[Peaks[i].Extrem]) * PeakBounds;
//			USum = 0; LSum = 0; SSum = 0;
//			//основная часть пика
//			for (j = Peaks[i].LeftBound + 1; j <= Peaks[i].RightBound - 1; j++) {
//				USum += (Data[j] - TruncLevel) * j;
//				LSum += Data[j] - TruncLevel;
//				SSum += (Data[j] - TruncLevel) * j * j;
//			}
//			//краевые точки на основании решения о добавлении трапеций
//			//Если пик шире единицы то в оценку входят половинки от крайних столбиков
//			if (Peaks[i].RightBound - Peaks[i].LeftBound > 0) {
//				UFSum = (Peaks[i].LeftBound + 0.25) * (Data[Peaks[i].LeftBound] - TruncLevel) / 2;
//				LFSum = ((double)Data[Peaks[i].LeftBound] - TruncLevel) / 2;
//				SFSum = (Peaks[i].LeftBound + 0.25) * (Peaks[i].LeftBound + 0.25) * (Data[Peaks[i].LeftBound] - TruncLevel) / 2;
//				UFSum += (Peaks[i].RightBound - 0.25) * (Data[Peaks[i].RightBound] - TruncLevel) / 2;
//				LFSum += ((double)Data[Peaks[i].RightBound] - TruncLevel) / 2;
//				SFSum += (Peaks[i].RightBound - 0.25) * (Peaks[i].RightBound - 0.25) * (Data[Peaks[i].RightBound] - TruncLevel) / 2;
//			}
//			else {
//				UFSum = 0;
//				LFSum = 0;
//				SFSum = 0;
//			}
//
//			//Расчет площади краевых треугольников
//			if ((Data[Peaks[i].RightBound] - Data[Peaks[i].RightBound + 1]) == 0)continue;
//			FRel1 = (Data[Peaks[i].RightBound] - TruncLevel) /
//				(Data[Peaks[i].RightBound] - Data[Peaks[i].RightBound + 1]);
//
//			FAvg = ((double)Data[Peaks[i].RightBound] - TruncLevel) * FRel1 / 2;
//			LFSum += FAvg;
//			UFSum += FAvg * (Peaks[i].RightBound + FRel1 / 2);
//			SFSum += FAvg * (Peaks[i].RightBound + FRel1 / 2) * (Peaks[i].RightBound + FRel1 / 2);
//
//			if ((Data[Peaks[i].LeftBound] - Data[Peaks[i].LeftBound - 1]) == 0)continue;
//			FRel2 = (Data[Peaks[i].LeftBound] - TruncLevel) /
//				(Data[Peaks[i].LeftBound] - (Data[Peaks[i].LeftBound - 1]));
//
//			FAvg = ((double)Data[Peaks[i].LeftBound] - TruncLevel) * FRel2 / 2;
//			LFSum += FAvg;
//			UFSum += FAvg * (Peaks[i].LeftBound - FRel2 / 2);
//			SFSum += FAvg * (Peaks[i].LeftBound - FRel2 / 2) * (Peaks[i].LeftBound - FRel2 / 2);
//
//
//			Peaks[i].Value = LSum + LFSum + (
//				Peaks[i].RightBound - Peaks[i].LeftBound + FRel1 + FRel2) * TruncLevel;
//			// к оценке пика добавляем площадь нижнего прямоугольника
//			if ((LSum + LFSum) == 0)continue;
//			Peaks[i].Center = (USum + UFSum) / (LSum + LFSum);
//			//Среднеквадратичное отклонение
//			if (((SSum + SFSum) / (LSum + LFSum)) - Peaks[i].Center * Peaks[i].Center > 0)
//				Peaks[i].SKO = sqrt(((SSum + SFSum) / (LSum + LFSum)) - Peaks[i].Center * Peaks[i].Center);
//			else Peaks[i].SKO = 1;
//
//			Peaks[i].Resolution = PeakTimeResolution(i);
//			Peaks[i].ExactLeftBound = FRel1;
//			Peaks[i].ExactRightBound = FRel2;
//		}
//
//		if (0) {
//			WorkCount = 0;
//			//Наименьшее корректное разрешение
//			for (i = 0; i < PeakCount; i++) {
//				if (PeakTimeResolution(i) > Resolution) {
//					Peaks[WorkCount].Extrem = Peaks[i].Extrem;
//					Peaks[WorkCount].LeftBound = Peaks[i].LeftBound;
//					Peaks[WorkCount].RightBound = Peaks[i].RightBound;
//					Peaks[WorkCount].Value = Peaks[i].Value;
//					Peaks[WorkCount].Center = Peaks[i].Center;
//					Peaks[WorkCount].SKO = Peaks[i].SKO;
//					//            Peaks[WorkCount].inSvert      = 0;
//					WorkCount++;
//				}
//			}
//			Count = WorkCount;
//			PeakCount = WorkCount;
//		}
//		//фильтр по дисперсии
//		if (DispFiltered) {
//			WorkCount = 0;
//			// разрешение по дисперсии
//			double DispRes;
//			//Наименьшее корректное разрешение по дисперсии
//			double ResLim;
//			ResLim = Resolution * (DispRate);
//			for (i = 0; i < PeakCount; i++) {
//				if (Peaks[i].SKO) {
//					DispRes = Peaks[i].Center / (Peaks[i].SKO * 4);
//					if (DispRes > ResLim) {
//						Peaks[WorkCount].Extrem = Peaks[i].Extrem;
//						Peaks[WorkCount].LeftBound = Peaks[i].LeftBound;
//						Peaks[WorkCount].RightBound = Peaks[i].RightBound;
//						Peaks[WorkCount].Value = Peaks[i].Value;
//						Peaks[WorkCount].Center = Peaks[i].Center;
//						Peaks[WorkCount].SKO = Peaks[i].SKO;
//						//            Peaks[WorkCount].inSvert      = 0;
//						WorkCount++;
//					}
//				}
//			}
//			Count = WorkCount;
//			PeakCount = WorkCount;
//		}
//
//		//Сортируем набор пиков по времени 
//		if (PDAlgParams->iMaxPeakCount) {
//			for (i = 0; i < PeakCount - 1; i++) {
//				for (j = 0; j < PeakCount - i - 1; j++) {
//					if (Peaks[j].Extrem > Peaks[j + 1].Extrem) {
//						Peaks[PeakCount] = Peaks[j];
//						Peaks[j] = Peaks[j + 1];
//						Peaks[j + 1] = Peaks[PeakCount];
//					}
//				}
//			}
//		}
//
//		int* c = (int*)*outPeaksStream;
//		TPeak* OutPeaks;
//
//
//		//Формат буфера результатов пик-детектинга: 
//		c[0] = PeakCount; //первые 4 байта - кол-во пиков
//		//Это сдвиг на + 4
//		c += 1;
//		OutPeaks = (TPeak*)c;
//
//		for (i = 0; i < PeakCount; i++) {
//			OutPeaks[i].Extrem = Peaks[i].Extrem;
//			OutPeaks[i].Center = Peaks[i].Center;
//			OutPeaks[i].FWRL = Peaks[i].FWRL;
//			OutPeaks[i].Value = Peaks[i].Value;
//			OutPeaks[i].Resolution = Peaks[i].Resolution;
//			OutPeaks[i].Height = Data[Peaks[i].Extrem];
//			OutPeaks[i].LeftBound = Peaks[i].ExactLeftBound;
//			OutPeaks[i].RightBound = Peaks[i].ExactRightBound;
//		}
//		c = c;
//		//delete[] Peaks;
//
//	}
//	catch (...) {
//		//if(NewPeaks) delete[] NewPeaks;
//		//if(Peaks) delete[] Peaks;
//		PeakCount = 0;
//		int* c = (int*)*outPeaksStream;
//		c[0] = PeakCount;
//	};
//}

//-------------------------------------------------------------------------------------


/*TPeak* Peaks;
int* Data;
int PeakCount;
int FDataLength;
int CurrentDataLength;
double Resolution;
double DispRate;
int NoiseRate;

bool ResFiltered;
bool DispFiltered;
int NoiseLevel;
double PeakBounds;
int PeakWidth;


//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
void NoiseLevelDetect(){

int * Extrems = new int[CurrentDataLength];
//Ищем все эктремумы

int j,Count=0,DCount,DefeatedCount;
double Sum = 0,Avg;

   for(j=1; j<=CurrentDataLength-2 ; j++){
	  if (Data[j]>Data[j-1] && Data[j]>Data[j+1]){
		 Extrems[Count]=Data[j];
		 Sum+=Data[j];
		 Count++;
	  }
   }
   DCount = Count;
   //Удаляем шумовые экстремумы
   //шумовым экстремумом считаетсям экстремум чей уровень не превышает
   //средний уровень шумового экстремума болеее чем в три раза
   //сначала определяем средний уровень всех экстремумов
   //потом удаляем из выборки все нешумовые
   //потом повторяем процесс для всех оставшихся
   //делаем проходы пока не удалим последний нештатный экстремум
   do {
	  //Определяем средний уровень
	  Avg = (Sum/DCount)*NoiseRate;
	  //выкидываем пики не превышающие этот уровень
	  DefeatedCount = 0;
	  for ( j=0 ; j<Count ; j++ ) {
		 if (Extrems[j]>Avg){
			Extrems[j]=0;
			DefeatedCount++;
		 }
	  }
	  DCount=0;
	  //Определяем новую среднюю величину экстремума
	  for ( j=0,Sum=0 ; j<Count ; j++ ) {
		  if (Extrems[j] != 0){
			 Sum+=Extrems[j];
			 DCount++;
		  }
	  }
   }
   while (DefeatedCount>0);
   delete[] Extrems;
   NoiseLevel = (int)Avg;
}

//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
double PeakTimeResolution(int Peak){

try{
   double Width,TruncLevel;

   Width = Peaks[Peak].RightBound - Peaks[Peak].LeftBound;
 if(Width==0) return 0;
   TruncLevel = ((double)Data[Peaks[Peak].Extrem])*PeakBounds;

 if((Data[Peaks[Peak].RightBound] - (Data[Peaks[Peak].RightBound+1]))==0) return 0;
  Width += (Data[Peaks[Peak].RightBound] - TruncLevel)/
		   (Data[Peaks[Peak].RightBound] - (Data[Peaks[Peak].RightBound+1]));

 if((Data[Peaks[Peak].LeftBound] - (Data[Peaks[Peak].LeftBound-1]))==0) return 0;
  Width += (Data[Peaks[Peak].LeftBound] - TruncLevel)/
		   (Data[Peaks[Peak].LeftBound] - (Data[Peaks[Peak].LeftBound-1]));

  return Peaks[Peak].Center/(2*Width);
 }catch(...){return 0;};
}

//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
PDALG_API void PeaksDetecting(int* RawData, int RawDataLength,
							  TPDAlgParams* PDAlgParams,
							  void** outPeaksStream){
	Data = RawData;
	NoiseLevel = PDAlgParams->iNoiseLevel;
	PeakBounds = PDAlgParams->dPeaksBounds;
	CurrentDataLength = RawDataLength;

	Resolution = 0;
	DispRate = 0;
	NoiseRate = 3;
	ResFiltered = false;
	DispFiltered = false;
	PeakWidth = 1;


int i,j,k,l,n,d;
int Count = 0,DefeatedCount,WorkCount,Half,Extrem,Max,MaxPeak,Rel,Prev;
double Sum = 0,Avg,Dist,Relat;
//TPeak *NewPeaks;

try{
	if((Peaks==NULL)||(FDataLength<RawDataLength)){
		FDataLength = RawDataLength;
		Peaks = new TPeak[FDataLength];
	}
   //Если задано определение уровня шума - определяем его
   if (PDAlgParams->bAutoNoise) {
	  NoiseLevelDetect();
	  PDAlgParams->iNoiseLevel = NoiseLevel;
   }

   //Ищем экстремумы превышающие уровень шума
   for(j=1 ; j<=RawDataLength-2 ; j++){
	  if (Data[j]>NoiseLevel && Data[j]>Data[j-1] && Data[j]>Data[j+1]){
		 Peaks[Count].Extrem=j;
		 Peaks[Count].Value = 0;
		 Count++;
	  }
//      if(Count>100) break;
   }

   WorkCount = 0;

   //Цикл групп

   for (i=0;i<Count;i++){
	  //Выделяем группу пиков между которыми не шум
	  k = i; //k - индекс первого из пиков группы
	  for (j=Peaks[i].Extrem ; Data[j]>NoiseLevel && i<Count && j<RawDataLength; j++ )
		 if (j==Peaks[i+1].Extrem) i++;
	  if(i==Count) i--;
	  //на этот момент i - это индекс последнего из пиков группы
	  //Цикл истинных пиков
	  if (k==1) {
		 WorkCount = 0;
	  }
	  while (1) {
		 //Ищем максимальный необработанный пик
		 for (j = k , Max = 0 ; j<=i ;j++)
			if (Data[Peaks[j].Extrem]>Max && Peaks[j].Value == 0 ) {
			   Max = Data[Peaks[j].Extrem];
			   MaxPeak = j;
			}
		 //если необработанных пиков больше не осталось, то выход
		 if (Max == 0) break;
		 //Максимальный пик это действительный пик
		 Peaks[MaxPeak].Value=2;
		 //От максимального пика ищем его падение до уровня
		 for ( j=Peaks[MaxPeak].Extrem , n=MaxPeak-1 ; (Data[j] >= (((double)Max)*PeakBounds)); j-- ) {
			//Если по дороге обнаружили что уровень стал больше пикового то текущий пик недействителен
			// Какой знак здесь нужно использовать > или >= ?!!!
			if (Data[j]>Max) {
			   Peaks[MaxPeak].Value=1;
			   break;
			}
			//если по дороге встречаются другие пики то считаем их входящими
			// в максимальный пик
			if ((n>=0)&& (Peaks[n].Extrem == j)) {
				Peaks[n].Value=1;
				n--;
			}
			if (j==0) break;
		 }
		 //Запоминаем левую границу пика
		 if (Peaks[MaxPeak].Value==1) continue;
		 else Peaks[MaxPeak].LeftBound = j+1;
		 //тоже самое в другую сторону
		 for ( j=Peaks[MaxPeak].Extrem , n=MaxPeak+1 ;(Data[j] >= (((double)Max)*PeakBounds)); j++ ) {
			if (Data[j]>Max) {
			   Peaks[MaxPeak].Value=1;
			   break;
			}
			if (Peaks[n].Extrem == j) {
			   Peaks[n].Value=1;
			   if(n<Count)n++;
			}
			if (j>=FDataLength) break;
		 }
		 if (Peaks[MaxPeak].Value==2) Peaks[MaxPeak].RightBound = j-1;
		 //Проверяем ширину пика - указанное число - это минимальное количество точек входящих в пик
		 if (Peaks[MaxPeak].RightBound-Peaks[MaxPeak].LeftBound+1 < PeakWidth && (Peaks[MaxPeak].Value==2))
			Peaks[MaxPeak].Value=1;
	  }
   }
   //убираем отфилтрованные пики
   //уплотняем массив
   WorkCount = 0;
   for (i = 0; i<Count ; i++) {
	  if ( Peaks[i].Value == 2 ) {
		  Peaks[WorkCount].Extrem       = Peaks[i].Extrem;
		  Peaks[WorkCount].LeftBound    = Peaks[i].LeftBound;
		  Peaks[WorkCount].RightBound   = Peaks[i].RightBound;
//          Peaks[WorkCount].inSvert      = 0;
		  WorkCount++;
	  }
   }
   Count = WorkCount;
   PeakCount = WorkCount;

   double USum,LSum,SSum;
   double UFSum=0,LFSum=0,SFSum=0,FRel1,FRel2,FAvg;
   //Интегрирование, поиск центроидов, поиск дисперсии
   //в данный момент границы пиков проложены по фактической точке уровня обрезки пиков
   //центроид оценивается по центру масс фигуры выше уровня обрезки пиков
   //оценка пика включает и нижний прямоугольник (до уровня обрезки) тоже
   double TruncLevel; //уровень обрезки пиков
   for (i = 0 ; i < Count; i++ ) {
	   TruncLevel = ((double)Data[Peaks[i].Extrem])*PeakBounds;
	   USum = 0 ; LSum = 0; SSum = 0;
	   //основная часть пика
	   for (j = Peaks[i].LeftBound+1 ; j<=Peaks[i].RightBound-1 ; j++ ){
		  USum += (Data[j] - TruncLevel)*j;
		  LSum += Data[j] - TruncLevel;
		  SSum += (Data[j] - TruncLevel) * j * j;
	   }
	   //краевые точки на основании решения о добавлении трапеций
	   //Если пик шире единицы то в оценку входят половинки от крайних столбиков
	   if (Peaks[i].RightBound - Peaks[i].LeftBound > 0 ){
		  UFSum =   (Peaks[i].LeftBound+0.25)*(Data[Peaks[i].LeftBound]- TruncLevel)/2;
		  LFSum =   ((double)Data[Peaks[i].LeftBound] - TruncLevel)/2;
		  SFSum =   (Peaks[i].LeftBound+0.25)*(Peaks[i].LeftBound+0.25)*(Data[Peaks[i].LeftBound]- TruncLevel)/2;
		  UFSum +=  (Peaks[i].RightBound-0.25)*(Data[Peaks[i].RightBound] - TruncLevel)/2;
		  LFSum +=  ((double)Data[Peaks[i].RightBound] - TruncLevel)/2;
		  SFSum +=  (Peaks[i].RightBound-0.25)*(Peaks[i].RightBound-0.25)*(Data[Peaks[i].RightBound] - TruncLevel)/2;
	   } else {
		  UFSum = 0;
		  LFSum = 0;
		  SFSum = 0;
	   }

	   //Расчет площади краевых треугольников
	   if((Data[Peaks[i].RightBound]-Data[Peaks[i].RightBound+1])==0)continue;
	   FRel1 = (Data[Peaks[i].RightBound] - TruncLevel)/
			  (Data[Peaks[i].RightBound] - Data[Peaks[i].RightBound+1]);

	   FAvg = ((double)Data[Peaks[i].RightBound]-TruncLevel)*FRel1/2;
	   LFSum += FAvg;
	   UFSum += FAvg*(Peaks[i].RightBound + FRel1/2);
	   SFSum += FAvg*(Peaks[i].RightBound + FRel1/2)*(Peaks[i].RightBound + FRel1/2);

	   if((Data[Peaks[i].LeftBound]-Data[Peaks[i].LeftBound-1])==0)continue;
	   FRel2 = (Data[Peaks[i].LeftBound] - TruncLevel)/
			  (Data[Peaks[i].LeftBound] - (Data[Peaks[i].LeftBound-1]));

	   FAvg = ((double)Data[Peaks[i].LeftBound] - TruncLevel)*FRel2/2;
	   LFSum += FAvg;
	   UFSum += FAvg*(Peaks[i].LeftBound - FRel2/2);
	   SFSum += FAvg*(Peaks[i].LeftBound - FRel2/2)*(Peaks[i].LeftBound - FRel2/2);


	   Peaks[i].Value = LSum+LFSum+(
		  Peaks[i].RightBound - Peaks[i].LeftBound+FRel1+FRel2)*TruncLevel;
		  // к оценке пика добавляем площадь нижнего прямоугольника
	   if((LSum+LFSum)==0)continue;
	   Peaks[i].Center = (USum+UFSum) / (LSum+LFSum);
	   //Среднеквадратичное отклонение
	   if(((SSum+SFSum) / (LSum+LFSum))-Peaks[i].Center*Peaks[i].Center>0)
		Peaks[i].SKO =sqrt(((SSum+SFSum) / (LSum+LFSum))-Peaks[i].Center*Peaks[i].Center);
	   else Peaks[i].SKO=1;

	   Peaks[i].Resolution = PeakTimeResolution(i);
   }

   if (ResFiltered) {
	 WorkCount = 0;
	 //Наименьшее корректное разрешение
	 for (i = 0; i<PeakCount ; i++) {
		if ( PeakTimeResolution(i) > Resolution ) {
			Peaks[WorkCount].Extrem       = Peaks[i].Extrem;
			Peaks[WorkCount].LeftBound    = Peaks[i].LeftBound;
			Peaks[WorkCount].RightBound   = Peaks[i].RightBound;
			Peaks[WorkCount].Value        = Peaks[i].Value;
			Peaks[WorkCount].Center       = Peaks[i].Center;
			Peaks[WorkCount].SKO          = Peaks[i].SKO;
//            Peaks[WorkCount].inSvert      = 0;
			WorkCount++;
		}
	 }
	 Count = WorkCount;
	 PeakCount = WorkCount;
   }
   //фильтр по дисперсии
   if (DispFiltered) {
	 WorkCount = 0;
	 // разрешение по дисперсии
	 double DispRes;
	 //Наименьшее корректное разрешение по дисперсии
	 double ResLim;
	 ResLim = Resolution*(DispRate);
	 for (i = 0; i<PeakCount ; i++) {
		if(Peaks[i].SKO){
		  DispRes = Peaks[i].Center / (Peaks[i].SKO * 4);
		if ( DispRes > ResLim ) {
			Peaks[WorkCount].Extrem       = Peaks[i].Extrem;
			Peaks[WorkCount].LeftBound    = Peaks[i].LeftBound;
			Peaks[WorkCount].RightBound   = Peaks[i].RightBound;
			Peaks[WorkCount].Value        = Peaks[i].Value;
			Peaks[WorkCount].Center       = Peaks[i].Center;
			Peaks[WorkCount].SKO          = Peaks[i].SKO;
//            Peaks[WorkCount].inSvert      = 0;
			WorkCount++;
		}}
	 }
	 Count = WorkCount;
	 PeakCount = WorkCount;
   }
   //после того как прошли все фильтры нужно убрать лишнюю память из массива пиков
//*outPeaks = new TPeak[PeakCount];
   int* c = (int*)*outPeaksStream;
   c[0] = PeakCount;
   c+=4;
   CopyMemory(c,Peaks,PeakCount*sizeof(TPeak));
   c = c;
   //delete[] Peaks;

 }catch(...){
   //if(NewPeaks) delete[] NewPeaks;
   //if(Peaks) delete[] Peaks;
   PeakCount = 0;
   int* c = (int*)*outPeaksStream;
   c[0] = PeakCount;
  };
 }
*/