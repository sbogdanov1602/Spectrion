#include "pch.h"
#include "ParametersMinusDlg.h"

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
