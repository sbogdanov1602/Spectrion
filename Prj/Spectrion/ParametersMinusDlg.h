#pragma once
#include "ParametersDlg.h"
class CParametersMinusDlg :
    public CParametersDlg
{
	CParametersMinusDlg(CWnd* pParent = nullptr, map<int, Ui_Parameter>* paramValues = nullptr)
		: CParametersDlg(pParent, paramValues) {};   // standard constructor

public:
	virtual BOOL OnInitDialog();
};

