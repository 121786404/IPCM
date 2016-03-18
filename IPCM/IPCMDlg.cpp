
// IPCMDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "IPCM.h"
#include "IPCMDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


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
	
	
	CDC* pDC = GetDlgItem(id)->GetDC();		// �����ʾ�ؼ��� DC
	HDC hDC = pDC->GetSafeHdc();				// ��ȡ HDC(�豸���) �����л�ͼ����
	
	Rect dst;
	

	//GetDlgItem(id)->MoveWindow(CRect(0, 0, 512, 512));
	GetDlgItem(id)->MoveWindow(CRect(0, 0, 512, 512* img.rows/img.cols));
	GetDlgItem(id)->GetClientRect(&m_display_rect);
	// ͼƬ���ڿؼ�����
	//int rw = rect.right - rect.left;			// ���ͼƬ�ؼ��Ŀ�͸�
	//int rh = rect.bottom - rect.top;
	//int iw = img.cols;						// ��ȡͼƬ�Ŀ�͸�
	//int ih = img.rows;
	//int tx = (int)(rw - iw) / 2;					// ʹͼƬ����ʾλ�������ڿؼ�������
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
			m_display_rect.left,       // ָ��Ŀ��������Ͻǵ�X�����꣬���߼���λ��ʾ����
			m_display_rect.top,       // ָ��Ŀ��������Ͻǵ�Y�����꣬���߼���λ��ʾ����
			m_display_rect.right - m_display_rect.left,          // ָ��DIB�Ŀ�ȣ����߼���λ��ʾ���
			m_display_rect.bottom - m_display_rect.top,          // ָ��DIB�ĸ߶ȣ����߼���λ��ʾ�߶�
			0,           // ָ��DIBλͼ���½ǵ�X�����꣬���߼���λ��ʾ����
			0,           // ָ��DIBλͼ���½ǵ�Y�����꣬���߼���λ��ʾ����
			0,           // ָ��DIB�е���ʼɨ����
			img.rows,//MIN(img.rows, rect.bottom - rect.top),          // ָ������lpvBitsָ��������а�����DIBɨ������Ŀ
			img.data,    // ָ��洢DIB��ɫ���ݵ��ֽ����������ָ��
			bmi,         // ָ��BITMAPINFO�ṹ��ָ�룬�ýṹ�����й�DIB����Ϣ
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

		Mat frame;  //����һ��Mat���������ڴ洢ÿһ֡��ͼ��
		dlg->m_capture >> frame;  //��ȡ��ǰ֡
		//imshow("��ȡ��Ƶ", frame);  //��ʾ��ǰ֡
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
		
		//GetMenu()->GetSubMenu(0)->ModifyMenu(ID_OPEN_CAM, MF_STRING|MF_BYCOMMAND, NULL, _T("������ͷ"));
		//GetMenu()->GetSubMenu(0)->ModifyMenu(2, MF_BYPOSITION, ID_OPEN_CAM, _T("������ͷ"));
	}
	//else
	{
		m_capture.open(0);
		if (!m_capture.isOpened())  
			return;
		m_video_play_wait_time = 30;
		m_pThreadVideoPlay = AfxBeginThread(ThreadVideoPlay, this);//�����߳�
		//GetMenu()->GetSubMenu(0)->ModifyMenu(ID_OPEN_CAM, MF_STRING|MF_BYCOMMAND, NULL, _T("�ر�����ͷ"));
		//GetMenu()->GetSubMenu(0)->ModifyMenu(2, MF_BYPOSITION, ID_OPEN_CAM, _T("�ر�����ͷ"));
	}
}

void CIPCMDlg::OnOpenVideo()
{
	SystemClear();

	CString strfilter;
	strfilter.Append(_T("��Ƶ�ļ�(*.mp4;*avi)|*.mp4;*.avi|"));
	strfilter.Append(_T("ES��(*.264;*265)|*.264;*.265|"));
	strfilter.Append(_T("All Files(*.*)|*.*||"));
	LPCTSTR lpszfilter = strfilter;
	CFileDialog dlg(TRUE, NULL, NULL, NULL, lpszfilter);

	dlg.m_ofn.lpstrTitle = _T("����Ƶ�ļ�");
	if (dlg.DoModal() != IDOK)
		return;
	CString FilePathName = dlg.GetPathName();

	USES_CONVERSION;
	m_capture.open(W2A(FilePathName));
	if (!m_capture.isOpened())
		return;
	m_video_play_wait_time = 1000/(int)m_capture.get(CAP_PROP_FPS);
	m_pThreadVideoPlay = AfxBeginThread(ThreadVideoPlay, this);//�����߳�
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
