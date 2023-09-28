#include "ScriptMgr.h"
#include "Player.h"
#include "SpellAuras.h"
#include "SpellAuraEffects.h"
#include "Log.h"
#include "Unit.h"
#include "Object.h"

enum Camouflage
{
    SPELL_HUNTER_CAMOUFLAGE_DURATION = 51755,
    SPELL_HUNTER_CAMOUFLAGE_PERIODIC = 80326,
    SPELL_HUNTER_CAMOUFLAGE_PERIODIC_TRIGGERED = 80325
};

class CamouflageSpellCheck : public UnitScript
{
public:
    CamouflageSpellCheck() : UnitScript("CamouflageSpellCheck") {}

    void OnDamage(Unit* attacker, Unit* victim, uint32& damage) override
    {
        if (attacker->HasAura(SPELL_HUNTER_CAMOUFLAGE_DURATION))
        {
            attacker->RemoveAura(SPELL_HUNTER_CAMOUFLAGE_DURATION);
            attacker->RemoveAura(SPELL_HUNTER_CAMOUFLAGE_PERIODIC);
            attacker->RemoveAura(SPELL_HUNTER_CAMOUFLAGE_PERIODIC_TRIGGERED);
        }
    }
};


void AddSC_CamouflageSpellCheck()
{
    new CamouflageSpellCheck();
}
