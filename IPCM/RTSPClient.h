#pragma once


// Handle类型
#define RTSP_Handle void*


/* 音视频帧标识 */
#define RTSP_VIDEO_FRAME_FLAG	0x00000001		/* 视频帧标志 */
#define RTSP_AUDIO_FRAME_FLAG	0x00000002		/* 音频帧标志 */
#define RTSP_EVENT_FRAME_FLAG	0x00000004		/* 事件帧标志 */
#define RTSP_RTP_FRAME_FLAG		0x00000008		/* RTP帧标志 */
#define RTSP_SDP_FRAME_FLAG		0x00000010		/* SDP帧标志 */
#define RTSP_MEDIA_INFO_FLAG	0x00000020		/* 媒体类型标志*/

/* 视频关键字标识 */
#define RTSP_VIDEO_FRAME_I		0x01		/* I帧 */
#define RTSP_VIDEO_FRAME_P		0x02		/* P帧 */
#define RTSP_VIDEO_FRAME_B		0x03		/* B帧 */
#if 0
/* 连接类型 */
typedef enum __RTP_CONNECT_TYPE
{
	RTP_OVER_TCP = 0x01,		/* RTP Over TCP */
	RTP_OVER_UDP					/* RTP Over UDP */
}RTP_CONNECT_TYPE;

/* 媒体信息 */
typedef struct _RTSP_MEDIA_INFO_T
{
	unsigned int u32VideoCodec;			/* 视频编码类型 */
	unsigned int u32VideoFps;			/* 视频帧率 */

	unsigned int u32AudioCodec;			/* 音频编码类型 */
	unsigned int u32AudioSamplerate;	/* 音频采样率 */
	unsigned int u32AudioChannel;		/* 音频通道数 */
}RTSP_MEDIA_INFO_T;

/* 帧信息 */
typedef struct
{
	unsigned int	codec;			/* 音视频格式 */

	unsigned int	type;			/* 视频帧类型 */
	unsigned char	fps;			/* 视频帧率 */
	unsigned short	width;			/* 视频宽 */
	unsigned short  height;			/* 视频高 */

	unsigned int	reserved1;		/* 保留参数1 */
	unsigned int	reserved2;		/* 保留参数2 */

	unsigned int	sample_rate;	/* 音频采样率 */
	unsigned int	channels;		/* 音频声道数 */

	unsigned int	length;			/* 音视频帧大小 */
	unsigned int    timestamp_usec;	/* 时间戳,微妙 */
	unsigned int	timestamp_sec;	/* 时间戳 秒 */

	float			bitrate;		/* 比特率 */
	float			losspacket;		/* 丢包率 */
}RTSP_FRAME_INFO;

#endif

/*
userData:	    用户数据
frameInfo:		帧结构数据
*/
typedef int(*RTSPCallBack)(void* userData, RTSP_FRAME_INFO* frameInfo);

/* 获取最后一次错误的错误码 */
int RTSP_GetErrCode();

/* 创建RTSPClient句柄  返回0表示成功，返回非0表示失败 */
int RTSP_Init(RTSP_Handle *handle);

/* 释放RTSPClient 参数为RTSPClient句柄 */
int RTSP_Deinit(RTSP_Handle *handle);

/* 设置数据回调 */
int RTSP_SetCallback(RTSP_Handle handle, RTSPCallBack _callback);

/* 打开网络流 */
int RTSP_OpenStream(RTSP_Handle handle, int _channelid, char *_url, RTP_CONNECT_TYPE _connType, unsigned int _mediaType, char *_username, char *_password, void *userPtr, int _reconn/*1000表示长连接,即如果网络断开自动重连, 其它值为连接次数*/, int outRtpPacket/*默认为0,即回调输出完整的帧, 如果为1,则输出RTP包*/);

/* 关闭网络流 */
int RTSP_CloseStream(RTSP_Handle handle);


