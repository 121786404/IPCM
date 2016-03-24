
// IPCMDlg.h : ͷ�ļ�
//

#pragma once
#include "afxwin.h"



// CIPCMDlg �Ի���
class CIPCMDlg : public CDialogEx
{
// ����
public:
	CIPCMDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_IPCM_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

private:
	Easy_RTSP_Handle m_fRTSPHandle ;
	void SystemClear();
	afx_msg void OnClose();
	afx_msg void OnOpenStream();
public:
	afx_msg void OnBnClickedShowNal();
	afx_msg void OnBnClickedPreview();
};
