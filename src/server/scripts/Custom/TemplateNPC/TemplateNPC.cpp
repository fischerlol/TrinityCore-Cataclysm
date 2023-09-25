#include "TemplateNPC.h"
#include "Item.h"
#include "DBCStores.h"
#include "Log.h"
#include "DatabaseEnv.h"
#include "CharacterDatabase.h"
#include "Player.h"
#include "WorldSession.h"
#include "ScriptedGossip.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "GossipDef.h"
#include "Creature.h"
#include "ObjectMgr.h"
#include "Chat.h"

enum GossipId
{
    GOSSIP_MAIN_MENU = 0,
    GOSSIP_PVP_GEAR = 100,
    GOSSIP_PVE_GEAR = 200,
    GOSSIP_EXTRACT_GEAR = 300,
    GOSSIP_HUNTER_PET = 400
};

void sTemplateNPC::LearnPlateMailSpells(Player* player)
{
    switch (player->getClass())
    {
    case CLASS_WARRIOR:
    case CLASS_PALADIN:
    case CLASS_DEATH_KNIGHT:
        player->LearnSpell(PLATE_MAIL, true);
        break;
    case CLASS_SHAMAN:
    case CLASS_HUNTER:
        player->LearnSpell(MAIL, true);
        break;
    default:
        break;
    }
}

void sTemplateNPC::ApplyBonus(Player* player, Item* item, EnchantmentSlot slot, uint32 bonusEntry)
{
    if (!item)
        return;

    if (!bonusEntry || bonusEntry == 0)
        return;

    player->ApplyEnchantment(item, slot, false);
    item->SetEnchantment(slot, bonusEntry, 0, 0);
    player->ApplyEnchantment(item, slot, true);
}

void sTemplateNPC::ApplyGlyph(Player* player, uint8 slot, uint32 glyphID)
{
    if (GlyphPropertiesEntry const* gp = sGlyphPropertiesStore.LookupEntry(glyphID))
    {
        if (uint32 oldGlyph = player->GetGlyph(0, slot))
        {
            player->RemoveAurasDueToSpell(sGlyphPropertiesStore.LookupEntry(oldGlyph)->SpellID);
            player->SetGlyph(slot, 0);
        }
        player->CastSpell(player, gp->SpellID, true);
        player->SetGlyph(slot, glyphID);
    }
}

void sTemplateNPC::RemoveAllGlyphs(Player* player)
{
    for (uint8 i = 0; i < MAX_GLYPH_SLOT_INDEX; ++i)
    {
        if (uint32 glyph = player->GetGlyph(0, i))
        {
            if (GlyphPropertiesEntry const* gp = sGlyphPropertiesStore.LookupEntry(glyph))
            {
                if (GlyphSlotEntry const* gs = sGlyphSlotStore.LookupEntry(player->GetGlyphSlot(i)))
                {
                    player->RemoveAurasDueToSpell(sGlyphPropertiesStore.LookupEntry(glyph)->SpellID);
                    player->SetGlyph(i, 0);
                    player->SendTalentsInfoData(false); // this is somewhat an in-game glyph realtime update (apply/remove)
                }
            }
        }
    }
}

void sTemplateNPC::LearnTemplateTalents(Player* player)
{
    for (TalentContainer::const_iterator itr = m_TalentContainer.begin(); itr != m_TalentContainer.end(); ++itr)
    {
        if ((*itr)->playerClass == GetClassString(player).c_str() && (*itr)->playerSpec == sTalentsSpec)
        {
            player->LearnSpell((*itr)->talentId, false);
            player->AddTalent((*itr)->talentId, player->GetActiveSpec(), true);
        }
    }
    player->SetFreeTalentPoints(0);
    player->SendTalentsInfoData(false);
}

void sTemplateNPC::LearnTemplateGlyphs(Player* player)
{
    for (GlyphContainer::const_iterator itr = m_GlyphContainer.begin(); itr != m_GlyphContainer.end(); ++itr)
    {
        if ((*itr)->playerClass == GetClassString(player).c_str() && (*itr)->playerSpec == sTalentsSpec)
            ApplyGlyph(player, (*itr)->slot, (*itr)->glyph);
    }
    player->SendTalentsInfoData(false);
}

void sTemplateNPC::EquipTemplateGear(Player* player)
{
    if (player->getRace() == RACE_HUMAN)
    {
        for (HumanGearContainer::const_iterator itr = m_HumanGearContainer.begin(); itr != m_HumanGearContainer.end(); ++itr)
        {
            if ((*itr)->playerClass == GetClassString(player).c_str() && (*itr)->playerSpec == sTalentsSpec)
            {
                player->EquipNewItem((*itr)->pos, (*itr)->itemEntry, true); // Equip the item and apply enchants and gems
                ApplyBonus(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, (*itr)->pos), PERM_ENCHANTMENT_SLOT, (*itr)->enchant);
                ApplyBonus(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, (*itr)->pos), SOCK_ENCHANTMENT_SLOT, (*itr)->socket1);
                ApplyBonus(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, (*itr)->pos), SOCK_ENCHANTMENT_SLOT_2, (*itr)->socket2);
                ApplyBonus(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, (*itr)->pos), SOCK_ENCHANTMENT_SLOT_3, (*itr)->socket3);
                ApplyBonus(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, (*itr)->pos), BONUS_ENCHANTMENT_SLOT, (*itr)->bonusEnchant);
                ApplyBonus(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, (*itr)->pos), PRISMATIC_ENCHANTMENT_SLOT, (*itr)->prismaticEnchant);
            }
        }
    }
    else if (player->GetTeam() == ALLIANCE && player->getRace() != RACE_HUMAN)
    {
        for (AllianceGearContainer::const_iterator itr = m_AllianceGearContainer.begin(); itr != m_AllianceGearContainer.end(); ++itr)
        {
            if ((*itr)->playerClass == GetClassString(player).c_str() && (*itr)->playerSpec == sTalentsSpec)
            {
                player->EquipNewItem((*itr)->pos, (*itr)->itemEntry, true); // Equip the item and apply enchants and gems
                ApplyBonus(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, (*itr)->pos), PERM_ENCHANTMENT_SLOT, (*itr)->enchant);
                ApplyBonus(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, (*itr)->pos), SOCK_ENCHANTMENT_SLOT, (*itr)->socket1);
                ApplyBonus(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, (*itr)->pos), SOCK_ENCHANTMENT_SLOT_2, (*itr)->socket2);
                ApplyBonus(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, (*itr)->pos), SOCK_ENCHANTMENT_SLOT_3, (*itr)->socket3);
                ApplyBonus(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, (*itr)->pos), BONUS_ENCHANTMENT_SLOT, (*itr)->bonusEnchant);
                ApplyBonus(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, (*itr)->pos), PRISMATIC_ENCHANTMENT_SLOT, (*itr)->prismaticEnchant);
            }
        }
    }
    else if (player->GetTeam() == HORDE)
    {
        for (HordeGearContainer::const_iterator itr = m_HordeGearContainer.begin(); itr != m_HordeGearContainer.end(); ++itr)
        {
            if ((*itr)->playerClass == GetClassString(player).c_str() && (*itr)->playerSpec == sTalentsSpec)
            {
                player->EquipNewItem((*itr)->pos, (*itr)->itemEntry, true); // Equip the item and apply enchants and gems
                ApplyBonus(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, (*itr)->pos), PERM_ENCHANTMENT_SLOT, (*itr)->enchant);
                ApplyBonus(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, (*itr)->pos), SOCK_ENCHANTMENT_SLOT, (*itr)->socket1);
                ApplyBonus(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, (*itr)->pos), SOCK_ENCHANTMENT_SLOT_2, (*itr)->socket2);
                ApplyBonus(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, (*itr)->pos), SOCK_ENCHANTMENT_SLOT_3, (*itr)->socket3);
                ApplyBonus(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, (*itr)->pos), BONUS_ENCHANTMENT_SLOT, (*itr)->bonusEnchant);
                ApplyBonus(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, (*itr)->pos), PRISMATIC_ENCHANTMENT_SLOT, (*itr)->prismaticEnchant);
            }
        }
    }
}

void sTemplateNPC::LoadTalentsContainer()
{
    for (TalentContainer::const_iterator itr = m_TalentContainer.begin(); itr != m_TalentContainer.end(); ++itr)
        delete* itr;

    m_TalentContainer.clear();

    uint32 oldMSTime = getMSTime();
    uint32 count = 0;

    QueryResult result = CharacterDatabase.Query("SELECT playerClass, playerSpec, talentId FROM template_npc_talents;");

    if (!result)
    {
        TC_LOG_INFO("server.worldserver", ">>TEMPLATE NPC: Loaded 0 talent templates. DB table `template_npc_talents` is empty!");
        return;
    }

    do
    {
        Field* fields = result->Fetch();

        TalentTemplate* pTalent = new TalentTemplate;

        pTalent->playerClass = fields[0].GetString();
        pTalent->playerSpec = fields[1].GetString();
        pTalent->talentId = fields[2].GetUInt32();

        m_TalentContainer.push_back(pTalent);
        ++count;
    } while (result->NextRow());
    TC_LOG_INFO("server.worldserver", ">>TEMPLATE NPC: Loaded %u talent templates in %u ms.", count, GetMSTimeDiffToNow(oldMSTime));
}

void sTemplateNPC::LoadGlyphsContainer()
{
    for (GlyphContainer::const_iterator itr = m_GlyphContainer.begin(); itr != m_GlyphContainer.end(); ++itr)
        delete* itr;

    m_GlyphContainer.clear();

    QueryResult result = CharacterDatabase.Query("SELECT playerClass, playerSpec, slot, glyph FROM template_npc_glyphs;");

    uint32 oldMSTime = getMSTime();
    uint32 count = 0;

    if (!result)
    {
        TC_LOG_INFO("server.worldserver", ">>TEMPLATE NPC: Loaded 0 glyph templates. DB table `template_npc_glyphs` is empty!");
        return;
    }

    do
    {
        Field* fields = result->Fetch();

        GlyphTemplate* pGlyph = new GlyphTemplate;

        pGlyph->playerClass = fields[0].GetString();
        pGlyph->playerSpec = fields[1].GetString();
        pGlyph->slot = fields[2].GetUInt8();
        pGlyph->glyph = fields[3].GetUInt32();

        m_GlyphContainer.push_back(pGlyph);
        ++count;
    } while (result->NextRow());
    TC_LOG_INFO("server.worldserver", ">>TEMPLATE NPC: Loaded %u glyph templates in %u ms.", count, GetMSTimeDiffToNow(oldMSTime));
}

void sTemplateNPC::LoadHumanGearContainer()
{
    for (HumanGearContainer::const_iterator itr = m_HumanGearContainer.begin(); itr != m_HumanGearContainer.end(); ++itr)
        delete* itr;

    m_HumanGearContainer.clear();

    QueryResult result = CharacterDatabase.Query("SELECT playerClass, playerSpec, pos, itemEntry, enchant, socket1, socket2, socket3, bonusEnchant, prismaticEnchant FROM template_npc_human;");

    uint32 oldMSTime = getMSTime();
    uint32 count = 0;

    if (!result)
    {
        TC_LOG_INFO("server.worldserver", ">>TEMPLATE NPC: Loaded 0 'gear templates. DB table `template_npc_human` is empty!");
        return;
    }

    do
    {
        Field* fields = result->Fetch();

        HumanGearTemplate* pItem = new HumanGearTemplate;

        pItem->playerClass = fields[0].GetString();
        pItem->playerSpec = fields[1].GetString();
        pItem->pos = fields[2].GetUInt8();
        pItem->itemEntry = fields[3].GetUInt32();
        pItem->enchant = fields[4].GetUInt32();
        pItem->socket1 = fields[5].GetUInt32();
        pItem->socket2 = fields[6].GetUInt32();
        pItem->socket3 = fields[7].GetUInt32();
        pItem->bonusEnchant = fields[8].GetUInt32();
        pItem->prismaticEnchant = fields[9].GetUInt32();

        m_HumanGearContainer.push_back(pItem);
        ++count;
    } while (result->NextRow());
    TC_LOG_INFO("server.worldserver", ">>TEMPLATE NPC: Loaded %u gear templates for Humans in %u ms.", count, GetMSTimeDiffToNow(oldMSTime));
}

void sTemplateNPC::LoadAllianceGearContainer()
{
    for (AllianceGearContainer::const_iterator itr = m_AllianceGearContainer.begin(); itr != m_AllianceGearContainer.end(); ++itr)
        delete* itr;

    m_AllianceGearContainer.clear();

    QueryResult result = CharacterDatabase.Query("SELECT playerClass, playerSpec, pos, itemEntry, enchant, socket1, socket2, socket3, bonusEnchant, prismaticEnchant FROM template_npc_alliance;");

    uint32 oldMSTime = getMSTime();
    uint32 count = 0;

    if (!result)
    {
        TC_LOG_INFO("server.worldserver", ">>TEMPLATE NPC: Loaded 0 'gear templates. DB table `template_npc_alliance` is empty!");
        return;
    }

    do
    {
        Field* fields = result->Fetch();

        AllianceGearTemplate* pItem = new AllianceGearTemplate;

        pItem->playerClass = fields[0].GetString();
        pItem->playerSpec = fields[1].GetString();
        pItem->pos = fields[2].GetUInt8();
        pItem->itemEntry = fields[3].GetUInt32();
        pItem->enchant = fields[4].GetUInt32();
        pItem->socket1 = fields[5].GetUInt32();
        pItem->socket2 = fields[6].GetUInt32();
        pItem->socket3 = fields[7].GetUInt32();
        pItem->bonusEnchant = fields[8].GetUInt32();
        pItem->prismaticEnchant = fields[9].GetUInt32();

        m_AllianceGearContainer.push_back(pItem);
        ++count;
    } while (result->NextRow());
    TC_LOG_INFO("server.worldserver", ">>TEMPLATE NPC: Loaded %u gear templates for Alliances in %u ms.", count, GetMSTimeDiffToNow(oldMSTime));
}

void sTemplateNPC::LoadHordeGearContainer()
{
    for (HordeGearContainer::const_iterator itr = m_HordeGearContainer.begin(); itr != m_HordeGearContainer.end(); ++itr)
        delete* itr;

    m_HordeGearContainer.clear();

    QueryResult result = CharacterDatabase.Query("SELECT playerClass, playerSpec, pos, itemEntry, enchant, socket1, socket2, socket3, bonusEnchant, prismaticEnchant FROM template_npc_horde;");

    uint32 oldMSTime = getMSTime();
    uint32 count = 0;

    if (!result)
    {
        TC_LOG_INFO("server.worldserver", ">>TEMPLATE NPC: Loaded 0 'gear templates. DB table `template_npc_horde` is empty!");
        return;
    }

    do
    {
        Field* fields = result->Fetch();

        HordeGearTemplate* pItem = new HordeGearTemplate;

        pItem->playerClass = fields[0].GetString();
        pItem->playerSpec = fields[1].GetString();
        pItem->pos = fields[2].GetUInt8();
        pItem->itemEntry = fields[3].GetUInt32();
        pItem->enchant = fields[4].GetUInt32();
        pItem->socket1 = fields[5].GetUInt32();
        pItem->socket2 = fields[6].GetUInt32();
        pItem->socket3 = fields[7].GetUInt32();
        pItem->bonusEnchant = fields[8].GetUInt32();
        pItem->prismaticEnchant = fields[9].GetUInt32();

        m_HordeGearContainer.push_back(pItem);
        ++count;
    } while (result->NextRow());
    TC_LOG_INFO("server.worldserver", ">>TEMPLATE NPC: Loaded %u gear templates for Hordes in %u ms.", count, GetMSTimeDiffToNow(oldMSTime));
}

std::string sTemplateNPC::GetClassString(Player* player)
{
    switch (player->getClass())
    {
    case CLASS_PRIEST:       return "Priest";      break;
    case CLASS_PALADIN:      return "Paladin";     break;
    case CLASS_WARRIOR:      return "Warrior";     break;
    case CLASS_MAGE:         return "Mage";        break;
    case CLASS_WARLOCK:      return "Warlock";     break;
    case CLASS_SHAMAN:       return "Shaman";      break;
    case CLASS_DRUID:        return "Druid";       break;
    case CLASS_HUNTER:       return "Hunter";      break;
    case CLASS_ROGUE:        return "Rogue";       break;
    case CLASS_DEATH_KNIGHT: return "DeathKnight"; break;
    default:
        break;
    }
    return "Unknown"; // Fix warning, this should never happen
}

bool sTemplateNPC::OverwriteTemplate(Player* player, std::string& playerSpecStr)
{
    // Delete old talent and glyph templates before extracting new ones
    CharacterDatabase.PExecute("DELETE FROM template_npc_talents WHERE playerClass = '%s' AND playerSpec = '%s';", GetClassString(player).c_str(), playerSpecStr.c_str());
    CharacterDatabase.PExecute("DELETE FROM template_npc_glyphs WHERE playerClass = '%s' AND playerSpec = '%s';", GetClassString(player).c_str(), playerSpecStr.c_str());

    // Delete old gear templates before extracting new ones
    if (player->getRace() == RACE_HUMAN)
    {
        CharacterDatabase.PExecute("DELETE FROM template_npc_human WHERE playerClass = '%s' AND playerSpec = '%s';", GetClassString(player).c_str(), playerSpecStr.c_str());
        player->GetSession()->SendAreaTriggerMessage("Template successfuly created!");
        return false;
    }
    else if (player->GetTeam() == ALLIANCE && player->getRace() != RACE_HUMAN)
    {
        CharacterDatabase.PExecute("DELETE FROM template_npc_alliance WHERE playerClass = '%s' AND playerSpec = '%s';", GetClassString(player).c_str(), playerSpecStr.c_str());
        player->GetSession()->SendAreaTriggerMessage("Template successfuly created!");
        return false;
    }
    else if (player->GetTeam() == HORDE)
    {                                                                                                        // ????????????? sTemplateNpcMgr here??
        CharacterDatabase.PExecute("DELETE FROM template_npc_horde WHERE playerClass = '%s' AND playerSpec = '%s';", GetClassString(player).c_str(), playerSpecStr.c_str());
        player->GetSession()->SendAreaTriggerMessage("Template successfuly created!");
        return false;
    }
    return true;
}

void sTemplateNPC::ExtractGearTemplateToDB(Player* player, std::string& playerSpecStr)
{
    CharacterDatabase.PExecute("DELETE FROM template_npc_human WHERE playerClass = '%s' AND playerSpec = '%s';", GetClassString(player).c_str(), playerSpecStr.c_str());
    CharacterDatabase.PExecute("DELETE FROM template_npc_alliance WHERE playerClass = '%s' AND playerSpec = '%s';", GetClassString(player).c_str(), playerSpecStr.c_str());
    CharacterDatabase.PExecute("DELETE FROM template_npc_horde WHERE playerClass = '%s' AND playerSpec = '%s';", GetClassString(player).c_str(), playerSpecStr.c_str());

    for (uint8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
    {
        Item* equippedItem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i);

        if (equippedItem)
        {
            CharacterDatabase.PExecute("INSERT INTO template_npc_human (`playerClass`, `playerSpec`, `pos`, `itemEntry`, `enchant`, `socket1`, `socket2`, `socket3`, `bonusEnchant`, `prismaticEnchant`) VALUES ('%s', '%s', '%u', '%u', '%u', '%u', '%u', '%u', '%u', '%u');"
                , GetClassString(player).c_str(), playerSpecStr.c_str(), equippedItem->GetSlot(), equippedItem->GetEntry(), equippedItem->GetEnchantmentId(PERM_ENCHANTMENT_SLOT),
                equippedItem->GetEnchantmentId(SOCK_ENCHANTMENT_SLOT), equippedItem->GetEnchantmentId(SOCK_ENCHANTMENT_SLOT_2), equippedItem->GetEnchantmentId(SOCK_ENCHANTMENT_SLOT_3),
                equippedItem->GetEnchantmentId(BONUS_ENCHANTMENT_SLOT), equippedItem->GetEnchantmentId(PRISMATIC_ENCHANTMENT_SLOT));
            CharacterDatabase.PExecute("INSERT INTO template_npc_alliance (`playerClass`, `playerSpec`, `pos`, `itemEntry`, `enchant`, `socket1`, `socket2`, `socket3`, `bonusEnchant`, `prismaticEnchant`) VALUES ('%s', '%s', '%u', '%u', '%u', '%u', '%u', '%u', '%u', '%u');"
                , GetClassString(player).c_str(), playerSpecStr.c_str(), equippedItem->GetSlot(), equippedItem->GetEntry(), equippedItem->GetEnchantmentId(PERM_ENCHANTMENT_SLOT),
                equippedItem->GetEnchantmentId(SOCK_ENCHANTMENT_SLOT), equippedItem->GetEnchantmentId(SOCK_ENCHANTMENT_SLOT_2), equippedItem->GetEnchantmentId(SOCK_ENCHANTMENT_SLOT_3),
                equippedItem->GetEnchantmentId(BONUS_ENCHANTMENT_SLOT), equippedItem->GetEnchantmentId(PRISMATIC_ENCHANTMENT_SLOT));
            CharacterDatabase.PExecute("INSERT INTO template_npc_horde (`playerClass`, `playerSpec`, `pos`, `itemEntry`, `enchant`, `socket1`, `socket2`, `socket3`, `bonusEnchant`, `prismaticEnchant`) VALUES ('%s', '%s', '%u', '%u', '%u', '%u', '%u', '%u', '%u', '%u');"
                , GetClassString(player).c_str(), playerSpecStr.c_str(), equippedItem->GetSlot(), equippedItem->GetEntry(), equippedItem->GetEnchantmentId(PERM_ENCHANTMENT_SLOT),
                equippedItem->GetEnchantmentId(SOCK_ENCHANTMENT_SLOT), equippedItem->GetEnchantmentId(SOCK_ENCHANTMENT_SLOT_2), equippedItem->GetEnchantmentId(SOCK_ENCHANTMENT_SLOT_3),
                equippedItem->GetEnchantmentId(BONUS_ENCHANTMENT_SLOT), equippedItem->GetEnchantmentId(PRISMATIC_ENCHANTMENT_SLOT));
        }

        /*if (equippedItem)
        {
            if (player->getRace() == RACE_HUMAN)
            {
                CharacterDatabase.PExecute("INSERT INTO template_npc_human (`playerClass`, `playerSpec`, `pos`, `itemEntry`, `enchant`, `socket1`, `socket2`, `socket3`, `bonusEnchant`, `prismaticEnchant`) VALUES ('%s', '%s', '%u', '%u', '%u', '%u', '%u', '%u', '%u', '%u');"
                    , GetClassString(player).c_str(), playerSpecStr.c_str(), equippedItem->GetSlot(), equippedItem->GetEntry(), equippedItem->GetEnchantmentId(PERM_ENCHANTMENT_SLOT),
                    equippedItem->GetEnchantmentId(SOCK_ENCHANTMENT_SLOT), equippedItem->GetEnchantmentId(SOCK_ENCHANTMENT_SLOT_2), equippedItem->GetEnchantmentId(SOCK_ENCHANTMENT_SLOT_3),
                    equippedItem->GetEnchantmentId(BONUS_ENCHANTMENT_SLOT), equippedItem->GetEnchantmentId(PRISMATIC_ENCHANTMENT_SLOT));
            }
            else if (player->GetTeam() == ALLIANCE && player->getRace() != RACE_HUMAN)
            {
                CharacterDatabase.PExecute("INSERT INTO template_npc_alliance (`playerClass`, `playerSpec`, `pos`, `itemEntry`, `enchant`, `socket1`, `socket2`, `socket3`, `bonusEnchant`, `prismaticEnchant`) VALUES ('%s', '%s', '%u', '%u', '%u', '%u', '%u', '%u', '%u', '%u');"
                    , GetClassString(player).c_str(), playerSpecStr.c_str(), equippedItem->GetSlot(), equippedItem->GetEntry(), equippedItem->GetEnchantmentId(PERM_ENCHANTMENT_SLOT),
                    equippedItem->GetEnchantmentId(SOCK_ENCHANTMENT_SLOT), equippedItem->GetEnchantmentId(SOCK_ENCHANTMENT_SLOT_2), equippedItem->GetEnchantmentId(SOCK_ENCHANTMENT_SLOT_3),
                    equippedItem->GetEnchantmentId(BONUS_ENCHANTMENT_SLOT), equippedItem->GetEnchantmentId(PRISMATIC_ENCHANTMENT_SLOT));
            }
            else if (player->GetTeam() == HORDE)
            {
                CharacterDatabase.PExecute("INSERT INTO template_npc_horde (`playerClass`, `playerSpec`, `pos`, `itemEntry`, `enchant`, `socket1`, `socket2`, `socket3`, `bonusEnchant`, `prismaticEnchant`) VALUES ('%s', '%s', '%u', '%u', '%u', '%u', '%u', '%u', '%u', '%u');"
                    , GetClassString(player).c_str(), playerSpecStr.c_str(), equippedItem->GetSlot(), equippedItem->GetEntry(), equippedItem->GetEnchantmentId(PERM_ENCHANTMENT_SLOT),
                    equippedItem->GetEnchantmentId(SOCK_ENCHANTMENT_SLOT), equippedItem->GetEnchantmentId(SOCK_ENCHANTMENT_SLOT_2), equippedItem->GetEnchantmentId(SOCK_ENCHANTMENT_SLOT_3),
                    equippedItem->GetEnchantmentId(BONUS_ENCHANTMENT_SLOT), equippedItem->GetEnchantmentId(PRISMATIC_ENCHANTMENT_SLOT));
            }
        }*/
    }
}

void sTemplateNPC::ExtractTalentTemplateToDB(Player* player, std::string& playerSpecStr)
{
    QueryResult result = CharacterDatabase.PQuery("SELECT spell FROM character_talent WHERE guid = '%u' "
        "AND talentGroup = '%u';", player->GetGUID(), player->GetActiveSpec());

    if (!result)
    {
        return;
    }
    else if (player->GetFreeTalentPoints() > 0)
    {
        player->GetSession()->SendAreaTriggerMessage("You have unspend talent points. Please spend all your talent points and re-extract the template.");
        return;
    }
    else
    {
        do
        {
            Field* fields = result->Fetch();
            uint32 spell = fields[0].GetUInt32();

            CharacterDatabase.PExecute("INSERT INTO template_npc_talents (playerClass, playerSpec, talentId) "
                "VALUES ('%s', '%s', '%u');", GetClassString(player).c_str(), playerSpecStr.c_str(), spell);
        } while (result->NextRow());
    }
}

void sTemplateNPC::ExtractGlyphsTemplateToDB(Player* player, std::string& playerSpecStr)
{
    CharacterDatabase.PExecute("DELETE FROM template_npc_glyphs WHERE playerClass = '%s' AND playerSpec = '%s';", GetClassString(player).c_str(), playerSpecStr.c_str());

    QueryResult result = CharacterDatabase.PQuery("SELECT glyph1, glyph2, glyph3, glyph4, glyph5, glyph6, glyph7, glyph8, glyph9 "
        "FROM character_glyphs WHERE guid = '%u' AND talentGroup = '%u';", player->GetGUID(), player->GetActiveSpec());

    for (uint8 slot = 0; slot < MAX_GLYPH_SLOT_INDEX; ++slot)
    {
        if (!result)
        {
            player->GetSession()->SendAreaTriggerMessage("Get glyphs and re-extract the template!");
            continue;
        }

        Field* fields = result->Fetch();
        uint32 glyph1 = fields[0].GetUInt32();
        uint32 glyph2 = fields[1].GetUInt32();
        uint32 glyph3 = fields[2].GetUInt32();
        uint32 glyph4 = fields[3].GetUInt32();
        uint32 glyph5 = fields[4].GetUInt32();
        uint32 glyph6 = fields[5].GetUInt32();
        uint32 glyph7 = fields[6].GetUInt32();
        uint32 glyph8 = fields[7].GetUInt32();
        uint32 glyph9 = fields[8].GetUInt32();

        uint32 storedGlyph;

        switch (slot)
        {
        case 0:
            storedGlyph = glyph1;
            break;
        case 1:
            storedGlyph = glyph2;
            break;
        case 2:
            storedGlyph = glyph3;
            break;
        case 3:
            storedGlyph = glyph4;
            break;
        case 4:
            storedGlyph = glyph5;
            break;
        case 5:
            storedGlyph = glyph6;
            break;
        case 6:
            storedGlyph = glyph7;
            break;
        case 7:
            storedGlyph = glyph8;
            break;
        case 8:
            storedGlyph = glyph9;
            break;
        default:
            break;
        }

        CharacterDatabase.PExecute("INSERT INTO template_npc_glyphs (playerClass, playerSpec, slot, glyph) "
            "VALUES ('%s', '%s', '%u', '%u');", GetClassString(player).c_str(), playerSpecStr.c_str(), slot, storedGlyph);
    }
}

bool sTemplateNPC::CanEquipTemplate(Player* player, std::string& playerSpecStr)
{
    if (player->getRace() == RACE_HUMAN)
    {
        QueryResult result = CharacterDatabase.PQuery("SELECT playerClass, playerSpec FROM template_npc_human "
            "WHERE playerClass = '%s' AND playerSpec = '%s';", GetClassString(player).c_str(), playerSpecStr.c_str());

        if (!result)
            return false;
    }
    else if (player->GetTeam() == ALLIANCE && player->getRace() != RACE_HUMAN)
    {
        QueryResult result = CharacterDatabase.PQuery("SELECT playerClass, playerSpec FROM template_npc_alliance "
            "WHERE playerClass = '%s' AND playerSpec = '%s';", GetClassString(player).c_str(), playerSpecStr.c_str());

        if (!result)
            return false;
    }
    else if (player->GetTeam() == HORDE)
    {
        QueryResult result = CharacterDatabase.PQuery("SELECT playerClass, playerSpec FROM template_npc_horde "
            "WHERE playerClass = '%s' AND playerSpec = '%s';", GetClassString(player).c_str(), playerSpecStr.c_str());

        if (!result)
            return false;
    }
    return true;
}

class TemplateNPC : public CreatureScript
{
public:
    TemplateNPC() : CreatureScript("TemplateNPC") { }

    struct TemplateNPC_AI : public ScriptedAI
    {
        TemplateNPC_AI(Creature* creature) : ScriptedAI(creature) { }

        bool GossipHello(Player* player) override
        {
            ClearGossipMenuFor(player);
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|cff00ff00|TInterface\\icons\\achievement_featsofstrength_gladiator_08:40|t|r Cataclysmic Gladiator Gear (Season 11)", GOSSIP_SENDER_MAIN, GOSSIP_PVP_GEAR);
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|cff00ff00|TInterface\\icons\\achievment_boss_madnessofdeathwing:40|t|r Best in Slot Raid Gear (T13)", GOSSIP_SENDER_MAIN, GOSSIP_PVE_GEAR);
            if (player->getClass() == CLASS_HUNTER && player->HasSpell(MASTERY_HUNTER))
            {
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|cff00ff00|TInterface\\icons\\ability_hunter_pet_spider:40|t|r Hunter Pets", GOSSIP_SENDER_MAIN, GOSSIP_HUNTER_PET);
            }
             
            SendGossipMenuFor(player, 55002, me->GetGUID());
            return true;
        }

        bool GossipSelect(Player* player, uint32 /*sender*/, uint32 uiAction) override
        {
            uint32 sender = player->PlayerTalkClass->GetGossipOptionSender(uiAction);
            uint32 action = player->PlayerTalkClass->GetGossipOptionAction(uiAction);

            Creature* creature = me;

            TC_LOG_INFO("server.worldserver", ">>TEMPLATE NPC: GossipSelect() called.");
            TC_LOG_INFO("server.worldserver", "Action: %d, Sender: %d", action, sender);
            if (sender != GOSSIP_SENDER_MAIN)
                return false;
            if (action == GOSSIP_MAIN_MENU)
            {
                ClearGossipMenuFor(player);
                GossipHello(player);
            }
            else if (action == GOSSIP_PVP_GEAR)
            {
                TC_LOG_INFO("server.worldserver", ">>TEMPLATE NPC: GOSSIP_PVP_GEAR selected.");
                ClearGossipMenuFor(player);
                AddPvPOptionsForClass(player);
                SendGossipMenuFor(player, 55002, me->GetGUID());
            }
            else if (action == GOSSIP_PVE_GEAR)
            {
                TC_LOG_INFO("server.worldserver", ">>TEMPLATE NPC: GOSSIP_PVE_GEAR selected.");
                ClearGossipMenuFor(player);
                AddPvEOptionsForClass(player);
                SendGossipMenuFor(player, 55002, me->GetGUID());
            }
            else if (action == GOSSIP_HUNTER_PET)
            {
                TC_LOG_INFO("server.worldserver", ">>TEMPLATE NPC: GOSSIP_HUNTER_PET selected.");
                ClearGossipMenuFor(player);
                AddPetOptionsForHunter(player);
                SendGossipMenuFor(player, 55002, me->GetGUID());
            }
            
            switch (action)
            {
            // PVP
            case GOSSIP_PVP_GEAR + 1:
                sTemplateNpcMgr->sTalentsSpec = "Arms-Warrior-PvP";
                EquipFullTemplateGear(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVP_GEAR + 2:
                sTemplateNpcMgr->sTalentsSpec = "Fury-Warrior-PvP";
                EquipFullTemplateGear(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVP_GEAR + 3:
                sTemplateNpcMgr->sTalentsSpec = "Protection-Warrior-PvP";
                EquipFullTemplateGear(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVP_GEAR + 4:
                sTemplateNpcMgr->sTalentsSpec = "Assassination-Rogue-PvP";
                EquipFullTemplateGear(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVP_GEAR + 5:
                sTemplateNpcMgr->sTalentsSpec = "Combat-Rogue-PvP";
                EquipFullTemplateGear(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVP_GEAR + 6:
                sTemplateNpcMgr->sTalentsSpec = "Subtlety-Rogue-PvP";
                EquipFullTemplateGear(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVP_GEAR + 7:
                sTemplateNpcMgr->sTalentsSpec = "Elemental-Shaman-PvP";
                EquipFullTemplateGear(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVP_GEAR + 8:
                sTemplateNpcMgr->sTalentsSpec = "Enhancement-Shaman-PvP";
                EquipFullTemplateGear(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVP_GEAR + 9:
                sTemplateNpcMgr->sTalentsSpec = "Restoration-Shaman-PvE";
                EquipFullTemplateGear(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVP_GEAR + 10:
                sTemplateNpcMgr->sTalentsSpec = "Beast-Mastery-Hunter-PvP";
                EquipFullTemplateGear(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVP_GEAR + 11:
                sTemplateNpcMgr->sTalentsSpec = "Marksmanship-Hunter-PvP";
                EquipFullTemplateGear(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVP_GEAR + 12:
                sTemplateNpcMgr->sTalentsSpec = "Survival-Hunter-PvP";
                EquipFullTemplateGear(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVP_GEAR + 13:
                sTemplateNpcMgr->sTalentsSpec = "Arcane-Mage-PvP";
                EquipFullTemplateGear(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVP_GEAR + 14:
                sTemplateNpcMgr->sTalentsSpec = "Fire-Mage-PvP";
                EquipFullTemplateGear(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVP_GEAR + 15:
                sTemplateNpcMgr->sTalentsSpec = "Frost-Mage-PvP";
                EquipFullTemplateGear(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVP_GEAR + 16:
                sTemplateNpcMgr->sTalentsSpec = "Affliction-Warlock-PvP";
                EquipFullTemplateGear(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVP_GEAR + 17:
                sTemplateNpcMgr->sTalentsSpec = "Demonology-Warlock-PvP";
                EquipFullTemplateGear(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVP_GEAR + 18:
                sTemplateNpcMgr->sTalentsSpec = "Destruction-Warlock-PvP";
                EquipFullTemplateGear(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;

            // PVE
            case GOSSIP_PVE_GEAR + 1:
                sTemplateNpcMgr->sTalentsSpec = "Arms-Warrior-PvE";
                EquipFullTemplateGear(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVE_GEAR + 2:
                sTemplateNpcMgr->sTalentsSpec = "Fury-Warrior-PvE-2H";
                EquipFullTemplateGear(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVE_GEAR + 3:
                sTemplateNpcMgr->sTalentsSpec = "Fury-Warrior-PvE-1H";
                EquipFullTemplateGear(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVE_GEAR + 4:
                sTemplateNpcMgr->sTalentsSpec = "Protection-Warrior-PvE";
                EquipFullTemplateGear(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVE_GEAR + 5:
                sTemplateNpcMgr->sTalentsSpec = "Assassination-Rogue-PvE";
                EquipFullTemplateGear(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVE_GEAR + 6:
                sTemplateNpcMgr->sTalentsSpec = "Combat-Rogue-PvE";
                EquipFullTemplateGear(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVE_GEAR + 7:
                sTemplateNpcMgr->sTalentsSpec = "Subtlety-Rogue-PvE";
                EquipFullTemplateGear(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVE_GEAR + 8:
                sTemplateNpcMgr->sTalentsSpec = "Elemental-Shaman-PvE";
                EquipFullTemplateGear(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVE_GEAR + 9:
                sTemplateNpcMgr->sTalentsSpec = "Enhancement-Shaman-PvE";
                EquipFullTemplateGear(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVE_GEAR + 10:
                sTemplateNpcMgr->sTalentsSpec = "Restoration-Shaman-PvE";
                EquipFullTemplateGear(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVE_GEAR + 11:
                sTemplateNpcMgr->sTalentsSpec = "Beast-Mastery-Hunter-PvE";
                EquipFullTemplateGear(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVE_GEAR + 12:
                sTemplateNpcMgr->sTalentsSpec = "Marksmanship-Hunter-PvE";
                EquipFullTemplateGear(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVE_GEAR + 13:
                sTemplateNpcMgr->sTalentsSpec = "Survival-Hunter-PvE";
                EquipFullTemplateGear(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVE_GEAR + 14:
                sTemplateNpcMgr->sTalentsSpec = "Arcane-Mage-PvE";
                EquipFullTemplateGear(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVE_GEAR + 15:
                sTemplateNpcMgr->sTalentsSpec = "Fire-Mage-PvE";
                EquipFullTemplateGear(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVE_GEAR + 16:
                sTemplateNpcMgr->sTalentsSpec = "Frost-Mage-PvE";
                EquipFullTemplateGear(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVE_GEAR + 17:
                sTemplateNpcMgr->sTalentsSpec = "Affliction-Warlock-PvE";
                EquipFullTemplateGear(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVE_GEAR + 18:
                sTemplateNpcMgr->sTalentsSpec = "Demonology-Warlock-PvE";
                EquipFullTemplateGear(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVE_GEAR + 19:
                sTemplateNpcMgr->sTalentsSpec = "Destruction-Warlock-PvE";
                EquipFullTemplateGear(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;

            // Hunter Pets
            case GOSSIP_HUNTER_PET + 1:
                CreatePet(player, creature, 52013); //fox
                break;
            case GOSSIP_HUNTER_PET + 2:
                CreatePet(player, creature, 52011); //shale spider
                break;
            case GOSSIP_HUNTER_PET + 3:
                CreatePet(player, creature, 48155); //sea gull
                break;
            }

            return true;
        }

        void AddPvPOptionsForClass(Player* player)
        {
            switch (player->getClass())
            {
            case CLASS_WARRIOR:
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|cff00ff00|TInterface\\icons\\Ability_Warrior_SavageBlow:40|t|r |cffff0000PvP|r / Arms", GOSSIP_SENDER_MAIN, GOSSIP_PVP_GEAR + 1);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|cff00ff00|TInterface\\icons\\Ability_warrior_innerrage:40|t|r |cffff0000PvP|r / Fury", GOSSIP_SENDER_MAIN, GOSSIP_PVP_GEAR + 2);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|cff00ff00|TInterface\\icons\\Ability_warrior_defensivestance:40|t|r |cffff0000PvP|r / Protection", GOSSIP_SENDER_MAIN, GOSSIP_PVP_GEAR + 3);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Back..", GOSSIP_SENDER_MAIN, GOSSIP_MAIN_MENU);
                break;
            case CLASS_ROGUE:
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|cff00ff00|TInterface\\icons\\Ability_Rogue_Eviscerate:40|t|r |cffff0000PvP|r / Assassination", GOSSIP_SENDER_MAIN, GOSSIP_PVP_GEAR + 4);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|cff00ff00|TInterface\\icons\\Ability_Backstab:40|t|r |cffff0000PvP|r / Combat", GOSSIP_SENDER_MAIN, GOSSIP_PVP_GEAR + 5);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|cff00ff00|TInterface\\icons\\Ability_Stealth:40|t|r |cffff0000PvP|r / Subtlety", GOSSIP_SENDER_MAIN, GOSSIP_PVP_GEAR + 6);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Back..", GOSSIP_SENDER_MAIN, GOSSIP_MAIN_MENU);
                break;
            case CLASS_SHAMAN:
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|cff00ff00|TInterface\\icons\\Spell_Nature_Lightning:40|t|r |cffff0000PvP|r / Elemental", GOSSIP_SENDER_MAIN, GOSSIP_PVP_GEAR + 7);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|cff00ff00|TInterface\\icons\\Spell_Nature_Lightningshield:40|t|r |cffff0000PvP|r / Enhancement", GOSSIP_SENDER_MAIN, GOSSIP_PVP_GEAR + 8);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|cff00ff00|TInterface\\icons\\Spell_Nature_Magicimmunity:40|t|r |cffff0000PvP|r / Restoration", GOSSIP_SENDER_MAIN, GOSSIP_PVP_GEAR + 9);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Back..", GOSSIP_SENDER_MAIN, GOSSIP_MAIN_MENU);
                break;
            case CLASS_HUNTER:
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|cff00ff00|TInterface\\icons\\Ability_Hunter_BestialDiscipline:40|t|r |cffff0000PvP|r / Beast Mastery", GOSSIP_SENDER_MAIN, GOSSIP_PVP_GEAR + 10);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|cff00ff00|TInterface\\icons\\Ability_Hunter_FocusedAim:40|t|r |cffff0000PvP|r / Marksmanship", GOSSIP_SENDER_MAIN, GOSSIP_PVP_GEAR + 11);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|cff00ff00|TInterface\\icons\\Ability_Hunter_Camouflage:40|t|r |cffff0000PvP|r / Survival", GOSSIP_SENDER_MAIN, GOSSIP_PVP_GEAR + 12);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Back..", GOSSIP_SENDER_MAIN, GOSSIP_MAIN_MENU);
                break;
            case CLASS_MAGE:
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|cff00ff00|TInterface\\icons\\Spell_Holy_MagicalSentry:40|t|r |cffff0000PvP|r / Arcane", GOSSIP_SENDER_MAIN, GOSSIP_PVP_GEAR + 13);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|cff00ff00|TInterface\\icons\\Spell_Fire_FireBolt02:40|t|r |cffff0000PvP|r / Fire", GOSSIP_SENDER_MAIN, GOSSIP_PVP_GEAR + 14);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|cff00ff00|TInterface\\icons\\Spell_Frost_FrostBolt02:40|t|r |cffff0000PvP|r / Frost", GOSSIP_SENDER_MAIN, GOSSIP_PVP_GEAR + 15);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Back..", GOSSIP_SENDER_MAIN, GOSSIP_MAIN_MENU);
                break;
            case CLASS_WARLOCK:
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|cff00ff00|TInterface\\icons\\Spell_Shadow_DeathCoil:40|t|r |cffff0000PvP|r / Arcane", GOSSIP_SENDER_MAIN, GOSSIP_PVP_GEAR + 16);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|cff00ff00|TInterface\\icons\\Spell_Shadow_Metamorphosis:40|t|r |cffff0000PvP|r / Fire", GOSSIP_SENDER_MAIN, GOSSIP_PVP_GEAR + 17);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|cff00ff00|TInterface\\icons\\Spell_Shadow_RainOfFire:40|t|r |cffff0000PvP|r / Frost", GOSSIP_SENDER_MAIN, GOSSIP_PVP_GEAR + 18);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Back..", GOSSIP_SENDER_MAIN, GOSSIP_MAIN_MENU);
                break;
            }
            SendGossipMenuFor(player, 55002, me->GetGUID());
        }

        void AddPvEOptionsForClass(Player* player)
        {
            switch (player->getClass())
            {
            case CLASS_WARRIOR:
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|cff00ff00|TInterface\\icons\\Ability_Warrior_SavageBlow:40|t|r |cff00ff00PvE|r / Arms", GOSSIP_SENDER_MAIN, GOSSIP_PVE_GEAR + 1);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|cff00ff00|TInterface\\icons\\Ability_warrior_innerrage:40|t|r |cff00ff00PvE|r / Fury 2H", GOSSIP_SENDER_MAIN, GOSSIP_PVE_GEAR + 2);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|cff00ff00|TInterface\\icons\\Ability_warrior_innerrage:40|t|r |cff00ff00PvE|r / Fury 1H", GOSSIP_SENDER_MAIN, GOSSIP_PVE_GEAR + 3);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|cff00ff00|TInterface\\icons\\Ability_warrior_defensivestance:40|t|r |cff00ff00PvE|r / Protection", GOSSIP_SENDER_MAIN, GOSSIP_PVE_GEAR + 4);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Back..", GOSSIP_SENDER_MAIN, GOSSIP_MAIN_MENU);
                break;
            case CLASS_ROGUE:
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|cff00ff00|TInterface\\icons\\Ability_Rogue_Eviscerate:40|t|r |cff00ff00PvE|r / Assasination", GOSSIP_SENDER_MAIN, GOSSIP_PVE_GEAR + 5);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|cff00ff00|TInterface\\icons\\Ability_Backstab:40|t|r |cff00ff00PvE|r / Combat", GOSSIP_SENDER_MAIN, GOSSIP_PVE_GEAR + 6);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|cff00ff00|TInterface\\icons\\Ability_Stealth:40|t|r |cff00ff00PvE|r / Subtlety", GOSSIP_SENDER_MAIN, GOSSIP_PVE_GEAR + 7);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Back..", GOSSIP_SENDER_MAIN, GOSSIP_MAIN_MENU);
                break;
            case CLASS_SHAMAN:
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|cff00ff00|TInterface\\icons\\Spell_Nature_Lightning:40|t|r |cff00ff00PvE|r / Elemental", GOSSIP_SENDER_MAIN, GOSSIP_PVE_GEAR + 8);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|cff00ff00|TInterface\\icons\\Spell_Nature_Lightningshield:40|t|r |cff00ff00PvE|r / Enhancement", GOSSIP_SENDER_MAIN, GOSSIP_PVE_GEAR + 9);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|cff00ff00|TInterface\\icons\\Spell_Nature_Magicimmunity:40|t|r |cff00ff00PvE|r / Restoration", GOSSIP_SENDER_MAIN, GOSSIP_PVE_GEAR + 10);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Back..", GOSSIP_SENDER_MAIN, GOSSIP_MAIN_MENU);
                break;
            case CLASS_HUNTER:
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|cff00ff00|TInterface\\icons\\Ability_Hunter_BestialDiscipline:40|t|r |cff00ff00PvE|r / Beast Mastery", GOSSIP_SENDER_MAIN, GOSSIP_PVE_GEAR + 11);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|cff00ff00|TInterface\\icons\\Ability_Hunter_FocusedAim:40|t|r |cff00ff00PvE|r / Marksmanship", GOSSIP_SENDER_MAIN, GOSSIP_PVE_GEAR + 12);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|cff00ff00|TInterface\\icons\\Ability_Hunter_Camouflage:40|t|r |cff00ff00PvE|r / Survival", GOSSIP_SENDER_MAIN, GOSSIP_PVE_GEAR + 13);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Back..", GOSSIP_SENDER_MAIN, GOSSIP_MAIN_MENU);
                break;
            case CLASS_MAGE:
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|cff00ff00|TInterface\\icons\\Spell_Holy_MagicalSentry:40|t|r |cff00ff00PvE|r / Affliction", GOSSIP_SENDER_MAIN, GOSSIP_PVE_GEAR + 14);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|cff00ff00|TInterface\\icons\\Spell_Fire_FireBolt02:40|t|r |cff00ff00PvE|r / Demonology", GOSSIP_SENDER_MAIN, GOSSIP_PVE_GEAR + 15);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|cff00ff00|TInterface\\icons\\Spell_Frost_FrostBolt02:40|t|r |cff00ff00PvE|r / Destruction", GOSSIP_SENDER_MAIN, GOSSIP_PVE_GEAR + 16);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Back..", GOSSIP_SENDER_MAIN, GOSSIP_MAIN_MENU);
                break;
            case CLASS_WARLOCK:
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|cff00ff00|TInterface\\icons\\Spell_Shadow_DeathCoil:40|t|r |cff00ff00PvE|r / Affliction", GOSSIP_SENDER_MAIN, GOSSIP_PVE_GEAR + 17);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|cff00ff00|TInterface\\icons\\Spell_Shadow_Metamorphosis:40|t|r |cff00ff00PvE|r / Demonology", GOSSIP_SENDER_MAIN, GOSSIP_PVE_GEAR + 18);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|cff00ff00|TInterface\\icons\\Spell_Shadow_RainOfFire:40|t|r |cff00ff00PvE|r / Destruction", GOSSIP_SENDER_MAIN, GOSSIP_PVE_GEAR + 19);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Back..", GOSSIP_SENDER_MAIN, GOSSIP_MAIN_MENU);
                break;
            }
            SendGossipMenuFor(player, 55002, me->GetGUID());
        }

        void AddPetOptionsForHunter(Player* player)
        {
            switch (player->getClass())
            {
            case CLASS_HUNTER:
                if (player->HasSpell(53270)) // beast mastery
                {
                    AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Shale Spider", GOSSIP_SENDER_MAIN, GOSSIP_HUNTER_PET + 1);
                }
                else
                { 
                    AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Fox", GOSSIP_SENDER_MAIN, GOSSIP_HUNTER_PET + 2);
                    AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Sea Gull", GOSSIP_SENDER_MAIN, GOSSIP_HUNTER_PET + 3);
                    AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Back..", GOSSIP_SENDER_MAIN, GOSSIP_MAIN_MENU);
                }
                break;
               
            }
            SendGossipMenuFor(player, 55002, me->GetGUID());
        }

        static void EquipFullTemplateGear(Player* player, std::string& playerSpecStr) // Merge
        {
            if (sTemplateNpcMgr->CanEquipTemplate(player, playerSpecStr) == false)
            {
                player->GetSession()->SendAreaTriggerMessage("There's no templates for %s specialization yet.", playerSpecStr.c_str());
                return;
            }

            // Don't let players to use Template feature while wearing some gear
            for (uint8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
            {
                if (Item* haveItemEquipped = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                {
                    if (haveItemEquipped)
                    {
                        //player->GetSession()->SendAreaTriggerMessage("You need to remove all your equipped items in order to use this feature!");
                        //CloseGossipMenuFor(player);
                        player->DestroyItem(INVENTORY_SLOT_BAG_0, i, true);
                        //return;
                    }
                }
            }

            // Don't let players to use Template feature after spending some talent points
            
            if (player->GetFreeTalentPoints() < 41)
            {
                //player->GetSession()->SendAreaTriggerMessage("You have already spent some talent points. You need to reset your talents first!");
                //CloseGossipMenuFor(player);
                //return;
            }
            

            //sTemplateNpcMgr->LearnTemplateTalents(player);
            sTemplateNpcMgr->LearnTemplateGlyphs(player);
            sTemplateNpcMgr->EquipTemplateGear(player);
            sTemplateNpcMgr->LearnPlateMailSpells(player);

            LearnGlyphs(player);

            player->GetSession()->SendAreaTriggerMessage("Successfuly equipped %s %s template!", playerSpecStr.c_str(), sTemplateNpcMgr->GetClassString(player).c_str());

            if (player->GetPowerType() == POWER_MANA)
                player->SetFullPower(POWER_MANA);

            player->SetFullHealth();

            // Learn Riding/Flying
            if (player->HasSpell(Artisan_Riding) ||
                player->HasSpell(Cold_Weather_Flying) ||
                player->HasSpell(Amani_War_Bear) ||
                player->HasSpell(Teach_Learn_Talent_Specialization_Switches) ||
                player->HasSpell(Learn_a_Second_Talent_Specialization))
                return;

            // Cast spells that teach dual spec
            // Both are also ImplicitTarget self and must be cast by player
            player->CastSpell(player, Teach_Learn_Talent_Specialization_Switches, player->GetGUID());
            player->CastSpell(player, Learn_a_Second_Talent_Specialization, player->GetGUID());

            player->LearnSpell(Artisan_Riding, false);
            player->LearnSpell(Cold_Weather_Flying, false);
            player->LearnSpell(Amani_War_Bear, false);

        }

        static void LearnOnlyTalentsAndGlyphs(Player* player, std::string& playerSpecStr) // Merge
        {
            if (sTemplateNpcMgr->CanEquipTemplate(player, playerSpecStr) == false)
            {
                player->GetSession()->SendAreaTriggerMessage("There's no templates for %s specialization yet.", playerSpecStr.c_str());
                return;
            }

            // Don't let players to use Template feature after spending some talent points
            if (player->GetFreeTalentPoints() < 71)
            {
                player->GetSession()->SendAreaTriggerMessage("You have already spent some talent points. You need to reset your talents first!");
                CloseGossipMenuFor(player);
                return;
            }

            sTemplateNpcMgr->LearnTemplateTalents(player);
            sTemplateNpcMgr->LearnTemplateGlyphs(player);
            //sTemplateNpcMgr->EquipTemplateGear(player);
            sTemplateNpcMgr->LearnPlateMailSpells(player);

            player->GetSession()->SendAreaTriggerMessage("Successfuly learned talent spec %s!", playerSpecStr.c_str());

            // Learn Riding/Flying
            if (player->HasSpell(Artisan_Riding) ||
                player->HasSpell(Cold_Weather_Flying) ||
                player->HasSpell(Amani_War_Bear) ||
                player->HasSpell(Teach_Learn_Talent_Specialization_Switches) ||
                player->HasSpell(Learn_a_Second_Talent_Specialization))
                return;

            // Cast spells that teach dual spec
            // Both are also ImplicitTarget self and must be cast by player
            player->CastSpell(player, Teach_Learn_Talent_Specialization_Switches, player->GetGUID());
            player->CastSpell(player, Learn_a_Second_Talent_Specialization, player->GetGUID());

            player->LearnSpell(Artisan_Riding, false);
            player->LearnSpell(Cold_Weather_Flying, false);
            player->LearnSpell(Amani_War_Bear, false);
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new TemplateNPC_AI(creature);
    }
};

class TemplateNPC_World : public WorldScript
{
public:
    TemplateNPC_World() : WorldScript("TemplateNPC_World") { }

    void OnStartup() override
    {
        // Load templates for Template NPC #1
        TC_LOG_INFO("server.loading", "== TEMPLATE NPC ===========================================================================");
        TC_LOG_INFO("server.loading", "Loading Template Talents...");
        sTemplateNpcMgr->LoadTalentsContainer();

        // Load templates for Template NPC #2
        TC_LOG_INFO("server.loading", "Loading Template Glyphs...");
        sTemplateNpcMgr->LoadGlyphsContainer();

        // Load templates for Template NPC #3
        TC_LOG_INFO("server.loading", "Loading Template Gear for Humans...");
        sTemplateNpcMgr->LoadHumanGearContainer();

        // Load templates for Template NPC #4
        TC_LOG_INFO("server.loading", "Loading Template Gear for Alliances...");
        sTemplateNpcMgr->LoadAllianceGearContainer();

        // Load templates for Template NPC #5
        TC_LOG_INFO("server.loading", "Loading Template Gear for Hordes...");
        sTemplateNpcMgr->LoadHordeGearContainer();
        TC_LOG_INFO("server.loading", "== END TEMPLATE NPC ===========================================================================");
    }
};

class TemplateNPC_command : public CommandScript
{
public:
    TemplateNPC_command() : CommandScript("TemplateNPC_command") { }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> TemplateNPCTable =
        {
            { "reload",  SEC_ADMINISTRATOR, true,  &HandleReloadTemplateNPCCommand, "" },
            { "gear",  SEC_ADMINISTRATOR, true,  &HandleExtractTemplateNPCGear, ""},
            { "glyphs",  SEC_ADMINISTRATOR, true,  &HandleExtractTemplateNPCGlyphs, ""},
            { "both",  SEC_ADMINISTRATOR, true,  &HandleExtractTemplateNPCBoth, ""}
        };

        static std::vector<ChatCommand> commandTable =
        {
            { "templatenpc", SEC_ADMINISTRATOR, true, nullptr, "", TemplateNPCTable }
        };
        return commandTable;
    }

    static bool HandleReloadTemplateNPCCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO("server.loading", "misc", "Reloading templates for Template NPC table...");
        sTemplateNpcMgr->LoadTalentsContainer();
        sTemplateNpcMgr->LoadGlyphsContainer();
        sTemplateNpcMgr->LoadHumanGearContainer();
        sTemplateNpcMgr->LoadAllianceGearContainer();
        sTemplateNpcMgr->LoadHordeGearContainer();
        handler->SendGlobalGMSysMessage("Template NPC templates reloaded.");
        return true;
    }

    static bool HandleExtractTemplateNPCGear(ChatHandler* handler, const char* args)
    {
        Player* player = handler->getSelectedPlayer();
        if (!player)
        {
            TC_LOG_INFO("server.worldserver", ">>TEMPLATE NPC: No player selected.");
            return false;
        }

        std::string spec = args;
        sTemplateNpcMgr->sTalentsSpec = spec;

        TC_LOG_INFO("server.worldserver", ">>TEMPLATE NPC: GOSSIP_EXTRACT_GEAR selected.");
        TC_LOG_INFO("server.worldserver", ">>TEMPLATE NPC: Player %s sTalentsSpec: %s", player->GetName().c_str(), spec.c_str());

        sTemplateNpcMgr->ExtractGearTemplateToDB(player, sTemplateNpcMgr->sTalentsSpec);
        return true;
    }

    static bool HandleExtractTemplateNPCGlyphs(ChatHandler* handler, const char* args)
    {
        Player* player = handler->getSelectedPlayer();
        if (!player)
        {
            // Handle the error appropriately, perhaps by sending a message to the admin
            TC_LOG_INFO("server.worldserver", ">>TEMPLATE NPC: No player selected.");
            return false;
        }

        std::string spec = args;
        sTemplateNpcMgr->sTalentsSpec = spec;

        TC_LOG_INFO("server.worldserver", ">>TEMPLATE NPC: GOSSIP_EXTRACT_GLYPHS selected.");
        TC_LOG_INFO("server.worldserver", ">>TEMPLATE NPC: Player %s sTalentsSpec: %s", player->GetName().c_str(), spec.c_str());
        sTemplateNpcMgr->ExtractGlyphsTemplateToDB(player, sTemplateNpcMgr->sTalentsSpec);

        return true;
    }


    static bool HandleExtractTemplateNPCBoth(ChatHandler* handler, const char* args)
    {
        Player* player = handler->getSelectedPlayer();
        if (!player)
        {
            TC_LOG_INFO("server.worldserver", ">>TEMPLATE NPC: No player selected.");
            return false;
        }

        std::string spec = args;
        sTemplateNpcMgr->sTalentsSpec = spec;

        TC_LOG_INFO("server.worldserver", ">>TEMPLATE NPC: GOSSIP_EXTRACT_BOTH selected.");
        TC_LOG_INFO("server.worldserver", ">>TEMPLATE NPC: Player %s sTalentsSpec: %s", player->GetName().c_str(), spec.c_str());

        sTemplateNpcMgr->ExtractGearTemplateToDB(player, sTemplateNpcMgr->sTalentsSpec);
        sTemplateNpcMgr->ExtractGlyphsTemplateToDB(player, sTemplateNpcMgr->sTalentsSpec);
        return true;
    }

};

void AddSC_TemplateNPC()
{
    new TemplateNPC();
    new TemplateNPC_World();
    new TemplateNPC_command();
}
