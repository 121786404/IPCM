#pragma once

extern "C"
{
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
}

//#define DUMP_PGM 1

typedef struct Image_Info_s
{
	unsigned char* data;
	int step;
	int width;
	int height;
	int cn;
}Image_Info_t;

class FFDecoder
{
public:
	FFDecoder();
	~FFDecoder();
	bool init(int codec_id);//, int width, int hegiht, int pixel_fmt);
	void release();
	bool decode(unsigned char *raw_buf, int buf_size, Image_Info_t* img);
private:
	bool m_bInited;
	unsigned int m_frame_count;
	Image_Info_t m_imginfo;

	AVCodec *m_codec;
	AVCodecContext *m_codec_ctx;
	SwsContext *m_sws_ctx;
	AVPacket m_avpkt;  
	AVFrame *m_yuv_picture;
	AVFrame m_rgb_picture;
};
/* ----------------------------------------------------------- */


