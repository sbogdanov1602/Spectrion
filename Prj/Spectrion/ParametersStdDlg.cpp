#include "pch.h"
#include "ParametersStdDlg.h"

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
