#include "pch.h"
#include "Experiment.h"
#include "SpectrionView.h"

CExperiment::CExperiment()
{
}

void CExperiment::Init()
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

void CExperiment::Save()
{
	pView->SaveInit();
}

void CExperiment::Load()
{
	pView->WorkDirectory = m_ExperimentDir;
	pView->LoadInit();
	pView->OnUpdate();
}

