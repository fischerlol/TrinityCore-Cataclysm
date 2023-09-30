#ifndef TALENT_FUNCTIONS_H
#define TALENT_FUNCTIONS_H

#include "Define.h"
#include "Player.h"
#include "Pet.h"
#include "Log.h"
#include "WorldSession.h"

enum Spells
{
    Amani_War_Bear = 43688,
    Artisan_Riding = 34091,
    Cold_Weather_Flying = 54197,
    Teach_Learn_Talent_Specialization_Switches = 63680,
    Learn_a_Second_Talent_Specialization = 63624
};

enum templateSpells
{
    PLATE_MAIL = 750,
    MAIL = 8737,
    PLATE_SPECIALIZATION_WARRIOR = 87509,
    PLATE_SPECIALIZATION_PALADIN = 87511,
    PLATE_SPECIALIZATION_DEATH_KNIGHT = 87510,
    MAIL_SPECIALIZATION_HUNTER = 87506,
    MAIL_SPECIALIZATION_SHAMAN = 87507,
    LEATHER_SPECIALIZATION_ROGUE = 87504,
    LEATHER_SPECIALIZATION_DRUID = 87505
};

enum mastery
{
    MASTERY_WARRIOR = 87500,
    MASTERY_SHAMAN = 87497,
    MASTERY_MAGE = 86467,
    MASTERY_DRUID = 87491,
    MASTERY_DEATH_KNIGHT = 87492,
    MASTERY_HUNTER = 87493,
    MASTERY_PALADIN = 87494,
    MASTERY_PRIEST = 87495,
    MASTERY_ROGUE = 87496,
    MASTERY_WARLOCK = 87498,
};

enum ItemsOnLogin
{
    DUST_OF_DISAPPEARANCE = 64670,
    DUST_OF_DISAPPEARANCE_COUNT = 100,
    VANISHING_POWDER = 63388,
    VANISHING_POWDER_COUNT = 100,
    THIRTY_SIX_SLOT_BAG = 23162,
    THIRTY_SIX_SLOT_BAG_COUNT = 4,
};

enum ShamanGlyphs
{
    // Prime
    GLYPH_EARTH_SHIELD = 63925,
    GLYPH_OF_EARTHLIVING_WEAPON = 55541,
    GLYPH_OF_FERAL_SPIRIT = 63903,
    GLYPH_OF_FIRE_ELEMENTAL_TOTEM = 55542,
    GLYPH_OF_FLAME_SHOCK = 55545,
    GLYPH_OF_FLAMETONGUE_WEAPON = 55546,
    GLYPH_OF_LAVA_BURST = 55539,
    GLYPH_OF_LAVA_LASH = 55560,
    GLYPH_OF_LIGHTNING_BOLT = 55554,
    GLYPH_OF_RIPTIDE = 63904,
    GLYPH_OF_SHOCKING = 55540,
    GLYPH_OF_STORMSTRIKE = 55559,
    GLYPH_OF_UNLEASHED_LIGHTNING = 101051,
    GLYPH_OF_WATER_SHIELD = 55535,
    GLYPH_OF_WINDFURY_WEAPON = 55562,

    // Major
    GLYPH_OF_CHAIN_HEAL = 55537,
    GLYPH_OF_CHAIN_LIGHTNING = 55538,
    GLYPH_OF_ELEMENTAL_MASTERY = 123,
    GLYPH_OF_FIRE_NOVA = 55544,
    GLYPH_OF_FROST_SHOCK = 55547,
    GLYPH_OF_GHOST_WOLF = 59287,
    GLYPH_OF_GROUNDING_TOTEM = 55558,
    GLYPH_OF_HEALING_STREAM_TOTEM = 55548,
    GLYPH_OF_HEALING_WAVE = 55551,
    GLYPH_OF_HEX = 63927,
    GLYPH_OF_LIGHTNING_SHIELD = 55553,
    GLYPH_OF_SHAMANISTIC_RAGE = 63926,
    GLYPH_OF_STONECLAW_TOTEM = 63929,
    GLYPH_OF_THUNDER = 63902,
    GLYPH_OF_TOTEMIC_RECALL = 55552,

    // Minor
    GLYPH_OF_ARTIC_WOLF = 58261,
    GLYPH_OF_ASTRAL_RECALL = 58260,
    GLYPH_OF_RENEWED_LIFE = 58263,
    GLYPH_OF_THUNDERSTORM = 62133,
    GLYPH_OF_WATER_BREATHING = 58264,
    GLYPH_OF_WATER_WALKING = 58265
};

enum RogueGlyphs
{
    // Prime


    // Major

    // Minor
};

enum HunterGlyphs
{
    // Prime
    GLYPH_OF_AIMED_SHOT = 56869,
    GLYPH_OF_ARCANE_SHOT = 56870,
    GLYPH_OF_CHIMERA_SHOT = 63741,
    GLYPH_OF_DAZZLED_PREY = 56881,
    GLYPH_OF_EXPLOSIVE_SHOT = 63854,
    GLYPH_OF_KILL_COMMAND = 56887,
    GLYPH_OF_KILL_SHOT = 63855,
    GLYPH_OF_RAPID_FIRE = 56883,
    GLYPH_OF_SERPENT_STING = 56884,
    GLYPH_OF_STEADY_SHOT = 56886,

    // Major
    GLYPH_OF_BEASTIAL_WRATH = 56874,
    GLYPH_OF_CONCUSSIVE_SHOT = 56873,
    GLYPH_OF_DETERRENCE = 56875,
    GLYPH_OF_DISENGAGE = 56876,
    GLYPH_OF_FREEZING_TRAP = 56877,
    GLYPH_OF_ICE_TRAP = 56878,
    GLYPH_OF_IMMOLATION_TRAP = 56880,
    GLYPH_OF_MASTERS_CALL = 63856,
    GLYPH_OF_MENDING = 56872,
    GLYPH_OF_MISDIRECTION = 56879,
    GLYPH_OF_RAPTOR_STRIKE = 63858,
    GLYPH_OF_SCATTER_SHOT = 63857,
    GLYPH_OF_SILENCING_SHOT = 56882,
    GLYPH_OF_SNAKE_TRAP = 56885,
    GLYPH_OF_TRAP_LAUNCHER = 56871,
    GLYPH_OF_WYVERN_STING = 56889,

    // Minor
    GLYPH_OF_ASPECT_OF_THE_PACK = 58232,
    GLYPH_OF_FEIGN_DEATH = 58229,
    GLYPH_OF_LESSER_PROPORTIONS = 58188,
    GLYPH_OF_REVIVE_PET = 58186,
    GLYPH_OF_SCARE_BEAST = 58234
};

enum WarriorGlyphs
{
    // Prime

    // Major

    // Minor
};

void RemoveStarterPets(Player* player)
{
    if (player->getClass() != CLASS_HUNTER)
        return;

    Pet* pet = player->GetPet();
    if (!pet)
        return;

    // Define your list of "starter pet" entry IDs.
    std::set<uint32> starterPetIDs = { 42717 /* add your starter pet IDs here */ };

    // Check if the pet's entry ID is in the list.
    if (starterPetIDs.find(pet->GetEntry()) != starterPetIDs.end())
    {
        // Remove the pet. Note: The number '1' means "abandon".
        player->RemovePet(pet, PET_SAVE_AS_DELETED, true);
        TC_LOG_INFO("server.worldserver", "Removed starter pet from player: %s", player->GetName().c_str());
    }
}

void CreatePet(Player* player, Creature* creature, uint32 entry)
{
    if (player->getClass() != CLASS_HUNTER)
        return;

    RemoveStarterPets(player);

    if (player->GetPet())
        return;

    Creature* creatureTarget = creature->SummonCreature(entry, player->GetPositionX(), player->GetPositionY() + 2, player->GetPositionZ(), player->GetOrientation(), TEMPSUMMON_CORPSE_TIMED_DESPAWN, 500);
    if (!creatureTarget)
        return;

    Pet* pet = player->CreateTamedPetFrom(creatureTarget, 0);
    if (!pet)
        return;

    // kill original creature
    creatureTarget->setDeathState(JUST_DIED);
    creatureTarget->RemoveCorpse();
    creatureTarget->SetHealth(0);

    // prepare visual effect for levelup
    pet->SetUInt32Value(UNIT_FIELD_LEVEL, player->getLevel() - 1);
    pet->GetMap()->AddToMap(pet->ToCreature());

    // visual effect for levelup
    pet->SetUInt32Value(UNIT_FIELD_LEVEL, player->getLevel());

    if (!pet->InitStatsForLevel(player->getLevel()))
        TC_LOG_INFO("server.worldserver", "Pet Create fail: No init stats for pet with entry %u", entry);

    pet->UpdateAllStats();
    player->SetMinion(pet, true);
    pet->SetObjectScale(1);
    TC_LOG_INFO("server.worldserver", "Custom scale set for pet: %f", pet->GetObjectScale());  // Log scale for debugging
    pet->SavePetToDB(PET_SAVE_CURRENT_STATE);
    pet->InitTalentForLevel();
    player->PetSpellInitialize();
}

void AddDustOfDisappearance(Player* player)
{
    uint32 currentItemCount = player->GetItemCount(DUST_OF_DISAPPEARANCE, true);
    uint32 itemsToAdd = 0;

    if (currentItemCount < DUST_OF_DISAPPEARANCE_COUNT)
    {
        itemsToAdd = DUST_OF_DISAPPEARANCE_COUNT - currentItemCount;
        player->AddItem(DUST_OF_DISAPPEARANCE, itemsToAdd);
    }
    return; 
}

void AddVanishingPowder(Player* player)
{
    uint32 currentItemCount = player->GetItemCount(VANISHING_POWDER, true);
    uint32 itemsToAdd = 0;

    if (currentItemCount < VANISHING_POWDER_COUNT)
    {
        itemsToAdd = VANISHING_POWDER_COUNT - currentItemCount;
        player->AddItem(VANISHING_POWDER, itemsToAdd);
    }
    return;
}


void AddBags(Player* player)
{
    int currentBagCount = 0;

    for (int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
    {
        if (Item* bag = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
        {
            ++currentBagCount;
        }
    }

    int bagsToAdd = THIRTY_SIX_SLOT_BAG_COUNT - currentBagCount;

    if (bagsToAdd > 0)
    {
        player->AddItem(THIRTY_SIX_SLOT_BAG, bagsToAdd);
    }
}

static void LearnGlyphs(Player* player)
{
    switch (player->getClass())
    {
    case CLASS_WARRIOR:
        AddDustOfDisappearance(player);
        player->LearnSpell(MASTERY_WARRIOR, false);
        player->LearnSpell(PLATE_SPECIALIZATION_WARRIOR, false);
        break;
    case CLASS_PRIEST:
        AddVanishingPowder(player);
        player->LearnSpell(MASTERY_PRIEST, false);
        break;
    case CLASS_PALADIN:
        AddDustOfDisappearance(player);
        player->LearnSpell(MASTERY_PALADIN, false);
        player->LearnSpell(PLATE_SPECIALIZATION_PALADIN, false);
        break;
    case CLASS_ROGUE:
        AddVanishingPowder(player);
        player->LearnSpell(MASTERY_ROGUE, false);
        player->LearnSpell(LEATHER_SPECIALIZATION_ROGUE, false);
        break;
    case CLASS_DEATH_KNIGHT:
        AddDustOfDisappearance(player);
        player->LearnSpell(MASTERY_DEATH_KNIGHT, false);
        break;
    case CLASS_MAGE:
        AddDustOfDisappearance(player);
        player->LearnSpell(MASTERY_MAGE, false);
        break;
    case CLASS_SHAMAN:
        AddVanishingPowder(player);
        player->LearnSpell(MASTERY_SHAMAN, false);
        player->LearnSpell(MAIL_SPECIALIZATION_SHAMAN, false);
        player->LearnSpell(GLYPH_OF_CHAIN_HEAL, false);
        player->LearnSpell(GLYPH_OF_CHAIN_LIGHTNING, false);
        player->LearnSpell(GLYPH_OF_ELEMENTAL_MASTERY, false);
        player->LearnSpell(GLYPH_OF_FIRE_NOVA, false);
        player->LearnSpell(GLYPH_OF_FROST_SHOCK, false);
        player->LearnSpell(GLYPH_OF_GHOST_WOLF, false);
        player->LearnSpell(GLYPH_OF_GROUNDING_TOTEM, false);
        player->LearnSpell(GLYPH_OF_HEALING_STREAM_TOTEM, false);
        player->LearnSpell(GLYPH_OF_HEALING_WAVE, false);
        player->LearnSpell(GLYPH_OF_HEX, false);
        player->LearnSpell(GLYPH_OF_LIGHTNING_SHIELD, false);
        player->LearnSpell(GLYPH_OF_SHAMANISTIC_RAGE, false);
        player->LearnSpell(GLYPH_OF_STONECLAW_TOTEM, false);
        player->LearnSpell(GLYPH_OF_THUNDER, false);
        player->LearnSpell(GLYPH_OF_TOTEMIC_RECALL, false);
        player->LearnSpell(GLYPH_OF_ARTIC_WOLF, false);
        player->LearnSpell(GLYPH_OF_ASTRAL_RECALL, false);
        player->LearnSpell(GLYPH_OF_RENEWED_LIFE, false);
        player->LearnSpell(GLYPH_OF_THUNDERSTORM, false);
        player->LearnSpell(GLYPH_OF_WATER_BREATHING, false);
        player->LearnSpell(GLYPH_OF_WATER_WALKING, false);
        player->LearnSpell(GLYPH_EARTH_SHIELD, false);
        player->LearnSpell(GLYPH_OF_EARTHLIVING_WEAPON, false);
        player->LearnSpell(GLYPH_OF_FERAL_SPIRIT, false);
        player->LearnSpell(GLYPH_OF_FIRE_ELEMENTAL_TOTEM, false);
        player->LearnSpell(GLYPH_OF_FLAME_SHOCK, false);
        player->LearnSpell(GLYPH_OF_FLAMETONGUE_WEAPON, false);
        player->LearnSpell(GLYPH_OF_LAVA_BURST, false);
        player->LearnSpell(GLYPH_OF_LAVA_LASH, false);
        player->LearnSpell(GLYPH_OF_LIGHTNING_BOLT, false);
        player->LearnSpell(GLYPH_OF_RIPTIDE, false);
        player->LearnSpell(GLYPH_OF_SHOCKING, false);
        player->LearnSpell(GLYPH_OF_STORMSTRIKE, false);
        player->LearnSpell(GLYPH_OF_UNLEASHED_LIGHTNING, false);
        player->LearnSpell(GLYPH_OF_WATER_SHIELD, false);
        player->LearnSpell(GLYPH_OF_WINDFURY_WEAPON, false);
        break;
    case CLASS_HUNTER:
        AddDustOfDisappearance(player);
        player->LearnSpell(MASTERY_HUNTER, false);
        player->LearnSpell(MAIL_SPECIALIZATION_HUNTER, false);
        player->LearnSpell(GLYPH_OF_AIMED_SHOT, false);
        player->LearnSpell(GLYPH_OF_ARCANE_SHOT, false);
        player->LearnSpell(GLYPH_OF_CHIMERA_SHOT, false);
        player->LearnSpell(GLYPH_OF_DAZZLED_PREY, false);
        player->LearnSpell(GLYPH_OF_EXPLOSIVE_SHOT, false);
        player->LearnSpell(GLYPH_OF_KILL_COMMAND, false);
        player->LearnSpell(GLYPH_OF_KILL_SHOT, false);
        player->LearnSpell(GLYPH_OF_RAPID_FIRE, false);
        player->LearnSpell(GLYPH_OF_SERPENT_STING, false);
        player->LearnSpell(GLYPH_OF_STEADY_SHOT, false);
        player->LearnSpell(GLYPH_OF_BEASTIAL_WRATH, false);
        player->LearnSpell(GLYPH_OF_CONCUSSIVE_SHOT, false);
        player->LearnSpell(GLYPH_OF_DETERRENCE, false);
        player->LearnSpell(GLYPH_OF_DISENGAGE, false);
        player->LearnSpell(GLYPH_OF_FREEZING_TRAP, false);
        player->LearnSpell(GLYPH_OF_ICE_TRAP, false);
        player->LearnSpell(GLYPH_OF_IMMOLATION_TRAP, false);
        player->LearnSpell(GLYPH_OF_MASTERS_CALL, false);
        player->LearnSpell(GLYPH_OF_MENDING, false);
        player->LearnSpell(GLYPH_OF_MISDIRECTION, false);
        player->LearnSpell(GLYPH_OF_RAPTOR_STRIKE, false);
        player->LearnSpell(GLYPH_OF_SCATTER_SHOT, false);
        player->LearnSpell(GLYPH_OF_SILENCING_SHOT, false);
        player->LearnSpell(GLYPH_OF_SNAKE_TRAP, false);
        player->LearnSpell(GLYPH_OF_TRAP_LAUNCHER, false);
        player->LearnSpell(GLYPH_OF_WYVERN_STING, false);
        player->LearnSpell(GLYPH_OF_ASPECT_OF_THE_PACK, false);
        player->LearnSpell(GLYPH_OF_FEIGN_DEATH, false);
        player->LearnSpell(GLYPH_OF_LESSER_PROPORTIONS, false);
        player->LearnSpell(GLYPH_OF_REVIVE_PET, false);
        player->LearnSpell(GLYPH_OF_SCARE_BEAST, false);
        break;
    case CLASS_DRUID:
        AddVanishingPowder(player);
        player->LearnSpell(MASTERY_DRUID, false);
        break;
    case CLASS_WARLOCK:
        AddVanishingPowder(player);
        player->LearnSpell(MASTERY_WARLOCK, false);
        break;
    default:
        break;
    }

    AddBags(player);
}

struct TalentTemplate
{
    std::string    playerClass;
    std::string    playerSpec;
    uint32         talentId;
    uint32         treeId;
};

struct TalentTreeId
{
    std::string    playerSpec;
    uint32         treeId;
};

struct GlyphTemplate
{
    std::string    playerClass;
    std::string    playerSpec;
    uint8          slot;
    uint32         glyph;
};

struct HumanGearTemplate
{
    std::string    playerClass;
    std::string    playerSpec;
    uint8          pos;
    uint32         itemEntry;
    uint32         enchant;
    uint32         socket1;
    uint32         socket2;
    uint32         socket3;
    uint32         bonusEnchant;
    uint32         prismaticEnchant;
    uint32         reforgeId;
};

struct AllianceGearTemplate
{
    std::string    playerClass;
    std::string    playerSpec;
    uint8          pos;
    uint32         itemEntry;
    uint32         enchant;
    uint32         socket1;
    uint32         socket2;
    uint32         socket3;
    uint32         bonusEnchant;
    uint32         prismaticEnchant;
    uint32         reforgeId;
};

struct HordeGearTemplate
{
    std::string    playerClass;
    std::string    playerSpec;
    uint8          pos;
    uint32         itemEntry;
    uint32         enchant;
    uint32         socket1;
    uint32         socket2;
    uint32         socket3;
    uint32         bonusEnchant;
    uint32         prismaticEnchant;
    uint32         reforgeId;
};

typedef std::vector<HumanGearTemplate*> HumanGearContainerPvE;
typedef std::vector<AllianceGearTemplate*> AllianceGearContainerPvE;
typedef std::vector<HordeGearTemplate*> HordeGearContainerPvE;
typedef std::vector<HumanGearTemplate*> HumanGearContainerPvP;
typedef std::vector<AllianceGearTemplate*> AllianceGearContainerPvP;
typedef std::vector<HordeGearTemplate*> HordeGearContainerPvP;

typedef std::vector<TalentTemplate*> TalentContainerPvE;
typedef std::vector<TalentTemplate*> TalentContainerPvP;
typedef std::vector<GlyphTemplate*> GlyphContainerPvE;
typedef std::vector<GlyphTemplate*> GlyphContainerPvP;
typedef std::vector<TalentTreeId*> TalentTreeIdContainer;

class sTemplateNPC
{
public:
    static sTemplateNPC* instance()
    {
        static sTemplateNPC* instance = new sTemplateNPC();
        return instance;
    }
    void LoadTalentsContainerPvE();
    void LoadTalentsContainerPvP();
    void LoadTalentTreeId();
    void LoadGlyphsContainerPvE();
    void LoadGlyphsContainerPvP();

    void LoadHumanGearContainerPvE();
    void LoadAllianceGearContainerPvE();
    void LoadHordeGearContainerPvE();
    void LoadHumanGearContainerPvP();
    void LoadAllianceGearContainerPvP();
    void LoadHordeGearContainerPvP();

    void ApplyGlyph(Player* player, uint8 slot, uint32 glyphID);
    void RemoveAllGlyphs(Player* player);
    void ApplyBonus(Player* player, Item* item, EnchantmentSlot slot, uint32 bonusEntry);

    bool OverwriteTemplate(Player* /*player*/, std::string& /*playerSpecStr*/);
    void ExtractGearTemplateToDBPvE(Player* /*player*/, std::string& /*playerSpecStr*/);
	void ExtractGearTemplateToDBHumanPvP(Player* /*player*/, std::string& /*playerSpecStr*/);
    void ExtractGearTemplateToDBAlliancePvP(Player* /*player*/, std::string& /*playerSpecStr*/);
    void ExtractGearTemplateToDBHordePvP(Player* /*player*/, std::string& /*playerSpecStr*/);
    void ExtractTalentTemplateToDBPvE(Player* /*player*/, std::string& /*playerSpecStr*/);
    void ExtractTalentTemplateToDBPvP(Player* /*player*/, std::string& /*playerSpecStr*/);
    void ExtractTalentTreeId(Player* /*player*/, std::string& /*playerSpecStr*/);
    void ExtractGlyphsTemplateToDBPvE(Player* /*player*/, std::string& /*playerSpecStr*/);
    void ExtractGlyphsTemplateToDBPvP(Player* /*player*/, std::string& /*playerSpecStr*/);
    bool CanEquipTemplatePvE(Player* /*player*/, std::string& /*playerSpecStr*/);
    bool CanEquipTemplatePvP(Player* /*player*/, std::string& /*playerSpecStr*/);

    std::string GetClassString(Player* /*player*/);
    std::string sTalentsSpec;

    void LearnTemplateTalentsPvE(Player* /*player*/);
    void LearnTemplateTalentsPvP(Player* /*player*/);
    void LearnTemplateGlyphsPvE(Player* /*player*/);
    void EquipTemplateGearPvE(Player* /*player*/);
    void LearnTemplateGlyphsPvP(Player* /*player*/);
    void EquipTemplateGearPvP(Player* /*player*/);

    void LearnPlateMailSpells(Player* /*player*/);

    GlyphContainerPvE m_GlyphContainerPvE;
    GlyphContainerPvP m_GlyphContainerPvP;
    TalentContainerPvE m_TalentContainerPvE;
    TalentContainerPvP m_TalentContainerPvP;
    TalentTreeIdContainer m_TalentTreeIdContainer;

    HumanGearContainerPvE m_HumanGearContainerPvE;
    AllianceGearContainerPvE m_AllianceGearContainerPvE;
    HordeGearContainerPvE m_HordeGearContainerPvE;
    HumanGearContainerPvP m_HumanGearContainerPvP;
    AllianceGearContainerPvP m_AllianceGearContainerPvP;
    HordeGearContainerPvP m_HordeGearContainerPvP;
};
#define sTemplateNpcMgr sTemplateNPC::instance()
#endif
