///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
//
// CompressAlg.cpp ontains a description of the CompressAlg class static methods 
// and some inline methods 
// 
// Author  :   Nikita Shulgin      :    ShulginNA@Lumex.ru
// Company :   Lumex Instruments   :    https://www.lumexinstruments.com/
//
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include <iostream>
#include "CompressAlg.h"

int CompressAlg::destBufCount = 500000;
int* CompressAlg::destBufCompress_ = new int[destBufCount];


int intOffset, bitOffset;
unsigned int* Dest;
unsigned int* Source;
const int Mask = 0xFFFFFFFE;
int MaxCodeLen;

int CodeTable[64];

int CodeTableFlat[] =
{ 0,5,  1,5,  2,5,  3,5,
 4,5,  5,5,  6,5,  7,5,  8,5,
 9,5, 10,5, 11,5, 12,5,
13,5, 14,5, 15,5, 16,5,
17,5, 18,5, 19,5, 20,5,
21,5, 22,5, 23,5, 24,5,
25,5, 26,5, 27,5, 28,5,
29,5, 30,5, 31,5 };

int CodeTable5[] =
{ 0,1,  4,3,  5,3,  6,3, 7,3 };

int CodeTable7[] =
{ 0,1,  4,3,  5,3,  12,4, 13,4, 14,4, 15,4 };

int CodeTable9[] =
{ 0,1,  4,3,  5,3,  12,4, 13,4, 28,5, 29,5, 30,5, 31,5 };

int CodeTable11[] =
{ 0,1,  4,3,  5,3,  24,5, 25,5, 26,5, 27,5, 28,5, 29,5, 30,5, 31,5 };

int CodeTable15[] =
{ 0,1,  4,3,  5,3, 24,5,
25,5, 26,5, 27,5, 56,6,
57,6, 58,6, 59,6, 60,6,
61,6, 62,6, 63,6 };

int CodeTable32[] =
{ 0,1,   4,3,   5,3,  24,5,
 25,5,  26,5,  27,5,  56,6,
 57,6,  58,6,  59,6, 240,8,
241,8, 242,8, 243,8, 244,8,
245,8, 246,8, 247,8, 496,9,
497,9, 498,9, 499,9, 500,9,
501,9, 502,9, 503,9, 504,9,
505,9, 506,9, 507,9, 508,9
};

int* DecodeTable;

//=================================================================================
inline void AddWord(int Word, int Len) {
	if (Len <= bitOffset) {

		bitOffset -= Len;
		Dest[intOffset] |= Word << bitOffset;
	}
	else {

		Dest[intOffset] |= Word >> (Len - bitOffset);
		intOffset++;
		bitOffset = 32 - (Len - bitOffset);
		Dest[intOffset] = Word << bitOffset;
	}
}
//---------------------------------------------------------------------------------
inline void AddSeq(int Word) {
	int Len, Buf = Word;
	for (Len = 0; Buf > 0; Len++, Buf = Buf >> 1);
	if (Word > 1) {

		Word &= (Mask << (Len - 1)) | (0xFFFFFFFF >> (33 - Len));

		AddWord(CodeTable[Len << 1], CodeTable[(Len << 1) + 1]);

		AddWord(Word, Len - 1);
	}
	else {
		if (Word) {

			AddWord(CodeTable[2], CodeTable[3]);
		}
		else {

			AddWord(CodeTable[0], CodeTable[1]);
		}
	}
}
//---------------------------------------------------------------------------------
//извлекает слово длиной Length из битовой последовательности
inline int GetWord(int Length) {
	if (Length == 0) return 0;
	if (Length <= bitOffset) {
		//чистка старших разрядов
		return (Source[intOffset] << (32 - bitOffset))
			//чистка младших разрядов
			>> (32 - Length);
	}
	else {
		return ((Source[intOffset] << (32 - bitOffset)) >>
			(32 - Length)) |
			(Source[intOffset + 1] >> (32 - (Length - bitOffset)));
	}
}

//---------------------------------------------------------------------------------
//сдвигает битовый указатель на Offs позиций offs<32
inline void MovePointer(int Offs) {
	if (bitOffset > Offs) {
		bitOffset -= Offs;
	}
	else {
		Offs -= bitOffset;
		intOffset++;
		bitOffset = 32 - Offs;
	}
}

//---------------------------------------------------------------------------------
//функция извлекает из упаковки очередное слово и возвращает его как результат работы
inline int GetSeq()
{
	//берем из битовой последовательности бит по максимальной длине кода
	int Prefix = GetWord(MaxCodeLen);
	//вытаскиваем из таблицы декодирования - фактичесекую длину кода и слова
	//передвигаем битовый указатель на длину кода
	MovePointer(DecodeTable[Prefix << 1]);
	//разбираемся с нулями
	if (DecodeTable[(Prefix << 1) + 1] == 0) return 0;
	//извлекаем слово
	int Word = GetWord(DecodeTable[(Prefix << 1) + 1] - 1);
	//передвигаем битовый указатель на слово
	MovePointer(DecodeTable[(Prefix << 1) + 1] - 1);
	//Добавляем единицу старшего разряда (можно поставить += но не знаю что будет эффективнее)
	Word |= 1 << (DecodeTable[(Prefix << 1) + 1] - 1);
	return Word;
}
//таблица декодирования имеет следующий формат:
//четные - длинна кода, нечетные - длинна слова за этим кодом
//---------------------------------------------------------------------------------
void MakeDecodingTable(int* CodeTable, int** DecodeTable, int Length) {
	//Выясняем максимальную длинну слова
	int Max = MaxCodeLen;
	//создаем таблицу потребного размера
	(*DecodeTable) = new int[1 << (Max + 1)];
	for (int i = 0; i < Length; i++) {
		if (!CodeTable[(i << 1) + 1]) continue;
		//рассчитываем начало фрагмента кодовой таблицы от i - го кода
		int Offs = (CodeTable[i << 1] << (Max - CodeTable[(i << 1) + 1]));
		//Для каждого слова заполняем его фрагмегнт таблицы декодирования
		for (int j = 0; j < (1 << (Max - CodeTable[(i << 1) + 1])); j++) {
			//помещаем длину кода
			(*DecodeTable)[(j + Offs) << 1] = CodeTable[(i << 1) + 1];
			//помещаем длину слова
			(*DecodeTable)[((j + Offs) << 1) + 1] = i;
		}
	}
}
//---------------------------------------------------------------------------------
int DataCompression(int* Dst, int* Src, int DstLen, int SrcLen)
{
	int Len, Sr;
	int Stat[32];
	//ZeroMemory(Stat, 128);
	memset(Stat, 0, 128);
	//ZeroMemory(CodeTable, 256);
	memset(CodeTable, 0, 256);

	for (int i = 0; i < SrcLen; i++) {
		Len = 0;
		for (Sr = Src[i]; Sr > 0; Len++) Sr = Sr >> 1;

		Stat[Len]++;
	}

	int Count = 0;
	for (int i = 0; i < 32; i++) {
		if (Stat[i] != 0) Count++;
	}

	int* WorkCodeTable;

	if (Count <= 5) { WorkCodeTable = CodeTable5; MaxCodeLen = 3; }
	else
		if (Count <= 7) { WorkCodeTable = CodeTable7; MaxCodeLen = 4; }
		else
			if (Count <= 9) { WorkCodeTable = CodeTable9; MaxCodeLen = 5; }
			else
				if (Count <= 11) { WorkCodeTable = CodeTable11; MaxCodeLen = 5; }
				else
					if (Count <= 15) { WorkCodeTable = CodeTable15; MaxCodeLen = 6; }
					else { WorkCodeTable = CodeTable32; MaxCodeLen = 9; }

	//ZeroMemory(CodeTable, 64);
	memset(CodeTable, 0, 64);
	int Max;

	CodeTable[0] = WorkCodeTable[0];
	CodeTable[1] = WorkCodeTable[1];
	for (int i = 1; i < Count; i++) {
		Max = 1;
		for (int j = 1; j < 32; j++) {
			if (Stat[j] >= Stat[Max]) Max = j;
		}
		CodeTable[(Max << 1)] = WorkCodeTable[i << 1];
		CodeTable[(Max << 1) + 1] = WorkCodeTable[(i << 1) + 1];
		Stat[Max] = 0;
	}

	Dst[0] = SrcLen;
	bitOffset = 32;
	intOffset = 1;
	Dest = (unsigned int*)Dst;
	Source = (unsigned int*)Src;

	AddWord(MaxCodeLen, 4);
	AddWord(Count, 5);
	AddWord(CodeTable[0], MaxCodeLen);
	AddWord(CodeTable[1], 4);
	for (int i = 2; i < 64; i += 2) {
		if (CodeTable[i]) {
			AddWord(i >> 1, 5);
			AddWord(CodeTable[i], MaxCodeLen);
			AddWord(CodeTable[i + 1], 4);
		}
	}

	for (int i = 0; i < SrcLen; i++) {
		AddSeq(Source[i]);
	}
	return intOffset;
}
//---------------------------------------------------------------------------------
int DataDecompression(int** Dst, int* Src)
{
	//(*Dst) = new int[Src[0]+1];
	Dest = (unsigned int*)(*Dst);
	Source = (unsigned int*)Src;
	bitOffset = 32;
	intOffset = 1;
	//ZeroMemory(CodeTable, 256);
	memset(CodeTable, 0, 256);
	MaxCodeLen = GetWord(4);
	MovePointer(4);
	int Count = GetWord(5);
	MovePointer(5);
	CodeTable[0] = GetWord(MaxCodeLen);
	MovePointer(MaxCodeLen);
	CodeTable[1] = GetWord(4);
	MovePointer(4);
	for (int i = 1; i < Count; i++) {
		int Index = GetWord(5);
		MovePointer(5);
		CodeTable[Index << 1] = GetWord(MaxCodeLen);
		MovePointer(MaxCodeLen);
		CodeTable[(Index << 1) + 1] = GetWord(4);
		MovePointer(4);
	}

	MakeDecodingTable(CodeTable, &DecodeTable, 32);
	for (int i = 0; i < Src[0]; i++) {
		Dest[i] = GetSeq();
	}
	//ERR4   
	delete[] DecodeTable;
	return Src[0];
}
//---------------------------------------------------------------------------------
int ZeroSupression(int* Dest, int* Src, int DstLen, int SrcLen, int Noiselevel)
{
	int i, DCount, ZCount;
	bool ZeroSupressionOk = false;
	do {
		DCount = 0; ZCount = 0;
		for (i = 0; i < SrcLen; i++) {
			if (DCount > DstLen) { Noiselevel++; break; }
			if (Src[i] > Noiselevel) {
				if (ZCount > 0) {
					Dest[DCount] = 0;
					DCount++;
					Dest[DCount] = ZCount;
					DCount++;
					ZCount = 0;
				}
				Dest[DCount] = Src[i];
				DCount++;
			}
			else {
				ZCount++;
			}
		}
	} while (i != SrcLen);

	//конечные нули - тоже добавляем
	if (ZCount > 0) {
		Dest[DCount] = 0;
		DCount++;
		Dest[DCount] = ZCount;
		DCount++;
		ZCount = 0;
	}

	return DCount;
}
//---------------------------------------------------------------------------------
// Здесь буферы Dest и Src должны быть разными
// Возврат - число сжатых символов в Src
// если он больше чем размер Dest то информация
// расшифрована неполностью

int ZeroDepression(int* Dest, int* Src, int DstLen, int SrcLen)
{
	int DstCount = 0;
	memset(Dest, 0, DstLen * sizeof(int));
	for (int i = 0; (i < SrcLen) && (DstCount < DstLen); i++) {
		if (Src[i] == 0) {
			i++;
			DstCount += Src[i];
		}
		else {
			Dest[DstCount] = Src[i];
			DstCount++;
		}
	}
	return DstCount;
}
//=================================================================================
//=================================================================================
int* cdata = nullptr, cdataBufCount = 0;

void* CompressAlg::GetCompressed(int* Data, int DataLength, int* PacketLength, int PacketType) {
	//int* adata;
	int Len = 0;
	switch (PacketType) {
	case 1:
		if (DataLength > cdataBufCount) {
			if (cdata)
				delete[] cdata;
			cdata = new int[DataLength];
			cdataBufCount = DataLength;
		}
		Len = ZeroSupression(cdata, Data, DataLength, DataLength, 0);
		break;

		/*      case 2:
				 cdata=PressData(&Len);
				 break;

			  case 3:
				 MassBuild(&adata,&Len);
				 cdata = new int[Len*2];
				 Len = ZeroSupression(cdata,adata,Len*2,Len,NoiseLevel);
				 delete[] adata;
				 break;
		*/
	}

	if ((Len) > destBufCount) 
	{
		if (destBufCompress_)
			delete[] destBufCompress_;
		destBufCount = Len;
		destBufCompress_ = new int[Len];
		
	}

	//ZeroMemory(dest, (Len) * 4);
	memset(destBufCompress_, 0, (Len) * 4);
	int CLen = DataCompression(destBufCompress_, cdata, Len, Len);
	*PacketLength = CLen + 1;
	return destBufCompress_;
}
//---------------------------------------------------------------------------------]
int* CompressAlg::GetDecompressed(int* Block, int ReturnedLength) {

	if (Block[0] > cdataBufCount) {
		if (cdata) delete[] cdata;
		cdata = new int[(Block[0]) + 1];
		cdataBufCount = Block[0] + 1;
		//ZeroMemory(cdata, Block[0] + 1);
		memset(cdata, 0, (Block[0]) + 1);
	}
	int Flen = DataDecompression(&cdata, Block);
	if (ReturnedLength > destBufCount) {
		if (destBufCompress_) 
			delete[] destBufCompress_;
		destBufCompress_ = new int[ReturnedLength];
		destBufCount = ReturnedLength;
	}
	ZeroDepression(destBufCompress_, cdata, ReturnedLength, Flen);
	return destBufCompress_;
}
//=================================================================================

