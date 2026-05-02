

#pragma once

#include "NativeGameplayTags.h"

namespace R1GameplayTags
{
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Action_SetDestination);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Action_LookClick);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Action_LookMouse);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Action_Inventory);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Action_GameMenu);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Action_SkillQ);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Action_SkillW);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Action_SkillE);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Action_SkillR);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Montage_Begin);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Montage_End);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Montage_Attack);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_HitReact);
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Attack);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_FistAttack);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_JumpAttack);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_LevelUp);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_HitReact);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ChainLightning);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_State_Dead);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_State_Casting);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_State_HitReact);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Skill_Magnitude);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Skill_Cost);

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

	// Audio
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Audio_Attack_Sword);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Audio_Skill_ChainLightning);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Audio_UI_Click);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Weapon_Impact);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Skill_ChainLightning);

}