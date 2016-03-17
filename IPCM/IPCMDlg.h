
// IPCMDlg.h : ͷ�ļ�
//

#pragma once
#include "D:\Program\opencv\install\include\opencv2\core\mat.hpp"


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
public:
	afx_msg void OnOpenImg();
    afx_msg void OnOpenCam();
	afx_msg void OnOpenVideo();

	Mat m_orgimg;
	CRect m_display_rect;
	VideoCapture m_capture;
	bool m_quit_video_play;
	int m_video_play_wait_time;
	CWinThread *m_pThreadVideoPlay;

	void ShowImage(Mat& img, UINT ID);
	void SystemClear();
	
	afx_msg void OnDestroy();
};
