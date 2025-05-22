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
	CParametersDlg(CWnd* pParent = nullptr, map<int, Ui_Parameter>* paramValues = nullptr);   // standard constructor
	virtual ~CParametersDlg();
	void RefreshDialog(map<int, Ui_Parameter>* paramValues);

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PARAMETERS };
#endif

protected:
	CGridCtrl m_Grid;
	CSize m_szMinimum;
	map<int, int> m_RowParam;
	map<int, Ui_Parameter>* m_pExternalParamValues;
	map<int, Ui_Parameter> m_ParamValues;
	GroupMap* m_groupMapRef;
	GroupParameterInfosMap* m_groupParamsRef;
	ParameterInfosMap* m_paramInfosRef;
	CString m_CurValue;
	bool m_bParamsWereSent;
	
	void PrepareGrid();

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	void FillGrid();
	void LoadComPorts();
	bool ComConfig(void);
	bool LoadParametersFromDevice();
	bool PutParametersToDevice();
	void ReturnDataBack();
	void LoadData();
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnEndEditGrid(NMHDR* pNotifyStruct, LRESULT* result);
	afx_msg void CParametersDlg::OnBeginEditGrid(NMHDR* pNotifyStruct, LRESULT* result);
	afx_msg void OnBnClickedBtnPutToController();
	afx_msg void OnBnClickedBtnLoadFromController();
	CComboBox m_PortName;
	CComboBox m_PortSpeed;
	afx_msg void OnCbnSelchangeCmbSpeed();
	afx_msg void OnCbnSelchangeCmbPort();
	afx_msg void OnBnClickedCancel();
};
