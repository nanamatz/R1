

#pragma once

#include "NativeGameplayTags.h"

namespace R1GameplayTags
{
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Action_SetDestination);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Action_Interaction);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Action_Inventory);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Action_GameMenu);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Montage_Begin);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Montage_End);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Montage_Attack);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_HitReact);
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Attack);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_LevelUp);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_HitReact);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ChainLightning);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_State_Dead);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_State_Casting);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_State_HitReact);

	
	// Common attributes
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_MaxHealth);
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

}