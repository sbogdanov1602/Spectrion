// CFileToCchDialog.cpp : implementation file
//

#include "pch.h"
#include "Spectrion.h"
#include "afxdialogex.h"
#include "CFileToCchDialog.h"
#include "DirDialog.h"
#include "CommonFunc.h"


// CFileToCchDialog dialog

IMPLEMENT_DYNAMIC(CFileToCchDialog, CDialogEx)

CFileToCchDialog::CFileToCchDialog(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_FILE_TO_CCH_DIALOG, pParent)
{
	m_IsSmpFilesMustDeleted = false;
}

CFileToCchDialog::~CFileToCchDialog()
{
}

void CFileToCchDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_EDIT1, m_FilesDirectory);
	DDX_Control(pDX, IDC_EDIT2, m_CchFilePath);
}


BEGIN_MESSAGE_MAP(CFileToCchDialog, CDialogEx)
	ON_BN_CLICKED(IDC_BTN_SMP_FILES, OnBnClickedBtnSmpFiles)
	ON_BN_CLICKED(IDC_BTN_CCH_FILE, OnBnClickedBtnCchFile)
	ON_BN_CLICKED(IDC_CHK_DELETE_SMP_FILES, &CFileToCchDialog::OnBnClickedChkDeleteSmpFiles)
END_MESSAGE_MAP()


// CFileToCchDialog message handlers

void CFileToCchDialog::OnBnClickedBtnSmpFiles()
{
	CDirDialog	dlg;
	dlg.SetWindowTitle(_T("Выбор каталога для набора спектров"));
	dlg.SetSelDir(m_SmpFilesDir);

	if (dlg.DoModal(this) == IDOK && m_SmpFilesDir != dlg.GetPathname())
	{
		m_SmpFilesDir = dlg.GetPathname();
		cEnsureDirBackSlash(m_SmpFilesDir);
		m_FilesDirectory.SetWindowText(m_SmpFilesDir);
		EnableButtons();
	}
	int i = 0;
}

void CFileToCchDialog::OnBnClickedBtnCchFile()
{
	CFileDialog dlg(FALSE, _T(".cch"), NULL, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST, _T("(*.cch)|*.cch"), this);
	if (dlg.DoModal() == IDOK /* && m_CchFilePathS != dlg.GetFileName()*/)
	{
		m_CchFilePathS = dlg.GetPathName();
		m_CchFilePath.SetWindowText(m_CchFilePathS);
		EnableButtons();
	}
}

bool CFileToCchDialog::ValidateFilesDirectory()
{
	auto s = GetSmpFilesDirectory();
	return s.GetLength() > 0;
}

bool CFileToCchDialog::ValidateCchFilePath()
{
	auto s = GetCchFilePath();
	return s.GetLength() > 0;
}

void CFileToCchDialog::EnableButtons()
{
	bool b = ValidateCchFilePath() && ValidateFilesDirectory();
	GetDlgItem(IDOK)->EnableWindow(b);
}

CString CFileToCchDialog::GetSmpFilesDirectory()
{
	return m_SmpFilesDir;
}

CString CFileToCchDialog::GetCchFilePath()
{
	return m_CchFilePathS;
}


BOOL CFileToCchDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	
	TCHAR currentDir[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, currentDir);
	m_SmpFilesDir = currentDir;
	cEnsureDirBackSlash(m_SmpFilesDir);
	m_FilesDirectory.SetWindowText(m_SmpFilesDir);
	EnableButtons();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CFileToCchDialog::OnBnClickedChkDeleteSmpFiles()
{
	m_IsSmpFilesMustDeleted = ((CButton*)GetDlgItem(IDC_CHK_DELETE_SMP_FILES))->GetCheck();
}
