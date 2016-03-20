
// IPCMDlg.cpp : ʵ���ļ�
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

// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
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


// CIPCMDlg �Ի���



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


// CIPCMDlg ��Ϣ�������

BOOL CIPCMDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
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

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������

	::SetWindowPos(this->m_hWnd, HWND_BOTTOM, 0, 0, 1024, 700, SWP_NOZORDER);
	CenterWindow();
	//AllocConsole();

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
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

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CIPCMDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
		CDialogEx::UpdateWindow();				// ����windows���ڣ�������ⲽ���ã�ͼƬ��ʾ�����������
		//ShowImage();		// �ػ�ͼƬ����
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
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
	CDC* pDC = GetDlgItem(id)->GetDC();		// �����ʾ�ؼ��� DC
	HDC hDC = pDC->GetSafeHdc();				// ��ȡ HDC(�豸���) �����л�ͼ����
	
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
			m_display_rect.left,       // ָ��Ŀ��������Ͻǵ�X�����꣬���߼���λ��ʾ����
			m_display_rect.top,       // ָ��Ŀ��������Ͻǵ�Y�����꣬���߼���λ��ʾ����
			m_display_rect.Width(), 
			m_display_rect.Height(),
			0,           // ָ��DIBλͼ���½ǵ�X�����꣬���߼���λ��ʾ����
			0,           // ָ��DIBλͼ���½ǵ�Y�����꣬���߼���λ��ʾ����
			0,           // ָ��DIB�е���ʼɨ����
			img->height,//MIN(img.rows, rect.bottom - rect.top),          // ָ������lpvBitsָ��������а�����DIBɨ������Ŀ
			img->data,    // ָ��洢DIB��ɫ���ݵ��ֽ����������ָ��
			(BITMAPINFO*)buffer,         // ָ��BITMAPINFO�ṹ��ָ�룬�ýṹ�����й�DIB����Ϣ
			DIB_RGB_COLORS);            // ָ��BITMAPINFO�ṹ�еĳ�ԱbmiColors�Ƿ������ȷ��RGBֵ��Ե�ɫ�����������ֵ
		
	}
	else
	{
#if 0
		if (img.cols > rect.right - rect.left)
		{
			::SetStretchBltMode(
				hDC,           // handle to device context
				HALFTONE);

			// ��Դ�������е�����ӳ�䵽Ŀ������������ؿ��У�����Ŀ�����ؿ��һ����ɫ��Դ���ص���ɫ�ӽ���
			// ��������HALFTONE����ģ֮��Ӧ�ó���������SetBrushOrgEx����������ˢ�ӵ���ʼ�㡣
			// ���û�гɹ�����ô�����ˢ��û��׼�����
		}
		else
		{
			::SetStretchBltMode(
				hDC,           // handle to device context
				COLORONCOLOR);

			// ɾ�����ء���ģʽɾ�����������������У�����������Ϣ
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
			dlg->m_capture >> frame;  //��ȡ��ǰ֡
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
	strfilter.Append(_T("ͼƬ�ļ�(*.jpg;*png;*bmp)|*.jpg;*.png;*.bmp|"));
	strfilter.Append(_T("All Files(*.*)|*.*||"));
	LPCTSTR lpszfilter = strfilter;
	CFileDialog dlg(TRUE, NULL, NULL, NULL, lpszfilter);

	dlg.m_ofn.lpstrTitle = _T("��ͼƬ�ļ�");
	if (dlg.DoModal() != IDOK)
		return;

	m_video_src = VIDEO_SOURCE_IMAGE;
	CString FilePathName = dlg.GetPathName();
	USES_CONVERSION;
	m_orgimg = imread(W2A(FilePathName));
	if(m_orgimg.empty())
	{
		MessageBox(_T("��ͼ���ļ�"), _T("����"), MB_ICONEXCLAMATION);
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
	strfilter.Append(_T("ES��(*.264;*265)|*.264;*.265|"));
	strfilter.Append(_T("��Ƶ�ļ�(*.mp4;*avi)|*.mp4;*.avi|"));
	strfilter.Append(_T("All Files(*.*)|*.*||"));
	LPCTSTR lpszfilter = strfilter;
	CFileDialog dlg(TRUE, NULL, NULL, NULL, lpszfilter);

	dlg.m_ofn.lpstrTitle = _T("����Ƶ�ļ�");
	if (dlg.DoModal() != IDOK)
		return;
	CString FilePathName = dlg.GetPathName();

	m_quit_video_play = false;
	USES_CONVERSION;
	m_capture.open(W2A(FilePathName));
	if (!m_capture.isOpened())
	{
		MessageBox(_T("����Ƶ��ʧ��"), _T("����"), MB_ICONEXCLAMATION);
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
		MessageBox(_T("������ͷʧ��"),_T("����"), MB_ICONEXCLAMATION);
		return;
	}

	m_video_src = VIDEO_SOURCE_CAMERA;
	m_video_play_wait_time = 30;
	m_pThreadVideoPlay = AfxBeginThread(ThreadVideoPlay, this);//�����߳�
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
			MessageBox(_T("����Ƶ��"), _T("����"), MB_ICONEXCLAMATION);
			return;
		}
	}
	else
	{
		m_capture.open(W2A(FilePathName));
		if (!m_capture.isOpened())
		{
			MessageBox(_T("����Ƶ��"), _T("����"), MB_ICONEXCLAMATION);
			return;
		}
	}

	m_video_src = VIDEO_SOURCE_NETWORK;
	m_video_play_wait_time = 30;
	m_pThreadVideoPlay = AfxBeginThread(ThreadVideoPlay, this);//�����߳�
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
