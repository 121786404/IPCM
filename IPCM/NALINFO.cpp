// NALINFO.cpp : implementation file
//

#include "stdafx.h"
#include "IPCM.h"
#include "NALINFO.h"
#include "afxdialogex.h"


// CNALINFO dialog

IMPLEMENT_DYNAMIC(CNALINFO, CDialogEx)

CNALINFO::CNALINFO(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_NAL, pParent)
{

}

CNALINFO::~CNALINFO()
{
}

void CNALINFO::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CNALINFO, CDialogEx)
END_MESSAGE_MAP()


// CNALINFO message handlers
