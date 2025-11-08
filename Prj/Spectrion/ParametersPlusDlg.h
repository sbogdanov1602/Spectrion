#pragma once
#include "ParametersDlg.h"
class CParametersPlusDlg :
    public CParametersDlg
{
public:
	CParametersPlusDlg(CWnd* pParent = nullptr, map<int, Ui_Parameter>* paramValues = nullptr)
		: CParametersDlg(pParent, paramValues) {
	};   // standard constructor

	virtual BOOL OnInitDialog();
};

