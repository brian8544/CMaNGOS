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
    // In Classic WoW, trainers cast a spell on the player to teach the actual ability:
    SPELL_CRUSADER_STRIKE_TEACH_1 = 7297,
    SPELL_CRUSADER_STRIKE_TEACH_2 = 8825,
    SPELL_CRUSADER_STRIKE_TEACH_3 = 8826,
    SPELL_CRUSADER_STRIKE_TEACH_4 = 10338,
    SPELL_CRUSADER_STRIKE_TEACH_5 = 10339,
    // Actual spells that are learned by trainer spellcast:
    SPELL_CRUSADER_STRIKE_1 = 2537,
    SPELL_CRUSADER_STRIKE_2 = 8823,
    SPELL_CRUSADER_STRIKE_3 = 8824,
    SPELL_CRUSADER_STRIKE_4 = 10336,
    SPELL_CRUSADER_STRIKE_5 = 10337,
    // Original Holy Strike spells use "zzOLDHoly Strike",
    // but since they are nuked from the 1.14.x client, we have to use these instead:
    SPELL_HOLY_STRIKE_1 = 13953,
    SPELL_HOLY_STRIKE_2 = 17143,
    SPELL_HOLY_STRIKE_3 = 17284,

    // Trainer casts a nice visual effect when teaching a spell.
    SPELL_VISUAL_EFFECT = 31726,

    // NPC entry on which we will do area checks.
    NPC_PALADIN_TRAINER = 11537,

    YELL_COOLDOWN = 10 * MINUTE
};

static const char* CALL_PLAYER_MESSAGES[] = {"%s! I sense the Light within you. Come, there is much I can teach you about the ancient powers of the paladins.",
                                             "%s! The Light calls to you. Seek me out, for I have knowledge of ancient paladin techniques.",
                                             "%s! Your dedication to the Light has not gone unnoticed. I can teach you forgotten powers.",
                                             "%s! Champion of the Light, approach! I would share with you the wisdom of ancient paladins."};

#define SAY_NOT_PALADIN "%s, you do not walk the path of the Light. This knowledge is not meant for you."
#define SAY_MAX_RANK "%s, you have mastered all that I can teach you, champion of the Light."
#define SAY_RETURN_LATER "%s, you have learned what I can teach you for now. Return when you have grown stronger in the Light."
#define SAY_TOO_WEAK "%s, you are not yet ready for this power. Train further and return to me."
#define SAY_SPELL_CAST "%s, the Light empowers you with new strength!"

#define GOSSIP_ITEM_LEARN "I seek to learn the ancient powers of the Light."
#define GOSSIP_ITEM_FAREWELL "Perhaps another time."

struct SpellRank
{
    uint32 level;
    uint32 teachSpell;
    uint32 actualSpell;
    uint32 holyStrikeSpell;
};

static const SpellRank SPELL_RANKS[] = {
    {10, SPELL_CRUSADER_STRIKE_TEACH_1, SPELL_CRUSADER_STRIKE_1, SPELL_HOLY_STRIKE_1},
    {22, SPELL_CRUSADER_STRIKE_TEACH_2, SPELL_CRUSADER_STRIKE_2, SPELL_HOLY_STRIKE_2},
    {34, SPELL_CRUSADER_STRIKE_TEACH_3, SPELL_CRUSADER_STRIKE_3, 0                  },
    {46, SPELL_CRUSADER_STRIKE_TEACH_4, SPELL_CRUSADER_STRIKE_4, SPELL_HOLY_STRIKE_3},
    {58, SPELL_CRUSADER_STRIKE_TEACH_5, SPELL_CRUSADER_STRIKE_5, 0                  }
};

static bool CanLearnSpells(Player* pPlayer)
{
    uint32 playerLevel = pPlayer->GetLevel();

    for (const auto& rank : SPELL_RANKS)
    {
        if (playerLevel >= rank.level && !pPlayer->HasSpell(rank.actualSpell))
            return true;
    }

    return false;
}

static void CallOutToPlayer(Creature* pCreature, Player* pPlayer)
{
    static uint32 messageIndex = 0;
    char message[256];

    snprintf(message, sizeof(message), CALL_PLAYER_MESSAGES[messageIndex], pPlayer->GetName());
    pCreature->MonsterYell(message, LANG_UNIVERSAL, nullptr);

    messageIndex = (messageIndex + 1) % (sizeof(CALL_PLAYER_MESSAGES) / sizeof(CALL_PLAYER_MESSAGES[0]));
}

struct npc_custom_paladin_spells_giverAI : public ScriptedAI
{
    npc_custom_paladin_spells_giverAI(Creature* pCreature) : ScriptedAI(pCreature) { Reset(); }

    uint32 m_uiCheckTimer;
    std::map<ObjectGuid, time_t> m_playerCooldowns;

    void Reset() override
    {
        m_uiCheckTimer = 1000;
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
                if (!pPlayer || pPlayer->IsGameMaster())
                    continue;

                if (pPlayer->getClass() != CLASS_PALADIN)
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

                if (CanLearnSpells(pPlayer))
                {
                    CallOutToPlayer(m_creature, pPlayer);
                    m_playerCooldowns[playerGuid] = currentTime + (10 * MINUTE); // 10 minutes cooldown, to prevent spamming the same character name constantly.
                }
            }

            m_uiCheckTimer = 1000;
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

static void TeachSpells(Player* pPlayer, Creature* pCreature)
{
    if (pPlayer->getClass() != CLASS_PALADIN)
    {
        char message[256];
        snprintf(message, sizeof(message), SAY_NOT_PALADIN, pPlayer->GetName());
        pCreature->MonsterSay(message, LANG_UNIVERSAL, pPlayer);
        return;
    }

    uint32 playerLevel = pPlayer->GetLevel();
    bool learnedSomething = false;
    bool hasCurrentRank = false;
    uint32 highestKnownRank = 0;

    // Find highest rank the player knows.
    for (int i = 4; i >= 0; i--)
    {
        if (pPlayer->HasSpell(SPELL_RANKS[i].actualSpell))
        {
            highestKnownRank = i;
            hasCurrentRank = true;
            break;
        }
    }

    // Teach all available spells.
    for (const auto& rank : SPELL_RANKS)
    {
        if (playerLevel >= rank.level && !pPlayer->HasSpell(rank.actualSpell))
        {
            pCreature->CastSpell(pPlayer, rank.teachSpell, TRIGGERED_OLD_TRIGGERED);

            if (rank.holyStrikeSpell != 0)
            {
                pPlayer->learnSpell(rank.holyStrikeSpell, false);
            }

            learnedSomething = true;
        }
    }

    char message[256];

    if (learnedSomething)
    {
        pCreature->CastSpell(pCreature, SPELL_VISUAL_EFFECT, TRIGGERED_OLD_TRIGGERED);
        snprintf(message, sizeof(message), SAY_SPELL_CAST, pPlayer->GetName());
        pCreature->MonsterSay(message, LANG_UNIVERSAL, pPlayer);
    }
    else if (pPlayer->HasSpell(SPELL_CRUSADER_STRIKE_5))
    {
        // Player has max rank.
        snprintf(message, sizeof(message), SAY_MAX_RANK, pPlayer->GetName());
        pCreature->MonsterSay(message, LANG_UNIVERSAL, pPlayer);
    }
    else if (hasCurrentRank && playerLevel < SPELL_RANKS[highestKnownRank + 1].level)
    {
        // Player has their current rank but isn't high enough level for next rank.
        snprintf(message, sizeof(message), SAY_RETURN_LATER, pPlayer->GetName());
        pCreature->MonsterSay(message, LANG_UNIVERSAL, pPlayer);
    }
    else
    {
        // Player doesn't have rank 1 yet (level 10 requirement).
        snprintf(message, sizeof(message), SAY_TOO_WEAK, pPlayer->GetName());
        pCreature->MonsterSay(message, LANG_UNIVERSAL, pPlayer);
    }
}

bool GossipSelect_custom_paladin_spells_giver(Player* pPlayer, Creature* pCreature, uint32 /*uiSender*/, uint32 uiAction)
{
    if (uiAction == GOSSIP_ACTION_INFO_DEF + 1)
    {
        TeachSpells(pPlayer, pCreature);
    }

    pPlayer->CLOSE_GOSSIP_MENU();
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