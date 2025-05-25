// COMParamsDlg.cpp : implementation file
//

#include "pch.h"
#include "Spectrion.h"
#include "afxdialogex.h"
#include "COMParamsDlg.h"
#include "ParamReadWrite.h"


// CCOMParamsDlg dialog

IMPLEMENT_DYNAMIC(CCOMParamsDlg, CDialogEx)

CCOMParamsDlg::CCOMParamsDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_COM_PARAMS, pParent)
{

}

CCOMParamsDlg::~CCOMParamsDlg()
{
}

void CCOMParamsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CMB_PORT, m_CmbPortName);
	DDX_Control(pDX, IDC_CMB_SPEED, m_CmbPortSpeed);
}


BEGIN_MESSAGE_MAP(CCOMParamsDlg, CDialogEx)
	ON_BN_CLICKED(IDCANCEL, &CCOMParamsDlg::OnBnClickedCancel)
	ON_CBN_SELCHANGE(IDC_CMB_SPEED, &CCOMParamsDlg::OnCbnSelchangeCmbSpeed)
	ON_CBN_SELCHANGE(IDC_CMB_PORT, &CCOMParamsDlg::OnCbnSelchangeCmbPort)
END_MESSAGE_MAP()


// CCOMParamsDlg message handlers

void CCOMParamsDlg::OnBnClickedCancel()
{
	CString s;
	m_CmbPortSpeed.GetWindowText(s);
	int portSpeed = _ttoi(s);
	ParamReadWrite.SetPortSpeed(portSpeed);

	CString portName;
	m_CmbPortName.GetWindowText(portName);
	if (!portName.IsEmpty()) {
		ParamReadWrite.SetPortName(portName);
	}
	CDialogEx::OnCancel();
}

void CCOMParamsDlg::OnCbnSelchangeCmbSpeed()
{
	CString s;
	m_CmbPortSpeed.GetWindowText(s);
	int portSpeed = _ttoi(s);
	ParamReadWrite.SetPortSpeed(portSpeed);
}

void CCOMParamsDlg::OnCbnSelchangeCmbPort()
{
	CString portName;
	m_CmbPortName.GetWindowText(portName);
	ParamReadWrite.SetPortName(portName);
}

BOOL CCOMParamsDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	LoadComPorts();
	if (m_CmbPortSpeed.GetCount() > 0) {
		m_CmbPortSpeed.SetCurSel(m_CmbPortSpeed.GetCount() - 1);
		CString s;
		m_CmbPortSpeed.GetWindowText(s);
		int portSpeed = _ttoi(s);
		ParamReadWrite.SetPortSpeed(portSpeed);
	}
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CCOMParamsDlg::LoadComPorts()
{
	CList<CString>& portNames = ParamReadWrite.LoadComPorts();
	for (int i = 0; i < portNames.GetCount(); i++) {
		auto portName = portNames.GetAt(portNames.FindIndex(i));
		m_CmbPortName.AddString(portName);
	}
	if (m_CmbPortName.GetCount() > 0) {
		CString portName;
		portName = ParamReadWrite.GetPortName();
		if (!portName.IsEmpty()) {
			int pos = m_CmbPortName.FindString(0, portName);
			if (pos >= 0) {
				m_CmbPortName.SetCurSel(pos);
			}
			else {
				m_CmbPortName.SetCurSel(0);
				m_CmbPortName.GetWindowText(portName);
				ParamReadWrite.SetPortName(portName);
			}
		}
		else {
			m_CmbPortName.SetCurSel(0);
			m_CmbPortName.GetWindowText(portName);
			ParamReadWrite.SetPortName(portName);
		}
	}
}