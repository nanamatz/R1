#include "R1GameplayTags.h"

namespace R1GameplayTags
{
	//Input action tags
	UE_DEFINE_GAMEPLAY_TAG(Input_Action_SetDestination, "Input.Action.SetDestination");
	UE_DEFINE_GAMEPLAY_TAG(Input_Action_Interaction, "Input.Action.Interaction");
	UE_DEFINE_GAMEPLAY_TAG(Input_Action_Inventory, "Input.Action.Inventory");
	UE_DEFINE_GAMEPLAY_TAG(Input_Action_GameMenu, "Input.Action.GameMenu");

	//Montage event tags
	UE_DEFINE_GAMEPLAY_TAG(Event_Montage_Attack,"Event.Monage.Attack");
	UE_DEFINE_GAMEPLAY_TAG(Event_Montage_Begin,"Event.Monage.Begin");
	UE_DEFINE_GAMEPLAY_TAG(Event_Montage_End,"Event.Monage.End");

	//Abiility tags
	UE_DEFINE_GAMEPLAY_TAG(Ability_Attack,"Ability.Attack");
	UE_DEFINE_GAMEPLAY_TAG(Ability_LevelUp,"Ability.LevelUp");

	//State tags
	UE_DEFINE_GAMEPLAY_TAG(Character_State_Dead,"Character.State.Dead");
	UE_DEFINE_GAMEPLAY_TAG(Character_State_Casting,"Character.State.Casting");

	//Common attributes
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_MaxHealth, "Data.Attribute.MaxHealth");
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_HealthRegeneration, "Data.Attribute.HealthRegeneration");
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_BaseDefence, "Data.Attribute.BaseDefence");
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_BaseDamage, "Data.Attribute.BaseDamage");
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_AttackRange, "Data.Attribute.AttackRange");
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_AttackRadius, "Data.Attribute.AttackRadius");
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_AttackSpeed, "Data.Attribute.AttackSpeed");
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_MoveSpeed, "Data.Attribute.MoveSpeed");

	//player only
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_MaxExp, "Data.Attribute.MaxExp");
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_MaxMana, "Data.Attribute.MaxMana");
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_ManaRegeneration, "Data.Attribute.ManaRegeneration");
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_Level, "Data.Attribute.Level");

	//monster only
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_Xp, "Data.Attribute.Xp");
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_AggroRange, "Data.Attribute.AggroRange");
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_AttackAngle, "Data.Attribute.AttackAngle");



}