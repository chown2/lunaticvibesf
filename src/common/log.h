#pragma once
#define PLOG_CAPTURE_FILE 
#include <plog/Log.h> // IWYU pragma: export
#include <string>
#include <sstream>

int InitLogger();

// 0:Debug 1:Info 2:Warning 3:Error 4:Off?
void SetLogLevel(int level);

int FreeLogger();
