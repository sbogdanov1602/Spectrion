#pragma once
#include "afxdialogex.h"


// CCOMParamsDlg dialog

class CCOMParamsDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CCOMParamsDlg)

public:
	CCOMParamsDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CCOMParamsDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_COM_PARAMS };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	void LoadComPorts();

	DECLARE_MESSAGE_MAP()

	CComboBox m_CmbPortName;
	CComboBox m_CmbPortSpeed;
public:
	afx_msg void OnBnClickedCancel();
	afx_msg void OnCbnSelchangeCmbSpeed();
	afx_msg void OnCbnSelchangeCmbPort();
	virtual BOOL OnInitDialog();
};
