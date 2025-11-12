#include "pch.h"
#include "ParametersStdDlg.h"
#include "ParamReadWrite.h"

BOOL CParametersStdDlg::OnInitDialog()
{
	CParametersDlg::OnInitDialog();
	
	m_groupMapRef = GetParamGroups();
	m_groupParamsRef = GetGroupParameterInfos();
	m_paramInfosRef = GetParameterInfos();

	PrepareGridAndRefresh();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


bool CParametersStdDlg::LoadParametersFromDevice()
{
	bool ret = false;
	ret = ParamReadWrite.LoadParametersFromDevice();
	if (ret)
	{
		auto paramsValues = ParamReadWrite.GetParameters();
		for (auto it = paramsValues.begin(); it != paramsValues.end(); it++) {
			m_ParamValues[it->first].dirty = false;
			m_ParamValues[it->first].parameter = it->second;
		}
	}
	ParamReadWrite.ClosePort();
	return ret;
}

bool CParametersStdDlg::PutParametersToDevice()
{
	map<int, Parameter> paramValues;
	for (auto it = m_ParamValues.begin(); it != m_ParamValues.end(); it++) {
		if (it->second.dirty) {
			paramValues[it->first] = it->second.parameter;
		}
	}

	m_bParamsWereSent = ParamReadWrite.PutParametersToDevice(paramValues);

	if (m_bParamsWereSent) {
		for (auto it = m_ParamValues.begin(); it != m_ParamValues.end(); it++) {
			it->second.dirty = false;
		}
	}
	ParamReadWrite.ClosePort();

	return m_bParamsWereSent;
}



