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
	SpriteVideo(int w, int h, const std::shared_ptr<sVideo>& pVid, int srcLine = -1);
	~SpriteVideo() override = default;
	
public:
	void startPlaying();
	void stopPlaying();
};