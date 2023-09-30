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
    GOSSIP_HUNTER_PET = 400,
    GOSSIP_RESET_TALENTS = 999
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

void sTemplateNPC::LearnTemplateTalentsPvE(Player* player)
{
    player->ResetTalents(true);

    for (TalentContainerPvE::const_iterator itr = m_TalentContainerPvE.begin(); itr != m_TalentContainerPvE.end(); ++itr)
    {
        if ((*itr)->playerClass == GetClassString(player).c_str() && (*itr)->playerSpec == sTalentsSpec)
        {
            
            player->LearnSpell((*itr)->talentId, false);
            player->AddTalent((*itr)->talentId, player->GetActiveSpec(), true);
        }
    }

    uint32 id = 0;

    for (TalentTreeIdContainer::const_iterator itr = m_TalentTreeIdContainer.begin(); itr != m_TalentTreeIdContainer.end(); ++itr)
    {
        if ((*itr)->playerSpec == sTalentsSpec)
        {
            id = (*itr)->treeId;
            break;
        }
    }

    if (id != 0)
    {
        // Set the treeId as the player's specialization
        player->SetPrimaryTalentTree(player->GetActiveSpec(), id);
    }
    else
    {
        TC_LOG_ERROR("server.worldserver", "Could not find treeId for spec %s", sTalentsSpec.c_str());
    }

    player->SetFreeTalentPoints(0);
    player->SendTalentsInfoData(false);
}

void sTemplateNPC::LearnTemplateTalentsPvP(Player* player)
{
    player->ResetTalents(true);

    for (TalentContainerPvP::const_iterator itr = m_TalentContainerPvP.begin(); itr != m_TalentContainerPvP.end(); ++itr)
    {
        if ((*itr)->playerClass == GetClassString(player).c_str() && (*itr)->playerSpec == sTalentsSpec)
        {
            player->LearnSpell((*itr)->talentId, false);
            player->AddTalent((*itr)->talentId, player->GetActiveSpec(), true);
        }
    }
    uint32 id = 0;

    for (TalentTreeIdContainer::const_iterator itr = m_TalentTreeIdContainer.begin(); itr != m_TalentTreeIdContainer.end(); ++itr)
    {
        if ((*itr)->playerSpec == sTalentsSpec)
        {
            id = (*itr)->treeId;
            break;
        }
    }

    if (id != 0)
    {
        // Set the treeId as the player's specialization
        player->SetPrimaryTalentTree(player->GetActiveSpec(), id);
    }
    else
    {
        TC_LOG_ERROR("server.worldserver", "Could not find treeId for spec %s", sTalentsSpec.c_str());
    }

    player->SetFreeTalentPoints(0);
    player->SendTalentsInfoData(false);
}

void sTemplateNPC::LearnTemplateGlyphsPvE(Player* player)
{
    for (GlyphContainerPvE::const_iterator itr = m_GlyphContainerPvE.begin(); itr != m_GlyphContainerPvE.end(); ++itr)
    {
        if ((*itr)->playerClass == GetClassString(player).c_str() && (*itr)->playerSpec == sTalentsSpec)
            ApplyGlyph(player, (*itr)->slot, (*itr)->glyph);
    }
    player->SendTalentsInfoData(false);
}

void sTemplateNPC::LearnTemplateGlyphsPvP(Player* player)
{
    for (GlyphContainerPvE::const_iterator itr = m_GlyphContainerPvP.begin(); itr != m_GlyphContainerPvP.end(); ++itr)
    {
        if ((*itr)->playerClass == GetClassString(player).c_str() && (*itr)->playerSpec == sTalentsSpec)
            ApplyGlyph(player, (*itr)->slot, (*itr)->glyph);
    }
    player->SendTalentsInfoData(false);
}

void sTemplateNPC::EquipTemplateGearPvE(Player* player)
{
    if (player->getRace() == RACE_HUMAN)
    {
        for (HumanGearContainerPvE::const_iterator itr = m_HumanGearContainerPvE.begin(); itr != m_HumanGearContainerPvE.end(); ++itr)
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
                ApplyBonus(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, (*itr)->pos), REFORGE_ENCHANTMENT_SLOT, (*itr)->reforgeId);
            }
        }
    }
    else if (player->GetTeam() == ALLIANCE && player->getRace() != RACE_HUMAN)
    {
        for (AllianceGearContainerPvE::const_iterator itr = m_AllianceGearContainerPvE.begin(); itr != m_AllianceGearContainerPvE.end(); ++itr)
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
                ApplyBonus(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, (*itr)->pos), REFORGE_ENCHANTMENT_SLOT, (*itr)->reforgeId);
            }
        }
    }
    else if (player->GetTeam() == HORDE)
    {
        for (HordeGearContainerPvE::const_iterator itr = m_HordeGearContainerPvE.begin(); itr != m_HordeGearContainerPvE.end(); ++itr)
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
                ApplyBonus(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, (*itr)->pos), REFORGE_ENCHANTMENT_SLOT, (*itr)->reforgeId);
            }
        }
    }
}

void sTemplateNPC::EquipTemplateGearPvP(Player* player)
{
    if (player->getRace() == RACE_HUMAN)
    {
        for (HumanGearContainerPvE::const_iterator itr = m_HumanGearContainerPvP.begin(); itr != m_HumanGearContainerPvP.end(); ++itr)
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
                ApplyBonus(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, (*itr)->pos), REFORGE_ENCHANTMENT_SLOT, (*itr)->reforgeId);
            }
        }
    }
    else if (player->GetTeam() == ALLIANCE && player->getRace() != RACE_HUMAN)
    {
        for (AllianceGearContainerPvE::const_iterator itr = m_AllianceGearContainerPvP.begin(); itr != m_AllianceGearContainerPvP.end(); ++itr)
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
                ApplyBonus(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, (*itr)->pos), REFORGE_ENCHANTMENT_SLOT, (*itr)->reforgeId);
            }
        }
    }
    else if (player->GetTeam() == HORDE)
    {
        for (HordeGearContainerPvE::const_iterator itr = m_HordeGearContainerPvP.begin(); itr != m_HordeGearContainerPvP.end(); ++itr)
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
                ApplyBonus(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, (*itr)->pos), REFORGE_ENCHANTMENT_SLOT, (*itr)->reforgeId);
            }
        }
    }
}


void sTemplateNPC::LoadTalentsContainerPvE()
{
    for (TalentContainerPvE::const_iterator itr = m_TalentContainerPvE.begin(); itr != m_TalentContainerPvE.end(); ++itr)
        delete* itr;

    m_TalentContainerPvE.clear();

    uint32 oldMSTime = getMSTime();
    uint32 count = 0;

    QueryResult result = CharacterDatabase.Query("SELECT playerClass, playerSpec, talentId FROM template_npc_talents_pve;");

    if (!result)
    {
        TC_LOG_INFO("server.worldserver", ">>TEMPLATE NPC: Loaded 0 PvE talent templates. DB table `template_npc_talents_pve` is empty!");
        return;
    }

    do
    {
        Field* fields = result->Fetch();

        TalentTemplate* pTalent = new TalentTemplate;

        pTalent->playerClass = fields[0].GetString();
        pTalent->playerSpec = fields[1].GetString();
        pTalent->talentId = fields[2].GetUInt32();
        m_TalentContainerPvE.push_back(pTalent);
        ++count;
    } while (result->NextRow());
    TC_LOG_INFO("server.worldserver", ">>TEMPLATE NPC: Loaded %u PvE talent templates in %u ms.", count, GetMSTimeDiffToNow(oldMSTime));
}

void sTemplateNPC::LoadTalentsContainerPvP()
{
    for (TalentContainerPvP::const_iterator itr = m_TalentContainerPvP.begin(); itr != m_TalentContainerPvP.end(); ++itr)
        delete* itr;

    m_TalentContainerPvP.clear();

    uint32 oldMSTime = getMSTime();
    uint32 count = 0;

    QueryResult result = CharacterDatabase.Query("SELECT playerClass, playerSpec, talentId FROM template_npc_talents_pvp;");

    if (!result)
    {
        TC_LOG_INFO("server.worldserver", ">>TEMPLATE NPC: Loaded 0 PvP talent templates. DB table `template_npc_talents_pvp` is empty!");
        return;
    }

    do
    {
        Field* fields = result->Fetch();

        TalentTemplate* pTalent = new TalentTemplate;

        pTalent->playerClass = fields[0].GetString();
        pTalent->playerSpec = fields[1].GetString();
        pTalent->talentId = fields[2].GetUInt32();

        m_TalentContainerPvP.push_back(pTalent);
        ++count;
    } while (result->NextRow());
    TC_LOG_INFO("server.worldserver", ">>TEMPLATE NPC: Loaded %u PvP talent templates in %u ms.", count, GetMSTimeDiffToNow(oldMSTime));
}

void sTemplateNPC::LoadTalentTreeId()
{
    for (TalentTreeIdContainer::const_iterator itr = m_TalentTreeIdContainer.begin(); itr != m_TalentTreeIdContainer.end(); ++itr)
        delete* itr;

    m_TalentTreeIdContainer.clear();

    uint32 oldMSTime = getMSTime();
    uint32 count = 0;

    QueryResult result = CharacterDatabase.Query("SELECT playerSpec, treeId FROM template_npc_talents_tree_id;");

    if (!result)
    {
        TC_LOG_INFO("server.worldserver", ">>TEMPLATE NPC: Loaded 0 template talent specializations. DB table `template_npc_talents_tree_id` is empty!");
        return;
    }

    do
    {
        Field* fields = result->Fetch();

        TalentTreeId* pTalent = new TalentTreeId;

        pTalent->playerSpec = fields[0].GetString();
        pTalent->treeId = fields[1].GetUInt32();

        m_TalentTreeIdContainer.push_back(pTalent);
        ++count;
    } while (result->NextRow());
    TC_LOG_INFO("server.worldserver", ">>TEMPLATE NPC: Loaded %u template talent specializations in %u ms.", count, GetMSTimeDiffToNow(oldMSTime));
}

void sTemplateNPC::LoadGlyphsContainerPvE()
{
    for (GlyphContainerPvE::const_iterator itr = m_GlyphContainerPvE.begin(); itr != m_GlyphContainerPvE.end(); ++itr)
        delete* itr;

    m_GlyphContainerPvE.clear();

    QueryResult result = CharacterDatabase.Query("SELECT playerClass, playerSpec, slot, glyph FROM template_npc_glyphs_pve;");

    uint32 oldMSTime = getMSTime();
    uint32 count = 0;

    if (!result)
    {
        TC_LOG_INFO("server.worldserver", ">>TEMPLATE NPC: Loaded 0 PvE glyph templates. DB table `template_npc_glyphs_pve` is empty!");
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

        m_GlyphContainerPvE.push_back(pGlyph);
        ++count;
    } while (result->NextRow());
    TC_LOG_INFO("server.worldserver", ">>TEMPLATE NPC: Loaded %u PvE glyph templates in %u ms.", count, GetMSTimeDiffToNow(oldMSTime));
}

void sTemplateNPC::LoadGlyphsContainerPvP()
{
    for (GlyphContainerPvP::const_iterator itr = m_GlyphContainerPvP.begin(); itr != m_GlyphContainerPvP.end(); ++itr)
        delete* itr;

    m_GlyphContainerPvP.clear();

    QueryResult result = CharacterDatabase.Query("SELECT playerClass, playerSpec, slot, glyph FROM template_npc_glyphs_pvp;");

    uint32 oldMSTime = getMSTime();
    uint32 count = 0;

    if (!result)
    {
        TC_LOG_INFO("server.worldserver", ">>TEMPLATE NPC: Loaded 0 PvP glyph templates. DB table `template_npc_glyphs_pvp` is empty!");
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

        m_GlyphContainerPvP.push_back(pGlyph);
        ++count;
    } while (result->NextRow());
    TC_LOG_INFO("server.worldserver", ">>TEMPLATE NPC: Loaded %u PvP glyph templates in %u ms.", count, GetMSTimeDiffToNow(oldMSTime));
}

void sTemplateNPC::LoadHumanGearContainerPvE()
{
    for (HumanGearContainerPvE::const_iterator itr = m_HumanGearContainerPvE.begin(); itr != m_HumanGearContainerPvE.end(); ++itr)
        delete* itr;

    m_HumanGearContainerPvE.clear();

    QueryResult result = CharacterDatabase.Query("SELECT playerClass, playerSpec, pos, itemEntry, enchant, socket1, socket2, socket3, bonusEnchant, prismaticEnchant, reforgeId FROM template_npc_human_pve;");

    uint32 oldMSTime = getMSTime();
    uint32 count = 0;

    if (!result)
    {
        TC_LOG_INFO("server.worldserver", ">>TEMPLATE NPC: Loaded 0 PvE 'gear templates. DB table `template_npc_human_pve` is empty!");
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

        m_HumanGearContainerPvE.push_back(pItem);
        ++count;
    } while (result->NextRow());
    TC_LOG_INFO("server.worldserver", ">>TEMPLATE NPC: Loaded %u PvE gear templates for Human in %u ms.", count, GetMSTimeDiffToNow(oldMSTime));
}

void sTemplateNPC::LoadAllianceGearContainerPvE()
{
    for (AllianceGearContainerPvE::const_iterator itr = m_AllianceGearContainerPvE.begin(); itr != m_AllianceGearContainerPvE.end(); ++itr)
        delete* itr;

    m_AllianceGearContainerPvE.clear();

    QueryResult result = CharacterDatabase.Query("SELECT playerClass, playerSpec, pos, itemEntry, enchant, socket1, socket2, socket3, bonusEnchant, prismaticEnchant, reforgeId FROM template_npc_alliance_pve;");

    uint32 oldMSTime = getMSTime();
    uint32 count = 0;

    if (!result)
    {
        TC_LOG_INFO("server.worldserver", ">>TEMPLATE NPC: Loaded 0 PvE 'gear templates. DB table `template_npc_alliance_pve` is empty!");
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

        m_AllianceGearContainerPvE.push_back(pItem);
        ++count;
    } while (result->NextRow());
    TC_LOG_INFO("server.worldserver", ">>TEMPLATE NPC: Loaded %u PvE gear templates for Alliance in %u ms.", count, GetMSTimeDiffToNow(oldMSTime));
}

void sTemplateNPC::LoadHordeGearContainerPvE()
{
    for (HordeGearContainerPvE::const_iterator itr = m_HordeGearContainerPvE.begin(); itr != m_HordeGearContainerPvE.end(); ++itr)
        delete* itr;

    m_HordeGearContainerPvE.clear();

    QueryResult result = CharacterDatabase.Query("SELECT playerClass, playerSpec, pos, itemEntry, enchant, socket1, socket2, socket3, bonusEnchant, prismaticEnchant, reforgeId FROM template_npc_horde_pve;");

    uint32 oldMSTime = getMSTime();
    uint32 count = 0;

    if (!result)
    {
        TC_LOG_INFO("server.worldserver", ">>TEMPLATE NPC: Loaded 0 PvE 'gear templates. DB table `template_npc_horde_pve` is empty!");
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

        m_HordeGearContainerPvE.push_back(pItem);
        ++count;
    } while (result->NextRow());
    TC_LOG_INFO("server.worldserver", ">>TEMPLATE NPC: Loaded %u PvE gear templates for Horde in %u ms.", count, GetMSTimeDiffToNow(oldMSTime));
}

void sTemplateNPC::LoadHumanGearContainerPvP()
{
    for (HumanGearContainerPvP::const_iterator itr = m_HumanGearContainerPvP.begin(); itr != m_HumanGearContainerPvP.end(); ++itr)
        delete* itr;

    m_HumanGearContainerPvP.clear();

    QueryResult result = CharacterDatabase.Query("SELECT playerClass, playerSpec, pos, itemEntry, enchant, socket1, socket2, socket3, bonusEnchant, prismaticEnchant, reforgeId FROM template_npc_human_pvp;");

    uint32 oldMSTime = getMSTime();
    uint32 count = 0;

    if (!result)
    {
        TC_LOG_INFO("server.worldserver", ">>TEMPLATE NPC: Loaded 0 PvP gear templates. DB table `template_npc_human_pvp` is empty!");
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
        pItem->reforgeId = fields[10].GetUInt32();

        m_HumanGearContainerPvP.push_back(pItem);
        ++count;
    } while (result->NextRow());
    TC_LOG_INFO("server.worldserver", ">>TEMPLATE NPC: Loaded %u PvP gear templates for Human in %u ms.", count, GetMSTimeDiffToNow(oldMSTime));
}

void sTemplateNPC::LoadAllianceGearContainerPvP()
{
    for (AllianceGearContainerPvP::const_iterator itr = m_AllianceGearContainerPvP.begin(); itr != m_AllianceGearContainerPvP.end(); ++itr)
        delete* itr;

    m_AllianceGearContainerPvP.clear();

    QueryResult result = CharacterDatabase.Query("SELECT playerClass, playerSpec, pos, itemEntry, enchant, socket1, socket2, socket3, bonusEnchant, prismaticEnchant, reforgeId FROM template_npc_alliance_pvp;");

    uint32 oldMSTime = getMSTime();
    uint32 count = 0;

    if (!result)
    {
        TC_LOG_INFO("server.worldserver", ">>TEMPLATE NPC: Loaded 0 PvP gear templates. DB table `template_npc_alliance_pvp` is empty!");
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

        m_AllianceGearContainerPvP.push_back(pItem);
        ++count;
    } while (result->NextRow());
    TC_LOG_INFO("server.worldserver", ">>TEMPLATE NPC: Loaded %u PvP gear templates for Alliance in %u ms.", count, GetMSTimeDiffToNow(oldMSTime));
}

void sTemplateNPC::LoadHordeGearContainerPvP()
{
    for (HordeGearContainerPvP::const_iterator itr = m_HordeGearContainerPvP.begin(); itr != m_HordeGearContainerPvP.end(); ++itr)
        delete* itr;

    m_HordeGearContainerPvP.clear();

    QueryResult result = CharacterDatabase.Query("SELECT playerClass, playerSpec, pos, itemEntry, enchant, socket1, socket2, socket3, bonusEnchant, prismaticEnchant, reforgeId FROM template_npc_horde_pvp;");

    uint32 oldMSTime = getMSTime();
    uint32 count = 0;

    if (!result)
    {
        TC_LOG_INFO("server.worldserver", ">>TEMPLATE NPC: Loaded 0 PvP gear templates. DB table `template_npc_horde_pve` is empty!");
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

        m_HordeGearContainerPvP.push_back(pItem);
        ++count;
    } while (result->NextRow());
    TC_LOG_INFO("server.worldserver", ">>TEMPLATE NPC: Loaded %u PvP gear templates for Horde in %u ms.", count, GetMSTimeDiffToNow(oldMSTime));
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
        CharacterDatabase.PExecute("DELETE FROM template_npc_human_pve WHERE playerClass = '%s' AND playerSpec = '%s';", GetClassString(player).c_str(), playerSpecStr.c_str());
        player->GetSession()->SendAreaTriggerMessage("Template successfuly created!");
        return false;
    }
    else if (player->GetTeam() == ALLIANCE && player->getRace() != RACE_HUMAN)
    {
        CharacterDatabase.PExecute("DELETE FROM template_npc_alliance_pve WHERE playerClass = '%s' AND playerSpec = '%s';", GetClassString(player).c_str(), playerSpecStr.c_str());
        player->GetSession()->SendAreaTriggerMessage("Template successfuly created!");
        return false;
    }
    else if (player->GetTeam() == HORDE)
    {                                                                                                        // ????????????? sTemplateNpcMgr here??
        CharacterDatabase.PExecute("DELETE FROM template_npc_horde_pve WHERE playerClass = '%s' AND playerSpec = '%s';", GetClassString(player).c_str(), playerSpecStr.c_str());
        player->GetSession()->SendAreaTriggerMessage("Template successfuly created!");
        return false;
    }
    return true;
}

void sTemplateNPC::ExtractGearTemplateToDBPvE(Player* player, std::string& playerSpecStr)
{
    TC_LOG_INFO("server.worldserver", ">>TEMPLATE NPC: Extracting gear template for %s %s...", GetClassString(player).c_str(), playerSpecStr.c_str());
    CharacterDatabase.PExecute("DELETE FROM template_npc_human_pve WHERE playerClass = '%s' AND playerSpec = '%s';", GetClassString(player).c_str(), playerSpecStr.c_str());
    CharacterDatabase.PExecute("DELETE FROM template_npc_alliance_pve WHERE playerClass = '%s' AND playerSpec = '%s';", GetClassString(player).c_str(), playerSpecStr.c_str());
    CharacterDatabase.PExecute("DELETE FROM template_npc_horde_pve WHERE playerClass = '%s' AND playerSpec = '%s';", GetClassString(player).c_str(), playerSpecStr.c_str());

    for (uint8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
    {
        Item* equippedItem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i);

        if (equippedItem)
        {
            CharacterDatabase.PExecute("INSERT INTO template_npc_human_pve (`playerClass`, `playerSpec`, `pos`, `itemEntry`, `enchant`, `socket1`, `socket2`, `socket3`, `bonusEnchant`, `prismaticEnchant`) VALUES ('%s', '%s', '%u', '%u', '%u', '%u', '%u', '%u', '%u', '%u');"
                , GetClassString(player).c_str(), playerSpecStr.c_str(), equippedItem->GetSlot(), equippedItem->GetEntry(), equippedItem->GetEnchantmentId(PERM_ENCHANTMENT_SLOT),
                equippedItem->GetEnchantmentId(SOCK_ENCHANTMENT_SLOT), equippedItem->GetEnchantmentId(SOCK_ENCHANTMENT_SLOT_2), equippedItem->GetEnchantmentId(SOCK_ENCHANTMENT_SLOT_3),
                equippedItem->GetEnchantmentId(BONUS_ENCHANTMENT_SLOT), equippedItem->GetEnchantmentId(PRISMATIC_ENCHANTMENT_SLOT));
            CharacterDatabase.PExecute("INSERT INTO template_npc_alliance_pve (`playerClass`, `playerSpec`, `pos`, `itemEntry`, `enchant`, `socket1`, `socket2`, `socket3`, `bonusEnchant`, `prismaticEnchant`) VALUES ('%s', '%s', '%u', '%u', '%u', '%u', '%u', '%u', '%u', '%u');"
                , GetClassString(player).c_str(), playerSpecStr.c_str(), equippedItem->GetSlot(), equippedItem->GetEntry(), equippedItem->GetEnchantmentId(PERM_ENCHANTMENT_SLOT),
                equippedItem->GetEnchantmentId(SOCK_ENCHANTMENT_SLOT), equippedItem->GetEnchantmentId(SOCK_ENCHANTMENT_SLOT_2), equippedItem->GetEnchantmentId(SOCK_ENCHANTMENT_SLOT_3),
                equippedItem->GetEnchantmentId(BONUS_ENCHANTMENT_SLOT), equippedItem->GetEnchantmentId(PRISMATIC_ENCHANTMENT_SLOT));
            CharacterDatabase.PExecute("INSERT INTO template_npc_horde_pve (`playerClass`, `playerSpec`, `pos`, `itemEntry`, `enchant`, `socket1`, `socket2`, `socket3`, `bonusEnchant`, `prismaticEnchant`) VALUES ('%s', '%s', '%u', '%u', '%u', '%u', '%u', '%u', '%u', '%u');"
                , GetClassString(player).c_str(), playerSpecStr.c_str(), equippedItem->GetSlot(), equippedItem->GetEntry(), equippedItem->GetEnchantmentId(PERM_ENCHANTMENT_SLOT),
                equippedItem->GetEnchantmentId(SOCK_ENCHANTMENT_SLOT), equippedItem->GetEnchantmentId(SOCK_ENCHANTMENT_SLOT_2), equippedItem->GetEnchantmentId(SOCK_ENCHANTMENT_SLOT_3),
                equippedItem->GetEnchantmentId(BONUS_ENCHANTMENT_SLOT), equippedItem->GetEnchantmentId(PRISMATIC_ENCHANTMENT_SLOT));
        }
    }
}

void sTemplateNPC::ExtractGearTemplateToDBHumanPvP(Player* player, std::string& playerSpecStr)
{
    TC_LOG_INFO("server.worldserver", ">>TEMPLATE NPC: Extracting gear template for %s %s...", GetClassString(player).c_str(), playerSpecStr.c_str());
    CharacterDatabase.PExecute("DELETE FROM template_npc_human_pvp WHERE playerClass = '%s' AND playerSpec = '%s';", GetClassString(player).c_str(), playerSpecStr.c_str());

    for (uint8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
    {
        Item* equippedItem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i);

        if (equippedItem)
        {
            CharacterDatabase.PExecute("INSERT INTO template_npc_human_pvp (`playerClass`, `playerSpec`, `pos`, `itemEntry`, `enchant`, `socket1`, `socket2`, `socket3`, `bonusEnchant`, `prismaticEnchant`) VALUES ('%s', '%s', '%u', '%u', '%u', '%u', '%u', '%u', '%u', '%u');"
                , GetClassString(player).c_str(), playerSpecStr.c_str(), equippedItem->GetSlot(), equippedItem->GetEntry(), equippedItem->GetEnchantmentId(PERM_ENCHANTMENT_SLOT),
                equippedItem->GetEnchantmentId(SOCK_ENCHANTMENT_SLOT), equippedItem->GetEnchantmentId(SOCK_ENCHANTMENT_SLOT_2), equippedItem->GetEnchantmentId(SOCK_ENCHANTMENT_SLOT_3),
                equippedItem->GetEnchantmentId(BONUS_ENCHANTMENT_SLOT), equippedItem->GetEnchantmentId(PRISMATIC_ENCHANTMENT_SLOT));
        }
    }
}

void sTemplateNPC::ExtractGearTemplateToDBAlliancePvP(Player* player, std::string& playerSpecStr)
{
    TC_LOG_INFO("server.worldserver", ">>TEMPLATE NPC: Extracting gear template for %s %s...", GetClassString(player).c_str(), playerSpecStr.c_str());
    CharacterDatabase.PExecute("DELETE FROM template_npc_alliance_pvp WHERE playerClass = '%s' AND playerSpec = '%s';", GetClassString(player).c_str(), playerSpecStr.c_str());

    for (uint8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
    {
        Item* equippedItem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i);

        if (equippedItem)
        {
            CharacterDatabase.PExecute("INSERT INTO template_npc_alliance_pvp (`playerClass`, `playerSpec`, `pos`, `itemEntry`, `enchant`, `socket1`, `socket2`, `socket3`, `bonusEnchant`, `prismaticEnchant`) VALUES ('%s', '%s', '%u', '%u', '%u', '%u', '%u', '%u', '%u', '%u');"
                , GetClassString(player).c_str(), playerSpecStr.c_str(), equippedItem->GetSlot(), equippedItem->GetEntry(), equippedItem->GetEnchantmentId(PERM_ENCHANTMENT_SLOT),
                equippedItem->GetEnchantmentId(SOCK_ENCHANTMENT_SLOT), equippedItem->GetEnchantmentId(SOCK_ENCHANTMENT_SLOT_2), equippedItem->GetEnchantmentId(SOCK_ENCHANTMENT_SLOT_3),
                equippedItem->GetEnchantmentId(BONUS_ENCHANTMENT_SLOT), equippedItem->GetEnchantmentId(PRISMATIC_ENCHANTMENT_SLOT));
        }
    }
}

void sTemplateNPC::ExtractGearTemplateToDBHordePvP(Player* player, std::string& playerSpecStr)
{
    TC_LOG_INFO("server.worldserver", ">>TEMPLATE NPC: Extracting gear template for %s %s...", GetClassString(player).c_str(), playerSpecStr.c_str());
    CharacterDatabase.PExecute("DELETE FROM template_npc_horde_pvp WHERE playerClass = '%s' AND playerSpec = '%s';", GetClassString(player).c_str(), playerSpecStr.c_str());

    for (uint8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
    {
        Item* equippedItem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i);

        if (equippedItem)
        {
            CharacterDatabase.PExecute("INSERT INTO template_npc_horde_pvp (`playerClass`, `playerSpec`, `pos`, `itemEntry`, `enchant`, `socket1`, `socket2`, `socket3`, `bonusEnchant`, `prismaticEnchant`) VALUES ('%s', '%s', '%u', '%u', '%u', '%u', '%u', '%u', '%u', '%u');"
                , GetClassString(player).c_str(), playerSpecStr.c_str(), equippedItem->GetSlot(), equippedItem->GetEntry(), equippedItem->GetEnchantmentId(PERM_ENCHANTMENT_SLOT),
                equippedItem->GetEnchantmentId(SOCK_ENCHANTMENT_SLOT), equippedItem->GetEnchantmentId(SOCK_ENCHANTMENT_SLOT_2), equippedItem->GetEnchantmentId(SOCK_ENCHANTMENT_SLOT_3),
                equippedItem->GetEnchantmentId(BONUS_ENCHANTMENT_SLOT), equippedItem->GetEnchantmentId(PRISMATIC_ENCHANTMENT_SLOT));
        }
    }
}

void sTemplateNPC::ExtractTalentTemplateToDBPvE(Player* player, std::string& playerSpecStr)
{
    CharacterDatabase.PExecute("DELETE FROM template_npc_talents_pve WHERE playerClass = '%s' AND playerSpec = '%s';", GetClassString(player).c_str(), playerSpecStr.c_str());
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

            CharacterDatabase.PExecute("INSERT INTO template_npc_talents_pve (playerClass, playerSpec, talentId) "
                "VALUES ('%s', '%s', '%u');", GetClassString(player).c_str(), playerSpecStr.c_str(), spell);
        } while (result->NextRow());
    }
}

void sTemplateNPC::ExtractTalentTemplateToDBPvP(Player* player, std::string& playerSpecStr)
{
    CharacterDatabase.PExecute("DELETE FROM template_npc_talents_pvp WHERE playerClass = '%s' AND playerSpec = '%s';", GetClassString(player).c_str(), playerSpecStr.c_str());
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

            CharacterDatabase.PExecute("INSERT INTO template_npc_talents_pvp (playerClass, playerSpec, talentId) "
                "VALUES ('%s', '%s', '%u');", GetClassString(player).c_str(), playerSpecStr.c_str(), spell);
        } while (result->NextRow());
    }
}

void sTemplateNPC::ExtractTalentTreeId(Player* player, std::string& playerSpecStr)
{
    TC_LOG_INFO("server.worldserver", ">>TEMPLATE NPC: Extracting gear template for %s %s...", GetClassString(player).c_str(), playerSpecStr.c_str());
    CharacterDatabase.PExecute("DELETE FROM template_npc_talents_tree_id WHERE playerSpec = '%s';", playerSpecStr.c_str());


    CharacterDatabase.PExecute("INSERT INTO template_npc_talents_tree_id (`playerSpec`, `treeId`) VALUES ('%s', '%u');", playerSpecStr.c_str(), player->GetPrimaryTalentTree(player->GetActiveSpec()));
}

void sTemplateNPC::ExtractGlyphsTemplateToDBPvE(Player* player, std::string& playerSpecStr)
{
    CharacterDatabase.PExecute("DELETE FROM template_npc_glyphs_pve WHERE playerClass = '%s' AND playerSpec = '%s';", GetClassString(player).c_str(), playerSpecStr.c_str());

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

void sTemplateNPC::ExtractGlyphsTemplateToDBPvP(Player* player, std::string& playerSpecStr)
{
    CharacterDatabase.PExecute("DELETE FROM template_npc_glyphs_pvp WHERE playerClass = '%s' AND playerSpec = '%s';", GetClassString(player).c_str(), playerSpecStr.c_str());

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

bool sTemplateNPC::CanEquipTemplatePvE(Player* player, std::string& playerSpecStr)
{
    if (player->getRace() == RACE_HUMAN)
    {
        QueryResult result = CharacterDatabase.PQuery("SELECT playerClass, playerSpec FROM template_npc_human_pve "
            "WHERE playerClass = '%s' AND playerSpec = '%s';", GetClassString(player).c_str(), playerSpecStr.c_str());

        if (!result)
            return false;
    }
    else if (player->GetTeam() == ALLIANCE && player->getRace() != RACE_HUMAN)
    {
        QueryResult result = CharacterDatabase.PQuery("SELECT playerClass, playerSpec FROM template_npc_alliance_pve "
            "WHERE playerClass = '%s' AND playerSpec = '%s';", GetClassString(player).c_str(), playerSpecStr.c_str());

        if (!result)
            return false;
    }
    else if (player->GetTeam() == HORDE)
    {
        QueryResult result = CharacterDatabase.PQuery("SELECT playerClass, playerSpec FROM template_npc_horde_pve "
            "WHERE playerClass = '%s' AND playerSpec = '%s';", GetClassString(player).c_str(), playerSpecStr.c_str());

        if (!result)
            return false;
    }
    return true;
}

bool sTemplateNPC::CanEquipTemplatePvP(Player* player, std::string& playerSpecStr)
{
    if (player->getRace() == RACE_HUMAN)
    {
        QueryResult result = CharacterDatabase.PQuery("SELECT playerClass, playerSpec FROM template_npc_human_pvp "
            "WHERE playerClass = '%s' AND playerSpec = '%s';", GetClassString(player).c_str(), playerSpecStr.c_str());

        if (!result)
            return false;
    }
    else if (player->GetTeam() == ALLIANCE && player->getRace() != RACE_HUMAN)
    {
        QueryResult result = CharacterDatabase.PQuery("SELECT playerClass, playerSpec FROM template_npc_alliance_pvp "
            "WHERE playerClass = '%s' AND playerSpec = '%s';", GetClassString(player).c_str(), playerSpecStr.c_str());

        if (!result)
            return false;
    }
    else if (player->GetTeam() == HORDE)
    {
        QueryResult result = CharacterDatabase.PQuery("SELECT playerClass, playerSpec FROM template_npc_horde_pvp "
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
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Reset Talents.", GOSSIP_SENDER_MAIN, GOSSIP_RESET_TALENTS);
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
            else if (action == GOSSIP_RESET_TALENTS)
            {
                TC_LOG_INFO("server.worldserver", ">>TEMPLATE NPC: GOSSIP_RESET_TALENTS selected.");
                player->ResetTalents(true);
                player->SendTalentsInfoData(false);
                CloseGossipMenuFor(player);
            }
            
            switch (action)
            {
            // PVP
            case GOSSIP_PVP_GEAR + 1:
                sTemplateNpcMgr->sTalentsSpec = "Arms-Warrior-PvP";
                EquipFullTemplateGearPvP(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVP_GEAR + 2:
                sTemplateNpcMgr->sTalentsSpec = "Fury-Warrior-PvP";
                EquipFullTemplateGearPvP(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVP_GEAR + 3:
                sTemplateNpcMgr->sTalentsSpec = "Protection-Warrior-PvP";
                EquipFullTemplateGearPvP(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVP_GEAR + 4:
                sTemplateNpcMgr->sTalentsSpec = "Assassination-Rogue-PvP";
                EquipFullTemplateGearPvP(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVP_GEAR + 5:
                sTemplateNpcMgr->sTalentsSpec = "Combat-Rogue-PvP";
                EquipFullTemplateGearPvP(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVP_GEAR + 6:
                sTemplateNpcMgr->sTalentsSpec = "Subtlety-Rogue-PvP";
                EquipFullTemplateGearPvP(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVP_GEAR + 7:
                sTemplateNpcMgr->sTalentsSpec = "Elemental-Shaman-PvP";
                EquipFullTemplateGearPvP(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVP_GEAR + 8:
                sTemplateNpcMgr->sTalentsSpec = "Enhancement-Shaman-PvP";
                EquipFullTemplateGearPvP(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVP_GEAR + 9:
                sTemplateNpcMgr->sTalentsSpec = "Restoration-Shaman-PvE";
                EquipFullTemplateGearPvP(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVP_GEAR + 10:
                sTemplateNpcMgr->sTalentsSpec = "Beast-Mastery-Hunter-PvP";
                EquipFullTemplateGearPvP(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVP_GEAR + 11:
                sTemplateNpcMgr->sTalentsSpec = "Marksmanship-Hunter-PvP";
                EquipFullTemplateGearPvP(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVP_GEAR + 12:
                sTemplateNpcMgr->sTalentsSpec = "Survival-Hunter-PvP";
                EquipFullTemplateGearPvP(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVP_GEAR + 13:
                sTemplateNpcMgr->sTalentsSpec = "Arcane-Mage-PvP";
                EquipFullTemplateGearPvP(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVP_GEAR + 14:
                sTemplateNpcMgr->sTalentsSpec = "Fire-Mage-PvP";
                EquipFullTemplateGearPvP(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVP_GEAR + 15:
                sTemplateNpcMgr->sTalentsSpec = "Frost-Mage-PvP";
                EquipFullTemplateGearPvP(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVP_GEAR + 16:
                sTemplateNpcMgr->sTalentsSpec = "Affliction-Warlock-PvP";
                EquipFullTemplateGearPvP(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVP_GEAR + 17:
                sTemplateNpcMgr->sTalentsSpec = "Demonology-Warlock-PvP";
                EquipFullTemplateGearPvP(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVP_GEAR + 18:
                sTemplateNpcMgr->sTalentsSpec = "Destruction-Warlock-PvP";
                EquipFullTemplateGearPvP(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVP_GEAR + 19:
                sTemplateNpcMgr->sTalentsSpec = "Holy-Paladin-PvP";
                EquipFullTemplateGearPvP(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVP_GEAR + 20:
                sTemplateNpcMgr->sTalentsSpec = "Protection-Paladin-PvP";
                EquipFullTemplateGearPvP(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVP_GEAR + 21:
                sTemplateNpcMgr->sTalentsSpec = "Retribution-Paladin-PvP";
                EquipFullTemplateGearPvP(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVP_GEAR + 22:
                sTemplateNpcMgr->sTalentsSpec = "Balance-Druid-PvP";
                EquipFullTemplateGearPvP(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVP_GEAR + 23:
                sTemplateNpcMgr->sTalentsSpec = "Feral-Druid-PvP";
                EquipFullTemplateGearPvP(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVP_GEAR + 31:
                sTemplateNpcMgr->sTalentsSpec = "Guardian-Druid-PvP";
                EquipFullTemplateGearPvP(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVP_GEAR + 24:
                sTemplateNpcMgr->sTalentsSpec = "Restoration-Druid-PvP";
                EquipFullTemplateGearPvP(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVP_GEAR + 25:
                sTemplateNpcMgr->sTalentsSpec = "Blood-Death-Knight-PvP";
                EquipFullTemplateGearPvP(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVP_GEAR + 26:
                sTemplateNpcMgr->sTalentsSpec = "Frost-Death-Knight-PvP";
                EquipFullTemplateGearPvP(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVP_GEAR + 27:
                sTemplateNpcMgr->sTalentsSpec = "Unholy-Death-Knight-PvP";
                EquipFullTemplateGearPvP(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVP_GEAR + 28:
                sTemplateNpcMgr->sTalentsSpec = "Discipline-Priest-PvP";
                EquipFullTemplateGearPvP(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVP_GEAR + 29:
                sTemplateNpcMgr->sTalentsSpec = "Holy-Priest-PvP";
                EquipFullTemplateGearPvP(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVP_GEAR + 30:
                sTemplateNpcMgr->sTalentsSpec = "Shadow-Priest-PvP";
                EquipFullTemplateGearPvP(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;

            // PVE
            case GOSSIP_PVE_GEAR + 1:
                sTemplateNpcMgr->sTalentsSpec = "Arms-Warrior-PvE";
                EquipFullTemplateGearPvE(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVE_GEAR + 2:
                sTemplateNpcMgr->sTalentsSpec = "Fury-Warrior-PvE-2H";
                EquipFullTemplateGearPvE(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVE_GEAR + 3:
                sTemplateNpcMgr->sTalentsSpec = "Fury-Warrior-PvE-1H";
                EquipFullTemplateGearPvE(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVE_GEAR + 4:
                sTemplateNpcMgr->sTalentsSpec = "Protection-Warrior-PvE";
                EquipFullTemplateGearPvE(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVE_GEAR + 5:
                sTemplateNpcMgr->sTalentsSpec = "Assassination-Rogue-PvE";
                EquipFullTemplateGearPvE(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVE_GEAR + 6:
                sTemplateNpcMgr->sTalentsSpec = "Combat-Rogue-PvE";
                EquipFullTemplateGearPvE(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVE_GEAR + 7:
                sTemplateNpcMgr->sTalentsSpec = "Subtlety-Rogue-PvE";
                EquipFullTemplateGearPvE(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVE_GEAR + 8:
                sTemplateNpcMgr->sTalentsSpec = "Elemental-Shaman-PvE";
                EquipFullTemplateGearPvE(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVE_GEAR + 9:
                sTemplateNpcMgr->sTalentsSpec = "Enhancement-Shaman-PvE";
                EquipFullTemplateGearPvE(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVE_GEAR + 10:
                sTemplateNpcMgr->sTalentsSpec = "Restoration-Shaman-PvE";
                EquipFullTemplateGearPvE(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVE_GEAR + 11:
                sTemplateNpcMgr->sTalentsSpec = "Beast-Mastery-Hunter-PvE";
                EquipFullTemplateGearPvE(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVE_GEAR + 12:
                sTemplateNpcMgr->sTalentsSpec = "Marksmanship-Hunter-PvE";
                EquipFullTemplateGearPvE(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVE_GEAR + 13:
                sTemplateNpcMgr->sTalentsSpec = "Survival-Hunter-PvE";
                EquipFullTemplateGearPvE(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVE_GEAR + 14:
                sTemplateNpcMgr->sTalentsSpec = "Arcane-Mage-PvE";
                EquipFullTemplateGearPvE(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVE_GEAR + 15:
                sTemplateNpcMgr->sTalentsSpec = "Fire-Mage-PvE";
                EquipFullTemplateGearPvE(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVE_GEAR + 16:
                sTemplateNpcMgr->sTalentsSpec = "Frost-Mage-PvE";
                EquipFullTemplateGearPvE(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVE_GEAR + 17:
                sTemplateNpcMgr->sTalentsSpec = "Affliction-Warlock-PvE";
                EquipFullTemplateGearPvE(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVE_GEAR + 18:
                sTemplateNpcMgr->sTalentsSpec = "Demonology-Warlock-PvE";
                EquipFullTemplateGearPvE(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVE_GEAR + 19:
                sTemplateNpcMgr->sTalentsSpec = "Destruction-Warlock-PvE";
                EquipFullTemplateGearPvE(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVE_GEAR + 20:
                sTemplateNpcMgr->sTalentsSpec = "Holy-Paladin-PvE";
                EquipFullTemplateGearPvE(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVE_GEAR + 21:
                sTemplateNpcMgr->sTalentsSpec = "Protection-Paladin-PvE";
                EquipFullTemplateGearPvE(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVE_GEAR + 22:
                sTemplateNpcMgr->sTalentsSpec = "Retribution-Paladin-PvE";
                EquipFullTemplateGearPvE(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVE_GEAR + 23:
                sTemplateNpcMgr->sTalentsSpec = "Balance-Druid-PvE";
                EquipFullTemplateGearPvE(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVE_GEAR + 24:
                sTemplateNpcMgr->sTalentsSpec = "Feral-Druid-PvE";
                EquipFullTemplateGearPvE(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVE_GEAR + 32:
                sTemplateNpcMgr->sTalentsSpec = "Guardian-Druid-PvE";
                EquipFullTemplateGearPvE(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVE_GEAR + 25:
                sTemplateNpcMgr->sTalentsSpec = "Restoration-Druid-PvE";
                EquipFullTemplateGearPvE(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVE_GEAR + 26:
                sTemplateNpcMgr->sTalentsSpec = "Blood-Death-Knight-PvE";
                EquipFullTemplateGearPvE(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVE_GEAR + 27:
                sTemplateNpcMgr->sTalentsSpec = "Frost-Death-Knight-PvE";
                EquipFullTemplateGearPvE(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVE_GEAR + 28:
                sTemplateNpcMgr->sTalentsSpec = "Unholy-Death-Knight-PvE";
                EquipFullTemplateGearPvE(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVE_GEAR + 29:
                sTemplateNpcMgr->sTalentsSpec = "Discipline-Priest-PvE";
                EquipFullTemplateGearPvE(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVE_GEAR + 30:
                sTemplateNpcMgr->sTalentsSpec = "Holy-Priest-PvE";
                EquipFullTemplateGearPvE(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            case GOSSIP_PVE_GEAR + 31:
                sTemplateNpcMgr->sTalentsSpec = "Shadow-Priest-PvE";
                EquipFullTemplateGearPvE(player, sTemplateNpcMgr->sTalentsSpec);
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
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|cff00ff00|TInterface\\icons\\Spell_Shadow_DeathCoil:40|t|r |cffff0000PvP|r / Affliction", GOSSIP_SENDER_MAIN, GOSSIP_PVP_GEAR + 16);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|cff00ff00|TInterface\\icons\\Spell_Shadow_Metamorphosis:40|t|r |cffff0000PvP|r / Demonology", GOSSIP_SENDER_MAIN, GOSSIP_PVP_GEAR + 17);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|cff00ff00|TInterface\\icons\\Spell_Shadow_RainOfFire:40|t|r |cffff0000PvP|r / Destruction", GOSSIP_SENDER_MAIN, GOSSIP_PVP_GEAR + 18);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Back..", GOSSIP_SENDER_MAIN, GOSSIP_MAIN_MENU);
                break;
            case CLASS_PALADIN:
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|cff00ff00|TInterface\\icons\\Spell_Holy_HolyBolt:40|t|r |cffff0000PvP|r / Holy", GOSSIP_SENDER_MAIN, GOSSIP_PVP_GEAR + 19);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|cff00ff00|TInterface\\icons\\Ability_Paladin_ShieldoftheTemplar:40|t|r |cffff0000PvP|r / Protection", GOSSIP_SENDER_MAIN, GOSSIP_PVP_GEAR + 20);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|cff00ff00|TInterface\\icons\\Spell_Holy_AuraOfLight:40|t|r |cffff0000PvP|r / Retribution", GOSSIP_SENDER_MAIN, GOSSIP_PVP_GEAR + 21);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Back..", GOSSIP_SENDER_MAIN, GOSSIP_MAIN_MENU);
                break;
            case CLASS_DRUID:
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|cff00ff00|TInterface\\icons\\Spell_Nature_StarFall:40|t|r |cffff0000PvP|r / Balance", GOSSIP_SENDER_MAIN, GOSSIP_PVP_GEAR + 22);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|cff00ff00|TInterface\\icons\\Ability_Racial_BearForm:40|t|r |cffff0000PvP|r / Feral", GOSSIP_SENDER_MAIN, GOSSIP_PVP_GEAR + 23);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|cff00ff00|TInterface\\icons\\Ability_Racial_BearForm:40|t|r |cffff0000PvP|r / Guardian", GOSSIP_SENDER_MAIN, GOSSIP_PVP_GEAR + 23);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|cff00ff00|TInterface\\icons\\Spell_Nature_HealingTouch:40|t|r |cffff0000PvP|r / Restoration", GOSSIP_SENDER_MAIN, GOSSIP_PVP_GEAR + 24);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Back..", GOSSIP_SENDER_MAIN, GOSSIP_MAIN_MENU);
                break;
            case CLASS_DEATH_KNIGHT:
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|cff00ff00|TInterface\\icons\\Spell_Deathknight_BloodPresence:40|t|r |cffff0000PvP|r / Blood", GOSSIP_SENDER_MAIN, GOSSIP_PVP_GEAR + 25);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|cff00ff00|TInterface\\icons\\Spell_Deathknight_FrostPresence:40|t|r |cffff0000PvP|r / Frost", GOSSIP_SENDER_MAIN, GOSSIP_PVP_GEAR + 26);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|cff00ff00|TInterface\\icons\\Spell_Deathknight_UnholyPresence:40|t|r |cffff0000PvP|r / Unholy", GOSSIP_SENDER_MAIN, GOSSIP_PVP_GEAR + 27);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Back..", GOSSIP_SENDER_MAIN, GOSSIP_MAIN_MENU);
                break;
            case CLASS_PRIEST:
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|cff00ff00|TInterface\\icons\\Spell_Holy_PowerWordShield:40|t|r |cffff0000PvP|r / Discipline", GOSSIP_SENDER_MAIN, GOSSIP_PVP_GEAR + 28);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|cff00ff00|TInterface\\icons\\Spell_Holy_GuardianSpirit:40|t|r |cffff0000PvP|r / Holy", GOSSIP_SENDER_MAIN, GOSSIP_PVP_GEAR + 29);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|cff00ff00|TInterface\\icons\\Spell_Shadow_ShadowWordPain:40|t|r |cffff0000PvP|r / Shadow", GOSSIP_SENDER_MAIN, GOSSIP_PVP_GEAR + 30);
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
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|cff00ff00|TInterface\\icons\\Spell_Holy_MagicalSentry:40|t|r |cff00ff00PvE|r / Arcane", GOSSIP_SENDER_MAIN, GOSSIP_PVE_GEAR + 14);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|cff00ff00|TInterface\\icons\\Spell_Fire_FireBolt02:40|t|r |cff00ff00PvE|r / Fire", GOSSIP_SENDER_MAIN, GOSSIP_PVE_GEAR + 15);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|cff00ff00|TInterface\\icons\\Spell_Frost_FrostBolt02:40|t|r |cff00ff00PvE|r / Frost", GOSSIP_SENDER_MAIN, GOSSIP_PVE_GEAR + 16);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Back..", GOSSIP_SENDER_MAIN, GOSSIP_MAIN_MENU);
                break;
            case CLASS_WARLOCK:
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|cff00ff00|TInterface\\icons\\Spell_Shadow_DeathCoil:40|t|r |cff00ff00PvE|r / Affliction", GOSSIP_SENDER_MAIN, GOSSIP_PVE_GEAR + 17);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|cff00ff00|TInterface\\icons\\Spell_Shadow_Metamorphosis:40|t|r |cff00ff00PvE|r / Demonology", GOSSIP_SENDER_MAIN, GOSSIP_PVE_GEAR + 18);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|cff00ff00|TInterface\\icons\\Spell_Shadow_RainOfFire:40|t|r |cff00ff00PvE|r / Destruction", GOSSIP_SENDER_MAIN, GOSSIP_PVE_GEAR + 19);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Back..", GOSSIP_SENDER_MAIN, GOSSIP_MAIN_MENU);
                break;
            case CLASS_PALADIN:
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|cff00ff00|TInterface\\icons\\Spell_Holy_HolyBolt:40|t|r |cff00ff00PvE|r / Holy", GOSSIP_SENDER_MAIN, GOSSIP_PVE_GEAR + 20);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|cff00ff00|TInterface\\icons\\Ability_Paladin_ShieldoftheTemplar:40|t|r |cff00ff00PvE|r / Protection", GOSSIP_SENDER_MAIN, GOSSIP_PVE_GEAR + 21);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|cff00ff00|TInterface\\icons\\Spell_Holy_AuraOfLight:40|t|r |cff00ff00PvE|r / Retribution", GOSSIP_SENDER_MAIN, GOSSIP_PVE_GEAR + 22);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Back..", GOSSIP_SENDER_MAIN, GOSSIP_MAIN_MENU);
                break;
            case CLASS_DRUID:
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|cff00ff00|TInterface\\icons\\Spell_Nature_StarFall:40|t|r |cff00ff00PvE|r / Balance", GOSSIP_SENDER_MAIN, GOSSIP_PVE_GEAR + 23);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|cff00ff00|TInterface\\icons\\Ability_Racial_BearForm:40|t|r |cff00ff00PvE|r / Feral", GOSSIP_SENDER_MAIN, GOSSIP_PVE_GEAR + 24);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|cff00ff00|TInterface\\icons\\Ability_Racial_BearForm:40|t|r |cff00ff00PvE|r / Guardian", GOSSIP_SENDER_MAIN, GOSSIP_PVE_GEAR + 32);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|cff00ff00|TInterface\\icons\\Spell_Nature_HealingTouch:40|t|r |cff00ff00PvE|r / Restoration", GOSSIP_SENDER_MAIN, GOSSIP_PVE_GEAR + 25);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Back..", GOSSIP_SENDER_MAIN, GOSSIP_MAIN_MENU);
                break;
            case CLASS_DEATH_KNIGHT:
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|cff00ff00|TInterface\\icons\\Spell_Deathknight_BloodPresence:40|t|r |cff00ff00PvE|r / Blood", GOSSIP_SENDER_MAIN, GOSSIP_PVE_GEAR + 26);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|cff00ff00|TInterface\\icons\\Spell_Deathknight_FrostPresence:40|t|r |cff00ff00PvE|r / Frost", GOSSIP_SENDER_MAIN, GOSSIP_PVE_GEAR + 27);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|cff00ff00|TInterface\\icons\\Spell_Deathknight_UnholyPresence:40|t|r |cff00ff00PvE|r / Unholy", GOSSIP_SENDER_MAIN, GOSSIP_PVE_GEAR + 28);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Back..", GOSSIP_SENDER_MAIN, GOSSIP_MAIN_MENU);
                break;
            case CLASS_PRIEST:
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|cff00ff00|TInterface\\icons\\Spell_Holy_PowerWordShield:40|t|r |cff00ff00PvE|r / Discipline", GOSSIP_SENDER_MAIN, GOSSIP_PVE_GEAR + 29);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|cff00ff00|TInterface\\icons\\Spell_Holy_GuardianSpirit:40|t|r |cff00ff00PvE|r / Holy", GOSSIP_SENDER_MAIN, GOSSIP_PVE_GEAR + 30);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|cff00ff00|TInterface\\icons\\Spell_Shadow_ShadowWordPain:40|t|r |cff00ff00PvE|r / Shadow", GOSSIP_SENDER_MAIN, GOSSIP_PVE_GEAR + 31);
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

        static void EquipFullTemplateGearPvE(Player* player, std::string& playerSpecStr) // Merge
        {
            if (sTemplateNpcMgr->CanEquipTemplatePvE(player, playerSpecStr) == false)
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
                        player->DestroyItem(INVENTORY_SLOT_BAG_0, i, true);
                    }
                }
            }
            
            sTemplateNpcMgr->LearnTemplateTalentsPvE(player);
            sTemplateNpcMgr->LearnTemplateGlyphsPvE(player);
            sTemplateNpcMgr->EquipTemplateGearPvE(player);
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

        static void EquipFullTemplateGearPvP(Player* player, std::string& playerSpecStr) // Merge
        {
            if (sTemplateNpcMgr->CanEquipTemplatePvP(player, playerSpecStr) == false)
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
                        player->DestroyItem(INVENTORY_SLOT_BAG_0, i, true);
                    }
                }
            }

            sTemplateNpcMgr->LearnTemplateTalentsPvP(player);
            sTemplateNpcMgr->LearnTemplateGlyphsPvP(player);
            sTemplateNpcMgr->EquipTemplateGearPvP(player);
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
        TC_LOG_INFO("server.loading", "== TEMPLATE NPC ===========================================================================");
        TC_LOG_INFO("server.loading", "Loading Template Talents PvE...");
        sTemplateNpcMgr->LoadTalentsContainerPvE();

        TC_LOG_INFO("server.loading", "Loading Template Glyphs PvE...");
        sTemplateNpcMgr->LoadGlyphsContainerPvE();

        TC_LOG_INFO("server.loading", "Loading Template Gear for Humans PvE...");
        sTemplateNpcMgr->LoadHumanGearContainerPvE();

        TC_LOG_INFO("server.loading", "Loading Template Gear for Alliance PvE...");
        sTemplateNpcMgr->LoadAllianceGearContainerPvE();

        TC_LOG_INFO("server.loading", "Loading Template Gear for Horde PvE...");
        sTemplateNpcMgr->LoadHordeGearContainerPvE();

        TC_LOG_INFO("server.loading", "Loading Template Talents PvP...");
        sTemplateNpcMgr->LoadTalentsContainerPvP();

        TC_LOG_INFO("server.loading", "Loading Template Glyphs PvP...");
        sTemplateNpcMgr->LoadGlyphsContainerPvP();

        TC_LOG_INFO("server.loading", "Loading Template Gear for Humans PvP...");
        sTemplateNpcMgr->LoadHumanGearContainerPvP();

        TC_LOG_INFO("server.loading", "Loading Template Talent Specializations...");
        sTemplateNpcMgr->LoadTalentTreeId();
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
            { "gear-pve",  SEC_ADMINISTRATOR, true,  &HandleExtractTemplateNPCGearPvE, ""},
            { "glyphs-pve",  SEC_ADMINISTRATOR, true,  &HandleExtractTemplateNPCGlyphsPvE, ""},
            { "glyphs-pvp",  SEC_ADMINISTRATOR, true,  &HandleExtractTemplateNPCGlyphsPvP, ""},
            { "talents-pve",  SEC_ADMINISTRATOR, true,  &HandleExtractTemplateNPCTalentsPvE, ""},
            { "talents-pvp",  SEC_ADMINISTRATOR, true,  &HandleExtractTemplateNPCTalentsPvP, ""},
            { "human-pvp",  SEC_ADMINISTRATOR, true,  &HandleExtractTemplateNPCGearHumanPvP, ""},
            { "alliance-pvp",  SEC_ADMINISTRATOR, true,  &HandleExtractTemplateNPCGearAlliancePvP, ""},
            { "horde-pvp",  SEC_ADMINISTRATOR, true,  &HandleExtractTemplateNPCGearHordePvP, ""}
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
        sTemplateNpcMgr->LoadTalentsContainerPvE();
        sTemplateNpcMgr->LoadTalentsContainerPvP();
        sTemplateNpcMgr->LoadGlyphsContainerPvE();
        sTemplateNpcMgr->LoadHumanGearContainerPvE();
        sTemplateNpcMgr->LoadAllianceGearContainerPvE();
        sTemplateNpcMgr->LoadHordeGearContainerPvE();
        sTemplateNpcMgr->LoadGlyphsContainerPvP();
        sTemplateNpcMgr->LoadHumanGearContainerPvP();
        sTemplateNpcMgr->LoadAllianceGearContainerPvP();
        sTemplateNpcMgr->LoadHordeGearContainerPvP();
        handler->SendGlobalGMSysMessage("Template NPC templates reloaded.");
        return true;
    }

    static bool HandleExtractTemplateNPCGearPvE(ChatHandler* handler, const char* args)
    {
        Player* player = handler->GetSession()->GetPlayer();
        if (!player)
        {
            TC_LOG_INFO("server.worldserver", ">>TEMPLATE NPC: No player selected.");
            return false;
        }

        std::string spec = args;
        sTemplateNpcMgr->sTalentsSpec = spec;

        TC_LOG_INFO("server.worldserver", ">>TEMPLATE NPC: GOSSIP_EXTRACT_GEAR selected.");
        TC_LOG_INFO("server.worldserver", ">>TEMPLATE NPC: Player %s sTalentsSpec: %s", player->GetName().c_str(), spec.c_str());
        sTemplateNpcMgr->ExtractGearTemplateToDBPvE(player, sTemplateNpcMgr->sTalentsSpec);

        handler->SendSysMessage("Successfully extracted PvE Gear for Player.");

        return true;
    }

    static bool HandleExtractTemplateNPCGlyphsPvE(ChatHandler* handler, const char* args)
    {
        Player* player = handler->GetSession()->GetPlayer();
        if (!player)
        {
            // Handle the error appropriately, perhaps by sending a message to the admin
            TC_LOG_INFO("server.worldserver", ">>TEMPLATE NPC: No player selected.");
            return false;
        }

        std::string spec = args;
        sTemplateNpcMgr->sTalentsSpec = spec;
        TC_LOG_INFO("server.worldserver", ">>TEMPLATE NPC: Player %s sTalentsSpec: %s", player->GetName().c_str(), spec.c_str());
        sTemplateNpcMgr->ExtractGlyphsTemplateToDBPvE(player, sTemplateNpcMgr->sTalentsSpec);

        handler->SendSysMessage("Successfully extracted PvE Glyphs for Player.");

        return true;
    }

    static bool HandleExtractTemplateNPCTalentsPvE(ChatHandler* handler, const char* args)
    {
        Player* player = handler->GetSession()->GetPlayer();
        if (!player)
        {
            TC_LOG_INFO("server.worldserver", ">>TEMPLATE NPC: No player selected.");
            return false;
        }

        std::string spec = args;
        sTemplateNpcMgr->sTalentsSpec = spec;
        TC_LOG_INFO("server.worldserver", ">>TEMPLATE NPC: Player %s sTalentsSpec: %s", player->GetName().c_str(), spec.c_str());
        sTemplateNpcMgr->ExtractTalentTemplateToDBPvE(player, sTemplateNpcMgr->sTalentsSpec);
        sTemplateNpcMgr->ExtractTalentTreeId(player, sTemplateNpcMgr->sTalentsSpec);

        handler->SendSysMessage("Successfully extracted PvE Talents for Player.");

        return true;
    }

    static bool HandleExtractTemplateNPCTalentsPvP(ChatHandler* handler, const char* args)
    {
        Player* player = handler->GetSession()->GetPlayer();
        if (!player)
        {
            TC_LOG_INFO("server.worldserver", ">>TEMPLATE NPC: No player selected.");
            return false;
        }

        std::string spec = args;
        sTemplateNpcMgr->sTalentsSpec = spec;
        TC_LOG_INFO("server.worldserver", ">>TEMPLATE NPC: Player %s sTalentsSpec: %s", player->GetName().c_str(), spec.c_str());
        sTemplateNpcMgr->ExtractTalentTemplateToDBPvP(player, sTemplateNpcMgr->sTalentsSpec);
        sTemplateNpcMgr->ExtractTalentTreeId(player, sTemplateNpcMgr->sTalentsSpec);

        handler->SendSysMessage("Successfully extracted PvP Talents for Player.");

        return true;
    }

    static bool HandleExtractTemplateNPCGlyphsPvP(ChatHandler* handler, const char* args)
    {
        Player* player = handler->GetSession()->GetPlayer();
        if (!player)
        {
            TC_LOG_INFO("server.worldserver", ">>TEMPLATE NPC: No player selected.");
            return false;
        }

        std::string spec = args;
        sTemplateNpcMgr->sTalentsSpec = spec;
        TC_LOG_INFO("server.worldserver", ">>TEMPLATE NPC: Player %s sTalentsSpec: %s", player->GetName().c_str(), spec.c_str());
        sTemplateNpcMgr->ExtractGlyphsTemplateToDBPvP(player, sTemplateNpcMgr->sTalentsSpec);

        handler->SendSysMessage("Successfully extracted PvP Glyphs for Player.");

        return true;
    }

    static bool HandleExtractTemplateNPCGearHumanPvP(ChatHandler* handler, const char* args)
    {
        Player* player = handler->GetSession()->GetPlayer();
        if (!player)
        {
            TC_LOG_INFO("server.worldserver", ">>TEMPLATE NPC: No player selected.");
            return false;
        }

        std::string spec = args;
        sTemplateNpcMgr->sTalentsSpec = spec;
        TC_LOG_INFO("server.worldserver", ">>TEMPLATE NPC: Player %s sTalentsSpec: %s", player->GetName().c_str(), spec.c_str());
        sTemplateNpcMgr->ExtractGearTemplateToDBHumanPvP(player, sTemplateNpcMgr->sTalentsSpec);

        handler->SendSysMessage("Successfully extracted PvP Human Gear for Player.");

        return true;
    }

    static bool HandleExtractTemplateNPCGearAlliancePvP(ChatHandler* handler, const char* args)
    {
        Player* player = handler->GetSession()->GetPlayer();
        if (!player)
        {
            TC_LOG_INFO("server.worldserver", ">>TEMPLATE NPC: No player selected.");
            return false;
        }

        std::string spec = args;
        sTemplateNpcMgr->sTalentsSpec = spec;
        TC_LOG_INFO("server.worldserver", ">>TEMPLATE NPC: Player %s sTalentsSpec: %s", player->GetName().c_str(), spec.c_str());
        sTemplateNpcMgr->ExtractGearTemplateToDBAlliancePvP(player, sTemplateNpcMgr->sTalentsSpec);

        handler->SendSysMessage("Successfully extracted PvP Alliance Gear for Player.");

        return true;
    }

    static bool HandleExtractTemplateNPCGearHordePvP(ChatHandler* handler, const char* args)
    {
        Player* player = handler->GetSession()->GetPlayer();
        if (!player)
        {
            TC_LOG_INFO("server.worldserver", ">>TEMPLATE NPC: No player selected.");
            return false;
        }

        std::string spec = args;
        sTemplateNpcMgr->sTalentsSpec = spec;
        TC_LOG_INFO("server.worldserver", ">>TEMPLATE NPC: Player %s sTalentsSpec: %s", player->GetName().c_str(), spec.c_str());
        sTemplateNpcMgr->ExtractGearTemplateToDBHordePvP(player, sTemplateNpcMgr->sTalentsSpec);

        handler->SendSysMessage("Successfully extracted PvP Horde Gear for Player.");

        return true;
    }

};

void AddSC_TemplateNPC()
{
    new TemplateNPC();
    new TemplateNPC_World();
    new TemplateNPC_command();
}
