#ifndef TALENT_FUNCTIONS_H
#define TALENT_FUNCTIONS_H

#include "Define.h"
#include "Player.h"

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
    PLATE_SPECIALIZATION = 87509,
    LEATHER_SPECIALIZATION = 87504,
    MAIL_SPECIALIZATION = 87507,
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
    VANISHING_POWDER = 64670,
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
        player->LearnSpell(MASTERY_WARRIOR, false);
        player->LearnSpell(PLATE_SPECIALIZATION, false);
        break;
    case CLASS_PRIEST:
        break;
    case CLASS_PALADIN:
        break;
    case CLASS_ROGUE:
        player->LearnSpell(MASTERY_ROGUE, false);
        player->LearnSpell(LEATHER_SPECIALIZATION, false);
        break;
    case CLASS_DEATH_KNIGHT:
        break;
    case CLASS_MAGE:
        break;
    case CLASS_SHAMAN:
        player->LearnSpell(MAIL, false);
        player->LearnSpell(MASTERY_SHAMAN, false);
        player->LearnSpell(MAIL_SPECIALIZATION, false);
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
        break;
    case CLASS_DRUID:
        break;
    case CLASS_WARLOCK:
        break;
    default:
        break;
    }

    AddBags(player);
    player->AddItem(VANISHING_POWDER, VANISHING_POWDER_COUNT);

}

struct TalentTemplate
{
    std::string    playerClass;
    std::string    playerSpec;
    uint32         talentId;
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
};

typedef std::vector<HumanGearTemplate*> HumanGearContainer;
typedef std::vector<AllianceGearTemplate*> AllianceGearContainer;
typedef std::vector<HordeGearTemplate*> HordeGearContainer;

typedef std::vector<TalentTemplate*> TalentContainer;
typedef std::vector<GlyphTemplate*> GlyphContainer;

class sTemplateNPC
{
public:
    static sTemplateNPC* instance()
    {
        static sTemplateNPC* instance = new sTemplateNPC();
        return instance;
    }
    void LoadTalentsContainer();
    void LoadGlyphsContainer();

    void LoadHumanGearContainer();
    void LoadAllianceGearContainer();
    void LoadHordeGearContainer();

    void ApplyGlyph(Player* player, uint8 slot, uint32 glyphID);
    void RemoveAllGlyphs(Player* player);
    void ApplyBonus(Player* player, Item* item, EnchantmentSlot slot, uint32 bonusEntry);

    bool OverwriteTemplate(Player* /*player*/, std::string& /*playerSpecStr*/);
    void ExtractGearTemplateToDB(Player* /*player*/, std::string& /*playerSpecStr*/);
    void ExtractTalentTemplateToDB(Player* /*player*/, std::string& /*playerSpecStr*/);
    void ExtractGlyphsTemplateToDB(Player* /*player*/, std::string& /*playerSpecStr*/);
    bool CanEquipTemplate(Player* /*player*/, std::string& /*playerSpecStr*/);

    std::string GetClassString(Player* /*player*/);
    std::string sTalentsSpec;

    void LearnTemplateTalents(Player* /*player*/);
    void LearnTemplateGlyphs(Player* /*player*/);
    void EquipTemplateGear(Player* /*player*/);

    void LearnPlateMailSpells(Player* /*player*/);

    GlyphContainer m_GlyphContainer;
    TalentContainer m_TalentContainer;

    HumanGearContainer m_HumanGearContainer;
    AllianceGearContainer m_AllianceGearContainer;
    HordeGearContainer m_HordeGearContainer;
};
#define sTemplateNpcMgr sTemplateNPC::instance()
#endif
