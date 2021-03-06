-----------------------------------
-- Area: Behemoth's Dominion
-- NPC:  ???
-- @pos -267 -19 74 127
-----------------------------------
package.loaded["scripts/zones/Behemoths_Dominion/TextIDs"] = nil;
-----------------------------------

require("scripts/globals/settings");
require("scripts/globals/status");
require("scripts/zones/Behemoths_Dominion/TextIDs");

-----------------------------------
-- onTrade Action
-----------------------------------

function onTrade(player,npc,trade)

	local Behemoth = GetMobAction(17297440);
	local KingBehemoth = GetMobAction(17297441);
	
	-- Trade Savory Shank
	if((KingBehemoth == ACTION_NONE or KingBehemoth == ACTION_SPAWN) and trade:hasItemQty(3342,1) and trade:getItemCount() == 1) then -- Check trade, and if mob is ACTION_NONE (0) or waiting to spawn (24)
		player:tradeComplete();
		SpawnMob(17297441,180):updateEnmity(player);
	-- Trade Beastly Shank
	elseif((Behemoth == ACTION_NONE or Behemoth == ACTION_SPAWN) and trade:hasItemQty(3341,1) and trade:getItemCount() == 1) then 
		player:tradeComplete();
		SpawnMob(17297440,180):updateEnmity(player);
	end
	

end;

-----------------------------------
-- onTrigger Action
-----------------------------------

function onTrigger(player,npc)
	player:messageSpecial(IRREPRESSIBLE_MIGHT);
end;

-----------------------------------
-- onEventUpdate
-----------------------------------

function onEventUpdate(player,csid,option)
--printf("CSID: %u",csid);
--printf("RESULT: %u",option);
end;

-----------------------------------
-- onEventFinish
-----------------------------------

function onEventFinish(player,csid,option)
--printf("CSID: %u",csid);
--printf("RESULT: %u",option);
end;