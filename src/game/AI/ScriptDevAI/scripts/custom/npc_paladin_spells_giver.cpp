/* ScriptData
SDName: custom_paladin_spells_giver
SD%Complete: 100
SDComment: Teaches paladins 2 unused spells.
SDCategory: Custom Scripts
EndScriptData
*/

#include "AI/ScriptDevAI/include/sc_common.h"

enum
{
    // In Classic WoW, trainers cast a spell on the player to teach the actual ability.
    SPELL_CRUSADER_STRIKE_TEACH_1 = 7297,
    SPELL_CRUSADER_STRIKE_TEACH_2 = 8825,
    SPELL_CRUSADER_STRIKE_TEACH_3 = 8826,
    SPELL_CRUSADER_STRIKE_TEACH_4 = 10338,
    SPELL_CRUSADER_STRIKE_TEACH_5 = 10339,

    SPELL_CRUSADER_STRIKE_1 = 2537,
    SPELL_CRUSADER_STRIKE_2 = 8823,
    SPELL_CRUSADER_STRIKE_3 = 8824,
    SPELL_CRUSADER_STRIKE_4 = 10336,
    SPELL_CRUSADER_STRIKE_5 = 10337,

    // Original Holy Strike spells use "zzOLDHoly Strike". But since they are nuked from the 1.14.x client, we have to use these instead:
    SPELL_HOLY_STRIKE_1 = 13953,
    SPELL_HOLY_STRIKE_2 = 17143,
    SPELL_HOLY_STRIKE_3 = 17284,

    SPELL_VISUAL_EFFECT = 31726,

    NPC_PALADIN_TRAINER = 11537, // TEMP NPC WE USE FOR THIS!

    TIMER_CHECK_PALADINS = 1000 // TODO: Replace the timer with a zone check so players always see the NPC's yell.
};

#define SAY_NOT_PALADIN "You do not walk the path of the Light, mortal. This knowledge is not meant for you."
#define SAY_MAX_RANK "You have mastered all that I can teach you, champion of the Light."
#define SAY_RETURN_LATER "You have learned what I can teach you for now. Return when you have grown stronger in the Light."
#define SAY_TOO_WEAK "You are not yet ready for this power. Train further and return to me."
#define SAY_SPELL_CAST "The Light empowers you with new strength!"
#define SAY_CALL_PLAYER "%s! I sense the Light within you. Come, there is much I can teach you about the ancient powers of the paladins." // This one needs a few different lines, so that it doesn't look like it's constantly the same.

#define GOSSIP_ITEM_LEARN "I seek to learn the ancient powers of the Light."
#define GOSSIP_ITEM_FAREWELL "Perhaps another time."

struct npc_custom_paladin_spells_giverAI : public ScriptedAI
{
    npc_custom_paladin_spells_giverAI(Creature* pCreature) : ScriptedAI(pCreature) { Reset(); }

    uint32 m_uiCheckTimer;
    std::map<ObjectGuid, time_t> m_playerCooldowns;

    void Reset() override
    {
        m_uiCheckTimer = TIMER_CHECK_PALADINS;
        m_playerCooldowns.clear();
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (m_uiCheckTimer <= uiDiff)
        {
            time_t currentTime = time(nullptr);

            std::list<Player*> playerList;
            GetPlayerListWithEntryInWorld(playerList, m_creature, 20.0f);

            for (auto* pPlayer : playerList)
            {
                if (!pPlayer || pPlayer->IsGameMaster() || pPlayer->getClass() != CLASS_PALADIN)
                    continue;

                ObjectGuid playerGuid = pPlayer->GetObjectGuid();

                auto it = m_playerCooldowns.find(playerGuid);
                if (it != m_playerCooldowns.end())
                {
                    if (currentTime < it->second)
                        continue;
                    else
                        m_playerCooldowns.erase(it);
                }

                uint32 playerLevel = pPlayer->GetLevel();
                bool canLearn = false;

                if ((playerLevel >= 10 && !pPlayer->HasSpell(SPELL_CRUSADER_STRIKE_1)) || (playerLevel >= 22 && !pPlayer->HasSpell(SPELL_CRUSADER_STRIKE_2)) ||
                    (playerLevel >= 34 && !pPlayer->HasSpell(SPELL_CRUSADER_STRIKE_3)) || (playerLevel >= 46 && !pPlayer->HasSpell(SPELL_CRUSADER_STRIKE_4)) ||
                    (playerLevel >= 58 && !pPlayer->HasSpell(SPELL_CRUSADER_STRIKE_5)))
                {
                    canLearn = true;
                }

                if (canLearn)
                {
                    char message[256];
                    snprintf(message, sizeof(message), SAY_CALL_PLAYER, pPlayer->GetName());
                    m_creature->MonsterYell(message, LANG_UNIVERSAL, nullptr);

                    // Set 10 minute cooldown for this player, so that his name is not spammed constantly.
                    m_playerCooldowns[playerGuid] = currentTime + (10 * MINUTE);
                }
            }

            m_uiCheckTimer = TIMER_CHECK_PALADINS;
        }
        else
            m_uiCheckTimer -= uiDiff;

        ScriptedAI::UpdateAI(uiDiff);
    }
};

UnitAI* GetAI_npc_custom_paladin_spells_giver(Creature* pCreature)
{
    return new npc_custom_paladin_spells_giverAI(pCreature);
}

bool GossipHello_custom_paladin_spells_giver(Player* pPlayer, Creature* pCreature)
{
    if (pPlayer->getClass() == CLASS_PALADIN)
    {
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER, GOSSIP_ITEM_LEARN, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
    }

    pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_FAREWELL, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);

    pPlayer->GetPlayerMenu()->SendGossipMenu(907, pCreature->GetObjectGuid());

    return true;
}

bool GossipSelect_custom_paladin_spells_giver(Player* pPlayer, Creature* pCreature, uint32 /*uiSender*/, uint32 uiAction)
{
    switch (uiAction)
    {
        case GOSSIP_ACTION_INFO_DEF + 1: {
            if (pPlayer->getClass() != CLASS_PALADIN)
            {
                pCreature->MonsterSay(SAY_NOT_PALADIN, LANG_UNIVERSAL, pPlayer);
                pPlayer->CLOSE_GOSSIP_MENU();
                return true;
            }

            uint32 playerLevel = pPlayer->GetLevel();

            struct SpellRank
            {
                uint32 level;
                uint32 teachSpell;
                uint32 actualSpell;
                uint32 holyStrikeSpell;
            };

            SpellRank ranks[] = {
                {10, SPELL_CRUSADER_STRIKE_TEACH_1, SPELL_CRUSADER_STRIKE_1, SPELL_HOLY_STRIKE_1},
                {22, SPELL_CRUSADER_STRIKE_TEACH_2, SPELL_CRUSADER_STRIKE_2, SPELL_HOLY_STRIKE_2},
                {34, SPELL_CRUSADER_STRIKE_TEACH_3, SPELL_CRUSADER_STRIKE_3, 0                  },
                {46, SPELL_CRUSADER_STRIKE_TEACH_4, SPELL_CRUSADER_STRIKE_4, SPELL_HOLY_STRIKE_3},
                {58, SPELL_CRUSADER_STRIKE_TEACH_5, SPELL_CRUSADER_STRIKE_5, 0                  }
            };

            bool learnedSomething = false;

            for (int i = 0; i < 5; i++)
            {
                if (playerLevel >= ranks[i].level && !pPlayer->HasSpell(ranks[i].actualSpell))
                {
                    pCreature->CastSpell(pPlayer, ranks[i].teachSpell, TRIGGERED_OLD_TRIGGERED);

                    if (ranks[i].holyStrikeSpell != 0)
                    {
                        pPlayer->learnSpell(ranks[i].holyStrikeSpell, false);
                    }

                    learnedSomething = true;
                }
            }

            if (learnedSomething)
            {
                pCreature->CastSpell(pCreature, SPELL_VISUAL_EFFECT, TRIGGERED_OLD_TRIGGERED);
                pCreature->MonsterSay(SAY_SPELL_CAST, LANG_UNIVERSAL, pPlayer);
                pPlayer->CLOSE_GOSSIP_MENU();
                return true;
            }

            if (pPlayer->HasSpell(SPELL_CRUSADER_STRIKE_5))
            {
                pCreature->MonsterSay(SAY_MAX_RANK, LANG_UNIVERSAL, pPlayer);
                pPlayer->CLOSE_GOSSIP_MENU();
                return true;
            }

            pCreature->MonsterSay(SAY_TOO_WEAK, LANG_UNIVERSAL, pPlayer);
            pPlayer->CLOSE_GOSSIP_MENU();
            return true;
        }
        case GOSSIP_ACTION_INFO_DEF + 2: {
            pPlayer->CLOSE_GOSSIP_MENU();
            return true;
        }
    }

    return true;
}

void AddSC_custom_paladin_spells_giver()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "custom_paladin_spells_giver";
    pNewScript->GetAI = &GetAI_npc_custom_paladin_spells_giver;
    pNewScript->pGossipHello = &GossipHello_custom_paladin_spells_giver;
    pNewScript->pGossipSelect = &GossipSelect_custom_paladin_spells_giver;
    pNewScript->RegisterSelf(false);
}

bool AreaTrigger_at_custom_paladin_spells_giver(Player* pPlayer, AreaTriggerEntry const* /*pAt*/)
{
    if (pPlayer->IsGameMaster() || pPlayer->getClass() != CLASS_PALADIN)
        return false;

    uint32 playerLevel = pPlayer->GetLevel();

    // I think this whole code can be reduced, since it's the same yell. No need for seperate ifs:
    if (playerLevel >= 10 && !pPlayer->HasSpell(SPELL_CRUSADER_STRIKE_1))
    {
        if (Creature* trainer = GetClosestCreatureWithEntry(pPlayer, NPC_PALADIN_TRAINER, 20.0f))
        {
            char message[256];
            snprintf(message, sizeof(message), SAY_CALL_PLAYER, pPlayer->GetName());
            trainer->MonsterYell(message, LANG_UNIVERSAL, nullptr);
        }
        return true;
    }

    if (playerLevel >= 22 && !pPlayer->HasSpell(SPELL_CRUSADER_STRIKE_2))
    {
        if (Creature* trainer = GetClosestCreatureWithEntry(pPlayer, NPC_PALADIN_TRAINER, 20.0f))
        {
            char message[256];
            snprintf(message, sizeof(message), SAY_CALL_PLAYER, pPlayer->GetName());
            trainer->MonsterYell(message, LANG_UNIVERSAL, nullptr);
        }
        return true;
    }

    if (playerLevel >= 34 && !pPlayer->HasSpell(SPELL_CRUSADER_STRIKE_3))
    {
        if (Creature* trainer = GetClosestCreatureWithEntry(pPlayer, NPC_PALADIN_TRAINER, 20.0f))
        {
            char message[256];
            snprintf(message, sizeof(message), SAY_CALL_PLAYER, pPlayer->GetName());
            trainer->MonsterYell(message, LANG_UNIVERSAL, nullptr);
        }
        return true;
    }

    if (playerLevel >= 46 && !pPlayer->HasSpell(SPELL_CRUSADER_STRIKE_4))
    {
        if (Creature* trainer = GetClosestCreatureWithEntry(pPlayer, NPC_PALADIN_TRAINER, 20.0f))
        {
            char message[256];
            snprintf(message, sizeof(message), SAY_CALL_PLAYER, pPlayer->GetName());
            trainer->MonsterYell(message, LANG_UNIVERSAL, nullptr);
        }
        return true;
    }

    if (playerLevel >= 58 && !pPlayer->HasSpell(SPELL_CRUSADER_STRIKE_5))
    {
        if (Creature* trainer = GetClosestCreatureWithEntry(pPlayer, NPC_PALADIN_TRAINER, 20.0f))
        {
            char message[256];
            snprintf(message, sizeof(message), SAY_CALL_PLAYER, pPlayer->GetName());
            trainer->MonsterYell(message, LANG_UNIVERSAL, nullptr);
        }
        return true;
    }

    return false;
}