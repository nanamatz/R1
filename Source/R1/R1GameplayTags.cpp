#include "R1GameplayTags.h"
#include "R1Define.h"

namespace R1GameplayTags
{
	//Input action tags
	UE_DEFINE_GAMEPLAY_TAG(Input_Action_SetDestination, "Input.Action.SetDestination");
	UE_DEFINE_GAMEPLAY_TAG(Input_Action_LookClick, "Input.Action.LookClick");
	UE_DEFINE_GAMEPLAY_TAG(Input_Action_LookMouse, "Input.Action.LookMouse");
	UE_DEFINE_GAMEPLAY_TAG(Input_Action_Inventory, "Input.Action.Inventory");
	UE_DEFINE_GAMEPLAY_TAG(Input_Action_GameMenu, "Input.Action.GameMenu");
	UE_DEFINE_GAMEPLAY_TAG(Input_Action_CharacterStatUI, "Input.Action.CharacterStatUI");
	UE_DEFINE_GAMEPLAY_TAG(Input_Action_SkillQ, "Input.Action.SkillQ");
	UE_DEFINE_GAMEPLAY_TAG(Input_Action_SkillW, "Input.Action.SkillW");
	UE_DEFINE_GAMEPLAY_TAG(Input_Action_SkillE, "Input.Action.SkillE");
	UE_DEFINE_GAMEPLAY_TAG(Input_Action_SkillR, "Input.Action.SkillR");

	//Montage event tags
	UE_DEFINE_GAMEPLAY_TAG(Event_Montage_Attack,"Event.Monage.Attack");
	UE_DEFINE_GAMEPLAY_TAG(Event_Montage_Begin,"Event.Monage.Begin");
	UE_DEFINE_GAMEPLAY_TAG(Event_Montage_End,"Event.Monage.End");
	UE_DEFINE_GAMEPLAY_TAG(Event_HitReact,"Event.HitReact");
	UE_DEFINE_GAMEPLAY_TAG(Event_Hit_Critical, "Event.Hit.Critical");

	//Skill key release event tags (hold-type skills)
	UE_DEFINE_GAMEPLAY_TAG(Event_Skill_Release_Q, "Event.Skill.Release.Q");
	UE_DEFINE_GAMEPLAY_TAG(Event_Skill_Release_W, "Event.Skill.Release.W");
	UE_DEFINE_GAMEPLAY_TAG(Event_Skill_Release_E, "Event.Skill.Release.E");
	UE_DEFINE_GAMEPLAY_TAG(Event_Skill_Release_R, "Event.Skill.Release.R");
	UE_DEFINE_GAMEPLAY_TAG(Event_Montage_BladeWave, "Event.Montage.BladeWave");

	FGameplayTag GetSkillReleaseTag(ER1SkillSlot Slot)
	{
		switch (Slot)
		{
		case ER1SkillSlot::Q: return Event_Skill_Release_Q;
		case ER1SkillSlot::W: return Event_Skill_Release_W;
		case ER1SkillSlot::E: return Event_Skill_Release_E;
		case ER1SkillSlot::R: return Event_Skill_Release_R;
		default:              return FGameplayTag();
		}
	}

	//Abiility tags
	UE_DEFINE_GAMEPLAY_TAG(Ability_Attack,"Ability.Attack");
	UE_DEFINE_GAMEPLAY_TAG(Ability_JumpAttack,"Ability.JumpAttack");

	UE_DEFINE_GAMEPLAY_TAG(Ability_LevelUp,"Ability.LevelUp");
	UE_DEFINE_GAMEPLAY_TAG(Ability_HitReact,"Ability.HitReact");
	UE_DEFINE_GAMEPLAY_TAG(Ability_ChainLightning,"Ability.ChainLightning");

	//Monster tags
	UE_DEFINE_GAMEPLAY_TAG(Ability_Monster_Attack, "Ability.Monster.Attack");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Monster_SpecialAttack, "Ability.Monster.SpecialAttack");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Monster_Skill_GroundAttack, "Ability.Monster.Skill.GroundAttack");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Monster_Skill_WaveAttack, "Ability.Monster.Skill.WaveAttack");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Monster_Skill_LeapAttack, "Ability.Monster.Skill.LeapAttack");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Monster_Skill_Summon, "Ability.Monster.Skill.Summon");


	//State tags
	UE_DEFINE_GAMEPLAY_TAG(Character_State_Dead,"Character.State.Dead");
	UE_DEFINE_GAMEPLAY_TAG(Character_State_Frozen,"Character.State.Frozen");
	UE_DEFINE_GAMEPLAY_TAG(Character_State_Burning,"Character.State.Burning");

	// 상태이상/효과 분류 태그 (GE 에셋 태그·제거 쿼리용)
	UE_DEFINE_GAMEPLAY_TAG(Effect_DoT, "Effect.DoT");
	UE_DEFINE_GAMEPLAY_TAG(Effect_DoT_Burning, "Effect.DoT.Burning");
	UE_DEFINE_GAMEPLAY_TAG(Effect_Frost, "Effect.Frost");
	UE_DEFINE_GAMEPLAY_TAG(Status_Frozen, "Status.Frozen");
	UE_DEFINE_GAMEPLAY_TAG(Character_State_Casting,"Character.State.Casting");
	UE_DEFINE_GAMEPLAY_TAG(Character_State_HitReact,"Character.State.HitReact");
	UE_DEFINE_GAMEPLAY_TAG(Character_State_UnInterruptable,"Character.State.UnInterruptable");

	UE_DEFINE_GAMEPLAY_TAG(Data_Skill_Magnitude, "Data.Skill.Magnitude");
	UE_DEFINE_GAMEPLAY_TAG(Data_Skill_Cost, "Data.Skill.Cost");
	UE_DEFINE_GAMEPLAY_TAG(Data_Skill_Cooldown, "Data.Skill.Cooldown");
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Skill_BladeWave, "Cooldown.Skill.BladeWave");
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Boss_BabyGround, "Cooldown.Boss.BabyGround");
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Boss_RavagerCharge, "Cooldown.Boss.RavagerCharge");

	//Common attributes
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_MaxHealth, "Data.Attribute.MaxHealth");
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_Health, "Data.Attribute.Health");
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_HealthRegeneration, "Data.Attribute.HealthRegeneration");
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_BaseDefence, "Data.Attribute.BaseDefence");
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_BaseDamage, "Data.Attribute.BaseDamage");
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_AttackRange, "Data.Attribute.AttackRange");
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_AttackRadius, "Data.Attribute.AttackRadius");
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_AttackSpeed, "Data.Attribute.AttackSpeed");
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_CriticalHitChance, "Data.Attribute.CriticalHitChance");
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_CriticalHitMultiplier, "Data.Attribute.CriticalHitMultiplier");
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_MoveSpeed, "Data.Attribute.MoveSpeed");

	//player only
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_MaxExp, "Data.Attribute.MaxExp");
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_MaxMana, "Data.Attribute.MaxMana");
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_Mana, "Data.Attribute.Mana");
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_ManaRegeneration, "Data.Attribute.ManaRegeneration");
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_Level, "Data.Attribute.Level");
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_WeaponDamage, "Data.Attribute.WeaponDamage");
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_EquipDefence, "Data.Attribute.EquipDefence");
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_DamageMultiplier, "Data.Attribute.DamageMultiplier");
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_DefenceMultiplier, "Data.Attribute.DefenceMultiplier");
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_Ability, "Data.Attribute.Ability");

	//monster only
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_Xp, "Data.Attribute.Xp");
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_AggroRange, "Data.Attribute.AggroRange");
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_AttackAngle, "Data.Attribute.AttackAngle");

	// Meta Upgrades
	UE_DEFINE_GAMEPLAY_TAG(Meta_Upgrade_MaxHealth, "Meta.Upgrade.MaxHealth");
	UE_DEFINE_GAMEPLAY_TAG(Meta_Upgrade_MaxMana, "Meta.Upgrade.MaxMana");
	UE_DEFINE_GAMEPLAY_TAG(Meta_Upgrade_Damage, "Meta.Upgrade.Damage");
	UE_DEFINE_GAMEPLAY_TAG(Meta_Upgrade_Defense, "Meta.Upgrade.Defense");
	UE_DEFINE_GAMEPLAY_TAG(Meta_Upgrade_MoveSpeed, "Meta.Upgrade.MoveSpeed");
	UE_DEFINE_GAMEPLAY_TAG(Meta_Upgrade_ExtraGold, "Meta.Upgrade.ExtraGold");
	UE_DEFINE_GAMEPLAY_TAG(Meta_Upgrade_ExtraExp, "Meta.Upgrade.ExtraExp");
	UE_DEFINE_GAMEPLAY_TAG(Meta_Upgrade_HealthRegen, "Meta.Upgrade.HealthRegen");
	UE_DEFINE_GAMEPLAY_TAG(Meta_Upgrade_ManaRegen, "Meta.Upgrade.ManaRegen");
	UE_DEFINE_GAMEPLAY_TAG(Meta_Upgrade_Luck, "Meta.Upgrade.Luck");
	UE_DEFINE_GAMEPLAY_TAG(Meta_Upgrade_Honor, "Meta.Upgrade.Honor");
	UE_DEFINE_GAMEPLAY_TAG(Meta_Upgrade_Revive, "Meta.Upgrade.Revive");

	// Run Stat Upgrades
	UE_DEFINE_GAMEPLAY_TAG(Stat_Run_Points, "Stat.Run.Points");
	UE_DEFINE_GAMEPLAY_TAG(Stat_Run_Upgrade_Damage, "Stat.Run.Upgrade.Damage");
	UE_DEFINE_GAMEPLAY_TAG(Stat_Run_Upgrade_Power, "Stat.Run.Upgrade.Power");

	UE_DEFINE_GAMEPLAY_TAG(Stat_Run_Upgrade_Dexterity, "Stat.Run.Upgrade.Dexterity");
	UE_DEFINE_GAMEPLAY_TAG(Stat_Run_Upgrade_AttackSpeed, "Stat.Run.Upgrade.AttackSpeed");
	UE_DEFINE_GAMEPLAY_TAG(Stat_Run_Upgrade_AttackRange, "Stat.Run.Upgrade.AttackRange");

	UE_DEFINE_GAMEPLAY_TAG(Stat_Run_Upgrade_Speed, "Stat.Run.Upgrade.Speed");
	UE_DEFINE_GAMEPLAY_TAG(Stat_Run_Upgrade_MoveSpeed, "Stat.Run.Upgrade.MoveSpeed");

	UE_DEFINE_GAMEPLAY_TAG(Stat_Run_Upgrade_Wisdom, "Stat.Run.Upgrade.Wisdom");
	UE_DEFINE_GAMEPLAY_TAG(Stat_Run_Upgrade_Mana, "Stat.Run.Upgrade.Mana");

	UE_DEFINE_GAMEPLAY_TAG(Stat_Run_Upgrade_Vitality, "Stat.Run.Upgrade.Vitality");
	UE_DEFINE_GAMEPLAY_TAG(Stat_Run_Upgrade_Health, "Stat.Run.Upgrade.Health");

	UE_DEFINE_GAMEPLAY_TAG(Stat_Run_Upgrade_Sharpness, "Stat.Run.Upgrade.Sharpness");
	UE_DEFINE_GAMEPLAY_TAG(Stat_Run_Upgrade_CritChance, "Stat.Run.Upgrade.CritChance");
	UE_DEFINE_GAMEPLAY_TAG(Stat_Run_Upgrade_CritMulti, "Stat.Run.Upgrade.CritMulti");

	UE_DEFINE_GAMEPLAY_TAG(Stat_Run_Upgrade_Spirit, "Stat.Run.Upgrade.Spirit");
	UE_DEFINE_GAMEPLAY_TAG(Stat_Run_Upgrade_ManaRegen, "Stat.Run.Upgrade.ManaRegen");
	UE_DEFINE_GAMEPLAY_TAG(Stat_Run_Upgrade_HealthRegen, "Stat.Run.Upgrade.HealthRegen");
	// Audio
	UE_DEFINE_GAMEPLAY_TAG(Audio_Attack_Sword, "Audio.Attack.Sword");
	UE_DEFINE_GAMEPLAY_TAG(Audio_Attack_Fist, "Audio.Attack.Fist");
	UE_DEFINE_GAMEPLAY_TAG(Audio_Skill_ChainLightning, "Audio.Skill.ChainLightning");
	UE_DEFINE_GAMEPLAY_TAG(Audio_UI_Click, "Audio.UI.Click");

	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Weapon_Impact, "GameplayCue.Weapon.Impact");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Skill_ChainLightning, "GameplayCue.Skill.ChainLightning");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Status_Burning, "GameplayCue.Status.Burning");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Status_Frozen, "GameplayCue.Status.Frozen");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Status_Frost, "GameplayCue.Status.Frost");

}