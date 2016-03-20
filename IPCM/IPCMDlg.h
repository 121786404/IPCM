
// IPCMDlg.h : 头文件
//

#pragma once
#include "afxwin.h"


// CIPCMDlg 对话框
class CIPCMDlg : public CDialogEx
{
// 构造
public:
	CIPCMDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_IPCM_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
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

	CCapture_FFMPEG m_capture_ffmpeg;
	bool m_bUseFFmpeg;
	bool m_quit_video_play;
	int m_video_src;

	int m_video_play_wait_time;
	CWinThread *m_pThreadVideoPlay;
	
	void ShowImage(Image_Info* img, UINT ID);
	void SetStreamInfo();

	void SystemClear();
	

	afx_msg void OnClose();
	afx_msg void OnOpenStream();


	CEdit m_input;

	//CEdit	m_pix_fmt;
	CEdit	m_frame_num;
	CEdit	m_video_size;
	CEdit	m_fps;
	//CEdit	m_audio_decoder_name;
	//CEdit	m_audio_sample_rate;
	//CEdit	m_audio_channels;
	//CEdit	m_aqsize;
	//CEdit	m_vqsize;
	//CEdit	m_avdiff;
	afx_msg void OnBnClickedNalInfo();
};
