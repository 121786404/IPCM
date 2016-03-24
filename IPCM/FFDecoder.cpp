#include "stdafx.h"
#include "FFDecoder.h"

/* ------------------------------------------------------------ */
FFDecoder::FFDecoder()
{
	//m_width = 0;
	//m_hegiht = 0;
	m_bInited = false;
}

FFDecoder::~FFDecoder()
{

}


/* ------------------------------------------------------------ */

bool  FFDecoder::init(int codec_id)
{
	if (m_bInited == true)
	{
		TRACE("ffdecoder has inited!\n");
		return true;
	}

	avcodec_register_all();
	av_init_packet(&m_avpkt);
	memset(&m_rgb_picture, 0, sizeof(m_rgb_picture));

	m_codec = avcodec_find_decoder((AVCodecID)codec_id);
	if (!m_codec) {
		TRACE("codec not found\n");
		goto INIT_FAIL;
	}

	m_codec_ctx = avcodec_alloc_context3(m_codec);
	if (!m_codec_ctx) {
		TRACE("Can not allocate decoder context\n");
		goto INIT_FAIL;
	}
	if (avcodec_open2(m_codec_ctx, m_codec, NULL) < 0) {
		TRACE("could not open codec\n");
		goto INIT_FAIL;
	}

	m_yuv_picture = av_frame_alloc();
	if (!m_yuv_picture) {
		TRACE("Could not allocate video frame\n");
		goto INIT_FAIL;
	}

	TRACE("Decoder init finish\n");
	m_bInited = true;
	m_frame_count = 0;
	m_sws_ctx = NULL;
	memset(&m_imginfo, 0, sizeof(Image_Info_t));
	return true;

INIT_FAIL:
	release();
	return false;
}
/* --------------------------------------------------------------------------------------- */
void FFDecoder::release()
{
	if (m_sws_ctx)
	{
		sws_freeContext(m_sws_ctx);
		m_sws_ctx = NULL;
	}

	if(m_yuv_picture)
		av_frame_free(&m_yuv_picture);
	
	if (m_codec_ctx)
	{
		avcodec_close(m_codec_ctx);
		m_codec_ctx = NULL;
	}

	av_frame_unref(&m_rgb_picture);
	
	if (m_avpkt.data) {
		av_packet_unref(&m_avpkt);
		m_avpkt.data = NULL;
	}
	
	m_bInited = false;
	m_frame_count = 0;
}

#ifdef DUMP_PGM
static void pgm_save(unsigned char *buf, int wrap, int xsize, int ysize,
	char *filename)
{
	FILE *f;
	int i;

	f = fopen(filename, "w");
	fprintf(f, "P5\n%d %d\n%d\n", xsize, ysize, 255);
	for (i = 0; i < ysize; i++)
		fwrite(buf + i * wrap, 1, xsize, f);
	fclose(f);
}
#endif

/* --------------------------------------------------------------------------------------- */
bool FFDecoder::decode(unsigned char *raw_buf, int buf_size, Image_Info_t *img)
{
	m_avpkt.data = raw_buf;
	m_avpkt.size = buf_size;
	if (m_avpkt.size == 0) {
		TRACE("0 size packet\n");
		return false;
	}

	int got_picture = 0;
	int len = avcodec_decode_video2(m_codec_ctx, m_yuv_picture, &got_picture, &m_avpkt);
	if (len < 0) {
		TRACE("Error while decoding frame \n");
		return false;
	}

	if (false == got_picture)
		return false;

	assert(len == m_avpkt.size);

#ifdef DUMP_PGM
	TRACE("decode got frame len %d\n", m_frame->pkt_size);
	m_frame_count++;
	char buf[1024];
	fflush(stdout);
	/* the picture is allocated by the decoder, no need to free it */
	snprintf(buf, sizeof(buf), "test%02d.pgm", m_frame_count);
	pgm_save(m_frame->data[0], m_frame->linesize[0], m_frame->width, m_frame->height, buf);
#endif

	if (m_sws_ctx == NULL)
	{
		int buffer_width = m_codec_ctx->coded_width;
		int	buffer_height = m_codec_ctx->coded_height;

		m_sws_ctx = sws_getCachedContext(
			m_sws_ctx,
			buffer_width, buffer_height,
			(AVPixelFormat)m_yuv_picture->format,
			buffer_width, buffer_height,
			AV_PIX_FMT_BGR24,
			SWS_BICUBIC, //SWS_FAST_BILINEAR
			NULL, NULL, NULL
			);

		if (m_sws_ctx == NULL)
			return false;

		av_frame_unref(&m_rgb_picture);
		m_rgb_picture.format = AV_PIX_FMT_BGR24;
		m_rgb_picture.width = buffer_width;
		m_rgb_picture.height = buffer_height;

		if (0 != av_frame_get_buffer(&m_rgb_picture, 32))
		{
			TRACE("OutOfMemory\n");
			return false;
		}

		m_imginfo.width = m_codec_ctx->width;
		m_imginfo.height = m_codec_ctx->height;
		m_imginfo.cn = 3;
		m_imginfo.data = m_rgb_picture.data[0];
		m_imginfo.step = m_rgb_picture.linesize[0];
	}

	sws_scale(
		m_sws_ctx,
		m_yuv_picture->data,
		m_yuv_picture->linesize,
		0, m_codec_ctx->coded_height,
		m_rgb_picture.data,
		m_rgb_picture.linesize
		);

	memcpy(img, &m_imginfo, sizeof(Image_Info_t));

	return true;
}



