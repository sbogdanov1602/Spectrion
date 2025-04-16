// ProgressBarDlg.cpp : implementation file
//

#include "pch.h"
#include "Spectrion.h"
#include "afxdialogex.h"
#include "ProgressBarDlg.h"


// ProgressBarDlg dialog

IMPLEMENT_DYNAMIC(ProgressBarDlg, CDialog)

ProgressBarDlg::ProgressBarDlg(CWnd* pParent /*=nullptr*/, LPCTSTR processName /*=L""*/, int maxBarValue /*=100*/, int step /*=1*/)
	: CDialog(IDD_PROGRESS_BAR_DLG, pParent)
{
	m_MaxBarValue = maxBarValue;
	m_Step = step;
	m_ProcessName = processName;
	m_Stopped = false;
}

ProgressBarDlg::~ProgressBarDlg()
{
}

void ProgressBarDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS_BAR, m_ProgressBar);
}

BOOL ProgressBarDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	GetDlgItem(IDC_PROCESS_NAME)->SetWindowText(m_ProcessName);
	m_ProgressBar.SetRange(1, m_MaxBarValue);
	m_ProgressBar.SetStep(m_Step);
	return true;
}

BEGIN_MESSAGE_MAP(ProgressBarDlg, CDialog)
	ON_BN_CLICKED(IDABORT, &ProgressBarDlg::OnBnClickedAbort)
END_MESSAGE_MAP()


// ProgressBarDlg message handlers

void ProgressBarDlg::StepIt() 
{
	m_ProgressBar.StepIt(); 
//	GetDlgItem(IDABORT)->SetFocus();
}

void ProgressBarDlg::SetMaxBarValue(int maxBarValue)
{
	int min, max;
	m_ProgressBar.GetRange(min, max);
	m_ProgressBar.SetRange(min, maxBarValue);
}

void ProgressBarDlg::OnBnClickedAbort()
{
	m_Stopped = true;
}
