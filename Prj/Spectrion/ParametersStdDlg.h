#pragma once
#include "ParametersDlg.h"

class CParametersStdDlg : public CParametersDlg
{

public:
	CParametersStdDlg(CWnd* pParent = nullptr, map<int, Ui_Parameter>* paramValues = nullptr)
		: CParametersDlg(pParent, paramValues) {};   // standard constructor
	virtual BOOL OnInitDialog();
};

