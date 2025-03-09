#pragma once
#include "afxdialogex.h"


// CFileToCchDialog dialog

class CFileToCchDialog : public CDialogEx
{
	DECLARE_DYNAMIC(CFileToCchDialog)

public:
	CFileToCchDialog(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CFileToCchDialog();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_FILE_TO_CCH_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	virtual BOOL OnInitDialog();
	void EnableButtons();
	bool ValidateCchFilePath();
	bool ValidateFilesDirectory();

	CString m_SmpFilesDir;
	CString m_CchFilePathS;
public:
	CEdit m_FilesDirectory;
	CEdit m_CchFilePath;

	CString GetSmpFilesDirectory();
	CString GetCchFilePath();

	afx_msg void OnBnClickedBtnSmpFiles();
	afx_msg void OnBnClickedBtnCchFile();
};
