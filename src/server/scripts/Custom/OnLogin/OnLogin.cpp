#include "ScriptMgr.h"
#include "Log.h"
#include "Pet.h"
#include "ObjectMgr.h"
#include "Log.h"
#include "Item.h"
#include "Bag.h"
#include "WorldSession.h"

class OnLoginPlayerScripts : public PlayerScript
{
public:
    OnLoginPlayerScripts() : PlayerScript("OnLoginPlayerScripts") {}

    void HandleUndead(Player* player)
    {
        if (player->getRace() != RACE_UNDEAD_PLAYER)
        {
            return;
        }

        player->RemoveAura(73523);
    }

    void HandleGnome(Player* player)
    {
        if (player->getRace() != RACE_GNOME)
        {
            return;
        }

        player->RemoveAura(80653);
    }

    void HandleDKSkip(Player* player)
    {
        if (player->getClass() != CLASS_DEATH_KNIGHT)
        {
            return;
        }

        player->LearnSpell(50977, false);
        ClearBags(player);
        ClearGear(player);
        player->SaveToDB();
    }

    void HandleWorgenSkip(Player* player)
    {
        if (player->getRace() != RACE_WORGEN)
        {
            return;
        }

        int WORGEN_QUEST = 26706;
        Quest const* questTemplate = sObjectMgr->GetQuestTemplate(WORGEN_QUEST);

        if (player->GetQuestStatus(WORGEN_QUEST) == QUEST_STATUS_NONE)
        {
            player->AddQuest(questTemplate, nullptr);
            player->RewardQuest(questTemplate, 0, player, false);
        }

        ClearBags(player);
        ClearGear(player);
        player->SaveToDB();
    }

    void HandleGoblinSkip(Player* player)
    {
        if (player->getRace() != RACE_GOBLIN)
        {
            return;
        }

        int GOBLIN_QUEST = 25265;
        Quest const* questTemplate = sObjectMgr->GetQuestTemplate(GOBLIN_QUEST);

        if (player->GetQuestStatus(GOBLIN_QUEST) == QUEST_STATUS_NONE)
        {
            player->AddQuest(questTemplate, nullptr);
            player->RewardQuest(questTemplate, 0, player, false);
        }

        ClearBags(player);
        ClearGear(player);
        player->SaveToDB();
    }

    void ClearBags(Player* player)
    {
        // First, remove all items from the bags.
        for (uint8 bag = INVENTORY_SLOT_BAG_START; bag < INVENTORY_SLOT_BAG_END; ++bag)
        {
            Bag* equippedBag = player->GetBagByPos(bag);
            if (equippedBag)
            {
                for (uint32 slot = 0; slot < equippedBag->GetBagSize(); ++slot)
                {
                    Item* item = player->GetItemByPos(bag, slot);
                    if (item)
                    {
                        // Skip Hearthstone
                        if (item->GetEntry() == 6948)
                            continue;

                        // Destroy the item
                        player->DestroyItem(bag, slot, true);
                    }
                }

                // Now that the bag is empty, we can destroy it.
                player->DestroyItem(INVENTORY_SLOT_BAG_0, bag, true);
            }
        }

        // Then, remove all items from the main backpack.
        for (uint32 slot = 23; slot <= 38; ++slot)
        {
            Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
            if (item)
            {
                // Skip Hearthstone
                if (item->GetEntry() == 6948)
                    continue;

                // Destroy the item
                player->DestroyItem(INVENTORY_SLOT_BAG_0, slot, true);
            }
        }

        player->SetMoney(0);
    }

    void ClearGear(Player* player)
    {
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
    }

    void OnLogin(Player* player, bool loginFirst) override
    {
        if (loginFirst) // This is the player's first login
        {
            HandleDKSkip(player);
            HandleWorgenSkip(player);
            HandleGoblinSkip(player);
            HandleUndead(player);
            HandleGnome(player);

            float x = 4324.137695f; // Replace with the X coordinate of the destination
            float y = -2871.365234f; // Replace with the Y coordinate of the destination
            float z = 2.549443f; // Replace with the Z coordinate of the destination
            float o = 2.254905f; // Replace with the orientation of the destination
            uint32 mapId = 0; // Replace with the map ID of the destination
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
        Pet* pet = player->GetPet();

        if (pet)
        {
            pet->SetObjectScale(1.0f); // Set the pet's scale to 1.0
        }
    }
};

void AddSC_OnLoginPlayerScripts()
{
    new OnLoginPlayerScripts();
    new ScalePetOnLogin();
}

