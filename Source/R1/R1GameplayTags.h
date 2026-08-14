

#pragma once

#include "NativeGameplayTags.h"

enum class ER1SkillSlot : uint8;

namespace R1GameplayTags
{
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Action_SetDestination);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Action_LookClick);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Action_LookMouse);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Action_Inventory);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Action_GameMenu);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Action_CharacterStatUI);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Action_SkillQ);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Action_SkillW);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Action_SkillE);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Action_SkillR);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Montage_Begin);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Montage_End);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Montage_Attack);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_HitReact);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Hit_Critical);

	// 스킬 키 릴리즈(홀드 해제) 이벤트 — 홀드형 스킬(블레이드 웨이브 등)이 슬롯별로 대기
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Skill_Release_Q);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Skill_Release_W);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Skill_Release_E);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Skill_Release_R);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Montage_BladeWave);

	// 슬롯(Q/W/E/R)에 해당하는 릴리즈 이벤트 태그. None이면 빈 태그.
	FGameplayTag GetSkillReleaseTag(ER1SkillSlot Slot);
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Attack);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_JumpAttack);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Monster_Attack);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Monster_SpecialAttack);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Monster_Skill_WaveAttack);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Monster_Skill_GroundAttack);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Monster_Skill_LeapAttack);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Monster_Skill_Summon);


	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_LevelUp);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_HitReact);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ChainLightning);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_State_Dead);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_State_Frozen);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_State_Burning);

	// 상태이상/효과 분류 태그 (GE 에셋 태그·제거 쿼리용)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Effect_DoT);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Effect_DoT_Burning);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Effect_Frost);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Frozen);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_State_Casting);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_State_HitReact);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_State_UnInterruptable);


	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Skill_Magnitude);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Skill_Cost);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Skill_Cooldown);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cooldown_Skill_BladeWave);

	// Common attributes
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_MaxHealth);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_Health);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_HealthRegeneration);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_MoveSpeed);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_BaseDamage);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_BaseDefence);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_AttackRange);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_AttackRadius);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_AttackSpeed);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_CriticalHitChance);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_CriticalHitMultiplier);


	//player only
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_MaxExp);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_MaxMana);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_Mana);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_ManaRegeneration);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_Level);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_WeaponDamage);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_EquipDefence);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_DamageMultiplier);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_DefenceMultiplier);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_Ability);

	//monster only
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_Xp);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_AggroRange);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_AttackAngle);


	//Meta Progression
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Meta_Upgrade_MaxHealth);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Meta_Upgrade_MaxMana);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Meta_Upgrade_Damage);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Meta_Upgrade_Defense);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Meta_Upgrade_MoveSpeed);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Meta_Upgrade_ExtraGold);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Meta_Upgrade_ExtraExp);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Meta_Upgrade_HealthRegen);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Meta_Upgrade_ManaRegen);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Meta_Upgrade_Luck);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Meta_Upgrade_Honor);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Meta_Upgrade_Revive);

	// Run Stat Upgrades
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Stat_Run_Points);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Stat_Run_Upgrade_Damage);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Stat_Run_Upgrade_Power);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Stat_Run_Upgrade_Dex);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Stat_Run_Upgrade_AttackSpeed);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Stat_Run_Upgrade_AttackRange);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Stat_Run_Upgrade_Speed);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Stat_Run_Upgrade_MoveSpeed);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Stat_Run_Upgrade_Wisdom);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Stat_Run_Upgrade_Mana);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Stat_Run_Upgrade_Vitality);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Stat_Run_Upgrade_Health);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Stat_Run_Upgrade_Sharpness);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Stat_Run_Upgrade_CritChance);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Stat_Run_Upgrade_CritMulti);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Stat_Run_Upgrade_Spirit);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Stat_Run_Upgrade_HealthRegen);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Stat_Run_Upgrade_ManaRegen);

	// Audio
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Audio_Attack_Sword);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Audio_Attack_Fist);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Audio_Skill_ChainLightning);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Audio_UI_Click);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Weapon_Impact);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Skill_ChainLightning);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Status_Burning);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Status_Frozen);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Status_Frost);

}