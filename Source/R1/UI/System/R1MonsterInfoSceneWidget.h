
#pragma once

#include "CoreMinimal.h"
#include "UI/R1UserWidget.h"
#include "R1MonsterInfoSceneWidget.generated.h"

/**
 * 
 */
UCLASS()
class R1_API UR1MonsterInfoSceneWidget : public UR1UserWidget
{
	GENERATED_BODY()

public:
	void SetMonster(class AR1Monster* InMonster);

private:
	UFUNCTION()
	void HandleHpChanged(float Ratio);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UR1MonsterInfoWidget> MonsterInfoWidget;

	UPROPERTY()
	TWeakObjectPtr<class AR1Monster> TargetMonster;
};
