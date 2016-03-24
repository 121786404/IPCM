
// IPCMDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "IPCM.h"
#include "IPCMDlg.h"
#include "InputDlg.h"
#include "afxdialogex.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static FILE* g_dump_file = NULL;
static FFDecoder g_ffdecoder;
static char  g_url[1024];
static bool m_bPreview;

static void dump_stream(unsigned char *buf, int size)
{
	if (g_dump_file == NULL)
		g_dump_file = fopen("IPCM_DUMP.h264", "wb");

	fwrite(buf, 1, size, g_dump_file);
}


static void ConvetMattoImage(Mat mat, Image_Info_t* img)
{
	img->data = mat.data;
	//img->step  =
	img->width = mat.cols;
	img->height = mat.rows;
	img->cn = mat.channels();
}

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CIPCMDlg 对话框



CIPCMDlg::CIPCMDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_IPCM_DIALOG, pParent)

{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_fRTSPHandle = NULL;
	m_bPreview = true;
}

void CIPCMDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CIPCMDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()
	ON_COMMAND(ID_OPEN_STREAM, &CIPCMDlg::OnOpenStream)
	ON_BN_CLICKED(IDC_SHOW_NAL, &CIPCMDlg::OnBnClickedShowNal)
	ON_BN_CLICKED(IDC_PREVIEW, &CIPCMDlg::OnBnClickedPreview)
END_MESSAGE_MAP()


// CIPCMDlg 消息处理程序

BOOL CIPCMDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	::SetWindowPos(this->m_hWnd, HWND_BOTTOM, 0, 0, 1024, 700, SWP_NOZORDER);
	CenterWindow();
	//AllocConsole();

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CIPCMDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
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
		CDialogEx::UpdateWindow();				// 更新windows窗口，如果无这步调用，图片显示还会出现问题
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CIPCMDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


/* RTSPClient数据回调 */
static int RTSPClientCallBack(void* userData, RTSP_FRAME_INFO* frameInfo)
{
	dump_stream((unsigned char*)frameInfo->data, frameInfo->size);
#if 0
	if (_frameType == EASY_SDK_VIDEO_FRAME_FLAG)//回调视频数据，包含00 00 00 01头
	{
#if 0
		if (_frameInfo->codec == EASY_SDK_VIDEO_CODEC_H264)
		{
			/*
			每一个I关键帧都是SPS+PPS+IDR的组合
			|---------------------|----------------|-------------------------------------|
			|                     |                |                                     |
			0-----------------reserved1--------reserved2-------------------------------length
			*/
			if (_frameInfo->type == EASY_SDK_VIDEO_FRAME_I)
			{
				TRACE("Get I H264 Len:%d \ttimestamp:%u.%u\n", _frameInfo->length, _frameInfo->timestamp_sec, _frameInfo->timestamp_usec);
				char sps[2048] = { 0 };
				char pps[2048] = { 0 };
				char* IFrame = NULL;
				unsigned int spsLen, ppsLen, iFrameLen = 0;

				spsLen = _frameInfo->reserved1;							// SPS数据长度
				ppsLen = _frameInfo->reserved2 - _frameInfo->reserved1;	// PPS数据长度
				iFrameLen = _frameInfo->length - spsLen - ppsLen;		// IDR数据长度

				memcpy(sps, _pBuf, spsLen);			//SPS数据，包含00 00 00 01
				memcpy(pps, _pBuf + spsLen, ppsLen);	//PPS数据，包含00 00 00 01
				IFrame = _pBuf + spsLen + ppsLen;	//IDR数据，包含00 00 00 01
			}
			else if (_frameInfo->type == EASY_SDK_VIDEO_FRAME_P)
			{
				TRACE("Get P H264 Len:%d \ttimestamp:%u.%u\n", _frameInfo->length, _frameInfo->timestamp_sec, _frameInfo->timestamp_usec);
			}
		}
#endif
		//dump_stream((unsigned char*)_pBuf, _frameInfo->length);

		g_ffdecoder.init(_frameInfo->codec);
		Image_Info_t img;
		if (false == g_ffdecoder.decode((unsigned char *)_pBuf, _frameInfo->length, &img))
		{
			TRACE("Decoder Error\n");
			return 0;
		}
	
		if (m_bPreview)
		{
			Mat display_img(img.height, img.width, CV_8UC3, img.data, img.width*img.cn);
			resizeWindow(g_url, 960, 540);
			moveWindow(g_url, 0, 0);
			imshow(g_url, display_img);
			waitKey(30);  //延时30ms
		}
	}
	else if (_frameType == EASY_SDK_EVENT_FRAME_FLAG)//回调连接状态事件
	{
		// 初始连接或者连接失败再次进行重连
		if (NULL == _pBuf && NULL == _frameInfo)
		{
			TRACE("Connecting: ...\n");
		}

		// 有丢帧情况!
		else if (NULL != _frameInfo && _frameInfo->type == 0xF1)
		{
			TRACE("Lose Packet ...\n");
		}
	}
	else if (_frameType == EASY_SDK_MEDIA_INFO_FLAG)//回调出媒体信息
	{
		if (_pBuf != NULL)
		{
			EASY_MEDIA_INFO_T mediainfo;
			memset(&mediainfo, 0x00, sizeof(EASY_MEDIA_INFO_T));
			memcpy(&mediainfo, _pBuf, sizeof(EASY_MEDIA_INFO_T));
			TRACE("RTSP DESCRIBE Get Media Info: video:%u fps:%u audio:%u channel:%u sampleRate:%u \n",
				mediainfo.u32VideoCodec, mediainfo.u32VideoFps, mediainfo.u32AudioCodec, mediainfo.u32AudioChannel, mediainfo.u32AudioSamplerate);
		}
	}
#endif 


	g_ffdecoder.init(28);//  _frameInfo->codec);
	Image_Info_t img;
	if (false == g_ffdecoder.decode((unsigned char *)frameInfo->data, frameInfo->size, &img))
	{
		TRACE("Decoder Error\n");
		return 0;
	}

	if (m_bPreview)
	{
		Mat display_img(img.height, img.width, CV_8UC3, img.data, img.width*img.cn);
		resizeWindow(g_url, 960, 540);
		moveWindow(g_url, 0, 0);
		imshow(g_url, display_img);
		waitKey(30);  //延时30ms
	}

	return 0;
}

void CIPCMDlg::OnOpenStream()
{
	SystemClear();

	CInputDlg m_dlg;
	if (m_dlg.DoModal() != IDOK)
		return;

	//CString FilePathName("rtsp://192.168.111.1/IPCM.265");
	CString FilePathName("rtsp://192.168.111.1/IPCM.264");

	USES_CONVERSION;

	strcpy(g_url, W2A(FilePathName));

	//创建RTSPSource
	//EasyRTSP_Init(&m_fRTSPHandle);
	RTSP_Init(&m_fRTSPHandle);
	// 可以根据fRTSPHanlde判断，也可以根据EasyRTSP_Init是否返回0判断
	//if (NULL == m_fRTSPHandle)
		//return;

	RTSP_SetCallback(m_fRTSPHandle, RTSPClientCallBack);
	RTSP_OpenStream(m_fRTSPHandle, 0, g_url, RTP_OVER_TCP, EASY_SDK_VIDEO_FRAME_FLAG, 0, 0, NULL, 1000, 0);
	//EasyRTSP_SetCallback(m_fRTSPHandle, RTSPClientCallBack);
	//EasyRTSP_OpenStream(m_fRTSPHandle, 0, g_url, RTP_OVER_TCP, EASY_SDK_VIDEO_FRAME_FLAG, 0, 0, NULL, 1000, 0);
	namedWindow(g_url, CV_WINDOW_NORMAL|CV_WINDOW_KEEPRATIO);
}

void CIPCMDlg::SystemClear()
{
	destroyWindow(g_url);
	memset(g_url, 0, sizeof(g_url));
	
	if (g_dump_file)
	{
		fclose(g_dump_file);
		g_dump_file = NULL;
	}

	if (m_fRTSPHandle)
	{
		// 关闭RTSPClient
		RTSP_CloseStream(m_fRTSPHandle);
		//EasyRTSP_CloseStream(m_fRTSPHandle);

		// 释放RTSPHandle
		RTSP_Deinit(&m_fRTSPHandle);
		//EasyRTSP_Deinit(&m_fRTSPHandle);

		m_fRTSPHandle = NULL;
	}

	g_ffdecoder.release();

}

void CIPCMDlg::OnClose()
{
	SystemClear();
	CDialogEx::OnClose();
}

void CIPCMDlg::OnBnClickedShowNal()
{
	// TODO: Add your control notification handler code here
}


void CIPCMDlg::OnBnClickedPreview()
{
	// TODO: Add your control notification handler code here
	if (m_bPreview)
	{
		m_bPreview = false;
		GetDlgItem (IDC_PREVIEW)->SetWindowText(_T("预览"));
		destroyWindow(g_url);
	}
	else
	{
		m_bPreview = true;
		GetDlgItem(IDC_PREVIEW)->SetWindowText(_T("关闭预览"));
		namedWindow(g_url, CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO);
	}
}
