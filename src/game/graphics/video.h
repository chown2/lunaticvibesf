#pragma once

#include <future>
#include <memory>
#include <set>
#include <shared_mutex>
#include <type_traits>

#include "common/types.h"
#include "graphics.h"

inline const std::set<std::string> video_file_extensions =
{
	".mpg",
	".mp2",
	".mpeg",
	".mpeg2",
	".mpe",
	".mpv",
	".flv",
	".mp4",
	".m4p",
	".m4v",
	".f4v",
	".avi",
	".wmv",
	".mkv",
	".webm",
	".mov",
	".m1v",
};

extern "C"
{
	struct AVFormatContext;
	struct AVCodec;
	struct AVCodecContext;
	struct AVFrame;
	struct AVPacket;
}
class SceneBase;
class SkinBase;

void video_init();

namespace lunaticvibes {
struct AVFrameDeleter
{
    void operator()(AVFrame *avp);
};
using AVFramePtr = std::unique_ptr<std::remove_pointer_t<AVFrame>, AVFrameDeleter>;
struct AVPacketDeleter
{
    void operator()(AVPacket *avp);
};
using AVPacketPtr = std::unique_ptr<std::remove_pointer_t<AVPacket>, AVPacketDeleter>;
} // namespace lunaticvibes

// libav decoder wrap
class sVideo
{
	friend class SceneBase;
	friend class SkinBase;
public:
	Path file;
	bool haveVideo = false;

private:

	// decoder params
	AVFormatContext *pFormatCtx = nullptr;
	const AVCodec *pCodec = nullptr;
	AVCodecContext *pCodecCtx = nullptr;
	lunaticvibes::AVFramePtr pFrame;
	lunaticvibes::AVPacketPtr pPacket;
	int videoIndex = -1;
	int64_t decoded_frames = 0;
	std::chrono::time_point<std::chrono::system_clock> startTime;
	std::future<void> decodeEnd;

	// render properties
	Path filePath;
	double speed = 1.0;
	int w = -1, h = -1;		// set in setVideo()
	bool playing = false;
	bool finished = false;
	bool decoding = false;
	bool firstFrame = true;
	bool valid = false;
	bool loop_playback = false;

public:
	sVideo() = default;
	sVideo(const Path& file, double speed = 1.0, bool loop = false) { setVideo(file, speed, loop); }
	virtual ~sVideo();
	int setVideo(const Path& file, double speed, bool loop = false);
	int unsetVideo();
	int getW() { return w; }
	int getH() { return h; }
	bool isPlaying() const { return playing; }

public:
	// properties
	Texture::PixelFormat getFormat();

public:
	// video playback control
	void startPlaying();
	void stopPlaying();
	void decodeLoop();

	int getDecodedFrames() { return decoded_frames; }
	AVFrame* getFrame() { return valid ? pFrame.get() : NULL; }

public:
	std::shared_mutex video_frame_mutex;

public:
	void setSpeed(double speed) { this->speed = speed; }
	void seek(int64_t second, bool backwards = false);
	
};