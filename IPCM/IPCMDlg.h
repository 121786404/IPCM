
// IPCMDlg.h : ͷ�ļ�
//

#pragma once

typedef struct fps_info_s
{
	time_t recive_time;
	unsigned frame_num;
}fps_info_t;

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
