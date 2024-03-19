#pragma once

#include <map>

#include "common/beat.h"
#include "video.h"

class TextureVideo : public Texture
{
protected:
	std::shared_ptr<sVideo> pVideo;
	unsigned decoded_frames = ~0;
	PixelFormat format;
	bool updated = false;

public:
	TextureVideo(std::shared_ptr<sVideo> pv);
	~TextureVideo() override;
	void start();
	void stop();
	void seek(int64_t sec);
	void setSpeed(double speed);
	void update();
	void reset();

	void draw(RectF dstRect,
		const Color c, const BlendMode blend, const bool filter, const double angleInDegrees) const override;
	void draw(RectF dstRect,
		const Color c, const BlendMode blend, const bool filter, const double angleInDegrees, const Point& center) const override;
	void draw(const Rect& srcRect, RectF dstRect,
		const Color c, const BlendMode blend, const bool filter, const double angleInDegrees) const override;
	void draw(const Rect& srcRect, RectF dstRect,
		const Color c, const BlendMode blend, const bool filter, const double angleInDegrees, const Point& center) const override;

	void stopUpdate();

private:
	static std::shared_ptr<std::shared_mutex> texMapMutex;
	static std::shared_ptr<std::map<uintptr_t, TextureVideo*>> textures;
	std::shared_ptr<std::shared_mutex> pTexMapMutex;	// ref counter guard
	std::shared_ptr<std::map<uintptr_t, TextureVideo*>> pTextures;	// ref counter guard
public:
	static void updateAll();
};

///////////////////////////////////////////////////////////////////////////////

class ChartObjectBMS;
class TextureBmsBga: public Texture
{
protected:
	mutable std::shared_mutex idxLock;
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
			lunaticvibes::Time time;
			bool base, layer, poor;
		};

		std::shared_ptr<Texture> pt = nullptr;
		lunaticvibes::Time playStartTime = 0;	// video

		obj() = default;
		obj(Ty t, std::shared_ptr<Texture> pt) :type(t), pt(pt) {}
	};

protected:
	std::map<size_t, obj> objs;
	std::map<size_t, obj> objs_layer;
	std::vector<std::pair<lunaticvibes::Time, size_t>> baseSlot, layerSlot, poorSlot;
	decltype(baseSlot.begin()) baseIt;
	decltype(layerSlot.begin()) layerIt;
	decltype(poorSlot.begin()) poorIt;
	bool inPoor = false;
	
public:
	TextureBmsBga(int x = 256, int y = 256) : Texture(nullptr, x, y)
	{
		baseIt = baseSlot.begin();
		layerIt = layerSlot.begin();
		poorIt = poorSlot.begin();
	}
	~TextureBmsBga() override
	{
		stopUpdate();
	}

public:
	bool addBmp(size_t idx, Path path);
	bool setSlot(size_t idx, const lunaticvibes::Time& time, bool base, bool layer, bool poor);
	void sortSlot();
	bool setSlotFromBMS(ChartObjectBMS& bms);
	virtual void seek(const lunaticvibes::Time& t);

	virtual void update(const lunaticvibes::Time& t, bool poor);
	void draw(const Rect& srcRect, RectF dstRect,
		const Color c, const BlendMode blend, const bool filter, const double angleInDegrees) const override;
	void draw(const Rect& srcRect, RectF dstRect,
		const Color c, const BlendMode blend, const bool filter, const double angleInDegrees,
		const Point& center) const override;

	void reset();
	void clear();

	void setLoaded();
	void stopUpdate();

	void setVideoSpeed();
};


///////////////////////////////////////////////////////////////////////////////

class TextureDynamic : public Texture
{
protected:
	Texture* _dynTexture = nullptr;

public:
	TextureDynamic();
	~TextureDynamic() override = default;

public:
	void setPath(const Path& path);

	void draw(RectF dstRect,
		const Color c, const BlendMode blend, const bool filter, const double angleInDegrees) const override;
	void draw(RectF dstRect,
		const Color c, const BlendMode blend, const bool filter, const double angleInDegrees, const Point& center) const override;
	void draw(const Rect& srcRect, RectF dstRect,
		const Color c, const BlendMode blend, const bool filter, const double angleInDegrees) const override;
	void draw(const Rect& srcRect, RectF dstRect,
		const Color c, const BlendMode blend, const bool filter, const double angleInDegrees, const Point& center) const override;
};