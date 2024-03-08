#pragma once
#include "soundset.h"
#include "game/skin/skin.h"
#include <vector>
#include <string>

class SoundSetLR2 : public vSoundSet
{
public:
	SoundSetLR2();
	SoundSetLR2(Path p);
	~SoundSetLR2() override = default;
	void loadCSV(Path p);
	bool parseHeader(const std::vector<StringContent>& tokens);
	bool parseBody(const std::vector<StringContent>& tokens);

private:
	struct CustomFile
	{
		StringContent title;
		size_t value;
		std::vector<StringContent> label;

		// file
		StringContent filepath;
		std::vector<Path> pathList;
		size_t defIdx;
	};
	std::vector<CustomFile> customfiles;
	std::vector<size_t> customizeRandom;

protected:
	Path filePath;
	Path thumbnailPath;

	unsigned csvLineNumber = 0;          // line parsing index

	std::map<std::string, Path> soundFilePath;
	bool loadPath(const std::string& key, const std::string& rawpath);

public:
	Path getPathBGMSelect() const override;
	Path getPathBGMDecide() const override;

	Path getPathSoundOpenFolder() const override;
	Path getPathSoundCloseFolder() const override;
	Path getPathSoundOpenPanel() const override;
	Path getPathSoundClosePanel() const override;
	Path getPathSoundOptionChange() const override;
	Path getPathSoundDifficultyChange() const override;

	Path getPathSoundScreenshot() const override;

	Path getPathBGMResultClear() const override;
	Path getPathBGMResultFailed() const override;
	Path getPathSoundFailed() const override;
	Path getPathSoundLandmine() const override;
	Path getPathSoundScratch() const override;

	Path getPathBGMCourseClear() const override;
	Path getPathBGMCourseFailed() const override;

public:
	virtual size_t getCustomizeOptionCount() const;
	virtual SkinBase::CustomizeOption getCustomizeOptionInfo(size_t idx) const;
	StringPath getFilePath() const;

};