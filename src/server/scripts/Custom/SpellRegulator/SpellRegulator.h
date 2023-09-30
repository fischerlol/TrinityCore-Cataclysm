#pragma once
#include "DatabaseEnv.h"
#include "QueryResult.h"
#include "Log.h"
#include "ScriptMgr.h"
#include "Timer.h"

class SpellRegulator
{
public:
    static SpellRegulator* instance()
    {
        static SpellRegulator instance;
        return &instance;
    }

    void Regulate(uint32& damage, uint32 spellId)
    {
        TC_LOG_INFO("server.worldserver", "Regulate called with spellId: %u and initial damage: %u", spellId, damage);

        if (RegulatorContainer.find(spellId) == RegulatorContainer.end())
        {
            TC_LOG_INFO("server.worldserver", "No regulation found for spellId: %u.", spellId);
            return;
        }

        float val = RegulatorContainer[spellId];

        TC_LOG_INFO("server.worldserver", "Regulation value found for spellId %u is: %f.", spellId, val);

        if (!val || val == 100.0f)
        {
            TC_LOG_INFO("server.worldserver", "Regulation value is either 0 or 100. No adjustment made.");
            return;
        }

        uint32 originalDamage = damage;
        damage = (damage / 100.0f) * val;
        TC_LOG_INFO("server.worldserver", "Adjusted damage for spellId %u from %u to %u.", spellId, originalDamage, damage);
    }


    void LoadFromDB()
    {
        RegulatorContainer.clear();
        uint32 msTime = getMSTime();
        QueryResult result = WorldDatabase.Query("SELECT * FROM spellregulator");

        if (!result)
            return;

        uint32 count = 0;
        do {
            Field* fields = result->Fetch();
            RegulatorContainer[fields[0].GetUInt32()] = fields[1].GetFloat();
            ++count;
        } while (result->NextRow());
        TC_LOG_INFO("server.loading", "Loaded %u regulated spells in %u ms", count, GetMSTimeDiffToNow(msTime));
    }

private:
    std::unordered_map<uint32, float> RegulatorContainer; // spellid, percentage
};

#define sSpellRegulator SpellRegulator::instance()

class RegulatorLoader : public WorldScript
{
public:
    RegulatorLoader() : WorldScript("SpellRegulatorLoader") {}

    void OnStartup() override
    {
        sSpellRegulator->LoadFromDB();
    }
};
