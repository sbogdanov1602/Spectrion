#pragma once
#include "ParametersDlg.h"
class CParametersMinusDlg :
    public CParametersDlg
{
public:
	CParametersMinusDlg(CWnd* pParent = nullptr, map<int, Ui_Parameter>* paramValues = nullptr)
		: CParametersDlg(pParent, paramValues) {};   // standard constructor

	virtual BOOL OnInitDialog();
};

