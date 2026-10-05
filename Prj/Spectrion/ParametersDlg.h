#pragma once
#include "afxdialogex.h"
#include "Resource.h"
#include "GridCtrl.h"

const int C_PARAMETERS_NAME = 0;
const int C_PARAMETERS_VALUE = 1;

// ParametersDlg dialog

class CParametersDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CParametersDlg)

public:
	CParametersDlg(CWnd* pParent = nullptr, map<int, Parameter>* paramValues = nullptr, bool blocked = false);   // standard constructor
	virtual ~CParametersDlg();

	void RefreshDialog(map<int, Parameter>* paramValues, bool blocked = false);
	bool IsBlocked() { return m_bBlocked; }

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PARAMETERS };
#endif

protected:
	CString m_sModeName;
	CString m_sExperiment;
	bool m_bBlocked;
	CGridCtrl m_Grid;
	CSize m_szMinimum;
	map<int, int> m_RowParam;
	/*map<int, Ui_Parameter>* */ map<int, Parameter>* m_pExternalParamValues;
	map<int, Ui_Parameter> m_ParamValues;
	GroupMap* m_groupMapRef;
	GroupParameterInfosMap* m_groupParamsRef;
	ParameterInfosMap* m_paramInfosRef;
	CString m_CurValue;
	bool m_bParamsWereSent;

	COLORREF m_defaultBkColour;
	COLORREF m_changedBkColour;
	COLORREF m_notSaveBkColour;

	void PrepareGrid();

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	void FillGrid();
	//void LoadComPorts();
	bool ComConfig(void);
	void ReturnDataBack();
	void LoadData();
	void PrepareGridAndRefresh();
	void BlockDialog();

	virtual bool LoadParametersFromDevice();
	virtual bool PutParametersToDevice();

	DECLARE_MESSAGE_MAP()

	BOOL GetTitleBarRect(RECT& rcTitleBar);
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnEndEditGrid(NMHDR* pNotifyStruct, LRESULT* result);
	afx_msg void CParametersDlg::OnBeginEditGrid(NMHDR* pNotifyStruct, LRESULT* result);
	afx_msg void OnBnClickedBtnPutToController();
	afx_msg void OnBnClickedBtnLoadFromController();
	//CComboBox m_PortName;
	//CComboBox m_PortSpeed;
	//afx_msg void OnCbnSelchangeCmbSpeed();
	//afx_msg void OnCbnSelchangeCmbPort();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnNcPaint();

	CMFCButton m_BtnPut;
	CMFCButton m_BtnLoad;
};
