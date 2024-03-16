#pragma once
#include "ruleset.h"
#include "ruleset_bms.h"
#include "ruleset_network.h"

#include "cereal/cereal.hpp"
#include "cereal/types/string.hpp"

class RulesetBMSNetwork : public RulesetBMS, public vRulesetNetwork
{
protected:
    bool isInitialized = false;
    unsigned playerIndex = 0;

    PlayModifierRandomType randomLeft = PlayModifierRandomType::NONE;
    PlayModifierRandomType randomRight = PlayModifierRandomType::NONE;
    uint8_t assist_mask = 0;
    bool dpflip = false;

    bool _isFinished = false;

public:
    RulesetBMSNetwork(GameModeKeys keys, unsigned playerIndex);

public:
    bool isFinished() const override { return _isFinished; }

    // Register to InputWrapper
    void updatePress(InputMask& pg, const lunaticvibes::Time& t) override {}
    // Register to InputWrapper
    void updateHold(InputMask& hg, const lunaticvibes::Time& t) override {}
    // Register to InputWrapper
    void updateRelease(InputMask& rg, const lunaticvibes::Time& t) override {}
    // Register to InputWrapper
    void updateAxis(double s1, double s2, const lunaticvibes::Time& t) override {}
    // Called by ScenePlay
    void update(const lunaticvibes::Time& t) override;

public:
    struct PayloadInit
    {
        int8_t randomLeft = 0; // PlayModifierRandomType
        int8_t randomRight = 0; // PlayModifierRandomType
        int8_t gauge = 0;  // PlayModifierGaugeType
        uint8_t assist_mask = 0;
        bool dpflip = false;
        double health = 1.0;
        double maxMoneyScore = 200000.0;
        std::string modifierText;
        std::string modifierTextShort;

        template<class Archive>
        void serialize(Archive& ar)
        {
            ar(randomLeft);
            ar(randomRight);
            ar(gauge);
            ar(assist_mask);
            ar(dpflip);
            ar(health);
            ar(maxMoneyScore);
            ar(modifierText);
            ar(modifierTextShort);
        }
    };
    static std::vector<unsigned char> packInit(const std::shared_ptr<RulesetBMS>& local);
    bool unpackInit(const std::vector<unsigned char>& payload) override;

    struct PayloadFrame
    {
        // BasicData
        double health = 1.0;
        double acc = 0.0;
        double total_acc = 0.0;
        uint32_t combo;
        uint32_t maxCombo;
        uint32_t judge[32];

        // RulesetBMS
        double moneyScore = 0.0;
        unsigned exScore = 0;
        bool isFinished = false;
        bool isCleared = false;
        bool isFailed = false;

        template<class Archive>
        void serialize(Archive& ar)
        {
            ar(health);
            ar(acc);
            ar(total_acc);
            ar(combo);
            ar(maxCombo);
            ar(judge);
            ar(moneyScore);
            ar(exScore);
            ar(isFinished);
            ar(isCleared);
            ar(isFailed);
        }
    };
    static std::vector<unsigned char> packFrame(const std::shared_ptr<RulesetBMS>& local);
    bool unpackFrame(std::vector<unsigned char>& payload) override;
};

