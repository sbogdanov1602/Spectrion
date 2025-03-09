#pragma once
#include <afxstr.h>

UINT ConvertSmpToCch(LPVOID pParam);

struct ThreadData
{
	CString SmpDir;
	CString CchFilePath;
};