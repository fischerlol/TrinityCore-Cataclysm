#include "ScriptMgr.h"
#include "Player.h"
#include "ObjectAccessor.h"
#include <unordered_map>

class CustomPlayerScript : public WorldScript
{
public:
    CustomPlayerScript() : WorldScript("CustomPlayerScript") {}

    void OnUpdate(uint32 /*diff*/) override
    {
        // This map holds player GUIDs for those who have the aura 109864
        static std::unordered_map<uint32, bool> playerAuraStates;

        // Iterate through all online players
        HashMapHolder<Player>::MapType const& players = ObjectAccessor::GetPlayers();
        for (const auto& itr : players)
        {
            Player* player = itr.second;
            if (!player)
                continue;

            uint32 playerGUID = player->GetGUID().GetCounter();

            // Check if player has aura 109864
            if (player->HasAura(109864))
            {
                // Set the flag for this player
                playerAuraStates[playerGUID] = true;
            }
            else
            {
                // Check if player previously had the aura
                if (playerAuraStates.find(playerGUID) != playerAuraStates.end() && playerAuraStates[playerGUID])
                {
                    // Remove aura 109863 if the player has it
                    if (player->HasAura(109863))
                    {
                        player->RemoveAura(109863);
                    }

                    // Reset the flag for this player
                    playerAuraStates[playerGUID] = false;
                }
            }
        }
    }
};

void AddSC_CustomPlayerScript()
{
    new CustomPlayerScript();
}
