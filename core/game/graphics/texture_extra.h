#pragma once
#include <map>
#include <set>
#include <variant>
#include "asynclooper.h"
#include "graphics.h"
#include "video.h"
#include "beat.h"

extern const std::set<std::string> video_file_extensions;

class TextureVideo : public Texture, public AsyncLooper
{
protected:
	std::shared_ptr<sVideo> pVideo;
	unsigned decoded_frames;
	PixelFormat format;

public:
	TextureVideo(std::shared_ptr<sVideo> pv);
	virtual ~TextureVideo();
	void start();
	void seek(int64_t sec);
	void update();
};

class ScrollBMS;
class TextureBmsBga: public Texture
{
protected:
	size_t baseIdx = INDEX_INVALID;
	size_t layerIdx = INDEX_INVALID;
	size_t poorIdx = INDEX_INVALID;

	class obj
	{
	public:
		enum class Ty{
			EMPTY,
			PIC,
			VIDEO
		} type = Ty::EMPTY;

		struct playslot {
			timestamp time;
			bool base, layer, poor;
		};

		std::shared_ptr<Texture> pt = nullptr;
		timestamp playStartTime = 0;	// video

		obj() = default;
		obj(Ty t, std::shared_ptr<Texture> pt) :type(t), pt(pt) {}
	};

protected:
	std::map<size_t, obj> objs;
	std::vector<std::pair<timestamp, size_t>> baseSlot, layerSlot, poorSlot;
	decltype(baseSlot.begin()) baseIt;
	decltype(layerSlot.begin()) layerIt;
	decltype(poorSlot.begin()) poorIt;
	bool inPoor;
	
public:
	TextureBmsBga(int x = 256, int y = 256) : Texture(nullptr, x, y) {}
	virtual ~TextureBmsBga() = default;

public:
	bool addBmp(size_t idx, const Path& path);
	bool setSlot(size_t idx, timestamp time, bool base, bool layer, bool poor);
	void sortSlot();
	bool setSlotFromBMS(ScrollBMS& bms);
	virtual void seek(timestamp t);

	virtual void update(timestamp t, bool poor);
	virtual void draw(const Rect& srcRect, Rect dstRect,
		const Color c, const BlendMode blend, const bool filter, const double angleInDegrees) const override;
	virtual void draw(const Rect& srcRect, Rect dstRect,
		const Color c, const BlendMode blend, const bool filter, const double angleInDegrees,
		const Point& center) const override;

};