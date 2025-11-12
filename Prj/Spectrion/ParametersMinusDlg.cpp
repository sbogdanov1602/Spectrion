#include "pch.h"
#include "ParametersMinusDlg.h"
#include "ParamReadWrite.h"

BOOL CParametersMinusDlg::OnInitDialog()
{
	CParametersDlg::OnInitDialog();

	m_groupMapRef = GetParamMinusGroups();
	m_groupParamsRef = GetGroupParameterMinusInfos();
	m_paramInfosRef = GetParameterMinusInfos();

	PrepareGridAndRefresh();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

bool CParametersMinusDlg::LoadParametersFromDevice()
{
	bool ret = false;
	ret = ParamReadWrite.Load4ByteParametersFromDevice(m_paramInfosRef);
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

bool CParametersMinusDlg::PutParametersToDevice()
{
	map<int, Parameter> paramValues;
	for (auto it = m_ParamValues.begin(); it != m_ParamValues.end(); it++) {
		if (it->second.dirty) {
			paramValues[it->first] = it->second.parameter;
		}
	}

	m_bParamsWereSent = ParamReadWrite.Put4ByteParametersToDevice(paramValues, m_paramInfosRef);

	if (m_bParamsWereSent) {
		for (auto it = m_ParamValues.begin(); it != m_ParamValues.end(); it++) {
			it->second.dirty = false;
		}
	}
	ParamReadWrite.ClosePort();

	return CParametersDlg::PutParametersToDevice();
}

