
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "R1MonsterInfoWidget.generated.h"

UCLASS()
class R1_API UR1MonsterInfoWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void UpdateMonsterInfo(FName CharacterRowName, float HpRatio);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	void RefreshLocalization();

	FName CachedMonsterRowName;

protected:
	UPROPERTY(meta = (BindWidget), BlueprintReadWrite)
	TObjectPtr<class UProgressBar> HpBar;

	UPROPERTY(meta = (BindWidget), BlueprintReadWrite)
	TObjectPtr<class UTextBlock> MonsterName;
};
