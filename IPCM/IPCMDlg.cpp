
// IPCMDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "IPCM.h"
#include "IPCMDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


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
	
	CString FilePathName = dlg.GetPathName();           
	USES_CONVERSION;
	m_orgimg = imread(W2A(FilePathName));
	ShowImage(m_orgimg, IDC_SHOW_IMAGE);
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


void CIPCMDlg::ShowImage(Mat& img, UINT id)	
{
	
	
	CDC* pDC = GetDlgItem(id)->GetDC();		// 获得显示控件的 DC
	HDC hDC = pDC->GetSafeHdc();				// 获取 HDC(设备句柄) 来进行绘图操作
	
	Rect dst;
	

	//GetDlgItem(id)->MoveWindow(CRect(0, 0, 512, 512));
	GetDlgItem(id)->MoveWindow(CRect(0, 0, 512, 512* img.rows/img.cols));
	GetDlgItem(id)->GetClientRect(&m_display_rect);
	// 图片置于控件中央
	//int rw = rect.right - rect.left;			// 求出图片控件的宽和高
	//int rh = rect.bottom - rect.top;
	//int iw = img.cols;						// 读取图片的宽和高
	//int ih = img.rows;
	//int tx = (int)(rw - iw) / 2;					// 使图片的显示位置正好在控件的正中
	//int ty = (int)(rh - ih) / 2;
	//SetRect(rect, tx, ty, tx + iw, ty + ih);
	//rect.NormalizeRect();


	//int from_x = 0;
	//int from_y = 0;
	//int bmp_w = img.cols, bmp_h = img.rows;
	
	//dst.x = rect.left;
	//dst.y = rect.top;
	//dst.width = rect.right - rect.left;
	//dst.height = rect.bottom - rect.top;

	assert(img.depth() == CV_8U);
	uchar buffer[sizeof(BITMAPINFOHEADER) + 1024];
	BITMAPINFO* bmi = (BITMAPINFO*)buffer;
	FillBitmapInfo(bmi, img.cols, img.rows, img.channels(), 0);// img.origin);

	if (m_display_rect.right - m_display_rect.left == img.cols && m_display_rect.bottom - m_display_rect.top == img.rows)
	{

		::SetDIBitsToDevice(
			hDC, 
			m_display_rect.left,       // 指定目标矩形左上角的X轴坐标，按逻辑单位表示坐标
			m_display_rect.top,       // 指字目标矩形左上角的Y轴坐标，按逻辑单位表示坐标
			m_display_rect.right - m_display_rect.left,          // 指定DIB的宽度，按逻辑单位表示宽度
			m_display_rect.bottom - m_display_rect.top,          // 指定DIB的高度，按逻辑单位表示高度
			0,           // 指定DIB位图左下角的X轴坐标，按逻辑单位表示坐标
			0,           // 指定DIB位图左下角的Y轴坐标，按逻辑单位表示坐标
			0,           // 指定DIB中的起始扫描线
			img.rows,//MIN(img.rows, rect.bottom - rect.top),          // 指定参数lpvBits指向的数组中包含的DIB扫描线数目
			img.data,    // 指向存储DIB颜色数据的字节类型数组的指针
			bmi,         // 指向BITMAPINFO结构的指针，该结构包含有关DIB的信息
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
			m_display_rect.left, m_display_rect.top, m_display_rect.right - m_display_rect.left, m_display_rect.bottom - m_display_rect.top,
			0, 0, img.cols, img.rows,
			img.data, bmi, DIB_RGB_COLORS, SRCCOPY);
	}
	
	ReleaseDC(pDC);
}

static UINT ThreadVideoPlay(LPVOID lpParam) {
	CIPCMDlg *dlg = (CIPCMDlg *)lpParam;
	
	while(1)
	{
		if (dlg->m_quit_video_play)
			break;

		Mat frame;  //定义一个Mat变量，用于存储每一帧的图像
		dlg->m_capture >> frame;  //读取当前帧
		//imshow("读取视频", frame);  //显示当前帧
		dlg->ShowImage(frame, IDC_SHOW_IMAGE);
		Sleep(dlg->m_video_play_wait_time);
		//TRACE("ThreadCamPlay runing \n");
	}
	dlg->m_pThreadVideoPlay = NULL;
	return 0;
}

void CIPCMDlg::OnOpenCam()
{
	// TODO: Add your command handler code here
	SystemClear();

	//if (m_capture.isOpened())
	{
		//TRACE("ThreadCamPlay release\n");
		//m_capture.release();
	
		//Mat Z = Mat::zeros(m_display_rect.Height(), m_display_rect.Width(), CV_8U);
		//ShowImage(Z, IDC_SHOW_IMAGE);
		
		//GetMenu()->GetSubMenu(0)->ModifyMenu(ID_OPEN_CAM, MF_STRING|MF_BYCOMMAND, NULL, _T("打开摄像头"));
		//GetMenu()->GetSubMenu(0)->ModifyMenu(2, MF_BYPOSITION, ID_OPEN_CAM, _T("打开摄像头"));
	}
	//else
	{
		m_capture.open(0);
		if (!m_capture.isOpened())  
			return;
		m_video_play_wait_time = 30;
		m_pThreadVideoPlay = AfxBeginThread(ThreadVideoPlay, this);//开启线程
		//GetMenu()->GetSubMenu(0)->ModifyMenu(ID_OPEN_CAM, MF_STRING|MF_BYCOMMAND, NULL, _T("关闭摄像头"));
		//GetMenu()->GetSubMenu(0)->ModifyMenu(2, MF_BYPOSITION, ID_OPEN_CAM, _T("关闭摄像头"));
	}
}

void CIPCMDlg::OnOpenVideo()
{
	SystemClear();

	CString strfilter;
	strfilter.Append(_T("视频文件(*.mp4;*avi)|*.mp4;*.avi|"));
	strfilter.Append(_T("ES流(*.264;*265)|*.264;*.265|"));
	strfilter.Append(_T("All Files(*.*)|*.*||"));
	LPCTSTR lpszfilter = strfilter;
	CFileDialog dlg(TRUE, NULL, NULL, NULL, lpszfilter);

	dlg.m_ofn.lpstrTitle = _T("打开视频文件");
	if (dlg.DoModal() != IDOK)
		return;
	CString FilePathName = dlg.GetPathName();

	USES_CONVERSION;
	m_capture.open(W2A(FilePathName));
	if (!m_capture.isOpened())
		return;
	m_video_play_wait_time = 1000/(int)m_capture.get(CAP_PROP_FPS);
	m_pThreadVideoPlay = AfxBeginThread(ThreadVideoPlay, this);//开启线程
}

void CIPCMDlg::SystemClear()
{
	
	while (m_pThreadVideoPlay)
		Sleep(100);
	
	if (m_capture.isOpened())
		m_capture.release();
	
	//TRACE("SystemClear DONE!\n");
}


void CIPCMDlg::OnDestroy()
{
	CDialogEx::OnDestroy();
	m_quit_video_play = true;
	SystemClear();
}
