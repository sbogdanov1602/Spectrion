#pragma once
#include <afxstr.h>
#include "ProgressBarDlg.h"

UINT ConvertSmpToCch(LPVOID pParam);

struct ThreadData
{
	CString SmpDir;
	CString CchFilePath;
	ProgressBarDlg* pProgressBarDlg;
	HWND hWnd;
	bool SmpFilesMustDelete;
};