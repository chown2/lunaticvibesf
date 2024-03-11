#pragma once
#include "ruleset.h"
#include <vector>

class vRulesetNetwork: virtual public RulesetBase
{
public:
    static std::vector<unsigned char> packInit(const std::shared_ptr<RulesetBase>& local);
    bool virtual unpackInit(const std::vector<unsigned char>& payload) = 0;
    static std::vector<unsigned char> packFrame(const std::shared_ptr<RulesetBase>& local);
    bool virtual unpackFrame(std::vector<unsigned char>& payload) = 0;
};