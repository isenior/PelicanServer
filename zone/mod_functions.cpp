#include "client.h"
#include "entity.h"
#include "mob.h"
#include "npc.h"
#include "worldserver.h"
#include "zone.h"

class Spawn2;
struct Consider_Struct;
struct DBTradeskillRecipe_Struct;

namespace EQ
{
	class ItemInstance;
}

extern EntityList entity_list;
extern Zone* zone;

extern WorldServer worldserver;

// Custom
static const bool isDebug = true;

//All functions that modify a value are passed the value as it was computed by default formulas and bonuses.  In most cases this should be the final value that will be used.

//These are called when a zone boots or is repopped
void Zone::mod_init() { return; }
void Zone::mod_repop() { return; }

//Pre-spawn hook called from the NPC object to be spawned
void NPC::mod_prespawn(Spawn2 *sp) { return; }

//Base damage from NPC::Attack
int NPC::mod_npc_damage(int damage, EQ::skills::SkillType skillinuse, int hand, const EQ::ItemData* weapon, Mob* other) { return(damage); }

//Mob c has been given credit for a kill.  This is called after the regular EVENT_KILLED_MERIT event.
void NPC::mod_npc_killed_merit(Mob* c) { return; }

//Mob oos has been given credit for a kill.  This is called after the regular EVENT_DEATH event.
void NPC::mod_npc_killed(Mob* oos) { return; }

//Base damage from Client::Attack - can cover myriad skill types

// CUSTOM 
int Client::mod_client_damage(int damage, EQ::skills::SkillType skillinuse, int hand, const EQ::ItemInstance* weapon, Mob* other)
{ 
	if (isDebug) { Message(0,"Modding Melee Damage: %i", damage);}
	float modifier = (GetSTR() / 80.0) + (GetDEX() / 160.0);
	if (modifier > 1)
	{
		damage *= modifier;
	}
	if (isDebug) { Message(0,"Resulting Melee Damage: %i", damage);}
	return(damage); 
}

//message is char[4096], don't screw it up. Return true for normal behavior, false to return immediately.
// Channels:
// 0  - Guild Chat
// 2  - Group Chat
// 3  - Shout
// 4  - Auction
// 5  - Out of Character
// 6  - Broadcast
// 7  - Tell
// 8  - Say
// 11 - GMSay
// 15 - Raid Chat
// 20 - UCS Relay for UF client and later
// 22 - Emotes for UF and later
bool Client::mod_client_message(char* message, uint8 chan_num) { return(true); }

//Skillup override.  When this is called the regular skillup check has failed.  Return false to proceed with default behavior.
//This will NOT allow a client to increase skill past a cap.
bool Client::mod_can_increase_skill(EQ::skills::SkillType skillid, Mob* against_who) { return(false); }

//chance of general skill increase, rolled against 0-99 where higher chance is better.
int16 Client::mod_increase_skill_chance(int16 chance, Mob* against_who) { return(chance); }

//Max percent of health you can bind wound starting with default value for class, item, and AA bonuses
int Client::mod_bindwound_percent(int max_percent, Mob* bindmob) { return(max_percent); }

//Final bind HP value after bonuses
// CUSTOM
int Client::mod_bindwound_hp(int bindhps, Mob* bindmob) 
{ 
	if (isDebug) { Message(0,"Modding Binding Value: %i", bindhps); }
	float modifier = GetWIS()/30.0;
	if (modifier > 1)
	{
		bindhps *= modifier;
	}
	if (isDebug) { Message(0,"Modded : %i", bindhps); }
	return(bindhps); 
}

//Client haste as calculated by default formulas - In percent from 0-100
int Client::mod_client_haste(int h) { return(h); }

//Haste cap override
int Client::mod_client_haste_cap(int cap) { return(cap); }

//This is called when a client cons a mob
void Client::mod_consider(Mob* tmob, Consider_Struct* con) { return; }

//Return true to continue with normal behavior, false returns in the parent function
bool Client::mod_saylink(const std::string& response, bool silentsaylink) { return(true); }

//Client pet power as calculated by default formulas and bonuses
// CUSTOM
int16 Client::mod_pet_power(int16 act_power, uint16 spell_id) 
{ 
	if (isDebug) { Message(0,"Original Pet Power: %i", act_power); }
	int modifier = GetCHA() - 80;
	if (modifier > 0)
	{
		act_power += modifier;
	}
	if (isDebug) { Message(0,"Modded Pet Power: %i", act_power); }
	return(act_power); 
}

//Chance to combine rolled against a random 0-99 where higher is better.
float Client::mod_tradeskill_chance(float chance, DBTradeskillRecipe_Struct *spec) { return(chance); }

//Chance to skillup rolled against a random 0-99 where higher is better.
float Client::mod_tradeskill_skillup(float chance_stage2) { return(chance_stage2); }

//Tribute value override
int32 Client::mod_tribute_item_value(int32 pts, const EQ::ItemInstance* item) { return(pts); }

//Death reporting
void Client::mod_client_death_npc(Mob* killerMob) { return; }
void Client::mod_client_death_duel(Mob* killerMob) { return; }
void Client::mod_client_death_env() { return; }

//Calculated xp before consider modifier, called whenever a client gets XP for killing a mob.
int32 Client::mod_client_xp(int32 in_xp, NPC *npc) { return(in_xp); }

//Client XP formula.  Changes here will cause clients to change level after gaining or losing xp.
//Either modify this before your server goes live, or be prepared to write a quest script that fixes levels.
//To adjust how much XP is given per kill, use mod_client_xp
uint32 Client::mod_client_xp_for_level(uint32 xp, uint16 check_level) { return(xp); }

//Food and drink values as computed by consume requests.  Return < 0 to abort the request.
int Client::mod_food_value(const EQ::ItemData *item, int change) { return(change); }
int Client::mod_drink_value(const EQ::ItemData *item, int change) { return(change); }

//effect_value - Spell effect value as calculated by default formulas.  You will want to ignore effects that don't lend themselves to scaling - pet ID's, gate coords, etc.
// CUSTOM
int Mob::mod_effect_value(int effect_value, uint16 spell_id, int effect_type, Mob* caster, uint16 caster_id) 
{ 
	int bard_bonus = 0;
	
	// We really only want the caster if this is a song; otherwise, the bonus is based off of the client
	if (IsBardSong(spell_id))
	{
		// Sometimes, the caster_id is passed but not the mob (Mainly bonuses like regen spells and DS)
		if (caster == nullptr && caster_id > 0)
		{
			caster = entity_list.GetMob(caster_id);
		}
		
		if (caster && caster->GetClass() == BARD) // Bards are special
		{
			bard_bonus = (GetCHA() + GetDEX() - 160)/10;
			if (bard_bonus < 0) { bard_bonus = 0;}
			caster->Message(0,"You're a bard! Bonus: %i", bard_bonus);
		}
	}
	
	if (isDebug)
	{
		switch (effect_type)
		{
			case SE_BardAEDot:
				Message(0,"Modding SE_BardAEDot: %i", effect_value);
				if (caster) { Message(0,"Caster: Modding SE_BardAEDot: %i", effect_value); }
				break;
			case SE_CompleteHeal:
				Message(0,"Modding SE_CompleteHeal: %i", effect_value);
				if (caster) { Message(0,"Caster: Modding SE_CompleteHeal: %i", effect_value); }
				break;
			case SE_CurrentHP:
				Message(0,"Modding SE_CurrentHP: %i", effect_value);
				if (caster) { Message(0,"Caster: Modding SE_CurrentHP: %i", effect_value); }
				break;
			case SE_CurrentHPOnce:
				Message(0,"Modding SE_CurrentHPOnce: %i", effect_value);
				if (caster) { Message(0,"Caster: Modding SE_CurrentHPOnce: %i", effect_value); }
				break;
			case SE_HealOverTime:
				Message(0,"Modding SE_HealOverTime: %i", effect_value);
				if (caster) { Message(0,"Caster: Modding SE_HealOverTime: %i", effect_value); }
				break;
			case SE_DamageShield:
				Message(0,"Modding SE_DamageShield: %i", effect_value);
				if (caster) { Message(0,"Caster: Modding SE_DamageShield: %i", effect_value); }
				break;
			case SE_Rune:
				Message(0,"Modding SE_Rune: %i", effect_value);
				if (caster) { Message(0,"Caster: Modding SE_Rune: %i", effect_value); }
				break;
			case SE_ManaRegen_v2:
				Message(0,"Modding SE_ManaRegen_v2: %i", effect_value);
				if (caster) { Message(0,"Caster: Modding SE_ManaRegen_v2: %i", effect_value); }
				break;
			case SE_CurrentMana:
				Message(0,"Modding SE_CurrentMana: %i", effect_value);
				if (caster) { Message(0,"Caster: Modding SE_CurrentMana: %i", effect_value); }
				break;
			case SE_CurrentManaOnce:
				Message(0,"Modding SE_CurrentManaOnce: %i", effect_value);
				if (caster) { Message(0,"Caster: Modding SE_CurrentManaOnce: %i", effect_value); }
				break;
			default:
				Message(0,"Modding unknown type: %i", effect_value);
				if (caster) { Message(0,"Caster: Modding unknown type: %i", effect_value); }
				break;
		}
	}
	// Spell effects that require a caster
	if (caster && (caster->IsClient() || (caster->IsPet() && caster->GetOwner() && caster->GetOwner()->IsClient())))
	{		
		// Damage or healing spell
		if (effect_type == SE_BardAEDot || effect_type == SE_CompleteHeal ||
			effect_type == SE_CurrentHP || effect_type == SE_CurrentHPOnce || effect_type == SE_HealOverTime)
		{
			if (effect_value < 0) //Nuke or Dot
			{
				if (this != caster) // Only apply damage bonus if casting on someone else to avoid death from AEs and so on
				{
					effect_value -= bard_bonus;
					if (caster->GetINT() > 50)
					{
						effect_value *= (caster->GetINT() / 50.0);
					}			
				}		
			}
			else //Heal or HoT
			{
				effect_value += bard_bonus;
				if (caster->GetWIS() > 50)
				{
					effect_value *= (caster->GetWIS() / 50.0);
				}
			}
		}
	}
	// Damage shields and runes are based on the target and not the caster
	if (this->IsClient() || (this->IsPet() && this->GetOwner() && this->GetOwner()->IsClient()))
	{
		// SE_CurrentHP will only trigger without a caster on the first tick of a regen buff
		if (effect_type == SE_DamageShield || effect_type == SE_Rune || (effect_type == SE_CurrentHP && caster == nullptr))
		{
			if (effect_value < 0)
			{
				effect_value -= bard_bonus;
			}
			else
			{
				effect_value += bard_bonus;
				
			}
			if (GetSTA() > 80)
			{
				effect_value *= (GetSTA() / 80.0);
			}	
		}
		else if (effect_type == SE_ManaRegen_v2 || effect_type == SE_CurrentMana || effect_type == SE_CurrentManaOnce) // Mana regen based on target's int
		{
			effect_value += bard_bonus;
			if (GetINT() > 80)
			{
				effect_value *= (GetINT() / 80.0);
			}
		}
	}
	
	if (isDebug)
	{
		if (caster) { caster->Message(0,"Caster - Modded effect value: %i", effect_value); }
		Message(0,"Modded effect value: %i", effect_value);
	}
	return(effect_value); 
}

//chancetohit - 0 to 100 percent - set over 1000 for a guaranteed hit
float Mob::mod_hit_chance(float chancetohit, EQ::skills::SkillType skillinuse, Mob* attacker) { return(chancetohit); }

//Final riposte chance
// CUSTOM 
float Mob::mod_riposte_chance(float ripostechance, Mob* attacker) 
{ 
	if (!(IsClient() || (IsPet() && GetOwner() && GetOwner()->IsClient())))
	{
		return ripostechance;
	}
	if (isDebug) { Message(0,"Modding riposte chance: %.2f", ripostechance); }
	int agi = GetAGI();
	if (agi > 80)
	{
		ripostechance += logf(agi - 80) / logf(5);
	}
	if (isDebug) { Message(0,"Modded riposte chance: %.2f", ripostechance); }
	return(ripostechance); 
}

//Final block chance
// CUSTOM
float Mob::mod_block_chance(float blockchance, Mob* attacker) 
{ 
	if (!(IsClient() || (IsPet() && GetOwner() && GetOwner()->IsClient())))
	{
		return blockchance;
	}
	if (isDebug) { Message(0,"Modding block chance: %.2f", blockchance); }
	int agi = GetAGI();
	if (agi > 80)
	{
		blockchance += logf(agi - 80) / logf(5);
	}
	if (isDebug) { Message(0,"Modded block chance: %.2f", blockchance); }
	return(blockchance); 
}

//Final parry chance
// CUSTOM
float Mob::mod_parry_chance(float parrychance, Mob* attacker) 
{ 
	if (!(IsClient() || (IsPet() && GetOwner() && GetOwner()->IsClient())))
	{
		return parrychance;
	}
	if (isDebug) { Message(0,"Modding parry chance: %.2f", parrychance); }
	int agi = GetAGI();
	if (agi > 80)
	{
		parrychance += logf(agi - 80) / logf(5);
	}
	if (isDebug) { Message(0,"Modded parry chance: %.2f", parrychance); }
	return(parrychance); 
}

//Final dodge chance
// CUSTOM
float Mob::mod_dodge_chance(float dodgechance, Mob* attacker) 
{ 
	if (!(IsClient() || (IsPet() && GetOwner() && GetOwner()->IsClient())))
	{
		return dodgechance;
	}
	if (isDebug) { Message(0,"Modding dodge chance: %.2f", dodgechance); }
	int agi = GetAGI();
	if (agi > 80)
	{
		dodgechance += logf(agi - 80) / logf(5);
	}
	if (isDebug) { Message(0,"Modded dodge chance: %.2f", dodgechance); }
	return(dodgechance); 
}

//Monk AC Bonus weight cap.  Defined in Combat:MonkACBonusWeight
//Usually 15, a monk under this weight threshold gets an AC bonus
float Mob::mod_monk_weight(float monkweight, Mob* attacker) { return(monkweight); }

//Mitigation rating is compared to incoming attack rating.  Higher is better.
// CUSTOM
float Mob::mod_mitigation_rating(float mitigation_rating, Mob* attacker) 
{ 
	if (!(IsClient() || (IsPet() && GetOwner() && GetOwner()->IsClient())))
	{
		return mitigation_rating;
	}
	if (isDebug) { Message(0,"Modding mitigation rating: %.2f", mitigation_rating); }
	int modifier = GetSTA() - 80;
	if (modifier > 0)
	{
		mitigation_rating += modifier;
	}
	if (isDebug) { Message(0,"Modded mitigation rating: %.2f", mitigation_rating); }
	return(mitigation_rating); 
}

float Mob::mod_attack_rating(float attack_rating, Mob* defender) { return(attack_rating); }

//Kick damage after all other bonuses are applied
// CUSTOM
int32 Mob::mod_kick_damage(int32 dmg) 
{ 
	if (!(IsClient() || (IsPet() && GetOwner() && GetOwner()->IsClient())))
	{
		return dmg;
	}
	if (isDebug) { Message(0,"Modding kick damage: %i", dmg); }
	float bonus = (GetSTR() / 80.0) + (GetDEX() / 160.0);
	if (bonus > 1)
	{
		dmg *= bonus;
	}
	if (IsClient())
	{
		const EQEmu::ItemInstance* itm = nullptr;
		itm = CastToClient()->GetInv().GetItem(EQEmu::invslot::slotFeet);
		if (itm)
			dmg += dmg * (itm->GetItem()->AC / 100.0);
	}
	if (isDebug) { Message(0,"Modded kick damage: %i", dmg); }
	return(dmg); 
}

//Slam and bash damage after all other bonuses are applied
// CUSTOM
int32 Mob::mod_bash_damage(int32 dmg) 
{ 
	if (!(IsClient() || (IsPet() && GetOwner() && GetOwner()->IsClient())))
	{
		return dmg;
	}
	if (isDebug) { Message(0,"Modding bash damage: %i", dmg); }
	float bonus = (GetSTR() / 80.0) + (GetDEX() / 160.0);
	if (bonus > 1)
	{
		dmg *= bonus;
	}
	if (IsClient())
	{
		const EQEmu::ItemInstance* itm = nullptr;
		itm = CastToClient()->GetInv().GetItem(EQEmu::invslot::slotSecondary);
		if (itm)
			dmg += dmg * (itm->GetItem()->AC / 100.0);
	}
	if (isDebug) { Message(0,"Modded bash damage: %i", dmg); }
	return(dmg); 
}

//Frenzy damage after all other bonuses are applied
// CUSTOM
int32 Mob::mod_frenzy_damage(int32 dmg) 
{
	if (!(IsClient() || (IsPet() && GetOwner() && GetOwner()->IsClient())))
	{
		return dmg;
	}
	if (isDebug) { Message(0,"Modding frenzy damage: %i", dmg); }
	float bonus = (GetSTR() / 80.0) + (GetDEX() / 160.0);
	if (bonus > 1)
	{
		dmg *= bonus;
	}
	if (isDebug) { Message(0,"Modded frenzy damage: %i", dmg); }
	return(dmg); 
}

//Special attack damage after all other bonuses are applied.
// CUSTOM
int32 Mob::mod_monk_special_damage(int32 ndamage, EQ::skills::SkillType skill_type
{ 
	if (!(IsClient() || (IsPet() && GetOwner() && GetOwner()->IsClient())))
	{
		return ndamage;
	}
	if (isDebug) { Message(0,"Modding monk damage: %i", ndamage); }
	float bonus = (GetSTR() / 80.0) + (GetDEX() / 160.0);
	if (bonus > 1)
	{
		ndamage *= bonus;
	}
	if (IsClient())
	{
		const EQEmu::ItemInstance* itm = nullptr;
		switch (skill_type)
		{
			case EQEmu::skills::SkillTigerClaw:
			case EQEmu::skills::SkillDragonPunch :
			case EQEmu::skills::SkillEagleStrike:
				itm = CastToClient()->GetInv().GetItem(EQEmu::invslot::slotHands);
				if (itm)
					ndamage += ndamage * (itm->GetItem()->AC / 100.0);
				break;
			case EQEmu::skills::SkillFlyingKick:
			case EQEmu::skills::SkillRoundKick:
				itm = CastToClient()->GetInv().GetItem(EQEmu::invslot::slotFeet);
				if (itm)
					ndamage += ndamage * (itm->GetItem()->AC / 100.0);
				break;
		}
	}
	if (isDebug) { Message(0,"Modded monk damage: %i", ndamage); }
	return(ndamage); 
}

//ndamage - Backstab damage as calculated by default formulas
// CUSTOM
int32 Mob::mod_backstab_damage(int32 ndamage) 
{ 
	if (!(IsClient() || (IsPet() && GetOwner() && GetOwner()->IsClient())))
	{
		return ndamage;
	}
	if (isDebug) { Message(0,"Modding backstab damage: %i", ndamage); }
	float bonus = (GetDEX() / 80.0) + (GetSTR() / 160.0);
	if (bonus > 1)
	{
		ndamage *= bonus;
	}
	if (isDebug) { Message(0,"Modded backstab damage: %i", ndamage); }
	return(ndamage); 
}

//Chance for 50+ archery bonus damage if Combat:UseArcheryBonusRoll is true.  Base is Combat:ArcheryBonusChance
int Mob::mod_archery_bonus_chance(int bonuschance, const EQ::ItemInstance* RangeWeapon) { return(bonuschance); }

//Archery bonus damage
// CUSTOM
uint32 Mob::mod_archery_bonus_damage(uint32 MaxDmg, const EQ::ItemInstance* RangeWeapon)
{ 
	// // This gets modified again in mod_archery_damage. This could be useful if rangers suck
	// if (!(IsClient() || (IsPet() && GetOwner()->IsClient())))
	// {
		// return MaxDmg;
	// }
	// Message(0,"Modding bow bonus damage: %i", MaxDmg);
	// float bonus = (GetDEX() / 100.0) + (GetSTR() / 300.0);
	// if (bonus > 1)
	// {
		// MaxDmg *= bonus;
	// }
	// Message(0,"Modded value: %i", MaxDmg);
	return(MaxDmg); 
}

//Final archery damage including bonus if it was applied.
// CUSTOM
int32 Mob::mod_archery_damage(int32 TotalDmg, bool hasbonus, const EQ::ItemInstance* RangeWeapon)
{ 
	if (!(IsClient() || (IsPet() && GetOwner() && GetOwner()->IsClient())))
	{
		return TotalDmg;
	}
	if (isDebug) { Message(0,"Modding archery damage: %i", TotalDmg); }
	float bonus = (GetDEX() / 80.0) + (GetSTR() / 250.0);
	if (bonus > 1)
	{
		TotalDmg *= bonus;
	}
	if (isDebug) { Message(0,"Modded archery damage: %i", TotalDmg); }
	return(TotalDmg); 
}

//Thrown weapon damage after all other calcs
// CUSTOM
uint16 Mob::mod_throwing_damage(uint16 MaxDmg) 
{
	if (!(IsClient() || (IsPet() && GetOwner() && GetOwner()->IsClient())))
	{
		return MaxDmg;
	}
	if (isDebug) { Message(0,"Modding throwing damage: %i", MaxDmg); }
	float bonus = (GetDEX() / 80.0) + (GetSTR() / 250.0);
	if (bonus > 1)
	{
		MaxDmg *= bonus;
	}
	if (isDebug) { Message(0,"Modded throwing damage: %i", MaxDmg); }
	return(MaxDmg); 
}

//Spell cast time in milliseconds - will not sync with client cast time bar, but does work.
// CUSTOM
int32 Mob::mod_cast_time(int32 cast_time) 
{ 
	if (!(IsClient() || (IsPet() && GetOwner() && GetOwner()->IsClient())))
	{
		return cast_time;
	}
	if (isDebug) { Message(0,"Modding cast time: %i", cast_time); }
	float modifier = GetDEX() / 100.0;
	if (modifier > 1)
	{
		cast_time /= modifier;
	}
	if (isDebug) { Message(0,"Modded cast time: %i", cast_time); }
	return(cast_time); 
}

//res - Default buff duration formula
// CUSTOM
int Mob::mod_buff_duration(int res, Mob* caster, Mob* target, uint16 spell_id) 
{ 
	if (!(IsClient() || (IsPet() && GetOwner() && GetOwner()->IsClient())))
	{
		return res;
	}
	if (isDebug) { Message(0,"Modding buff duration: %i", res); }
	if (IsBeneficialSpell(spell_id))
	{
		if (caster)
		{
			float modifier = (caster->GetWIS() + caster->GetINT())/200.0;
			if (modifier > 1)
			{
				res *= modifier;
			}
		}
	}
	if (isDebug) { Message(0,"Modded buff duration: %i", res); }
	return(res); 
}

//Spell stack override - If this returns anything < 2, it will ignore all other stacking rules.
// See spells.cpp: Mob::CheckStackConflict
//  0 - No conflict
//  1 - Overwrite, spellid1 is replaced by spellid2
// -1 - Blocked, spellid2 will not land
//  2 - Default stacking behavior
int Mob::mod_spell_stack(uint16 spellid1, int caster_level1, Mob* caster1, uint16 spellid2, int caster_level2, Mob* caster2) { return(2); }

//Sum of various resists rolled against a value of 200.
int Mob::mod_spell_resist(int resist_chance, int level_mod, int resist_modifier, int target_resist, uint8 resist_type, uint16 spell_id, Mob* caster) {
	return(resist_chance);
}

//Spell is cast by this on spelltar, called from spellontarget after the event_cast_on NPC event
void Mob::mod_spell_cast(uint16 spell_id, Mob* spelltar, bool reflect, bool use_resist_adjust, int16 resist_adjust, bool isproc) { return; }

//At this point all applicable aggro checks have succeeded.  Attacker should aggro unless we return false.
bool Mob::mod_will_aggro(Mob *attacker, Mob *on) { return(true); }
