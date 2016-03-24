// InputDlg.cpp : implementation file
//

#include "stdafx.h"
#include "IPCM.h"
#include "InputDlg.h"
#include "afxdialogex.h"


// CInputDlg dialog

IMPLEMENT_DYNAMIC(CInputDlg, CDialogEx)

CInputDlg::CInputDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_INPUT_STREAM, pParent)
{

}

CInputDlg::~CInputDlg()
{
}

void CInputDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_INPUT, m_input);
}

CString CInputDlg::GetInputString()
{
	return m_strinput;
}

void CInputDlg::SetInputString(CString str)
{
	SetDlgItemText(IDC_EDIT_INPUT, str);
	UpdateData(FALSE);;
}

BEGIN_MESSAGE_MAP(CInputDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CInputDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CInputDlg message handlers


void CInputDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here

	UpdateData(TRUE);
	GetDlgItemText(IDC_EDIT_INPUT, m_strinput);
	CDialogEx::OnOK();
}


