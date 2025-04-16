#pragma once
#include "afxdialogex.h"


// ProgressBarDlg dialog

class ProgressBarDlg : public CDialog
{
	DECLARE_DYNAMIC(ProgressBarDlg)

public:
	ProgressBarDlg(CWnd* pParent = nullptr, LPCTSTR processName = L"", int maxBarValue = 100, int step = 1);   // standard constructor
	virtual ~ProgressBarDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PROGRESS_BAR_DLG };
#endif

protected:
	CProgressCtrl m_ProgressBar;
	int m_MaxBarValue;
	int m_Step;
	CString m_ProcessName;
	bool m_Stopped;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	bool IsStopped() const { return m_Stopped; };
	void StepIt();
	void SetMaxBarValue(int maxBarValue);
	afx_msg void OnBnClickedAbort();
};
