#include "stdafx.h"
#include "FFMPEG_Capture.h"

static int get_number_of_cpus(void)
{
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	return (int)sysinfo.dwNumberOfProcessors;
}


#define LIBAVFORMAT_INTERRUPT_TIMEOUT_MS 5000

static
inline LARGE_INTEGER get_filetime_offset()
{
	SYSTEMTIME s;
	FILETIME f;
	LARGE_INTEGER t;

	s.wYear = 1970;
	s.wMonth = 1;
	s.wDay = 1;
	s.wHour = 0;
	s.wMinute = 0;
	s.wSecond = 0;
	s.wMilliseconds = 0;
	SystemTimeToFileTime(&s, &f);
	t.QuadPart = f.dwHighDateTime;
	t.QuadPart <<= 32;
	t.QuadPart |= f.dwLowDateTime;
	return t;
}

static
inline void get_monotonic_time(timespec *tv)
{
	LARGE_INTEGER           t;
	FILETIME				f;
	double                  microseconds;
	static LARGE_INTEGER    offset;
	static double           frequencyToMicroseconds;
	static int              initialized = 0;
	static BOOL             usePerformanceCounter = 0;

	if (!initialized)
	{
		LARGE_INTEGER performanceFrequency;
		initialized = 1;
		usePerformanceCounter = QueryPerformanceFrequency(&performanceFrequency);
		if (usePerformanceCounter)
		{
			QueryPerformanceCounter(&offset);
			frequencyToMicroseconds = (double)performanceFrequency.QuadPart / 1000000.;
		}
		else
		{
			offset = get_filetime_offset();
			frequencyToMicroseconds = 10.;
		}
	}

	if (usePerformanceCounter)
	{
		QueryPerformanceCounter(&t);
	}
	else {
		GetSystemTimeAsFileTime(&f);
		t.QuadPart = f.dwHighDateTime;
		t.QuadPart <<= 32;
		t.QuadPart |= f.dwLowDateTime;
	}

	t.QuadPart -= offset.QuadPart;
	microseconds = (double)t.QuadPart / frequencyToMicroseconds;
	t.QuadPart = (LONGLONG)microseconds;
	tv->tv_sec = t.QuadPart / 1000000;
	tv->tv_nsec = (t.QuadPart % 1000000) * 1000;
}

static
inline timespec get_monotonic_time_diff(timespec start, timespec end)
{
    timespec temp;
    if (end.tv_nsec - start.tv_nsec < 0)
    {
        temp.tv_sec = end.tv_sec - start.tv_sec - 1;
        temp.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
    }
    else
    {
        temp.tv_sec = end.tv_sec - start.tv_sec;
        temp.tv_nsec = end.tv_nsec - start.tv_nsec;
    }
    return temp;
}

static
inline double get_monotonic_time_diff_ms(timespec time1, timespec time2)
{
    timespec delta = get_monotonic_time_diff(time1, time2);
    double milliseconds = delta.tv_sec * 1000 + (double)delta.tv_nsec / 1000000.0;

    return milliseconds;
}

static
inline void _opencv_ffmpeg_free(void** ptr)
{
	if (*ptr) free(*ptr);
	*ptr = 0;
}

static
inline int _opencv_ffmpeg_interrupt_callback(void *ptr)
{
	AVInterruptCallbackMetadata* metadata = (AVInterruptCallbackMetadata*)ptr;
	assert(metadata);

	timespec now;
	get_monotonic_time(&now);

	metadata->timeout = get_monotonic_time_diff_ms(metadata->value, now) > metadata->timeout_after_ms;

	return metadata->timeout ? -1 : 0;
}


CCapture_FFMPEG::CCapture_FFMPEG()
{
	init();
}

CCapture_FFMPEG::~CCapture_FFMPEG()
{

}
void CCapture_FFMPEG::init()
{
	ic = 0;
	video_stream = -1;
	video_st = 0;
	picture = 0;
	picture_pts = AV_NOPTS_VALUE;
	first_frame_number = -1;
	memset(&rgb_picture, 0, sizeof(rgb_picture));
	memset(&frame, 0, sizeof(frame));
	filename = 0;
	memset(&packet, 0, sizeof(packet));
	av_init_packet(&packet);
	img_convert_ctx = 0;

	avcodec = 0;
	frame_number = 0;
	eps_zero = 0.000025;

	dict = NULL;
	out_width = -1;
	out_height = -1;
	av_register_all();
	avformat_network_init();
}


void CCapture_FFMPEG::close()
{
	if (img_convert_ctx)
	{
		sws_freeContext(img_convert_ctx);
		img_convert_ctx = 0;
	}

	if (picture)
	{
		av_frame_free(&picture);
	}

	if (video_st)
	{
		avcodec_close(video_st->codec);
		video_st = NULL;
	}

	if (ic)
	{
		avformat_close_input(&ic);
		ic = NULL;
	}

	av_frame_unref(&rgb_picture);

	// free last packet if exist
	if (packet.data) {
		av_packet_unref(&packet);
		packet.data = NULL;
	}

	if (dict != NULL)
		av_dict_free(&dict);

	init();
}


bool CCapture_FFMPEG::open(const char* _filename)
{
	unsigned i;
	bool valid = false;

	close();


    /* interrupt callback */
    interrupt_metadata.timeout_after_ms = LIBAVFORMAT_INTERRUPT_TIMEOUT_MS;
    get_monotonic_time(&interrupt_metadata.value);

    ic = avformat_alloc_context();
    ic->interrupt_callback.callback = _opencv_ffmpeg_interrupt_callback;
    ic->interrupt_callback.opaque = &interrupt_metadata;


	av_dict_set(&dict, "rtsp_transport", "tcp", 0);
	int err = avformat_open_input(&ic, _filename, NULL, &dict);

	if (err < 0)
	{
		OutputDebugString(_T("Error opening file"));
		goto exit_func;
	}
	err = avformat_find_stream_info(ic, NULL);

	if (err < 0)
	{
		OutputDebugString(_T("Could not find codec parameters"));
		goto exit_func;
	}
	for (i = 0; i < ic->nb_streams; i++)
	{
		AVCodecContext *enc = ic->streams[i]->codec;
		enc->thread_count = get_number_of_cpus();
		if (AVMEDIA_TYPE_VIDEO == enc->codec_type && video_stream < 0)
		{
			// backup encoder' width/height
			int enc_width = enc->width;
			int enc_height = enc->height;

			AVCodec *codec = avcodec_find_decoder(enc->codec_id);
			if (!codec || avcodec_open2(enc, codec, NULL) < 0)
				goto exit_func;

			video_stream = i;
			video_st = ic->streams[i];

			picture = av_frame_alloc();
			frame.width = enc->width;
			frame.height = enc->height;
			frame.cn = 3;
			frame.step = 0;
			frame.data = NULL;
			break;
		}
	}

	if (video_stream >= 0) valid = true;

exit_func:

	if (!valid)
		close();

	return valid;
}

bool CCapture_FFMPEG::grabFrame()
{
	bool valid = false;
	int got_picture;

	int count_errs = 0;
	const int max_number_of_attempts = 1 << 9;

	if (!ic || !video_st)  return false;

	if (ic->streams[video_stream]->nb_frames > 0 &&
		frame_number > ic->streams[video_stream]->nb_frames)
		return false;

	picture_pts = AV_NOPTS_VALUE;

	// get the next frame
	while (!valid)
	{
		av_packet_unref(&packet);
        if (interrupt_metadata.timeout)
        {
            valid = false;
            break;
        }
		int ret = av_read_frame(ic, &packet);
		
		if (ret == AVERROR(EAGAIN)) continue;
		/* else if (ret < 0) break; */
		
		if (packet.stream_index != video_stream)
		{
			av_packet_unref(&packet);
			count_errs++;
			if (count_errs > max_number_of_attempts)
				break;
			continue;
		}
		
		avcodec_decode_video2(video_st->codec, picture, &got_picture, &packet);
		
		// Did we get a video frame?
		if (got_picture)
		{
			//picture_pts = picture->best_effort_timestamp;
			if (picture_pts == AV_NOPTS_VALUE)
				picture_pts = packet.pts != AV_NOPTS_VALUE && packet.pts != 0 ? packet.pts : packet.dts;
			frame_number++;
			valid = true;
            get_monotonic_time(&interrupt_metadata.value);
		}
		else
		{
			count_errs++;
			if (count_errs > max_number_of_attempts)
				break;
		}
	}

	if (valid && first_frame_number < 0)
		first_frame_number = dts_to_frame_number(picture_pts);

	// return if we have a new picture or not
	return valid;
}

static bool ScaleRectang(int src_width, int src_height, int* dst_width, int* dst_height)
{
	if (src_width == 1920)
	{
		if (src_height == 1080)
		{
			*dst_width = 576;
			*dst_height = 324;
			return true;
		}
		else if (src_height == 1088)
		{
			*dst_width = 540;
			*dst_height = 272;
			return true;
		}
	}
	else if (src_width == 1280 && src_height == 720)
	{
		*dst_width = 512;
		*dst_height = 288;
		return true;
	}
	else if (src_width == 640 && src_height == 480)
	{
		*dst_width = 640;
		*dst_height = 480;
		return true;
	}
	return false;
}

bool CCapture_FFMPEG::retrieveFrame(Image_Info* img)
{
	if (!video_st || !picture->data[0])
		return false;

	if (img_convert_ctx == NULL ||
		frame.width != video_st->codec->width ||
		frame.height != video_st->codec->height ||
		frame.data == NULL)
	{
		// Some sws_scale optimizations have some assumptions about alignment of data/step/width/height
		// Also we use coded_width/height to workaround problem with legacy ffmpeg versions (like n0.8)
		//int buffer_width = video_st->codec->coded_width, buffer_height = video_st->codec->coded_height;
		int src_width  = video_st->codec->coded_width;
		int src_height = video_st->codec->coded_height;
		int dst_width, dst_height;
		if (out_width != -1 || out_height != -1)
		{
			dst_width = out_width;
			dst_height = out_height;
		}
		else 
		{
			if (ScaleRectang(src_width, src_height, &out_width, &out_height))
			{
				dst_width = out_width;
				dst_height = out_height;
			}
			else
			{
				dst_width = src_width;
				dst_height = src_height;
			}
		}

		img_convert_ctx = sws_getCachedContext(
			img_convert_ctx,
			src_width, src_height,
			video_st->codec->pix_fmt,
			dst_width, dst_height,
			AV_PIX_FMT_BGR24,
			SWS_BICUBIC, //SWS_FAST_BILINEAR
			NULL, NULL, NULL
			);

		if (img_convert_ctx == NULL)
			return false;//CV_Error(0, "Cannot initialize the conversion context!");

		av_frame_unref(&rgb_picture);
		rgb_picture.format = AV_PIX_FMT_BGR24;
		rgb_picture.width = dst_width;
		rgb_picture.height = dst_height;
		rgb_picture.linesize[0] = dst_width*3;
	
		if (0 != av_frame_get_buffer(&rgb_picture, 32))
		{
			OutputDebugString(_T("OutOfMemory"));
			return false;
		}

		frame.width = dst_width;
		frame.height = dst_height;
		frame.cn = 3;
		frame.data = rgb_picture.data[0];
		frame.step = rgb_picture.linesize[0];
	}

	sws_scale(
		img_convert_ctx,
		picture->data,
		picture->linesize,
		0, video_st->codec->coded_height,
		rgb_picture.data,
		rgb_picture.linesize
		);

	img->data = frame.data;
	img->step = frame.step;
	img->width = frame.width;
	img->height = frame.height;
	img->cn = frame.cn;

	return true;
}

double CCapture_FFMPEG::getProperty(int property_id) const
{
	if (!video_st) return 0;

	switch (property_id)
	{
	case FFMPEG_CAP_PROP_POS_MSEC:
		return 1000.0*(double)frame_number / get_fps();
	case FFMPEG_CAP_PROP_POS_FRAMES:
		return (double)frame_number;
	case FFMPEG_CAP_PROP_POS_AVI_RATIO:
		return r2d(ic->streams[video_stream]->time_base);
	case FFMPEG_CAP_PROP_FRAME_WIDTH:
		return (double)frame.width;
	case FFMPEG_CAP_PROP_FRAME_HEIGHT:
		return (double)frame.height;
	case FFMPEG_CAP_PROP_FPS:
		return get_fps();
	case FFMPEG_CAP_PROP_FOURCC:
		return (double)video_st->codec->codec_tag;
	case FFMPEG_CAP_PROP_SAR_NUM:
		return get_sample_aspect_ratio(ic->streams[video_stream]).num;
	case FFMPEG_CAP_PROP_SAR_DEN:
		return get_sample_aspect_ratio(ic->streams[video_stream]).den;
	default:
		break;
	}

	return 0;
}

bool CCapture_FFMPEG::setProperty(int property_id, double value)
{
	if (!video_st) return false;

	switch (property_id)
	{
	case FFMPEG_CAP_PROP_OUTHEIGHT:
		break;
	case FFMPEG_CAP_PROP_OUTWIDTH:
		break;
	default:
		return false;
	}

	return true;
}

double CCapture_FFMPEG::r2d(AVRational r) const
{
	return r.num == 0 || r.den == 0 ? 0. : (double)r.num / (double)r.den;
}

int CCapture_FFMPEG::get_bitrate() const
{
	return (int)ic->bit_rate;
}

double CCapture_FFMPEG::get_fps() const
{
	double fps = r2d(ic->streams[video_stream]->avg_frame_rate);

	if (fps < eps_zero)
	{
		fps = r2d(ic->streams[video_stream]->avg_frame_rate);
	}

	if (fps < eps_zero)
	{
		fps = 1.0 / r2d(ic->streams[video_stream]->codec->time_base);
	}

	return fps;
}

int64_t CCapture_FFMPEG::dts_to_frame_number(int64_t dts)
{
	double sec = dts_to_sec(dts);
	return (int64_t)(get_fps() * sec + 0.5);
}

AVRational CCapture_FFMPEG::get_sample_aspect_ratio(AVStream *stream) const
{
	AVRational undef = { 0, 1 };
	AVRational stream_sample_aspect_ratio = stream ? stream->sample_aspect_ratio : undef;
	AVRational frame_sample_aspect_ratio = stream && stream->codec ? stream->codec->sample_aspect_ratio : undef;

	av_reduce(&stream_sample_aspect_ratio.num, &stream_sample_aspect_ratio.den,
		stream_sample_aspect_ratio.num, stream_sample_aspect_ratio.den, INT_MAX);
	if (stream_sample_aspect_ratio.num <= 0 || stream_sample_aspect_ratio.den <= 0)
		stream_sample_aspect_ratio = undef;

	av_reduce(&frame_sample_aspect_ratio.num, &frame_sample_aspect_ratio.den,
		frame_sample_aspect_ratio.num, frame_sample_aspect_ratio.den, INT_MAX);
	if (frame_sample_aspect_ratio.num <= 0 || frame_sample_aspect_ratio.den <= 0)
		frame_sample_aspect_ratio = undef;

	if (stream_sample_aspect_ratio.num)
		return stream_sample_aspect_ratio;
	else
		return frame_sample_aspect_ratio;
}

double CCapture_FFMPEG::dts_to_sec(int64_t dts)
{
	return (double)(dts - ic->streams[video_stream]->start_time) *
		r2d(ic->streams[video_stream]->time_base);
}