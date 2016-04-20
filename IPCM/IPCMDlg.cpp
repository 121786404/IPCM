
// IPCMDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "IPCM.h"
#include "IPCMDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CIPCMDlg 对话框



CIPCMDlg::CIPCMDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_IPCM_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CIPCMDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	m_pThreadVideoPlay = NULL;
	m_quit_video_play = false;
	m_dump_file = NULL;
	m_bPreview = false;
	m_bShowFpsSpline = false;

	m_bPreview = true;
	m_bShowFpsSpline = true;

	m_fpsImage = NULL;
	m_previewImage = NULL;
	m_begine_time = 0;
	m_current_frame_num = 0;
}

BEGIN_MESSAGE_MAP(CIPCMDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_OPEN_URL, &CIPCMDlg::OnBnClickedOpenUrl)
END_MESSAGE_MAP()


// CIPCMDlg 消息处理程序

BOOL CIPCMDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	
	GetDlgItem(IDC_URL)->SetWindowText(_T("rtsp://172.21.17.49/video0"));

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CIPCMDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CIPCMDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CIPCMDlg::DumpStream(unsigned char *buf, int size)
{
	if (m_dump_file == NULL)
		fopen_s(&m_dump_file, "IPCM_DUMP.h264", "wb");

	fwrite(buf, 1, size, m_dump_file);
}


#define FPS_TIME_WINDOW 300
#define FPS_MAX 40 
static int fps_window_scale_x = 4;
static int fps_window_scale_y = 10;
static int fps_window_border = fps_window_scale_x * 10;		// border around graph within the image
static int fps_window_width = fps_window_scale_x*FPS_TIME_WINDOW + 2 * fps_window_border;
static int fps_window_height = fps_window_scale_y*FPS_MAX + 2 * fps_window_border;

static void drawUCharGraph(std::vector<fps_info_t>* fps_info_vec, IplImage* imageGraph)
{
	cvSet(imageGraph, CV_RGB(255, 255, 255));
	cvLine(imageGraph, cvPoint(0, fps_window_height - (fps_window_border + 30 * fps_window_scale_y)), cvPoint(fps_window_width - 1, fps_window_height - (fps_window_border + 30 * fps_window_scale_y)), CV_RGB(185, 0, 0), 1, CV_AA);	// Draw a line from the previous point to the new point
	cvLine(imageGraph, cvPoint(0, fps_window_height - (fps_window_border + 25 * fps_window_scale_y)), cvPoint(fps_window_width - 1, fps_window_height - (fps_window_border + 25 * fps_window_scale_y)), CV_RGB(185, 0, 0), 1, CV_AA);	// Draw a line from the previous point to the new point
	cvLine(imageGraph, cvPoint(0, fps_window_height - (fps_window_border + 20 * fps_window_scale_y)), cvPoint(fps_window_width - 1, fps_window_height - (fps_window_border + 20 * fps_window_scale_y)), CV_RGB(185, 0, 0), 1, CV_AA);	// Draw a line from the previous point to the new point

	int x0 = fps_window_scale_x * 0;
	int y0 = fps_window_scale_y*(fps_info_vec->begin()->frame_num);
	CvPoint ptPrev = cvPoint(fps_window_border + x0, fps_window_height - (fps_window_border + y0));

	std::vector<fps_info_t>::iterator iter;
	for (iter = fps_info_vec->begin(); iter != fps_info_vec->end(); iter++)
	{
		int x = fps_window_scale_x*((int)(iter->recive_time - fps_info_vec->begin()->recive_time));
		int y = fps_window_scale_y*iter->frame_num;
		CvPoint ptNew = cvPoint(fps_window_border + x, fps_window_height - (fps_window_border + y));
		cvLine(imageGraph, ptPrev, ptNew, CV_RGB(0, 0, 0), 1, CV_AA);	// Draw a line from the previous point to the new point
		ptPrev = ptNew;
	}
}


static UINT ThreadVideoPlay(LPVOID lpParam) {
	CIPCMDlg *dlg = (CIPCMDlg *)lpParam;
	Image_Info img;

	while (1)
	{
		if (dlg->m_quit_video_play)
			break;

		if (false == dlg->m_capture_ffmpeg.grabFrame())
		{
			TRACE("grabFrame fail\n");
			continue;;
		}

		if (false == dlg->m_capture_ffmpeg.retrieveFrame(&img))
		{
			TRACE("retrieveFrame fail\n");
			break;
		}

		time_t current_time = time(NULL);
		if (current_time != dlg->m_begine_time)
		{
			if (dlg->m_fps_info_vec.size() == FPS_TIME_WINDOW)
				dlg->m_fps_info_vec.erase(dlg->m_fps_info_vec.begin());

			fps_info_t fps_info;
			fps_info.recive_time = current_time;
			fps_info.frame_num = dlg->m_current_frame_num;
			dlg->m_fps_info_vec.push_back(fps_info);
			if (dlg->m_bShowFpsSpline)
			{
				drawUCharGraph(&dlg->m_fps_info_vec, dlg->m_fpsImage);
				cvShowImage("fps", dlg->m_fpsImage);
			}
			dlg->m_begine_time = current_time;
			dlg->m_current_frame_num = 0;
		}
		else
		{
			dlg->m_current_frame_num++;
		}

		//DumpStream(packet_buf, packet_size);

		if (dlg->m_bPreview)
		{
			if (dlg->m_previewImage == NULL)
			{
				dlg->m_previewImage = cvCreateImage(cvSize(img.width, img.height), IPL_DEPTH_8U, img.cn);
				cvResizeWindow(dlg->m_url, img.width, img.height);
			}
			memcpy(dlg->m_previewImage->imageData, img.data, dlg->m_previewImage->imageSize);
			IplImage* tmp = dlg->m_previewImage;
			cvShowImage(dlg->m_url, dlg->m_previewImage);
		}
	}

	dlg->m_pThreadVideoPlay = NULL;
	return 0;
}

void CIPCMDlg::SetStreamInfo()
{
	double fps = m_capture_ffmpeg.get(FFMPEG_CAP_PROP_FPS);
	unsigned int fourcc = (unsigned int)m_capture_ffmpeg.get(FFMPEG_CAP_PROP_FOURCC);
	unsigned int width = (unsigned int)m_capture_ffmpeg.get(FFMPEG_CAP_PROP_FRAME_WIDTH);
	unsigned int height = (unsigned int)m_capture_ffmpeg.get(FFMPEG_CAP_PROP_FRAME_HEIGHT);
	int sar_num = (int)m_capture_ffmpeg.get(FFMPEG_CAP_PROP_SAR_NUM);
	int sar_den = (int)m_capture_ffmpeg.get(FFMPEG_CAP_PROP_SAR_DEN);


	CString tmp;
	//tmp.Format(_T("%d"), (int)frame_count);		           m_frame_num.SetWindowText(tmp);
	//tmp.Format(_T("%d x %d "), (int)width, (int)height);   m_video_size.SetWindowText(tmp);
	//tmp.Format(_T("%5.2ffps"), fps);					   m_fps.SetWindowText(tmp);
}


void CIPCMDlg::SystemClear()
{
	cvDestroyWindow(m_url);
	cvDestroyWindow("fps");


	memset(m_url, 0, sizeof(m_url));

	if (m_previewImage)
		cvReleaseImage(&m_previewImage);

	if (m_fpsImage)
		cvReleaseImage(&m_fpsImage);

	if (m_dump_file)
	{
		fclose(m_dump_file);
		m_dump_file = NULL;
	}

	m_fps_info_vec.clear();
}

void CIPCMDlg::OnBnClickedOpenUrl()
{
	CString FilePathName;
	GetDlgItem(IDC_URL)->GetWindowText(FilePathName);
	
	SystemClear();

	
	// FilePathName = "rtsp://172.21.16.85/IPCM.265";
	// FilePathName = "rtsp://172.21.16.85/IPCM.264";
	// FilePathName = "rtsp://172.21.17.49/video0";
	// FilePathName = "rtsp://172.21.17.55/video0";

	if (FilePathName.IsEmpty())
	{
		return;
	}

	USES_CONVERSION;
	strcpy_s(m_url, W2A(FilePathName));

	m_capture_ffmpeg.open(W2A(FilePathName));
	if (!m_capture_ffmpeg.isOpened())
	{
		MessageBox(_T("打开视频流"), _T("错误"), MB_ICONEXCLAMATION);
		return;
	}
	//memset(&m_ff_packet, 0, sizeof(m_ff_packet));
	//av_init_packet(&m_ff_packet);

	SetStreamInfo();
	m_quit_video_play = false;
	m_pThreadVideoPlay = AfxBeginThread(ThreadVideoPlay, this);//开启线程

	m_begine_time = time(NULL);
	if (m_bShowFpsSpline)
	{
		m_fpsImage = cvCreateImage(cvSize(fps_window_width, fps_window_height), 8, 3);
		cvNamedWindow("fps");
	}

	if (m_bPreview)
	{
		//m_previewImage = NULL;
		cvNamedWindow(m_url);// , CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO);
							 //cvMoveWindow(g_url, 0, 0);
	}
}
