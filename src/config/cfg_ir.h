#pragma once

#include <config/config.h>
#include <string>

namespace lunaticvibes
{

class ConfigIr final : public vConfig
{
private:
    std::string _id;

public:
    ConfigIr() = delete;
    ConfigIr(const std::string& profile, const std::string& ir_id);
    ~ConfigIr() override;

    void setDefaults() noexcept override;

    [[nodiscard]] const std::string& id() const { return _id; }
};

} // namespace lunaticvibes
