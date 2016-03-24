#pragma once


// Handle����
#define RTSP_Handle void*


/* ����Ƶ֡��ʶ */
#define RTSP_VIDEO_FRAME_FLAG	0x00000001		/* ��Ƶ֡��־ */
#define RTSP_AUDIO_FRAME_FLAG	0x00000002		/* ��Ƶ֡��־ */
#define RTSP_EVENT_FRAME_FLAG	0x00000004		/* �¼�֡��־ */
#define RTSP_RTP_FRAME_FLAG		0x00000008		/* RTP֡��־ */
#define RTSP_SDP_FRAME_FLAG		0x00000010		/* SDP֡��־ */
#define RTSP_MEDIA_INFO_FLAG	0x00000020		/* ý�����ͱ�־*/

/* ��Ƶ�ؼ��ֱ�ʶ */
#define RTSP_VIDEO_FRAME_I		0x01		/* I֡ */
#define RTSP_VIDEO_FRAME_P		0x02		/* P֡ */
#define RTSP_VIDEO_FRAME_B		0x03		/* B֡ */
#if 0
/* �������� */
typedef enum __RTP_CONNECT_TYPE
{
	RTP_OVER_TCP = 0x01,		/* RTP Over TCP */
	RTP_OVER_UDP					/* RTP Over UDP */
}RTP_CONNECT_TYPE;

/* ý����Ϣ */
typedef struct _RTSP_MEDIA_INFO_T
{
	unsigned int u32VideoCodec;			/* ��Ƶ�������� */
	unsigned int u32VideoFps;			/* ��Ƶ֡�� */

	unsigned int u32AudioCodec;			/* ��Ƶ�������� */
	unsigned int u32AudioSamplerate;	/* ��Ƶ������ */
	unsigned int u32AudioChannel;		/* ��Ƶͨ���� */
}RTSP_MEDIA_INFO_T;

/* ֡��Ϣ */
typedef struct
{
	unsigned int	codec;			/* ����Ƶ��ʽ */

	unsigned int	type;			/* ��Ƶ֡���� */
	unsigned char	fps;			/* ��Ƶ֡�� */
	unsigned short	width;			/* ��Ƶ�� */
	unsigned short  height;			/* ��Ƶ�� */

	unsigned int	reserved1;		/* ��������1 */
	unsigned int	reserved2;		/* ��������2 */

	unsigned int	sample_rate;	/* ��Ƶ������ */
	unsigned int	channels;		/* ��Ƶ������ */

	unsigned int	length;			/* ����Ƶ֡��С */
	unsigned int    timestamp_usec;	/* ʱ���,΢�� */
	unsigned int	timestamp_sec;	/* ʱ��� �� */

	float			bitrate;		/* ������ */
	float			losspacket;		/* ������ */
}RTSP_FRAME_INFO;

#endif

/*
userData:	    �û�����
frameInfo:		֡�ṹ����
*/
typedef int(*RTSPCallBack)(void* userData, RTSP_FRAME_INFO* frameInfo);

/* ��ȡ���һ�δ���Ĵ����� */
int RTSP_GetErrCode();

/* ����RTSPClient���  ����0��ʾ�ɹ������ط�0��ʾʧ�� */
int RTSP_Init(RTSP_Handle *handle);

/* �ͷ�RTSPClient ����ΪRTSPClient��� */
int RTSP_Deinit(RTSP_Handle *handle);

/* �������ݻص� */
int RTSP_SetCallback(RTSP_Handle handle, RTSPCallBack _callback);

/* �������� */
int RTSP_OpenStream(RTSP_Handle handle, int _channelid, char *_url, RTP_CONNECT_TYPE _connType, unsigned int _mediaType, char *_username, char *_password, void *userPtr, int _reconn/*1000��ʾ������,���������Ͽ��Զ�����, ����ֵΪ���Ӵ���*/, int outRtpPacket/*Ĭ��Ϊ0,���ص����������֡, ���Ϊ1,�����RTP��*/);

/* �ر������� */
int RTSP_CloseStream(RTSP_Handle handle);


