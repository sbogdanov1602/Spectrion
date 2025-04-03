#pragma once
#include "afxdialogex.h"
#include "Resource.h"
#include "GridCtrl.h"

const int C_PARAMETERS_NAME = 0;
const int C_PARAMETERS_VALUE = 1;

// ParametersDlg dialog

class CParametersDlg : public CDialog
{
	DECLARE_DYNAMIC(CParametersDlg)

public:
	CParametersDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CParametersDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PARAMETERS };
#endif

protected:
	CGridCtrl m_Grid;
	CSize m_szMinimum;
	map<int, int> m_RowParam;
	map<int, Ui_Parameter> m_ParamValues;
	GroupMap& m_groupMapRef = GetParamGroups();
	GroupParameterInfosMap& m_groupParamsRef = GetGroupParameterInfos();
	ParameterInfosMap& m_paramInfosRef = GetParameterInfos();
	
	void CParametersDlg::PrepareGrid();

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	void FillGrid();

	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnEndEditGrid(NMHDR* pNotifyStruct, LRESULT* result);
	afx_msg void CParametersDlg::OnBeginEditGrid(NMHDR* pNotifyStruct, LRESULT* result);
	afx_msg void OnBnClickedBtnPutToController();
	afx_msg void OnBnClickedBtnLoadFromController();
};
