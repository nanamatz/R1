

#pragma once

#include "CoreMinimal.h"
#include "UI/R1UserWidget.h"
#include "R1StatDetailRow.generated.h"

class UTextBlock;

/**
 * 
 */
UCLASS()
class R1_API UR1StatDetailRow : public UR1UserWidget
{
	GENERATED_BODY()

public:
	// 기획자가 부모 위젯(CharacterStatUI)의 디테일 패널에서 직접 입력할 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "R1|UI Setup")
	FText EditorAttributeName;

	// C++에서 데이터 주입 짝을 찾을 때 사용할 함수
	FText GetAttributeNameText() const { return EditorAttributeName; }

protected:
	virtual void NativePreConstruct() override;

public:
	void InjectData(const FText& InName, const FText& InValue);
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_AttributeName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Amount;
};
