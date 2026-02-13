#include "R1GameplayTags.h"

namespace R1GameplayTags
{
	UE_DEFINE_GAMEPLAY_TAG(Input_Action_SetDestination, "Input.Action.SetDestination");
	UE_DEFINE_GAMEPLAY_TAG(Input_Action_Interaction, "Input.Action.Interaction");
	UE_DEFINE_GAMEPLAY_TAG(Input_Action_Inventory, "Input.Action.Inventory");

	UE_DEFINE_GAMEPLAY_TAG(Event_Montage_Attack,"Event.Monage.Attack");
	UE_DEFINE_GAMEPLAY_TAG(Event_Montage_Begin,"Event.Monage.Begin");
	UE_DEFINE_GAMEPLAY_TAG(Event_Montage_End,"Event.Monage.End");


	UE_DEFINE_GAMEPLAY_TAG(Ability_Attack,"Ability.Attack");
	
	UE_DEFINE_GAMEPLAY_TAG(Character_State_Dead,"Character.State.Dead");
	UE_DEFINE_GAMEPLAY_TAG(Character_State_Casting,"Character.State.Casting");

	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_MaxHealth, "Data.Attribute.MaxHealth");
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_BaseDefence, "Data.Attribute.BaseDefence");
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_BaseDamage, "Data.Attribute.BaseDamage");
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_AttackRange, "Data.Attribute.AttackRange");
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_AttackRadius, "Data.Attribute.AttackRadius");

	//player only
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_MaxExp, "Data.Attribute.MaxExp");
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_MaxMana, "Data.Attribute.MaxMana");
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_Level, "Data.Attribute.Level");

	//monster only
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_Xp, "Data.Attribute.Xp");
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_AggroRange, "Data.Attribute.AggroRange");



}