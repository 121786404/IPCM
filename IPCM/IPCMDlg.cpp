
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


enum
{
	VIDEO_SOURCE_NONE = 0,
	VIDEO_SOURCE_IMAGE = 1,
	VIDEO_SOURCE_FILE = 2,
	VIDEO_SOURCE_CAMERA = 3,
	VIDEO_SOURCE_NETWORK = 4,
	VIDEO_SOURCE_TITAL = 5
};

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
	m_video_play_wait_time = 0;
	m_pThreadVideoPlay = NULL;
	m_quit_video_play = false;
	m_video_src = 0;
	m_bUseFFmpeg = false;
	m_bUseFFmpeg = true;
}

void CIPCMDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CIPCMDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_COMMAND(ID_OPEN_IMG, &CIPCMDlg::OnOpenImg)
	ON_COMMAND(ID_OPEN_CAM, &CIPCMDlg::OnOpenCam)
	ON_COMMAND(ID_OPEN_VIDEO, &CIPCMDlg::OnOpenVideo)
	ON_WM_DESTROY()
	ON_COMMAND(ID_OPEN_STREAM, &CIPCMDlg::OnOpenStream)
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
		//ShowImage();		// 重绘图片函数
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CIPCMDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


static void ConvetMattoImage(Mat mat, Image_Info* img)
{
	img->data = mat.data;
	//img->step  =
	img->width = mat.cols;
	img->height = mat.rows;
	img->cn = mat.channels();
}


static void  FillBitmapInfo(BITMAPINFO* bmi, int width, int height, int channels, int origin)
{
	assert(bmi && width >= 0 && height >= 0 && (channels == 1 || channels == 3));

	BITMAPINFOHEADER* bmih = &(bmi->bmiHeader);

	memset(bmih, 0, sizeof(*bmih));
	bmih->biSize = sizeof(BITMAPINFOHEADER);
	bmih->biWidth = width;
	bmih->biHeight = origin ? abs(height) : -abs(height);
	bmih->biPlanes = 1;
	bmih->biBitCount = (unsigned short)(channels*8);
	bmih->biCompression = BI_RGB;
	
	RGBQUAD* palette = bmi->bmiColors;
	int i;
	for (i = 0; i < 256; i++)
	{
		palette[i].rgbBlue = palette[i].rgbGreen = palette[i].rgbRed = (BYTE)i;
		palette[i].rgbReserved = 0;
	}
}


void CIPCMDlg::ShowImage(Image_Info* img, UINT id)	
{
	CDC* pDC = GetDlgItem(id)->GetDC();		// 获得显示控件的 DC
	HDC hDC = pDC->GetSafeHdc();				// 获取 HDC(设备句柄) 来进行绘图操作
	
	Rect dst;
	int display_width, display_height;
	if (img->width <= 540)
	{
		display_width = img->width;
		display_height = img->height;
	}
	else
	{
		display_width  = 540;
		display_height = 540 * img->height / img->width;
	}

	GetDlgItem(id)->MoveWindow(CRect(0, 0, display_width, display_height));
	GetDlgItem(id)->GetClientRect(&m_display_rect);

	//assert(img.depth() == CV_8U);
	uchar buffer[sizeof(BITMAPINFOHEADER) + 1024];
	FillBitmapInfo((BITMAPINFO*)buffer, img->width, img->height, img->cn, 0);

	if (m_display_rect.Width() == img->width && m_display_rect.Height() == img->height)
	{

		::SetDIBitsToDevice(
			hDC, 
			m_display_rect.left,       // 指定目标矩形左上角的X轴坐标，按逻辑单位表示坐标
			m_display_rect.top,       // 指字目标矩形左上角的Y轴坐标，按逻辑单位表示坐标
			m_display_rect.Width(), 
			m_display_rect.Height(),
			0,           // 指定DIB位图左下角的X轴坐标，按逻辑单位表示坐标
			0,           // 指定DIB位图左下角的Y轴坐标，按逻辑单位表示坐标
			0,           // 指定DIB中的起始扫描线
			img->height,//MIN(img.rows, rect.bottom - rect.top),          // 指定参数lpvBits指向的数组中包含的DIB扫描线数目
			img->data,    // 指向存储DIB颜色数据的字节类型数组的指针
			(BITMAPINFO*)buffer,         // 指向BITMAPINFO结构的指针，该结构包含有关DIB的信息
			DIB_RGB_COLORS);            // 指向BITMAPINFO结构中的成员bmiColors是否包含明确的RGB值或对调色板进行索引的值
		
	}
	else
	{
#if 0
		if (img.cols > rect.right - rect.left)
		{
			::SetStretchBltMode(
				hDC,           // handle to device context
				HALFTONE);

			// 将源矩形区中的像素映射到目标矩形区的像素块中，覆盖目标像素块的一般颜色与源像素的颜色接近。
			// 在设置完HALFTONE拉伸模之后，应用程序必须调用SetBrushOrgEx函数来设置刷子的起始点。
			// 如果没有成功，那么会出现刷子没对准的情况
		}
		else
		{
			::SetStretchBltMode(
				hDC,           // handle to device context
				COLORONCOLOR);

			// 删除像素。该模式删除所有消除的像素行，不保留其信息
		}
#endif

		::StretchDIBits(
			hDC,
			m_display_rect.left, m_display_rect.top, m_display_rect.Width(), m_display_rect.Height(),
			0, 0, img->width ,img->height,
			img->data, (BITMAPINFO*)buffer, DIB_RGB_COLORS, SRCCOPY);
	}
	
	ReleaseDC(pDC);
}

static UINT ThreadVideoPlay(LPVOID lpParam) {
	CIPCMDlg *dlg = (CIPCMDlg *)lpParam;
	Image_Info img;
	Mat frame; 

	while (1)
	{
		if (dlg->m_quit_video_play)
				break;
		
		if (dlg->m_video_src == VIDEO_SOURCE_NETWORK && dlg->m_bUseFFmpeg)
		{
			dlg->m_capture_ffmpeg.grabFrame();
			dlg->m_capture_ffmpeg.retrieveFrame(&img);
		}
		else
		{
			dlg->m_capture >> frame;  //读取当前帧
			if (frame.empty())
				break;
			
			//IplImage frame;
			//cvInitImageHeader(&frame, cvSize(img.width, img.height), 8, img.cn);
			//cvSetData(&frame, img.data, img.step);
			//TRACE("%s %d\n", __FUNCTION__, __LINE__);
			ConvetMattoImage(frame, &img);
		}
		
		dlg->ShowImage(&img, IDC_SHOW_IMAGE);
		Sleep(dlg->m_video_play_wait_time);
	}


	dlg->m_pThreadVideoPlay = NULL;
	return 0;
}


void CIPCMDlg::OnOpenImg()
{
	SystemClear();

	CString strfilter;
	strfilter.Append(_T("图片文件(*.jpg;*png;*bmp)|*.jpg;*.png;*.bmp|"));
	strfilter.Append(_T("All Files(*.*)|*.*||"));
	LPCTSTR lpszfilter = strfilter;
	CFileDialog dlg(TRUE, NULL, NULL, NULL, lpszfilter);

	dlg.m_ofn.lpstrTitle = _T("打开图片文件");
	if (dlg.DoModal() != IDOK)
		return;

	m_video_src = VIDEO_SOURCE_IMAGE;
	CString FilePathName = dlg.GetPathName();
	USES_CONVERSION;
	m_orgimg = imread(W2A(FilePathName));
	if(m_orgimg.empty())
	{
		MessageBox(_T("打开图像文件"), _T("错误"), MB_ICONEXCLAMATION);
		return;
	}
	Image_Info img;
	ConvetMattoImage(m_orgimg, &img);
	ShowImage(&img, IDC_SHOW_IMAGE);
}

void CIPCMDlg::OnOpenVideo()
{
	SystemClear();

	CString strfilter;
	strfilter.Append(_T("ES流(*.264;*265)|*.264;*.265|"));
	strfilter.Append(_T("视频文件(*.mp4;*avi)|*.mp4;*.avi|"));
	strfilter.Append(_T("All Files(*.*)|*.*||"));
	LPCTSTR lpszfilter = strfilter;
	CFileDialog dlg(TRUE, NULL, NULL, NULL, lpszfilter);

	dlg.m_ofn.lpstrTitle = _T("打开视频文件");
	if (dlg.DoModal() != IDOK)
		return;
	CString FilePathName = dlg.GetPathName();

	m_quit_video_play = false;
	USES_CONVERSION;
	m_capture.open(W2A(FilePathName));
	if (!m_capture.isOpened())
	{
		MessageBox(_T("打开视频流失败"), _T("错误"), MB_ICONEXCLAMATION);
		return;
	}

	m_video_src = VIDEO_SOURCE_FILE;
	m_video_play_wait_time = 1000 / (int)m_capture.get(CAP_PROP_FPS);
	m_pThreadVideoPlay = AfxBeginThread(ThreadVideoPlay, this);
}

void CIPCMDlg::OnOpenCam()
{
	SystemClear();

	m_quit_video_play = false;
	m_capture.open(0);
	if (!m_capture.isOpened())
	{
		MessageBox(_T("打开摄像头失败"),_T("错误"), MB_ICONEXCLAMATION);
		return;
	}

	m_video_src = VIDEO_SOURCE_CAMERA;
	m_video_play_wait_time = 30;
	m_pThreadVideoPlay = AfxBeginThread(ThreadVideoPlay, this);//开启线程
}

void CIPCMDlg::OnOpenStream()
{
	SystemClear();
	
	// TODO: Add your command handler code here
	CInputDlg m_dlg;
	if (m_dlg.DoModal() != IDOK)
		return;

	//CString FilePathName = m_dlg.GetInputString();
	//CString FilePathName("rtsp://172.21.16.64/video0");
	//CString FilePathName("rtsp://172.21.16.112/IPCM.264");
	CString FilePathName("rtsp://192.168.199.206/IPCM.264");
	//CString FilePathName("rtsp://172.21.16.112/fukua.264");
	m_quit_video_play = false;
	USES_CONVERSION;
	
	if (m_bUseFFmpeg)
	{
		m_capture_ffmpeg.open(W2A(FilePathName));
		if (!m_capture_ffmpeg.isOpened())
		{
			MessageBox(_T("打开视频流"), _T("错误"), MB_ICONEXCLAMATION);
			return;
		}
	}
	else
	{
		m_capture.open(W2A(FilePathName));
		if (!m_capture.isOpened())
		{
			MessageBox(_T("打开视频流"), _T("错误"), MB_ICONEXCLAMATION);
			return;
		}
	}

	m_video_src = VIDEO_SOURCE_NETWORK;
	m_video_play_wait_time = 30;
	m_pThreadVideoPlay = AfxBeginThread(ThreadVideoPlay, this);//开启线程
}


void CIPCMDlg::SystemClear()
{
	m_quit_video_play = true;
	while (m_pThreadVideoPlay)
	{
		Sleep(100);
	}
	if (m_capture.isOpened())
		m_capture.release();
	
	if (m_bUseFFmpeg && m_capture_ffmpeg.isOpened())
		m_capture_ffmpeg.release();
}

void CIPCMDlg::OnClose()
{
	SystemClear();
	CDialogEx::OnClose();
}
