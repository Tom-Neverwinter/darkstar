-----------------------------------	
-- Area: Sauromugue Champaign	
-- MOB:  Goblin Gambler	
-----------------------------------	
	
require("/scripts/globals/fieldsofvalor");	
	
-----------------------------------	
-- onMobDeath	
-----------------------------------	
	
function onMobDeath(mob,killer)	
	checkRegime(killer,mob,97,2);
	checkRegime(killer,mob,98,2);
end;	
