#include "pch.h"
#include "Experiment.h"
#include "SpectrionView.h"
#include "CommonFunc.h"
#include "FileSystem.h"
#include "DfxCached.h"

using namespace filesystem;

CExperiment::CExperiment()
{
}

void CExperiment::Init()
{
	InitDeviceParameters();
	pView->Init();
	pView->OnUpdate();
	pView->UpdateData(FALSE);
}

void CExperiment::InitDeviceParameters()
{
	auto groupMapRef = GetParamGroups();
	auto groupParamsRef = GetGroupParameterInfos();
	auto paramInfosRef = GetParameterInfos();

	for (auto it = paramInfosRef->begin(); it != paramInfosRef->end(); it++) {
		Ui_Parameter ui_param;
		ui_param.dirty = false;
		ui_param.parameter.paramId = it->second.paramId;
		switch (it->second.type)
		{
		case ParamType::i:
		{
			ui_param.parameter.iValue = 0;
			break;
		}
		case ParamType::b:
		{
			ui_param.parameter.bValue = false;
			break;
		}
		default:
			break;
		}
		m_ParamValues[ui_param.parameter.paramId] = ui_param;
	}
}

void CExperiment::LoadDeviceParameters()
{
	CString fname = MakePath(m_ExperimentDir, ParametersIniName);
	if (IsExistFile(fname)) 
	{
		auto paramInfosRef = GetParameterInfos();
		CDfxCache dfx(fname, 'w');

		CString Section = _T("Set");
		int i, N = dfx.GetProfileIntW(fname, Section, _T("Count"), 0);
		m_ParamValues.clear();
		for (auto it = paramInfosRef->begin(); it != paramInfosRef->end(); it++) {
			Ui_Parameter ui_param;
			ui_param.dirty = false;
			ui_param.parameter.paramId = it->second.paramId;

			Section.Format(_T("%d"), ui_param.parameter.paramId);
			int itmpi = dfx.GetProfileIntW(fname, Section, _T("Value"), -1);
			if (itmpi < 0) {
				continue;
			}
			switch (it->second.type)
			{
			case ParamType::i:
			{
				ui_param.parameter.iValue = itmpi;
				break;
			}
			case ParamType::b:
			{
				ui_param.parameter.bValue = (itmpi == 1) ? true : false;
				break;
			}
			default:
				break;
			}
			m_ParamValues[ui_param.parameter.paramId] = ui_param;

			CString displayName = dfx.GetProfileStringW(fname, Section, _T("DisplayName"), _T(""));
		}
	}
	else 
	{
		InitDeviceParameters();
	}
}

void CExperiment::SaveDeviceParameters()
{
	CString fname = MakePath(m_ExperimentDir, ParametersIniName);
	::DeleteFile(fname);

	CDfxCache dfx(fname, 'w');

	CString Section = _T("Set");
	int N = int(m_ParamValues.size());
	dfx.DfxPutProfileIntW(fname, Section, _T("Count"), N);
	auto paramInfosRef = GetParameterInfos();

	for (auto it = m_ParamValues.begin(); it != m_ParamValues.end(); it++)
	{
		auto paramInfo = (*paramInfosRef)[it->first];

		Section.Format(_T("%d"), it->first);
		int val = (paramInfo.type == ParamType::i) ? it->second.parameter.iValue : (int)(it->second.parameter.bValue);
		CString s = paramInfo.displayName.c_str();
		dfx.DfxPutProfileStringW(fname, Section, _T("DisplayName"), s);
		dfx.DfxPutProfileIntW(fname, Section, _T("Value"), val);
	}
}

void CExperiment::Save()
{
	pView->WorkDirectory = m_ExperimentDir;
	pView->SaveInit();
	SaveDeviceParameters();
}

void CExperiment::Load()
{
	LoadDeviceParameters();
	pView->WorkDirectory = m_ExperimentDir;
	pView->SpectraCursorsLegendDelete();
	pView->LoadInit();
	pView->OnUpdate();
	pView->UpdateData(FALSE);
}

