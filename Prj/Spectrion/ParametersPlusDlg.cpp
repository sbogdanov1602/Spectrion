#include "pch.h"
#include "ParametersPlusDlg.h"

BOOL CParametersPlusDlg::OnInitDialog()
{
	CParametersDlg::OnInitDialog();

	m_groupMapRef = GetParamPlusGroups();
	m_groupParamsRef = GetGroupParameterPlusInfos();
	m_paramInfosRef = GetParameterPlusInfos();

	PrepareGridAndRefresh();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
