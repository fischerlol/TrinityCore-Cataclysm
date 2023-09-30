#include "ScriptMgr.h"
#include "Player.h"
#include "SpellAuras.h"
#include "Log.h"
#include "Pet.h"

class player_feign_death_check : public PlayerScript
{
public:
    player_feign_death_check() : PlayerScript("player_feign_death_check") { }

    void OnUpdate(Player* player, uint32 /*diff*/) override
    {
        const uint32 FEIGN_DEATH_SPELL = 5384;
        const uint32 TARGET_SPELL = 77769;

        // Check if the player had the Feign Death aura in the last tick.
        if (player->HasAura(FEIGN_DEATH_SPELL))
        {
            _hasFeignDeath[player->GetGUID()] = true;
        }
        else
        {
            // Check if they had it last tick but don't now.
            if (_hasFeignDeath[player->GetGUID()])
            {
                int32 remainingDuration = 0;
                bool hasExistingAura = false;

                if (Aura* existingAura = player->GetAura(TARGET_SPELL))
                {
                    hasExistingAura = true;
                    remainingDuration = existingAura->GetDuration();
                }

                // Cast the spell on the player.
                player->CastSpell(player, TARGET_SPELL, true);

                if (hasExistingAura)
                {
                    if (Aura* newAura = player->GetAura(TARGET_SPELL))
                    {
                        newAura->SetDuration(remainingDuration);
                    }
                }
                else
                {
                    if (Aura* newAura = player->GetAura(TARGET_SPELL))
                    {
                        newAura->Remove();
                    }
                }

                _hasFeignDeath[player->GetGUID()] = false;
            }
        }
    }


private:
    std::unordered_map<ObjectGuid, bool> _hasFeignDeath;
};


void AddSC_player_feign_death_check()
{
    new player_feign_death_check();
}
