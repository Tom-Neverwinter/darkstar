﻿/*
===========================================================================

  Copyright (c) 2010-2014 Darkstar Dev Teams

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see http://www.gnu.org/licenses/

  This file is part of DarkStar-server source code.

===========================================================================
*/

#include "../../common/utils.h"

#include <math.h>

#include "battleutils.h"
#include "../grades.h"
#include "../trait.h"
#include "mobutils.h"
#include "../mob_modifier.h"
#include "../mob_spell_list.h"
#include "../spell.h"
#include <vector>

namespace mobutils
{
	ModsMap_t mobFamilyModsList;
	ModsMap_t mobPoolModsList;
	ModsMap_t mobSpawnModsList;

/************************************************************************
*																		*
*  Расчет базовой величины оружия монстров								*
*																		*
************************************************************************/

uint16 GetWeaponDamage(CMobEntity* PMob)
{
	float MainLevel = PMob->GetMLevel();
    return (uint16)(MainLevel * ((float)PMob->m_dmgMult / 100.0f) * (MainLevel < 40 ? 1.4 - MainLevel / 100 : 1));
}

/************************************************************************
*																		*
*  Базовое значение для расчера характеристик							*
*  (на название не хватило фантазии)									*
*																		*
************************************************************************/

uint16 GetBaseToRank(uint8 rank, uint16 lvl)
{
	switch (rank)
	{
		case 1: return (5+((lvl-1)*50)/100);
		case 2: return (4+((lvl-1)*45)/100);
		case 3: return (4+((lvl-1)*40)/100);
		case 4: return (3+((lvl-1)*35)/100);
		case 5: return (3+((lvl-1)*30)/100);
		case 6: return (2+((lvl-1)*25)/100);
		case 7: return (2+((lvl-1)*20)/100);
	}
	return 0;
}

/************************************************************************
*																		*
*  Базовое значение для расчерта защиты и уклонения						*
*  (на название не хватило фантазии)									*
*																		*
************************************************************************/

uint16 GetBase(CMobEntity * PMob, uint8 rank)
 {
 	uint8 lvl = PMob->GetMLevel();
 	if(lvl > 50){
 		switch(rank){
 			case 1:
 				return (float)153+(lvl-50)*5.0;
 			case 2:
 				return (float)147+(lvl-50)*4.9;
 			case 3:
 				return (float)136+(lvl-50)*4.8;
 			case 4:
 				return (float)126+(lvl-50)*4.7;
 			case 5:
 				return (float)116+(lvl-50)*4.5;
 			case 6:
 				return (float)106+(lvl-50)*4.4;
 			case 7:
 				return (float)96+(lvl-50)*4.3;
 		}
 	} else {
 		switch(rank){
 			case 1:
 				return (float)6+(lvl-1)*3.0;
 			case 2:
 				return (float)5+(lvl-1)*2.9;
 			case 3:
 				return (float)5+(lvl-1)*2.8;
 			case 4:
 				return (float)4+(lvl-1)*2.7;
 			case 5:
 				return (float)4+(lvl-1)*2.5;
 			case 6:
 				return (float)3+(lvl-1)*2.4;
 			case 7:
 				return (float)3+(lvl-1)*2.3;
 		}
 	}
	return 0;
}

/************************************************************************
*																		*
*  Расчет атрибутов (характеристик) монстра								*
*																		*
************************************************************************/

void CalculateStats(CMobEntity * PMob)
{
	// remove all to keep mods in sync
	PMob->StatusEffectContainer->KillAllStatusEffect();
	PMob->restoreModifiers();
	PMob->restoreMobModifiers();

	// set a random job
	if(PMob->getMobMod(MOBMOD_RAND_JOB))
	{
		bool firstOption = rand()%2 == 0;
		SKILLTYPE meleeSkill = SKILL_NON;
		JOBTYPE job = JOB_NON;
		uint16 spellList = 0;

		// skeleton
		if(PMob->m_Family == 227)
		{
			if(firstOption)
			{
				// blm
				job = JOB_BLM;
				// taken from mob_pools modelid
				int8 look[23] = {0x00, 0x00, 0x34, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
				memcpy(&PMob->look, look, 23);
				spellList = 28; // undead spell list
				meleeSkill = SKILL_SYH;
			}
			else
			{
				// war
				job = JOB_WAR;
				int8 look[23] = {0x00, 0x00, 0x3C, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
				memcpy(&PMob->look, look, 23);
				meleeSkill = SKILL_AXE;
			}
		}
		// evil weapon
		else if(PMob->m_Family == 110 || PMob->m_Family == 111)
		{
			if(firstOption)
			{
				// rdm
				job = JOB_RDM;
				meleeSkill = SKILL_SWD;
				spellList = 42; // evil weapon spell list
			}
			else
			{
				// war
				job = JOB_WAR;
				meleeSkill = SKILL_AXE;
			}
		}
		else
		{
			ShowError("mobutils::CalculateStats Undefined family is being set as a random job %d\n", PMob->m_Family);
		}

		PMob->SetMJob(job);
		PMob->SetSJob(job);
		PMob->m_SpellListContainer = mobSpellList::GetMobSpellList(spellList);
		PMob->m_Weapons[SLOT_MAIN]->setSkillType(meleeSkill);
	}

	bool isNM = PMob->m_Type & MOBTYPE_NOTORIOUS;
	JOBTYPE mJob = PMob->GetMJob();
	JOBTYPE sJob = PMob->GetSJob();
	uint8 mLvl = PMob->GetMLevel();
	ZONETYPE zoneType = PMob->loc.zone->GetType();

	// event mob types will always have custom roaming
	if(PMob->m_Type & MOBTYPE_EVENT)
	{
		PMob->m_roamFlags |= ROAMFLAG_EVENT;
		PMob->m_maxRoamDistance = 0.2f; // always go back to spawn
	}

	if(isNM)
	{
		// enmity range is larger
		PMob->m_enmityRange = 28;
	}

	if(mJob == JOB_DRG)
	{
		// drg can use 2 hour multiple times
		PMob->setMobMod(MOBMOD_2HOUR_MULTI, 1);
	}

	if(PMob->HPmodifier == 0){

		float growth = 1.06;
		float base = 18.0;
		uint8 lvl = PMob->GetMLevel();

		//give hp boost every 10 levels after 25
		//special boosts at 25 and 50
		if(lvl > 75){
			growth = 1.28;
		}else if(lvl > 65){
			growth = 1.27;
		} else if(lvl > 55){
			growth = 1.25;
		} else if(lvl > 50){
			growth = 1.21;
		} else if(lvl > 45){
			growth = 1.17;
		} else if(lvl > 35){
			growth = 1.14;
		} else if(lvl > 25){
			growth = 1.1;
		}

		// pets have lower health
		if(PMob->PMaster != NULL)
		{
			growth = 0.95;
		}


		PMob->health.maxhp = (int16)(base * pow(lvl, growth) * PMob->HPscale);

		if(isNM)
		{
			PMob->health.maxhp *= 2.0;
			if(PMob->GetMLevel() > 75){
				PMob->health.maxhp *= 2.5;
			}
		}

	} else {
		PMob->health.maxhp = PMob->HPmodifier;
	}

	bool hasMp = false;

	switch(mJob){
	case JOB_PLD:
	case JOB_WHM:
	case JOB_BLM:
	case JOB_RDM:
	case JOB_DRK:
	case JOB_BLU:
	case JOB_SCH:
	case JOB_SMN:
		hasMp = true;
		break;
	}

	switch(sJob){
	case JOB_PLD:
	case JOB_WHM:
	case JOB_BLM:
	case JOB_RDM:
	case JOB_DRK:
	case JOB_BLU:
	case JOB_SCH:
	case JOB_SMN:
		hasMp = true;
		break;
	}

	if(PMob->getMobMod(MOBMOD_MP_BASE))
	{
		hasMp = true;
	}

	if(hasMp)
	{
		float scale = PMob->MPscale;

		if(PMob->getMobMod(MOBMOD_MP_BASE))
		{
			scale = (float)PMob->getMobMod(MOBMOD_MP_BASE) / 100.0f;
		}

		if(PMob->MPmodifier == 0){
			PMob->health.maxmp = (int16)(18.2 * pow(PMob->GetMLevel(),1.1075) * scale) + 10;
			if(isNM){
			PMob->health.maxmp *= 1.5;
				if(PMob->GetMLevel()>75){
					PMob->health.maxmp *= 1.5;
				}
			}
		} else {
			PMob->health.maxmp = PMob->MPmodifier;
		}
	}

    PMob->UpdateHealth();

	PMob->health.tp = 0;
    PMob->health.hp = PMob->GetMaxHP();
    PMob->health.mp = PMob->GetMaxMP();

	PMob->m_Weapons[SLOT_MAIN]->setDamage(GetWeaponDamage(PMob));

    //reduce weapon delay of MNK
    if(PMob->GetMJob()==JOB_MNK){
		PMob->m_Weapons[SLOT_MAIN]->resetDelay();
    }

	uint16 fSTR = GetBaseToRank(PMob->strRank, PMob->GetMLevel());
	uint16 fDEX = GetBaseToRank(PMob->dexRank, PMob->GetMLevel());
	uint16 fVIT = GetBaseToRank(PMob->vitRank, PMob->GetMLevel());
	uint16 fAGI = GetBaseToRank(PMob->agiRank, PMob->GetMLevel());
	uint16 fINT = GetBaseToRank(PMob->intRank, PMob->GetMLevel());
	uint16 fMND = GetBaseToRank(PMob->mndRank, PMob->GetMLevel());
	uint16 fCHR = GetBaseToRank(PMob->chrRank, PMob->GetMLevel());

	uint16 mSTR = GetBaseToRank(grade::GetJobGrade(PMob->GetMJob(),2), PMob->GetMLevel());
	uint16 mDEX = GetBaseToRank(grade::GetJobGrade(PMob->GetMJob(),3), PMob->GetMLevel());
	uint16 mVIT = GetBaseToRank(grade::GetJobGrade(PMob->GetMJob(),4), PMob->GetMLevel());
	uint16 mAGI = GetBaseToRank(grade::GetJobGrade(PMob->GetMJob(),5), PMob->GetMLevel());
	uint16 mINT = GetBaseToRank(grade::GetJobGrade(PMob->GetMJob(),6), PMob->GetMLevel());
	uint16 mMND = GetBaseToRank(grade::GetJobGrade(PMob->GetMJob(),7), PMob->GetMLevel());
	uint16 mCHR = GetBaseToRank(grade::GetJobGrade(PMob->GetMJob(),8), PMob->GetMLevel());

	uint16 sSTR = GetBaseToRank(grade::GetJobGrade(PMob->GetSJob(),2), PMob->GetSLevel());
	uint16 sDEX = GetBaseToRank(grade::GetJobGrade(PMob->GetSJob(),3), PMob->GetSLevel());
	uint16 sVIT = GetBaseToRank(grade::GetJobGrade(PMob->GetSJob(),4), PMob->GetSLevel());
	uint16 sAGI = GetBaseToRank(grade::GetJobGrade(PMob->GetSJob(),5), PMob->GetSLevel());
	uint16 sINT = GetBaseToRank(grade::GetJobGrade(PMob->GetSJob(),6), PMob->GetSLevel());
	uint16 sMND = GetBaseToRank(grade::GetJobGrade(PMob->GetSJob(),7), PMob->GetSLevel());
	uint16 sCHR = GetBaseToRank(grade::GetJobGrade(PMob->GetSJob(),8), PMob->GetSLevel());

	if(PMob->GetSLevel() > 15)
	{
		sSTR /= 2;
		sDEX /= 2;
		sAGI /= 2;
		sINT /= 2;
		sMND /= 2;
		sCHR /= 2;
		sVIT /= 2;
	} else {
		sSTR = 0;
		sDEX = 0;
		sAGI = 0;
		sINT = 0;
		sMND = 0;
		sCHR = 0;
		sVIT = 0;
	}

	PMob->stats.STR = fSTR + mSTR + sSTR;
	PMob->stats.DEX = fDEX + mDEX + sDEX;
	PMob->stats.VIT = fVIT + mVIT + sVIT;
	PMob->stats.AGI = fAGI + mAGI + sAGI;
	PMob->stats.INT = fINT + mINT + sINT;
	PMob->stats.MND = fMND + mMND + sMND;
	PMob->stats.CHR = fCHR + mCHR + sCHR;

	if(isNM){
		PMob->stats.STR *= 1.5;
		PMob->stats.DEX *= 1.5;
		PMob->stats.VIT *= 1.5;
		PMob->stats.AGI *= 1.5;
		PMob->stats.INT *= 1.5;
		PMob->stats.MND *= 1.5;
		PMob->stats.CHR *= 1.5;
	}

	// aggro mobs move around a bit more often
	if(PMob->m_Behaviour != BEHAVIOUR_NONE && PMob->speed != 0)
	{
		PMob->setMobMod(MOBMOD_ROAM_COOL, 30);
	}

	// setup special ability
	if(mJob == JOB_RNG)
	{

		// giga
		if(PMob->m_Family == 126 && PMob->m_Family <= 130)
		{
			PMob->setMobMod(MOBMOD_SPECIAL_COOL, 35);
			// catapult
			PMob->setMobMod(MOBMOD_SPECIAL_SKILL, 402);
		}
		else
		{
			// all other rangers
			PMob->setMobMod(MOBMOD_SPECIAL_COOL, 20);
			PMob->setMobMod(MOBMOD_SPECIAL_SKILL, 16);
		}

	}
	else if(mJob == JOB_NIN)
	{
		PMob->setMobMod(MOBMOD_SPECIAL_COOL, 35);
		PMob->setMobMod(MOBMOD_SPECIAL_SKILL, 16);
	}
	else if(mJob == JOB_DRG && PMob->m_Family != 193)
	{
		PMob->setMobMod(MOBMOD_SPECIAL_COOL, 45);

		// sahigans
		if(PMob->m_Family == 213)
		{
			PMob->setMobMod(MOBMOD_SPECIAL_SKILL, 514);
		}
		else
		{
			// all other dragoons
			PMob->setMobMod(MOBMOD_SPECIAL_SKILL, 808);
		}
	}

	// all pets must be defined in the mob_pets file
	// set recast times for summoning pets
	if(PMob->loc.zone->GetType() != ZONETYPE_DYNAMIS)
	{
		if(mJob == JOB_BST)
		{
			PMob->setMobMod(MOBMOD_SPECIAL_COOL, 100);
			PMob->setMobMod(MOBMOD_SPECIAL_SKILL, 761);
		}
		else if(mJob == JOB_DRG && !isNM)
		{
			// only drgs in 3rd expansion calls wyvern as non-NM
			// include fomors
			if(PMob->loc.zone->GetContinentID() == THE_ARADJIAH_CONTINENT || PMob->m_Family == 115)
			{
				// 20 min recast
				PMob->setMobMod(MOBMOD_SPECIAL_SKILL, 476);
				PMob->setMobMod(MOBMOD_SPECIAL_COOL, 720);
			}
		}
		else if(mJob == JOB_PUP)
		{
			PMob->setMobMod(MOBMOD_SPECIAL_SKILL, 1645);
			PMob->setMobMod(MOBMOD_SPECIAL_COOL, 720);
		}
	}

	// ambush antlions
	if(PMob->m_Family == 357)
	{
		PMob->setMobMod(MOBMOD_SPECIAL_SKILL, 22);
		PMob->setMobMod(MOBMOD_SPECIAL_COOL, 1);
		PMob->m_specialFlags |= SPECIALFLAG_HIDDEN;
		PMob->m_roamFlags |= ROAMFLAG_AMBUSH;
	}

	if(PMob->m_Family == 362)
	{
		// rapido doesn't stop
		PMob->m_roamFlags |= ROAMFLAG_IGNORE;
	}


	// handle standback
	// mobs that stand back: blm, whm, rng, cor, nin
	if(mJob == JOB_RNG)
	{

		// giga
		if(PMob->m_Family == 126 && PMob->m_Family <= 130)
		{
			PMob->setMobMod(MOBMOD_STANDBACK_TIME, 20);
		}
		else if(PMob->m_Family == 199)
		{
			// they stay back forever
			PMob->setMobMod(MOBMOD_STANDBACK_TIME, 90);
		}
		else
		{
			PMob->setMobMod(MOBMOD_STANDBACK_TIME, 60);
		}

	}
	else if(mJob == JOB_COR)
	{
		PMob->setMobMod(MOBMOD_STANDBACK_TIME, 60);
	}
	else if(mJob == JOB_BLM)
	{
		PMob->setMobMod(MOBMOD_STANDBACK_TIME, 42);
	}
	else if(mJob == JOB_WHM)
	{
		PMob->setMobMod(MOBMOD_STANDBACK_TIME, 32);
	}
	else if(mJob == JOB_NIN)
	{
		PMob->setMobMod(MOBMOD_STANDBACK_TIME, 25);
	}

	// mobs with zero speed cannot standback
	if(PMob->speed == 0)
	{
		PMob->setMobMod(MOBMOD_STANDBACK_TIME, 0);
	}

    // special case, give spell list to my pet
    if(PMob->getMobMod(MOBMOD_PET_SPELL_LIST) && PMob->PPet != NULL)
    {
    	// Stubborn_Dredvodd
    	CMobEntity* PPet = (CMobEntity*)PMob->PPet;

    	// give pet spell list
    	PPet->m_SpellListContainer = mobSpellList::GetMobSpellList(PMob->getMobMod(MOBMOD_PET_SPELL_LIST));
    }

    if(PMob->getMobMod(MOBMOD_SPELL_LIST))
    {
    	PMob->m_SpellListContainer = mobSpellList::GetMobSpellList(PMob->getMobMod(MOBMOD_SPELL_LIST));
    }

	// TODO: this should be put into its own column
	// has not been decided where yet
	if(PMob->m_Family == 258 || PMob->m_Family == 276)
	{
		// makes worms roam by going into ground / back up
		PMob->m_roamFlags |= ROAMFLAG_WORM;
	}

	if(PMob->m_Behaviour != BEHAVIOUR_NONE && PMob->loc.zone->GetType() == ZONETYPE_OUTDOORS)
	{
		PMob->m_roamFlags |= ROAMFLAG_MEDIUM;
	}
	else
	{
		PMob->m_roamFlags |= ROAMFLAG_SMALL;
	}

	if(PMob->m_roamFlags & ROAMFLAG_AMBUSH)
	{
		// always stay close to spawn
		PMob->m_maxRoamDistance = 2.0f;
	}

	// cap all stats for lvl / job
	for (int i=SKILL_DIV; i <=SKILL_BLU; i++) {
		uint16 maxSkill = battleutils::GetMaxSkill((SKILLTYPE)i,PMob->GetMJob(),PMob->GetMLevel());
		if (maxSkill != 0) {
			PMob->WorkingSkills.skill[i] = maxSkill;
		}
		else //if the mob is WAR/BLM and can cast spell
		{
			// set skill as high as main level, so their spells won't get resisted
			uint16 maxSubSkill = battleutils::GetMaxSkill((SKILLTYPE)i,PMob->GetSJob(),PMob->GetMLevel());

			if (maxSubSkill != 0)
			{
				PMob->WorkingSkills.skill[i] = maxSubSkill;
			}
		}
	}

	if(zoneType == ZONETYPE_DYNAMIS || zoneType == ZONETYPE_BATTLEFIELD)
	{
		// never despawn
		PMob->SetDespawnTimer(0);
		// do not roam around
		PMob->m_roamFlags |= ROAMFLAG_EVENT;
		PMob->m_maxRoamDistance = 0.5f;
	}

	if((zoneType == ZONETYPE_BATTLEFIELD) && (PMob->m_bcnmID != 864) && (PMob->m_bcnmID != 704))
	{
		// bcnmID 864 (desires of emptiness) and 704 (darkness named) don't superlink
		// force all mobs in same instance to superlink
		// plus one in case id is zero
		PMob->setMobMod(MOBMOD_SUPERLINK, PMob->m_instanceID);
	}

	uint8 evaRank = battleutils::GetSkillRank(SKILL_EVA, PMob->GetMJob());

    PMob->addModifier(MOD_DEF, GetBase(PMob,PMob->defRank));
    PMob->addModifier(MOD_EVA, GetBase(PMob,evaRank));
    PMob->addModifier(MOD_ATT, GetBase(PMob,PMob->attRank));
    PMob->addModifier(MOD_ACC, GetBase(PMob,PMob->accRank));

	SKILLTYPE mEvasionRating = SKILL_ELE;

	if(mLvl > 83)
	{
		mEvasionRating = SKILL_SWD;
	}

	//natural magic evasion
	PMob->addModifier(MOD_MEVA, battleutils::GetMaxSkill(mEvasionRating, JOB_RDM, mLvl));

	if((PMob->m_Type & MOBTYPE_NOTORIOUS) && mJob == JOB_WHM && mLvl >= 25)
	{
		// whm nms have stronger regen effect
		PMob->addModifier(MOD_REGEN, PMob->GetMLevel()/4);
	}

	// add traits for sub and main
	AddTraits(PMob, mJob, mLvl);
	AddTraits(PMob, PMob->GetSJob(), PMob->GetSLevel());
}

void AddTraits(CMobEntity* PMob, JOBTYPE jobID, uint8 lvl)
{
	TraitList_t* PTraitsList = traits::GetTraits(jobID);
    for (uint8 i = 0; i <  PTraitsList->size(); ++i)
	{
		CTrait* PTrait = PTraitsList->at(i);
		if (lvl >= PTrait->getLevel() && PTrait->getLevel() > 0)
		{
            PMob->addModifier(PTrait->getMod(), PTrait->getValue());
		}
	}
}

/* Gets the available spells for the specified monster. This looks up the types of spells the monster
 * can cast based on the bitmask in mob_pools (which has been preloaded into memory). It then resolves
 * the list of actual spell IDs based on the bits set. This mapping from bitmask > spellids is based
 * on numerous factors including the type of mob, the level of the mob, etc. The list of spells are
 * then stored in PMob->PSpellContainer
 */
void GetAvailableSpells(CMobEntity* PMob) {
	//make sure the mob actually has a spell list
	if (PMob->m_SpellListContainer == NULL)
	{
		return;
	}

	// setup recast times
	switch(PMob->GetMJob())
	{
		case JOB_BLM:
		case JOB_BRD:
			PMob->setMobMod(MOBMOD_MAGIC_COOL, 30);
		break;
		case JOB_BLU:
			PMob->setMobMod(MOBMOD_MAGIC_COOL, 40);
		break;
		case JOB_NIN:
		case JOB_WHM:
		case JOB_RDM:
			PMob->setMobMod(MOBMOD_MAGIC_COOL, 35);
		break;
		case JOB_SMN:
			PMob->setMobMod(MOBMOD_MAGIC_COOL, 70);
		break;
		default:
			PMob->setMobMod(MOBMOD_MAGIC_COOL, 45);
		break;

	}

	// change spell chances
	switch(PMob->GetMJob())
	{
		case JOB_SMN:
			// smn only has "buffs"
			PMob->defaultMobMod(MOBMOD_BUFF_CHANCE, 100);
		break;
		case JOB_BLM:
			PMob->defaultMobMod(MOBMOD_GA_CHANCE, 40);
			PMob->defaultMobMod(MOBMOD_BUFF_CHANCE, 15);
		break;
		case JOB_RDM:
			PMob->defaultMobMod(MOBMOD_GA_CHANCE, 15);
			PMob->defaultMobMod(MOBMOD_BUFF_CHANCE, 40);
		break;
		case JOB_NIN:
			PMob->defaultMobMod(MOBMOD_BUFF_CHANCE, 30);
		break;
		case JOB_BRD:
			PMob->defaultMobMod(MOBMOD_GA_CHANCE, 25);
			PMob->defaultMobMod(MOBMOD_BUFF_CHANCE, 60);
		break;
	}

	if(PMob->m_Type & MOBTYPE_NOTORIOUS)
	{
		// NMs cure earlier
		PMob->defaultMobMod(MOBMOD_HP_HEAL_CHANCE, 50);
		PMob->defaultMobMod(MOBMOD_HEAL_CHANCE, 40);
	}

	// catch all non-defaulted spell chances
	PMob->defaultMobMod(MOBMOD_GA_CHANCE, 45);
	PMob->defaultMobMod(MOBMOD_NA_CHANCE, 40);
	PMob->defaultMobMod(MOBMOD_BUFF_CHANCE, 35);
	PMob->defaultMobMod(MOBMOD_HEAL_CHANCE, 40);
	PMob->defaultMobMod(MOBMOD_HP_HEAL_CHANCE, 25);

	// clear spell list
	PMob->SpellContainer->ClearSpells();

	//insert the rest of the spells
	for (std::vector<MobSpell_t>::iterator it = PMob->m_SpellListContainer->m_spellList.begin(); it != PMob->m_SpellListContainer->m_spellList.end() ; ++it)
	{
		if (PMob->GetMLevel() >= (*it).min_level && PMob->GetMLevel() <= (*it).max_level)
		{
			PMob->SpellContainer->AddSpell((*it).spellId);
		}
	}

	// make sure mob has mp to cast spells
	if(PMob->health.maxmp == 0 && PMob->SpellContainer != NULL && PMob->SpellContainer->HasMPSpells())
	{
		ShowError("mobutils::CalculateStats Mob (%u) has no mp for casting spells!\n", PMob->id);
	}
}

void InitializeMob(CMobEntity* PMob, CZone* PZone)
{
	// add special mob mods

    // this only has to be added once
    AddCustomMods(PMob);

    ZONETYPE zoneType = PZone->GetType();

	// do not despawn if I match this criteria
	if((PMob->m_Type & MOBTYPE_NOTORIOUS) || (PMob->m_Type & MOBTYPE_EVENT) || MOB_NO_DESPAWN || zoneType == ZONETYPE_DYNAMIS || zoneType == ZONETYPE_BATTLEFIELD)
	{
		PMob->setMobMod(MOBMOD_NO_DESPAWN, 1);
	}

	if(zoneType == ZONETYPE_DYNAMIS)
	{
		// no gil drop and no mugging!
		PMob->setMobMod(MOBMOD_GIL_MAX, -1);
		PMob->setMobMod(MOBMOD_MUG_GIL, -1);
	}

	// add two hours
	// this mod should be added to all NMs that two hour.
	// for dynamis mobs the statue should give mobs its two hour
	if(PMob->GetMLevel() >= 10)
	{
		if(PMob->m_Type & MOBTYPE_NOTORIOUS || zoneType == ZONETYPE_DYNAMIS)
		{
			if(PMob->m_EcoSystem == SYSTEM_BEASTMEN ||
				PMob->m_EcoSystem == SYSTEM_HUMANOID)
			{
				PMob->defaultMobMod(MOBMOD_MAIN_2HOUR, 1);
			}
		}
	}

	PMob->m_Immunity |= PMob->getMobMod(MOBMOD_IMMUNITY);

	PMob->defaultMobMod(MOBMOD_SKILLS, PMob->m_Family);
	PMob->defaultMobMod(MOBMOD_LINK_RADIUS, MOB_LINK_RADIUS);
	PMob->defaultMobMod(MOBMOD_TP_USE_CHANCE, MOB_TP_USE_CHANCE);
	PMob->defaultMobMod(MOBMOD_ROAM_COOL, 45);

    // Killer Effect
    switch (PMob->m_EcoSystem)
      {
        case SYSTEM_AMORPH:   PMob->addModifier(MOD_BIRD_KILLER,     5); break;
        case SYSTEM_AQUAN:    PMob->addModifier(MOD_AMORPH_KILLER,   5); break;
        case SYSTEM_ARCANA:   PMob->addModifier(MOD_UNDEAD_KILLER,   5); break;
        case SYSTEM_BEAST:    PMob->addModifier(MOD_LIZARD_KILLER,   5); break;
        case SYSTEM_BIRD:     PMob->addModifier(MOD_AQUAN_KILLER,    5); break;
        case SYSTEM_DEMON:    PMob->addModifier(MOD_DRAGON_KILLER,   5); break;
        case SYSTEM_DRAGON:   PMob->addModifier(MOD_DEMON_KILLER,    5); break;
        case SYSTEM_LIZARD:   PMob->addModifier(MOD_VERMIN_KILLER,   5); break;
        case SYSTEM_LUMINION: PMob->addModifier(MOD_LUMORIAN_KILLER, 5); break;
        case SYSTEM_LUMORIAN: PMob->addModifier(MOD_LUMINION_KILLER, 5); break;
        case SYSTEM_PLANTOID: PMob->addModifier(MOD_BEAST_KILLER,    5); break;
        case SYSTEM_UNDEAD:   PMob->addModifier(MOD_ARCANA_KILLER,   5); break;
        case SYSTEM_VERMIN:   PMob->addModifier(MOD_PLANTOID_KILLER, 5); break;
      }
}

/*
Loads up custom mob mods from mob_pool_mods and mob_family_mods table. This will allow you to customize
a mobs regen rate, magic defense, triple attack rate from a table instead of hardcoding it.

Usage:

	Evil weapons have a magic defense boost. So pop that into mob_family_mods table.
	Goblin Diggers have a vermin killer trait, so find its poolid and put it in mod_pool_mods table.

*/
void LoadCustomMods()
{

	// load family mods
	const int8 QueryFamilyMods[] = "SELECT familyid, modid, value, type FROM mob_family_mods;";

    int32 ret = Sql_Query(SqlHandle, QueryFamilyMods);

	if(ret != SQL_ERROR && Sql_NumRows(SqlHandle) != 0)
	{
		while(Sql_NextRow(SqlHandle) == SQL_SUCCESS)
		{
			ModsList_t* familyMods = GetMobFamilyMods(Sql_GetUIntData(SqlHandle,0), true);

			CModifier* mod = new CModifier(Sql_GetUIntData(SqlHandle,1));
			mod->setModAmount(Sql_GetIntData(SqlHandle,2));

			uint16 type = Sql_GetUIntData(SqlHandle,3);
			if(type == 1)
			{
				familyMods->mobMods.push_back(mod);
			}
			else
			{
				familyMods->mods.push_back(mod);
			}
		}
	}

	// load pool mods
	const int8 QueryPoolMods[] = "SELECT poolid, modid, value, type FROM mob_pool_mods;";

    ret = Sql_Query(SqlHandle, QueryPoolMods);

	if(ret != SQL_ERROR && Sql_NumRows(SqlHandle) != 0)
	{
		while(Sql_NextRow(SqlHandle) == SQL_SUCCESS)
		{
			uint16 pool = Sql_GetUIntData(SqlHandle,0);
			ModsList_t* poolMods = GetMobPoolMods(pool, true);

			uint16 id = Sql_GetUIntData(SqlHandle,1);


			CModifier* mod = new CModifier(id);
			mod->setModAmount(Sql_GetUIntData(SqlHandle,2));

			uint16 type = Sql_GetUIntData(SqlHandle,3);
			if(type == 1)
			{
				poolMods->mobMods.push_back(mod);
			}
			else
			{
				poolMods->mods.push_back(mod);
			}
		}
	}

	// load spawn mods
	const int8 QuerySpawnMods[] = "SELECT mobid, modid, value, type FROM mob_spawn_mods;";

    ret = Sql_Query(SqlHandle, QuerySpawnMods);

	if(ret != SQL_ERROR && Sql_NumRows(SqlHandle) != 0)
	{
		while(Sql_NextRow(SqlHandle) == SQL_SUCCESS)
		{
			ModsList_t* spawnMods = GetMobSpawnMods(Sql_GetUIntData(SqlHandle,0), true);

			CModifier* mod = new CModifier(Sql_GetUIntData(SqlHandle,1));
			mod->setModAmount(Sql_GetUIntData(SqlHandle,2));

			uint16 type = Sql_GetUIntData(SqlHandle,3);
			if(type == 1)
			{
				spawnMods->mobMods.push_back(mod);
			}
			else
			{
				spawnMods->mods.push_back(mod);
			}
		}
	}
}

ModsList_t* GetMobFamilyMods(uint16 familyId, bool create)
{
	if(mobFamilyModsList[familyId])
	{
		return mobFamilyModsList[familyId];
	}

	if(create)
	{
		// create new one
		ModsList_t* mods = new ModsList_t;
		mods->id = familyId;

		mobFamilyModsList[familyId] = mods;

		return mods;
	}

	return NULL;
}

ModsList_t* GetMobPoolMods(uint32 poolId, bool create)
{
	if(mobPoolModsList[poolId])
	{
		return mobPoolModsList[poolId];
	}

	if(create)
	{
		// create new one
		ModsList_t* mods = new ModsList_t;
		mods->id = poolId;

		mobPoolModsList[poolId] = mods;

		return mods;
	}

	return NULL;
}

ModsList_t* GetMobSpawnMods(uint32 mobId, bool create)
{
	if(mobSpawnModsList[mobId])
	{
		return mobSpawnModsList[mobId];
	}

	if(create)
	{
		// create new one
		ModsList_t* mods = new ModsList_t;
		mods->id = mobId;

		mobSpawnModsList[mobId] = mods;

		return mods;
	}

	return NULL;
}

void AddCustomMods(CMobEntity* PMob)
{

	// find my families custom mods
	ModsList_t* PFamilyMods = GetMobFamilyMods(PMob->m_Family);

	if(PFamilyMods != NULL)
	{
		// add them
		for(std::vector<CModifier*>::iterator it = PFamilyMods->mods.begin(); it != PFamilyMods->mods.end() ; ++it)
		{
			PMob->addModifier((*it)->getModID(), (*it)->getModAmount());
		}

		for(std::vector<CModifier*>::iterator it = PFamilyMods->mobMods.begin(); it != PFamilyMods->mobMods.end() ; ++it)
		{
			PMob->setMobMod((*it)->getModID(), (*it)->getModAmount());
		}
	}

	// find my pools custom mods
	ModsList_t* PPoolMods = GetMobPoolMods(PMob->m_Pool);

	if(PPoolMods != NULL)
	{
		// add them
		for(std::vector<CModifier*>::iterator it = PPoolMods->mods.begin(); it != PPoolMods->mods.end() ; ++it)
		{
			PMob->addModifier((*it)->getModID(), (*it)->getModAmount());
		}

		for(std::vector<CModifier*>::iterator it = PPoolMods->mobMods.begin(); it != PPoolMods->mobMods.end() ; ++it)
		{
			PMob->setMobMod((*it)->getModID(), (*it)->getModAmount());
		}
	}

	// find my pools custom mods
	ModsList_t* PSpawnMods = GetMobSpawnMods(PMob->id);

	if(PSpawnMods != NULL)
	{
		// add them
		for(std::vector<CModifier*>::iterator it = PSpawnMods->mods.begin(); it != PSpawnMods->mods.end() ; ++it)
		{
			PMob->addModifier((*it)->getModID(), (*it)->getModAmount());
		}

		for(std::vector<CModifier*>::iterator it = PSpawnMods->mobMods.begin(); it != PSpawnMods->mobMods.end() ; ++it)
		{
			PMob->setMobMod((*it)->getModID(), (*it)->getModAmount());
		}
	}
}

void SetupMaat(CMobEntity* PMob, JOBTYPE job)
{
	//set job based on characters job
	PMob->ChangeMJob(job);

	// reset just incase
	CalculateStats(PMob);

	PMob->m_Weapons[SLOT_MAIN]->setDelay((240*1000)/60);

	// this is kind a hacky but make nin maat always double attacks
	switch(PMob->GetMJob()){
		case JOB_NIN:
			PMob->setModifier(MOD_DOUBLE_ATTACK, 100);
			PMob->m_Weapons[SLOT_MAIN]->resetDelay();
		break;
	}
}

}; // namespace mobutils
