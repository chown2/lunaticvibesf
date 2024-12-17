#include <common/in_test_mode.h>
#include <common/log.h>
#include <common/sysutil.h>
#include <config/config_mgr.h>

#include <gmock/gmock.h>

bool lunaticvibes::in_test_mode()
{
    return true;
}

int main(int argc, char** argv)
{
    executablePath = GetExecutablePath();
    std::filesystem::current_path(executablePath);
    std::filesystem::current_path("test"); // TODO: maybe remove this?

    SetThreadAsMainThread();

    lunaticvibes::InitLogger("apptest.log");
    lunaticvibes::SetLogLevel(lunaticvibes::LogLevel::Verbose);

    ConfigMgr::init();

    ::testing::InitGoogleTest(&argc, argv);
    ::testing::InitGoogleMock(&argc, argv);

    int n = RUN_ALL_TESTS();

    return n;
}
