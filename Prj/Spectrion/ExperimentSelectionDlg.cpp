// ExperimentSelectionDlg.cpp : implementation file
//

#include "pch.h"
#include "Spectrion.h"
#include "afxdialogex.h"
#include "ExperimentSelectionDlg.h"
#include "CommonFunc.h"

// CExperimentSelectionDlg dialog

IMPLEMENT_DYNAMIC(CExperimentSelectionDlg, CDialogEx)

CExperimentSelectionDlg::CExperimentSelectionDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_EXPERIMENT_SELECTION, pParent)
{

}

CExperimentSelectionDlg::~CExperimentSelectionDlg()
{
}

void CExperimentSelectionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_NEW_EXPERIMENT, m_IsNewExperiment);
	DDX_Control(pDX, IDC_STATIC_NAME, m_StaticName);
	DDX_Control(pDX, IDC_EXPERIMENT_NAME, m_ExperimentName);
	DDX_Control(pDX, IDC_STATIC_SELECTION, m_StaticSelect);
	DDX_Control(pDX, IDC_CMB_EXPERIMENT, m_CmbExperiment);
}


BEGIN_MESSAGE_MAP(CExperimentSelectionDlg, CDialogEx)
	ON_BN_CLICKED(IDC_NEW_EXPERIMENT, &CExperimentSelectionDlg::OnBnClickedNewExperiment)
	ON_BN_CLICKED(IDOK, &CExperimentSelectionDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CExperimentSelectionDlg::OnBnClickedCancel)
	ON_EN_CHANGE(IDC_EXPERIMENT_NAME, &CExperimentSelectionDlg::OnEnChangeExperimentName)
END_MESSAGE_MAP()


void CExperimentSelectionDlg::OnBnClickedNewExperiment()
{
	if (m_IsNewExperiment.GetCheck() > 0) {
		m_StaticName.ShowWindow(SW_SHOW);
		m_ExperimentName.ShowWindow(SW_SHOW);
		m_StaticSelect.ShowWindow(SW_HIDE);
		m_CmbExperiment.ShowWindow(SW_HIDE);
	}
	else {
		m_StaticName.ShowWindow(SW_HIDE);
		m_ExperimentName.ShowWindow(SW_HIDE);
		m_StaticSelect.ShowWindow(SW_SHOW);
		m_CmbExperiment.ShowWindow(SW_SHOW);
	}
}

BOOL CExperimentSelectionDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	int idx = CB_ERR;
	m_IsNewExperiment.SetCheck(0);
	if (FindSubFolders(theApp.m_DataDir, m_Experiments)) {
		auto count = m_Experiments.GetCount();
		for (int i = 0; i < count; i++) {
			auto s = m_Experiments.GetAt(m_Experiments.FindIndex(i));
			s = s.Right(s.GetLength() - s.ReverseFind('\\') - 1);
			if (theApp.m_pExperiment != nullptr) {
				if (s == theApp.m_pExperiment->m_ExperimentName) {
					idx = i;
				}
			}
			m_CmbExperiment.InsertString(i, s);
		}
		if (count > 0) {
			if (idx != CB_ERR) {
				m_CmbExperiment.SetCurSel(idx);
			}
			else {
				m_CmbExperiment.SetCurSel(0);
			}
		}
	}
	OnBnClickedNewExperiment();
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CExperimentSelectionDlg::OnBnClickedOk()
{
	UpdateData(TRUE);
	m_bIsNewExperiment = m_IsNewExperiment.GetCheck() > 0;
	if (m_bIsNewExperiment) {
		m_ExperimentName.GetWindowText(m_ExperimentNameOut);
	}
	else {
		m_CmbExperiment.GetLBText(m_CmbExperiment.GetCurSel(), m_ExperimentNameOut);
	}

	m_ExperimentNameOut.Trim();
	if (!m_bIsNewExperiment) {
		CDialogEx::OnOK();
	}
	else {
		int pos = m_CmbExperiment.FindString(0, m_ExperimentNameOut);
		if (pos >= 0) {
			cPCR_Error(L"Эксперимент с таким именем уже существует. Измените имя.");
		}
		else {
			CDialogEx::OnOK();
		}
	}
}

void CExperimentSelectionDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnCancel();
}

CString CExperimentSelectionDlg::GetExperimentName()
{
	return m_ExperimentNameOut;
}

void CExperimentSelectionDlg::OnEnChangeExperimentName()
{
	CString s;
	m_ExperimentName.GetWindowText(s);
	if (Validate(s)) {
		m_ExperimentNameOut = s;
	}
	else {
		cPCR_Error(L"Имя эксперимента может содержать только буквы, цифры или символ '_'.");
		// L"Имя эксперимента может содержать только буквы, цифры или символ '_'."
		m_ExperimentName.SetWindowText(m_ExperimentNameOut);
	}
}

bool CExperimentSelectionDlg::Validate(CString s)
{
	bool ret = true;
	for (int i = 0; i < s.GetLength() && ret; i++) {
		_TCHAR c = s[i];
		ret = ret && (iswxdigit(c) || iswalpha(c) || c == '_');
	}
	return ret;
}
