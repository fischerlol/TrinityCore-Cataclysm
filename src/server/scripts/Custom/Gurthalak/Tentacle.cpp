#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "WorldSession.h"
#include "Player.h"
#include "Unit.h"
#include "TemporarySummon.h"
#include "SpellAuraEffects.h"

enum TentacleStuff
{
    EVENT_MIND_FLAY = 1,
    SPELL_MIND_FLAY = 52586,
    BASE_DAMAGE_LFR = 9881,
    BASE_DAMAGE_NH = 11155,
    BASE_DAMAGE_HC = 12429
};

class npc_tentacle_of_the_old_ones : public CreatureScript
{
public:
    npc_tentacle_of_the_old_ones() : CreatureScript("npc_tentacle_of_the_old_ones") {}

    struct npc_tentacle_of_the_old_onesAI : public ScriptedAI
    {
        npc_tentacle_of_the_old_onesAI(Creature* creature) : ScriptedAI(creature) {}

        void IsSummonedBy(Unit* summoner)
        {
            DoCast(me, 89962, true); // not sniff verified but include all scalings which we need
            DoCast(me, 61783, true); // not sniff verified but include all scalings which we need

            //Because something removes this Aura after this Hook..
            me->CastSpell(70416, true);
            me->CastSpell(109905, true);
            //me->CastWithDelay(100, me, 70416, true); // Sniff verified
            //me->CastWithDelay(100, me, 109905, true); // Sniff verified
        }

        void Reset()
        {
            me->StopMoving();
            events.ScheduleEvent(EVENT_MIND_FLAY, 100);
        }

        void UpdateAI(uint32 const diff) override
        {
            me->StopMoving();
            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_MIND_FLAY:
                    if (Unit* summoner = me->ToTempSummon()->GetSummoner())
                    {
                        if (Unit* target = summoner->getAttackerForHelper())
                        {
                            int32 baseDamage = BASE_DAMAGE_LFR;
                            if (me->GetEntry() != 58077)
                                baseDamage = me->GetEntry() == 57220 ? BASE_DAMAGE_NH : BASE_DAMAGE_HC;

                            if (AuraEffect* aurEff = summoner->GetAuraEffect(77515, EFFECT_0))
                                AddPct(baseDamage, aurEff->GetAmount());

                            CastSpellExtraArgs args;
                            args.AddSpellBP0(baseDamage); // where baseDamage is the custom damage for your Mind Flay spell

                            me->CastSpell(target, SPELL_MIND_FLAY, args);
                            events.ScheduleEvent(EVENT_MIND_FLAY, 3000);
                        }
                        else
                            events.ScheduleEvent(EVENT_MIND_FLAY, 100);
                    }
                    break;
                default:
                    break;
                }
            }
        }

    private:
        EventMap events;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_tentacle_of_the_old_onesAI(creature);
    }
};
void AddSC_npc_tentacle_of_the_old_ones()
{
    new npc_tentacle_of_the_old_ones();
}
