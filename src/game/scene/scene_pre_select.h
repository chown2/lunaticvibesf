#pragma once
#include "scene.h"
#include "scene_context.h"

class ScenePreSelect: public SceneBase
{
public:
    ScenePreSelect();
    ~ScenePreSelect() override;

protected:
    // Looper callbacks
    void _updateAsync() override;
    std::function<void()> _updateCallback;

    void updateLoadSongs();
    void updateLoadTables();
    void updateLoadCourses();
    void loadFinished();

    void updateImgui() override;


protected:
    SongListProperties rootFolderProp;
    bool startedLoadSong = false;
    bool startedLoadTable = false;
    bool startedLoadCourse = false;
    std::chrono::system_clock::time_point loadSongTimer;
    std::future<void> loadSongEnd;
    std::future<void> loadTableEnd;
    std::future<void> loadCourseEnd;
    int prevChartLoaded = 0;
    std::string textHint;
    std::string textHint2;
    bool loadingFinished = false;

public:
    bool isLoadingFinished() const;
};