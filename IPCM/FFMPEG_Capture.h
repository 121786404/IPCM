#pragma once

enum
{
	FFMPEG_CAP_PROP_POS_MSEC = 0,
	FFMPEG_CAP_PROP_POS_FRAMES = 1,
	FFMPEG_CAP_PROP_POS_AVI_RATIO = 2,
	FFMPEG_CAP_PROP_FRAME_WIDTH = 3,
	FFMPEG_CAP_PROP_FRAME_HEIGHT = 4,
	FFMPEG_CAP_PROP_FPS = 5,
	FFMPEG_CAP_PROP_FOURCC = 6,
	FFMPEG_CAP_PROP_FRAME_COUNT = 7,
	FFMPEG_CAP_PROP_SAR_NUM = 40,
	FFMPEG_CAP_PROP_SAR_DEN = 41,
	FFMPEG_CAP_PROP_OUTHEIGHT = 80,
	FFMPEG_CAP_PROP_OUTWIDTH =81
};

struct Image_Info
{
	unsigned char* data;
	int step;
	int width;
	int height;
	int cn;
};


class CCapture_FFMPEG
{
public:
	CCapture_FFMPEG();
	virtual ~CCapture_FFMPEG();
	virtual void init();
	virtual bool open(const char* filename);
	virtual void close();
	virtual double getProperty(int) const;
	virtual bool setProperty(int propIdx, double propVal);

	bool grabFrame();
	bool retrieveFrame(Image_Info* img);

private:
	double  get_fps() const;
	int     get_bitrate() const;
	AVRational get_sample_aspect_ratio(AVStream *stream) const;
	double  r2d(AVRational r) const;
	int64_t dts_to_frame_number(int64_t dts);
	double  dts_to_sec(int64_t dts);

	AVFormatContext * ic;
	AVCodec         * avcodec;
	int               video_stream;
	AVStream        * video_st;
	AVFrame         * picture;
	AVFrame           rgb_picture;
	int64_t           picture_pts;

	AVPacket          packet;
	Image_Info      frame;
	struct SwsContext *img_convert_ctx;

	int64_t frame_number, first_frame_number;
	int out_width;
	int out_height;
	double eps_zero;
	/*
	'filename' contains the filename of the videosource,
	'filename==NULL' indicates that ffmpeg's seek support works
	for the particular file.
	'filename!=NULL' indicates that the slow fallback function is used for seeking,
	and so the filename is needed to reopen the file on backward seeking.
	*/
	char * filename;
	AVDictionary *dict;
};