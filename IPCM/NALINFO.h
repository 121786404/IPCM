#pragma once


// CNALINFO dialog

class CNALINFO : public CDialogEx
{
	DECLARE_DYNAMIC(CNALINFO)

public:
	CNALINFO(CWnd* pParent = NULL);   // standard constructor
	virtual ~CNALINFO();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_NAL };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
