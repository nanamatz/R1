

#pragma once

#include "NativeGameplayTags.h"

namespace R1GameplayTags
{
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Action_SetDestination);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Action_Interaction);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Action_Inventory);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Montage_Begin);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Montage_End);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Montage_Attack);
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Attack);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_State_Dead);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_State_Casting);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_MaxHealth);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_BaseDamage);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_BaseDefence);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_AttackRange);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_AttackRadius);

	//player only
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_MaxExp);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_MaxMana);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_Level);

	//monster only
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_Xp);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_AggroRange);

}