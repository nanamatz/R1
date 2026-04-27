#include "UI/System/R1MonsterInfoSceneWidget.h"
#include "UI/System/R1MonsterInfoWidget.h"
#include "Character/R1Monster.h"
#include "AbilitySystem/Attribute/R1AttributeSet.h"

void UR1MonsterInfoSceneWidget::SetMonster(AR1Monster* InMonster)
{
	if (TargetMonster.IsValid())
	{
		TargetMonster->OnHpChanged.RemoveAll(this);
	}

	TargetMonster = InMonster;

	if (TargetMonster.IsValid())
	{
		TargetMonster->OnHpChanged.AddDynamic(this, &UR1MonsterInfoSceneWidget::HandleHpChanged);

		// 초기 값 설정
		MonsterInfoWidget->UpdateMonsterInfo(TargetMonster->GetCharacterRowName().ToString(), TargetMonster->GetHealthRatio());
	}
}

void UR1MonsterInfoSceneWidget::HandleHpChanged(float Ratio)
{
	if (TargetMonster.IsValid() && MonsterInfoWidget)
	{
		MonsterInfoWidget->UpdateMonsterInfo(TargetMonster->GetCharacterRowName().ToString(), Ratio);
	}
}
