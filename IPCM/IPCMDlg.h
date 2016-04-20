
// IPCMDlg.h : 头文件
//

#pragma once

typedef struct fps_info_s
{
	time_t recive_time;
	unsigned frame_num;
}fps_info_t;

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
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOpenUrl();


public:
	CWinThread *m_pThreadVideoPlay;
	bool m_quit_video_play;

	CCapture_FFMPEG m_capture_ffmpeg;
	FILE* m_dump_file;
	char  m_url[1024];
	bool m_bPreview;
	bool m_bShowFpsSpline;
	IplImage* m_fpsImage;
	IplImage* m_previewImage;
	time_t m_begine_time;
	unsigned m_current_frame_num;

	std::vector<fps_info_t> m_fps_info_vec;

	void SystemClear();
	void SetStreamInfo();
	void DumpStream(unsigned char *buf, int size);
};
