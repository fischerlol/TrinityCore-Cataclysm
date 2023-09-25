#include "ScriptMgr.h"
#include "Log.h"
#include "Pet.h"
#include "ObjectMgr.h"
#include "Log.h"

class OnLoginPlayerScripts : public PlayerScript
{
public:
    OnLoginPlayerScripts() : PlayerScript("OnLoginPlayerScripts") {}

    void OnLogin(Player* player, bool loginFirst) override
    {
        int STARTER_QUESTS[33] = { 12593, 12619, 12842, 12848, 12636, 12641, 12657, 12678, 12679, 12680, 12687, 12698, 12701, 12706, 12716, 12719, 12720, 12722, 12724, 12725, 12727, 12733, -1, 12751,
        12754, 12755, 12756, 12757, 12779, 12801, 13165, 13166 };

        if (loginFirst) // This is the player's first login
        {
            switch (player->getClass())
            {
            case CLASS_DEATH_KNIGHT:
                for (int questId : STARTER_QUESTS)
                {
                    Quest const* questTemplate = sObjectMgr->GetQuestTemplate(questId);
                    if (questTemplate == nullptr)
                    {
                        TC_LOG_INFO("server.worldserver", "Quest %u not found", questId);
                        continue;
                    }

                    if (player->GetQuestStatus(questId) == QUEST_STATUS_NONE)
                    {
                        player->AddQuest(questTemplate, nullptr);
                        player->RewardQuest(questTemplate, 0, player, false);
                    }
                }

                break;
            }


            float x = 4331.907715f; // Replace with the X coordinate of the destination
            float y = -2881.159424f; // Replace with the Y coordinate of the destination
            float z = 0.921211f; // Replace with the Z coordinate of the destination
            float o = 2.265909f; // Replace with the orientation of the destination
            uint32 mapId = 0; // Replace with the map ID of the destination
            //X: 4331.907715 Y: -2881.159424 Z: 0.921311 Orientation: 2.281618
            player->TeleportTo(mapId, x, y, z, o);
        }

        
    }
};

class ScalePetOnLogin : public PlayerScript
{
public:
    ScalePetOnLogin() : PlayerScript("ScalePetOnLogin") {}

    void OnLogin(Player* player, bool /*firstLogin*/) override
    {
        TC_LOG_INFO("server.worldserver", "Player %s has logged in", player->GetName().c_str());

        Pet* pet = player->GetPet();

        if (pet)
        {
            pet->SetObjectScale(1.0f); // Set the pet's scale to 1.0
            TC_LOG_INFO("server.worldserver", "Set scale of pet for player %s to 1.0", player->GetName().c_str());
        }
    }
};

void AddSC_OnLoginPlayerScripts()
{
    new OnLoginPlayerScripts();
    new ScalePetOnLogin();
}

