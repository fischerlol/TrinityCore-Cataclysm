#include "ScriptMgr.h"
#include "Player.h"
#include "SpellInfo.h"
#include "SpellScript.h"
#include "Log.h"
#include "Spell.h"
#include "Pet.h"
#include "World.h"

class ScalePetOnLogin : public SpellScript
{
    void HandleAfterCast()
    {
        if (Player* player = GetCaster()->ToPlayer())
        {
            TC_LOG_INFO("server.worldserver", "Player %s has successfully cast a Call Pet spell", player->GetName().c_str());
            Pet* pet = player->GetPet();
            if (pet)
            {
                pet->SetObjectScale(1.0f);
                TC_LOG_INFO("server.worldserver", "Set scale of pet for player %s to 1.0", player->GetName().c_str());
            }
        }
    }

    void Register() override
    {
        AfterCast.Register(&ScalePetOnLogin::HandleAfterCast);
    }
};


void AddSC_ScalePetOnLogin()
{
    RegisterSpellScript(ScalePetOnLogin);
}

