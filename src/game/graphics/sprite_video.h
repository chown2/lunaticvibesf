#pragma once

#include "sprite.h"
#include "game/graphics/texture_extra.h"
#include "common/types.h"
#include <memory>

class sVideo;
struct AVFrame;

class SpriteVideo : public SpriteStatic
{
protected:
	int w, h;
	Texture::PixelFormat format;

public:
	SpriteVideo(int w, int h, std::shared_ptr<sVideo> pVid, int srcLine = -1);
	virtual ~SpriteVideo() = default;
	
public:
	void startPlaying();
	void stopPlaying();
};