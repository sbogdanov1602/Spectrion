#pragma once
using namespace std;

class CExperiment
{
public:
	CExperiment();

	CString m_ExperimentName;
	CString m_ExperimentDir;
	map<int, Ui_Parameter> m_ParamValues;
	void InitDeviceParameters();
	void LoadDeviceParameters();
	void SaveDeviceParameters();
	void Init();
	void Save();
	void Load();
	bool LoadParametersFromDevice();
protected:
	CString m_FileInit = _T("SpecTest.ini");
	CString m_FileInit2 = _T("Spectrion.ini");
	CString m_FileParams = _T("Parameters.ini");
};

