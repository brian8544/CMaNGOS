#include "Custom/Custom.h"
#include "Database/DatabaseEnv.h"

#ifdef ENABLE_PLAYERBOTS
uint32 Custom::GetOnlineBotsCount()
{
    // Get all online characters first
    std::unique_ptr<QueryResult> charResult(CharacterDatabase.Query("SELECT DISTINCT account FROM characters WHERE online = 1"));
    if (!charResult)
        return 0;

    // Use a set to ensure unique account counting (avoid double-counting)
    std::set<uint32> botAccounts;

    // Check each account in LoginDatabase to see if it's a bot
    do
    {
        Field* charFields = charResult->Fetch();
        uint32 accountId = charFields[0].GetUInt32();

        // Query LoginDatabase to check if this account is a bot
        std::unique_ptr<QueryResult> loginResult(LoginDatabase.PQuery("SELECT 1 FROM account WHERE id = %u AND username LIKE 'RNDBOT%%'", accountId));
        if (loginResult)
        {
            botAccounts.insert(accountId);
        }
    }
    while (charResult->NextRow());

    return botAccounts.size();
}
#endif
