
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
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CIPCMDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


/* RTSPClient���ݻص� */
static int RTSPClientCallBack(void* userData, RTSP_FRAME_INFO* frameInfo)
{
	dump_stream((unsigned char*)frameInfo->data, frameInfo->size);
#if 0
	if (_frameType == EASY_SDK_VIDEO_FRAME_FLAG)//�ص���Ƶ���ݣ�����00 00 00 01ͷ
	{
#if 0
		if (_frameInfo->codec == EASY_SDK_VIDEO_CODEC_H264)
		{
			/*
			ÿһ��I�ؼ�֡����SPS+PPS+IDR�����
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

				spsLen = _frameInfo->reserved1;							// SPS���ݳ���
				ppsLen = _frameInfo->reserved2 - _frameInfo->reserved1;	// PPS���ݳ���
				iFrameLen = _frameInfo->length - spsLen - ppsLen;		// IDR���ݳ���

				memcpy(sps, _pBuf, spsLen);			//SPS���ݣ�����00 00 00 01
				memcpy(pps, _pBuf + spsLen, ppsLen);	//PPS���ݣ�����00 00 00 01
				IFrame = _pBuf + spsLen + ppsLen;	//IDR���ݣ�����00 00 00 01
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
			waitKey(30);  //��ʱ30ms
		}
	}
	else if (_frameType == EASY_SDK_EVENT_FRAME_FLAG)//�ص�����״̬�¼�
	{
		// ��ʼ���ӻ�������ʧ���ٴν�������
		if (NULL == _pBuf && NULL == _frameInfo)
		{
			TRACE("Connecting: ...\n");
		}

		// �ж�֡���!
		else if (NULL != _frameInfo && _frameInfo->type == 0xF1)
		{
			TRACE("Lose Packet ...\n");
		}
	}
	else if (_frameType == EASY_SDK_MEDIA_INFO_FLAG)//�ص���ý����Ϣ
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
		waitKey(30);  //��ʱ30ms
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

	//����RTSPSource
	//EasyRTSP_Init(&m_fRTSPHandle);
	RTSP_Init(&m_fRTSPHandle);
	// ���Ը���fRTSPHanlde�жϣ�Ҳ���Ը���EasyRTSP_Init�Ƿ񷵻�0�ж�
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
		// �ر�RTSPClient
		RTSP_CloseStream(m_fRTSPHandle);
		//EasyRTSP_CloseStream(m_fRTSPHandle);

		// �ͷ�RTSPHandle
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
		GetDlgItem (IDC_PREVIEW)->SetWindowText(_T("Ԥ��"));
		destroyWindow(g_url);
	}
	else
	{
		m_bPreview = true;
		GetDlgItem(IDC_PREVIEW)->SetWindowText(_T("�ر�Ԥ��"));
		namedWindow(g_url, CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO);
	}
}
